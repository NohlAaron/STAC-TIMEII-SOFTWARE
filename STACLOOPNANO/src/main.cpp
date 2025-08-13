#include <Arduino.h>
#include <Servo.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM6DS33.h>

/* This is currently TIME II's Main Control loop code. Right now, we currently have most of a phase 1 implementation complete.
We should begin to work on Implementations of other phases and transistioning between each phase. */


int resistorPin = A1;
int raw = 0;
float Vin = 5;  // for analog voltage pins
float Vout = 0;
float R1 = 3300; // Known resistor value in ohms
float R2 = 0;
float buffer = 0;

#define HEATPAD_PIN 5
uint8_t motorPins[] = {2,3};

File dataFile;
const uint8_t chipSelect = 4;

Servo servo1, servo2;

// ---------------- IMU additions ----------------
Adafruit_LSM6DS33 imu;
const float G_SI = 9.80665f;
const int   MA_WIN = 4;
float azBuf[MA_WIN] = {0};
int   azIdx = 0;
bool  azFilled = false;
float lastAz_g = 0.0f;
float lastAzMa_g = 0.0f;
const float LIFTOFF_G   = 1.15f; //use 1.15f for actual launch, use 1.05f for testing
const float ZERO_G_MAX  = 0.20f;
const float DESCENT_G   = 1.20f;
const int   SUSTAIN_N   = 3; //use 3 for actual launch, use 2 for testing
bool liftoffDetected   = false;
bool zeroGActive       = false;
bool descentDetected   = false;
unsigned long liftoffMs = 0;
int cntLiftoff = 0, cntZeroG = 0, cntDescent = 0;
bool gImuOk = false;
// ------------------------------------------------

void setup() {
  servo1.attach(motorPins[0]);
  servo2.attach(motorPins[1]);
  pinMode(HEATPAD_PIN, OUTPUT);
  digitalWrite(HEATPAD_PIN, LOW);

  Serial.begin(115200);
  delay(1000);  
  Serial.println(F("Started Timer."));

  pinMode(10, OUTPUT);         // keep SPI in master
  pinMode(4, OUTPUT);          // your CS pin
  digitalWrite(4, HIGH);       // deselect card before init
  delay(50);
  if (!SD.begin(chipSelect)) {
    Serial.println(F("SD card init failed!"));
    return;
  }
  Serial.println(F("SD card initialized."));
  delay(100);

  dataFile = SD.open("DATAFILE.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(F("File Opened! TIME II temperature + IMU log"));
    dataFile.println(F("Waiting 8s to init..."));
    Serial.println(F("File success."));
    Serial.println(F("waiting 8s to init...."));
    delay(8000);
    dataFile.println(F("ms,flight_s,therm_R2_ohm,imu_az_g,imu_az_ma_g,state_note"));
    dataFile.close();
  } else {
    Serial.println(F("SD card error"));
  }

  // ---- IMU init ----
  Wire.begin();
  gImuOk = imu.begin_I2C() || imu.begin_I2C(0x6A) || imu.begin_I2C(0x6B);
  if (!gImuOk) {
    Serial.println(F("ERROR: LSM6DS33 not found at 0x6A/0x6B; continuing without IMU."));
  } else {
    Serial.println(F("LSM6DS33 initialized."));
  }
}

float readResistor(int pin){
  delay(10); // settle voltage
  int sum = 0;
  analogRead(pin); // throwaway
  for (int i = 0; i < 20; i++) {
    sum += analogRead(pin);
    delay(2);  // allow minor settle
  }
  raw = sum / 20;

  if (raw < 3 || raw > 1020) {
    Serial.print(F("ADC extreme: "));
    Serial.println(raw);
    Serial.println(F("ADC out of range. Skipping..."));
    return 0;
  }

  float Vout = (raw / 1023.0) * Vin;
  float buffer = Vout / (Vin - Vout);
  float R2 = R1 * buffer;

  Serial.print(F("Vout pin"));
  Serial.print(pin);
  Serial.print(": ");
  Serial.println(Vout, 3);

  Serial.print(F("thermistor prediction"));
  Serial.print(pin);
  Serial.print(F(": "));
  Serial.println(R2, 2);

  float expectedVout = Vin * R2 / (R1 + R2);
  Serial.print(F("Expected Vout for R2: "));
  Serial.println(expectedVout, 3);

  delay(500);
  return R2;
}

