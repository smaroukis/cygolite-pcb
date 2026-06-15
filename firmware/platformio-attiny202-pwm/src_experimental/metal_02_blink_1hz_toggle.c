
#define F_CPU 3333333UL

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    // Set PA3 as output
    PORTA.DIRSET = PIN3_bm;

    while (1) {
        // Toggle PA3
        PORTA.OUTTGL = PIN3_bm;

        // Wait 500 ms
        _delay_ms(500);
    }
}