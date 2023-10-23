/*
CPMUMount
Disk mounter for the Ultimate II+ in CPMS
Written in 2023 by Xander Mol

https://github.com/xahmol/CPMUTools

https://www.idreamtin8bits.com/

Code and resources from others used:

-   Scott Hutter - Ultimate II+ library:
    https://github.com/xlar54/ultimateii-dos-lib/
    (library for the UII+ UCI functions)

-   z88dk C cross compiler for Z80 targets, including C128 CP/M
    https://github.com/z88dk/z88dk/wiki

-   Jochen Metzinger - ctools
    https://github.com/mist64/ctools

-   ntp2ultimate by MaxPlap
    https://github.com/MaxPlap/ntp2ultimate
    Time via NTP code

-   EPOCH-to-time-date-converter by sidsingh78
    https://github.com/sidsingh78/EPOCH-to-time-date-converter/blob/master/epoch_conv.c

-   Bart van Leeuwen for providing CP/M images, testing and advice

-   Gideon Zweijtzer for creating the Ultimate II+ cartridge and the Ultimate64, and the Ultimate Command Interface enabling this software.
   
-   Tested using real hardware (C128D, C128DCR) using CP/M 3.0 and ZP/M+.

Licensed under the GNU General Public License v3.0

The code can be used freely as long as you retain
a notice describing original source and author.

THE PROGRAMS ARE DISTRIBUTED IN THE HOPE THAT THEY WILL BE USEFUL,
BUT WITHOUT ANY WARRANTY. USE THEM AT YOUR OWN RISK!
*/

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <cpm.h>
#include <c128/vdc.h>
#include <c128/cia.h>
#include "include/defines.h"
#include "include/core.h"
#include "include/mount_common.h"
#include "include/ultimate_common_lib.h"
#include "include/ultimate_dos_lib.h"
#include "include/ultimate_time_lib.h"

// Global variables
unsigned char validdrive[4] = {0,0,0,0};
char entrytypes[7][4] = {
    "DIR",
    "D64",
    "D71",
    "D81",
    "DNP",
    "!TL"
};
unsigned char targetdrive;
unsigned char firmwareflag;

// UCI routines

unsigned char CheckUCIStatus() {
// Function to check UII+ status and print error box if applicable

    if (!uii_success()) {
        ClearArea(0,24,80,1);
        printstrvdc(0,24,colorError,"Error: ");
        sprintf(buffer,"%s. Press key.",uii_status);
        printstrvdc(7,24,colorText,buffer);
        cgetc();
        ClearArea(0,24,80,1);
        return 1;
    }
    return 0;
}

void SetValidDrives() {
// Initialise variables to defines which drive IDs are valid targets

    unsigned char drive;

    targetdrive = 0;
    firmwareflag = 0;

    if(!uii_detect()) {
        printf("No Ultimate Command Interface.\n\r");
        exit(1);
    }

	uii_settarget(TARGET_DOS1);
    if(uii_isdataavailable())
	{
		uii_abort();
	}

    // Set dir at home dir
    uii_change_dir_home();

    // Get device info from UCI
    if(!uii_parse_deviceinfo()) {
        mountconfig.auto_override = 1;
        firmwareflag = 2;
    }

    if(mountconfig.auto_override) {
    // Set valid drives with the override settings in the config data
        
        for(drive=0;drive<4;drive++) {
            validdrive[drive] = mountconfig.valid[drive];
        }
    } else {
    // Set valid drives with auto detection

      for(drive=0;drive<4;drive++)
      {
          validdrive[drive]=0;

          // Check if drive ID is a Ultimate emulated drive
          if(uii_devinfo[0].id == drive+8) {
              validdrive[drive] = 1;
          }
          if(uii_devinfo[1].id == drive+8) {
              validdrive[drive] = 2;
          }
          if(validdrive[drive] && !targetdrive) { targetdrive = drive+1; }
      }
    }

    // Set target drive to override if set and valid
    if(mountconfig.target && validdrive[mountconfig.target-1]) { targetdrive = mountconfig.target; }
}