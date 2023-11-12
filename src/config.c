/*
CPMUMount
Disk mounter for the Ultimate II+ in CP/M
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

unsigned char selectionlines[] = {7, 9, 10, 11, 12, 13, 14, 18, 19, 20, 21};

void PrintTargetDrive(unsigned char target, unsigned char ypos)
{
    // Print target drive setting

    ClearArea(20, ypos, 20, 1);
    if (target)
    {
        sprintf(buffer, "%c", target - 1 + 'A');
    }
    else
    {
        sprintf(buffer, "Not set");
    }
    printstrvdc(20, ypos, colorText, buffer);
}

void PrintConfigData()
{
    // Print present config data

    unsigned char ypos = 3;
    unsigned char drive;

    sprintf(buffer, "Version: %s", version);
    printstrvdc(0, ypos++, colorText, buffer);
    printstrvdc(0, ypos++, colorText, "Present configuration.");

    ypos++;
    printstrvdc(0, ypos++, colorSuccess, "UMount settings:");
    sprintf(buffer, "Detection override: %s", config.auto_override ? "Yes" : "No");
    printstrvdc(0, ypos++, colorText, buffer);

    if (firmwareflag == 2)
    {
        sprintf(buffer, "Autodetection not supported in firmware.");
    }
    else
    {
        sprintf(buffer, "Autodetection of valid drives succeeded.");
    }
    printstrvdc(0, ypos++, colorText, buffer);

    for (drive = 0; drive < MAXDRIVES; drive++)
    {
        sprintf(buffer, " %c                  ", 'A' + drive);
        if (!validdrive[drive])
        {
            sprintf(buffer + 18, ": No target");
        }
        else
        {
            if (!config.auto_override)
            {
                sprintf(buffer + 18, ": %02d, %s %s", drive + 8, uii_device_tyoe(uii_devinfo[validdrive[drive] - 1].type), (uii_device_tyoe(uii_devinfo[validdrive[drive] - 1].power)) ? "On " : "Off");
            }
            else
            {
                sprintf(buffer + 18, ": %02d, manual", drive + 8);
            }
        }
        printstrvdc(0, ypos++, colorText, buffer);
    }

    printstrvdc(0, ypos, colorText, "Target drive      :");
    PrintTargetDrive(targetdrive, ypos++);

    ypos++;
    printstrvdc(0, ypos++, colorSuccess, "UTime settings:");
    printstrvdc(0, ypos++, colorText, "NTP host:");
    printstrvdc(0, ypos++, colorText, config.ntphost);
    sprintf(buffer, "UTC offset        : %ld", config.secondsfromutc);
    printstrvdc(0, ypos++, colorText, buffer);
    sprintf(buffer, "Update from NTP   : %sabled", config.ntpon ? "En" : "Dis");
    printstrvdc(0, ypos++, colorText, buffer);
    sprintf(buffer, "Verbose           : %sabled", config.verbose ? "En" : "Dis");
    printstrvdc(0, ypos++, colorText, buffer);
}

unsigned char SelectConfigOption(unsigned char select)
{
    // Selection of which config option to edit. 0 = exit

    unsigned char key, xpos, ypos, length;

    do
    {
        xpos = (select == 8) ? 0 : 20;
        ypos = selectionlines[select - 1];
        length = (select == 8) ? 80 : 12;
        fillattrvdc(xpos, ypos, length, colorSelect);
        key = cgetc();
        fillattrvdc(xpos, ypos, length, colorText);
        if (key == CURS_DOWN || key == CURU_DOWN)
        {
            if (select == 11)
            {
                select = 1;
            }
            else
            {
                select++;
            }
            if (select > 1 && select < 7 && !config.auto_override)
            {
                select = 7;
            }
        }
        if (key == CURS_UP || key == CURU_UP)
        {
            if (select == 1)
            {
                select = 11;
            }
            else
            {
                select--;
            }
            if (select > 1 && select < 7 && !config.auto_override)
            {
                select = 1;
            }
        }
    } while (key != K_RETURN && key != K_ESCAPE);

    if (key == K_RETURN)
    {
        return select;
    }
    else
    {
        return 0;
    }
}

void main()
{
    unsigned char select = 1;
    unsigned char changed = 0;
    unsigned char key, newval;
    char offsetinput[10] = "";
    char *ptrend;

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
    ReadConfigfile(1);

    // Get valid UII+ drives
    SetValidDrives();

    // Print present data
    PrintConfigData();

    do
    {
        // Select option to edit
        ClearArea(0, 24, 80, 1);
        printstrvdc(0, 24, colorText, "Select option to edit using Cursor Up and Down. RETURN to select, ESC to quit.");
        select = SelectConfigOption(select);
        ClearArea(0, 24, 80, 1);

        switch (select)
        {
        // Toggle manual override
        case 1:
            config.auto_override = !config.auto_override;
            changed = 1;
            SetValidDrives();
            ClearArea(0, 3, 80, 21);
            PrintConfigData();
            break;

        // Set drive validity manual
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            if (config.auto_override)
            {
                config.valid[select - 2] = !config.valid[select - 2];
                changed = 1;
                if (config.valid[select - 2])
                {
                    sprintf(buffer, "%02d, manual", select + 6);
                }
                else
                {
                    sprintf(buffer, "No target ");
                }
                printstrvdc(20, selectionlines[select - 1], colorSelect, buffer);
            }
            break;

        // Set target drive
        case 7:
            printstrvdc(0, 24, colorText, "Select target drive using Cursor Up and Down. RETURN to select, ESC to quit.");
            newval = config.target;
            do
            {
                key = cgetc();
                if (key == CURS_DOWN || key == CURU_DOWN)
                {
                    if (newval == MAXDRIVES)
                    {
                        newval = 0;
                    }
                    else
                    {
                        newval++;
                    }
                }
                if (key == CURS_UP || key == CURU_UP)
                {
                    if (newval == 0)
                    {
                        newval = MAXDRIVES;
                    }
                    else
                    {
                        newval--;
                    }
                }
                PrintTargetDrive(newval, selectionlines[select - 1]);
            } while (key != K_RETURN && key != K_ESCAPE);
            if (key = K_RETURN)
            {
                changed = 1;
                config.target = newval;
            }
            else
            {
                PrintTargetDrive(newval, selectionlines[select - 1]);
            }
            break;

        // Enter NTP server host name
        case 8:
            printstrvdc(0, 24, colorText, "Enter hostname of NTP server, ESC to quit.");
            if (textInput(0, selectionlines[select - 1], config.ntphost, 80) != -1)
            {
                changed = 1;
            }
            break;

        // Enter NTP offset to UTC
        case 9:
            printstrvdc(0, 24, colorText, "Enter NTP time offset in seconds to UTC, ESC to quit.");
            sprintf(offsetinput, "%ld", config.secondsfromutc);
            if (textInput(20, selectionlines[select - 1], offsetinput, 80) != -1)
            {
                config.secondsfromutc = strtol(offsetinput, &ptrend, 10);
                changed = 1;
            }
            break;

        // Toggle enable/disable update via NTP or enable/disable verbosity
        case 10:
        case 11:
            if (select == 10)
            {
                config.ntpon = !config.ntpon;
                newval = config.ntpon;
            }
            else
            {
                config.verbose = !config.verbose;
                newval = config.verbose;
            }
            changed = 1;
            sprintf(buffer, "%sabled ", newval ? "En" : "Dis");
            printstrvdc(20, selectionlines[select - 1], colorText, buffer);
            break;

        default:
            break;
        }
    } while (select != 0);

    if (changed)
    {
        WriteConfigfile(1);
    }

    // Exit program
    done(0);
}