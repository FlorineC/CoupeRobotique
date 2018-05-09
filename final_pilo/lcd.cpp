/** here we just say hello, and return to boot for next update.
*/
#include "board.cpp"

#include <TM1637Display.h>

#define HEADERSIZE 5
#define WRITE 0xF0
#define READ 0xF1
#define MAXOFFSET 250

#define Myserial P_COM3.serial2
#define CLK P_COM0_BIS.Pin.PIN16
#define DIO P_COM0_BIS.Pin.PIN18 /*marron*/
#define GND P_COM0_BIS.Pin.PIN04
#define VCC P_COM0_BIS.Pin.PIN02


int num, bsend, i;
uint8_t WriteOK = 0xFB;
int incomingByte = 0;
byte buff_header[5];
byte data[250];

TM1637Display display(CLK, DIO);

/*
  LED can display string or number
*/

void displayLED(LED4x7_t& segs)
{
  unsigned long newtime = micros();

  if ((newtime - segs.time) > ((unsigned long)segs.rollerover * 1000LL)) {
    if ((segs.string[segs.index] != 0x0) && (segs.index < LED_STRING_LENMAX) && (segs.index != -1)) {
      segs.time = newtime;
      if (segs.string[segs.index] != 0x0) {
        segs.data[0] = SevenSegmentASCII[segs.string[segs.index + 0] - 32];
        if (segs.string[segs.index + 1] != 0x0) {
          segs.data[1] = SevenSegmentASCII[segs.string[segs.index + 1] - 32];
          if (segs.string[segs.index + 2] != 0x0) {
            segs.data[2] = SevenSegmentASCII[segs.string[segs.index + 2] - 32];
            if (segs.string[segs.index + 3] != 0x0)
              segs.data[3] = SevenSegmentASCII[segs.string[segs.index + 3] - 32];
          }
        }
      }
      segs.index++;
    }
    else
      segs.index = -1;
    display.setBrightness(segs.Brightness);

    if ((segs.index == -1) && (segs.number != 0xFFFF))
      display.showNumberDec(segs.number, true);
    else
      display.setSegments((const uint8_t*)segs.data);
  }
}
volatile int toto = 0x12345678;
void setupLed()
{
  volatile int titi = toto;
  led.data[0] = 0xFF; // currently displayed in segment
  led.data[1] = 0xFF; // currently displayed in segment
  led.data[2] = 0xFF; // currently displayed in segment
  led.data[3] = 0xFF; // currently displayed in segment
  led.Brightness = 0xF;
  led.index = -1; // curent index of displyed string, -1 when not used.
  led.string = ""; // string display in rolling mode finished by /0x0
  led.Brightness = 0x7; // string display in rolling mode finished by /0x0
  led.rollerover = 500;
  led.number = 0; // display a number between 0 and 9999
  led.time = micros(); // display a number between 0 and 9999
}

void setupLCD()
{
  // put your setup code here, to run once:
  pinMode(P_COM4.Pin.P5, OUTPUT);
  pinMode(P_COM4.Pin.P4, INPUT);
  Myserial.begin(115200);
  setupLed();
  memmap = &led.data[0];

}

int indexloop = 0;
void loop()
{
  if (digitalRead(P_COM4.Pin.P4) == HIGH) {
    num = ((num * 11 / 3) + 1) % 5000;
    if (bsend == 0) {
      Myserial.print(num);
      bsend = 1;
    }
  }
  else {
    i++;
    if (i > 10) {
      i = 0;
      bsend = 0;
    }
  }
  digitalWrite(P_COM4.Pin.P5, HIGH);
  if (Myserial.available() >= 5) {
    Myserial.readBytes(buff_header, 5);
    int len = buff_header[4];
    if (buff_header[0] == WRITE) {
      digitalWrite(P_COM4.Pin.P5, LOW);
      if (Myserial.readBytes(&memmap[buff_header[1]], len) == len) {
        //data received ok
        Myserial.write(0xFB); // indicate transaction is ok
      }
      else {
        Myserial.write(0xFF); // send error to rpi
      }
    }
    else if (buff_header[0] == READ) {
      uint8_t* buffer = (uint8_t*)malloc(sizeof(uint8_t) * (len + 1));
      buffer[0] = 0xFC; // indicate an answer to a get request;
      int offset = buff_header[1];
      for (int i = 0; i < len; i++) {
        buffer[i + 1] = memmap[offset + i];
      }
      Myserial.Print::write(buffer, len + 1);
    }
    else {
      Myserial.write(0xFF);
    }
  }

  displayLED(led);
}

