#define F_CPU 3333333UL

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

#include "uart0.h"
#include "pwm0.h"

static const uint8_t duty_steps[] = {
    0,    // 0%
    64,   // 25%
    128,  // 50%
    192,  // 75%
    255   // 100%
};

#define DUTY_STEP_COUNT (sizeof(duty_steps) / sizeof(duty_steps[0]))

static uint8_t duty_index = 0;
static bool pwm_enabled = true;

// SET DEBUG HERE
static bool debug_enabled = false;

static void print_help(void) {
    uart0_print("\r\n");
    uart0_print("UART PWM control test\r\n");
    uart0_print("commands:\r\n");
    uart0_print("  0 = 0% duty\r\n");
    uart0_print("  1 = 25% duty\r\n");
    uart0_print("  2 = 50% duty\r\n");
    uart0_print("  3 = 75% duty\r\n");
    uart0_print("  4 = 100% duty\r\n");
    uart0_print("  n = next duty\r\n");
    uart0_print("  p = previous duty\r\n");
    uart0_print("  x = disable PWM output\r\n");
    uart0_print("  e = enable PWM output\r\n");
    uart0_print("  s = status\r\n");
    uart0_print("  d = toggle RX debug\r\n");
    uart0_print("  ? = help\r\n");
    uart0_print("\r\n");
}

static void apply_duty(void) {
    pwm0_set_duty_8bit(duty_steps[duty_index]);
}

static void print_status(void) {
    uart0_print("duty_index=");
    uart0_print_u16(duty_index);

    uart0_print(" duty=");
    uart0_print_u16(duty_steps[duty_index]);

    uart0_print(" pwm=");
    uart0_print(pwm_enabled ? "enabled" : "disabled");

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
        case 'n':
            next_duty();
            print_status();
            break;

        case 'p':
            previous_duty();
            print_status();
            break;

        case 'x':
            pwm0_disable_pa3_output();
            pwm_enabled = false;
            print_status();
            break;

        case 'e':
            pwm0_enable_pa3_output();
            pwm_enabled = true;
            apply_duty();
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
    pwm0_init_pa3();

    duty_index = 0;
    apply_duty();

    uart0_print("\r\nboot: uart pwm control\r\n");
    print_help();
    print_status();

    while (1) {
        char c;

        if (uart0_try_getc(&c)) {
            handle_command(c);
        }
    }
}