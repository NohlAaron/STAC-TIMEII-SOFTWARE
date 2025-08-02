#include <Arduino.h>
/* Test Code to use a transistor to turn aheatpad on and off*/

#define HEATPAD_PIN 2

void setup() {
  Serial.begin(9600);
  delay(1000);
  pinMode(HEATPAD_PIN, OUTPUT);

  Serial.println("Heatpad initialized!");

}

void loop() {
  digitalWrite(HEATPAD_PIN, HIGH);  
  Serial.println("Heatpad On....");
  delay(30000);                      
  digitalWrite(HEATPAD_PIN, LOW);   
  Serial.println("Heatpad Off...");
  delay(10000);                     
}

