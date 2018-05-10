#include <Wire.h>
#include <SPI.h>
#include <variant.h>
#include <bootloaders/boot.h>

#if defined(BOARD_ID_Pilo)
#error not done for this one
#endif

#include "comm.h"

#include <WireUtility.h>

/*==================================================================================================*/
/*==================================================================================================*/
/*==================================================================================================*/

#include <PID_v1.h>

// called this way, it uses the default address 0x40
Zmotor3 motorBoard = Zmotor3();

#define perimetre (430/2)    /*mm*/
#define entreAxe  336 /*mm*/

#define M1_CA P_Encoder[5-1].Pin.IB  
#define M1_CB P_Encoder[5-1].Pin.IA   
#define M1_MP motorBoard.getPin(MOTOR3_P6_PWM) /*rouge-rouge*/
#define M1_MM motorBoard.getPin(MOTOR3_IO_0)  /*blanc-blanc*/
#define M1_EN motorBoard.getPin(MOTOR3_EN_0)  /*blanc*/

#define M2_CA  P_Encoder[6-1].Pin.IA  
#define M2_CB  P_Encoder[6-1].Pin.IB  
#define M2_MP motorBoard.getPin(MOTOR3_P10_PWM)  /*rouge*/
#define M2_MM  motorBoard.getPin(MOTOR3_IO_1)  /*blanc*/
#define M2_EN  motorBoard.getPin(MOTOR3_EN_1)  /*blanc*/


#define PIN_DEMAR  P_Encoder[1-1].Pin.IA  
#define PIN_CAMP  P_Encoder[1-1].Pin.IB 

//PA16   3 WireB  //sercom3
//PB22 0 5 PCOM0  //sercom0
//PC28 1 wireA   //sercom1
//PA31 1 FTDI   //sercom1

#define MySerial (P_COM0.serial)
#define  MyWireMotor WireB 

#define PcomSerial MySerial

//#define MySerial SerialFTDI  //sercom1
//#define  MyWireMotor P_COM0.wire   //sercom0

#define LED_BUILTIN LED_TOP

tdevice device;

void setupMotorBoard() {
	MyWireMotor.begin();
	MyWireMotor.setClock(100000);

	volatile int ip = scan(MySerial, MyWireMotor);
	while (ip = scanNext(MySerial, MyWireMotor) != 0)
		;

	motorBoard.begin(&MyWireMotor, 0x22, 0x42);
	motorBoard.analogWriteResolution(12);
	setPinExtender(&motorBoard); // connect the board to arduino API.
	// define pin as output

	//put defaul state : enabled low/low
	pinMode(motorBoard.getPin(MOTOR3_EN_0), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_EN_1), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_EN_2), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_EN_3), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_EN_4), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_EN_5), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_EN_6), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_EN_7), OUTPUT);

	pinMode(motorBoard.getPin(MOTOR3_IO_0), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_IO_1), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_IO_2), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_IO_3), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_IO_4), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_IO_5), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_IO_6), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_IO_7), OUTPUT);

	pinMode(motorBoard.getPin(MOTOR3_PWM_0), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_PWM_1), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_PWM_2), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_PWM_3), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_PWM_4), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_PWM_5), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_PWM_6), OUTPUT);
	pinMode(motorBoard.getPin(MOTOR3_PWM_7), OUTPUT);
//===========TEST MOTOR ==========================
	digitalWrite((M1_MM), HIGH);
	digitalWrite((M1_MP), HIGH);

	digitalWrite((M2_MM), HIGH);
	digitalWrite((M2_MP), HIGH);

	digitalWrite(M1_EN, HIGH);
	digitalWrite(M2_EN, HIGH);

}

/////////////////////////////////////////////////////////////////////////

//ZEncoder enc(A0,A2,FULL, NULL);
CMDMOTOR cmd1(M1_CA, M1_CB, M1_MP, M1_MM);
CMDMOTOR cmd2(M2_CA, M2_CB, M2_MP, M2_MM);
int count = 0;
void privateIntHandler1() {
	cmd1.getEncoder()->update();
	count++;
}
void privateIntHandler2() {
	cmd2.getEncoder()->update();
}

tcmd cmdempty = { .config = 0, .accelerator = 0, { 0, 0 }, { 0, 0 } };
void loopPIDAutoTune();
// the setup function runs once when you press reset or power the board
void setupCMD() {
	MySerial.print("setup CMD \r\n");
//  delay(500);
	cmd1.setPin(M1_CA, M1_CB, M1_MP, M1_MM); //need t do it again after board init
	cmd2.setPin(M2_CA, M2_CB, M2_MP, M2_MM);
	cmd1.setup();
	cmd2.setup();
	// delay(500);
	cmd1.getEncoder()->attachEncoderInt(privateIntHandler1);
	cmd2.getEncoder()->attachEncoderInt(privateIntHandler2);
	MySerial.print("setup CMD end \r\n");

	device.current = cmdempty;
	device.next[0] = cmdempty;
	device.next[1] = cmdempty;
	device.next[2] = cmdempty;
//  loopPIDAutoTune();
}
void setupPWM() {

}

