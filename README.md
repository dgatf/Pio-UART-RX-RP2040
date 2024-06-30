## UART RX library for RP2040

A library that implements the UART receiver protocol for the RP2040 using PIO and a DMA buffer (optional).

It is compatible with the [SDK](https://raspberrypi.github.io/pico-sdk-doxygen/) and [Arduino](https://github.com/earlephilhower/arduino-pico).

How to use it:

- With SDK. Add *uart_rx.pio, uart_rx.h and uart_rx.c* to your project. Modify CMakeLists.txt. Add *pico_generate_pio_header* and the required libraries (pico_stdlib, hardware_irq, hardware_pio, hardware_dma). See [CMakeLists.txt](sdk/CMakeLists.txt)
- With Arduino. Add *uart_rx.pio.h, uart_rx.h and uart_rx.c* to your project  
- Change UART_RX_IRQ_NUM if conflicts with other state machines irqs. Valid values 0 to 3.  


Library parameters:

**UART_RX_BUFFER_RING_BITS**. Bit mask for the ring buffer. **BUFFER_SIZE = 1 << UART_RX_BUFFER_RING_BITS**  
**UART_RX_DMA**. 1 to enable DMA buffer, 0 to disable DMA buffer. If enabled, the library uses 2 DMA channels.

Functions:  

**void uart_rx_init(PIO pio, uint pin, uint baudrate)**  

Initialize the UART  

Parameters:  
&nbsp;&nbsp;**pio** - load the capture program at pio0 or pio1  
&nbsp;&nbsp;**pin** - set receiver pin  
&nbsp;&nbsp;**baudrate** - set the baudrate   
\
**void uart_rx_set_handler(uart_rx_handler_t handler, uint irq)**  

Set the interrupt handler  

Parameters:  
&nbsp;&nbsp;**handler** - function to handle bytes reception  
&nbsp;&nbsp;**irq** - select the pio irq. Valid values for pio0: PIO0_IRQ_0, PIO0_IRQ_1 and for pio1: PIO1_IRQ_0, PIO1_IRQ_1. Useful if other states machines are also using irqs.  
\
**char uart_rx_read(void)**

Returns:  
&nbsp;&nbsp;A character from the buffer   
\
**char uart_rx_peek(void)**

Returns:  
&nbsp;&nbsp;A character from the buffer without removing it from the buffer   
\
**uint uart_rx_available(void)**

Returns:  
&nbsp;&nbsp;Available bytes from buffer  
\
**bool uart_rx_overflow(void)**

Returns:  
&nbsp;&nbsp;True if the buffer is overflown or false if not  
\
**void uart_rx_remove(void)**  

Reset handler, remove pio program from memory and free DMA channel if enabled   
\
Handler functions:  
\
**void uart_rx_handler(void)**  

If the handler is defined, it is called when a character is received   
