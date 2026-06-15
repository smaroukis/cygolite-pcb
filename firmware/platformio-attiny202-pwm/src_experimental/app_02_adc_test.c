#define F_CPU 3333333UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "uart0.h"
#include "adc0.h"

int main(void) {
    uart0_init();

    // For this standalone ADC test, PA3 is only an analog input.
    PORTA.DIRCLR = PIN3_bm;

    adc0_init_internal_4v3();

    uart0_print("adc test start\r\n");

    while (1) {
        uint16_t raw = adc0_read_ain3_raw();
        uint16_t mv = adc0_raw_to_mv(raw);

        uart0_print("raw=");
        uart0_print_u16(raw);
        uart0_print(" mv=");
        uart0_print_u16(mv);
        uart0_print("\r\n");

        _delay_ms(250);
    }
}