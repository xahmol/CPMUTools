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
#include "include/ultimate_common_lib.h"
#include "include/ultimate_dos_lib.h"
#include "include/ultimate_time_lib.h"

// Global variables
char buffer[81];                        // Buffer memory for tempory storage
char version[25];                       // Version info string
ushort vdcDispMem;                      // Variable for VDC display memory start, to be set with vdcmap()
char filename[13] = "cpmutool.cfg";     // Filename for configuration file

// Default config settings
// NTP server host name: pool.ntp.org is default
// Seconds from UTC: 3600 is Central European Time (Central European Summer Time would be 7200)
// UTime verbosoty: UTime gives textual feedback (1) or remains silent (0, default)
// NTP on: set time via NTP server on (1, default) or synch with UII+ RTC only (0)
// Auto pverride: Detect valid drives for mounting images automatically (1) or manually (0)
// Valid: manual setting of drive validity, 1=valid, 0=no target.
// Target: target drive on starting UMount, 0 is default and sets on first valid drive as target
struct ConfigStruct config = { "pool.ntp.org",3600,0,1,0,{0,0,0,0},0 };

// Common routines for CPMUMount and CPMUConfig

// Generic routiens
void init() {
    savevdc();                          // Saveguard VDC registers
    mapvdc();                           // Get VDC environment
    outvdc(vdcFgBgColor,vdcBlack);      // Set background to black
    setcursorvdc(0,0,vdcCurNone);       // No curor
    clrattrvdc(colorText);              // Clear attributes with standard text color
    clrscrvdc(0x20);                    // Clear screen with spaces
}

void done(unsigned char exitcode) {
    clrattrvdc(colorText);              // Clear attributes with standard text color
    clrscrvdc(0x20);                    // Clear screen with spaces
    restorevdc();                       // Restore VDC registers
    setcursorvdc(7,7,vdcCurRate32);     // Restore cursor
    exit(exitcode);                     // Exit to CP/M
}

void delay(ushort D) {
/* delay in tens of seconds (1/10sec) */
  setintctrlcia(cia2,ciaClearIcr); /* disable all cia 2 interrupts */
  settimerbcia(cia2,D,ciaCountA);  /* timer b counts timer a underflows */
  settimeracia(cia2,timervalcia(10),ciaCPUCont); /* set timer a 1/1000 sec */
  while ((inp(cia2+ciaIntCtrl) & 0x02) == 0);      /* wait for count down */
}

/**
 * input/modify a string.
 * based on version 1.0e, then modified.
 * @param[in] xpos screen x where input starts.
 * @param[in] ypos screen y where input starts.
 * @param[in,out] str string that is edited, it can have content and must have at least @p size + 1 bytes. Maximum size if 255 bytes.
 * @param[in] size maximum length of @p str in bytes.
 * @return -1 if input was aborted.
 * @return >= 0 length of edited string @p str.
 */
