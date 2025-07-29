#include <Arduino.h>
#include <Servo.h>
#include <SD.h>
#include <SPI.h>

File testFile;
int count = 0;
const int chipSelect = 5;

void setup() {
  Serial.begin(9600);
  delay(1000);

  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }

  Serial.println("SD card initialized.");
  delay(100);

  testFile = SD.open("LOOPTEST.txt", FILE_WRITE);
  if (testFile) {
    testFile.println("File Opened! starting count....");
    Serial.println("File opened successfully.");
  } else {
    Serial.println("Failed to open file.");
  }
  testFile.close();
  
}

void loop() {
  testFile = SD.open("LOOPTEST.txt", FILE_WRITE);
  if (testFile) {
    testFile.print("Hello World! Count: ");
    testFile.println(count);
    testFile.flush();  // write to card
    testFile.close(); 
  } else {
    Serial.println("File pointer is invalid.");
  }

  count++;
  delay(1000);
}

