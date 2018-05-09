#include "board.h"
#include "color.h"
#include "LCD.h"
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








void loop() {

  loop(color);

}