void displayEncoder() {

	PcomSerial.print("displayCMD1()  \r");
	PcomSerial.print("CMD1 getValue=\t");
	PcomSerial.print(cmd1.getEncoder()->getValue());
	PcomSerial.print("\t, getDirection=");
	PcomSerial.print(cmd1.getEncoder()->getDirection());
	PcomSerial.print("\t, getSpeed= ");
	PcomSerial.print(cmd1.getEncoder()->getSpeed());
	//PcomSerial.print("\t, getDeltaValue=");
	// PcomSerial.print(cmd1.getEncoder()->getDeltaValue());
	PcomSerial.print("\r\n");

	PcomSerial.print("CMD2 getValue=\t");
	PcomSerial.print(cmd2.getEncoder()->getValue());
	PcomSerial.print("\t, getDirection=");
	PcomSerial.print(cmd2.getEncoder()->getDirection());
	PcomSerial.print("\t, getSpeed= ");
	PcomSerial.print(cmd2.getEncoder()->getSpeed());
	//PcomSerial.print("\t, getDeltaValue=");
	//PcomSerial.print(cmd2.getEncoder()->getDeltaValue());
	PcomSerial.print("\r\n");

}

/*
 msg rqt :dev@=H/R, @reg,  N
 msg aswr                       time, datas....

 msg rqt :dev@=H/W, @reg,  N,data[N]
 msg aswr                            ack
 */
#include <Ultrasonic.h>
double l = 0;

Ultrasonic *ultrasonic[NBHcsr04];
void setupUS() {
	for (int i = 0; i < NBHcsr04; i++)
		ultrasonic[i] = new Ultrasonic(P_US[i].Pin.TRIG, P_US[i].Pin.ECHO); // An ultrasonic sensor HC-04 (TRig,Echo)
}

void setupWheel() {
	device.wheel[0].y = 100;
	device.wheel[0].diameter = 68;
	device.wheel[0].degretoTick = 10;

	device.wheel[1].y = -100;
	device.wheel[1].diameter = 68;
	device.wheel[1].degretoTick = 10;

}
ZSharpIR *SharpIR[NBG2P];

const int SharpModel[NBG2P] = { ZSharpIR::GP2D12_24, //1
		ZSharpIR::GP2D12_24, //2
		ZSharpIR::GP2D12_24, //3
		ZSharpIR::GP2D12_24, //4
		ZSharpIR::GP2D12_24, //5
		ZSharpIR::GP2D12_24, //6
		ZSharpIR::GP2D12_24, //7
		ZSharpIR::GP2D12_24  //8
		};
/*	ZSharpIR.GP2Y0A41SK0F = 430 ;
 ZSharpIR.GP2Y0A21YK0F = 1080 ;
 ZSharpIR.GP2Y0A02YK0F = 20150 ;
 ZSharpIR.GP2Y0A710K0F = 100500 ;
 */
void setupSharp() {
	for (int i = 0; i < NBG2P; i++)
		SharpIR[i] = new ZSharpIR(P_GP2Y[i].Pin.VO, SharpModel[i]);
}

void testCmd() {

	tcmd cmd100 = { .config = 0, .accelerator = 0, { 2000, 2000 },
			{ 1000, 1000 } };
	tcmd cmdM100 = { .config = 0, .accelerator = 0, { -2000, -2000 }, { -1000,
			-1000 } };

//config;accelerator;d[NB_CMD];speed[NB_CMD];
	device.current = cmd100;
	device.next[0] = cmdM100;
	device.next[1] = cmd100;
	device.next[2] = cmdM100;
}

void setupIR() {
	for (int i = 0; i < NBIr; i++)
		pinMode(P_IR[i].Pin.IRE, OUTPUT);
	for (int i = 0; i < NBIr; i++)
		digitalWrite(P_IR[i].Pin.IRE, HIGH);
}

long starttime = 0;
void setup() {
	MySerial.begin(115200);  //115200 //9600

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)

	MySerial.println("Setup");
	setupWheel();
	setupMotorBoard();
	setupUS();
	setupIR();
	setupSharp();

	l = sqrt(
			(device.wheel[0].x - device.wheel[1].x)
					* (device.wheel[0].x - device.wheel[1].x)
					+ (device.wheel[0].y - device.wheel[1].y)
							* (device.wheel[0].y - device.wheel[1].y));

	setupCMD();

	MySerial.println("Setup End");
	starttime = micros();
	testCmd();
//	delay(2500);
}

void loopIR() {
	for (int i = 0; i < NBIr; i++) {
		digitalWrite(P_IR[i].Pin.IRE, HIGH);
		device.IR.value[i] = analogRead(P_IR[i].Pin.IRR);
		device.IR.time = micros();
		digitalWrite(P_IR[i].Pin.IRE, LOW);

	}
	device.IR.IRS = 0;
	for (int i = 0; i < NBIr; i++)
		device.IR.IRS |= ((device.IR.value[i] > device.IR.level) ? 1 : 0) << i;

	MySerial.println("Ir done");
}
volatile int Datalog[20];
void loopUS() {

	//for (int i = 0; i < NBHcsr04; i++)
          for (int i = 19; i >=0; i--)
	Datalog[i+1]=Datalog[i];
        
        {
        int i=0;
		Datalog[0]=device.Hcsr04.value[i] += ultrasonic[i]->distanceRead(CM) * 10/2;
            i=2;
		device.Hcsr04.value[i] += ultrasonic[i]->distanceRead(CM) * 10/2;
         } 
          
          for (int i = 0; i < NBHcsr04; i++)
		device.Hcsr04.value[i]/=2;
                
	device.Hcsr04.time = micros();
	/*  for(int i=0;i<NBHcsr04;i++)
	 device.Hcsr04.speed[i]=0;// not handle
	 */

//	MySerial.println("Hcsr04 done");
}
void loopSharp() {

	for (int i = 0; i < NBG2P; i++)
		device.G2P.value[i] = SharpIR[i]->distance();

	device.G2P.time = micros();
	/*   for(int i=0;i<NBG2P;i++)
	 device.G2P.speed[i]=0;// not handle*/

	MySerial.println("G2P done");
}

