#include <Arduino.h>
#include <Servo.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM6DS33.h>

// Global vars with smaller types to save memory
uint8_t resistorPin = A1;
uint8_t raw = 0;
const uint8_t HEATPAD_PIN = 5;
uint8_t motorPins[] = {2,3};
const uint8_t chipSelect = 4;

// Thermistor calculation values
const float Vin = 5.0f;
const uint16_t R1 = 3300;
float R2 = 0;

File dataFile;
Servo servo1, servo2;

// Compressed IMU variables
Adafruit_LSM6DS33 imu;
const float G_SI = 9.80665f;
const uint8_t MA_WIN = 4;
float azBuf[MA_WIN] = {0};
uint8_t azIdx = 0;
bool azFilled = false;
float lastAz_g = 0.0f;
float lastAzMa_g = 0.0f;
const float LIFTOFF_G = 1.15f;
const float ZERO_G_MAX = 0.20f;
const float DESCENT_G = 1.20f;
const uint8_t SUSTAIN_N = 3;

// Flight state flags (combined in a single byte to save RAM)
union {
  struct {
    uint8_t liftoff : 1;    // Bit 0: liftoff detected
    uint8_t zeroG : 1;      // Bit 1: zero-g detected
    uint8_t descent : 1;    // Bit 2: descent detected
    uint8_t injected : 1;   // Bit 3: injection performed
    uint8_t phase : 2;      // Bits 4-5: current phase (1-3)
    uint8_t lastPhase : 2;  // Bits 6-7: last phase (1-3)
  } flags;
  uint8_t value;
} flightState = {0};

// Flight counters (8-bit to save space)
uint8_t cntLiftoff = 0, cntZeroG = 0, cntDescent = 0;
uint32_t liftoffMs = 0;

void setup() {
  // Simplified setup with reduced serial output
  servo1.attach(motorPins[0]);
  servo2.attach(motorPins[1]);
  pinMode(HEATPAD_PIN, OUTPUT);
  digitalWrite(HEATPAD_PIN, LOW);

  Serial.begin(9600);
  
  if (!SD.begin(chipSelect)) {
    Serial.print(F("SD fail"));
    return;
  }
  
  dataFile = SD.open("DATA.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(F("TIME II log"));
    delay(8000);
    dataFile.println(F("ms,s,R,g,ma,note"));
    dataFile.close();
  }

  // IMU init
  Wire.begin();
  if (!imu.begin_I2C(0x6B)) {
    Serial.print(F("IMU fail"));
    while (1) delay(10);
  }
  
  flightState.flags.phase = 1;      // Start in phase 1
  flightState.flags.lastPhase = 0;  // No previous phase
}

float readResistor(uint8_t pin) {
  // Simplified resistor reading with less memory usage
  delay(10);
  uint16_t sum = 0;
  analogRead(pin); // throwaway
  
  for (uint8_t i = 0; i < 10; i++) { // Reduced samples to save time
    sum += analogRead(pin);
    delay(1);
  }
  raw = sum / 10;

  if (raw < 3 || raw > 1020) return 0;

  float Vout = (raw / 1023.0f) * Vin;
  R2 = R1 * (Vout / (Vin - Vout));
  
  return R2;
}

void logEvent(uint32_t ms, float R2, const char *note) {
  // Unified logging function to reduce code duplication
  dataFile = SD.open("DATA.txt", FILE_WRITE);
  if (!dataFile) return;
  
  float flight_s = flightState.flags.liftoff ? (ms - liftoffMs)/1000.0f : 0;
  
  dataFile.print(ms);
  dataFile.print(',');
  dataFile.print(flight_s, 1);
  dataFile.print(',');
  dataFile.print(R2, 0);
  dataFile.print(',');
  dataFile.print(lastAz_g, 2);
  dataFile.print(',');
  dataFile.print(lastAzMa_g, 2);
  
  if (note) {
    dataFile.print(',');
    dataFile.println(note);
  } else {
    dataFile.println();
  }
  
  dataFile.close();
}