// === CHANGED: add allowHeat + nowMs gate ===
void handleHeatPads(int reading, int pin, bool allowHeat, unsigned long nowMs){
  if (!allowHeat) {
    Serial.println(F("Heat window CLOSED by IMU. skipping heatpad."));
    return;
  }
  if (reading > 5000) {
    Serial.print(F("resistor: ")); Serial.print(pin); Serial.println(F(" >5000 ohms. Heatpad ON (30s)..."));
    digitalWrite(HEATPAD_PIN, HIGH);
    delay(30000);
    digitalWrite(HEATPAD_PIN, LOW);
    Serial.println(F("Heatpad OFF."));

    File f = SD.open("DATAFILE.txt", FILE_WRITE);
    if (f) {
      float flight_s = liftoffDetected ? (nowMs - liftoffMs)/1000.0f : 0.0f;
      f.print(nowMs); f.print(F(",")); f.print(flight_s,1); f.print(F(","));
      f.print(reading); f.print(F(","));
      f.print(lastAz_g,3); f.print(F(",")); f.print(lastAzMa_g,3); f.print(F(","));
      f.println(F("heatpad_pulse_30s"));
      f.close();
    }
  } else if (reading == 0){
    Serial.print(F("resistor")); Serial.print(pin); Serial.println(F(" reading fail."));
  } else {
    Serial.print(F("resistor ")); Serial.print(pin); Serial.println(F(" <= 5000 ohms (no heat)."));
  }
}

// === CHANGED: add imu_g (2nd arg) and log flight time & MA ===
void handleSDCard(int reading, float imu_g){
  unsigned long nowMs = millis();
  float flight_s = liftoffDetected ? (nowMs - liftoffMs)/1000.0f : 0.0f;

  dataFile = SD.open("DATAFILE.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.print(nowMs); dataFile.print(F(","));
    dataFile.print(flight_s,1); dataFile.print(F(","));
    dataFile.print(reading); dataFile.print(F(","));
    dataFile.print(imu_g,3); dataFile.print(F(","));
    dataFile.print(lastAzMa_g,3); dataFile.print(F(","));
    dataFile.println("");
    dataFile.flush();
    if(reading > 5000){
      dataFile.print(nowMs); dataFile.print(F(",")); dataFile.print(flight_s,1);
      dataFile.println(F(",,,," "thermistor_condition_met"));
    }
    dataFile.close();
  } else {
    Serial.println(F("File pointer is invalid."));
  }
}

void handleMotors(int motor){
  //TODO: Fix motor timings
  if (motor == 0){
    for (int pos = 0; pos <= 60; pos += 1) {
      servo1.write(pos);
      delay(15); // Small delay for smooth motion
    }
  } else {
    for (int pos = 0; pos <= 60; pos += 1) {
      servo2.write(pos);
      delay(15);
    }
  }

}

// === NEW: IMU updates + state machine ===
void handleIMU(){
  sensors_event_t a, g, t;
  imu.getEvent(&a, &g, &t);

  lastAz_g = fabs(a.acceleration.z) / G_SI;
  azBuf[azIdx] = lastAz_g;
  azIdx = (azIdx + 1) % MA_WIN;
  if (azIdx == 0) azFilled = true;

  int n = azFilled ? MA_WIN : azIdx;
  float sum = 0; for (int i = 0; i < n; i++) sum += azBuf[i];
  lastAzMa_g = (n > 0) ? (sum / n) : 0;

  if (!liftoffDetected) {
    if (lastAzMa_g >= LIFTOFF_G) { cntLiftoff++; } else { cntLiftoff = 0; }
    if (cntLiftoff >= SUSTAIN_N) {
      liftoffDetected = true;
      liftoffMs = millis();
      Serial.println(F("IMU: Liftoff detected."));
      File f = SD.open("DATAFILE.txt", FILE_WRITE);
      if (f) {
        f.print(liftoffMs); f.print(F(",0, ,"));
        f.print(lastAz_g,3); f.print(F(",")); f.print(lastAzMa_g,3); f.print(F(","));
        f.println(F("liftoff_detected"));
        f.close();
      }
    }
  } else if (!zeroGActive) {
    if (lastAzMa_g <= ZERO_G_MAX) { cntZeroG++; } else { cntZeroG = 0; }
    if (cntZeroG >= SUSTAIN_N) {
      zeroGActive = true;
      Serial.println(F("IMU: Zero-g phase detected."));
      File f = SD.open("DATAFILE.txt", FILE_WRITE);
      if (f) {
        unsigned long nowMs = millis();
        float flight_s = (nowMs - liftoffMs)/1000.0f;
        f.print(nowMs); f.print(F(",")); f.print(flight_s,1); f.print(F(","));
        f.print(lastAz_g,3); f.print(F(",")); f.print(lastAzMa_g,3); f.print(F(","));
        f.println(F("zero_g_start"));
        f.close();
      }
    }
  } else if (!descentDetected) {
    if (lastAzMa_g >= DESCENT_G) { cntDescent++; } else { cntDescent = 0; }
    if (cntDescent >= SUSTAIN_N) {
      descentDetected = true;
      Serial.println(F("IMU: Descent detected. Heat window closes."));
      File f = SD.open("DATAFILE.txt", FILE_WRITE);
      if (f) {
        unsigned long nowMs = millis();
        float flight_s = (nowMs - liftoffMs)/1000.0f;
        f.print(nowMs); f.print(F(",")); f.print(flight_s,1); f.print(F(", ,"));
        f.print(lastAz_g,3); f.print(F(",")); f.print(lastAzMa_g,3); f.print(F(","));
        f.println(F("descent_start"));
        f.close();
      }
    }
  }
}

