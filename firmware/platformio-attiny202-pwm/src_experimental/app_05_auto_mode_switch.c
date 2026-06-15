#define F_CPU 3333333UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "uart0.h"
#include "pwm0.h"
#include "adc0.h"

#define PWM_SETTLE_MS 1000
#define ADC_SETTLE_US 300
#define ADC_READ_GAP_MS 100

static void enter_pwm_output_mode(void) {
    /*
     * PA3 becomes an output again.
     * The PWM driver controls whether PA3 is true PWM,
     * forced low, or forced high depending on duty.
     */
    PORTA.DIRSET = PIN3_bm;
    pwm0_enable_pa3_output();
}

static void enter_adc_input_mode(void) {
    /*
     * PA3 becomes high impedance input for ADC.
     *
     * Sequence:
     *   1. Disconnect TCA0 waveform output from PA3.
     *   2. Force PA3 low while still output.
     *   3. Make PA3 input.
     *   4. Wait for external resistor network to settle.
     */
    pwm0_disable_pa3_output();

    PORTA.DIRCLR = PIN3_bm;

    _delay_us(ADC_SETTLE_US);
}

static void run_pwm_step(uint8_t duty, const char *label) {
    enter_pwm_output_mode();

    pwm0_set_duty_8bit(duty);

    uart0_print("PWM duty ");
    uart0_print(label);
    uart0_print(" raw=");
    uart0_print_u16(duty);
    uart0_print("\r\n");

    _delay_ms(PWM_SETTLE_MS);
}

static void print_adc_read(uint8_t read_number) {
    uint16_t raw = adc0_read_ain3_raw();
    uint16_t mv = adc0_raw_to_mv(raw);

    uart0_print("ADC read ");
    uart0_print_u16(read_number);

    uart0_print(": raw=");
    uart0_print_u16(raw);

    uart0_print(" mv=");
    uart0_print_u16(mv);

    uart0_print("\r\n");
}

static void run_adc_phase(void) {
    uart0_print("switching to ADC input mode\r\n");

    enter_adc_input_mode();

    /*
     * Optional throwaway read after switching pin mode.
     * This lets the ADC sampling capacitor settle after the PA3 role changed.
     */
    uint16_t throwaway = adc0_read_ain3_raw();
    (void)throwaway;

    print_adc_read(1);

    _delay_ms(ADC_READ_GAP_MS);

    print_adc_read(2);

    _delay_ms(PWM_SETTLE_MS);
}

int main(void) {
    uart0_init();
    adc0_init_internal_4v3();
    pwm0_init_pa3();

    uart0_print("\r\nboot: automatic PA3 PWM/ADC mux cycle\r\n");

    while (1) {
        run_pwm_step(0,   "0%");
        run_pwm_step(64,  "25%");
        run_pwm_step(192, "75%");
        run_pwm_step(255, "100%");

        run_adc_phase();

        uart0_print("cycle complete\r\n\r\n");
    }
}