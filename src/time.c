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

-   Jay Cotton - inettools-z80
    https://github.com/jayacotton/inettools-z80
    Inspiration for time set and NTP code for Z80 / CP/M

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
#include "include/ultimate_network_lib.h"

#define NTP_TIMESTAMP_DELTA 2208988800l // 0x83AA7E80
#define CPM_SETDATETIME 104

struct NTPFormat
{
    uint8_t li_vn_mode;      // Byte 0:      Eight bits. li, vn, and mode.
                             //              li.   Two bits.   Leap indicator.
                             //              vn.   Three bits. Version number of the protocol.
                             //              mode. Three bits. Client will pick mode 3 for client.
    uint8_t stratum;         // Byte 1:      Eight bits. Stratum level of the local clock.
    uint8_t poll;            // Byte 2:      Eight bits. Maximum interval between successive messages.
    uint8_t precision;       // Byte 3:      Eight bits. Precision of the local clock.
    uint32_t rootDelay;      // Bytr 4-7:    32 bits. Total round trip delay time.
    uint32_t rootDispersion; // Byte 8-11:   32 bits. Max error aloud from primary clock source.
    uint32_t refId;          // Byte 12-15:  32 bits. Reference clock identifier.
    uint32_t refTm_s;        // Byte 16-23:  64 bits. Reference time-stamp seconds.
    uint32_t refTm_f;
    uint32_t origTm_s; // Byte 24-31:  64 bits. Originate time-stamp seconds.
    uint32_t origTm_f;
    uint32_t rcTm_s; // Byte 32-39:  64 bits. Received time-stamp seconds.
    uint32_t rcTm_f;
    uint32_t txTm_s; // Byte 40-47:  64 bits and the most important field the client cares
    uint32_t txTm_f;
}; // Total: 384 bits or 48 bytes.
struct NTPFormat ntp_packet;

// Get NTP time functions
unsigned char CheckStatusTime()
{
    // Function to check UII+ status

    if (uii_status[0] != '0' || uii_status[1] != '0')
    {
        printf("\nStatus: %s Data:%s", uii_status, uii_data);
        return 1;
    }
    return 0;
}