void loop() {
  // Always update IMU readings first to get current flight status
  if (gImuOk) {
    handleIMU();
  } else {
    // keep sane defaults if no IMU
    lastAz_g = 0.0f;
    lastAzMa_g = 0.0f;
  }
  unsigned long nowMs = millis();
  
  // Phase determination based on IMU status
  int currentPhase = 1; // Default: Phase 1
  if (liftoffDetected && zeroGActive && !descentDetected) {
    currentPhase = 2; // Microgravity period - injection phase
  } else if (descentDetected) {
    currentPhase = 3; // Descent phase
  }

  // Handle phase transitions
  static int lastPhase = 0;
  if (currentPhase != lastPhase) {
    dataFile = SD.open("DATAFILE.txt", FILE_WRITE);
    if (dataFile) {
      switch(currentPhase) {
        case 1:
          dataFile.println(F("## PHASE 1 (Preload + Ascension) ##"));
          Serial.println(F("File opened. (phase 1)"));
          break;
        case 2:
          dataFile.println(F("Phase 1 over... starting Phase 2"));
          dataFile.println(F("## PHASE 2 (Injections) ##"));
          Serial.println(F("File opened. (phase 2)"));
          break;
        case 3:
          dataFile.println(F("Phase 2 over... starting Phase 3"));
          dataFile.println(F("## PHASE 3 (Descension)##"));
          Serial.println(F("File opened. (phase 3)"));
          break;
      }
      dataFile.close();
    } else {
      Serial.println(F("File error for phase transition."));
    }
    lastPhase = currentPhase;
  }

  // Execute phase-specific logic
  switch(currentPhase) {
    case 1: // Phase 1: Preload + Ascension
      {
        int reading = readResistor(resistorPin);
        // Only allow heat before descent is detected
        bool allowHeat = liftoffDetected && !descentDetected;
        handleSDCard(reading, lastAz_g);
        handleHeatPads(reading, resistorPin, allowHeat, nowMs);
      }
      break;
      
    case 2: // Phase 2: Injections - zero-g environment
      {
        int reading = readResistor(resistorPin);
        handleSDCard(reading, lastAz_g);
        
        // Log injection event
        static bool injectionDone = false;
        if (!injectionDone) {
          // Perform motor control for injections
          Serial.println(F("Performing injection in microgravity..."));
          handleMotors(0); // First motor
          delay(500);      // Short delay between motors
          handleMotors(1); // Second motor
          
          // Log the injection event
          dataFile = SD.open("DATAFILE.txt", FILE_WRITE);
          if (dataFile) {
            float flight_s = (nowMs - liftoffMs)/1000.0f;
            dataFile.print(nowMs); dataFile.print(F(","));
            dataFile.print(flight_s,1); dataFile.print(F(","));
            dataFile.print(reading); dataFile.print(F(","));
            dataFile.print(lastAz_g,3); dataFile.print(F(","));
            dataFile.print(lastAzMa_g,3); dataFile.print(F(","));
            dataFile.println(F("injection_performed"));
            dataFile.close();
          }
          injectionDone = true;
        }
      }
      break;
      
    case 3: // Phase 3: Descent
      {
        int reading = readResistor(resistorPin);
        // Heat is disabled during descent
        handleSDCard(reading, lastAz_g);
        handleHeatPads(reading, resistorPin, false, nowMs);
      }
      break;
  }

  // Always add a small delay to avoid overwhelming the system
  delay(100);
}