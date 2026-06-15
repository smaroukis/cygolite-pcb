/*
 * Lab 6A — UART Hello World
 *
 * Target:
 *   ATtiny202
 *
 * Purpose:
 *   Send text from the ATtiny202 to a Linux machine through a USB-UART adapter.
 *
 * Pin usage:
 *   PA6 = USART0 TX
 *   PA7 = USART0 RX
 *
 * Wiring:
 *   PA6 / TX -> USB-UART RX
 *   PA7 / RX -> USB-UART TX
 *   GND      -> USB-UART GND
 */

#define F_CPU 3333333UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define UART_BAUD 9600UL

#define UART_TX_PIN_bm PIN6_bm   // PA6 = USART0 TX
#define UART_RX_PIN_bm PIN7_bm   // PA7 = USART0 RX

static void uart0_init(void) {
    /*
     * Set PA6 as output for UART TX.
     * PA7 can remain input for UART RX.
     */
    PORTA.DIRSET = UART_TX_PIN_bm;
    PORTA.DIRCLR = UART_RX_PIN_bm;

    /*
     * Baud-rate formula for tinyAVR 0-series USART:
     *
     *   BAUD = 64 * F_CPU / (16 * baud)
     *
     * With F_CPU = 3,333,333 Hz and baud = 9600:
     *
     *   BAUD ≈ 1388
     */
    USART0.BAUD = (uint16_t)((64UL * F_CPU) / (16UL * UART_BAUD));

    /*
     * Enable transmitter.
     * We can leave receiver disabled for the first test.
     */
    USART0.CTRLB = USART_TXEN_bm;
}

static void uart0_putc(char c) {
    /*
     * Wait until transmit data register is empty.
     */
    while ((USART0.STATUS & USART_DREIF_bm) == 0) {
        ;
    }

    USART0.TXDATAL = c;
}

static void uart0_print(const char *s) {
    while (*s) {
        uart0_putc(*s++);
    }
}

static void uart0_print_u16(uint16_t value) {
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

int main(void) {
    uart0_init();

    uint16_t counter = 0;

    while (1) {
        uart0_print("hello uart counter=");
        uart0_print_u16(counter);
        uart0_print("\r\n");

        counter++;

        _delay_ms(500);
    }
}