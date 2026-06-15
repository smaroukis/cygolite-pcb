#define F_CPU 3333333UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "uart0.h"

int main(void) {
    uart0_init();

    uint16_t counter = 0;

    while (1) {
        uart0_print("uart driver ok counter=");
        uart0_print_u16(counter);
        uart0_print("\r\n");

        counter++;

        _delay_ms(500);
    }
}