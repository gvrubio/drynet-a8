#include "AnetConfig.h"                            // Configuration file for Anet
#include <thermistor.h>                            // Library for thermistor temperature reading
#include <ContinuousStepper.h>                     // Library for controlling stepper motors
#include <ContinuousStepper/Tickers/TimerOne.hpp>  // Timer for stepper motor control
#include "SparkFun_SHTC3.h"                        // Library for SHTC3 temperature and humidity sensor
#include "math.h"                                  // Math library for calculations
#include <ArduinoJson.h>                           // Library for handling JSON data

// ======================
// Object Declarations
// ======================

//// Stepper motor object
ContinuousStepper<StepperDriver, TimerOneTicker> stepper;

//// Thermistors for temperature measurement
thermistor therm1(TBED, 0);  // Bed thermistor
thermistor therm2(TEND, 0);  // Ambient thermistor

//// SHTC3 sensor object for temperature and humidity
SHTC3 mySHTC3;

//// JSON document for serial communication
JsonDocument json;
JsonDocument outJson;

// ======================
// Variable Declarations
// ======================

String serialIn;  // Buffer for incoming serial data

// Flags for system status
bool isStepperOn = 0;  // Stepper motor state
bool isFan1On = 0;     // Fan 1 state
bool isFan2On = 0;     // Fan 2 state
byte debug = 1;
unsigned int dryTimer = 0;

// Temperature control variables
float currentAirTemp = 0;  // Current ambient temperature
float currentBedTemp = 0;  // Current bed temperature
float targetAirTemp = 0;   // Target ambient temperature
float targetBedTemp = 0;   // Target bed temperature
float currentAbsHumidity;  // Current Absolute Humidity

// Presets
// Manual: no change
// OFF: 0c 0h 0b
// PLA: 50c 8H 100b
// PETG: 60C 8h 120B
// TPU: 65C 8H 120B
String preset = "manual";  // Default preset
String status = "STOP";    // RUN, RUNNING, STOP
// Time control variables
extern volatile unsigned long timer0_millis;
unsigned long lastMillis30000 = 0;  // Last recorded time for 30s interval
unsigned long lastMillis1000 = 0;   // Last recorded time for 1s interval
unsigned long lastMillis100 = 0;    // Last recorded time for 100ms interval
unsigned long lastMillis10 = 0;     // Last recorded time for 10ms interval
unsigned long lastMillis = 0;       // Last recorded time for runtime tracking

// ==========================
// Utility Functions
// ==========================

//// CALCULATIONS

#define e 2.718281828459045235360287471352  // Euler's constant

// Function to calculate absolute humidity using temperature and relative humidity
// FROM https://gist.github.com/mcavalcantib/a58c1e18824f7f1bcf02526bb202b6cc
float calculateAbsoluteHumidity(float hum, float temp) {
  float UA = ((6.112 * (pow(e, ((17.67 * temp) / (temp + 243.5)))) * hum * 2.1674) / (273.15 + temp));
  return UA;
}

//// GETTERS

// Function to get the current bed temperature
float getBedTemp() {
  float temp = therm1.analog2temp();  // Read temperature from thermistor
  currentBedTemp = temp;              // Update global variable
  return temp;
}

// Function to get the current ambient temperature
float getAmbTemp() {
  float temp = therm2.analog2temp();  // Read temperature from thermistor
  currentAirTemp = temp;              // Update global variable
  return temp;
}

// Function to decode and display SHTC3 sensor errors in a human-readable format
void errorDecoder(SHTC3_Status_TypeDef message) {
  switch (message) {
    case SHTC3_Status_Nominal: Serial.print("Nominal"); break;
    case SHTC3_Status_Error: Serial.print("Error"); break;
    case SHTC3_Status_CRC_Fail: Serial.print("CRC Fail"); break;
    default: Serial.print("Unknown return code"); break;
  }
}

// Function to get absolute humidity from SHTC3 sensor
float getAbsoluteHumidity() {
  SHTC3_Status_TypeDef result = mySHTC3.update();    // Update sensor data
  if (mySHTC3.lastStatus == SHTC3_Status_Nominal) {  // Check if sensor data is valid
    float i2cRelHum = mySHTC3.toPercent();           // Get relative humidity
    float i2ctemp = mySHTC3.toDegC();                // Get temperature
    currentAbsHumidity = calculateAbsoluteHumidity(i2cRelHum, i2ctemp);
    return currentAbsHumidity;
  } else {
    Serial.print("Update failed, error: ");
    errorDecoder(mySHTC3.lastStatus);
    Serial.println();
    return -1;  // Return error code
  }
}