void loopCoder() {

	device.coder.value[0] = (signed int) device.wheel[0].diameter
			* device.wheel[0].degretoTick * cmd1.getEncoder()->getValue();
	// device.coder.dvalue[0]+=device.wheel[0].diameter*device.wheel[0].degretoTick*cmd1.getEncoder()->getDeltaValue();
	device.coder.avgspeed[0] = (signed int) device.wheel[0].diameter
			* device.wheel[0].degretoTick * cmd1.getEncoder()->getSpeed();
	device.coder.value[1] = (signed int) device.wheel[1].diameter
			* device.wheel[1].degretoTick * cmd2.getEncoder()->getValue();
	// device.coder.dvalue[1]+=device.wheel[1].diameter*device.wheel[1].degretoTick*cmd2.getEncoder()->getDeltaValue();
	device.coder.avgspeed[1] = (signed int) device.wheel[1].diameter
			* device.wheel[1].degretoTick * cmd2.getEncoder()->getSpeed();

	device.coder.time = micros();

	MySerial.println("Coder done");
}
void loopCmd() {
	/*
	 device.current.speed
	 
	 short config;
	 signed short d[NB_CMD];
	 signed short speed[NB_CMD];
	 unsigned short accelerator;// max acceleration allowed.
	 */
	//   signed int d1=device.wheel[0].diameter*device.wheel[0].degretoTick*cmd1.getEncoder()->getDeltaValue();
	signed int d1 = cmd1.getEncoder()->getDeltaValue();
	device.current.d[0] -= d1;   //reminding distance
	//   signed int  d2=device.wheel[1].diameter*device.wheel[1].degretoTick*cmd2.getEncoder()->getDeltaValue();
	signed int d2 = cmd2.getEncoder()->getDeltaValue();
	device.current.d[1] -= d2; //reminding distance

	device.coder.dvalue[0] += d1;
	device.coder.dvalue[1] += d2;

	//IS CMD FINISHED ?

	if (((device.current.speed[0] > 0) & (device.current.d[0] <= 0))
			|| ((device.current.speed[0] < 0) & (device.current.d[0] >= 0)))
		device.current.speed[0] = 0; // finished
	if (((device.current.speed[1] > 0) & (device.current.d[1] <= 0))
			|| ((device.current.speed[1] < 0) & (device.current.d[1] >= 0)))
		device.current.speed[1] = 0; // finished

	//UPDATE SPEED & PWM
	cmd1.setPoint(device.current.speed[0]);
	cmd2.setPoint(device.current.speed[1]);

	// cmd1.loop();
	// cmd2.loop();

	//NEXT CMD
	if ((device.current.speed[0] == 0) && (device.current.speed[1] == 0)) // if finished to to next
		if ((device.next[0].speed[0] != 0)
				|| (device.next[0].speed[1] != 0)  // if stil a order
				|| (device.next[1].speed[0] != 0)
				|| (device.next[1].speed[1] != 0)
				|| (device.next[2].speed[0] != 0)
				|| (device.next[2].speed[1] != 0)) {
			device.current = device.next[0];
			device.next[0] = device.next[1];
			device.next[1] = device.next[2];
			device.next[2] = cmdempty;
			loopCmd();  //update now
		}

	MySerial.println("Cmd done");
}

void gohome() {
	int d1 = cmd1.getEncoder()->getValue();
	int d2 = cmd2.getEncoder()->getValue();

	int v1 = -1000;
	int v2 = -1000;
	//retour en place
	while (!((v1 == 0) && (v2 == 0))) {
		if (d1 > 0) {
			if (cmd1.getEncoder()->getValue() < 0)
				v1 = 0;
		} else {
			if (cmd1.getEncoder()->getValue() > 0)
				v1 = 0;
		}

		if (d2 > 0) {
			if (cmd2.getEncoder()->getValue() < 0)
				v2 = 0;
		} else {
			if (cmd2.getEncoder()->getValue() > 0)
				v2 = 0;
		}

		cmd1.setPWMValue(v1);
		cmd2.setPWMValue(v2);
		delay(20);
	}
	d1 = cmd1.getEncoder()->getValue();
	d2 = cmd2.getEncoder()->getValue();

}
#define cmdD cmd1
#define cmdG cmd2
#define MM_TO_TICK(a) (int)(((double)a)/(135.0/130.0))
#define MM_S_TO_PWMD(a) (int)(((double)a)*1+100)

