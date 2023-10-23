#ifndef __CORE_H_
#define __CORE_H_

// Global variables
extern char buffer[81];
extern char version[25];
extern unsigned char vdcDispMem;
extern char configbuffer[101];
extern char filename[13];
extern long secondsfromutc; 
extern unsigned char verbose;
extern unsigned char ntpon;
extern char host[81];
struct mconfig {
    unsigned char auto_override;
    unsigned char valid[4];
    unsigned char target;
};
extern struct mconfig mountconfig;

// Function prototypes
void init();
void done(unsigned char exitcode);
void delay(ushort D);
int textInput(ushort xpos, ushort ypos, char *str, ushort size);
void ClearArea(ushort x, ushort y, ushort width, ushort height);
void headertext(char* subtitle);
void WriteConfigfile(unsigned char vdcmode);
void ReadConfigfile(unsigned char vdcmode);

#endif