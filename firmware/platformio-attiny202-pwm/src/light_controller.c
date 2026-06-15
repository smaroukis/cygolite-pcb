#include "light_controller.h"

#include <stdint.h>
#include <stdbool.h>

#define LONG_PRESS_MS 1000

static LightState current_state = LIGHT_STATE_S0_OFF;
static bool long_press_consumed = false;

void light_controller_init(void) {
    current_state = LIGHT_STATE_S0_OFF;
    long_press_consumed = false;
}

bool light_controller_update_button_hold(uint16_t held_ms) {
    LightState previous_state = current_state;

    /*
     * Long press action happens while the button is still held.
     * It only fires once per physical hold.
     */
    if (!long_press_consumed && held_ms >= LONG_PRESS_MS) {
        current_state = LIGHT_STATE_S0_OFF;
        long_press_consumed = true;
    }

    return current_state != previous_state;
}

bool light_controller_handle_button_release(uint16_t duration_ms) {
    (void)duration_ms;

    LightState previous_state = current_state;

    /*
     * If the long-press action already happened while held,
     * release should do nothing.
     */
    if (long_press_consumed) {
        long_press_consumed = false;
        return false;
    }

    /*
     * Otherwise this was a short press.
     *
     * Short press cycle:
     *   S0 -> S1 -> S2 -> S3 -> S1
     */
    switch (current_state) {
        case LIGHT_STATE_S0_OFF:
            current_state = LIGHT_STATE_S1_DIM;
            break;

        case LIGHT_STATE_S1_DIM:
            current_state = LIGHT_STATE_S2_BRIGHT;
            break;

        case LIGHT_STATE_S2_BRIGHT:
            current_state = LIGHT_STATE_S3_FLASH;
            break;

        case LIGHT_STATE_S3_FLASH:
            current_state = LIGHT_STATE_S1_DIM;
            break;

        default:
            current_state = LIGHT_STATE_S0_OFF;
            break;
    }

    return current_state != previous_state;
}

LightState light_controller_get_state(void) {
    return current_state;
}

const char *light_controller_state_name(void) {
    switch (current_state) {
        case LIGHT_STATE_S0_OFF:
            return "S0_OFF";

        case LIGHT_STATE_S1_DIM:
            return "S1_DIM";

        case LIGHT_STATE_S2_BRIGHT:
            return "S2_BRIGHT";

        case LIGHT_STATE_S3_FLASH:
            return "S3_FLASH";

        default:
            return "UNKNOWN";
    }
}