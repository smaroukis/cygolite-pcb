
/*
 * Lab 4B — Analog Voltage to Output Frequency - Test for AIN3
 *
 * Target:
 *   ATtiny202
 *
 * Purpose:
 *   Read an analog voltage on PA3 / AIN3 and output a square wave on PA2.
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
 *   PA3 / AIN3
 *   PA2        = digital square-wave output bit banged
 *
 * Key teaching idea:
 *   The waveform timing loop should be kept fast.
 *   The ADC is read only occasionally so ADC conversion/math overhead
 *   does not dominate every output edge.
 */

#define F_CPU 3333333UL

#include <avr/io.h>
#include <util/delay_basic.h>
#include <stdint.h>

#define ANALOG_PIN_bm PIN3_bm
#define OUTPUT_PIN_bm PIN2_bm

// Nominal internal ADC reference voltage in millivolts.
// Adjust this after calibration if needed.
#define ADC_REF_MV 4300UL

// Number of output edges between ADC updates.
// Larger = steadier waveform, slower response to input changes.
// Smaller = faster response, more timing disturbance.
#define ADC_UPDATE_INTERVAL_EDGES 64

static void adc_init(void) {
    // Setup the Analog pin
    PORTA.DIRCLR = ANALOG_PIN_bm;

    // Select internal ADC reference, nominally about 4.3 V.
    VREF.CTRLA = VREF_ADC0REFSEL_4V34_gc;

    // Use internal reference for ADC0 and set ADC clock prescaler.
    ADC0.CTRLC = ADC_REFSEL_INTREF_gc | ADC_PRESC_DIV16_gc;

    // Select ADC input channel 
    ADC0.MUXPOS = ADC_MUXPOS_AIN3_gc;

    // Enable ADC.
    ADC0.CTRLA = ADC_ENABLE_bm;
}

static uint16_t adc_read(void) {
    // Start ADC conversion.
    ADC0.COMMAND = ADC_STCONV_bm;

    // Wait until conversion result is ready.
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) {
        ;
    }

    // Clear result-ready flag.
    ADC0.INTFLAGS = ADC_RESRDY_bm;

    // Return 10-bit ADC result: 0 to 1023.
    return ADC0.RES;
}

static void delay_us_variable(uint16_t delay_us) {
    const uint32_t MICROSECONDS_PER_SECOND = 1000000UL;
    const uint8_t CYCLES_PER_DELAY_COUNT = 4;

    /*
     * Step 1:
     *   Convert requested delay from microseconds to CPU cycles.
     *   total_cycles = delay_us * F_CPU / 1,000,000
     *
     * Example at F_CPU = 3,333,333 Hz:
     *   delay_us = 500
     *   total_cycles ≈ 500 * 3,333,333 / 1,000,000
     *   total_cycles ≈ 1667 cycles
     */
    uint32_t total_cycles =
        (((uint32_t)delay_us * F_CPU) + (MICROSECONDS_PER_SECOND / 2))
        / MICROSECONDS_PER_SECOND; // round to nearest integer trick = (value + divisor / 2) / divisor

    /*
     * Step 2:
     *   Convert CPU cycles to _delay_loop_2() counts.
     *   _delay_loop_2(counts) takes about 4 CPU cycles per count.
     *   counts = total_cycles / 4
     *
     * Example:
     *   total_cycles = 1667
     *   counts ≈ 1667 / 4
     *   counts ≈ 417
     */
    uint16_t counts =
        (total_cycles + (CYCLES_PER_DELAY_COUNT / 2))
        / CYCLES_PER_DELAY_COUNT;

    // Avoid passing zero to _delay_loop_2().
    if (counts == 0) {
        counts = 1;
    }

    _delay_loop_2(counts);
}

static uint16_t adc_to_frequency_hz(uint16_t adc) {
    /*
     * Convert ADC reading to frequency.
     *
     * With ADC_REF_MV = 4300:
     *
     *   adc = 0    -> 0 Hz-ish
     *   adc = 1023 -> about 4300 Hz
     *
     * Approximate mapping:
     *
     *   input millivolts ≈ output Hz
     */
    return ((uint32_t)adc * ADC_REF_MV) / 1023UL;
}

static uint16_t frequency_to_half_period_us(uint16_t frequency_hz) {
    if (frequency_hz < 1) {
        return 0;
    }

    /*
     * Square wave:
     *
     *   frequency = 1 / period
     *   period_us = 1,000,000 / frequency_hz
     *   half_period_us = 500,000 / frequency_hz
     */
    return 500000UL / frequency_hz;
}

int main(void) {
    adc_init();

    // Set pin output in header
    PORTA.DIRSET = OUTPUT_PIN_bm;

    // Default to about 1 kHz until the first ADC update.
    uint16_t half_period_us = 500;

    uint8_t update_counter = 0;

    while (1) {
        // Toggle output once per half-period.
        PORTA.OUTTGL = OUTPUT_PIN_bm;

        if (half_period_us > 0) {
            delay_us_variable(half_period_us);
        } else {
            PORTA.OUTCLR = OUTPUT_PIN_bm;
        }

        update_counter++;

        // Only read on predefined interval, not every loop.
        if (update_counter >= ADC_UPDATE_INTERVAL_EDGES) {
            update_counter = 0;

            uint16_t adc = adc_read();
            uint16_t frequency_hz = adc_to_frequency_hz(adc);

            half_period_us = frequency_to_half_period_us(frequency_hz);
        }
    }
}