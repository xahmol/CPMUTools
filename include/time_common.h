#ifndef __TIME_COMMON_H_
#define __TIME_COMMON_H_

// Functions to set time using NTP server
// Source: https://github.com/MaxPlap/ntp2ultimate

#define NTP_TIMESTAMP_DELTA 2208988800ul //0x83AA7E80

// Global variables
extern long secondsfromutc; 
extern unsigned char verbose;
extern unsigned char ntpon;
extern char host[81];
extern char configbuffer[95];
extern char filename[13];

#endif