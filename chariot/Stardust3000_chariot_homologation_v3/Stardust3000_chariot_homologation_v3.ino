
/*Stardust3000v2
 * Brochage de la carte Arduino Uno
 * Entrées sortie digitales en PWM :
 * 5 détection de passage du chariot sur fin de course optique, situé vers la position de chargement , idéalement en position 0
 * 6 Pwm vers Enable du L298N commandant le pôle B du moteur pas à pas NEMA17
 * 7 Pwm vers Enable du L298N commandant le pôle A du moteur pas à pas NEMA17
 * 
 * Entrées sorties digitales
 * 2 signal de fin de course entrée sortie de chariot
 * 3 commande vers L98N commandant marche arrière dans le chariot
 * 4 commande vers L98N commandant marche avant dans le chariot
 * 
 * 8 commande vers L98N commandant le pôle B du moteur pas à pas, fil bleu
 * 9 commande vers L98N commandant le pôle B du moteur pas à pas, fil jaune
 * 10 commande vers L98N commandant le pôle A du moteur pas à pas, fil vert
 * 11 commande vers L98N commandant le pôle A du moteur pas à pas, fil rouge
 * 
 * Emplacements du triage de cubes :
 * 2 tapis  : TP tapis principal monté sur chariot
 *            TA tapis de stockage , le plus à gauche, les colonnes de descente vers soi et les rouleaux de collecte loin de soi

 *   
 *  4 colonnes de descente :  DA , la plus à gauche, les colonnes de descente vers soi et les rouleaux de collecte loin de soi
 *                            DB , la deuxième en partant de la gauche
 *                            DC , la troisième en partant de la gauche
 *                            DD , la quatrième en partant de la gauche, donc la plus à droite
 *  
 *  1 emplacement de chargement du chariot : CH , l'endroit où les cubes arrivent en haut de la colonne de rammassage
 *  
 *  Instruction de mouvement : une seule variable : instMouv
 *  NOPE : aucune instruction, mise en maintien dans le tapis principal
 *  TPTA : Tapis principal vers tapis de stockage A
 *  TPDE : Tapis principal vers colonne de descente 
 *  TATP : Tapis de stockage A vers tapis principal
 *  CHTP : Arrivage du nouveau cube vers tapis principal
 *  
 *  Instruction de mouvement du chariot
 *  MVTA : Mouvement vers tapis A
 *  MVDA : Mouvement vers colonne de descente A
 *  MVDB : Mouvement vers colonne de descente B
 *  MVDC : Mouvement vers colonne de descente C
 *  MVDD : Mouvement vers colonne de descente D
 *  MVCH : Mouvement vers l'arrivage de nouveau cube
 *  
 *  Instructions de maintien : 4 variables pouvant avoir la valeur 1 ou 0 ; instMaintA,instMaintB, instMaintC, instMaintP
 *  1 signifiant maintien dans la zone concernée , 0 signifiant aucune opération dans la zone concernée
 *  
 *                            
 *                            
 * 
 * 
 */

#include <AccelStepper.h>
#include "PinChangeInterrupt.h"
AccelStepper stepper(4, 8, 9, 10, 11); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5

const byte nbCaracteres = 12; // taille du tableau recevant les données série
char reception[nbCaracteres]; // création d'un tableau de nbCaracteres caractères recevant les données série
boolean nouvelleReception = false; //drapeau indiquant la présence de caractère dans le buffer série
boolean deplacementEnCours = false; //drapeau indiquant que le chariot est en cours de déplacement

//Ports de commande enables des bobine de NEMA 17
const int EnableNEMA17A = 6;
const int EnableNEMA17B = 7;

