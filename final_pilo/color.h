#ifndef __COLOR_H
#define __COLOR_H

////////////////////////////////////////////////////////////////////////
//////////////////////   MEM   ////////////////////////////////////////

#define Undefined 'U'
#define Black 'N'
#define Blue 'B'
#define Green 'G'
#define Yellow 'Y'
#define Orange 'O'
#define White 'W'
#define CUBE_PRESENT 0
#define CUBE_ABSENT 1

typedef struct {
  char Left;
  char Right;
  char Column;
  char cubePresent;

} tcolor;
#endif

void setupColor();
void loop(tcolor &color);

