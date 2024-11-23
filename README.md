This is the new repository for all the Software Related to STAC's TIME II Project.
This code uses an STM32 Nucleo F446RE Board to Read thermoresistor values, and turn on heating pads and motors according to those values.

To edit or make this file, we recommend using VS Code's PlatformIO Extension to write code for the STM32. For more information on how to install and use PlatformIO, visit this link: 
https://platformio.org/

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


