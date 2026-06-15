#ifndef UART0_H
#define UART0_H

#include <stdint.h>
#include <stdbool.h>

void uart0_init(void);

void uart0_putc(char c);

void uart0_print(const char *s);

void uart0_print_u16(uint16_t value);

bool uart0_rx_available(void);

char uart0_getc(void);

bool uart0_try_getc(char *c);

#endif