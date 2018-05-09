#include "board.h"
#include "color.h"
#include "LCD.h"
#include "demarrage.h"
#include "slaveDevice.h"
//#include "color.cpp"
//#include "lcd.cpp"

tcolor color;
void setup() {
  pinMode(MyPin, OUTPUT);
  digitalWrite(MyPin, HIGH);   // turn the LED on (HIGH is the voltage level)

  MySerial.begin(9600);
  MySerial.println("Color View Test!");
  setupColor();

}





 tdevice device;


void loop() {

  loop(color);
  loopLCD();
  loopDemarrer();
  SlaveDevice_setup();
}