//Ports de commande du moteur N20 d'entrée sortie dans le chariot
const int N20avant = 4;
const int N20arriere = 3;
const int N20PWM = 12;
const int SDA_=A4;//Blanc
const int SCL_=A5;//bleu
const int initialisationChariot = 5;
const int FinDeCourseA = 255; // encore non opérationnel
const int FinDeCourseP = 2; // chariot vers ramp
const int FinDeCoursePavant = A0; // chariot vers colonne
const char sMVCH[5]="MVCH";
int i = 0; // variable utilitaire
// initialisation des variables d'instructions, de mouvement et de maintien.
String instMouv = "NOPE"; // instruction de mouvement
String mouvementDemandeNema17 = "";
int instMaintA = 0; // instruction de maintien
int instMaintP = 0;
int positionCourante = 0; // Position courante du chariot
int objectif = 0; // Position à atteindre pour le chariot
int decompteArretEnableNEMA17 = -1; // s'il est négatif, il n'a pas d'incidence
int decompteMarcheN20 = 0;
int initialisationDemande = 1;

// position des lieux de chargement/déchargement
const int CH = 0; // arrivage des nouveaux cubes: chargement
const int TA = 400; // tapis A : reserve
const int DA = 480; // descente A :colonne
const int DB = 146;//96; // descente B :colonne
const int DC = -186;//-216; // descente C :colonne
const int DD = -530; // descente D :colonne
//<CHDA><CHDB><CHDC><CHDD>

//##################### MEM ############################

typedef  union
{
  struct  {
   char action; // MONO , MONF , POUS, |FINISH, NOP 
   int16_t position_chariot; // CH,TA,DA,DB,DC,DD
   char presence_cube;
 // courant ? 
 
  } ;
uint8_t mem[20];
} collonne_t;

#define NOPE  '0'    
#define MVCH  '4'       //4  Retour du chariot en position de chargement
#define CHDA  '5' //5  Déchargement du cube en descente A
#define CHDB  '6' //6  Déchargement du cube en descente B
#define CHDC  '7' //,7  Déchargement du cube en descente C
#define CHDD  '8' //,8  Déchargement du cube en descente D
#define MVRA  '9' //,9  Déchargement du cube en réserve A
#define RADA  'A' //,10 Chargement du cube à partir de la réserve A et déchargement en descente A
#define RADB  'B' //,11 Chargement du cube à partir de la réserve A et déchargement en descente B
#define RABC  'C' //,12 Chargement du cube à partir de la réserve A et déchargement en descente C
#define RADD  'D' //,13 Chargement du cube à partir de la réserve A et déchargement en descente D

#define MVDA  'a' //5  deplacement du cube en descente A
#define MVDB  'b' //6  deplacement du cube en descente B
#define MVDC  'c' //,7  deplacement du cube en descente C
#define MVDD  'd' //,8  deplacement du cube en descente D
#define MVTA  'e' //,8  deplacement du cube en reseve A
#define TATP 'f' //, // tapis A vers tapis principal
#define CHTP 'n' // chargement nouveau cube 
#define TPTA 'r' // tapis principal vers tapis A : reserve
#define TPDE 'v' // tapis principal vers descente

#define FINISH 128
#define NOP 0
collonne_t device;//variable colone pour utiliser

void finish()
{
   device.action=device.action | FINISH;
   display();
}
//##################### MEM ############################

//##################### USART ############################


void receptionSerial() { // -----------------------------------------------------------------------------------------------------------------
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && nouvelleReception == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        reception[ndx] = rc;
        ndx++;
        if (ndx >= nbCaracteres) {
          ndx = nbCaracteres - 1;
        }
      } else {
        reception[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        nouvelleReception = true;
        Serial.print("recp=");
        Serial.println(reception);
        rc = 0;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }



}








