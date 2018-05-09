// ESSAI colonne V1
#include <Servo.h>

Servo piston;


typedef  union
{
  struct  {
   char action; // MONO , MONF , POUS, |FINISH, NOP 
   char presenceCubeEnHaut; // 0 non, 1 oui
 // courant ? 
 
  } ;
uint8_t mem[100];
} collonne_t;

#define MONO '1' //1 Mise en route de la colonne de montée
#define MONF '2' //2 // Arrêt de la colonne de montée
#define MOWT 'z' //z // wait : bas arret haut au ralenti
#define POUS '3' //3 // Activation du poussoir de chargement du cube
#define FINISH 128
#define NOP 0
collonne_t device;//variable colone pour utiliser

void finish()
{
   device.action=device.action | FINISH;
   display();
}

//##################### USART ############################
byte receivedByte;
boolean newData = false;
byte reception;


void recvOneByte() {
 if (Serial.available() > 0) {
  display();
 receivedByte = Serial.read();
 newData = true;
 }
}

void showNewData() {
 if (newData == true) {
 Serial.print("Caractère reçu : ");
 Serial.println(receivedByte);
 device.action= receivedByte;
 newData = false;
 display();
 }
}


//##################### USART ############################
//##################### WIRE ############################
#include <Wire.h>
#include <WireUtility.h>
#include <ZWireDevice.h>

#define DeviceAddress 0x15
#define MyWirePc Wire

ZWireDevice slave = ZWireDevice();
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

//##################### WIRE ############################
void stopMONF();
void coloneMONO();
void poussePOUS();
void waitNOWAIT();
void stopMONF();



void display()
{/*
 Serial.println("device:"); 
  Serial.print("\tdevice.RampePositionChargement = 0x"); Serial.println(device.RampePositionChargement,HEX); 
  Serial.print("\tdevice.RampePositionDechargement = 0x"); Serial.println(device.RampePositionDechargement,HEX); 
  Serial.print("\tdevice.RampePositionTransport = 0x"); Serial.println(device.RampePositionTransport,HEX); 
  Serial.print("\tdevice.action = 0x"); Serial.println(device.action,HEX);*/
 //  delay(100);
}

//Configuration des ports
const int roueGauche1 = 2;
const int roueGauche2= 4;
const int roueDroite  = 3;
const int etage1 = 5;
const int etage2 = 6;
const int etage3 = 9;
const int finCourse = 7;
const int poussoir=10;// pwm , vers servo
const int rouleau = 12;
const int roueExterieur = 11;
const int capteurCourant=A1;// entrÃ©e analogique
const int SDA_=A4;//Blanc
const int SCL_=A5;//bleu
int valeurCourant = 0;
int roueDroiteOperante = 0;

#define piston_position_nouveau_cube_force 181
#define piston_position_nouveau_cube 176
#define piston_position_pousse_cube 15


void setup() {
Serial.begin(115200);
Serial.println("Arduino reset");
pinMode(roueGauche1, OUTPUT);// vers in3 du  L298N (sens entrant)
pinMode(roueDroite, OUTPUT);
pinMode(roueGauche2, OUTPUT);// vers in4 du  L298N (sens ressortant)
pinMode(etage1, OUTPUT);
pinMode(etage2, OUTPUT);
pinMode(finCourse,INPUT_PULLUP );
pinMode(rouleau, OUTPUT);
pinMode(roueExterieur, OUTPUT);
pinMode(etage3, OUTPUT);
pinMode(poussoir, OUTPUT);
piston.attach(poussoir);
piston.write(piston_position_nouveau_cube_force);
delay(35);
piston.write(piston_position_nouveau_cube);
SlaveDevice_setup();
stopMONF();
Serial.println("collone est pret");
Serial.print("I2C addr=");
Serial.println(DeviceAddress);


}
#define LimitCourant 100

void displaycurrent()
{
   valeurCourant = analogRead(capteurCourant);
   delay(3);
   valeurCourant += analogRead(capteurCourant);
   delay(3);
   valeurCourant += analogRead(capteurCourant);
   delay(3);
   valeurCourant += analogRead(capteurCourant);
   valeurCourant/=4;
 Serial.print("current :");
 Serial.print(valeurCourant);
 Serial.println(" xK mA");
 }


 void homologation()
{
  Serial.print("device.action=");Serial.println(device.action,HEX);
 if (device.presenceCubeEnHaut==1)
  {
     Serial.println("presenceCubeEnHaut");
    if(device.action==(char)(MOWT|FINISH))
    {
      device.action=POUS;  
      Serial.println("POUS"); 
      delay(3500);
    }
    else
      device.action=MOWT;// keep the cube at top
     
  }
  else
  {
    device.action=MONO;
    Serial.println("MONO");
     
    delay(3500);
    }  
}

void loop() {

 
 recvOneByte();
 showNewData();

 homologation();
  
if (device.action == MONF){// all stop
  Serial.println("MONF"); 
  stopMONF();
    finish();
  }
else if (device.action == MONO){//colonne on
   Serial.println("MONO"); 
  coloneMONO();
    finish();
  }
else if (device.action == POUS){//pousse cube
   Serial.println("POUS"); 
   if (digitalRead(finCourse) == false )//cube present 
  {
    Serial.println("cube present "); 
   poussePOUS(); 
   finish();
  }  else
  Serial.println("cube absent "); 
   
  }
  
else if (device.action == MOWT){//pousse cube
  
   Serial.println("NOWT"); 
   Serial.println("cube present "); 
   waitNOWAIT();
   finish();
  }
else 
{}

//ANTI BOURRAGE :
//displaycurrent();
if (valeurCourant > LimitCourant && roueDroiteOperante == 1) {
digitalWrite(roueGauche1,LOW); // vers in3 du  L298N (sens entrant)
digitalWrite(roueGauche2,HIGH); // vers in4 du  L298N (sens ressortant)
delay(200);
digitalWrite(roueGauche1,HIGH); // vers in3 du  L298N (sens entrant)
digitalWrite(roueGauche2,LOW); // vers in4 du  L298N (sens ressortant)
delay(200);
//Serial.print (valeurCourant);
//Serial.print ("     ");
}
/////////////////////
device.presenceCubeEnHaut=digitalRead(finCourse) == false?1:0;
}


