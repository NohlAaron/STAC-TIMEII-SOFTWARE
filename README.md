This is the new repository for all the Software Related to STAC's TIME II Project.
STACLOOPNANO is a version of STACLOOP1 ported over to work with a Elegoo Nano Board. I have cleaned up the code a bit too and added some new task to fullfill within.
^We will be using this version from now on.

STACLOOP1 uses an STM32 Nucleo F446RE Board to Read thermoresistor values, and turn on heating pads and motors according to those values.
STACMOTORCONTROL is all code related to operating our Clinostat

To edit or make this file, we recommend using VS Code's PlatformIO Extension to write code for the STM32/Elegoo Nano. For the Nano, Arduino IDE works fine as you should be able to copy-paste the main.cpp files. For more information on how to install and use PlatformIO, visit this link: 
https://platformio.org/

UPDATE (7/2/2025): We have officially moved over to elegoo Nano. Here are some of the remaining tasks to complete before assembly + testing later this month:
-Finish SD Card Writing
-Implement IMU Reading (IMU not currently in Snode)
-Test Servo Motors + Heat Pads with 9V Batteries
-Retest Analog Sensor Readings
-LAST: replace usb power with 5V

----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

(4/5/2025): Added stepper motor control for the Clinostat, also as an update regarding the Main Motor Control, we will need to read an IMU value instead of In flight data. Should be easier to implement though than either flight data or reading gate voltages, and less unpredictable than timers. Here are our current toDos.
TODOS:
1. Reading IMU + Timers
2. Writing data to an SD Card
3. add servo motor control
4. Test Completed Code with a setup

UPDATE (11/22/2024): We have an optimized script for phase 1 to read Thermoresistor Values. We also included code regarding activating motors and heating pads + a timer to use during the actual Experiment. 
So far these are the bullet points for everything accomplished so far + ToDos going forward.
Complete:
1. Reading Thermoresistors
2. Turning on heatingPad (test with an actual pad instead of LED)
3. Turning on Motors (test with actual motors instead of LED)
4. Timer for Experiment
5. Overall easier code to read

TODOs:
1. Implement Code to write to an SD Card, make sure its written in a format easy to parse and make a table with
2. Implement Code to read OR Gate Voltage, for swapping between all phases, assuming we won't recieve flight data. If we are for sure going to recieve flight data, ignore this.
3. Implement Motor Timings. Discuss with Bio regarding the timings of each motor
4. Discuss with Mech the Motor Strength. May not be able to leave a motor on to inject syringes, might have to 'shake' the syringe by repeatedly turning motors on and off


(11/8/24): we have a Naive solution to read 5 thermoresistor values. Here are some steps to take in the future over the rest of Fall + next spring.

1. Optimize the sensor reading
2. Implement the heating pad Functionality
3. Implement motor Functionality
4. Implement writing to SD Card
5. Implement a Final Loop to change between all 3 brainstormed phases. (See discord and slack to view whiteboard of procedure)
6. Test out ideal R1 values, expected thermoresistor values during experiment, required accuracy for reading thermoresistor, and heating pad and motor timings. 
7. Combine STM32 and Final PCB Board for the completed board