void handleHeatPads(float reading, uint32_t nowMs) {
  // Only allow heat in proper phases
  if (!flightState.flags.liftoff || flightState.flags.descent) return;
  
  if (reading > 5000) {
    digitalWrite(HEATPAD_PIN, HIGH);
    delay(30000);
    digitalWrite(HEATPAD_PIN, LOW);
    logEvent(nowMs, reading, "heat_30s");
  }
}

void handleMotors() {
  // Simplified motor control
  for (uint8_t motor = 0; motor < 2; motor++) {
    Servo &servo = (motor == 0) ? servo1 : servo2;
    for (uint8_t pos = 0; pos <= 60; pos += 2) { // Increased step to save cycles
      servo.write(pos);
      delay(10);
    }
    delay(250);
  }
}

void handleIMU() {
  sensors_event_t a, g, t;
  imu.getEvent(&a, &g, &t);

  lastAz_g = fabs(a.acceleration.z) / G_SI;
  azBuf[azIdx] = lastAz_g;
  azIdx = (azIdx + 1) % MA_WIN;
  if (azIdx == 0) azFilled = true;

  // Calculate moving average
  uint8_t n = azFilled ? MA_WIN : azIdx;
  float sum = 0;
  for (uint8_t i = 0; i < n; i++) sum += azBuf[i];
  lastAzMa_g = (n > 0) ? (sum / n) : 0;

  uint32_t nowMs = millis();
  
  // State machine for flight detection
  if (!flightState.flags.liftoff) {
    if (lastAzMa_g >= LIFTOFF_G) {
      cntLiftoff++;
      if (cntLiftoff >= SUSTAIN_N) {
        flightState.flags.liftoff = 1;
        liftoffMs = nowMs;
        logEvent(nowMs, 0, "liftoff");
      }
    } else {
      cntLiftoff = 0;
    }
  } 
  else if (!flightState.flags.zeroG) {
    if (lastAzMa_g <= ZERO_G_MAX) {
      cntZeroG++;
      if (cntZeroG >= SUSTAIN_N) {
        flightState.flags.zeroG = 1;
        logEvent(nowMs, 0, "zero_g");
      }
    } else {
      cntZeroG = 0;
    }
  } 
  else if (!flightState.flags.descent) {
    if (lastAzMa_g >= DESCENT_G) {
      cntDescent++;
      if (cntDescent >= SUSTAIN_N) {
        flightState.flags.descent = 1;
        logEvent(nowMs, 0, "descent");
      }
    } else {
      cntDescent = 0;
    }
  }
}

void loop() {
  // Update IMU first for flight status
  handleIMU();
  uint32_t nowMs = millis();
  
  // Update phase based on flight status
  if (flightState.flags.liftoff && flightState.flags.zeroG && !flightState.flags.descent) {
    flightState.flags.phase = 2; // Zero-G phase
  } else if (flightState.flags.descent) {
    flightState.flags.phase = 3; // Descent phase
  }

  // Log phase transitions
  if (flightState.flags.phase != flightState.flags.lastPhase) {
    dataFile = SD.open("DATA.txt", FILE_WRITE);
    if (dataFile) {
      dataFile.print(F("## PHASE "));
      dataFile.print(flightState.flags.phase);
      dataFile.println(F(" ##"));
      dataFile.close();
    }
    flightState.flags.lastPhase = flightState.flags.phase;
  }

  // Read thermistor - common for all phases
  float reading = readResistor(resistorPin);
  
  // Phase-specific behavior
  switch (flightState.flags.phase) {
    case 1: // Phase 1: Ascension
      logEvent(nowMs, reading, NULL);
      handleHeatPads(reading, nowMs);
      break;
      
    case 2: // Phase 2: Injections
      logEvent(nowMs, reading, NULL);
      
      // Only inject once in phase 2
      if (!flightState.flags.injected) {
        handleMotors();
        flightState.flags.injected = 1;
        logEvent(nowMs, reading, "inject");
      }
      break;
      
    case 3: // Phase 3: Descent
      logEvent(nowMs, reading, NULL);
      // No heating in descent phase
      break;
  }

  delay(100); // Brief delay to prevent thrashing
}