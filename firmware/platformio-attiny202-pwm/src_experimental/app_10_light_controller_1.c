#define F_CPU 3333333UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

#include "uart0.h"
#include "tick.h"
#include "button.h"
#include "light_controller.h"

static void print_state(void) {
    uart0_print("state=");
    uart0_print(light_controller_state_name());
    uart0_print("\r\n");
}

int main(void) {
    uart0_init();
    tick_init_1ms();
    button_init();
    light_controller_init();

    sei();

    uart0_print("\r\nboot: light controller state machine test\r\n");
    print_state();

    while (1) {
        while (tick_take_1ms()) {
            button_update_1ms();

            if (button_is_pressed()) {
                uint16_t held_ms = button_current_press_duration_ms();

                if (light_controller_update_button_hold(held_ms)) {
                    uart0_print("long press threshold reached -> ");
                    uart0_print(light_controller_state_name());
                    uart0_print("\r\n");
                }
            }
        }

        if (button_take_pressed_event()) {
            uart0_print("button pressed\r\n");
        }

        uint16_t duration_ms = 0;

        if (button_take_released_event(&duration_ms)) {
            uart0_print("button released duration_ms=");
            uart0_print_u16(duration_ms);
            uart0_print("\r\n");

            bool changed = light_controller_handle_button_release(duration_ms);

            if (changed) {
                uart0_print("state changed -> ");
                uart0_print(light_controller_state_name());
                uart0_print("\r\n");
            } else {
                uart0_print("state unchanged -> ");
                uart0_print(light_controller_state_name());
                uart0_print("\r\n");
            }
        }
    }
}