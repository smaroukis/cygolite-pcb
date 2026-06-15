#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>

void battery_init(void);

uint16_t battery_read_muxed_pa3_raw(void);

uint16_t battery_read_muxed_pa3_pin_mv(void);

uint16_t battery_read_muxed_pa3_battery_mv(void);

#endif