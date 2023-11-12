/*****************************************************************
Ultimate 64/II+ Command Library - Time functions
Scott Hutter, Francesco Sblendorio

Based on ultimate_dos-1.2.docx and command interface.docx
https://github.com/markusC64/1541ultimate2/tree/master/doc

Disclaimer:  Because of the nature of DOS commands, use this code
soley at your own risk.

Patches and pull requests are welcome
******************************************************************/
#include <string.h>
#include "include/ultimate_common_lib.h"
#include "include/ultimate_time_lib.h"

#pragma code - name("BANKACCESS");

void uii_get_time(void)
{
	unsigned char cmd[] = {0x00, DOS_CMD_GET_TIME};
	uii_settarget(TARGET_DOS1);
	uii_sendcommand(cmd, 2);

	uii_readdata();
	uii_readstatus();
	uii_accept();
}

void uii_set_time(char *data)
{
	int x = 0;
	uii_command[0] = 0x00;
	uii_command[1] = DOS_CMD_SET_TIME;

	for (x = 0; x < 6; x++)
		uii_command[x + 2] = data[x];

	uii_settarget(TARGET_DOS1);
	uii_sendcommand((unsigned char *)uii_command, 8);

	uii_readstatus();
	uii_accept();
}