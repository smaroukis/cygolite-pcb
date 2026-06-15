#include "adc0.h"

#include <avr/io.h>
#include <stdint.h>

void adc0_init_internal_4v3(void) {
    // Select internal ADC reference, nominally about 4.3 V.
    VREF.CTRLA = VREF_ADC0REFSEL_4V34_gc;

    // ADC uses internal reference, ADC clock = peripheral clock / 16.
    ADC0.CTRLC = ADC_REFSEL_INTREF_gc | ADC_PRESC_DIV16_gc;

    // Enable ADC.
    ADC0.CTRLA = ADC_ENABLE_bm;
}

uint16_t adc0_read_raw(uint8_t muxpos) {
    // Select ADC input channel.
    ADC0.MUXPOS = muxpos;

    // Start conversion.
    ADC0.COMMAND = ADC_STCONV_bm;

    // Wait for conversion complete.
    while ((ADC0.INTFLAGS & ADC_RESRDY_bm) == 0) {
        ;
    }

    // Clear result-ready flag.
    ADC0.INTFLAGS = ADC_RESRDY_bm;

    // Return 10-bit result: 0 to 1023.
    return ADC0.RES;
}

uint16_t adc0_read_ain3_raw(void) {
    return adc0_read_raw(ADC_MUXPOS_AIN3_gc);
}

uint16_t adc0_raw_to_mv(uint16_t raw) {
    return ((uint32_t)raw * ADC0_REF_MV) / 1023UL;
}

uint16_t adc0_read_ain3_mv(void) {
    uint16_t raw = adc0_read_ain3_raw();
    return adc0_raw_to_mv(raw);
}