#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    LIGHT_STATE_S0_OFF,
    LIGHT_STATE_S1_DIM,
    LIGHT_STATE_S2_BRIGHT,
    LIGHT_STATE_S3_FLASH
} LightState;

void light_controller_init(void);

bool light_controller_handle_button_release(uint16_t duration_ms);

bool light_controller_update_button_hold(uint16_t held_ms);

LightState light_controller_get_state(void);

const char *light_controller_state_name(void);

#endif