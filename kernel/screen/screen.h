#include "../types/types.h"

#ifdef __SCREEN__

#define RAMSCREEN 0xB8000
#define SIZESCREEN 0xFA0
#define SCREENLIM 0xB8FA0

int vidram = 0xA0000;
int screenSizeX = 1280;
int screenSizeY = 1024;
int bankMaxPixel = 64000;
int CursorX = 0;
int CursorY = 18;
char kattr = 0x07;

typedef struct _modeInfoBlock {
  u16 attributes;
  u8 windowA, windowB;
  u16 granularity;
  u16 windowSize;
  u16 segmentA, segmentB;
  u32 winFuncPtr;                      /* ptr to INT 0x10 Function 0x4F05 */
  u16 pitch;                           /* bytes per scan line */

  u16 resolutionX, resolutionY;        /* resolution */
  u8 wChar, yChar, planes, bpp, banks; /* number of banks */
  u8 memoryModel, bankSize, imagePages;
  u8 reserved0;

  u8 readMask, redPosition;            /* color masks */
  u8 greenMask, greenPosition; 
  u8 blueMask, bluePosition;
  u8 reservedMask, reservedPosition;
  u8 directColorAttributes;

  u32 physbase;                        /* pointer to LFB in LFB modes */
  u32 offScreenMemOff;
  u16 offScreenMemSize;
  u8  reserved1 [206];
}modeInfoBlock;

#else

extern int vidram;
extern int screenSizeX;
extern int screenSizeY;
extern int bankMaxPixel;
extern int CursorX;
extern int CursorY;
extern char kattr;

#endif

void putpixel(unsigned char *, int, int, int, int, int);
void fillrect(unsigned char *, int, int, int, int, int, int, int);
void show_cursor();
void deletecar();
void scrollup(unsigned int);
void putcar(uchar, char);
void print(char *, char);
void printf(char *);
void move_cursor(int, int);
