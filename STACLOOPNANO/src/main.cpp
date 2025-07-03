#include <Arduino.h>
#include <Servo.h>
#include <SD.h>
#include <SPI.h>

/* This is currently TIME II's Main Control loop code. Right now, we currently have most of a phase 1 implementation complete.
We should begin to work on Implementations of other phases and transistioning between each phase. Thermoresistor should
use 3.3 Voltage Pins, heating pads + motors should use 5 V pins. 

Included libaries regarding SD Card Writing, Can't do without SD Card slot to connect. Guide on SD Card writing at:
https://www.circuitbasics.com/writing-data-to-files-on-an-sd-card-on-arduino/ 

7/2/25: This has been ported over to an elegoo Nano. Should test the thermoresistors to make sure the analog calculations are still correct
There is servo stuff here, but cant be finished until I get a battery clip for the servo + heat pads, I also need the IMU before I can do that
MAIN ToDos:
  -IMU testing
  -SD Card Writing
  -Servo Motors + Heater Pads (requires 9v pads)
*/
int resistorPins[] = {A1, A2, A3, A4, A5};
int raw = 0;
float Vin = 3.30;  // for analog voltage pins
float Vout = 0;
float R1 = 100.0; // Known resistor value in ohms
float R2 = 0;
float buffer = 0;

//int heatPad = GPIO_PIN_0;
int heatPads[] = {2,3,4,5,6};
int motorPins[] = {7,8};
#define heatPad LED_BUILTIN //temporary, just meant to simulate turning the pad on

Servo servo1, servo2;
int angle = 0;     // variable to store the servo position
int step = 1;      // increment step



void setup() {
  servo1.attach(motorPins[0]);
  servo2.attach(motorPins[1]);


  pinMode(13, OUTPUT);
}

float readResistor(int pin){
  raw = analogRead(pin);
  if(raw){
    buffer = raw * Vin;           // Calculate ADC output in terms of voltage
    Vout = buffer / 1023.0;       // 4096.0 for 12-bit resolution  
    if (Vout != 0) {              // Avoid division by zero
      buffer = (Vin / Vout) - 1;  // Calculate R2 based on Vout
      R2 = R1 * buffer;  
      Serial.print("Vout pin" + String(pin) + ": ");
      Serial.println(Vout);
      Serial.print("R2 pin" + String(pin) + ": ");
      Serial.println(R2);
      
    } else {
      Serial.println("Vout is 0, unable to calculate R2.");
    
    }
    delay(1000);
    return R2;

  } 
  return 0;
}

void handleHeatPads(int reading, int pin){
  if (reading > 1000) {
    Serial.println("resistor " + String(pin) + " is above 1000 ohms. Turning pad on...");
    digitalWrite(heatPad, HIGH);
    for(int x = 0; x < 5; x++){
      Serial.print('.');
      delay(1000);
    }
    Serial.println(" ");
    Serial.println("heating pad off, retrieving next resistor read....");
    digitalWrite(heatPad, LOW);

  } else if (reading == 0){
    Serial.println("resistor" + String(pin) + " reading fail, next resistor read....");

  }
    else {
    Serial.println("resistor " + String(pin) + " is below 1000 ohms. next resistor read...");

  }
  delay(1500);

}

void loop() {
  //phase 1 (checks every thermoresistor, turns pad on accordingly) :
  while (false){ //will change condition once all phases are complete
    for(int i = 0; i < 5; i++){
      int reading = readResistor(resistorPins[i]);
      handleHeatPads(reading, resistorPins[i]);
      
      //Include SD Card Writing Here
      //Include IMU Reading Here
    }
  }

  //TODO
  //phase 2(Continues phase 1 Procedure, now turns motors on in between):
  while (true){ //will change condition once all phases are complete
    for(int i = 0; i < 5; i++){
      int reading = readResistor(resistorPins[i]);
      handleHeatPads(reading, resistorPins[i]);

      //TODO: MOTOR WRITING
      servo1.write(angle);     // set servo position
      angle += step;            // update angle
      if (angle <= 0 || angle >= 180) {
        step = -step;           // reverse direction at limits
      }

      delay(15);  // delay for smooth motion (~60 deg/sec) 
      
      
      //Include SD Card Writing Here
      //Include IMU Reading Here
    }
  }

    //phase 3 (Converts back to phase 1 procedure):
    while (false){ //will change condition once all phases are complete
    for(int i = 0; i < 5; i++){
      int reading = readResistor(resistorPins[i]);
      handleHeatPads(reading, resistorPins[i]);

      //Include SD Card Writing Here
      //Include IMU Reading Here
    }
  }
}