#include <Arduino.h>
#include <Servo.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM6DS33.h>

/* TIME II – compact RAM/flash IMU gating + logging */

int resistorPin = A1;
int raw = 0;
const float Vin = 5;
const float R1  = 3300;

#define HEATPAD_PIN 5
const uint8_t motorPins[] = {2,3};

File dataFile;
const uint8_t chipSelect = 4;

Servo servo1, servo2;

// ---------------- IMU (memory-efficient) ----------------
Adafruit_LSM6DS33 imu;
bool gImuOk = false;

static inline int16_t to_mg(float a_ms2) {
  // 1000 / 9.80665 ≈ 101.9716
  return (int16_t)(a_ms2 * 101.9716f);
}

// small integer sqrt (no libm)
static inline uint32_t isqrt32(uint32_t x) {
  uint32_t op = x, res = 0, one = 1UL << 30;
  while (one > op) one >>= 2;
  while (one) {
    if (op >= res + one) { op -= res + one; res = (res >> 1) + one; }
    else                 { res >>= 1; }
    one >>= 2;
  }
  return res;
}

const uint8_t MA_WIN = 4;              // set to 3 or 4
int16_t axBuf[MA_WIN] = {0}, ayBuf[MA_WIN] = {0}, azBuf[MA_WIN] = {0};
int32_t sumAx = 0, sumAy = 0, sumAz = 0;
uint8_t maIdx = 0, maCount = 0;        // maCount <= MA_WIN

// thresholds (mg)
const int16_t LIFTOFF_MG  = 1150;      // 1.15 g (use 1050 for testing)
const int16_t ZERO_G_MAX  = 200;       // 0.20 g
const int16_t DESCENT_MG  = 1200;      // 1.20 g
const uint8_t SUSTAIN_N   = 3;         // 3 for flight, 2 for testing

// state
bool liftoffDetected=false, zeroGActive=false, descentDetected=false;
unsigned long liftoffMs = 0;
uint8_t cntLiftoff=0, cntZeroG=0, cntDescent=0;

// telemetry (keep interfaces)
float lastAz_g = 0.0f;                 // signed a_z (g) for logs
float lastAzMa_g = 0.0f;               // will store |â|_MA (g) for logs
//--------------------------------------------------------

void setup() {
  servo1.attach(motorPins[0]);
  servo2.attach(motorPins[1]);
  pinMode(HEATPAD_PIN, OUTPUT);
  digitalWrite(HEATPAD_PIN, LOW);

  Serial.begin(115200);
  delay(200);
  Serial.println(F("TIME II boot"));

  pinMode(10, OUTPUT);  // SPI master
  pinMode(4, OUTPUT);   // SD CS
  digitalWrite(4, HIGH);

  if (!SD.begin(chipSelect)) {
    Serial.println(F("SD init FAIL"));
  } else {
    Serial.println(F("SD OK"));
    dataFile = SD.open("DATAFILE.txt", FILE_WRITE);
    if (dataFile) {
      dataFile.println(F("File Opened! TIME II temperature + IMU log"));
      dataFile.println(F("Waiting 8s to init..."));
      delay(8000);
      dataFile.println(F("ms,flight_s,therm_R2_ohm,imu_az_g,imu_a_mag_ma_g,state_note"));
      dataFile.close();
    }
  }

  Wire.begin();
  Wire.setClock(400000);                  // fast I2C
  gImuOk = imu.begin_I2C(0x6B) || imu.begin_I2C(0x6A) || imu.begin_I2C();
  if (gImuOk) {
    imu.setAccelRange(LSM6DS_ACCEL_RANGE_8_G);   // avoid clipping at ~2.9 g
    imu.setAccelDataRate(LSM6DS_RATE_104_HZ);    // stable ODR
    Serial.println(F("IMU OK"));
  } else {
    Serial.println(F("IMU not found; continuing without IMU"));
  }
}

