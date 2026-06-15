/*
 * Lab 5A — Fixed Hardware PWM with TCA0
 *
 * Target:
 *   ATtiny202
 *
 * Purpose:
 *   Generate a fixed PWM waveform on PA3 using TCA0 hardware.
 *
 * Pin usage:
 *   PA3 = TCA0 WO0 PWM output
 *
 * Expected result:
 *   PA3 outputs a fast PWM square wave.
 *
 * With:
 *   F_CPU = 3,333,333 Hz
 *   PER   = 255 (chosen s.t. PWM freq > 13kHz to avoid showing flicker)
 *
 * PWM frequency is approximately:
 *
 *   F_PWM = F_CPU / (PER + 1)
 *         = 3,333,333 / 256
 *         ≈ 13.0 kHz
 *
 * Duty cycle:
 *   CMP0 = 128
 *   PER  = 255
 *
 *   duty ≈ 128 / 256 ≈ 50%
 */

#define F_CPU 3333333UL

#include <avr/io.h>

int main(void) {
    // 1. Make PA3 an output.
    PORTA.DIRSET = PIN3_bm;

    // 2. Stop TCA0 while configuring it.
    TCA0.SINGLE.CTRLA = 0;

    // 3. Configure TCA0 for single-slope PWM.
    //    Enable compare channel 0 output, WO0.
    TCA0.SINGLE.CTRLB =
        TCA_SINGLE_WGMODE_SINGLESLOPE_gc |
        TCA_SINGLE_CMP0EN_bm;

    // 4. Set PWM period.
    //    Timer counts 0, 1, 2, ... 255, then rolls over.
    TCA0.SINGLE.PER = 255;

    // 5. Set duty cycle.
    //    128 out of 256 counts gives about 50% duty.
    TCA0.SINGLE.CMP0 = 128;

    // 6. Start TCA0 with no prescaler.
    TCA0.SINGLE.CTRLA =
        TCA_SINGLE_CLKSEL_DIV1_gc |
        TCA_SINGLE_ENABLE_bm;

    // 7. Nothing needed in the main loop.
    //    Hardware keeps generating PWM.
    while (1) {
    }
}