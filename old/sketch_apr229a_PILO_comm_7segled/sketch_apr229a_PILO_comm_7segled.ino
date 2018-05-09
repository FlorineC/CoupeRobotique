#include <Wire.h>
#include <SPI.h>
#include <variant.h>
#include <bootloaders/boot.h>
/** here we just say hello, and return to boot for next update.
*/
#define HEADERSIZE  5
#define WRITE 0xF0
#define READ  0xF1
#define MAXOFFSET 250

#define Myserial P_COM3.serial2
    #define CLK P_COM0_BIS.Pin.PIN08
    #define DIO P_COM0_BIS.Pin.PIN07/*marron*/
    #define GND P_COM0_BIS.Pin.PIN04
    #define VCC P_COM0_BIS.Pin.PIN02
    
int num,bsend,i;

struct WR_header {
  uint8_t AddrDev[2];
  uint8_t AddrMem[2];
  uint8_t Count;
};
uint8_t WriteOK = 0xFB;
int incomingByte=0;
byte buff_header[5];
byte data[250];




#include <TM1637Display.h>

const uint8_t SEG_DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
  };


TM1637Display display(CLK, DIO);

const uint8_t SevenSegmentASCII[96] = {
  0x00, /* (space) */
  0x86, /* ! */
  0x22, /* " */
  0x7E, /* # */
  0x6D, /* $ */
  0xD2, /* % */
  0x46, /* & */
  0x20, /* ' */
  0x29, /* ( */
  0x0B, /* ) */
  0x21, /* * */
  0x70, /* + */
  0x10, /* , */
  0x40, /* - */
  0x80, /* . */
  0x52, /* / */
  0x3F, /* 0 */
  0x06, /* 1 */
  0x5B, /* 2 */
  0x4F, /* 3 */
  0x66, /* 4 */
  0x6D, /* 5 */
  0x7D, /* 6 */
  0x07, /* 7 */
  0x7F, /* 8 */
  0x6F, /* 9 */
  0x09, /* : */
  0x0D, /* ; */
  0x61, /* < */
  0x48, /* = */
  0x43, /* > */
  0xD3, /* ? */
  0x5F, /* @ */
  0x77, /* A */
  0x7C, /* B */
  0x39, /* C */
  0x5E, /* D */
  0x79, /* E */
  0x71, /* F */
  0x3D, /* G */
  0x76, /* H */
  0x30, /* I */
  0x1E, /* J */
  0x75, /* K */
  0x38, /* L */
  0x15, /* M */
  0x37, /* N */
  0x3F, /* O */
  0x73, /* P */
  0x6B, /* Q */
  0x33, /* R */
  0x6D, /* S */
  0x78, /* T */
  0x3E, /* U */
  0x3E, /* V */
  0x2A, /* W */
  0x76, /* X */
  0x6E, /* Y */
  0x5B, /* Z */
  0x39, /* [ */
  0x64, /* \ */
  0x0F, /* ] */
  0x23, /* ^ */
  0x08, /* _ */
  0x02, /* ` */
  0x5F, /* a */
  0x7C, /* b */
  0x58, /* c */
  0x5E, /* d */
  0x7B, /* e */
  0x71, /* f */
  0x6F, /* g */
  0x74, /* h */
  0x10, /* i */
  0x0C, /* j */
  0x75, /* k */
  0x30, /* l */
  0x14, /* m */
  0x54, /* n */
  0x5C, /* o */
  0x73, /* p */
  0x67, /* q */
  0x50, /* r */
  0x6D, /* s */
  0x78, /* t */
  0x1C, /* u */
  0x1C, /* v */
  0x14, /* w */
  0x76, /* x */
  0x6E, /* y */
  0x5B, /* z */
  0x46, /* { */
  0x30, /* | */
  0x70, /* } */
  0x01, /* ~ */
  0x00, /* (del) */
};
#define LED_STRING_LENMAX 64
/*
* LED can display string or number
*/
typedef  union {
  
        struct  {
      
        uint8_t data[4];// currently displayed in segment
        char Brightness;// contast
        signed char index;// curent index of displyed string, -1 when not used.
        String string;// string display in rolling mode finished by /0x0
        uint16_t number;// display a number between 0 and 9999
        uint16_t rollerover;
        unsigned long time;
        } ;
  
  uint8_t memmap[MAXOFFSET];
} LED4x7_t;

 LED4x7_t led;

  