int couleur_distance=(1200);
int couleur_vitesse=(150);

#define US_D_ID 0
#define US_G_ID 2

#define campOrange 'O'
#define campVert 'V'
#define KEcartRoue  280

void av2(signed int d1t,signed int d2t, signed int v1t,signed int v2t);
void arc_de_cercle(int Rayon, int Angle)
{
	
	double liDistance1;
	double liDistance2;
	double liDistanceD;
	double liDistanceG;

	if (Rayon <= KEcartRoue/2) {
		while(1);
	}
#define M_PI 3.121592

	// Calculs
	liDistance1 = ((double)Angle /360.0) * M_PI;
	liDistance2 = liDistance1 * KEcartRoue / 2;
	liDistance1 = liDistance1 * (double)Rayon;
	liDistanceD = (liDistance1 + liDistance2);
	liDistanceG = (liDistance1 - liDistance2);
int Vitesse0,Vitesse1;
int KVitesseStandard=120;
	
	// Adapatation de la vitesse
	if (liDistanceD > liDistanceG) {
		Vitesse0 = KVitesseStandard;
		Vitesse1 = (int16_t)((liDistanceD / liDistanceG) * KVitesseStandard);
	}
	else {
		Vitesse0 = (int16_t)((liDistanceG / liDistanceD) * KVitesseStandard);
		Vitesse1 = KVitesseStandard;
	}



 av2(liDistanceD,liDistanceD, Vitesse0,Vitesse1);
}
void timeoutArret();  
void evitement_Arret_depart(int camp)
{
timeoutArret();
 loopUS();
 bool status=true;
 if(camp!=campOrange)
 status=(device.Hcsr04.value[US_D_ID]<100) ;
 else
 status=(device.Hcsr04.value[US_G_ID]<100);
 
if (status )
{
  int v1=cmdD.getPoint();
  int v2=cmdG.getPoint();
  cmdD.stop();
  cmdG.stop();
 // while(1);
  while (status )
  {loopUS() ;
  if(camp!=campOrange)
 status=(device.Hcsr04.value[US_D_ID]<100) ;
 else
 status=(device.Hcsr04.value[US_G_ID]<100);
 }
  cmdD.setPoint(v1);
  cmdG.setPoint(v2);
}
}

void evitement_Arret()
{
timeoutArret();

 loopUS();
if ((device.Hcsr04.value[US_D_ID]<100) ||(device.Hcsr04.value[US_G_ID]<100) )
{
  int v1=cmdD.getPoint();
  int v2=cmdG.getPoint();
  cmdD.stop();
  cmdG.stop();
 // while(1);
  while ((device.Hcsr04.value[US_D_ID]<100) ||(device.Hcsr04.value[US_G_ID]<100) )
  loopUS() ;
  cmdD.setPoint(v1);
  cmdG.setPoint(v2);
}
}
void av2(signed int d1t,signed int d2t, signed int v1t,signed int v2t)
{
	delay(300);
	int d1,d2,d1_,d2_;
	int distanceTICK=MM_TO_TICK((d1t+d2t)/2);
	int v1=MM_S_TO_PWMD(v1t);
int v2=MM_S_TO_PWMD(v2t);

	d1=d1_ = cmdD.getEncoder()->getValue();
	d2=d2_ = cmdG.getEncoder()->getValue();
// no PID :
        int time1=micros();

	cmdD.setPoint(v1);
        cmdG.setPoint( v2);
	while(abs((d1_+d2_)-(d1+d2))<distanceTICK*2)
	{
        cmdD.loop();
        cmdG.loop();
         evitement_Arret();
		d1_ = cmdD.getEncoder()->getValue();
	    d2_ = cmdG.getEncoder()->getValue();
	}
        int time2=micros();
    int volatile duree=time2-time1;
	//STOP
cmdD.stop();
	cmdG.stop();
      duree=time2-time1;
    duree=duree;


}
void av(signed int couleur_distance, signed int couleur_vitesse)
{
//	delay(300);
	int d1,d2,d1_,d2_;
	int distanceTICK=MM_TO_TICK(couleur_distance);
	int vitesseTICK=MM_S_TO_PWMD(couleur_vitesse);

	d1=d1_ = cmdD.getEncoder()->getValue();
	d2=d2_ = cmdG.getEncoder()->getValue();
// no PID :
        int time1=micros();

	cmdD.setPoint(vitesseTICK);
        cmdG.setPoint( vitesseTICK);
	while(abs((d1_+d2_)-(d1+d2))<abs(distanceTICK*2))
	{
        cmdD.loop();
        cmdG.loop();
         evitement_Arret();
		d1_ = cmdD.getEncoder()->getValue();
	    d2_ = cmdG.getEncoder()->getValue();
	}
        int time2=micros();
    int volatile duree=time2-time1;
	//STOP
cmdD.stop();
	cmdG.stop();
      duree=time2-time1;
    duree=duree;


}
void tourne(signed int degre, signed int couleur_vitesse)
{
	int d1,d2,d1_,d2_;
	int vitesseTICK=MM_S_TO_PWMD(couleur_vitesse);
	// no PID :
        int time1=micros();

d1_=d1 = cmdD.getEncoder()->getValue();
	d2_=d2 = cmdG.getEncoder()->getValue();

int vG=-vitesseTICK;
int vD=vitesseTICK;
if (degre<0)
{
  vG=vitesseTICK;
  vD=-vitesseTICK;
}

signed int distTour=(((280.0*3.14)*degre)/360)*2;

        distTour=MM_TO_TICK(distTour);
        int dist=((d1_-d1)-(d2_-d2));
               
	d1 = cmdD.getEncoder()->getValue();
	d2 = cmdG.getEncoder()->getValue();
      cmdD.setPoint(  vG);
        cmdG.setPoint( vD);
  
 	while(abs(dist)<abs(distTour))
	{
        cmdD.loop();
        cmdG.loop();
         evitement_Arret();
        dist=((d1_-d1)-(d2_-d2));
		d1_ = cmdD.getEncoder()->getValue();
	    d2_ = cmdG.getEncoder()->getValue();
	}

cmdD.stop();
cmdG.stop();
        int time2=micros();
    int volatile duree=time2-time1;

}

