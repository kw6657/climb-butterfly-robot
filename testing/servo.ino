#include <Arduino.h>

// Define pins
const uint8_t SERVO_1_PIN = 38;
const uint8_t SERVO_2_PIN = 39;

// PWM configuration
const uint32_t PWM_FREQ = 333;
const uint8_t PWM_RESOLUTION = 14;
const uint8_t PWM_CHANNEL = 0;

// Movement speed configuration
const int PULSE_STEP = 5;       // Microseconds
const int STEP_DELAY = 10;      // Milliseconds

int currentPulse = 1500;

// Generate PWM signal for both servos
void setServoPulse(int pulse_us) {

  uint32_t duty = (uint32_t)(
    (uint64_t)pulse_us * PWM_FREQ *
    (1UL << PWM_RESOLUTION) / 1000000
  );

  ledcWriteChannel(PWM_CHANNEL, duty);
}

// Gradually move both servos
void moveServos(int targetPulse) {

  while (currentPulse != targetPulse) {

    if (currentPulse < targetPulse) {
      currentPulse = min(
        currentPulse + PULSE_STEP, targetPulse
      );
    }
    else {
      currentPulse = max(
        currentPulse - PULSE_STEP, targetPulse
      );
    }

    setServoPulse(currentPulse);
    delay(STEP_DELAY);
  }

  Serial.print("Target pulse reached: ");
  Serial.println(currentPulse);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  bool servo1 = ledcAttachChannel(
    SERVO_1_PIN, PWM_FREQ,
    PWM_RESOLUTION, PWM_CHANNEL
  );

  bool servo2 = ledcAttachChannel(
    SERVO_2_PIN, PWM_FREQ,
    PWM_RESOLUTION, PWM_CHANNEL
  );

  if (!servo1 || !servo2) {
    Serial.println("PWM initialization failed!");

    while (true) {
      delay(1000);
    }
  }

  Serial.println("Both servos initialized!");

  // Centre both servos
  setServoPulse(currentPulse);
  delay(2000);
}

void loop() {

  moveServos(1200);
  delay(1000);

  moveServos(1500);
  delay(1000);

  moveServos(1800);
  delay(1000);

  moveServos(1500);
  delay(1000);
}
