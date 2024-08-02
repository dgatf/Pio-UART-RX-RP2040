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

#ifdef __cplusplus
extern "C" {
#endif

#include "hardware/pio.h"

typedef void (*uart_rx_handler_t)(void);

void uart_rx_init(PIO pio, uint pin, uint baudrate);
void uart_rx_set_handler(uart_rx_handler_t handler, uint irq);
char uart_rx_read(void);
char uart_rx_peek(void);
uint uart_rx_available(void);
bool uart_rx_overflow(void);
void uart_rx_remove(void);

#ifdef __cplusplus
}
#endif

#endif
