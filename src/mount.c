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
    struct DirElement* firstprint;
    ushort position;
};
struct Directory presentdir;

// Progress bar
static const char progressBar[4] = { 0xA5, 0xA1, 0xA7, ' ' };
static const char progressRev[4] = { 0,    0,    1,    1 };

// REU size variables
char reuname[21] = "cpm";                                                     // Set cpm.reu as default
unsigned char reusize = 7;                                                      // Set 16 MB as default
char* reusizelist[8] = { "128 KB","256 KB","512 KB","1 MB","2 MB","4 MB","8 MB","16 MB"};
unsigned char reusizes[8] = { 1,3,7,15,31,63,127,255 };

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
    unsigned char datalength,count;
    unsigned char xpos = 0;

    // Free memory of previous dir if any
    if(presentdir.firstelement) { Freedir(); }

    // Clear directory values
    presentdir.firstelement = 0;
    presentdir.firstprint = 0;
    presentdir.position = 0;

    // Initialise reading dir
    uii_open_dir();
    if(!uii_success()) {
        uii_abort();
        //ClearArea(0,24,80,1);
        //printstrvdc(0,24,colorText,uii_status);
        //cgetc();
        return;
    }
    uii_get_dir();

    // Loop while dir data is available or memory is full
    while(uii_isdataavailable())
	{
        // Reset entry type
        presenttype = 0;

        // Get next dir entry
		uii_readdata();
		uii_accept();

        datalength = strlen(uii_data);

        // print progress bar
        if ((count>>2) >= 50) {
            xpos = 0;
            count=0;
            ClearArea(0,24,50,1);
        } else {
            fillattrvdc(xpos + (count>>2),24,1,progressRev[count & 3]?colorText+vdcRvsVid:colorText);
            filldspvdc(xpos + (count>>2),24,1,progressBar[count & 3]);
            count++;
        }

        //ClearArea(0,23,80,2);
        //printstrvdc(0,24,colorText,uii_status);
        //printstrvdc(0,24,colorText,uii_data);
        //cgetc();

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
            datalength--; // Minus 1 for first attribute byte
            if(datalength>20) { datalength=20; presenttype=6; }    // Truncate for max 20
            memset(buffer,0,21);
            strlcpy(buffer,uii_data+1,datalength+1);

            // Allocate memory for dir entry
            present = calloc(1, 26);

            // Break loop if memory is full
            if(!present) {
                // Abort UCI dir reading
                uii_abort();
                //ClearArea(0,24,80,1);
                //printstrvdc(0,24,colorError,"Memory full");
                //cgetc();
                return;
            }

            // Set direntry data
            presentdirelement = present;
            strcpy(presentdirelement->filename,buffer);
            presentdirelement->type = presenttype;

            // Set direntry pointers
            if(!previous) { presentdir.firstelement = present; presentdir.firstprint = present; previous=present; }
            else {
                presentdirelement->prev = previous;     // Set prev in new entry
                presentdirelement = previous;           // Load previous element
                presentdirelement->next = present;      // Set next in this prev element
                previous=present;                       // Update previous pointer
            }

            //ClearArea(0,24,80,1);
            //sprintf(buffer,"%-20s %d %3d %3d",uii_data+1,presenttype,datalength,strlen(uii_data));
            //printstrvdc(0,24,colorText,buffer);
            //cgetc();
        }
	}
    present = presentdir.firstelement;
    presentdirelement = presentdir.firstelement;
    ClearArea(0,24,50,1);
}

// Screen printing

void DrawIDandPath() {
// Draw UCI ID and pathname.

    ushort length;

    // Clear area
    ClearArea(0,3,50,2);

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
            if(!config.auto_override) {
                sprintf(buffer+3,"%02d, %s %s",drive+8,uii_device_tyoe(uii_devinfo[validdrive[drive]-1].type),(uii_device_tyoe(uii_devinfo[validdrive[drive]-1].power))?"On ":"Off");
            } else {
                sprintf(buffer+3,"%02d, manual.");
            }
        }
        printstrvdc(51,4+drive,colorText,buffer);
    }    
}

