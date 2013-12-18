/*
 * $Header: /Chris/00/helios/msc/RCS/finddrv.c,v 2.0 91/08/21 18:06:22 chris
 * Exp Locker: chris $
 */

#include "msc.h"
#include <stdio.h>
#include <ctype.h>
#include <attrib.h>
#include <nonansi.h>
#include "rdevinfo.h"

#define puthex(c)	fputc (((c) < 10 ? (c) + '0' : (c) + '7'), stdout )
#define putchr(c)	fputc (((c) < 32 || c > 126 ? '.' : (c)), stdout )

char           *header = "\n\
xxxxxx | x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA cB xC xD xE xF | ascii\n\
-------+-------------------------------------------------+-----------------";

void
ShowPage (word start, word size, byte * data)
{
    word            end = start + size;
    word            line = start & ~0xf;
    word            pos;

    puts (header);
    for (line = start & ~0xf; line < end; line += 16) {
	pos = 0;
	fprintf (stdout, "%06x | ", line);
	for (pos = line; pos < line + 16; pos++) {
	    if (pos < start || pos >= end)
		fputs ("   ", stdout);
	    else {
		puthex (data[pos] >> 4);
		puthex (data[pos] & 0x0f);
		putchr (' ');
	    }
	}
	fputs ("| ", stdout);
	for (pos = line; pos < line + 16; pos++) {
	    if (pos < start || pos >= end)
		putchr (' ');
	    else
		putchr (data[pos]);
	}
	fputc ('\n', stdout);
    }
}

Attributes      oattr;
Attributes      nattr;
Stream         *input;

void
InitScreen (void)
{
    input = Heliosno (stdin);
    GetAttributes (input, &oattr);
    nattr = oattr;
    RemoveAttribute (&nattr, ConsoleEcho);
    RemoveAttribute (&nattr, ConsolePause);
    RemoveAttribute (&nattr, ConsoleIgnoreBreak);
    RemoveAttribute (&nattr, ConsoleBreakInterrupt);
    RemoveAttribute (&nattr, ConsoleRawOutput);
    AddAttribute (&nattr, ConsoleRawInput);
    SetAttributes (input, &nattr);
    setvbuf (stdin, NULL, _IONBF, 0);
}

void
TidyScreen (void)
{
    fputs ("\x09b25B\r\x9bJ", stdout);
    SetAttributes (input, &oattr);
}

DCB            *DevOpen (char *, DiscDevInfo *);
void            DevClose (DCB *);
void            DevOperate (DCB *, DiscReq *);

DiscDCB        *ddcb;
DiscReq         dreq;
RCB             request;
byte            Data[0x10000];


int
main (int argc, char **argv)
{
    char           *fname = argv [0];
    DiscDevInfo    *ddinfo = NULL;	/* disc device info		 */
    void           *devinfo;		/* devinfo pointer		 */
    InfoNode       *info;		/* devinfo node pointer		 */
    char           *driver;		/* name of device driver	 */
    word            retries;
    word            id;
    word            ok;

    if (argc != 2) {
	printf ("Usage: remote <MSCboard> %s <DiscDevice>\n", fname);
	return 1;
    }
/* get devinfo and driver name	 */
    if ((devinfo = load_devinfo ()) == NULL) {
	IOdebug ("%s: cannot find DevInfo !", fname);
	return 1;			/* I cannot proceed without..	 */
    }
    if ((info = find_info (devinfo, Info_DiscDev, argv[1])) == NULL) {
	IOdebug ("%s: cannot find %s entry in DevInfo !", fname, argv[1]);
	return 1;			/* I cannot proceed without..	 */
    }
    ddinfo = (DiscDevInfo *) RTOA (info->Info);
    driver = RTOA (ddinfo->Name);
    ddcb = (DiscDCB *) DevOpen (driver, ddinfo);
/* Load and initialise the	 */
/* Device driver.		 */
    if (ddcb == NULL) {			/* I cannot proceed without...	 */
	IOdebug ("%s: failed to open device driver %s !", fname, driver);
	return 1;
    }
    InitScreen ();

    memset (&request, 0, sizeof (RCB));
    memset (Data, 0, 0x10000);

    fputc ('\n', stdout);

    for (id = 0; id < 8; id++) {
	retries = 3;
again:
	request.Data = Data;
	request.Req = &dreq;
	SetID (request.DriveID, id);
	SetLUN (request.DriveID, 0);
	request.BlkSize = 512;
	request.Read = 1;
	request.Block = 0;
	dreq.DevReq.Timeout = -1;
	request.Regs[0] = 6;
	request.Regs[3] = 0x03;
	request.Regs[4] = 0x00;
	request.Regs[5] = 0x00;
	request.Regs[6] = 0x00;
	request.Regs[7] = 0x7f;
	request.Regs[8] = 0x00;
	request.Size = 127;

	printf ("\rChecking ID %d...", id);
	fflush (stdout);
	WriteWord (ddcb->Channels[0], (word) & request);
	ReadWord (ddcb->Channels[0], ok);
	if (request.Status < 0)
	    continue;
	if (request.Status && retries-- > 0)
	    goto again;

	request.Regs[3] = 0x12;
	request.Regs[4] = 0x00;
	request.Regs[5] = 0x00;
	request.Regs[6] = 0x00;
	request.Regs[7] = 0x80;
	request.Regs[8] = 0x00;
	request.Size = 128;

	printf ("\rChecking Device Identity...");
	fflush (stdout);
	WriteWord (ddcb->Channels[0], (word) & request);
	ReadWord (ddcb->Channels[0], ok);
	if (request.Status < 0)
	    continue;
	if (request.Status && retries-- > 0)
	    goto again;

	printf ("\rInquiry result for Device with ID %d :\n", id);
	ShowPage (0, 128 - request.sdp.Rest, Data);
	printf ("\nPress any key to continue...");
	fflush (stdout);
	ok = fgetc (stdin);
	printf ("\r\x09bK");
    }

    DevClose ((DCB *) ddcb);
    TidyScreen ();
}