int textInput(ushort xpos, ushort ypos, char *str, ushort size)
{
  ushort idx = strlen(str);
  ushort b,c,flag;

  printstrvdc(xpos,ypos,colorDirEntry,str);
  printstrvdc(xpos+idx,ypos,colorSelect," ");

  while(1)
    {
      c = cgetc();
      switch (c)
        {
      case K_ESCAPE:
        return -1;

      case K_RETURN:
        idx = strlen(str);
        str[idx] = 0;
        return idx;

      case K_DEL:
        if (idx)
          {
            fillattrvdc(xpos,ypos,idx+1,colorDirEntry);
            --idx;
            filldspvdc(xpos+idx,ypos,2,' ');
            for(c = idx; 1; ++c)
              {
                b = str[c+1];
                str[c] = b;
                if (b == 0)
                  break;
              }
            printstrvdc(xpos,ypos,colorDirEntry,str);
            printstrvdc(xpos+idx,ypos,colorSelect," ");
          }
        break;

        //case CH_INS:
        //  c = strlen(str);
        //  if (c < size &&
        //      c > 0 &&
        //      idx < c)
        //    {
        //      ++c;
        //      while(c >= idx)
        //        {
        //          str[c+1] = str[c];
        //          if (c == 0)
        //            break;
        //          --c;
        //        }
        //      str[idx] = ' ';
        //      cputsxy(xpos, ypos, str);
        //      gotoxy(xpos + idx, ypos);
        //    }
        //  break;

      case CURS_LEFT:
      case CURU_LEFT:
        if (idx)
          {
            fillattrvdc(xpos+idx,ypos,1,colorDirEntry);
            --idx;
            fillattrvdc(xpos+idx,ypos,1,colorSelect);
          }
        break;

      case CURS_RIGHT:
      case CURU_RIGHT:
        if (idx < strlen(str) &&
            idx < size)
          {
            fillattrvdc(xpos+idx,ypos,1,colorDirEntry);
            ++idx;
            fillattrvdc(xpos+idx,ypos,1,colorSelect);
          }
        break;

      default:
        if (isprint(c) &&
            idx < size)
          {
            flag = (str[idx] == 0);
            str[idx] = c;
            printstrvdc(xpos,ypos,colorDirEntry,str);
            ++idx;
            fillattrvdc(xpos+idx,ypos,1,colorSelect);
            if (flag)
              str[idx+1] = 0;
            break;
          }
        break;
      }
    }
  return 0;
}

// Screen routines

void ClearArea(ushort x, ushort y, ushort width, ushort height) {
    ushort line;

    for(line=0;line<height;line++) {
        filldspvdc(x,y+line,width,' ');
        fillattrvdc(x,y+line,width,colorText);
    }
}

void headertext(char* subtitle)
{
    // Draw header text
    // Input: subtitle is text to draw on second line

    sprintf(buffer,"%-80s","CPM UTools: Tootset for using the Ultimate II+ with CP/M");
    printstrvdc(0,0,colorHeader1,buffer);


    sprintf(buffer,"%-80s",subtitle);
    printstrvdc(0,1,colorHeader2,buffer);

    uii_get_time();
    if(uii_success()) {
        printstrvdc(80-strlen(uii_data),1,colorHeader2,uii_data);
    }
}

// Config file routimes

void ErrorMessage(char* stage, unsigned char vdcmode) {
// Config file handling error handling message

    sprintf(buffer,"Error in %s of config file. Press key.",stage);

    if(vdcmode) {
    // Error message in VDC mode
        printstrvdc(0,24,colorError,"Error! ");
        printstrvdc(7,24,colorText,buffer);
        cgetc();
        done(1);
    } else {
        printf("%s",buffer);
        exit(1);
    }
}

void WriteConfigfile(unsigned char vdcmode) {
// Write config file

    FILE* file;

    if(!vdcmode) { printf("Open config flle for writing.\n\r"); } else {
        printstrvdc(0,24,colorText,"Writing configuration file. Please wait.");
    }

    // Open file for write
    file = fopen(filename,"w");
    if(!file) { ErrorMessage("opeming for write",1); }

    // Write to file
    if(!vdcmode) { printf("Write to file.\n\r"); }
    if(!fwrite(&config,sizeof(config),1,file)) { fclose(file); ErrorMessage("writing",1); }

    // Close file
    if(!vdcmode) { printf("Close file.\n\r"); } else { ClearArea(0,24,80,1); }
    fclose(file);
}

void ReadConfigfile(unsigned char vdcmode) {
// Read config file

    FILE* file;

    if(!vdcmode) { printf("Open config file for reading.\n\r"); } else {
        printstrvdc(0,24,colorText,"Reading configuration file. Please wait.");
    }

    // Open file for read
    file = fopen(filename,"r");

    // Check if file exists, write one instead
    if(!file) { ClearArea(0,24,80,1); WriteConfigfile(vdcmode); return; }

    // Reading file
    if(!vdcmode) { printf("Reading from file.\n\r"); }
    if(!fread(&config,sizeof(config),1,file)) { fclose(file); ErrorMessage("reading",1); }

    // Close file
    if(!vdcmode) { printf("Close file.\n\r"); } else { ClearArea(0,24,80,1); }
    fclose(file);
}