/**
degre -360..360
vitesseTICK>0
*/
void avtourne(signed int degre, signed int couleur_vitesse)
{
	int d1,d2,d1_,d2_;
	int vitesseTICK=MM_S_TO_PWMD(couleur_vitesse);
	// no PID :
        int time1=micros();

int vG=vitesseTICK;
int vD=0;
if (degre<0)
{
  vG=0;
  vD=vitesseTICK;
}
signed int distTour=((280*3.14*degre)/360*2);

        distTour=MM_TO_TICK(distTour);
	d1_=d1 = cmdD.getEncoder()->getValue();
	d2_=d2 = cmdG.getEncoder()->getValue();
	 int dist=((d1_-d1)-(d2_-d2));
        cmdD.setPoint(  vG);
        cmdG.setPoint( vD);
	while(abs(dist)<abs(distTour))
	{
         dist=((d1_-d1)-(d2_-d2));
	cmdD.loop();
        cmdG.loop();
       
        evitement_Arret();
		d1_ = cmdD.getEncoder()->getValue();
	    d2_ = cmdG.getEncoder()->getValue();
	}

cmdD.stop();
cmdG.stop();
}
void avtourne2(signed int degre, signed int couleur_vitesse)
{
	int d1,d2,d1_,d2_;
	int vitesseTICK=MM_S_TO_PWMD(couleur_vitesse);
	// no PID :
        int time1=micros();

int vG=vitesseTICK/2+vitesseTICK;
int vD=vitesseTICK/2;
if (degre<0)
{
  vG=0;
  vD=vitesseTICK;
}
signed int distTour=((280*3.14*degre)/360*2);

        distTour=MM_TO_TICK(distTour);
	d1_=d1 = cmdD.getEncoder()->getValue();
	d2_=d2 = cmdG.getEncoder()->getValue();
	 int dist=((d1_-d1)-(d2_-d2));
        cmdD.setPoint(  vG);
        cmdG.setPoint( vD);
	while(abs(dist)<abs(distTour))
	{
         dist=((d1_-d1)-(d2_-d2));
	cmdD.loop();
        cmdG.loop();
       
        evitement_Arret();
		d1_ = cmdD.getEncoder()->getValue();
	    d2_ = cmdG.getEncoder()->getValue();
	}

cmdD.stop();
cmdG.stop();
}
void homologation(int camp)
{

///////////////////////////////////////////////////////////////////////////
//////////////////////////// alume panneau ://///////////////////////

	int d1,d2,d1_,d2_;
	int distanceTICK=MM_TO_TICK(couleur_distance);
	int vitesseTICK=MM_S_TO_PWMD(couleur_vitesse);
	// no PID :
        int time1=micros();
	cmdD.stop();
	cmdG.stop();
	d1_=d1 = cmdD.getEncoder()->getValue();
	d2_=d2 = cmdG.getEncoder()->getValue();
//	cmdD.setPWMValue(vitesseTICK);
//        cmdG.setPWMValue( vitesseTICK);
if (camp==campOrange)
{
cmdD.setPoint(vitesseTICK+50);
        cmdG.setPoint( vitesseTICK);
        }
        else
 {
cmdD.setPoint(vitesseTICK);
        cmdG.setPoint( vitesseTICK+50);
        }       
        
        
	while(abs((d1_+d2_)-(d1+d2))<distanceTICK*2)
	{
        cmdD.loop();
        cmdG.loop();
     //   evitement_Arret_depart( camp);
		d1_ = cmdD.getEncoder()->getValue();
	    d2_ = cmdG.getEncoder()->getValue();
	}
        int time2=micros();
	//STOP
cmdD.stop();
	cmdG.stop();
    int volatile duree=time2-time1;
    duree=duree;
     
    
    
    
    
    
    
    
    
//////////////////////////////////////////////////////////////////
///// RETOUR /////////////////////////////

////////////////////////////////////////////////////////////////////////////
/////////////////// tourne
int angle=90;
    if (camp==campOrange)
    angle=-90;
   // av(-couleur_distance, -couleur_vitesse);
    ///////////////////////////////////////////////////////////////////////
    
    delay(300);
    arc_de_cercle(500/*rayon*/, -angle);
   
    avtourne2(angle, couleur_vitesse);
   
  /*  avtourne2(5, couleur_vitesse);
    av(30, couleur_vitesse);
    avtourne2(5, couleur_vitesse);
    av(30, couleur_vitesse);
    avtourne2(10, couleur_vitesse);
    av(50, couleur_vitesse);
    avtourne2(25, couleur_vitesse);
    av(50, couleur_vitesse);
    avtourne2(45, couleur_vitesse);
    
    */
  //  avtourne(angle, couleur_vitesse);delay(300);
	
        
        
        ///////////////////////////////////////////////////////////////
        av(800, couleur_vitesse);delay(300);
    tourne(-angle, couleur_vitesse);delay(300);
	 av(500, couleur_vitesse);delay(300);
    tourne(-angle, couleur_vitesse);delay(300);
	 av(800, couleur_vitesse);delay(300);
	 av(-100, -couleur_vitesse);delay(300);
	 av(-100, -couleur_vitesse);delay(300);
	 av(-100, -couleur_vitesse);delay(300);
    
        
///////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

}

