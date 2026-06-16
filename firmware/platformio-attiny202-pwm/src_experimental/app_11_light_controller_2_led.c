#define F_CPU 3333333UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

#include "uart0.h"
#include "tick.h"
#include "button.h"
#include "pwm0.h"
#include "light_controller.h"

#define DUTY_OFF     0
#define DUTY_DIM     64
#define DUTY_BRIGHT  192
#define DUTY_FLASH   255

#define FLASH_HALF_PERIOD_MS 500

static LightState output_state = LIGHT_STATE_S0_OFF;
static uint16_t flash_timer_ms = 0;
static bool flash_on = false;

static void print_state(void) {
    uart0_print("state=");
    uart0_print(light_controller_state_name());
    uart0_print("\r\n");
}

static void print_state_changed(void) {
    uart0_print("state changed -> ");
    uart0_print(light_controller_state_name());
    uart0_print("\r\n");
}

static void light_output_apply_state(void) {
    LightState state = light_controller_get_state();

    output_state = state;
    flash_timer_ms = 0;
    flash_on = false;

    switch (state) {
        case LIGHT_STATE_S0_OFF:
            pwm0_set_duty_8bit(DUTY_OFF);
            uart0_print("output: OFF duty=");
            uart0_print_u16(DUTY_OFF);
            uart0_print("\r\n");
            break;

        case LIGHT_STATE_S1_DIM:
            pwm0_set_duty_8bit(DUTY_DIM);
            uart0_print("output: DIM duty=");
            uart0_print_u16(DUTY_DIM);
            uart0_print("\r\n");
            break;

        case LIGHT_STATE_S2_BRIGHT:
            pwm0_set_duty_8bit(DUTY_BRIGHT);
            uart0_print("output: BRIGHT duty=");
            uart0_print_u16(DUTY_BRIGHT);
            uart0_print("\r\n");
            break;

        case LIGHT_STATE_S3_FLASH:
            /*
            * Start flash mode in the OFF phase.
            * This gives an immediate visible transition into flash mode.
            */
            flash_on = false;
            flash_timer_ms = 0;

            pwm0_set_duty_8bit(DUTY_OFF);

            uart0_print("output: FLASH start OFF phase\r\n");
            break;

        default:
            pwm0_set_duty_8bit(DUTY_OFF);
            uart0_print("output: UNKNOWN -> OFF\r\n");
            break;
    }
}

static void light_output_update_1ms(void) {
    LightState state = light_controller_get_state();

    /*
     * If something changed state, apply the new output behavior.
     */
    if (state != output_state) {
        light_output_apply_state();
    }

    /*
     * Flash mode is time-dependent.
     * All other modes are static duty.
     */
    if (state == LIGHT_STATE_S3_FLASH) {
        flash_timer_ms++;

        if (flash_timer_ms >= FLASH_HALF_PERIOD_MS) {
            flash_timer_ms = 0;
            flash_on = !flash_on;

            if (flash_on) {
                pwm0_set_duty_8bit(DUTY_FLASH);
            } else {
                pwm0_set_duty_8bit(DUTY_OFF);
            }
        }
    }
}

int main(void) {
    uart0_init();
    tick_init_1ms();
    button_init();
    pwm0_init_pa3();
    light_controller_init();

    sei();

    uart0_print("\r\nboot: real LED PWM state machine test\r\n");

    print_state();
    light_output_apply_state();

    while (1) {
        while (tick_take_1ms()) {
            button_update_1ms();

            /*
             * Long press action happens while button is still held.
             */
            if (button_is_pressed()) {
                uint16_t held_ms = button_current_press_duration_ms();

                if (light_controller_update_button_hold(held_ms)) {
                    uart0_print("long press threshold reached -> ");
                    uart0_print(light_controller_state_name());
                    uart0_print("\r\n");

                    light_output_apply_state();
                }
            }

            /*
             * PWM flash timing also runs from the 1 ms tick.
             */
            light_output_update_1ms();
        }

        /*
         * Short press action happens on release,
         * unless long press was already consumed.
         */
        uint16_t duration_ms = 0;

        if (button_take_released_event(&duration_ms)) {
            uart0_print("button released duration_ms=");
            uart0_print_u16(duration_ms);
            uart0_print("\r\n");

            bool changed = light_controller_handle_button_release(duration_ms);

            if (changed) {
                print_state_changed();
                light_output_apply_state();
            } else {
                uart0_print("state unchanged -> ");
                uart0_print(light_controller_state_name());
                uart0_print("\r\n");
            }
        }
    }
}