void showNewData() {

  if (nouvelleReception == true) {
    Serial.print("reçu : ");
    Serial.println(reception);
    instMouv = reception;


    
 if (instMouv == "CHTP") {device.action= CHTP;} // chargement nouveau cube 
  else if (instMouv == "TPTA") {device.action= TPTA;} // tapis principal vers tapis A : reserve
  else if (instMouv == "TPDE") {device.action= TPDE;} // tapis principal vers colonne descente 
  else if (instMouv == "TATP") {device.action= TATP;} // tapis A vers tapis principal
  else if (instMouv == "NOPE") {device.action= NOPE;}     
  else if (instMouv == "MVTA" ) // MVTA : Mouvement chariot vers tapis A : reserve
     {device.action= MVTA;}  
     else if (instMouv == "MVDA" ) // MVDA : Mouvement chariot vers descente A
      {device.action= MVDA;}
      else if (instMouv == "MVDB" ) // MVDB : Mouvement chariot vers descente B
       {device.action= MVDB;}
       else if (instMouv == "MVDC" ) // MVDC : Mouvement chariot vers descente C
       {device.action= MVDC;}
       else if (instMouv == "MVDD") // MVDD : Mouvement chariot vers descente D
       {device.action= MVDD;}
      else if (instMouv == "CHDA" ) // Déchargement du cube en descente A
      {device.action= CHDA;}
      else if (instMouv == "CHDB" ) // Déchargement du cube en descente B
       {device.action= CHDB;}
       else if (instMouv == "CHDC" ) // Déchargement du cube en descente C
       {device.action= CHDC;}
       else if (instMouv == "CHDD") // Déchargement du cube en descente D
       {device.action= CHDD;}
       else if (instMouv == sMVCH ) // MVCH : Mouvement chariot vers l'arrivage de nouveaux cubes : chargement
       {device.action= MVCH;
       }

     
    nouvelleReception = false;
  }
  /*else {
    instMouv = "NOPE"; // si la commande n'est pas connue, on s'assure du maintien du cube dans le tapis principal
    device.action= NOPE;
  }*/
  

 display();
 
}




//##################### USART ############################
//##################### WIRE ############################
#include <Wire.h>
#include <WireUtility.h>
#include <ZWireDevice.h>

#define DeviceAddress 0x16
#define MyWirePc Wire

ZWireDevice slave = ZWireDevice();
void SlaveDevice_requestEvent()
{
   Serial.println("Sd rEv");
  slave.requestEvent();
  display();
}
void SlaveDevice_receiveEvent(int iData)
{
  
 Serial.println("Sd rec");
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

void display()
{/*
 Serial.println("device:"); 
  Serial.print("\tdevice.RampePositionChargement = 0x"); Serial.println(device.RampePositionChargement,HEX); 
  Serial.print("\tdevice.RampePositionDechargement = 0x"); Serial.println(device.RampePositionDechargement,HEX); 
  Serial.print("\tdevice.RampePositionTransport = 0x"); Serial.println(device.RampePositionTransport,HEX); 
  Serial.print("\tdevice.action = 0x"); Serial.println(device.action,HEX);*/
 //  delay(100);
}
void setup() {
  
  nouvelleReception = false; //drapeau indiquant la présence de caractère dans le buffer série
  deplacementEnCours = false; //drapeau indiquant que le chariot est en cours de déplacement
  //Initialisation du port série
  Serial.begin(115200);
  Serial.println("reset");
  delay(1000);
  SlaveDevice_setup();
  // Configuration du NEMA17
  pinMode(EnableNEMA17A, OUTPUT);
  pinMode(EnableNEMA17B, OUTPUT);
  stepper.setMaxSpeed(1000);//1000
  stepper.setAcceleration(1000);//1000
  stepper.setCurrentPosition(0);
  // Configuration des ports du moteur N20
  pinMode(N20arriere, OUTPUT);
  pinMode(N20avant, OUTPUT);
  pinMode(FinDeCourseP, INPUT_PULLUP);
  pinMode(FinDeCoursePavant, INPUT_PULLUP);
  //pinMode(FinDeCourseA, INPUT_PULLUP);// encore non opérationnel
  pinMode(initialisationChariot, INPUT_PULLUP);  // 0 chariot
  decompteArretEnableNEMA17 = -1;
  decompteMarcheN20 = 0;

// do zero
 // PCintPort::attachInterrupt(initialisationChariot, doZeroNEMA,FALLING); // attach a PinChange Interrupt to our pin on the rising edge
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(initialisationChariot), doZeroNEMA, FALLING);
    initialisationDemande = 1;
    objectif = DC;
    assignationObjectif();
    objectif = DC;
    assignationObjectif();
    objectif = DD;
    assignationObjectif();
    initialisationDemande = 1;
    objectif = DA;
    assignationObjectif();
    objectif = DA;
    assignationObjectif();
    objectif = CH;
    assignationObjectif();
  Serial.println("chariot ok");
  Serial.print("I2C addr=");
Serial.println(DeviceAddress);

//Serial.println("Help");
//Serial.println("old : <CHTP><MVDD> <MVDA><MVDD> <MVDA><MVDD> <MVDA><MVDD> <MVDA><MVCH><TPDE>");
//Serial.println("new : <CHDA><CHDB><CHDC><CHDD><MVCH>");
//<MVDA><MVDB><MVDC><MVDD>
//
  stepper.setMaxSpeed(1500);//1000
  stepper.setAcceleration(1500);//1000<MVDA><MVDD>
  device.action=NOPE;
   arretEnableNEMA17now();
}
char vfdcpa=-1 ;
char vfdcp=-1;

