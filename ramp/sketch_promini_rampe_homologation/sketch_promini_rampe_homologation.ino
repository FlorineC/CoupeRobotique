#include <Servo.h>



#define MySerial Serial
    

Servo MG995;
Servo MG946;

typedef  union
{
  struct  {
	char RampePositionChargement; // 0 non, 1 oui
  char RampePositionDechargement; // 0 non, 1 oui
  char RampePositionTransport; // 0 non, 1 oui
	char action; // RAON , RAOF , RATR, |FINISH, NOP 
 
  } ;
uint8_t mem[100];
} rampe_t;

rampe_t device;//variable colone pour utiliser

void display()
{
 Serial.println("device:"); 
  Serial.print("\tdevice.RampePositionChargement = 0x"); Serial.println(device.RampePositionChargement,HEX); 
  Serial.print("\tdevice.RampePositionDechargement = 0x"); Serial.println(device.RampePositionDechargement,HEX); 
  Serial.print("\tdevice.RampePositionTransport = 0x"); Serial.println(device.RampePositionTransport,HEX); 
  Serial.print("\tdevice.action = 0x"); Serial.println(device.action,HEX);
   delay(100);
}

byte receivedByte;
boolean newData = false;
byte reception;
boolean nouvelleReception = false; //drapeau indiquant la présence de caractère dans le buffer série 
//Ports de commande du moteur N20 d'entrée sortie dans le chariot
const int CommandeMG995 = 3;
const int CommandeMG946 = 5;
const int SDA_=A4;//Blanc
const int SCL_=A5;//bleu
int i = 0; // variable utilitaire
// initialisation des variables d'instructions, de mouvement et de maintien.
String instMouv = "NOPE"; // instruction BAS NIVEAU de mouvement
int positionServo = 0;



//##################### WIRE ############################
#include <Wire.h>
#include <WireUtility.h>
#include <ZWireDevice.h>

#define DeviceAddress 0x14
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
  Serial.println("Adresse du device :"+DeviceAddress);
}

//##################### WIRE ############################




void setup() {
//Serial.begin (57600);
nouvelleReception = false; //drapeau indiquant la présence de caractère dans le buffer série
//Initialisation du port série
Serial.begin(115200);
Serial.println("Rampe est prêt");
SlaveDevice_setup();

pinMode(CommandeMG995, OUTPUT);
pinMode(CommandeMG946, OUTPUT);
MG995.attach(CommandeMG995);
MG946.attach(CommandeMG946);
MG995.write(0);
MG946.write(0);
positionServo = 0;
delay (70);

}

#define RAON (char)'E' //14 // ordre haut niveau de déployer la rampe
#define RAOF (char)'F' //15 // ordre haut niveau de replier la rampe
#define RATR (char)'G' //16 // ordre de mettre la rampe en position de transport
#define FINISH 128
#define NOP 0
#define RATR_Position 146
#define RAOF_Position 105
#define RAON_Position 0

void loop() {
  
 recvOneByte();
 showNewData();
 

// Balayage des instructions HAUT NIVEAU 

if (device.action == RAON){//RAON  1 au clavier DEPLOIEMENT
  Serial.println("RAON");
  device.RampePositionChargement=0; // 0 non, 1 oui
  device.RampePositionDechargement=0; // 0 non, 1 oui
  device.RampePositionTransport=0; // 0 non, 1 oui
  while (positionServo !=RAON_Position ){
  if (positionServo < RAON_Position) {
    positionServo ++;}
    else {
    positionServo --;
    }
   positionnement();
  }
    
    
    device.RampePositionChargement=1;
    finish();
  }


else if (device.action == RAOF ){//RAOF  2 au clavier DECHARGEMENT
  Serial.println("RAOF");
  device.RampePositionChargement=0; // 0 non, 1 oui
  device.RampePositionDechargement=0; // 0 non, 1 oui
  device.RampePositionTransport=0; // 0 non, 1 oui
        while (positionServo !=RAOF_Position ){
  if (positionServo < RAOF_Position) {
    positionServo ++;}
    else {
    positionServo --;
    }
    positionnement();
  }
        
        device.RampePositionDechargement=1;
        finish();
       }

else if (device.action == RATR){//RATR   3 au clavier TRANSPORT
  Serial.println("RATR");
  device.RampePositionChargement=0; // 0 non, 1 oui
  device.RampePositionDechargement=0; // 0 non, 1 oui
  device.RampePositionTransport=0; // 0 non, 1 oui
       while (positionServo != RATR_Position){
  if (positionServo < RATR_Position
  ) {
    positionServo ++;}
    else {
    positionServo --;
    }
    positionnement();
  }
        
        device.RampePositionTransport=1;
        finish();
        }                       

} // fin de Loop

 

 
void finish()
{
  Serial.println("FINISH");
   device.action=device.action | FINISH;
   display();
}

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

 void positionnement() {
  Serial.println( positionServo);
   MG995.write(positionServo);
   MG946.write(positionServo);
   delay(20);
 }
 

  

