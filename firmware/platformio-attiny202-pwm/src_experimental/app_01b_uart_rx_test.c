#define F_CPU 3333333UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#include "uart0.h"

static const uint8_t values[] = {
    0,
    25,
    50,
    75,
    100
};

#define VALUE_COUNT (sizeof(values) / sizeof(values[0]))

static uint8_t index = 0;

static void print_help(void) {
    uart0_print("\r\n");
    uart0_print("UART RX test\r\n");
    uart0_print("commands:\r\n");
    uart0_print("  n = next value\r\n");
    uart0_print("  r = reset to zero\r\n");
    uart0_print("  ? = help\r\n");
    uart0_print("\r\n");
}

static void print_value(void) {
    uart0_print("index=");
    uart0_print_u16(index);

    uart0_print(" value=");
    uart0_print_u16(values[index]);

    uart0_print("\r\n");
}

static void handle_command(char c) {
    // Ignore Enter/newline characters.
    if (c == '\r' || c == '\n') {
        return;
    }

    // Echo command so you know the ATtiny received it.
    uart0_print("received: ");
    uart0_putc(c);
    uart0_print("\r\n");

    switch (c) {
        case 'n':
            index++;

            if (index >= VALUE_COUNT) {
                index = 0;
            }

            print_value();
            break;

        case 'r':
            index = 0;
            print_value();
            break;

        case '?':
            print_help();
            print_value();
            break;

        default:
            uart0_print("unknown command\r\n");
            print_help();
            break;
    }
}

int main(void) {
    uart0_init();

    uart0_print("\r\nboot\r\n");
    print_help();
    print_value();

    while (1) {
        char c;

        if (uart0_try_getc(&c)) {
            handle_command(c);
        }

        // Nothing else yet.
    }
}