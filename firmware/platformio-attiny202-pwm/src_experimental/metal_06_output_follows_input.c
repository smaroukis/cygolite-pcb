#define F_CPU 3333333UL

#include <avr/io.h>

int main(void) {
    // PA3 = output
    PORTA.DIRSET = PIN3_bm;

    // PA7 = input
    PORTA.DIRCLR = PIN7_bm;

    while (1) {
        // Read PA7 input
        if (PORTA.IN & PIN7_bm) {
            // If PA7 is high, set PA3 high
            PORTA.OUTSET = PIN3_bm;
        } else {
            // If PA7 is low, set PA3 low
            PORTA.OUTCLR = PIN3_bm;
        }
    }
}