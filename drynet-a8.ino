#include "AnetFunctions.h"

void setup() {
  // Start Serial communication at 115200 baud rate
  Serial.begin(115200);
  Serial1.begin(115200);

  // Wait for the serial connection to initialize
  while (Serial == false) {};

  // Set pin modes for various outputs
  pinMode(HOTBED, OUTPUT);
  pinMode(FAN1, OUTPUT);
  pinMode(FAN2, OUTPUT);
  pinMode(XYE_ENABLE, OUTPUT);  // Stepper motor enable pin

  // Initialize stepper motor with direction and step pins
  stepper.begin(E_STEP, E_DIR);
  stepper.spin(50);  // Start spinning the stepper motor at 50 steps per second

  // Initialize I2C communication
  Wire.begin();

  // Initialize the temperature and humidity sensor
  errorDecoder(mySHTC3.begin());
  preset = "manual";
}

void loop() {

  parseSerial();
  // Get the current time in milliseconds
  unsigned long currentMillis = millis();

  // Toggle FAN1 status every 30 seconds
  if ((currentMillis - lastMillis30000) > 30000) {

    if (isFan1On || preset == "OFF") {
      isFan1On = 0;
      setFan1Status(0);  // Turn off FAN1
    } else {
      isFan1On = 1;
      setFan1Status(255);  // Turn on FAN1 at full speed
    }
    setMillis(0);
    lastMillis30000 = millis();
  }


  // Execute every 1 second
  if ((currentMillis - lastMillis1000) > 1000) {
    if (isFan2On) {
      setFan2Status(255);  // Turn off FAN1
    } else {
      setFan2Status(0);  // Turn on FAN1 at full speed
    }

    // Set the hotbed temperature based on the received JSON data
    setBedTemp(targetBedTemp, targetAirTemp);
    if (debug) {
      Serial.println(getAmbTemp());
      Serial.println(getBedTemp());
      Serial.println(millis());
      Serial.println(getAbsoluteHumidity());
      Serial.println(preset);
      Serial.println(dryTimer);
    }
    // Update last execution time
    lastMillis1000 = millis();
    setPreset();
    if (dryTimer > 1) {
      dryTimer--;
    } else {
      preset = "OFF";
    }
  }

  // Execute every 100 milliseconds
  if ((currentMillis - lastMillis100) > 100) {
    // Code here
    lastMillis100 = currentMillis;
  }

  // Execute every 10 milliseconds
  if ((currentMillis - lastMillis10) > 10) {
    // Control the stepper motor based on isStepperOn flag
    if (isStepperOn) {
      digitalWrite(XYE_ENABLE, LOW);  // Enable stepper motor
      if (!stepper.isSpinning()) {
        stepper.spin(50);  // Keep spinning at 50 steps per second
      }
    } else {
      digitalWrite(XYE_ENABLE, HIGH);  // Disable stepper motor
      stepper.stop();                  // Stop motor movement
    }
    lastMillis10 = millis();
  }



  // Update lastMillis to the current time
  lastMillis = millis();
}