void homologationtest2()
{

///////////////////////////////////////////////////////////////////////////
//////////////////////////// alume panneau ://///////////////////////

	int d1,d2,d1_,d2_;
	int distanceTICK=MM_TO_TICK(couleur_distance);
	int vitesseTICK=MM_S_TO_PWMD(couleur_vitesse);
	// no PID :
        int time1=micros();
	cmdD.stop();
	cmdG.stop();
	d1_=d1 = cmdD.getEncoder()->getValue();
	d2_=d2 = cmdG.getEncoder()->getValue();
//	cmdD.setPWMValue(vitesseTICK);
//        cmdG.setPWMValue( vitesseTICK);
cmdD.setPoint(vitesseTICK+50);
        cmdG.setPoint( vitesseTICK);
	while(abs((d1_+d2_)-(d1+d2))<distanceTICK*2)
	{
        cmdD.loop();
        cmdG.loop();
        
		d1_ = cmdD.getEncoder()->getValue();
	    d2_ = cmdG.getEncoder()->getValue();
	}
        int time2=micros();
	//STOP
cmdD.stop();
	cmdG.stop();
    int volatile duree=time2-time1;
    duree=duree;
    
    
    
    
    
    
    
    
    
//////////////////////////////////////////////////////////////////
///// RETOUR /////////////////////////////
delay(300);
cmdD.setPoint(-vitesseTICK);
        cmdG.setPoint( -vitesseTICK);
	while((d1_+d2_)>(d1+d2))
	{
        cmdD.loop();
        cmdG.loop();
        
		d1_ = cmdD.getEncoder()->getValue();
	    d2_ = cmdG.getEncoder()->getValue();
	}
         time2=micros();
	//STOP
cmdD.stop();
	cmdG.stop();
      duree=time2-time1;
    duree=duree;

////////////////////////////////////////////////////////////////////////////
/////////////////// tourne
    
///////////////////////////////////////////////////////////////////////////////////
{
	d1_=d1 = cmdD.getEncoder()->getValue();
	d2_=d2 = cmdG.getEncoder()->getValue();

int vG=vitesseTICK;
int vD=-vitesseTICK;
signed int degre=90;
if (degre<0)
{
  vG=-vitesseTICK;
  vD=vitesseTICK;
}

signed int distTour=(((280.0*3.14)*degre)/360)*2;

        distTour=MM_TO_TICK(distTour);
        int dist=((d1_-d1)-(d2_-d2));
               
	d1 = cmdD.getEncoder()->getValue();
	d2 = cmdG.getEncoder()->getValue();
      cmdD.setPoint(  vG);
        cmdG.setPoint( vD);
  
 	while(dist<(distTour))
	{
        cmdD.loop();
        cmdG.loop();
        dist=((d1_-d1)-(d2_-d2));
		d1_ = cmdD.getEncoder()->getValue();
	    d2_ = cmdG.getEncoder()->getValue();
	}

cmdD.stop();
cmdG.stop();

}

//////////////////////////////////////////////////////////////////////
{
int vG=vitesseTICK;
int vD=0;
signed int degre=-90;
if (degre<0)
{
  vG=0;
  vD=vitesseTICK;
}

signed int distTour=((280*3.14*degre)/360*2);

        distTour=MM_TO_TICK(distTour);
	d1 = cmdD.getEncoder()->getValue();
	d2 = cmdG.getEncoder()->getValue();
	 int dist=((d1_-d1)-(d2_-d2));
        cmdD.setPoint(  vG);
        cmdG.setPoint( vD);
	while(abs(dist)<abs(distTour))
	{
         dist=((d1_-d1)-(d2_-d2));
	cmdD.loop();
        cmdG.loop();
        
		d1_ = cmdD.getEncoder()->getValue();
	    d2_ = cmdG.getEncoder()->getValue();
	}

cmdD.stop();
cmdG.stop();
}
while(1);
}

