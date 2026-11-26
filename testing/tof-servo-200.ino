#include <Arduino.h>
#include <Wire.h>
#include <VL53L1X.h>

// ================================
// Define pins
// ================================
const uint8_t SCL_PIN = 15;
const uint8_t SDA_PIN = 16;

const uint8_t SERVO1_PIN = 1;
const uint8_t SERVO2_PIN = 2;

// ================================
// ToF configuration
// ================================
VL53L1X tof;

const uint16_t DISTANCE_THRESHOLD = 200; // mm

// ================================
// PWM configuration
// ================================
const uint8_t PWM_CHANNEL = 0;
const uint32_t PWM_FREQ = 50;           // Hz
const uint8_t PWM_RESOLUTION = 14;
const uint32_t PWM_PERIOD = 20000;      // us

// Servo movement limits
const uint16_t MIN_PULSE = 1200;        // us
const uint16_t MAX_PULSE = 1800;        // us

const uint32_t SERVO_INTERVAL = 500;    // ms

// ================================
// Servo state variables
// ================================
bool servosActive = false;
bool servoPosition = false;

unsigned long lastServoUpdate = 0;

// ================================
// Send identical PWM to both servos
// ================================
void setServoPulse(uint16_t pulseWidth) {

  uint32_t duty = (uint32_t)pulseWidth *
                  (1UL << PWM_RESOLUTION) /
                  PWM_PERIOD;

  ledcWrite(SERVO1_PIN, duty);
  // Both pins share the same PWM channel
}

// ================================
// Disable both servo signals
// ================================
void stopServos() {

  ledcWrite(SERVO1_PIN, 0);

  servosActive = false;
}

// ================================
// Update servo movement
// ================================
void updateServos() {

  unsigned long currentTime = millis();

  // Activate servos for the first time
  if (!servosActive) {

    servosActive = true;
    servoPosition = false;

    setServoPulse(MIN_PULSE);

    lastServoUpdate = currentTime;
  }

  // Alternate between two positions
  if (currentTime - lastServoUpdate >= SERVO_INTERVAL) {

    servoPosition = !servoPosition;

    if (servoPosition) {
      setServoPulse(MAX_PULSE);
    }
    else {
      setServoPulse(MIN_PULSE);
    }

    lastServoUpdate = currentTime;
  }
}

// ================================
// Setup
// ================================
void setup() {

  Serial.begin(115200);

  // Initialize PWM output pins
  pinMode(SERVO1_PIN, OUTPUT);
  pinMode(SERVO2_PIN, OUTPUT);

  digitalWrite(SERVO1_PIN, LOW);
  digitalWrite(SERVO2_PIN, LOW);

  // Assign both servos to the same PWM channel
  bool pwm1 = ledcAttachChannel(
    SERVO1_PIN, PWM_FREQ, PWM_RESOLUTION, PWM_CHANNEL
  );

  bool pwm2 = ledcAttachChannel(
    SERVO2_PIN, PWM_FREQ, PWM_RESOLUTION, PWM_CHANNEL
  );

  if (!pwm1 || !pwm2) {

    if (pwm1) {
      ledcWrite(SERVO1_PIN, 0);
    }

    if (pwm2) {
      ledcWrite(SERVO2_PIN, 0);
    }

    Serial.println("PWM initialization failed!");

    while (true) {
      delay(1000);
    }
  }

  // Keep servos inactive initially
  stopServos();

  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  // Initialize ToF sensor
  tof.setTimeout(150);

  if (!tof.init()) {

    Serial.println("ToF initialization failed!");

    while (true) {
      delay(1000);
    }
  }

  // Configure ToF sensor
  tof.setDistanceMode(VL53L1X::Long);
  tof.setMeasurementTimingBudget(50000);

  // Measure distance every 50 ms
  tof.startContinuous(50);

  Serial.println("ToF initialized successfully.");
  Serial.println("PWM initialized successfully.");
  Serial.println("System ready.");
}

// ================================
// Main loop
// ================================
void loop() {

  // Read distance in mm
  uint16_t distance = tof.read();

  // Check measurement validity
  if (tof.timeoutOccurred() ||
      tof.ranging_data.range_status != VL53L1X::RangeValid) {

    stopServos();

    Serial.println("Invalid ToF reading | Servos: OFF");

    return;
  }

  // Activate or disable servos
  if (distance > DISTANCE_THRESHOLD) {

    updateServos();

  }
  else {

    stopServos();
  }

  // Display information
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" mm");

  Serial.print(" | Servos: ");
  Serial.print(servosActive ? "ON" : "OFF");

  Serial.print(" | PWM: ");

  if (servosActive) {

    Serial.print(
      servoPosition ? MAX_PULSE : MIN_PULSE
    );

    Serial.println(" us");

  }
  else {

    Serial.println("Disabled");
  }
}