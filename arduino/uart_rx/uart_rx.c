/*
 * Copyright (c) 2022, Daniel Gorbea
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * Library for UART RX protocol for RP2040
 */

#include "uart_rx.h"

#define BUFFER_SIZE (1 << UART_RX_BUFFER_RING_BITS)

static uint sm_, offset_, buffer_pos_, irq_;
volatile static uint transfer_count_;
static PIO pio_;
static void (*handler_)() = NULL;
volatile static uint8_t buffer_[BUFFER_SIZE] __attribute__((aligned(BUFFER_SIZE * sizeof(uint8_t))));

#if UART_RX_DMA
static const uint reload_counter_ = 0xffffffff;
static uint dma_channel_, dma_channel_reload_counter_;
#endif

static inline void handler_pio(void);

void uart_rx_init(PIO pio, uint pin, uint baudrate)
{
    // init pio
    pio_ = pio;
    sm_ = pio_claim_unused_sm(pio_, true);
    pio_sm_set_consecutive_pindirs(pio_, sm_, pin, 1, false);
    pio_gpio_init(pio_, pin);
    gpio_pull_up(pin);
    offset_ = pio_add_program(pio_, &uart_rx_program);
    pio_sm_config pio_config = uart_rx_program_get_default_config(offset_);
    sm_config_set_in_pins(&pio_config, pin);
    sm_config_set_jmp_pin(&pio_config, pin);
    sm_config_set_in_shift(&pio_config, true, false, 32);
    sm_config_set_fifo_join(&pio_config, PIO_FIFO_JOIN_RX);
    float div = (float)clock_get_hz(clk_sys) / (UART_RX_CYCLES_PER_BIT * baudrate);
    sm_config_set_clkdiv(&pio_config, div);
    pio_sm_init(pio_, sm_, offset_, &pio_config);

#if UART_RX_DMA
    // init dma reload counter
    dma_channel_reload_counter_ = dma_claim_unused_channel(true);
    dma_channel_ = dma_claim_unused_channel(true);
    dma_channel_config config_dma_channel_reload_counter = dma_channel_get_default_config(dma_channel_reload_counter_);
    channel_config_set_transfer_data_size(&config_dma_channel_reload_counter, DMA_SIZE_32);
    channel_config_set_write_increment(&config_dma_channel_reload_counter, false);
    channel_config_set_read_increment(&config_dma_channel_reload_counter, false);
    dma_channel_configure(
        dma_channel_reload_counter_,
        &config_dma_channel_reload_counter,
        &dma_hw->ch[dma_channel_].al1_transfer_count_trig, // write address
        &reload_counter_,                                  // read address
        1,
        false);

    // init dma buffer
    dma_channel_config config_dma = dma_channel_get_default_config(dma_channel_);
    channel_config_set_transfer_data_size(&config_dma, DMA_SIZE_8);
    channel_config_set_ring(&config_dma, true, UART_RX_BUFFER_RING_BITS);
    channel_config_set_write_increment(&config_dma, true);
    channel_config_set_read_increment(&config_dma, false);
    channel_config_set_dreq(&config_dma, pio_get_dreq(pio_, sm_, false));
    channel_config_set_chain_to(&config_dma, dma_channel_reload_counter_);
    dma_channel_configure(
        dma_channel_,
        &config_dma,
        &buffer_,                     // write address
        (uint8_t *)&pio0->rxf[0] + 3, // read address
        0xffffffff,
        false);

    dma_channel_start(dma_channel_);
#endif

    pio_sm_set_enabled(pio_, sm_, true);
}

void uart_rx_set_handler(uart_rx_handler_t handler, uint irq)
{
    handler_ = handler;
    if (irq == PIO0_IRQ_0 || irq == PIO1_IRQ_0)
        pio_set_irq0_source_enabled(pio_, (enum pio_interrupt_source)(pis_interrupt0 + UART_RX_IRQ_NUM), true);
    else
        pio_set_irq1_source_enabled(pio_, (enum pio_interrupt_source)(pis_interrupt0 + UART_RX_IRQ_NUM), true);
    pio_interrupt_clear(pio_, UART_RX_IRQ_NUM);
    irq_set_exclusive_handler(irq, handler_pio);
    irq_set_enabled(irq, true);
}

char uart_rx_read(void)
{
#if UART_RX_DMA
    transfer_count_ = 0xffffffff - dma_hw->ch[dma_channel_].transfer_count;
#endif

    if (uart_rx_overflow())
        buffer_pos_ = transfer_count_ - BUFFER_SIZE;
    char value = buffer_[buffer_pos_ % BUFFER_SIZE];
    if (buffer_pos_ < transfer_count_)
        buffer_pos_++;
    return value;
}

char uart_rx_peek(void)
{
#if UART_RX_DMA
    transfer_count_ = 0xffffffff - dma_hw->ch[dma_channel_].transfer_count;
#endif

    if (uart_rx_overflow())
        buffer_pos_ = transfer_count_ - BUFFER_SIZE;
    return buffer_[buffer_pos_ % BUFFER_SIZE];
}

uint uart_rx_available(void)
{
#if UART_RX_DMA
    transfer_count_ = 0xffffffff - dma_hw->ch[dma_channel_].transfer_count;
#endif

    uint available = transfer_count_ - buffer_pos_;
    if (available > BUFFER_SIZE)
        available = BUFFER_SIZE;
    return available;
}

bool uart_rx_overflow(void)
{
#if UART_RX_DMA
    transfer_count_ = 0xffffffff - dma_hw->ch[dma_channel_].transfer_count;
#endif

    if (transfer_count_ - buffer_pos_ > BUFFER_SIZE)
        return true;
    return false;
}

void uart_rx_remove(void)
{
    handler_ = NULL;
    pio_remove_program(pio_, &uart_rx_program, offset_);
    pio_sm_unclaim(pio_, sm_);

#if UART_RX_DMA
    dma_channel_abort(dma_channel_);
    dma_channel_unclaim(dma_channel_);
    dma_channel_abort(dma_channel_reload_counter_);
    dma_channel_unclaim(dma_channel_reload_counter_);
#endif
}

static inline void handler_pio(void)
{
#if UART_RX_DMA
    if (handler_)
        handler_();
#else
    if (handler_)
    {
        while (pio_sm_get_rx_fifo_level(pio_, sm_))
        {
            uint data = pio_sm_get_blocking(pio_, sm_);
            buffer_[transfer_count_ % BUFFER_SIZE] = (data >> 24);
            transfer_count_++;
            if (handler_)
                handler_();
        }
    }
#endif

    pio_interrupt_clear(pio_, UART_RX_IRQ_NUM);
}
