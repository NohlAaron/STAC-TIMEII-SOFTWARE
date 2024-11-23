/* This is currently TIME II's Main Control loop code. Right now, we currently have most of a phase 1 implementation complete.
We should begin to work on Implementations of other phases and transistioning between each phase. Thermoresistor should
use 3.3 Voltage Pins, heating pads + motors should use 5 V pins. 

*/

#include <Arduino.h>

//Included libaries regarding SD Card Writing, Can't do without SD Card slot to connect. Guide on SD Card writing at:
// https://www.circuitbasics.com/writing-data-to-files-on-an-sd-card-on-arduino/ 
#include <SD.h>
#include <SPI.h>

int resistorPins[] = {PIN_A1, PIN_A2, PIN_A3, PIN_A4, PIN_A5};
int raw = 0;
float Vin = 3.30;  // for analog voltage pins
float Vout = 0;
float R1 = 100.0; // Known resistor value in ohms
float R2 = 0;
float buffer = 0;

//int heatPad = GPIO_PIN_0;
int motorPins[] = {GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5};
#define heatPad LED_BUILTIN //temporary, just meant to simulate turning the pad on

uint32_t timer_counter;
TIM_HandleTypeDef htim2;

void setup(){
  pinMode(heatPad, LOW);
   
  HAL_Init();
  SystemClock_Config();
  __HAL_RCC_TIM2_CLK_ENABLE();
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 72 - 1; // Assuming 72MHz clock, prescaler gives 1µs resolution
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xFFFFFFFF; // Max period (32-bit counter)
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
    while (1); // Initialization error
  }
  HAL_TIM_Base_Start(&htim2); // Start the timer

  Serial.begin(9600);
  Serial.println("Started Timer.");
}

int readResistor(int pin){
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

void heatPads(int reading, int pin){
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


//TODO, currently returns motor timer but should include the 5 cases for each motors
void motorTimer(){
    timer_counter = __HAL_TIM_GET_COUNTER(&htim2);
    float timeATM = timer_counter / 1000000.0; 
    Serial.print("STM32 Timer: ");
    Serial.print(timeATM, 6);
    Serial.println(" seconds.");
}

void loop(){
  //phase 1 (checks every thermoresistor, turns pad on accordingly) :
  while (true){ //will change condition once all phases are complete
    for(int i = 0; i < 5; i++){
      int reading = readResistor(resistorPins[i]);
      heatPads(reading, resistorPins[i]);
      
    }
  }

  //TODO
  //phase 2(Continues phase 1 Procedure, now turns motors on in between):
  while (false){ //will change condition once all phases are complete
    for(int i = 0; i < 5; i++){
      int reading = readResistor(resistorPins[i]);
      heatPads(reading, resistorPins[i]);
      
      motorTimer();
    }
  }

  //phase 3 (Converts back to phase 1 procedure):
    while (false){ //will change condition once all phases are complete
    for(int i = 0; i < 5; i++){
      int reading = readResistor(resistorPins[i]);
      heatPads(reading, resistorPins[i]);

    }
  }
}