float readResistor(int pin){
  delay(10);
  int sum = 0;
  (void)analogRead(pin);
  for (uint8_t i = 0; i < 20; i++) { sum += analogRead(pin); delay(2); }
  raw = sum / 20;

  if (raw < 3 || raw > 1020) { Serial.println(F("ADC out of range")); return 0; }

  float Vout = (raw / 1023.0f) * Vin;
  float R2   = R1 * (Vout / (Vin - Vout));

  // Minimal prints; keep if you need
  // Serial.print(F("R2=")); Serial.println(R2, 2);

  delay(200);
  return R2;
}

void handleHeatPads(int reading, int pin, bool allowHeat, unsigned long nowMs){
  if (!allowHeat) return;

  if (reading > 5000) {
    digitalWrite(HEATPAD_PIN, HIGH);
    delay(30000);
    digitalWrite(HEATPAD_PIN, LOW);

    File f = SD.open("DATAFILE.txt", FILE_WRITE);
    if (f) {
      float flight_s = liftoffDetected ? (nowMs - liftoffMs)/1000.0f : 0.0f;
      f.print(nowMs); f.print(F(",")); f.print(flight_s,1); f.print(F(","));
      f.print(reading); f.print(F(","));
      f.print(lastAz_g,3); f.print(F(",")); f.print(lastAzMa_g,3); f.print(F(","));
      f.println(F("heatpad_pulse_30s"));
      f.close();
    }
  }
}

void handleSDCard(int reading, float imu_g){
  unsigned long nowMs = millis();
  float flight_s = liftoffDetected ? (nowMs - liftoffMs)/1000.0f : 0.0f;

  dataFile = SD.open("DATAFILE.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.print(nowMs); dataFile.print(F(","));
    dataFile.print(flight_s,1); dataFile.print(F(","));
    dataFile.print(reading); dataFile.print(F(","));
    dataFile.print(imu_g,3); dataFile.print(F(","));
    dataFile.print(lastAzMa_g,3); dataFile.print(F(",")); // |â|_MA (g)
    dataFile.println();
    dataFile.close();
  }
}

void handleMotors(int motor){
  Servo &s = (motor==0) ? servo1 : servo2;
  for (uint8_t pos = 0; pos <= 60; pos++) { s.write(pos); delay(15); }
}

// ---- IMU: per-axis MA in mg + magnitude of smoothed vector ----
void handleIMU(){
  sensors_event_t a, g, t;
  imu.getEvent(&a, &g, &t);

  // raw components in mg (signed)
  int16_t ax = to_mg(a.acceleration.x);
  int16_t ay = to_mg(a.acceleration.y);
  int16_t az = to_mg(a.acceleration.z);

  // keep signed a_z (g) for logs
  lastAz_g = (float)az / 1000.0f;

  // update running sums (remove oldest, add newest)
  if (maCount == MA_WIN) {
    sumAx -= axBuf[maIdx]; sumAy -= ayBuf[maIdx]; sumAz -= azBuf[maIdx];
  } else {
    maCount++;
  }
  axBuf[maIdx] = ax; ayBuf[maIdx] = ay; azBuf[maIdx] = az;
  sumAx += ax; sumAy += ay; sumAz += az;
  maIdx = (maIdx + 1) % MA_WIN;

  // per-axis MA (mg)
  int16_t axMa = (int16_t)(sumAx / maCount);
  int16_t ayMa = (int16_t)(sumAy / maCount);
  int16_t azMa = (int16_t)(sumAz / maCount);

  // magnitude of smoothed vector (mg) using integer sqrt
  uint32_t m2 = (uint32_t)( (int32_t)axMa*axMa ) +
                (uint32_t)( (int32_t)ayMa*ayMa ) +
                (uint32_t)( (int32_t)azMa*azMa );
  uint16_t aMagMa_mg = (uint16_t)isqrt32(m2);
  lastAzMa_g = aMagMa_mg / 1000.0f;   // store for logs (|â|_MA in g)

  // --- state machine on |â|_MA (mg) ---
  cntLiftoff = (aMagMa_mg >= LIFTOFF_MG) ? (uint8_t)(cntLiftoff+1) : 0;
  if (!liftoffDetected && cntLiftoff >= SUSTAIN_N) {
    liftoffDetected = true; liftoffMs = millis();
    File f = SD.open("DATAFILE.txt", FILE_WRITE);
    if (f) { f.print(liftoffMs); f.println(F(",0,,,," "liftoff_detected")); f.close(); }
  }

  if (liftoffDetected && !zeroGActive) {
    cntZeroG = (aMagMa_mg <= ZERO_G_MAX) ? (uint8_t)(cntZeroG+1) : 0;
    if (cntZeroG >= SUSTAIN_N) {
      zeroGActive = true;
      File f = SD.open("DATAFILE.txt", FILE_WRITE);
      if (f) { unsigned long ms=millis(); float fs=(ms-liftoffMs)/1000.0f;
        f.print(ms); f.print(F(",")); f.print(fs,1); f.println(F(",,,," "zero_g_start")); f.close(); }
    }
  }

  if (zeroGActive && !descentDetected) {
    cntDescent = (aMagMa_mg >= DESCENT_MG) ? (uint8_t)(cntDescent+1) : 0;
    if (cntDescent >= SUSTAIN_N) {
      descentDetected = true;
      File f = SD.open("DATAFILE.txt", FILE_WRITE);
      if (f) { unsigned long ms=millis(); float fs=(ms-liftoffMs)/1000.0f;
        f.print(ms); f.print(F(",")); f.print(fs,1); f.println(F(",,,," "descent_start")); f.close(); }
    }
  }

  // bus/rail hiccup watchdog: if |a|_MA ~0 for a bit, soft reinit
  static uint8_t flat=0;
  if (aMagMa_mg < 50) { if (++flat > 10) { imu.begin_I2C(0x6B); flat = 0; } }
  else flat = 0;
}

