/*
 * cMapping.h
 *
 *  Created on: 1 mai 2018
 *      Author: Nox
 */

#ifndef CMAPPING_H_
#define CMAPPING_H_

#include <iostream>

#define MAXOFFSET 250
#define KNbRoues 2
#define KNbCommandeMoteur 4

typedef uint8_t tCommandeCubeur;
typedef uint8_t tCouleurCubeur;

struct sCouleursCubeur {
  union {
    uint32_t Valeur;
    struct {
      uint8_t CouleurDebut;
      uint8_t CouleurMilieu;
      uint8_t CouleurFin;
      uint8_t Vide;
    } Capteurs;
  };
};

struct sCommandeMoteur {
  uint16_t Config; // Non utilisé
  int16_t Distance[KNbRoues];
  int16_t Vitesse[KNbRoues];
  int16_t AccelerationMax; // max acceleration allowed
};

struct sCommandesMoteur {
  sCommandeMoteur Commandes[KNbCommandeMoteur];
};

struct sRetourMoteur {
  uint32_t time;// last update time of internal clock
  int16_t dvalue[KNbRoues]; // delta of tick since last read
  int16_t dtime[KNbRoues]; // delta time between the 2 last tick
  int16_t avgspeed[KNbRoues]; // dcoder/delta time
    int32_t value[KNbRoues];// absolute value
};

struct sCapteursCubeurA {
  union {
    uint8_t Valeur;
    struct {
      uint8_t PresenceHaut:1;
      uint8_t Vide:7;
    } Capteurs;
  };
};

struct sCapteursCubeurD {
  union {
    uint8_t Valeur;
    struct {
      uint8_t PositionChargeur:2;
      uint8_t PresenceReserve:1;
      uint8_t Vide:5;
    } Capteurs;
  };
};

// Cubeur (Ascendant, Descendant, Rampe)
struct sCubeurA {
  tCommandeCubeur CmdRtr;

  tCouleurCubeur Couleur;
  sCapteursCubeurA Capteurs;
};

struct sCubeurD {
  tCommandeCubeur CmdRtr;

  sCapteursCubeurD Capteurs;
};

struct sCubeurR {
  tCommandeCubeur CmdRtr;
};

union mappingPlanMemoire_t {
  struct {
    // Moteur
    struct {
      sCommandesMoteur Commandes;
      sRetourMoteur Retour;
    } Moteur;

    // Cubeur (Ascendant, Descendant)
    struct {
      sCubeurA CubeurA;
      sCubeurD CubeurD;
      sCubeurR CubeurR;
      sCouleursCubeur CouleursCubeur;
    } Cubeur;

    // Afficheur
    struct {
        uint8_t data[4];// currently displayed in segment -> 84
        char Brightness;// contast
        signed char index;// curent index of displyed string, -1 when not used.
        char string[64];// string display in rolling mode finished by /0x0
        uint16_t number;// display a number between 0 and 9999 -> 106
        uint16_t rollerover;
        unsigned long time;
    } Afficheur;
  };
  uint8_t memmap[MAXOFFSET];
};

#endif /* CMAPPING_H_ */
