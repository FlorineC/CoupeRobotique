#include "board.h"
#include "demarrage.h"

#define button P_ANA0.Pin.PIN16

void setupDemarrage()
{
	pinMode(button, INPUT);
}

void loopDemarrer(){
	device.boutonDemarrer = digitalRead(button) == HIGH ? 1 : 0;
}
