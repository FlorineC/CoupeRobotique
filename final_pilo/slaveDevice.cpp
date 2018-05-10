#include <ZWireDevice.h>
#include <WireUtility.h>
#include "board.h"


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
slave.begin(MyWireSlave,DeviceAddress,(uint8_t *)(device.mem));
//slave.setSerialDebug(MyWireSlave);
MyWireSlave.onReceive(SlaveDevice_receiveEvent);
MyWireSlave.onRequest(SlaveDevice_requestEvent);

  Serial.println("SlaveDevice_setup");
}


