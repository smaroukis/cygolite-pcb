// Results
// 440Hz at 1.0V input
// 465Hz at 1.1V input
// 569Hz at 2.0V input
// what is overhead of sw delay? 1.0V should be 1kHz, or 0.5ms = 500us half period. 
// actual half at 440Hz is 1/2*1/440 - 1.136ms which has 1.136 - 0.5ms = 636us over head
// check at 569Hz: expected at 2.0V should be 2kHz or 0.25ms half period. Actual is 1/2*1/569 = 0.88ms or 0.25 - 0.88 = 630us overhead (deterministic
// So we found our smoking gun

/*
 * Lab 4A — Analog Voltage to Output Frequency
 *
 * Target: ATtiny202
 *
 * Purpose:
 *   Read an analog voltage on PA2 / AIN2 and output a square wave on PA3.
 *
 * Behavior:
 *   Output frequency roughly follows input voltage:
 *
 *     0.3 V  ->  300 Hz
 *     1.0 V  ->  1 kHz
 *     2.5 V  ->  2.5 kHz
 *     4.3 V  ->  4.3 kHz
 *
 * ADC reference:
 *   Uses the internal ~4.3 V reference.
 *
 * Pin usage:
 *   PA2 / AIN2 = analog input
 *   PA3        = digital square-wave output
 *
 * Note:
 *   This version still uses software timing, but uses _delay_loop_2()
 *   instead of repeatedly calling _delay_us(1).
 */

#define F_CPU 3333333UL

#include <avr/io.h>
#include <util/delay.h>
#include <util/delay_basic.h>
#include <stdint.h>

#define ANALOG_PIN_bm PIN2_bm
#define OUTPUT_PIN_bm PIN3_bm

// Nominal internal ADC reference voltage in millivolts.
// Adjust this after calibration if needed.
#define ADC_REF_MV 4300UL

static void adc_init(void) {
    // PA2 input
    PORTA.DIRCLR = ANALOG_PIN_bm;

    // Select internal ADC reference, nominally about 4.3 V
    VREF.CTRLA = VREF_ADC0REFSEL_4V34_gc;

    // Use internal reference for ADC0 and set ADC clock prescaler
    ADC0.CTRLC = ADC_REFSEL_INTREF_gc | ADC_PRESC_DIV16_gc;

    // Select ADC input channel AIN2, which is PA2
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;

    // Enable ADC
    ADC0.CTRLA = ADC_ENABLE_bm;
}

static uint16_t adc_read(void) {
    // Start ADC conversion
    ADC0.COMMAND = ADC_STCONV_bm;

    // Wait until conversion result is ready
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) {
        ;
    }

    // Clear result-ready flag
    ADC0.INTFLAGS = ADC_RESRDY_bm;

    // Return 10-bit ADC result: 0 to 1023
    return ADC0.RES;
}

static void delay_us_variable(uint16_t us) {
    /*
     * _delay_loop_2(count) takes about 4 CPU cycles per count.
     *
     * For a desired delay in microseconds:
     *
     *   counts = us * F_CPU / 4,000,000
     *
     * Example at F_CPU = 3,333,333 Hz:
     *
     *   500 us -> about 416 counts
     */
    uint16_t counts = ((uint32_t)us * (F_CPU / 1000UL)) / 4000UL;

    if (counts == 0) {
        counts = 1;
    }

    _delay_loop_2(counts);
}

int main(void) {
    adc_init();

    // PA3 output
    PORTA.DIRSET = OUTPUT_PIN_bm;

    while (1) {
        uint16_t adc = adc_read();

        /*
         * Convert ADC reading to frequency.
         *
         * With ADC_REF_MV = 4300:
         *   adc = 0    -> 0 Hz-ish
         *   adc = 1023 -> about 4300 Hz
         *
         * Approximate mapping:
         *   input millivolts ≈ output Hz
         */
        uint16_t frequency_hz = ((uint32_t)adc * ADC_REF_MV) / 1023UL;

        if (frequency_hz < 1) {
            PORTA.OUTCLR = OUTPUT_PIN_bm;
            continue;
        }

        // Square wave:
        // half_period_us = 500,000 / frequency
        uint16_t half_period_us = 500000UL / frequency_hz;

        PORTA.OUTTGL = OUTPUT_PIN_bm;
        delay_us_variable(half_period_us);
    }
}