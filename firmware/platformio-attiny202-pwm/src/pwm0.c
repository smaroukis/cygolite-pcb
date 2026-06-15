#include "pwm0.h"

#include <avr/io.h>
#include <stdint.h>

#define PWM0_PIN_bm PIN3_bm   // PA3 / TCA0 WO0
static uint8_t pwm0_current_duty = 0;

void pwm0_init_pa3(void) {
    // PA3 output
    PORTA.DIRSET = PWM0_PIN_bm;

    // Stop TCA0 while configuring.
    TCA0.SINGLE.CTRLA = 0;

    /*
     * Single-slope PWM.
     * Enable compare channel 0 output = WO0 on PA3.
     */
    TCA0.SINGLE.CTRLB =
        TCA_SINGLE_WGMODE_SINGLESLOPE_gc |
        TCA_SINGLE_CMP0EN_bm;

    /*
     * 8-bit-style PWM period.
     *
     * With F_CPU ≈ 3.333 MHz:
     *   PWM frequency ≈ 3,333,333 / 256
     *   PWM frequency ≈ 13 kHz
     */
    TCA0.SINGLE.PER = 255;

    // Start at 0% duty.
    TCA0.SINGLE.CMP0 = 0;

    // Start timer, no prescaler.
    TCA0.SINGLE.CTRLA =
        TCA_SINGLE_CLKSEL_DIV1_gc |
        TCA_SINGLE_ENABLE_bm;
}

void pwm0_set_duty_8bit(uint8_t duty) {
    pwm0_current_duty = duty;

    if (duty == 0) {
        TCA0.SINGLE.CTRLB &= ~TCA_SINGLE_CMP0EN_bp; // clear -> disable
        PORTA.DIRSET = PWM0_PIN_bm; 
        PORTA.OUTCLR = PWM0_PIN_bm;
        return;
    }

    if (duty >= 255) {
        TCA0.SINGLE.CTRLB &= ~TCA_SINGLE_CMP0EN_bp; // clear -> disable
        PORTA.DIRSET = PWM0_PIN_bm; 
        PORTA.OUTCLR = PWM0_PIN_bm;
        return;
    }

    // else
    PORTA.DIRSET = PWM0_PIN_bm;
    TCA0.SINGLE.CMP0 = duty;
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm; // enable
}

uint8_t pwm0_get_duty_8bit(void) {
    return pwm0_current_duty;
}

void pwm0_disable_pa3_output(void) {
    /*
     * Disconnect TCA0 compare channel 0 from PA3.
     * The timer can keep running internally.
     */
    TCA0.SINGLE.CTRLB &= ~TCA_SINGLE_CMP0EN_bm;

    // Force PA3 low while PWM is disconnected.
    PORTA.OUTCLR = PWM0_PIN_bm;
}

void pwm0_enable_pa3_output(void) {
    // Make sure PA3 is configured as output.
    PORTA.DIRSET = PWM0_PIN_bm;

    // Reconnect TCA0 compare channel 0 to PA3.
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm;
}