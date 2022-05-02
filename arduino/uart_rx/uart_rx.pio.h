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
#endif

