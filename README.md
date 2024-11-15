This is the new repository for all the Software Related to STAC's TIME II Project.
This code uses an STM32 Nucleo F446RE Board to Read thermoresistor values, and turn on heating pads and motors according to those values.

To edit or make this file, we recommend using VS Code's PlatformIO Extension to write code for the STM32. For more information on how to install and use PlatformIO, visit this link: 
https://platformio.org/

Currently (11/8/24), we have a Naive solution to read 5 thermoresistor values. Here are some steps to take in the future over the rest of Fall + next spring.

1. Optimize the sensor reading
2. Implement the heating pad Functionality
3. Implement motor Functionality
4. Implement writing to SD Card
5. Implement a Final Loop to change between all 3 brainstormed phases. (See discord and slack to view whiteboard of procedure)
6. Test out ideal R1 values, expected thermoresistor values during experiment, required accuracy for reading thermoresistor, and heating pad and motor timings. 
7. Combine STM32 and Final PCB Board for the completed board


