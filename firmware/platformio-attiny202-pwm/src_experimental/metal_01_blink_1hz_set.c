
#define F_CPU 3333333UL

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    PORTA.DIRSET = PIN3_bm;   // PA3 output

    while (1) {
        PORTA.OUTSET = PIN3_bm;  // PA3 high
        _delay_ms(500);

        PORTA.OUTCLR = PIN3_bm;  // PA3 low
        _delay_ms(500);
    }
}