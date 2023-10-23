#ifndef __CORE_H_
#define __CORE_H_

// Global variables
extern char buffer[81];
extern char version[25];
extern unsigned char vdcDispMem;
extern unsigned char validdrive[4];
extern char entrytypes[7][4];
extern unsigned char targetdrive;

// Function prototypes
void init();
void done(unsigned char exitcode);
void delay(ushort D);
int textInput(ushort xpos, ushort ypos, char *str, ushort size);
void ClearArea(ushort x, ushort y, ushort width, ushort height);
void headertext(char* subtitle);

#endif