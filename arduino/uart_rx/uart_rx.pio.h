/**
   -------------------------------------------------------------------------------

   Copyright (c) 2022, Daniel Gorbea
   All rights reserved.

   This source code is licensed under the MIT-style license found in the
   LICENSE file in the root directory of this source tree.

   -------------------------------------------------------------------------------
*/

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

#define IRQ_NUM 0

// ------- //
// uart_rx //
// ------- //

#define uart_rx_wrap_target 0
#define uart_rx_wrap 8

static const uint16_t uart_rx_program_instructions[] = {
            //     .wrap_target
    0x2020, //  0: wait   0 pin, 0                   
    0xea27, //  1: set    x, 7                   [10]
    0x4001, //  2: in     pins, 1                    
    0x0642, //  3: jmp    x--, 2                 [6] 
    0x00c7, //  4: jmp    pin, 7                     
    0x20a0, //  5: wait   1 pin, 0                   
    0x0000, //  6: jmp    0                          
    0x8020, //  7: push   block                      
    0xc000+IRQ_NUM, //  8: irq    nowait 0                   
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program uart_rx_program = {
    .instructions = uart_rx_program_instructions,
    .length = 9,
    .origin = -1,
};

static inline pio_sm_config uart_rx_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + uart_rx_wrap_target, offset + uart_rx_wrap);
    return c;
}

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
typedef void (*uart_rx_handler_t)(uint8_t data);
static uint sm_uart_rx;
static PIO pio_uart_rx;
static void (*uart_rx_handler_p)(uint8_t data) = NULL;
static inline void uart_rx_init(PIO pio, uint sm, uint pin, uint baudrate, uint irq);
static inline void uart_rx_set_handler(uart_rx_handler_t handler);
static inline void uart_rx_irq();
static inline void uart_rx_init(PIO pio, uint sm, uint pin, uint baudrate, uint irq)
{
    pio_uart_rx = pio;
    sm_uart_rx = sm;
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, false);
    pio_gpio_init(pio, pin);
    uint offset = pio_add_program(pio, &uart_rx_program);
    pio_sm_config c = uart_rx_program_get_default_config(offset);
    sm_config_set_in_pins(&c, pin);
    sm_config_set_jmp_pin(&c, pin);
    sm_config_set_in_shift(&c, true, false, 32);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    float div = (float)clock_get_hz(clk_sys) / (8 * baudrate);
    sm_config_set_clkdiv(&c, div);
    if (irq == PIO0_IRQ_0 || irq == PIO1_IRQ_0)
        pio_set_irq0_source_enabled(pio, (enum pio_interrupt_source)(pis_interrupt0 + IRQ_NUM), true);
    else
        pio_set_irq1_source_enabled(pio, (enum pio_interrupt_source)(pis_interrupt0 + IRQ_NUM), true);
    pio_interrupt_clear(pio, IRQ_NUM);
    pio_sm_init(pio, sm_uart_rx, offset, &c);
    pio_sm_set_enabled(pio, sm_uart_rx, true);
    irq_set_exclusive_handler(irq, uart_rx_irq);
    irq_set_enabled(irq, true);
}
static inline void uart_rx_set_handler(uart_rx_handler_t handler)
{
    uart_rx_handler_p = handler;
}
static inline void uart_rx_irq()
{
    pio_interrupt_clear(pio_uart_rx, IRQ_NUM);
    if (uart_rx_handler_p)
    {
        uint data = pio_sm_get_blocking(pio_uart_rx, sm_uart_rx);
        uart_rx_handler_p(data >> 24);
    }
}

#endif
