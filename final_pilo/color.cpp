// from #include "C:\Temp\robot\git\intern\robot1\arduino\codeFlorine\colorview_pilo_may08\colorview_pilo_may08.ino"

#include "board.h"
////////////////////////////////////////////////////////////////////////
//////////////////////   MEM   ////////////////////////////////////////

#include "color.h"
#include "ZTCS34725.h"

ZTCS34725 colorLeft = ZTCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
ZTCS34725 colorRight = ZTCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
ZTCS34725 colorColumn = ZTCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
#define pinPresenceCubeColorCaptor P_ANA1.Pin.PIN16


//////////////////////////////////////////////////////////////////////
//////////////// ZTCS34725 //////////////////////////////////////////

// for a common anode LED, connect the common pin to +5V
// for common cathode, connect the common to ground

// set to false if using a common cathode LED
#define commonAnode true
// our RGB -> eye-recognized gamma color
byte gammatable[256];


void setupTCS34725(ZTCS34725 &tcs, TwoWire & thewire) {
  if (tcs.begin(thewire)) {
    MySerial.println("Found sensor");
  } else {
    MySerial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }

  // use these three pins to drive an LED
  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);

  // thanks PhilB for this gamma table!
  // it helps convert RGB colors to what humans see
  for (int i = 0; i < 256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, (float)2.5);
    x *= 255;

    if (commonAnode) {
      gammatable[i] = 255 - x;
    } else {
      gammatable[i] = x;
    }
  }
  digitalWrite(MyPin, LOW);   // turn the LED on (HIGH is the voltage level)

}



bool detectColor(int rValue, int gValue, int bValue, float rReference, float gReference, float bReference, float errorMargin) {
  if ((rReference * (1 - errorMargin) <= (int)rValue && (int)rValue <= rReference * (1 + errorMargin)) &&
      (bReference * (1 - errorMargin) <= (int)bValue && (int)bValue <= bReference * (1 + errorMargin)) &&
      (gReference * (1 - errorMargin) <= (int)gValue && (int)gValue <= gReference * (1 + errorMargin))) {
    return true;
  } else {
    return false;
  }
}



char getColor(ZTCS34725 & tcs) {
  uint16_t clear, red, green, blue;
  char result = Undefined;

  tcs.setInterrupt(false);      // turn on LED

  delay(60);  // takes 50ms to read

  tcs.getRawData(&red, &green, &blue, &clear);

  tcs.setInterrupt(true);  // turn off LED
  if (MySerialDebug)
  {
    MySerialDebug.print("C:\t"); MySerial.print(clear);
    MySerialDebug.print("\tR:\t"); MySerial.print(red);
    MySerialDebug.print("\tG:\t"); MySerial.print(green);
    MySerialDebug.print("\tB:\t"); MySerial.print(blue);
  }
  // Figure out some basic hex code for visualization
  uint32_t sum = clear * 0.9;
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
  r *= 256; g *= 256; b *= 256;
  if (MySerialDebug)
  {
    MySerialDebug.print("\t");
    MySerialDebug.print((int)r, HEX);
    MySerialDebug.print((int)g, HEX);
    MySerialDebug.print((int)b, HEX);
    MySerialDebug.println();

    //MySerialDebug.print((int)r ); MySerialDebug.print(" "); MySerialDebug.print((int)g);MySerialDebug.print(" ");  MySerialDebug.println((int)b );
  }
  analogWrite(redpin, gammatable[(int)r]);
  analogWrite(greenpin, gammatable[(int)g]);
  analogWrite(bluepin, gammatable[(int)b]);

  float errorMargin = 0.3;

  float rYellow = 130;
  float gYellow = 113;
  float bYellow = 51;

  float rOrange = 170;
  float gOrange = 76;
  float bOrange = 51;

  float rGreen = 75;
  float gGreen = 150;
  float bGreen = 68;

  float rBlue = 42;
  float gBlue = 108;
  float bBlue = 145;

  float rBlack = 79;
  float gBlack = 108;
  float bBlack = 97;


  float rWhite = 86;
  float gWhite = 108;
  float bWhite = 85;

  if (detectColor((int) r, (int) g, (int) b, rOrange, gOrange, bOrange, errorMargin))
  {
    MySerial.println("ORANGE");
    result = Orange;

  }
  else if (detectColor((int) r, (int) g, (int) b, rYellow, gYellow, bYellow, errorMargin))
  {
    MySerial.println("YELLOW");
    result = Yellow;

  }
  else if (detectColor((int) r, (int) g, (int) b, rWhite, gWhite, bWhite, errorMargin))
  {
    MySerial.println("WHITE");
    result = White;
  }
  else if (detectColor((int) r, (int) g, (int) b, rGreen, gGreen, bGreen, errorMargin))
  {
    MySerial.println("GREEN");
    result = Green;

  }
  else if (detectColor((int) r, (int) g, (int) b, rBlue, gBlue, bBlue, errorMargin))
  {
    MySerial.println("BLUE");
    result = Bleue;
  }
  else if (detectColor((int) r, (int) g, (int) b, rBlack, gBlack, bBlack, errorMargin))
  {
    MySerial.println("BLACK");
    result = Black;
  }

  if (MySerialDebug)
  {
    MySerial.println((int)r);
    MySerial.println((int)g);
    MySerial.println((int)b);
  }
  return result;

}




/////////////////////////////////////////////////////

void setupColor() {

  setupTCS34725(colorLeft, P_COM1.wire);
  setupTCS34725(colorRight, P_COM5.wire);
  setupTCS34725(colorColumn, P_COM0_BIS.wire);


  // initialize the pushbutton pin as an input:
  pinMode(pinPresenceCubeColorCaptor, INPUT_PULLUP);

}
void loop(tcolor &color) {

  color.Left = getColor( colorLeft);
  color.Right = getColor( colorRight);
  color.Column = getColor( colorColumn);
  color.cubePresent = digitalRead(pinPresenceCubeColorCaptor) == LOW ? CUBE_PRESENT : CUBE_ABSENT;

}







