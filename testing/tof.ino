#include <Wire.h>
#include <VL53L1X.h>

// Define pins
const uint8_t SCL_PIN = 15;
const uint8_t SDA_PIN = 16;

// Create ToF sensor object
VL53L1X tof;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize I2C communication
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);  // 100 kHz

  // Initialize ToF sensor
  tof.setTimeout(500);

  if (!tof.init()) {
    Serial.println("Failed to initialize VL53L1X!");

    while (1) {
      delay(1000);
    }
  }

  Serial.println("VL53L1X initialized successfully!");

  // Configure distance measurement
  tof.setDistanceMode(VL53L1X::Long);
  tof.setMeasurementTimingBudget(50000);

  // Start continuous measurement every 50 ms
  tof.startContinuous(50);

  Serial.println("Starting distance measurements...");
}

void loop() {
  // Read distance in millimetres
  uint16_t distance = tof.read();

  // Check measurement validity
  if (tof.timeoutOccurred()) {
    Serial.println("Measurement timeout!");
    return;
  }

  // Print distance
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" mm");

  // Print measurement status
  Serial.print(" | Status: ");
  Serial.println(
    VL53L1X::rangeStatusToString(
      tof.ranging_data.range_status
    )
  );
}