#include "uart_rx_pio.h"

static uint sm;
static PIO pio;
static void (*uart_rx_handler)(uint8_t data) = NULL;

static inline void handler_pio();

uint uart_rx_init(PIO pio, uint pin, uint baudrate, uint irq)
{
    pio = pio;
    sm = pio_claim_unused_sm(pio, true);

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

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
    irq_set_exclusive_handler(irq, handler_pio);
    irq_set_enabled(irq, true);

    return sm;
}

void uart_rx_set_handler(uart_rx_handler_t handler)
{
    uart_rx_handler = handler;
}

static inline void handler_pio()
{
    pio_interrupt_clear(pio, IRQ_NUM);
    if (uart_rx_handler)
    {
        uint data = pio_sm_get_blocking(pio, sm);
        uart_rx_handler(data >> 24);
    }
}