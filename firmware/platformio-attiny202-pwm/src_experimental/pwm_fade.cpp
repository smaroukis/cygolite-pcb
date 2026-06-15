#include <Arduino.h>

const uint8_t PWM_PIN = PIN_PA7;

int duty = 0;
int stepSize = 5;

void setup() {
  pinMode(PWM_PIN, OUTPUT);
}

void loop() {
  analogWrite(PWM_PIN, duty);

  duty += stepSize;

  // Bounce at the ends: 0 -> 255 -> 0 -> 255 ...
  if (duty >= 255) {
    duty = 255;
    stepSize = -stepSize;
  }

  if (duty <= 0) {
    duty = 0;
    stepSize = -stepSize;
  }

  delay(20);
}