void homologation_test1()
{
	int d1,d2,d1_,d2_;
	int distanceTICK=MM_TO_TICK(couleur_distance);
	int vitesseTICK=MM_S_TO_PWMD(couleur_vitesse);
	// no PID :
        int time1=micros();
	cmdD.stop();
	cmdG.stop();
	d1_=d1 = cmdD.getEncoder()->getValue();
	d2_=d2 = cmdG.getEncoder()->getValue();
//	cmdD.setPWMValue(vitesseTICK);
//        cmdG.setPWMValue( vitesseTICK);
cmdD.setPoint(vitesseTICK);
        cmdG.setPoint( vitesseTICK);
	while(abs((d1_+d2_)-(d1+d2))<distanceTICK*2)
	{
        cmdD.loop();
        cmdG.loop();
        
		d1_ = cmdD.getEncoder()->getValue();
	    d2_ = cmdG.getEncoder()->getValue();
	}
        int time2=micros();
	//STOP
cmdD.stop();
	cmdG.stop();
    int volatile duree=time2-time1;
    duree=duree;
//////////////////////////////////////////////////////////////////
///// RETOUR /////////////////////////////

cmdD.setPoint(-vitesseTICK);
        cmdG.setPoint( -vitesseTICK);
	while((d1_+d2_)>(d1+d2))
	{
        cmdD.loop();
        cmdG.loop();
        
		d1_ = cmdD.getEncoder()->getValue();
	    d2_ = cmdG.getEncoder()->getValue();
	}
         time2=micros();
	//STOP
cmdD.stop();
	cmdG.stop();
      duree=time2-time1;
    duree=duree;



while(1);
}

