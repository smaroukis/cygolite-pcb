
// Integrated read battery voltage as self contained function, requires battery.c
#define F_CPU 3333333UL

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

#include "uart0.h"
#include "pwm0.h"
#include "battery.h"

static const uint8_t duty_steps[] = {
    0,
    64,
    128,
    192,
    255
};

#define DUTY_STEP_COUNT (sizeof(duty_steps) / sizeof(duty_steps[0]))

static uint8_t duty_index = 0;

static void print_help(void) {
    uart0_print("\r\n");
    uart0_print("battery mux test\r\n");
    uart0_print("commands:\r\n");
    uart0_print("  0 = 0% duty\r\n");
    uart0_print("  1 = 25% duty\r\n");
    uart0_print("  2 = 50% duty\r\n");
    uart0_print("  3 = 75% duty\r\n");
    uart0_print("  4 = 100% duty\r\n");
    uart0_print("  b = muxed battery/ADC read\r\n");
    uart0_print("  s = status\r\n");
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

    uart0_print("\r\n");
}

// TODO - this actualy performs two ADC reads (read_muxed_raw and read_muxed_battery) - add a pub fn to convert raw without another read
static void read_battery(void) {
    uint16_t raw = battery_read_muxed_pa3_raw();
    uint16_t pin_mv = adc0_raw_to_mv(raw);
    uint16_t battery_mv = battery_read_muxed_pa3_battery_mv();

    uart0_print("battery mux read: raw=");
    uart0_print_u16(raw);

    uart0_print(" pin_mv=");
    uart0_print_u16(pin_mv);

    uart0_print(" battery_mv=");
    uart0_print_u16(battery_mv);

    uart0_print(" restored_duty=");
    uart0_print_u16(pwm0_get_duty_8bit());

    uart0_print("\r\n");
}

static void handle_command(char c) {
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
        case 'b':
            read_battery();
            break;

        case 's':
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
    battery_init();

    duty_index = 0;
    apply_duty();

    uart0_print("\r\nboot: battery mux test\r\n");
    print_help();
    print_status();

    while (1) {
        char c;

        if (uart0_try_getc(&c)) {
            handle_command(c);
        }
    }
}