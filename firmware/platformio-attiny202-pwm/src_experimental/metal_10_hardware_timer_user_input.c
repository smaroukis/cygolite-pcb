/*
 * Lab 5B — Button-Controlled Hardware PWM Duty Cycle
 *
 * Target:
 *   ATtiny202
 *
 * Purpose:
 *   Use a debounced button press to step through PWM duty cycles.
 *
 * Pin usage:
 *   PA1 = button input with external pulldown
 *   PA3 = TCA0 WO0 PWM output
 *
 * Behavior:
 *   Each clean button press advances to the next duty cycle:
 *
 *     0% -> 25% -> 50% -> 75% -> 100% -> back to 0%
 *
 * Key teaching idea:
 *   The button/debounce code only decides when a valid press occurred.
 *   The PWM hardware keeps generating the waveform.
 *   On each button press, software only updates CMP0.
 */

#define F_CPU 3333333UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#include "button_debounce.h"

#define BUTTON_PIN_bm PIN1_bm   // PA1 input
#define PWM_PIN_bm    PIN3_bm   // PA3 / TCA0 WO0 output

#define DEBOUNCE_MS 25

static const uint16_t duty_steps[] = {
    0,    // 0%
    64,   // about 25%
    128,  // about 50%
    192,  // about 75%
    255   // about 100%
}; // max PER = 255 for 13kHz to avoid showing flicker on LED

#define DUTY_STEP_COUNT (sizeof(duty_steps) / sizeof(duty_steps[0]))

static bool button_raw_is_pressed(void) {
    /*
     * External pulldown:
     *
     *   released -> PA1 = LOW
     *   pressed  -> PA1 = HIGH
     */
    return (PORTA.IN & BUTTON_PIN_bm) != 0;
}

static void pwm_init(void) {
    // PA3 output
    PORTA.DIRSET = PWM_PIN_bm;

    // Stop TCA0 while configuring it
    TCA0.SINGLE.CTRLA = 0;

    // Single-slope PWM, enable compare channel 0 output = WO0
    TCA0.SINGLE.CTRLB =
        TCA_SINGLE_WGMODE_SINGLESLOPE_gc |
        TCA_SINGLE_CMP0EN_bm;

    // 8-bit-style PWM period: 0 to 255
    TCA0.SINGLE.PER = 255;

    // Start at 0% duty
    TCA0.SINGLE.CMP0 = 0;

    // Start timer, no prescaler
    TCA0.SINGLE.CTRLA =
        TCA_SINGLE_CLKSEL_DIV1_gc |
        TCA_SINGLE_ENABLE_bm;
}

static void pwm_set_duty(uint16_t duty) {
    /*
     * CMP0 controls compare channel 0 duty.
     *
     * With PER = 255:
     *   CMP0 = 0   -> about 0%
     *   CMP0 = 128 -> about 50%
     *   CMP0 = 255 -> about 100%
     */
    TCA0.SINGLE.CMP0 = duty;
}

int main(void) {
    // PA1 input
    PORTA.DIRCLR = BUTTON_PIN_bm;

    pwm_init();

    ButtonDebounce button;
    button_debounce_init(&button, DEBOUNCE_MS);

    uint8_t duty_index = 0;
    pwm_set_duty(duty_steps[duty_index]);

    while (1) {
        bool raw_pressed = button_raw_is_pressed();

        if (button_debounce_pressed_event(&button, raw_pressed)) {
            duty_index++;

            if (duty_index >= DUTY_STEP_COUNT) {
                duty_index = 0;
            }

            pwm_set_duty(duty_steps[duty_index]);
        }

        /*
         * The debounce module assumes this function is called every 1 ms.
         */
        _delay_ms(1);
    }
}