/** to do test case : inversion coder, inversion roue vs codeur
inversion sens roues

*/
void test()
{
	int d1,d2,d1_,d2_;
	// no PID :
	cmdD.stop();
	cmdG.stop();
	int couleur_distance=500;
	d1_=d1 = cmdD.getEncoder()->getValue();
	d2_=d2 = cmdG.getEncoder()->getValue();
	cmdD.setPWMValue(1000);
  //  cmdG.setPWMValue(3000);
	while((d1_+d2_)-(d1+d2)<couleur_distance*2)
	{
		d1_ = cmdD.getEncoder()->getValue();
	    d2_ = cmdG.getEncoder()->getValue();
	}
	//STOP
	cmdD.setPWMValue(0);
    cmdG.setPWMValue(0);
	
}
void calibratemotor() {

	for (int i = 0; i < 4096; i += 100) {
		cmd1.setPWMValue(i);
		cmd2.setPWMValue(i);
		delay(5);
	}
	delay(800);
	int Vmax = cmd1.getEncoder()->getSpeed();
	Vmax = max(Vmax, cmd2.getEncoder()->getSpeed());

	for (signed int i = 4096; i > 0; i -= 100) {
		cmd1.setPWMValue(i);
		cmd2.setPWMValue(i);
		delay(5);
	}
	int v1 = -1000;
	int v2 = -1000;
	gohome();
	double Kp = 4096.0 / Vmax;
	v1 = v2 = (Vmax * 60) / 100;
	int res = false;
	/**/

	cmd1.stop();
	cmd2.stop();
	while (1)
		;
	/**/

	for (; !res; Kp += Kp) {
		cmd2.pid->SetTunings(Kp, 0, 0, P_ON_E);  // Kp,  Ki,  Kd,  POn)
		cmd1.pid->SetTunings(Kp, 0, 0, P_ON_E);  // Kp,  Ki,  Kd,  POn)

		cmd1.setPoint(v1);
		cmd2.setPoint(v2);
		delay(300);
		int V1a = cmd1.getEncoder()->getSpeed();
		int V2a = cmd1.getEncoder()->getSpeed();
		res = (V1a > v1 * 80 / 100) && (V2a > v2 * 80 / 100);
		cmd1.stop();
		cmd2.stop();
		gohome();
	}

	double Ki = 4096.0 / Vmax / 0.01 * 0.5;  // refresh  10ms; target 500ms
	for (; (cmd1.Output < 4096) && (cmd2.Output < 4096); Kp += Kp / 10) {
		cmd2.pid->SetTunings(0, Ki, 0, P_ON_E);  // Kp,  Ki,  Kd,  POn)
		cmd1.pid->SetTunings(0, Ki, 0, P_ON_E);  // Kp,  Ki,  Kd,  POn)

		cmd1.setPoint(v1);
		cmd2.setPoint(v1);
		int time = millis();
		while ((cmd1.Output < 4096) && (cmd2.Output < 4096)
				&& ((millis() - time) < 300))
			;
		res = (cmd1.getEncoder()->getValue() > v1 * 0.95)
				&& (cmd2.getEncoder()->getValue() > v2 * 0.95);
		gohome();
	}
//Ki=Ki/3;

}
int globaltime;
void timeoutArret()
{

volatile int dure=micros()-globaltime;
dure/=1000000;
if (dure>100)
  while(1);
}
void loop() {
//AA 40
//BB 31
/*
for(int i=0;i<100;i++)
{
  pinMode(i, OUTPUT);
  digitalWrite(i, HIGH);   // turn the LED on (HIGH is the voltage level)
}
*/
 delay(300);
   
   globaltime=micros();
   pinMode(PIN_DEMAR, INPUT_PULLUP);
   pinMode(PIN_CAMP, INPUT_PULLUP);
delay(100);
if ( digitalRead(PIN_DEMAR)==HIGH) // secteur
{
//choix camp
    if (digitalRead(PIN_CAMP)==LOW)
    {
      pinMode(LED_TOP, OUTPUT);
      digitalWrite(LED_TOP, HIGH);   // turn the LED on (HIGH is the voltage level)
      pinMode(LED_BOTTOM, OUTPUT);
      digitalWrite(LED_BOTTOM, HIGH);   // turn the LED on (HIGH is the voltage level)
    //  delay(100);
    }
    else
    {
      pinMode(LED_TOP, OUTPUT);
      digitalWrite(LED_TOP, LOW);   // turn the LED on (HIGH is the voltage level)
      pinMode(LED_BOTTOM, OUTPUT);
      digitalWrite(LED_BOTTOM, LOW);   // turn the LED on (HIGH is the voltage level)
    //  delay(100); 
    }

}
else //BATERRY
{
// attente tirette
    while( digitalRead(PIN_DEMAR)==LOW);
    delay(100);
    while( digitalRead(PIN_DEMAR)==HIGH)
    {
//choix camp
    if (digitalRead(PIN_CAMP)==LOW)
    {
      pinMode(LED_TOP, OUTPUT);
      digitalWrite(LED_TOP, HIGH);   // turn the LED on (HIGH is the voltage level)
      pinMode(LED_BOTTOM, OUTPUT);
      digitalWrite(LED_BOTTOM, HIGH);   // turn the LED on (HIGH is the voltage level)
    //  delay(100);
    }
    else
    {
      pinMode(LED_TOP, OUTPUT);
      digitalWrite(LED_TOP, LOW);   // turn the LED on (HIGH is the voltage level)
      pinMode(LED_BOTTOM, OUTPUT);
      digitalWrite(LED_BOTTOM, LOW);   // turn the LED on (HIGH is the voltage level)
    //  delay(100); 
    }

    }
}

if (digitalRead(PIN_CAMP)==LOW)
{
  homologation(campVert);
}
else
{
  homologation(campOrange);

}
volatile int dure=micros()-globaltime;
dure=dure/1000000;
while(1);
/*
	calibratemotor();*/

//MySerial.println("loop");

	// IR
	//   loopIR();

	volatile double result = 0;
        while(1)
	loopUS();
	/*
	 while(1)
	 result=5.0*analogRead(GP2Y3)/1024;*/
	/*
	 while(1){*/
	loopSharp();
	/*
	 result=device.G2P.value[2];
	 }*/
	loopCoder();
	loopCmd();

	MySerial.print("US : ");
	for (int i = 0; i < NBHcsr04; i++) {
		MySerial.print(device.Hcsr04.value[i]);
		MySerial.print(", ");
	}
	MySerial.println(" mm");

	MySerial.print("G2P : ");
	for (int i = 0; i < NBG2P; i++) {
		MySerial.print(device.G2P.value[i]);
		MySerial.print(", ");
	}
	MySerial.println(" mm");

	///coder
	/*
	 // odometry
	 //http://manubatbat.free.fr/doc/positionning/node5.html
	 #if 1==1 ///NB_WHEEL==2


	 //init values
	 double xnold=odometry.x;
	 double ynold=odometry.y;
	 double thetanold=odometry.theta;

	 // compute
	 double xn=0;
	 double yn=0;
	 double thetan=0;
	 double coderd=device.coder.dvalue[0];
	 double coderg=device.coder.dvalue[1];
	 device.coder.dvalue[0]=0;
	 device.coder.dvalue[1]=0;
	 double ddn=coderd*device.wheel[0].degretoTick/180*3.141592*device.*wheel[0].diameter/2;
	 double dgn=coderg*device.wheel[1].degretoTick/180*3.141592*device.wheel[1].diameter/2;
	 double dmoyn=(ddn+dgn)/2;
	 double ddifn=(ddn-dgn)/2;
	 double dxn=dmoyn*cos(theta);
	 double dyn=dmoyn*sin(thetanold);
	 double dthetan=ddifn/L;
	 xn=xnold+dxn;
	 yn=ynold+dyn;
	 thetan=dthetan+thetanold;

	 //write values
	 odometry.x=xn;
	 odometry.y=yn;
	 odometry.theta=thetan;
	 odometry.time=coder.time;

	 #else
	 #error not supported
	 #endif
	 */
//delay(500);
	/// command

	// displayEncoder();

	MySerial.print("time �s: ");
	MySerial.println(micros());

//    delay(1000);//wait the flush serial on the line  before re boost
	if (((micros() - starttime) >> 20) > 95) /// after 100s
		jumpInBoot(); // I finish, I return on boot for next sketch. to run me again reset.  

}

//#define Serial Myserial
#define Serial1 MySerial

void dummy() {

	Serial1.begin(9600);
	if (Serial1.available()) {
		int inByte = Serial1.read();
		Serial.write(inByte);
	}
}

void loopPIDAutoTune() {
	while (1) {
		cmd1.loop();
		cmd2.loop();
//delay(50);
	}
}
#ifdef __cplusplus
extern "C" {
#endif

int sysTickHook(void) {
	if (millis() % 10) {
		cmd1.loop();
		cmd2.loop();
	}

	return false; // keep systick implementation of delay
}
#ifdef __cplusplus
}
#endif
