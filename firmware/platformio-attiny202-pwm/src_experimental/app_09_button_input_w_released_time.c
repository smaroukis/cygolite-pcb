#define F_CPU 3333333UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "uart0.h"
#include "tick.h"
#include "button.h"

int main(void) {
    uart0_init();
    tick_init_1ms();
    button_init();

    sei();

    uart0_print("\r\nboot: button press duration test\r\n");

    while (1) {
        while (tick_take_1ms()) {
            button_update_1ms();
        }

        if (button_take_pressed_event()) {
            uart0_print("button pressed\r\n");
        }

        uint16_t duration_ms = 0;

        if (button_take_released_event(&duration_ms)) {
            uart0_print("button released duration_ms=");
            uart0_print_u16(duration_ms);
            uart0_print("\r\n");
        }
    }
}