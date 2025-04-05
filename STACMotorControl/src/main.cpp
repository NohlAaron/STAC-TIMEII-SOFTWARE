#include <Arduino.h>
/*Motor Control code for the Clinostat, this code uses platformIO to use an STM32 but just copy paste this
code into Arduino IDE for using a standard arduino board. */

// Define all pins
const int buttonswitch = 2;
const int enableX = 6;
const int stepX = 7;
const int directionX = 8;
const int enableY = 3;
const int stepY = 4;
const int directionY = 5;

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");

  // Set all pins to outputs except button
  pinMode(buttonswitch, INPUT_PULLUP);  // Button with internal pull-up
  pinMode(enableX, OUTPUT);
  pinMode(stepX, OUTPUT);
  pinMode(directionX, OUTPUT);
  pinMode(enableY, OUTPUT);
  pinMode(stepY, OUTPUT);
  pinMode(directionX, OUTPUT);

  // Set motor directions
  digitalWrite(directionX, HIGH);
  digitalWrite(directionY, HIGH);

  // Disable motors initially
  digitalWrite(enableX, HIGH);
  digitalWrite(enableY, HIGH);

  Serial.println("Initialized!");
}

void loop() {
  int switchState = digitalRead(buttonswitch);
  Serial.print("Switch state: ");
  Serial.println(switchState);
  //delay(100);  // Just to prevent serial flooding

  if (switchState == LOW) {
    Serial.println("Button Switched ON - Motors Running");

    // Enable motors
    digitalWrite(enableX, LOW);
    digitalWrite(enableY, LOW);

    // Step motors 200 times
    for (int i = 0; i < 200; i++) {
      digitalWrite(stepX, HIGH);
      digitalWrite(stepX, LOW);
      delayMicroseconds(200);
      // X Motor Controls ^^^
      digitalWrite(stepY, HIGH);
      digitalWrite(stepY, LOW);
      delayMicroseconds(5000);
      // Y Motor Controls ^^^
    }

    // Optional: disable motors again
    digitalWrite(enableX, HIGH);
    digitalWrite(enableY, HIGH);

    Serial.println("Motors Disabled");
  }
}