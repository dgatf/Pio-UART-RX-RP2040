/*
 * -------------------------------------------------------------------------------
 *
 * Copyright (c) 2022, Daniel Gorbea
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * -------------------------------------------------------------------------------
 *
 *  Library that implements uart rx protocol for the RP2040 using PIO
 *
 *  Rx pin is GPIO0. Baud rate is 9600
 *
 * -------------------------------------------------------------------------------
 */

#include <stdio.h>

#include "pico/stdlib.h"
#include "uart_rx.h"

void rx_handler(void) {
    // if a quick response is needed, use this interrupt...
}

int main() {
    stdio_init_all();
    PIO pio = pio0;         // values: pio0, pio1
    uint pin = 0;           // rx pin. Any gpio is valid
    uint irq = PIO0_IRQ_0;  // values for pio0: PIO0_IRQ_0, PIO0_IRQ_1. values for pio1: PIO1_IRQ_0, PIO1_IRQ_1
    uint baudrate = 9600;
    uart_rx_init(pio, pin, baudrate);
    uart_rx_set_handler(rx_handler, irq);

    while (1) {
        // ...or poll the buffer
        while (uart_rx_available()) printf("%c", uart_rx_read());
    }
}
