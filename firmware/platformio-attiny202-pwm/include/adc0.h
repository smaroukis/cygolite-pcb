#ifndef ADC0_DRIVER_H
#define ADC0_DRIVER_H

#include <stdint.h>

#define ADC0_REF_MV 4300UL

void adc0_init_internal_4v3(void);

uint16_t adc0_read_raw(uint8_t muxpos);

uint16_t adc0_read_ain3_raw(void);

uint16_t adc0_raw_to_mv(uint16_t raw);

uint16_t adc0_read_ain3_mv(void);

#endif