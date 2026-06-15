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

    uart0_print("\r\nboot: nonblocking button debounce test\r\n");

    uint16_t press_count = 0;

    while (1) {
        /*
         * Run 1 ms tasks whenever the timer says one tick elapsed.
         * This is not a delay. Main can do other work between ticks.
         */
        while (tick_take_1ms()) {
            button_update_1ms();
        }

        /*
         * Consume button event outside the ISR.
         * No UART printing inside interrupts.
         */
        if (button_take_pressed_event()) {
            press_count++;

            uart0_print("button press count=");
            uart0_print_u16(press_count);
            uart0_print("\r\n");
        }

        /*
         * Later:
         *   light_controller_update();
         *   battery_update();
         *   pwm_update();
         */
    }
}