void loop() {
  
//Serial.println (digitalRead(initialisationChariot)); // pour contrôler le fonctionnement du capteur de position de chariot. Il renvoie la valeur 0 quand le chariot est en position 0
  receptionSerial();
  showNewData();

  homologation();
//  <MVCH><CHTP><MVDD><TPDE><MVTA>
//<CHTP><MVDD> <MVDA><MVDD> <MVDA><MVDD> <MVDA><MVDD> <MVDA><MVCH><TPDE>
//<MVDA><MVDB><MVDC><MVDD>
//<CHDA><CHDB><CHDC><CHDD>

// Balayage des instructions 
  if (device.action == CHTP) {chargementNouveauCube ();finish();} // chargement nouveau cube 
  else if (device.action == TPTA) {finish();} // tapis principal vers tapis A : reserve
  else if (device.action == TPDE) {tapisPrincipalVersDescente ();finish();} // tapis principal vers colonne descente 
  else if (device.action == TATP) {finish();} // tapis A vers tapis principal
  else if (device.action == NOPE){finish();  } 
  
  else if (device.action == MVCH)
  { 
    objectif =CH;
    assignationObjectif();
    mouvementDemandeNema17 = sMVCH;
    chargementNouveauCube ();
    finish();  
    }  
  else if (device.action == CHDA)
  {
    if(objectif !=CH)
    {
    objectif =CH;
    assignationObjectif();
    mouvementDemandeNema17 = sMVCH;
    arretEnableNEMA17now() ;
    }
    chargementNouveauCube ();
    objectif = DA;
    assignationObjectif();
    mouvementDemandeNema17 = "MVDA";
    arretEnableNEMA17now() ;
    tapisPrincipalVersDescente (); 
    objectif =CH;
    assignationObjectif();
    mouvementDemandeNema17 = sMVCH; 
    arretEnableNEMA17now();  
    finish();  
    }  
  else if (device.action == CHDB)
 {
  if(objectif !=CH)
    {
    objectif =CH;
    assignationObjectif();
    mouvementDemandeNema17 = sMVCH;
    arretEnableNEMA17now() ;
    }
    chargementNouveauCube ();
    objectif = DB;
    assignationObjectif();
    mouvementDemandeNema17 = "MVDB"; 
    arretEnableNEMA17now(); 
    tapisPrincipalVersDescente (); 
    objectif =CH;
    assignationObjectif();
    mouvementDemandeNema17 = sMVCH; 
    arretEnableNEMA17now(); 
    finish();  
    }
  else if (device.action == CHDC)
 {
  if(objectif !=CH)
    {
    objectif =CH;
    assignationObjectif();
    mouvementDemandeNema17 = sMVCH;
    arretEnableNEMA17now() ;
    }
    chargementNouveauCube ();
    objectif = DC;
    assignationObjectif();
    mouvementDemandeNema17 = "MVDC";  
    arretEnableNEMA17now() ;
    tapisPrincipalVersDescente ();  
    objectif =CH;
    assignationObjectif();
    mouvementDemandeNema17 = sMVCH;
    arretEnableNEMA17now();
    finish();  
    }  
  else if (device.action == CHDD)
 {
  if(objectif !=CH)
    {
    objectif =CH;
    assignationObjectif();
    mouvementDemandeNema17 = sMVCH;
    arretEnableNEMA17now() ;
    }
    chargementNouveauCube ();
    objectif = DD;
    assignationObjectif();
    mouvementDemandeNema17 = "MVDD";  
    arretEnableNEMA17now();
    tapisPrincipalVersDescente ();  
    objectif =CH;
    assignationObjectif();
    mouvementDemandeNema17 = sMVCH;
    arretEnableNEMA17now();
    finish();  
    } 
   
  else
  if ((device.action == MVTA) && (deplacementEnCours == false)) // MVTA : Mouvement chariot vers tapis A : reserve
      {
    objectif = TA;
    assignationObjectif();
    mouvementDemandeNema17 = "MVTA";
    finish();
  } else if ((device.action == MVDA) && (deplacementEnCours == false)) // MVDA : Mouvement chariot vers descente A
      {
    objectif = DA;
    assignationObjectif();
    mouvementDemandeNema17 = "MVDA";
    finish();
  } else if ((device.action == MVDB) && (deplacementEnCours == false)) // MVDB : Mouvement chariot vers descente B
      {
    objectif = DB;
    assignationObjectif();
    mouvementDemandeNema17 = "MVDB";
    finish();
  } else if ((device.action == MVDC) && (deplacementEnCours == false)) // MVDC : Mouvement chariot vers descente C
      {
    objectif = DC;
    assignationObjectif();
    mouvementDemandeNema17 = "MVDC";
    finish();
  } else if ((device.action == MVDD) && (deplacementEnCours == false)) // MVDD : Mouvement chariot vers descente D
      {
    objectif = DD;
    assignationObjectif();
    mouvementDemandeNema17 = "MVDD";
    finish();
  } else if ((device.action == MVCH) && (deplacementEnCours == false)) // MVCH : Mouvement chariot vers l'arrivage de nouveaux cubes : chargement
      {
  //  objectif = CH - 50 + (100 * (stepper.currentPosition() < 0)); //on fixe un objectif de 50 pas au delà de 0 pour être sûr que l'on atteindra la réinitialisation
  //  initialisationDemande = 1;
  //  assignationObjectif();
    objectif =CH;
    assignationObjectif();
    mouvementDemandeNema17 = sMVCH;
    finish();
  }

  //Accomplissement des actions en cours
  commandeMaintiens(); // Commande des maintiens sur tapis
  commandeMouvementNEMA17(); // Commande du Nema 17 tant que le drapeau déplacement en cours est true,les commandes de mouvement de chariot ne seront pas reconnues
  arretEnableNEMA17();

//displaycapteur();

} // fin de Loop
void displaycapteur()
{
  
  //if (vfdcpa!=digitalRead(FinDeCoursePavant))
{
 Serial.print(   "FinDeCoursePavant=");
 Serial.print(   digitalRead(FinDeCoursePavant));
 
      }
//      if (vfdcp!=digitalRead(FinDeCourseP))
{
 Serial.print(   "FinDeCourseP=");
 Serial.println(   digitalRead(FinDeCourseP));
 
      }
vfdcpa=digitalRead(FinDeCoursePavant) ;
vfdcp=digitalRead(FinDeCourseP);

}
void homologation()
{
   Serial.print(   "device.action=");
    Serial.println(  device.action,HEX);
  if ((char)(device.action)==(char)NOPE)
  device.action=CHDA;
  else
  if (FINISH==(device.action & FINISH)) //finie
  {
    Serial.println(   "finished");
    device.action &=~FINISH;
    
    if (device.action==CHDD)
    device.action=CHDA;
    else
    {
    device.action++;
    }
    }
   Serial.print(   "device.action=");
    Serial.println(  device.action,HEX);
}

