#include "uart.hpp"

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"

#define UART_ID     uart1
#define UART_BAUD   115200
#define DATA_BITS   8
#define STOP_BITS   1
#define PARITY      UART_PARITY_NONE

// DOUT GP5, UART1 RX
#define UART_RX_PIN     5
// Let's just use the corresponding UART1 TX pin
#define UART_TX_PIN     4 // Not used really

#define UART_IRQ    (UART_ID == uart0 ? UART0_IRQ : UART1_IRQ)

#define BUFFER_SIZE 2048
static int buffer_index = 0;
static uint8_t han_buffer[BUFFER_SIZE+1];
static volatile bool han_buffer_complete;

// Interrupt routine
static void on_han_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        
        // Start character
        if (ch == '/') {
            buffer_index = 0;
            han_buffer_complete = false;
        }
        else {
            buffer_index ++;
        }

        if (buffer_index < BUFFER_SIZE-1) {
            // Make sure the string is always null terminated
            han_buffer[buffer_index+1] = 0x00;
            han_buffer[buffer_index] = ch;
        }
        
        // Need to include the crc checksum as well.
        if (ch == '!') {
            han_buffer_complete = true;
        }
    }
}


void han_uart_init() {
    uart_init(UART_ID, 2400);
    buffer_index = 0;
    han_buffer[0] = 0;  // Ensure the string is null terminated
    han_buffer_complete = false;

    //gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART); 
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART); 

    int __unused actual = uart_set_baudrate(UART_ID, UART_BAUD);
    // Turn off CTS/RTS flow control
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // May want to turn this off
    uart_set_fifo_enabled(UART_ID, false);

    irq_set_exclusive_handler(UART_IRQ, on_han_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Enable irq on RX only
    uart_set_irq_enables (UART_ID, true, false);
}


bool han_data_available()
{
    return han_buffer_complete;
}

int han_copy_buffer(uint8_t *dst, size_t n)
{
    const char *buf = (const char *) han_buffer;
    int len = strlen(buf);
    strncpy ((char *)dst, buf, n);
    han_buffer_complete = false;
    return len;
}

const uint8_t *han_raw_buffer()
{
    return han_buffer;
}

void han_mark_buffer_read()
{
    han_buffer_complete = false;
}

