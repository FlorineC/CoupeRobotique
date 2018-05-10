
#include <stdio.h>
#include <stdlib.h>

#include <Zmotor2.h>
#include <Zmotor3.h>
#include <ZEncoder.h>
#include <ZCmdMotor.h>
#include <ZSharpIR.h>

/*
https://www.youtube.com/watch?v=lujgeInpejY
http://wiki.ros.org/rosjava
https://github.com/rosjava/rosjava_core
http://rosjava.github.io/rosjava_core/0.1.6/getting_started.html

http://www.gurucoding.com/en/raspberry_pi_eclipse/raspberry_pi_cross_compilation_in_eclipse.php
http://www.raspberry-projects.com/pi/programming-in-c/compilers-and-ides/eclipse/create-new-eclipse-project-for-the-rpi

https://playground.arduino.cc/Code/Eclipse

https://www.materiel.net/pc-portable/acer-aspire-a315-21-97ja-144195.html

http://eclipse.baeyens.it/
*/

#define NBHcsr04 6
#define NBCoder 2
#define NBG2P 8
#define NBIr 8
#define NBPwm 8
#define NB_WHEEL 2
#define NB_CMD 2
typedef struct  {
    short config;
    unsigned short accelerator;// max acceleration allowed.
    signed short d[NB_CMD];
    signed short speed[NB_CMD];
} tcmd;


typedef struct {
     double x;// in mm
    double y;// in mm
     double theta;// in degre
     double diameter;// in mm
     double degretoTick;// in degre ° per tick
} tWheel;

typedef  union
{
  struct {
    struct {//@ : 0
        unsigned int time;// last update time of internal clock
        signed short  dvalue[NBCoder]; // delta of tick since last read
   //     signed short  dtime[NBCoder]; // delta time between the 2 last tick
        signed short  avgspeed[NBCoder]; // dcoder/delta time
        signed int  value[NBCoder];// absolute value
    } coder;//@ : 4+10*NBCoder

    struct {
        unsigned int time;// last update time of internal clock
        signed short  value[NBHcsr04];// distance in mm
  //      signed short  speed[NBHcsr04];// delta(d)/dt : mm/s
    } Hcsr04;
    struct {
        unsigned int time;// last update time of internal clock
        signed short  value[NBG2P];//distance in mm
  //      signed short  speed[NBG2P];// delta(d)/dt : mm/s
    } G2P;
    struct {
        signed short level;// level used to identify 0/1
        unsigned int time;// last update time of internal clock

        unsigned short  IRS;//status ON:1/OFF:0, bit information algo is value>level

        signed short  value[NBIr];//ligth ADC result
        signed short  dvalue[NBIr];// delta IR ON/OFF
    } IR;
    struct {
        unsigned int time;// last update time of internal clock
        signed short  value[NBPwm]; /// >0 : H & PWM or <0 : L & /PWM
        signed short  max; //max PWM value allowed.
    } PWM;

    tcmd current; // write on , will fush the pipline and overwrite the current command. if empty , read at 0
    tcmd next[3];  // write on , will put the command on the pipepline at good index
/*
    struct {
        unsigned int time;// last update time of internal clock
        signed double x;// in mm
        signed double y;// in mm
        signed double theta;// in degre
        wheel  wheel[NB_WHEEL];// wheel coordinate
    }    odometry;
*/
  tWheel  wheel[NB_WHEEL];// wheel coordinate

    struct {
        union {
            unsigned int value;
            struct {
                unsigned int odometryext:1 ;//1 : odometry info from coder  send outside.
                unsigned int Hcsr04on:1;// enable HC SR04 and consumption
                unsigned int G2Pon:1;// enable G2P and consumption
                unsigned int IRon:1;// enable IR sensor and consumption
                unsigned int PWMon:1;// enable PWM and consumption
                unsigned int cmdon:1 ;//1 : asservissement on.
                unsigned int odometry:1 ;//1 : internal odometry on, it consume the odometryext info.

                unsigned int empty:(32-7);
            };
        } config;
        unsigned char addressdev;// default=0x23
    }
    general;
} ;

char mem[100];
}tdevice;

