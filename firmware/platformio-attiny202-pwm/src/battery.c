#include "battery.h"

#include <avr/io.h>
#include <stdint.h>

#undef F_CPU
#define F_CPU 3333333UL
#include <util/delay.h>

#include "adc0.h"
#include "pwm0.h"

#define BATTERY_ADC_SETTLE_US 300
#define BATTERY_ADC_SAMPLES   2

/*
 * Effective voltage divider factor:
 *   K = Vpin / Vbat
 *
 * Therefore:
 *   Vbat = Vpin / K
 *
 * Store K as a fraction to avoid floating point.
 *
 * Example ideal 100k / 100k divider:
 *   K = 0.5 = 1 / 2
 */
#define BATTERY_DIVIDER_K_NUM  4975UL // K=0.4975
#define BATTERY_DIVIDER_K_DEN  10000UL

// ================ Static Declares to see Privates ===============
static uint16_t battery_pin_mv_to_battery_mv(uint16_t pin_mv);


// ================= Pub Fns ================

void battery_init(void) {
    adc0_init_internal_4v3();
}

uint16_t battery_read_muxed_pa3_raw(void) {
    uint8_t saved_duty = pwm0_get_duty_8bit();

    /*
     * PA3 was PWM output.
     * Temporarily convert it to ADC input.
     */
    pwm0_disable_pa3_output();

    PORTA.DIRCLR = PIN3_bm;

    _delay_us(BATTERY_ADC_SETTLE_US);

    /*
     * Throwaway read after switching PA3 mode.
     */
    (void)adc0_read_ain3_raw();

    uint32_t sum = 0;

    for (uint8_t i = 0; i < BATTERY_ADC_SAMPLES; i++) {
        sum += adc0_read_ain3_raw();
    }

    uint16_t raw = sum / BATTERY_ADC_SAMPLES;

    /*
     * Restore PA3 as PWM output with previous duty.
     */
    PORTA.DIRSET = PIN3_bm;

    pwm0_enable_pa3_output();
    pwm0_set_duty_8bit(saved_duty);

    return raw;
}

uint16_t battery_read_muxed_pa3_pin_mv(void) {
    uint16_t raw = battery_read_muxed_pa3_raw();
    return adc0_raw_to_mv(raw);
}

uint16_t battery_read_muxed_pa3_battery_mv(void) {
    uint16_t pin_mv = battery_read_muxed_pa3_pin_mv();
    return battery_pin_mv_to_battery_mv(pin_mv);
}


// ================= Private ======================
static uint16_t battery_pin_mv_to_battery_mv(uint16_t pin_mv) {
    /*
     * K = Vpin / Vbat
     *
     * Vbat = Vpin / K
     *
     * Since:
     *
     *   K = BATTERY_DIVIDER_K_NUM / BATTERY_DIVIDER_K_DEN
     *
     * Then:
     *
     *   Vbat = Vpin * BATTERY_DIVIDER_K_DEN / BATTERY_DIVIDER_K_NUM
     */
    uint32_t battery_mv =
        ((uint32_t)pin_mv * BATTERY_DIVIDER_K_DEN + (BATTERY_DIVIDER_K_NUM / 2))
        / BATTERY_DIVIDER_K_NUM;

    return (uint16_t)battery_mv;
}