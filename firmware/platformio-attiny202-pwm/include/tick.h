#ifndef TICK_H
#define TICK_H

#include <stdbool.h>
#include <stdint.h>

void tick_init_1ms(void);

bool tick_take_1ms(void);

#endif