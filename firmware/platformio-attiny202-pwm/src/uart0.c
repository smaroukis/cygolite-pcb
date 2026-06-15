#include "uart0.h"

#include <avr/io.h>
#include <stdint.h>

#define UART0_CLOCK_HZ 3333333UL
#define UART0_BAUD    9600UL

#define UART_TX_PIN_bm PIN6_bm   // PA6 = USART0 TX
#define UART_RX_PIN_bm PIN7_bm   // PA7 = USART0 RX

void uart0_init(void) {
    // PA6 output for UART TX
    PORTA.DIRSET = UART_TX_PIN_bm;

    // PA7 input for UART RX
    PORTA.DIRCLR = UART_RX_PIN_bm;

    // Baud rate: BAUD = 64 * clock / (16 * baud)
    USART0.BAUD = (uint16_t)((64UL * UART0_CLOCK_HZ) / (16UL * UART0_BAUD));

    // Enable Tx and Rx
    USART0.CTRLB = USART_TXEN_bm | USART_RXEN_bm;
}

void uart0_putc(char c) {
    while ((USART0.STATUS & USART_DREIF_bm) == 0) {
        ;
    }

    USART0.TXDATAL = c;
}

void uart0_print(const char *s) {
    while (*s) {
        uart0_putc(*s++);
    }
}

void uart0_print_u16(uint16_t value) {
    char digits[5];
    uint8_t count = 0;

    if (value == 0) {
        uart0_putc('0');
        return;
    }

    while (value > 0) {
        digits[count] = '0' + (value % 10);
        value /= 10;
        count++;
    }

    while (count > 0) {
        count--;
        uart0_putc(digits[count]);
    }
}

bool uart0_rx_available(void) {
    return (USART0.STATUS & USART_RXCIF_bm) != 0;
}

char uart0_getc(void) {
    while (!uart0_rx_available()) {
        ;
    }

    return USART0.RXDATAL;
}

bool uart0_try_getc(char *c) {
    if (!uart0_rx_available()) {
        return false;
    }

    *c = USART0.RXDATAL;
    return true;
}