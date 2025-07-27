#include <Arduino.h>
#include <Servo.h>

Servo servo1;
Servo servo2;

void setup() {
  servo1.attach(3);
  servo2.attach(4);
  Serial.begin(115200);
}

void loop() {
      // Sweep from 0 to 180 degrees
    Serial.println("sweeping left....");
    for (int pos = 0; pos <= 180; pos += 1) {
        servo1.write(pos);
        servo2.write(pos);
        delay(15); // Small delay for smooth motion
    }

    // Sweep back from 180 to 0 degrees
    Serial.println("sweeping right....");
    for (int pos = 180; pos >= 0; pos -= 1) {
        servo1.write(pos);
        servo2.write(pos);
        delay(15);
    }
}