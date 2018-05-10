#include <ZWireDevice.h>
#include <WireUtility.h>
#include "board.h"


ZWireDevice slave = ZWireDevice();

void displayDevice()
{
	delay(100);
}

void SlaveDevice_requestEvent()
{
   Serial.println("SlaveDevice_receiveEvent");
  slave.requestEvent();
  displayDevice();
}
void SlaveDevice_receiveEvent(int iData)
{
  
 Serial.println("SlaveDevice_receiveEvent");
  slave.receiveEvent(iData);
  displayDevice();
  }

void SlaveDevice_setup()
{
  
  volatile int ip = scan(MySerial, MyWireSlave);
   int device_addr = ip;
  while (ip = scanNext(MySerial, MyWireSlave) != 0)
    ;
  MySerial.println("setup end");
 
  MySerial.print("new device_addr=0x");
  MySerial.println(device_addr, HEX);
  
//I2C slave configuration
slave.begin(MyWireSlave,DeviceAddress,(uint8_t *)(device.mem));
//slave.setSerialDebug(MyWireSlave);
MyWireSlave.onReceive(SlaveDevice_receiveEvent);
MyWireSlave.onRequest(SlaveDevice_requestEvent);

  Serial.println("SlaveDevice_setup");


  uint8_t data[1];
data[1]=0x45;
MyWireSlave.begin();
  slave.set(0x14,3, 1,data );
  delay(1000);
  
  data[1]=0x46;
  slave.set(0x14,3, 1,data );
  delay(1000);
  data[1]=0x47;
slave.set(0x14,3, 1,data );
  
}


