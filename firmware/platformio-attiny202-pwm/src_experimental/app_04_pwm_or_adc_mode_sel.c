#define F_CPU 3333333UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#include "uart0.h"
#include "pwm0.h"
#include "adc0.h"

static const uint8_t duty_steps[] = {
    0,    // 0%
    64,   // 25%
    128,  // 50%
    192,  // 75%
    255   // 100%
};

#define DUTY_STEP_COUNT (sizeof(duty_steps) / sizeof(duty_steps[0]))

typedef enum {
    PA3_MODE_PWM_OUTPUT,
    PA3_MODE_ADC_INPUT
} Pa3Mode;

static uint8_t duty_index = 0;
static bool debug_enabled = false;
static Pa3Mode pa3_mode = PA3_MODE_PWM_OUTPUT;

static void print_help(void) {
    uart0_print("\r\n");
    uart0_print("PA3 mux manual test\r\n");
    uart0_print("commands:\r\n");
    uart0_print("  o = PA3 PWM output mode\r\n");
    uart0_print("  i = PA3 ADC input mode\r\n");
    uart0_print("  0 = 0% duty\r\n");
    uart0_print("  1 = 25% duty\r\n");
    uart0_print("  2 = 50% duty\r\n");
    uart0_print("  3 = 75% duty\r\n");
    uart0_print("  4 = 100% duty\r\n");
    uart0_print("  n = next duty\r\n");
    uart0_print("  p = previous duty\r\n");
    uart0_print("  r = read ADC once\r\n");
    uart0_print("  s = status\r\n");
    uart0_print("  d = toggle RX debug\r\n");
    uart0_print("  ? = help\r\n");
    uart0_print("\r\n");
}

static const char *mode_name(void) {
    if (pa3_mode == PA3_MODE_PWM_OUTPUT) {
        return "PWM_OUTPUT";
    }

    return "ADC_INPUT";
}

static void apply_duty(void) {
    /*
     * Only actively apply duty while PA3 is in output mode.
     * If PA3 is in ADC mode, remember the duty_index but do not drive PA3.
     */
    if (pa3_mode == PA3_MODE_PWM_OUTPUT) {
        pwm0_set_duty_8bit(duty_steps[duty_index]);
    }
}

static void print_status(void) {
    uart0_print("mode=");
    uart0_print(mode_name());

    uart0_print(" duty_index=");
    uart0_print_u16(duty_index);

    uart0_print(" duty=");
    uart0_print_u16(duty_steps[duty_index]);

    uart0_print(" debug=");
    uart0_print(debug_enabled ? "enabled" : "disabled");

    uart0_print("\r\n");
}

static void print_rx_debug(char c) {
    uint8_t byte = (uint8_t)c;

    uart0_print("raw rx: dec=");
    uart0_print_u16(byte);

    uart0_print(" char=");

    if (byte >= 32 && byte <= 126) {
        uart0_putc(c);
    } else {
        uart0_print(".");
    }

    uart0_print("\r\n");

    if (c == '\r') {
        uart0_print("that was CR\r\n");
    }

    if (c == '\n') {
        uart0_print("that was LF\r\n");
    }
}

static void enter_pwm_output_mode(void) {
    /*
     * PA3 becomes a driven output again.
     * Re-enable PWM output path, then apply remembered duty.
     */
    pa3_mode = PA3_MODE_PWM_OUTPUT;

    pwm0_enable_pa3_output();
    apply_duty();

    uart0_print("entered PWM output mode\r\n");
    print_status();
}

static void enter_adc_input_mode(void) {
    /*
     * PA3 becomes high impedance input for ADC.
     *
     * Sequence:
     *   1. Disconnect TCA0 waveform output from PA3.
     *   2. Force output low while still output, via pwm0_disable_pa3_output().
     *   3. Make PA3 input.
     *   4. Let the external analog network settle.
     */
    pwm0_disable_pa3_output();

    PORTA.DIRCLR = PIN3_bm;

    pa3_mode = PA3_MODE_ADC_INPUT;

    _delay_us(200);

    uart0_print("entered ADC input mode\r\n");
    print_status();
}

static void read_adc_once(void) {
    if (pa3_mode != PA3_MODE_ADC_INPUT) {
        uart0_print("ADC read blocked: switch to input mode first with 'i'\r\n");
        return;
    }

    /*
     * Do one throwaway read after mode switching / source changes.
     * Then take the real reading.
     */
    uint16_t throwaway = adc0_read_ain3_raw();
    (void)throwaway;

    uint16_t raw = adc0_read_ain3_raw();
    uint16_t mv = adc0_raw_to_mv(raw);

    uart0_print("adc raw=");
    uart0_print_u16(raw);

    uart0_print(" mv=");
    uart0_print_u16(mv);

    uart0_print("\r\n");
}

static void next_duty(void) {
    duty_index++;

    if (duty_index >= DUTY_STEP_COUNT) {
        duty_index = 0;
    }

    apply_duty();
}

static void previous_duty(void) {
    if (duty_index == 0) {
        duty_index = DUTY_STEP_COUNT - 1;
    } else {
        duty_index--;
    }

    apply_duty();
}

static void handle_command(char c) {
    if (debug_enabled) {
        print_rx_debug(c);
    }

    // Ignore Enter/newline characters.
    if (c == '\r' || c == '\n') {
        return;
    }

    if (c >= '0' && c <= '4') {
        duty_index = c - '0';
        apply_duty();
        print_status();
        return;
    }

    switch (c) {
        case 'o':
            enter_pwm_output_mode();
            break;

        case 'i':
            enter_adc_input_mode();
            break;

        case 'r':
            read_adc_once();
            break;

        case 'n':
            next_duty();
            print_status();
            break;

        case 'p':
            previous_duty();
            print_status();
            break;

        case 's':
            print_status();
            break;

        case 'd':
            debug_enabled = !debug_enabled;
            print_status();
            break;

        case '?':
            print_help();
            print_status();
            break;

        default:
            uart0_print("unknown command\r\n");
            break;
    }
}

int main(void) {
    uart0_init();
    adc0_init_internal_4v3();
    pwm0_init_pa3();

    duty_index = 0;
    pa3_mode = PA3_MODE_PWM_OUTPUT;
    apply_duty();

    uart0_print("\r\nboot: PA3 mux manual test\r\n");
    print_help();
    print_status();

    while (1) {
        char c;

        if (uart0_try_getc(&c)) {
            handle_command(c);
        }
    }
}