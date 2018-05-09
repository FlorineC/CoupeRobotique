#include <Wire.h>
#include "ZTCS34725.h"
#include "color.h"
//red : A5, orange :GND, green : A5, blue : A4
// Pick analog outputs, for the UNO these three work well
// use ~560  ohm resistor between Red & Blue, ~1K for green (its brighter)


#ifdef ARDUINO_AVR_UNO
#define redpin -1
#define greenpin -1
#define bluepin -1

    #define MySerial Serial //usb
    #define Mywire Wire // A4 SDA //A5 SCL
    
#elif defined(BOARD_ID_Pilo)

    #include <SPI.h>
    #include <variant.h>
    #include <bootloaders/boot.h>
    #define Serialpc P_COM3.serial2
    #define wirecard 
   
#define redpin -1
#define greenpin -1
#define bluepin -1

    #define MySerial P_COM3.serial2 //usb
    #define Mywire P_COM0_BIS.wire // Pcomx.pin6 SDA //Pcomx.pin5 SCL
    #define MyPin P_COM3.Pin.PIN09 //usb
    
    
#elif defined(BOARD_ID_Captor)



#endif
#include "color.cpp"
#define MySerialDebug MySerial
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



