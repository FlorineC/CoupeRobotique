#ifndef __BOARD_H
#define __BOARD_H
#include <Wire.h>
#include "ZTCS34725.h"
#include "arduino.h"
#include <variant.h>
#include "lcd.h"
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

#else 
#error

#endif
#define MySerialDebug MySerial
#endif

typedef union {
  struct {
    tcolor color;
    LED4x7_t led;
    uint32_t boutonDemarrer;
  };
uint8_t mem[100];
} tdevice;
extern tdevice device;

