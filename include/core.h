#ifndef __CORE_H_
#define __CORE_H_

// Global variables
extern char buffer[81];
extern char version[25];
extern unsigned char vdcDispMem;
extern char filename[13];
struct ConfigStruct {
    char ntphost[81];
    long secondsfromutc;
    unsigned char verbose;
    unsigned char ntpon;
    unsigned char auto_override;
    unsigned char valid[4];
    unsigned char target;
};
extern struct ConfigStruct config;

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