#ifndef __MOUNT_COMMON_H_
#define __MOUNT_COMMON_H_

// Global variables
struct mconfig {
    unsigned char auto_override;
    unsigned char valid[4];
    unsigned char target;
};
extern struct mconfig mountconfig;

extern unsigned char validdrive[4];
extern char entrytypes[7][4];
extern unsigned char targetdrive;
extern unsigned char firmwareflag;

// Function prototypes
void init();
void done(unsigned char exitcode);
void ClearArea(ushort x, ushort y, ushort width, ushort height);
void headertext(char* subtitle);
unsigned char CheckStatus();
unsigned char Checkcommandsupport();
void ReadConfigdata();
void SetValidDrives();

#endif