char *UNIX_epoch_to_UII_time(uint32_t epoch)
{
    // Convert UNIX time epoch to UII readable time format

    static unsigned char month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    unsigned char ntp_hour, ntp_minute, ntp_second, ntp_day, ntp_month;
    unsigned char leap_days = 0;
    unsigned char leap_year_ind = 0;
    unsigned int temp_days, i;
    unsigned int ntp_year, days_since_epoch, day_of_year;
    char settime[6];

    // printf("\nEpoch input: %lu",epoch);

    // Adjust for timezone
    epoch += config.secondsfromutc;
    // printf("\nOffset: %ld New epoch: %lu",config.secondsfromutc,epoch);

    // Calculate time
    ntp_second = (unsigned char)(epoch - (epoch / 60) * 60);
    epoch /= 60;
    // printf("\nSeconds: %u Epoch in minutes: %lu",ntp_second,epoch);

    ntp_minute = (unsigned char)(epoch - (epoch / 60) * 60);
    epoch /= 60;
    // printf("\nMinutes: %u Epoch in hours: %lu",ntp_minute,epoch);

    ntp_hour = (unsigned char)(epoch - (epoch / 24) * 24);
    epoch /= 24;
    // printf("\nHours: %u Epoch in days: %lu",ntp_hour,epoch);

    // Calculate date

    // Number of days since epoch
    days_since_epoch = epoch;
    // ball parking year, may not be accurate!
    ntp_year = 1970 + (days_since_epoch / 365);
    // Calculating number of leap days since epoch/1970
    for (i = 1972; i < ntp_year; i += 4)
    {
        if (((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0))
            leap_days++;
    }
    // Calculating accurate current year by (days_since_epoch - extra leap days)
    ntp_year = 1970 + ((days_since_epoch - leap_days) / 365);
    day_of_year = ((days_since_epoch - leap_days) % 365) + 1;

    if (((ntp_year % 4 == 0) && (ntp_year % 100 != 0)) || (ntp_year % 400 == 0))
    {
        month_days[1] = 29; // February = 29 days for leap years
        leap_year_ind = 1;  // if current year is leap, set indicator to 1
    }
    else
    {
        month_days[1] = 28;
    } // February = 28 days for non-leap years

    // Calculating current Month
    temp_days = 0;
    for (ntp_month = 0; ntp_month <= 11; ntp_month++)
    {
        if (day_of_year <= temp_days)
            break;
        temp_days = temp_days + month_days[ntp_month];
    }

    // Calculating current Date
    temp_days = temp_days - month_days[ntp_month - 1];
    ntp_day = day_of_year - temp_days;

    // Build UII time
    settime[0] = ntp_year - 1900;
    settime[1] = ntp_month;
    settime[2] = ntp_day;
    settime[3] = ntp_hour;
    settime[4] = ntp_minute;
    settime[5] = ntp_second;

    return settime;
}

uint32_t Reverse32(uint32_t value)
{
    // Reverse endianness

    return (((value & 0x000000FF) << 24) |
            ((value & 0x0000FF00) << 8) |
            ((value & 0x00FF0000) >> 8) |
            ((value & 0xFF000000) >> 24));
}

uint32_t ConvertTS(uint32_t timestamp)
{
    // Convert NTP timestamp to UNIX epoch

    return Reverse32(timestamp) - NTP_TIMESTAMP_DELTA;
}

void get_ntp_time()
{
    // Function to get time from NTP server and set UII+ time with this

    unsigned char attempt = 1;
    unsigned char clock;
    unsigned char fullcmd[] = {0x00, NET_CMD_SOCKET_WRITE, 0x00,
                               0x1b, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char socket = 0;
    unsigned long t;

    if (config.verbose)
    {
        ("\nUpdating UII+ time from NTP Server.");
    }

    uii_get_time();

    if (config.verbose)
    {
        printf("\nUltimate datetime: %s", uii_data);
        printf("\nConnecting to: %s", config.ntphost);
    }

    socket = uii_udpconnect(config.ntphost, 123); // https://github.com/markusC64/1541ultimate2/blob/master/software/io/network/network_target.cc
    if (CheckStatusTime())
    {
        uii_socketclose(socket);
        return;
    }

    if (config.verbose)
    {
        printf("\nSending NTP request");
    }

    fullcmd[2] = socket;
    uii_settarget(TARGET_NETWORK);
    uii_sendcommand(fullcmd, 51); // 3 + sizeof( ntp_packet ));
    uii_readstatus();
    uii_accept();
    if (CheckStatusTime())
    {
        uii_socketclose(socket);
        return;
    }

    // Do maximum of 4 attempts at receiving data
    do
    {
        // Add delay of a second to avoid time to wait on response being too short
        delay(10);

        // Print attempt number
        if (config.verbose)
        {
            printf("\nReading result attempt %d", attempt);
        }

        // Try to read incoming data
        uii_socketread(socket, 50); // 2 + sizeof( ntp_packet ));

        // If data received, end loop. Else do new attempt till counter = 5
        if (uii_success())
        {
            attempt = 5;
        }
        else
        {
            attempt++;
        }

    } while (attempt < 5);

    if (CheckStatusTime())
    {
        uii_socketclose(socket);
        return;
    }

    memcpy(&ntp_packet, &uii_data + 2, 48);

    // printf("\nLI VN MODE: %u",ntp_packet.li_vn_mode);
    // printf("\nStratum   : %u",ntp_packet.stratum);
    // printf("\nPol       : %u",ntp_packet.poll);
    // printf("\nPrecision : %u",ntp_packet.precision);
    // printf("\nRoot delay: %lu",ConvertTS(ntp_packet.rootDelay));
    // printf("\nRoot disp : %lu",ConvertTS(ntp_packet.rootDispersion));
    // printf("\nrefId     : %lu",ConvertTS(ntp_packet.refId));
    // printf("\nrefTm_s   : %lu",ConvertTS(ntp_packet.refTm_s));
    // printf("\nrefTm_f   : %lu",ConvertTS(ntp_packet.refTm_f));
    // printf("\norigTm_s  : %lu",ConvertTS(ntp_packet.origTm_s));
    // printf("\nOrigTm_f  : %lu",ConvertTS(ntp_packet.origTm_f));
    // printf("\nrcTm_s    : %lu",ConvertTS(ntp_packet.rcTm_s));
    // printf("\nrcTm_f    : %lu",ConvertTS(ntp_packet.rcTm_f));
    // printf("\ntxTm_s    : %lu",ConvertTS(ntp_packet.txTm_s));
    // printf("\ntxTm_f    : %lu",ConvertTS(ntp_packet.txTm_f));

    // Convert time received to UCI format
    // Bytes 32-35 of NTP packet, rcTm_s = received time in seconds, Big Endian order

    // t = uii_data[37] | (((unsigned long)uii_data[36])<<8)| (((unsigned long)uii_data[35])<<16)| (((unsigned long)uii_data[34])<<24);
    // t -= NTP_TIMESTAMP_DELTA;

    t = ConvertTS(ntp_packet.rcTm_s);

    // Close socket
    uii_socketclose(socket);

    // Print time received and parse to UII+ format
    if (config.verbose)
    {
        printf("\nUnix epoch %lu", t);
    }

    // Set UII+ RTC clock
    uii_set_time(UNIX_epoch_to_UII_time(t));

    if (config.verbose)
    {
        printf("\nStatus: %s", uii_status);
    }

    uii_get_time();

    if (config.verbose)
    {
        printf("\nRTC clock set to %s", uii_data);
    }
}

unsigned char int2bcd(unsigned char input)
{
    // Conversion of an integer number to BCD
    // Adapted from source: https://github.com/jayacotton/inettools-z80/

    unsigned char high = 0;

    while (input >= 10)
    {
        high++;
        input -= 10;
    }

    return ((high << 4) | input);
}

unsigned char isleap(unsigned int year)
{
    // Calculate if year is leap year
    return (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0));
}

void CPMSetDateTime(unsigned char seconds, unsigned char minutes, unsigned char hour, unsigned char day, unsigned char month, unsigned int year)
{
    // Set date and time in CP/M OS
    // Adapted from source: https://github.com/jayacotton/inettools-z80/

    /* year	- real years binary
       month - real month binary
       day	 - real day binary		@DATE   jan 1 1978
       hour	- hour (24 hr clock) binary	@HOUR	bcd
       minute  - minute binary		@MIN	bcd
       second - second binary		@SEC	bcd
    */

    unsigned char bcd_buffer[8];
    unsigned char mtab[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // Days per month
    unsigned int y;
    unsigned int c_leaps;
    unsigned int c_noleaps;
    unsigned int days;

    c_leaps = 0;
    c_noleaps = 0;

    /* run through all the years from 1978 and count up leap years */
    for (y = 1978; y <= year; y++)
    {
        if (isleap(y))
        {
            c_leaps++;
        }
    }
    c_noleaps = (year - 1978) - c_leaps;

    /* compute days since jan 1 1978 */

    /* days for the years */
    days = (c_noleaps * 365) + (c_leaps * 366);

    /* days for this week */
    days += day;

    /* days for all the months this year */
    for (y = 1; y <= month - 1; y++)
    {
        days += mtab[y];
    }

    /* deal with possible leap year this year */
    if (isleap(year))
    {
        days++;
    }

    if (config.verbose)
    {
        printf("\nCP/M: days since Jan 1, 1978: %u", days);
    }

    /* set up the set time buffer for CP/M 3 */
    bcd_buffer[0] = days & 0xff; /* low high order ? */
    bcd_buffer[1] = ((days >> 8) & 0xff);
    bcd_buffer[2] = int2bcd(hour);
    bcd_buffer[3] = int2bcd(minutes);
    bcd_buffer[4] = int2bcd(seconds);
    bdos(CPM_SETDATETIME, (int)&bcd_buffer); // BDOS function call 104: Set date and Time
}

void timeSynch()
{
    // Synch system time with UII+ RTC
    char *ptrend;
    unsigned int year;
    unsigned char month, day, hour, minutes, seconds;

    // Get UII+ RTC time
    uii_get_time();

    if (!CheckStatusTime())
    {
        if (config.verbose)
        {
            printf("\nGet UII+ RTC. Time: %s", uii_data);
        }

        // Copy year
        strlcpy(buffer, uii_data, 5);
        buffer[4] = 0;
        year = strtol(buffer, &ptrend, 10);

        // Copy month
        buffer[0] = uii_data[5];
        buffer[1] = uii_data[6];
        buffer[2] = 0;
        month = strtol(buffer, &ptrend, 10);

        // Copy day
        buffer[0] = uii_data[8];
        buffer[1] = uii_data[9];
        day = strtol(buffer, &ptrend, 10);

        // Copy hour
        buffer[0] = uii_data[11];
        buffer[1] = uii_data[12];
        hour = strtol(buffer, &ptrend, 10);

        // Copy minutes
        buffer[0] = uii_data[14];
        buffer[1] = uii_data[15];
        minutes = strtol(buffer, &ptrend, 10);

        // Copy seconds
        buffer[0] = uii_data[17];
        buffer[1] = uii_data[18];
        seconds = strtol(buffer, &ptrend, 10);

        // Set CP/M system time
        CPMSetDateTime(seconds, minutes, hour, day, month, year);

        if (config.verbose)
        {
            printf("\nDate and time set.");
        }
    }
}

// Main program
void main()
{

    // Read config file
    ReadConfigfile(0);

    // If enabled, get NTP time
    if (config.ntpon)
    {
        get_ntp_time();
    }

    // Synch CP/M with UII+ RTC
    timeSynch();
}