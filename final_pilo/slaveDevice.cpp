#include "board.h"
#include <Wire.h>
#include <ZWireDevice.h>
#include <WireUtility.h>


#define DeviceAddress 0x15
#define MyWirePc P_COM2.wire

ZWireDevice slave = ZWireDevice();

void display()
{
	delay(100);
}

void SlaveDevice_requestEvent()
{
   Serial.println("SlaveDevice_receiveEvent");
  slave.requestEvent();
  display();
}
void SlaveDevice_receiveEvent(int iData)
{
  
 Serial.println("SlaveDevice_receiveEvent");
  slave.receiveEvent(iData);
  display();
  }

void SlaveDevice_setup()
{
  
//I2C slave configuration
slave.begin(MyWirePc,DeviceAddress,(uint8_t *)(device.mem));
//slave.setSerialDebug(MySerial);
MyWirePc.onReceive(SlaveDevice_receiveEvent);
MyWirePc.onRequest(SlaveDevice_requestEvent);

  Serial.println("SlaveDevice_setup");
}