void commandeMaintiens() { // balayage des variables insMaint et actualisation de la rotation des moteurs concernés ---------------------
  if (instMaintP == 1) {
    analogWrite(N20PWM, 255);

    if (digitalRead(FinDeCoursePavant) & !digitalRead(FinDeCourseP)) { // pas de détection sur le fin de course avant ni arrière , commande de marche arrière.
      digitalWrite(N20arriere, LOW);
      digitalWrite(N20avant, HIGH);
    } else if (digitalRead(FinDeCoursePavant) & digitalRead(FinDeCourseP)) { // pas de détection sur le fin de course avant mais détection sur le fin de course arrière , commande de marche avant.
      digitalWrite(N20arriere, HIGH);
      digitalWrite(N20avant, LOW);
    } else if (!digitalRead(FinDeCoursePavant)
        & !digitalRead(FinDeCourseP)) { // détection sur le fin de course avant mais pas arrière , commande de marche arrière.
      digitalWrite(N20arriere, LOW);
      digitalWrite(N20avant, HIGH);
    } else if (!digitalRead(FinDeCoursePavant)
        & digitalRead(FinDeCourseP)) { // détection sur le fin de course avant et arrière , commande d'arrêt
      digitalWrite(N20arriere, LOW);
      digitalWrite(N20avant, LOW);
    }
  }

}
void tapisVersArriere()
{
      analogWrite(N20PWM, 255);//pousse a l'arriere
    digitalWrite(N20avant, LOW);//
    digitalWrite(N20arriere, HIGH);//
}
void tapisStop()
{
      analogWrite(N20PWM, 255);//pousse a l'arriere
    digitalWrite(N20avant, LOW);//
    digitalWrite(N20arriere, HIGH);//
}
void chargementNouveauCube() { // Chargement d'un nouveau cube sur le tapis principal CHTP  ----------------------------------------------
  Serial.println(
      "Ordre reçu : <CHTP>");//  Chargement en cours du nouveau cube vers tapis principal 
displaycapteur();
tapisVersArriere();
  while (digitalRead(FinDeCourseP) == HIGH) {
    analogWrite(N20PWM, 255);
    digitalWrite(N20avant, LOW);//vers l'avant
    digitalWrite(N20arriere, HIGH);
  //  displaycapteur();
  arretEnableNEMA17();
  Serial.println("charge");
 // delay(10000);
  }
  
  analogWrite(N20PWM, 255);
  digitalWrite(N20avant, LOW);
  digitalWrite(N20arriere, LOW);
  instMaintP = 1;
  displaycapteur();
  Serial.println("attente cube");
//  delay(10000);
  bool value=!((!digitalRead(FinDeCoursePavant))
        && digitalRead(FinDeCourseP));
  while(value)
        {
      
  Serial.println("Maintiens");
         arretEnableNEMA17();
         commandeMaintiens();
  //    delay(10000);      
    
        value=!((!digitalRead(FinDeCoursePavant))
        && digitalRead(FinDeCourseP));
        
          }
          displaycapteur();
          delay(300);//wait servo


        tapisVersArriere();
  delay(300);//wait servo
        tapisStop();

  Serial.println(
      "<CHTP&>");//Chargement du nouveau cube vers tapis principal terminé , ordre retourné :
//  Serial.println("Mise en maintien");
}

