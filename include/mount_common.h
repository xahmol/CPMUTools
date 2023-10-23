#ifndef __MOUNT_COMMON_H_
#define __MOUNT_COMMON_H_

// Global variables
extern unsigned char validdrive[4];
extern char entrytypes[7][4];
extern unsigned char targetdrive;
extern unsigned char firmwareflag;

// Function prototypes
unsigned char CheckUCIStatus();
void SetValidDrives();

#endif