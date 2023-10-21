#ifndef __MOUNT_COMMON_H_
#define __MOUNT_COMMON_H_

// Global variables
extern unsigned char validdrive[4];
extern char entrytypes[7][4];
extern unsigned char targetdrive;

// Function prototypes
void init();
void done(unsigned char exitcode);
void delay(ushort D);
void ClearArea(ushort x, ushort y, ushort width, ushort height);
void headertext(char* subtitle);
unsigned char CheckStatus();
unsigned char Checkcommandsupport();
void ReadConfigdata();
void SetValidDrives();

#endif