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
int motorPins[] = {2,3};

File dataFile;
const int chipSelect = 4;

Servo servo1, servo2;

void setup() {
  servo1.attach(motorPins[0]);
  servo2.attach(motorPins[1]);
  pinMode(HEATPAD_PIN, OUTPUT);

  Serial.begin(9600);
  Serial.println("Started Timer.");

  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }

  Serial.println("SD card initialized.");
  delay(100);

  dataFile = SD.open("DATAFILE.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println("File Opened! This is the temperature data for TIME II!");
    dataFile.println("Waiting 8 seconds to initalize...");
    Serial.println("File opened successfully.");
    Serial.println("waiting 8 seconds to initalize....");
    delay(8000);
  } else {
    Serial.println("Failed to open file.");
  }
  dataFile.close();


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
    Serial.print("ADC extreme: ");
    Serial.println(raw);
    Serial.println("ADC out of range. Skipping...");
    return 0;
  }

  float Vout = (raw / 1023.0) * Vin;
  float buffer = Vout / (Vin - Vout);
  float R2 = R1 * buffer;

  Serial.print("Vout pin");
  Serial.print(pin);
  Serial.print(": ");
  Serial.println(Vout, 3);

  Serial.print("thermistor prediction");
  Serial.print(pin);
  Serial.print(": ");
  Serial.println(R2, 2);

  float expectedVout = Vin * R2 / (R1 + R2);
  Serial.print("Expected Vout for R2: ");
  Serial.println(expectedVout, 3);

  delay(500);
  return R2;
}

void handleHeatPads(int reading, int pin){
  if (reading > 5000) {
    Serial.println("resistor " + String(pin) + " is above 5000 ohms. Turning pad on...");
    digitalWrite(HEATPAD_PIN, HIGH); 
    Serial.println("Heatpad On....");
    delay(30000);  
    digitalWrite(HEATPAD_PIN, LOW);   
    Serial.println("Heatpad Off...");

  } else if (reading == 0){
    Serial.println("resistor" + String(pin) + " reading fail.");

  } else {
    Serial.println("resistor " + String(pin) + " is below 5000 ohms.");

  }
}

void handleSDCard(int reading){
  dataFile = SD.open("DATAFILE.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.print("Thermistor Reading: ");
    dataFile.println(reading);
    dataFile.flush();  // write to card
    if(reading > 5000){
      dataFile.println("Heatpad on for 30 seconds...");
    }
    dataFile.close(); 
  } else {
    Serial.println("File pointer is invalid.");
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
void handleIMU(int pin){
  //TODO: Fill in
}


void loop() {
  //phase 1 (checks every thermoresistor, turns pad on accordingly) :
  dataFile = SD.open("DATAFILE.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println("############################ PHASE 1 (Preload + Ascension) ############################");
    Serial.println("File opened successfully. (phase 1)");
  } else {
    Serial.println("Failed to open file. (phase 1)");
  }
  dataFile.close();
  while (true){ //TODO: Include IMU function
    int reading = readResistor(resistorPin);
    handleSDCard(reading);
    handleHeatPads(reading, resistorPin);      
    //Include IMU Reading Here
 
  }

  //phase 2(Continues phase 1 Procedure, now turns motors on in between):
  dataFile = SD.open("DATAFILE.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Phase 1 over... starting Phase 2");
    dataFile.println("############################ PHASE 2 (Injections) ############################");
    Serial.println("File opened successfully. (phase 2)");
  } else {
    Serial.println("Failed to open file. (phase 2)");
  }
  dataFile.close();
  while (false){ //TODO: Include IMU Function
    //TODO: plan injection timings

  }

  //phase 3 (Converts back to phase 1 procedure):
  dataFile = SD.open("DATAFILE.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Phase 2 over... starting Phase 3");
    dataFile.println("############################ PHASE 3 (Descension) ############################");
    Serial.println("File opened successfully. (phase 3)");
  } else {
    Serial.println("Failed to open file. (phase 3)");
  }
  dataFile.close();
  while (false){ //TODO: Set to True once IMU Function is implemented.
    int reading = readResistor(resistorPin);
    handleSDCard(reading);  
    handleHeatPads(reading, resistorPin);

  }
}