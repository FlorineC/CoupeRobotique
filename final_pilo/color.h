#if !defined(__COLOR__)
#define __COLOR__ 1

////////////////////////////////////////////////////////////////////////
//////////////////////   MEM   ////////////////////////////////////////

#define Undefined 'U'
#define Black 'N'
#define Bleue 'B'
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
void setupColor();
void loop(tcolor &color);
#endif
