/**
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
 *  A pio program that implements a simple uart rx protocol for the RP2040
 * 
 *  Rx pin is GPIO7. Baud rate is 9600
 * 
 * -------------------------------------------------------------------------------
 */

#include "uart_rx_pio.h"

static void rx_handler(uint8_t data)
{
    printf("%c", data);
}

int main()
{
    stdio_init_all();
	
    PIO pio = pio0;        // values: pio0, pio1
    uint sm;               //
    uint pin = 7;          //
    uint irq = PIO0_IRQ_0; // values for pio0: PIO0_IRQ_0, PIO0_IRQ_1. values for pio1: PIO1_IRQ_0, PIO1_IRQ_1
    uint baudrate = 9600;
    
    sm = uart_rx_init(pio, pin, baudrate, irq);
    uart_rx_set_handler(rx_handler);

    while (1)
    {
    }
}