void tapisPrincipalVersDescente() // tapis principal vers colonne descente   -----------------------------------------------------------------
{
  
  Serial.println(
      "Ordre reçu : <TPDE>");//Déchargement en cours du tapis principal dans la colonne de descente  
  displaycapteur();
  if((digitalRead(FinDeCourseP) == LOW) && (digitalRead(FinDeCoursePavant) == HIGH) )
  {
    Serial.println(
      "/!\ pa de cube!");
    return;
    }
  while (digitalRead(FinDeCourseP) == LOW) // tant que le capteur ne détecte pas la présence du cube, on l'envoie vers l'arrière

  {
    analogWrite(N20PWM, 255);
    digitalWrite(N20avant, HIGH);
    digitalWrite(N20arriere, LOW);

  }displaycapteur();
  while (digitalRead(FinDeCourseP) == HIGH) // tant que le capteur détecte la présence du cube, on l'envoie vers l'arrière
  {
    analogWrite(N20PWM, 255);
    digitalWrite(N20avant, HIGH);
    digitalWrite(N20arriere, LOW);
  }displaycapteur();
  analogWrite(N20PWM, 255);
  digitalWrite(N20avant, LOW);
  digitalWrite(N20arriere, LOW);
  instMaintP = 0;
  Serial.println(
      "<TPDE&> done");//Déchargement du tapis principal dans la descente terminé , ordre retourné , arrêt du maintien en chariot

}
void doZeroNEMA()
{
    Serial.println("doZeroNEMA");
  if ((digitalRead(initialisationChariot) == 0)
        && (initialisationDemande == 1)) {
      stepper.setCurrentPosition(0);
      //stepper.moveTo(objectif);
      Serial.println("initialisation");
      initialisationDemande = 0;
    }
  }
