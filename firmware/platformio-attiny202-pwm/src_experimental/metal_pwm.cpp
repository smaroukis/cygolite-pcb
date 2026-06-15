#include <Arduino.h>  // Bring in Arduino and ATtiny register definitions

void setup() {        // Runs once after reset

  PORTA.DIRSET = PIN3_bm;  // Make PA3 an output

  TCA0.SINGLE.CTRLA = 0;   // Stop TCA0 before configuring it

  TCA0.SINGLE.CTRLB =
      TCA_SINGLE_WGMODE_SINGLESLOPE_gc |  // Select single-slope PWM mode
      TCA_SINGLE_CMP0EN_bm;               // Enable compare channel 0 output, WO0

  TCA0.SINGLE.PER = 255;   // Set PWM period/top count

  TCA0.SINGLE.CMP0 = 128;  // Set duty cycle to about 50%

  TCA0.SINGLE.CTRLA =
      TCA_SINGLE_CLKSEL_DIV1_gc |  // Run timer at full speed
      TCA_SINGLE_ENABLE_bm;        // Start TCA0
}

void loop() {
  // Nothing needed; hardware PWM keeps running
}