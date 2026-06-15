#define F_CPU 3333333UL

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    PORTA.DIRSET = PIN3_bm;

    while (1) {
        PORTA.OUTSET = PIN3_bm;
        _delay_us(500);

        PORTA.OUTCLR = PIN3_bm;
        _delay_us(500);
    }
}