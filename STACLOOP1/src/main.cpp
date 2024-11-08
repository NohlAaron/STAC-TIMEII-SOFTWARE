#include <Arduino.h>

int analogPin1 = PIN_A1;
int analogPin2 = PIN_A2;
int analogPin3 = PIN_A3;
int analogPin4 = PIN_A4;
int analogPin5 = PIN_A5;
int raw = 0;
float Vin = 3.30;  // Adjust to 3.3 if that's your reference voltage
float Vout = 0;
float R1 = 100.0; // Known resistor value in ohms
float R2 = 0;
float buffer = 0;

void setup(){
  Serial.begin(9600);
}

void loop(){
  raw = analogRead(analogPin1);
  if(raw){
    //Serial.print("raw read: ");
    //Serial.println(raw);
    buffer = raw * Vin;           // Calculate ADC output in terms of voltage
    Vout = buffer / 1023.0;       // 4096.0 for 12-bit resolution
    
    if (Vout != 0) {              // Avoid division by zero
      buffer = (Vin / Vout) - 1;  // Calculate R2 based on Vout
      R2 = R1 * buffer;
      
      Serial.print("Vout pin1: ");
      Serial.println(Vout);
      Serial.print("R2 pin1: ");
      Serial.println(R2);
      
    } else {
      Serial.println("Vout is 0, unable to calculate R2.");
    
    }

    delay(1000);
  }
  raw = analogRead(analogPin2);
  if(raw){
    //Serial.print("raw read: ");
    //Serial.println(raw);
    buffer = raw * Vin;           // Calculate ADC output in terms of voltage
    Vout = buffer / 1023.0;       // 4096.0 for 12-bit resolution
    
    if (Vout != 0) {              // Avoid division by zero
      buffer = (Vin / Vout) - 1;  // Calculate R2 based on Vout
      R2 = R1 * buffer;
      
      Serial.print("Vout pin2: ");
      Serial.println(Vout);
      Serial.print("R2 pin2: ");
      Serial.println(R2);
      
    } else {
      Serial.println("Vout is 0, unable to calculate R2.");
    
    }

    delay(1000);
  }
  raw = analogRead(analogPin3);
  if(raw){
    //Serial.print("raw read: ");
    //Serial.println(raw);
    buffer = raw * Vin;           // Calculate ADC output in terms of voltage
    Vout = buffer / 1023.0;       // 4096.0 for 12-bit resolution
    
    if (Vout != 0) {              // Avoid division by zero
      buffer = (Vin / Vout) - 1;  // Calculate R2 based on Vout
      R2 = R1 * buffer;
      
      Serial.print("Vout pin3: ");
      Serial.println(Vout);
      Serial.print("R2 pin3: ");
      Serial.println(R2);
      
    } else {
      Serial.println("Vout is 0, unable to calculate R2.");
    
    }

    delay(1000);
  }
  raw = analogRead(analogPin4);
  if(raw){
    //Serial.print("raw read: ");
    //Serial.println(raw);
    buffer = raw * Vin;           // Calculate ADC output in terms of voltage
    Vout = buffer / 1023.0;       // 4096.0 for 12-bit resolution
    
    if (Vout != 0) {              // Avoid division by zero
      buffer = (Vin / Vout) - 1;  // Calculate R2 based on Vout
      R2 = R1 * buffer;
      
      Serial.print("Vout pin4: ");
      Serial.println(Vout);
      Serial.print("R2 pin4: ");
      Serial.println(R2);
      
    } else {
      Serial.println("Vout is 0, unable to calculate R2.");
    
    }

    delay(1000);
  }
  raw = analogRead(analogPin5);
  if(raw){
    //Serial.print("raw read: ");
    //Serial.println(raw);
    buffer = raw * Vin;           // Calculate ADC output in terms of voltage
    Vout = buffer / 1023.0;       // 4096.0 for 12-bit resolution
    
    if (Vout != 0) {              // Avoid division by zero
      buffer = (Vin / Vout) - 1;  // Calculate R2 based on Vout
      R2 = R1 * buffer;
      
      Serial.print("Vout pin5: ");
      Serial.println(Vout);
      Serial.print("R2 pin5: ");
      Serial.println(R2);
      
    } else {
      Serial.println("Vout is 0, unable to calculate R2.");
    
    }

    delay(1000);
  }
}
