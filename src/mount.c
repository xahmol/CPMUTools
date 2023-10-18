/*
GeoUMount
Disk mounter for the Ultimate II+ in GEOS
Written in 2023 by Xander Mol

https://github.com/xahmol/GeoUTools

https://www.idreamtin8bits.com/

Code and resources from others used:

-   Scott Hutter - Ultimate II+ library:
    https://github.com/xlar54/ultimateii-dos-lib/
    (library for the UII+ UCI functions)

-   z88dk C cross compiler for Z80 targets, including C128 CP/M
    https://github.com/z88dk/z88dk/wiki

-   Jochen Metzinger - ctools
    https://github.com/mist64/ctools

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
#include "include/mount_common.h"
#include "include/ultimate_common_lib.h"
#include "include/ultimate_dos_lib.h"
#include "include/ultimate_time_lib.h"
#include "include/ultimate_network_lib.h"

// Global variables
unsigned char reusavesize;

// Directory entry struct
struct DirElement {
    unsigned char type; // Type: 1=dir, 2=D64, 3=D71, 4=D81
	char filename[21];
    struct DirElement* next;
    struct DirElement* prev;
};
struct DirElement *presentdirelement;

struct Directory {
    struct DirElement* firstelement;
    struct DirElement* lastelement;
    struct DirElement* firstprint;
    struct DirElement* lastprint;
    ushort position;
};
struct Directory presentdir;

// Directory reading

void Freedir() {
// Free memory of presently loaded dir

    struct DirElement* next = 0;
    struct DirElement* present;

    present = presentdir.firstelement;
    
    do
    {
        presentdirelement = present;
        next = presentdirelement->next;
        free(present);
        present = next;
    } while (next);
}

void Readdir() {
// Read present dir of UCI DOS target

    struct DirElement* previous = 0;
    struct DirElement* present = 0;
    unsigned char presenttype;
    unsigned char datalength,maxlength;

    // Free memory of previous dir if any
    if(presentdir.firstelement) { Freedir(); }

    // Clear directory values
    presentdir.firstelement = 0;
    presentdir.firstprint = 0;
    presentdir.firstprint = 0;
    presentdir.lastprint = 0;
    presentdir.position = 0;

    // Initialise reading dir
    uii_open_dir();
    if(!uii_success()) {
        uii_abort();
        return;
    }

    // Loop while dir data is available or memory is full
    while(uii_isdataavailable())
	{
        // Reset entry type
        presenttype = 0;

        // Get next dir entry
		uii_readdata();
		uii_accept();

        datalength = strlen(uii_data);

        // Check if entry is a dir by checking if bit 4 of first byte is set
        if(uii_data[0]&0x10) { presenttype=1; }

        // Check if file is a matching image type
        if(!presenttype && datalength>4) {

            // Check for filename extension of a disk image (D64/G64, D71/G71 or D81)

            // First check for D or G as first letter of extension after a dot
            if( (   uii_data[datalength-4] == '.') && 
                (   uii_data[datalength-3] == 'd' || uii_data[datalength-3] == 'D' ||
                    uii_data[datalength-3] == 'g' || uii_data[datalength-3] == 'G' ) ) {

                // Check for D64/G64
                if( (uii_data[datalength-2] == '6') && (uii_data[datalength-1] == '4') ) {
                    presenttype = 2;
                }

                // Check for D71/G71
                if( (uii_data[datalength-2] == '7') && (uii_data[datalength-1] == '1') ) {
                    presenttype = 3;
                }

                // Check for D81
                if( (uii_data[datalength-2] == '8') && (uii_data[datalength-1] == '1') ) {
                    presenttype = 4;
                }
            }
        }

        if(presenttype) {
            // Get file or dir name to buffer
            maxlength = datalength-1; // Minus 1 for first attribute byte
            if(maxlength>20) {maxlength=20; presenttype=6; }    // Truncate for max 20
            memset(buffer,0,21);
            strlcpy(buffer,uii_data+1,maxlength);

            // Allocate memory for dir entry
            present = calloc(1, 26);

            // Break loop if memory is full
            if(!present) {
                // Abort UCI dir reading
                uii_abort();
                return;
            }

            // Set direntry data
            presentdirelement = present;
            strcpy(presentdirelement->filename,buffer);
            presentdirelement->type = presenttype;

            // Set direntry pointers
            presentdir.lastelement = present;       // Update dir last element
            if(!previous) { presentdir.firstelement = present; presentdir.firstprint = present; previous=present; }
            else {
                presentdirelement->prev = previous;     // Set prev in new entry
                presentdirelement = previous;           // Load previous element
                presentdirelement->next = present;      // Set next in this prev element
                previous=present;                       // Update previous pointer
            }
        }
	}
}

// Screen printing

void DrawIDandPath() {
// Draw UCI ID and pathname.

    ushort length;

    // Clear area
    ClearArea(0,3,80,2);

    // Get ID from UCI and print
    uii_identify();
    sprintf(buffer,"ID: %s",uii_data);
    printstrvdc(0,3,colorText,buffer);

    // Get present path from UCI and print
    uii_get_path();

    // Check if it fits else shorten
    length = strlen(uii_data);
    if(length>50) {
        strcpy(buffer,uii_data+length-50);
    } else {
        strcpy(buffer,uii_data);
    }
    printstrvdc(0,4,colorText,buffer);
}

void DrawDrivetypes() {
// Draw the drive types of the targets

    unsigned char drive;

    printstrvdc(51,3,colorText,"Drives:");
    for(drive=0;drive<4;drive++)
    {
        sprintf(buffer,"%c: ",'A'+drive);
        if(!validdrive[drive]) {
            sprintf(buffer+3,"No target");
        } else {
            sprintf(buffer+3,"%02d, %s",drive+8,uii_device_tyoe(uii_devinfo[validdrive[drive]-1].type));
        }
        printstrvdc(51,4+drive,colorText,buffer);
    }    
}

void DrawTargetdrive() {
// Draw presently selected target.

    ClearArea(51,9,30,1);
    if(targetdrive)
    {
        sprintf(buffer,"Target is drive %c",targetdrive-1+'A');
    }
    else
    {
        sprintf(buffer,"No valid target");
    }
    printstrvdc(51,9,colorText,buffer);
}

// Main

void main (void) {
    // Set version number in string variable
    sprintf(version,
            "v%2i.%2i - %c%c%c%c%c%c%c%c-%c%c%c%c",
            VERSION_MAJOR, VERSION_MINOR,
            BUILD_YEAR_CH0, BUILD_YEAR_CH1, BUILD_YEAR_CH2, BUILD_YEAR_CH3,
            BUILD_MONTH_CH0, BUILD_MONTH_CH1, BUILD_DAY_CH0, BUILD_DAY_CH1,
            BUILD_HOUR_CH0, BUILD_HOUR_CH1, BUILD_MIN_CH0, BUILD_MIN_CH1);

    // Set presentdir pointer at zero
    presentdir.firstelement = 0;

    // Get config data from application header
    //ReadConfigdata();

    // Get valid UII+ drives
    SetValidDrives();
    if(!targetdrive) {
        printf("No targets detected or set. Press OK to abort.");
        exit(1);
    }

    // Initialize screen
    init();
    headertext("(C) 2023 Xander Mol");
    DrawIDandPath();
    DrawDrivetypes();
    DrawTargetdrive();

    // Exit program
    cgetc();
    done(0);
}