// Use the endstops as an ugly but functional preset selector
void hardButtonManagement() {
  // PLA PRESET
  if (!digitalRead(X_STOP)) {
    while (!digitalRead(X_STOP)) {
      delay(10);
    }
    preset = "PLA";
    status = "RUN";
  }
  // PETG PRESET
  if (!digitalRead(Y_STOP)) {
    while (!digitalRead(Y_STOP)) {
      delay(10);
    }
    preset = "PET";
    status = "RUN";
  }
  // OFF
  if (!digitalRead(Z_STOP)) {
    while (!digitalRead(Z_STOP)) {
      delay(10);
    }
    preset = "OFF";
    status = "RUN";
  }
}

//// SETTERS
void setPreset() {
  if (preset == "OFF" && (status == "RUN" || status == "RUNNING")) {
    targetAirTemp = 0;  // Target ambient temperature
    targetBedTemp = 0;  // Target bed temperature
    isStepperOn = 0;    // Stepper motor state
    isFan2On = 0;       // Fan 2 state
    dryTimer = 0;       // 0 hours
    status = "STOP";
  } else if (preset == "PLA" && status == "RUN") {
    targetAirTemp = 50;   // Target ambient temperature
    targetBedTemp = 100;  // Target bed temperature
    isStepperOn = 1;      // Stepper motor state
    isFan2On = 1;         // Fan 2 state
    dryTimer = 28800;     //8 hours
    status = "RUNNING";
  } else if (preset == "PET" && status == "RUN") {
    targetAirTemp = 65;   // Target ambient temperature
    targetBedTemp = 110;  // Target bed temperature
    isStepperOn = 1;      // Stepper motor state
    isFan2On = 1;         // Fan 2 state
    dryTimer = 28800;
    status = "RUNNING";  //8 hours
  } else if (preset == "TPU" && status == "RUN") {
    targetAirTemp = 55;   // Target ambient temperature
    targetBedTemp = 110;  // Target bed temperature
    isStepperOn = 1;      // Stepper motor state
    isFan2On = 1;         // Fan 2 state
    dryTimer = 28800;     //8 hours
    status = "RUNNING";
  }
}

// Function to turn Fan 1 on or off
void setFan1Status(bool status) {
  if (status) {
    digitalWrite(FAN1, HIGH);  // Turn on Fan 1
  } else {
    digitalWrite(FAN1, LOW);  // Turn off Fan 1
  }
}

// Function to set Fan 2 speed
void setFan2Status(int status) {
  analogWrite(HOTEND, status);  // Adjust fan speed using PWM
}

// Function to turn the bed heater on or off
void setBedStatus(bool status) {
  if (status == 1) {
    digitalWrite(HOTBED, HIGH);  // Turn on bed heater
  } else {
    digitalWrite(HOTBED, LOW);  // Turn off bed heater
  }
}

// Function to regulate bed temperature
void setBedTemp(float bedTemp, float airTemp) {
  if (getBedTemp() < bedTemp - 0.1 && getAmbTemp() < airTemp) {
    setBedStatus(1);  // Turn on heater if temperature is too low
  } else if (getAmbTemp() > airTemp || getBedTemp() > bedTemp + 0.1) {
    setBedStatus(0);  // Turn off heater if temperature is too high
  }
}

// Function to regulate fan speed based on ambient temperature
void setFanTemp(int temp) {
  if (getAmbTemp() > temp) {
    setFan1Status(1);  // Turn on Fan 1 if temperature is too high
  } else if (getAmbTemp() < temp - 2) {
    setFan1Status(0);  // Turn off Fan 1 if temperature is within range
  }
}

//// SERIAL COMMUNICATION

// Function to parse incoming serial data in JSON format
void parseSerial() {
  if (Serial.available()) {
    deserializeJson(json, Serial);  // Deserialize JSON data from Serial
    if (json.containsKey("preset")) {
      preset = json["preset"].as<String>();
    }
    if (json.containsKey("dryTimer")) {
      dryTimer = json["dryTimer"];
    }
    if (json.containsKey("isStepperOn")) {
      isStepperOn = json["isStepperOn"];
    }
    if (json.containsKey("isFan1On")) {
      isFan1On = json["isFan1On"];
    }
    if (json.containsKey("isFan2On")) {
      isFan2On = json["isFan2On"];
    }
    if (json.containsKey("targetAirTemp")) {
      targetAirTemp = json["targetAirTemp"];
    }
    if (json.containsKey("targetBedTemp")) {
      targetBedTemp = json["targetBedTemp"];
    }
    if (json.containsKey("debug")) {
      debug = json["debug"];
    }
    if (json.containsKey("status")) {
      status = json["status"].as<String>();
    }
  }
}

// Function to read incoming serial data as a string
String readSerialString() {
  String input;
  while (Serial.available()) {
    input = Serial.readString();  // Read data from Serial as a string
  }
  return input;
}

void setMillis(unsigned long new_millis) {
  uint8_t oldSREG = SREG;
  cli();
  timer0_millis = new_millis;
  SREG = oldSREG;
}