void loop() {
  if (gImuOk) { handleIMU(); } else { lastAz_g = 0.0f; lastAzMa_g = 0.0f; }
  unsigned long nowMs = millis();

  // phase selection
  uint8_t phase = 1;
  if (liftoffDetected && zeroGActive && !descentDetected) phase = 2;
  else if (descentDetected) phase = 3;

  static uint8_t lastPhase = 0xFF;
  if (phase != lastPhase) {
    dataFile = SD.open("DATAFILE.txt", FILE_WRITE);
    if (dataFile) {
      if (phase==1) dataFile.println(F("## PHASE 1 (Preload + Ascension) ##"));
      if (phase==2) { dataFile.println(F("Phase 1 over... starting Phase 2")); dataFile.println(F("## PHASE 2 (Injections) ##")); }
      if (phase==3) { dataFile.println(F("Phase 2 over... starting Phase 3")); dataFile.println(F("## PHASE 3 (Descension) ##")); }
      dataFile.close();
    }
    lastPhase = phase;
  }

  switch (phase) {
    case 1: {
      int reading = readResistor(resistorPin);
      bool allowHeat = liftoffDetected && !descentDetected;  // includes zero-g
      handleSDCard(reading, lastAz_g);
      handleHeatPads(reading, resistorPin, allowHeat, nowMs);
    } break;

    case 2: {
      int reading = readResistor(resistorPin);
      handleSDCard(reading, lastAz_g);

      static bool injectionDone = false;
      if (!injectionDone) {
        handleMotors(0); delay(300); handleMotors(1);
        dataFile = SD.open("DATAFILE.txt", FILE_WRITE);
        if (dataFile) {
          float flight_s = (nowMs - liftoffMs)/1000.0f;
          dataFile.print(nowMs); dataFile.print(F(",")); dataFile.print(flight_s,1); dataFile.println(F(",,,,injection_performed"));
          dataFile.close();
        }
        injectionDone = true;
      }
    } break;

    case 3: {
      int reading = readResistor(resistorPin);
      handleSDCard(reading, lastAz_g);
      handleHeatPads(reading, resistorPin, false, nowMs);  // off in descent
    } break;
  }

  delay(100);
}