void DrawTargetdrive() {
// Draw presently selected target.

    ClearArea(51,9,29,1);
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

void PrintDirEntry(struct DirElement *present, unsigned char printpos) {
// Print entry

    unsigned char xpos = (printpos>17)?25:0;
    unsigned char ypos = (printpos>17)?printpos-13:5+printpos;
    unsigned char color = (printpos == presentdir.position)?colorSelect:colorDirEntry;

    sprintf(buffer,"%-20s %-3s",present->filename,entrytypes[present->type-1]);
    printstrvdc(xpos,ypos,color,buffer);
}

void DrawDir(unsigned char readdir) {
// Draw the dirlisting. Read dir is flag for 0 = just reprint same dir, 1 = read new dir

    unsigned char printpos = 0;
    struct DirElement *present;

    // Clear area
    ClearArea(0,5,50,18);

    // Read directory contents
    if(readdir) {
        DrawIDandPath();
        Readdir();
    }

    // Print no data if no valid entries in dir are found
    if(!presentdir.firstprint) {
        printstrvdc(1,5,colorError,"No data.");
    }
    // Print entries until area is filled or last item is reached
    else
    {
        // Get direlement
        present = presentdir.firstprint;

        // Loop while area is not full and further direntries are still present
        do
        {
            // Print entry and increase printpos
            PrintDirEntry(present, printpos++);

            // Check if next dir entry is present, if no: break. If yes: update present pointer
            if(!present->next) { break; }
            else { present = present->next; }

        } while (printpos<36);
    }

    present = presentdir.firstelement;
}

void DrawMenu() {
    unsigned char ypos = 11;
    
    printstrvdc(51,ypos++,colorText,"A-D   : Select target");
    printstrvdc(51,ypos++,colorText,"Curs  : Navigation");
    printstrvdc(51,ypos++,colorText,"RETURN: Select");
    printstrvdc(51,ypos++,colorText,"P / U : Page Down/Up");
    printstrvdc(51,ypos++,colorText,"T / E : Go to Top/End");
    printstrvdc(51,ypos++,colorText,"DEL   : Parent dir");
    printstrvdc(51,ypos++,colorText,"R     : Root dir");
    printstrvdc(51,ypos++,colorText,"H     : Home dir");
    printstrvdc(51,ypos++,colorText,"S     : Save REU");
    printstrvdc(51,ypos++,colorText,"I     : Version");
    printstrvdc(51,ypos++,colorText,"ESC   : Quit");
}

// Mounting routines

void EnableDrivePower(unsigned char ab) {
// Toggle power for the Ultimate drives
// - ab: Drive A = 0, Drive B = 1

    if(uii_devinfo[ab].exist) {
    // Drive selected and existing

        if(!uii_devinfo[ab].power) {
        // Power on drive A if needed
            if(!ab) { 
                uii_enable_drive_a();
            } else {
                uii_enable_drive_b();
            }
            delay(20);
        }
        CheckUCIStatus();
    }
}

void MountImage(ushort target,char* image) {

    ushort ult_drive;

    // Recheck drive statusses and see if power on/off is supported
    if(uii_parse_deviceinfo()) {
        EnableDrivePower((uii_devinfo[0].id == target+7)?0:1);  // Enable drive A if powered off
    }                                     

    // Mount image
    uii_mount_disk(target+7,image);                             

    // Print error message or success message
    if(!CheckUCIStatus()) {
        uii_parse_deviceinfo();                                 // Recheck drive statusses
        ClearArea(51,3,29,5);
        DrawDrivetypes();                                       // Redraw drive statusses
        printstrvdc(0,23,colorSucces,"Succes!");
        sprintf(buffer,"%s mounted on %c, ID %2d",image,target+'A'-1,target+7);
        printstrvdc(8,23,colorText,buffer);
        printstrvdc(0,24,colorText,"Press ESC to exit or any other key to mount another image.");
        if(cgetc() == K_ESCAPE) { done(0); }                    // Exit program if ESCAPE is pressed
        ClearArea(0,23,80,2);
    }
}

void saveREU() {
// Function to save present REU expanded memory to REU file

    unsigned char namelen,key;
    char savename[21];

    // Ask for filename
    printstrvdc(0,23,colorText,"Enter filename for the image:");
    if( textInput(0,24,reuname,16) == -1) { ClearArea(0,23,80,2); return ; }
    ClearArea(0,23,80,2);

    // Add extention based on RAM disk type
    strcpy(savename,reuname);
    namelen = strlen(savename);
    savename[namelen] = '.';
    strcpy(savename+1+namelen,"reu");

    // Check if file exists
    uii_open_file(1,savename);
    if(uii_success()) {
        uii_close_file();
        printstrvdc(0,24,colorText,"File exists. Overwrite? Y=Yes or N=No");
        do
        {
            key = cgetc();
        } while (key!='y' && key!='n');
        ClearArea(0,24,80,1);

        if( key == 'n') {
            // Exit function on NO on overwrite
            return;
        } else {
            // Delete existing file if YES on overwrite
            uii_delete_file(savename);
            if(CheckUCIStatus()) { return; }
        }
    } else {
        uii_abort();
    }

    // Select REU size
    printstrvdc(0,23,colorText,"Select REU size (+/-/ENTER):");
    do
    {
        sprintf(buffer,"REU file size: (%d) %s  ",reusize,reusizelist[reusize]);
        printstrvdc(0,24,colorText,buffer);
        key = cgetc();
        if(key == '+') {
            reusize++;
            if(reusize > 7) { reusize = 0; }
        }
        if(key == '-') {
            if(reusize == 0) { reusize = 7; }
            else { reusize--; }       
        }
    } while (key!=K_RETURN);
    ClearArea(0,23,80,2);

    // Print saving message
    sprintf(buffer,"Saving: %d KB to %s. Please wait.",(reusizes[reusize]+1)*64,savename);
    printstrvdc(0,24,colorText,buffer);

    // Save REU image
    uii_open_file(0x06,savename);
    if(CheckUCIStatus()) { return; }
    uii_save_reu(reusizes[reusize]);
    if(CheckUCIStatus()) { return; }
    uii_close_file();
    ClearArea(0,24,80,1);

    // Print result
    if( CheckUCIStatus() ) {
        printstrvdc(0,24,colorSucces,"Succes! ");
        sprintf(buffer,"Saved %d KB to %s. Press key.",(reusizes[reusize]+1)*64,savename);
        printstrvdc(8,24,colorText,buffer);
        cgetc();
        ClearArea(0,24,80,1);
    }
}

// Main

void main (void) {

    unsigned char key,element;
    struct DirElement *present;

    // Set version number in string variable
    sprintf(version,
            "v%2d.%2d - %c%c%c%c%c%c%c%c-%c%c%c%c",
            VERSION_MAJOR, VERSION_MINOR,
            BUILD_YEAR_CH0, BUILD_YEAR_CH1, BUILD_YEAR_CH2, BUILD_YEAR_CH3,
            BUILD_MONTH_CH0, BUILD_MONTH_CH1, BUILD_DAY_CH0, BUILD_DAY_CH1,
            BUILD_HOUR_CH0, BUILD_HOUR_CH1, BUILD_MIN_CH0, BUILD_MIN_CH1);

    // Set presentdir pointer at zero
    presentdir.firstelement = 0;

    // Get config data
    ReadConfigfile(0);

    // Get valid UII+ drives
    SetValidDrives();
    if(!targetdrive) {
        printf("No targets detected or set. Press OK to abort.");
        exit(1);
    }

    // Initialize screen
    init();
    headertext("UMount: Image mounter");
    DrawDrivetypes();
    DrawTargetdrive();
    DrawMenu();
    DrawDir(1);

    // Main loop
    presentdirelement = presentdir.firstelement;
    do
    {
        present = presentdirelement;

        //sprintf(buffer,"Pos    : %02d",presentdir.position);
        //printstrvdc(51,20,colorText,buffer);
        //sprintf(buffer,"Present: %4X",present);
        //printstrvdc(51,21,colorText,buffer);
        //sprintf(buffer,"PrDirEl: %4X",presentdirelement);
        //printstrvdc(51,22,colorText,buffer);
        //sprintf(buffer,"FirstEl: %4X",presentdir.firstelement);
        //printstrvdc(51,23,colorText,buffer);
        //sprintf(buffer,"FirstPr: %4X",presentdir.firstprint);
        //printstrvdc(51,24,colorText,buffer);

        key = cgetc();

        //sprintf(buffer,"Key    : %02X",key);
        //printstrvdc(51,19,colorText,buffer);


        switch (key)
        {
        case CURS_DOWN:
        case CURU_DOWN:
        // Go down
            if(presentdirelement && presentdirelement->next) {
                present = presentdirelement->next;
                presentdirelement = present;
                presentdir.position++;
                if(presentdir.position > 35) {
                    // Next page
                    presentdir.firstprint = present;
                    presentdir.position = 0;
                    DrawDir(0);
                    presentdirelement = present;
                } else {
                    // Next line same page
                    presentdirelement = present->prev;
                    PrintDirEntry(presentdirelement,presentdir.position-1);
                    presentdirelement = present;
                    PrintDirEntry(presentdirelement,presentdir.position);
                }
            }
            break;

        case CURS_UP:
        case CURU_UP:
        // Go up
            if(presentdirelement && presentdirelement->prev) {
                present = presentdirelement->prev;
                presentdirelement = present;
                if(presentdir.position == 0) {
                    // Previous page
                    // Go back 35 positions
                    for(element=0;element<35;element++) {
                        if(present->prev) {
                            present = present->prev;
                            presentdir.position++;
                        }
                    }
                    presentdir.firstprint = present;
                    DrawDir(0);
                } else {
                    // Previous line same page
                    presentdir.position--;
                    presentdirelement = present->next;
                    PrintDirEntry(presentdirelement,presentdir.position+1);
                    presentdirelement = present;
                    PrintDirEntry(presentdirelement,presentdir.position);
                }
            }
            break;

        case CURS_RIGHT:
        case CURU_RIGHT:
        // Go right
            if(presentdir.position<18) {
                // Check if not already right
                presentdir.position++;
                PrintDirEntry(present,presentdir.position-1);
                presentdir.position--;
                // Scroll 18 positions forward or to last element
                for(element=0;element<18;element++) {
                    if(present->next) {
                        present = present->next;
                        presentdir.position++;
                    }
                }
                PrintDirEntry(present,presentdir.position);
                presentdirelement = present;
            }
            break;

        case CURS_LEFT:
        case CURU_LEFT:
        // Go left
            if(presentdir.position>17) {
                // Check if not allready left
                presentdir.position++;
                PrintDirEntry(present,presentdir.position-1);
                presentdir.position--;
                // Scroll 18 positions back or to first element
                for(element=0;element<18;element++) {
                    if(present->prev) {
                        present = present->prev;
                        presentdir.position--;
                    }
                }
                PrintDirEntry(present,presentdir.position);
                presentdirelement = present;
            }
            break;

        case K_RETURN:
        case K_CR:
        // RETURN for select dir or image
            if(presentdir.firstelement && presentdirelement->type !=6 )
            // Check if a valid type
            {
                if(presentdirelement->type == 1) {
                    // Change directory
                    uii_change_dir(presentdirelement->filename);
                    CheckUCIStatus();
                    DrawDir(1);
                } else {
                    // Mount image
                    MountImage(targetdrive,presentdirelement->filename);
                }
            }
            break;

        case K_BACKKSPACE:
        case K_DEL:
        // Go to parent dir
            uii_change_dir("..");
            CheckUCIStatus();
            DrawDir(1);
            break;

        case 'p':
            if(presentdirelement->next) {
                // Check if not allready last item
                presentdir.position=0;
                // Go forward 36 positions
                for(element=0;element<36;element++) {
                    if(present->next) {
                        present = present->next;
                    }
                }
                presentdir.firstprint = present;
                DrawDir(0);
                presentdirelement = present;
            }
            break;

        case 'u':
            if(presentdirelement->prev) {
                // Check if not allready first item
                presentdir.position=0;
                // Go back 36 positions
                for(element=0;element<36;element++) {
                    if(present->prev) {
                        present = present->prev;
                    }
                }
                presentdir.firstprint = present;
                DrawDir(0);
                presentdirelement = present;
            }
            break;

        case 'r':
        // Go to root dir
            uii_change_dir("/");
            CheckUCIStatus();
            DrawDir(1);
            break;

        case 'h':
        // Go to home dir
            uii_change_dir_home();
            CheckUCIStatus();
            DrawDir(1);
            break;

        case 't':
        // Go to first entry
            if(presentdir.firstelement && presentdirelement->prev) {
                // Check if a dir is loaded and if not already first item
                presentdirelement = presentdir.firstelement;
                presentdir.firstprint = presentdir.firstelement;
                presentdir.position = 0;
                DrawDir(0);
            }
            break;

        case 'e':
        // Go to last entry
            if(presentdir.firstelement && presentdirelement->next) {
                // Check if a dir is loaded and if not already last item

                // Go to last item and update position and first printed
                element = presentdir.position;
                do
                {
                    present = present->next;
                    element++;
                    if(element>35) {
                        element = 0;
                        presentdir.firstprint = present;
                    }
                } while (present->next);
                presentdir.position = element;
                presentdirelement = present;
                DrawDir(0);
            }
            break;

        case 'a':
        case 'b':
        case 'c':
        case 'd':
            if(validdrive[key-'a']) {
                targetdrive = key - 'a' + 1;
                DrawTargetdrive();
            }
            break;
        
        case 's':
            saveREU();
            break;

        case 'i':
            sprintf(buffer,"Version: %s. Press key.",version);
            printstrvdc(0,24,colorText,buffer);
            cgetc();
            ClearArea(0,24,80,1);
            break;

        default:
            break;
        }
    } while (key != K_ESCAPE);

    // Exit program
    done(0);
}