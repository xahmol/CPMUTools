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

void PrintConfigData() {

    unsigned char ypos=3;
    unsigned char drive;

    printstrvdc(0,ypos++,colorText,"Present configuration.");

    ypos++;
    printstrvdc(0,ypos++,colorSucces,"UMount settings:");
    ypos++;
    sprintf(buffer,"Detection override: %s",mountconfig.auto_override?"Yes":"No");
    printstrvdc(0,ypos++,colorText,buffer);

    if(firmwareflag == 2) {
        sprintf(buffer,"Autodetection not supported in firmware.");
    } else {
        sprintf(buffer,"Autodetection of valid drives succeeded.");
    }
    printstrvdc(0,ypos++,colorText,buffer);

    for(drive=0;drive<4;drive++)
    {
        sprintf(buffer,"%c: ",'A'+drive);
        if(!validdrive[drive]) {
            sprintf(buffer+3,"No target");
        } else {
            sprintf(buffer+3,"%02d, %s %s",drive+8,uii_device_tyoe(uii_devinfo[validdrive[drive]-1].type),(uii_device_tyoe(uii_devinfo[validdrive[drive]-1].power))?"On ":"Off");
        }
        printstrvdc(0,ypos++,colorText,buffer);
    }   

    if(targetdrive) {
        sprintf(buffer,"Target drive      : %c",targetdrive-1+'A');
    } else {
        sprintf(buffer,"No valid target drive set");
    }
    printstrvdc(0,ypos++,colorText,buffer);

    ypos++;
    printstrvdc(0,ypos++,colorSucces,"UTime settings:");
    ypos++;
    printstrvdc(0,ypos++,colorText,"NTP host:");
    printstrvdc(0,ypos++,colorText,host);
    sprintf(buffer,"UTC offset        : %ld",secondsfromutc);
    printstrvdc(0,ypos++,colorText,buffer);
    sprintf(buffer,"Update from NTP   : %sabled",ntpon?"En":"Dis");
    printstrvdc(0,ypos++,colorText,buffer);
    sprintf(buffer,"Verbose           : %sabled",verbose?"En":"Dis");
    printstrvdc(0,ypos++,colorText,buffer);
}

void main() {
    unsigned char key;

    // Set version number in string variable
    sprintf(version,
            "v%2d.%2d - %c%c%c%c%c%c%c%c-%c%c%c%c",
            VERSION_MAJOR, VERSION_MINOR,
            BUILD_YEAR_CH0, BUILD_YEAR_CH1, BUILD_YEAR_CH2, BUILD_YEAR_CH3,
            BUILD_MONTH_CH0, BUILD_MONTH_CH1, BUILD_DAY_CH0, BUILD_DAY_CH1,
            BUILD_HOUR_CH0, BUILD_HOUR_CH1, BUILD_MIN_CH0, BUILD_MIN_CH1);

    // Initialize screen
    init();
    headertext("UConfig: Configuration tool");

    // Read config file
    memset(&mountconfig,0,sizeof(mountconfig));
    ReadConfigfile(1);

    // Get valid UII+ drives
    SetValidDrives();

    // Print present data
    PrintConfigData();

    // Exit program
    cgetc();
    done(0);
}