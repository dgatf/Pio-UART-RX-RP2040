/*
 * Copyright (c) 2022, Daniel Gorbea
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 
 *
 * Library for UART RX protocol for RP2040
 */

#ifndef UART_RX
#define UART_RX

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "uart_rx.pio.h"

typedef void (*uart_rx_handler_t)(uint8_t data);

uint uart_rx_init(PIO pio, uint pin, uint baudrate, uint irq);
void uart_rx_set_handler(uart_rx_handler_t handler);
void uart_rx_remove();

#endif
