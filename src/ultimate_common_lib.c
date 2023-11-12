/*****************************************************************
Ultimate 64/II+ Command Library - Core functions
Scott Hutter, Francesco Sblendorio

Based on ultimate_dos-1.2.docx and command interface.docx
https://github.com/markusC64/1541ultimate2/tree/master/doc

Disclaimer:  Because of the nature of DOS commands, use this code
soley at your own risk.

Patches and pull requests are welcome
******************************************************************

Adapted for z88dk by Xander Mol, 2023 */

#include <string.h>
#include <stdlib.h>
#include "include/ultimate_common_lib.h"

#pragma code - name("BANKACCESS");

// Commented out as these need to be addressed as IO register via IN and OUT on Z80
// unsigned char *id_reg = (unsigned char *)ID_REG;
// unsigned char *cmddatareg = (unsigned char *)CMD_DATA_REG;
// unsigned char *controlreg = (unsigned char *)CONTROL_REG;
// unsigned char *statusreg = (unsigned char *)STATUS_REG;
// unsigned char *respdatareg = (unsigned char *)RESP_DATA_REG;
// unsigned char *statusdatareg = (unsigned char *)STATUS_DATA_REG;

char uii_status[STATUS_QUEUE_SZ + 1];
char uii_data[(DATA_QUEUE_SZ * 2) + 1];
char uii_command[128];
char temp_string_onechar[2];
int uii_data_index;
int uii_data_len;

unsigned char uii_target = TARGET_DOS1;
struct DevInfo uii_devinfo[4];

// Core functions
unsigned char uii_detect(void)
{
	// Detect present of UCI via ID_REG. Value should be $C9
	if (inp(ID_REG) == 0xc9)
	{
		// Reset UCI
		uii_abort();

		// Return 1 for detected = true
		return 1;
	}
	else
	{
		// Return 0 for detected = false
		return 0;
	}
}

void uii_settarget(unsigned char id)
{
	uii_target = id;
}

void uii_freeze(void)
{
	unsigned char cmd[] = {0x00, 0x05};

	uii_settarget(TARGET_CONTROL);

	uii_sendcommand(cmd, 2);
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_identify(void)
{
	unsigned char cmd[] = {0x00, DOS_CMD_IDENTIFY};
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(cmd, 2);
	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_echo(void)
{
	unsigned char cmd[] = {0x00, DOS_CMD_ECHO};
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(cmd, 2);

	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_getinterfacecount(void)
{
	unsigned char tempTarget = uii_target;
	unsigned char cmd[] = {0x00, NET_CMD_GET_INTERFACE_COUNT};

	uii_settarget(TARGET_NETWORK);
	uii_sendcommand(cmd, 0x02);

	uii_readdata();
	uii_readstatus();
	uii_accept();

	uii_target = tempTarget;
}

void uii_sendcommand(unsigned char *bytes, int count)
{
	int x = 0;
	int success = 0;

	bytes[0] = uii_target;

	while (success == 0)
	{
		// Wait for idle state
		while (inp(STATUS_REG) & 0x35)
		{
			;
		};

		// Write byte by byte to data register
		while (x < count)
			outp(CMD_DATA_REG, bytes[x++]);

		// Send PUSH_CMD
		outp(CONTROL_REG, inp(CONTROL_REG) | 0x01);

		// check ERROR bit.  If set, clear it via ctrl reg, and try again
		if ((inp(STATUS_REG) & 4) == 4)
		{
			outp(CONTROL_REG, inp(CONTROL_REG) | 0x08);
		}
		else
		{
			// check for cmd busy
			while (((inp(STATUS_REG) & 32) == 0) && ((inp(STATUS_REG) & 16) == 16))
			{
				;
			}
			success = 1;
		}
	}
}

void uii_accept(void)
{
	// Acknowledge the data
	outp(CONTROL_REG, inp(CONTROL_REG) | 0x02);
	while (!(inp(STATUS_REG) & 2) == 0)
	{
		;
	};
}

int uii_isdataavailable(void)
{
	if (((inp(STATUS_REG) & 128) == 128))
		return 1;
	else
		return 0;
}

int uii_isstatusdataavailable(void)
{
	if (((inp(STATUS_REG) & 64) == 64))
		return 1;
	else
		return 0;
}

void uii_abort(void)
{
	// abort the command
	outp(CONTROL_REG, inp(CONTROL_REG) | 0x04);
}

int uii_readdata(void)
{
	int count = 0;
	uii_data[0] = 0;

	// If there is data to read
	while (uii_isdataavailable() && count < DATA_QUEUE_SZ * 2)
	{
		uii_data[count++] = inp(RESP_DATA_REG);
	}
	uii_data[count] = 0;
	return count;
}

int uii_readstatus(void)
{
	int count = 0;
	uii_status[0] = 0;

	while (uii_isstatusdataavailable() && count < STATUS_QUEUE_SZ)
	{
		uii_status[count++] = inp(STATUS_DATA_REG);
	}

	uii_status[count] = 0;
	return count;
}