void coloneMONO()
{
int timed=0; 
 if(timed!=0)
 {
   digitalWrite(rouleau, HIGH);
    delay(timed);
displaycurrent();
    digitalWrite(roueExterieur, HIGH); 
    delay(timed);
displaycurrent();

digitalWrite(roueGauche2,LOW); // vers in4 du  L298N (sens ressortant)
delay(timed);
displaycurrent();
digitalWrite(roueGauche1,HIGH); // vers in3 du  L298N (sens entrant)
delay(timed);
displaycurrent();
digitalWrite(roueDroite,HIGH); // vers 4eme mosfet
delay(timed);
displaycurrent();
digitalWrite(etage1,HIGH);
delay(timed);
displaycurrent();
digitalWrite(etage2,HIGH);
delay(timed);
displaycurrent();
digitalWrite(etage3,HIGH);
delay(timed);
displaycurrent();   
}else
{
digitalWrite(rouleau, HIGH);
digitalWrite(roueExterieur, HIGH); 
digitalWrite(roueGauche2,LOW); // vers in4 du  L298N (sens ressortant)
digitalWrite(roueGauche1,HIGH); // vers in3 du  L298N (sens entrant)
digitalWrite(roueDroite,HIGH); // vers 4eme mosfet
digitalWrite(etage1,HIGH);
digitalWrite(etage2,HIGH);
digitalWrite(etage3,HIGH);
    
}
roueDroiteOperante= 1;
}
//1212121212121212121212121212121212121212

void poussePOUS()
{
   piston.write(piston_position_pousse_cube);
    analogWrite(etage1,70);//etage ralenti
    analogWrite(etage2,70);
    /**est ce pertinent d'arreter ca ?
    digitalWrite(roueDroite,LOW); // vers 4eme mosfet
    roueDroiteOperante= 0;
    digitalWrite(roueGauche1,LOW); // vers in3 du  L298N (sens entrant)
    digitalWrite(roueGauche2,LOW); // vers in4 du  L298N (sens ressortant)
    /****/
    delay(600);
    piston.write(piston_position_nouveau_cube_force);  
    delay(400);
    piston.write(piston_position_nouveau_cube);
    digitalWrite(etage1,HIGH);
    digitalWrite(etage2,HIGH);
    digitalWrite(roueDroite,HIGH); // vers 4eme mosfet
    roueDroiteOperante= 1;
    digitalWrite(roueGauche1,HIGH); // vers in3 du  L298N (sens entrant)
    digitalWrite(roueGauche2,LOW); // vers in4 du  L298N (sens ressortant)
    delay(100);    
    digitalWrite(etage3,HIGH);
    device.presenceCubeEnHaut=digitalRead(finCourse) == false?1:0;
}

void waitNOWAIT()
{
    analogWrite(etage1,70);//etage ralenti
    analogWrite(etage2,70);
    /**est ce pertinent d'arreter ca ?
    digitalWrite(roueDroite,LOW); // vers 4eme mosfet
    roueDroiteOperante= 0;
    digitalWrite(roueGauche1,LOW); // vers in3 du  L298N (sens entrant)
    digitalWrite(roueGauche2,LOW); // vers in4 du  L298N (sens ressortant)
    /****/
    digitalWrite(etage1,HIGH);
    digitalWrite(etage2,HIGH);
    digitalWrite(roueDroite,HIGH); // vers 4eme mosfet
    roueDroiteOperante= 1;
    digitalWrite(roueGauche1,HIGH); // vers in3 du  L298N (sens entrant)
    digitalWrite(roueGauche2,LOW); // vers in4 du  L298N (sens ressortant)
    delay(100);    
    digitalWrite(etage3,HIGH);
    device.presenceCubeEnHaut=digitalRead(finCourse) == false?1:0;
}

void stopMONF()
{
   piston.write(piston_position_nouveau_cube_force);  
   delay(60);
   piston.write(piston_position_nouveau_cube);
    analogWrite(etage1,0);//etage ralenti
    analogWrite(etage2,0);
    digitalWrite(roueDroite,LOW); // vers 4eme mosfet
    roueDroiteOperante= 0;
    digitalWrite(roueGauche1,LOW); // vers in3 du  L298N (sens entrant)
    digitalWrite(roueGauche2,LOW); // vers in4 du  L298N (sens ressortant)
    digitalWrite(etage1,LOW);
    digitalWrite(etage2,LOW);
    digitalWrite(etage3,LOW);
    digitalWrite(roueDroite,LOW); // vers 4eme mosfet
    digitalWrite(roueGauche1,LOW); // vers in3 du  L298N (sens entrant)
    digitalWrite(roueGauche2,LOW); // vers in4 du  L298N (sens ressortant)  
    digitalWrite(rouleau, LOW);
    digitalWrite(roueExterieur, LOW);  
  }

