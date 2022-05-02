## Pio UART RX for the RP2040

A pio program that implements the UART protocol, for reception only, for the RP2040. It is compatible with the [SDK](https://raspberrypi.github.io/pico-sdk-doxygen/) and [Arduino](https://github.com/earlephilhower/arduino-pico).

How to use it:

- With SDK. Add *uart_rx.pio* to your project. Modify CMakeLists.txt. Add *pico_generate_pio_header* and the required libraries (pico_stdlib, hardware_irq, hardware_pio, hardware_clocks). See [CMakeLists.txt](sdk/CMakeLists.txt)
- With Arduino. Add *uart_rx.pio.h* to your project  

Functions:  

**void uart_rx_init(PIO pio, uint sm, uint pin, uint baudrate, uint irq)**  

Parameters:  
&nbsp;&nbsp;**pio** - load the capture program at pio0 or pio1  
&nbsp;&nbsp;**sm** - load the capture program at selected state machine. Valid values 0 to 4  
&nbsp;&nbsp;**pin** - set receiver pin  
&nbsp;&nbsp;**baudrate** - set the baudrate  
&nbsp;&nbsp;**irq** - select the pio irq. Valid values for pio0: PIO0_IRQ_0, PIO0_IRQ_1 and for pio1: PIO1_IRQ_0, PIO1_IRQ_1. Useful if other states machines are also using irqs.  
\
**void uart_rx_set_handler(uart_rx_handler_t handler)**  

Parameters:  
&nbsp;&nbsp;**handler** - function to handle bytes reception  
\
Handler functions:  
\
**void uart_rx_handler(uint8_t data)**  

Parameters received:  
&nbsp;&nbsp;**data** - byte received   