void commandeMouvementNEMA17() {
  if (stepper.distanceToGo() != 0) {
    stepper.run();
    if (digitalRead(initialisationChariot) == 0
        && initialisationDemande == 1) {
      stepper.setCurrentPosition(0);
      Serial.print("initialisation");
    }
  } else {
    if (deplacementEnCours == true) {
      Serial.print("Objectif ");
      Serial.print(stepper.currentPosition());
      Serial.println(
          " atteint, ordre retourné : <" + mouvementDemandeNema17
              + "&>");
      deplacementEnCours = false;
    //  initialisationDemande = 0;
      decompteArretEnableNEMA17 = 500;
    }
  }
}

void nouvelleInfo() {
  // cette fonction doit être appelée à chaque modification du plan mémoire
}

void assignationObjectif() {
  digitalWrite(EnableNEMA17A, HIGH);
  digitalWrite(EnableNEMA17B, HIGH);
 // stepper.runToNewPosition(objectif);//moveTo

 stepper.moveTo(objectif);
 commandeMaintiens();
 while(stepper.run())
 { 
  commandeMaintiens();
 }
  decompteArretEnableNEMA17 = -1;
  deplacementEnCours = true;

}

void arretEnableNEMA17() {
  //Serial.print ("decomptearret en entree");
  //Serial.print (decompteArretEnableNEMA17);                 
  if (decompteArretEnableNEMA17 > 0) {
    decompteArretEnableNEMA17--;
  } else {
    if (decompteArretEnableNEMA17 == 0) {
      digitalWrite(EnableNEMA17A, LOW);
      digitalWrite(EnableNEMA17B, LOW);
      Serial.println("Arrêt enable");
      decompteArretEnableNEMA17 = -1; // si il est négatif, il n'a plus d'incidence
    }
  }
}
void arretEnableNEMA17now() {
  //Serial.print ("decomptearret en entree");
  //Serial.print (decompteArretEnableNEMA17);                 
 /* {
      delay(100);
      digitalWrite(EnableNEMA17A, LOW);
      digitalWrite(EnableNEMA17B, LOW);
      Serial.println("Arrêt enable");
      decompteArretEnableNEMA17 = -1; // si il est négatif, il n'a plus d'incidence
    }*/
    while(decompteArretEnableNEMA17 >= 0)
  arretEnableNEMA17();
}