void displayLED(LED4x7_t &segs)
  {
    unsigned long newtime=micros();
    
    if ((newtime-segs.time)>((unsigned long)segs.rollerover*1000LL))
    {
  if (   (segs.string[segs.index]!=0x0) && (segs.index<LED_STRING_LENMAX) &&(segs.index!=-1))
  {
    segs.time=newtime;
    if (segs.string[segs.index]!=0x0)
    {
      segs.data[0]=SevenSegmentASCII[segs.string[segs.index+0]-32];
      if (segs.string[segs.index+1]!=0x0)
      {
        segs.data[1]=SevenSegmentASCII[segs.string[segs.index+1]-32];
        if (segs.string[segs.index+2]!=0x0)
        {
          segs.data[2]=SevenSegmentASCII[segs.string[segs.index+2]-32];
          if  (segs.string[segs.index+3]!=0x0)
          segs.data[3]=SevenSegmentASCII[segs.string[segs.index+3]-32];
        }
      }
    }
    segs.index++;
  }
  else 
    segs.index=-1;
    display.setBrightness(segs.Brightness);

    if ((segs.index==-1) && (segs.number!=0xFFFF))
      display.showNumberDec(segs.number, true);
    else
      display.setSegments((const uint8_t*)segs.data);
}
}
volatile  int toto=0x12345678;
void setupLed()
{
  volatile int titi=toto;
   led.data[0]=0xFF;// currently displayed in segment
  led.data[1]=0xFF;// currently displayed in segment
  led.data[2]=0xFF;// currently displayed in segment
  led.data[3]=0xFF;// currently displayed in segment
  led.Brightness=0xF;
  led.index=-1;// curent index of displyed string, -1 when not used.
  led.string="";// string display in rolling mode finished by /0x0
 led.Brightness=0x7;// string display in rolling mode finished by /0x0
  led.rollerover=500;
  led.number=0;// display a number between 0 and 9999
  led.time=micros();// display a number between 0 and 9999
  
 
}
void setup() {
// put your setup code here, to run once:
  pinMode(P_COM4.Pin.P5, OUTPUT);
  pinMode(P_COM4.Pin.P4, INPUT);
  Myserial.begin(115200);
  setupLed();
  memmap=&led.data[0];
}


 int indexloop=0;
void loop() {
  if(digitalRead(P_COM4.Pin.P4)==HIGH) {
    num =((num*11/3)+1)%5000;
    if(bsend==0){
      Myserial.print(num);
      bsend=1;
    }
  }
  else{
      i++;
      if (i>10){
        i=0;
        bsend=0;
      }
  }
  digitalWrite(P_COM4.Pin.P5, HIGH);
  if(Myserial.available() >=5) {
    Myserial.readBytes(buff_header,5);
    int len = buff_header[4];
    if(buff_header[0]==WRITE){
      digitalWrite(P_COM4.Pin.P5, LOW);
      if(Myserial.readBytes(&memmap[buff_header[1]],len)==len) {
        //data received ok
        Myserial.write(0xFB); // indicate transaction is ok
      }
      else {
        Myserial.write(0xFF); // send error to rpi
      }
    }
    else if (buff_header[0] == READ) {
      uint8_t* buffer = (uint8_t*)malloc(sizeof(uint8_t)*(len+1));
      buffer[0]=0xFC; // indicate an answer to a get request;
      int offset = buff_header[1];
      for (int i=0;i < len; i++){
        buffer[i+1] = memmap[offset+i];
      }
      Myserial.Print::write(buffer,len+1);
    }
    else
    {
      Myserial.write(0xFF);
    }
  }
  
  
  
   displayLED(led);
}

