/* $Header: /giga/HeliosRoot/Helios/servers/fdc/RCS/fdctest.c,v 1.4 1991/10/09 11:22:32 paul Exp $ */
/* $Source: /giga/HeliosRoot/Helios/servers/fdc/RCS/fdctest.c,v $ */
/************************************************************************/ 
/* fdctest.c - Tests of ARM Helios floppy device on functional 		*/
/*	       prototype						*/
/*									*/
/* Copyright (C) 1990 Active Book Company Ltd., Cambridge, England	*/
/*									*/
/* Author: Brian Knight, September 1990					*/
/************************************************************************/

#include <device.h>
#include <stdio.h>
#include <codes.h>
#include <stdlib.h>
#include <fault.h>
#include <string.h>
#include <process.h>
#include <abcARM/fproto.h>
#include <nonansi.h>

#include <dev/fdcdev.h>

#define DEVNAME "/files/helios/lib/fdcdev.d"
#define MAXCOM                25     /* Max number of commands */
#define ADDRESSING	    512     /* Blocksize to be used by driver	   */
#define BUFBLOCKS	      10
#define BUFSIZE	        (BUFBLOCKS * ADDRESSING)
#define DEFAULTPART	       0     /* Partition used when none specified */
#define DEFAULTXFERSIZE	       1     /* Read/write transfer size (blocks)  */
#define NCYLS		      80     /* Assume things about drive 	   */

/*----------------------------------------------------------------------*/

struct command
{
  char *name;
  void (*proc)();
} command_table[MAXCOM];


static int ncommands = 0;
static int finished = 0;
static unsigned char *buffer; 
static DCB *dcb;
static word sectorSize;

/*----------------------------------------------------------------------*/

/* Forward references */

void Error(char *mess);
void Action(DCB *dcb, DiscReq *req);
void PrintFault(char *prefix, int code);
void (*LookupCommand(char *name))(void);
void BuildCommandTable(void);
void AddCommand(char *name, void (*proc)());

/*----------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  DiscDevInfo   discInfo;
  PartitionInfo partInfo;
  DriveInfo     driveInfo;
  int		ibmFormat   = 0; /* IBM format disc */
  int		highDensity = 0;
  int 		arg;
  word		sectorsPerTrack;
  word		driveType = DT_MFM;
  word		helpNeeded = 0;

  printf("fdctest (%s %s)\n", __DATE__, __TIME__);

  buffer = (unsigned char *)Malloc(BUFSIZE);
  if (buffer == 0)
    { printf("failed to get buffer\n"); exit(1); }

  for (arg = 1; arg < argc; ++arg)
  {
    char *argWord = argv[arg];

    if (strcmp(argWord, "-ibm") == 0)
      { ibmFormat = 1; driveType |= DT_IBM; }
    else if (strcmp(argWord, "-hd") == 0)
      { highDensity = 1; driveType |= DT_HIGHDEN; }
    else if (strcmp(argWord, "-help") == 0)
      helpNeeded = 1;
    else
    {
      printf("argument `%s' ignored\n", argWord);
      helpNeeded = 1;
    }
  }

  if (helpNeeded)
     printf("usage: %s [-ibm] [-hd] [-help]\n", argv[0]);

  /* Deduce the number of sectors per track from the format and density */
  if (ibmFormat)
  {
    sectorSize      = 512;
    sectorsPerTrack = 9;
  }
  else
  {
    sectorSize	    = 1024;
    sectorsPerTrack = 5;
  }

  if (highDensity) sectorsPerTrack *= 2;

  printf("Treating disc as %s density %s format (%ld %ld-byte sectors/track)\n",
	 (highDensity ? "high" : "low"), (ibmFormat ? "IBM" : "ISO"),
	 sectorsPerTrack, sectorSize);

  /* Open device with one partition which is drive 0 */
  driveInfo.Next            =     -1; /* Example driver uses -1 for list end */
  driveInfo.DriveId         =      0;
  driveInfo.DriveType       = driveType;
  driveInfo.SectorSize      = sectorSize;
  driveInfo.SectorsPerTrack = sectorsPerTrack;
  driveInfo.TracksPerCyl    =      2;
  driveInfo.Cylinders       =  NCYLS;

  partInfo.Next        = -1; /* Example driver uses -1 for end of list */
  partInfo.Drive       =  0;
  partInfo.StartCyl    =  0;
  partInfo.EndCyl      = 79;
  partInfo.StartSector =  0;

  discInfo.Name       = ATOR("floppy");
  discInfo.Controller = FLOPPY_BASE;
  discInfo.Addressing = ADDRESSING; /* Used to set blockSize field of DCB */
  discInfo.Mode       = 0;
  discInfo.Drives     = (RPTR)&driveInfo;
  discInfo.Drives     = ATOR(discInfo.Drives);
  discInfo.Partitions = (RPTR)&partInfo;
  discInfo.Partitions = ATOR(discInfo.Partitions);

  dcb = OpenDevice(DEVNAME, &discInfo);
  if (dcb == 0) { Error("open failed"); exit(1); }

  BuildCommandTable(); 

  while (!finished)
  {
    char com[256], a1[256], a2[256], a3[256];
    char line[256];
    int nwords;
    
    printf("> "); fflush(stdout);
    gets(line);
    com[0] = 0; a1[0] = 0; a2[0] = 0; a3[0] = 0;
    nwords = sscanf(line, "%s %s %s %s", com, a1, a2, a3);

    if (nwords > 0)
    {
      void (*proc)() = LookupCommand(com);
      
      if ((int)proc) 
        (*proc)(com, a1, a2, a3);
      else
        printf("Unknown command '%s'\n", com);
    }
  }

  Free(buffer);
  CloseDevice(dcb);
  return 0;
}

/*----------------------------------------------------------------------*/

void 
AddCommand(char *name, void (*proc)(char *com, char *a1, char *a2, char *a3))
{
  struct command *next;

  if (ncommands >= MAXCOM)
  {
    printf("Too many commands (%d)\n", MAXCOM);
    exit(1);
  }
  
  next = &command_table[ncommands++];
  next->name = name;
  next->proc = proc;
}

/*----------------------------------------------------------------------*/

void 
(*LookupCommand(name))()
  char *name;
{
  int i;
  
  for (i=0; i<ncommands; i++)
    if (strcmp(name, command_table[i].name) == 0)
      return command_table[i].proc;
      
  return 0;
}

/*********************************************************************/
/* Command procs						     */
/*********************************************************************/

void 
HelpCom(char *com, char *a1, char *a2, char *a3)
{
  int i;
  com = com; a1 = a1; a2 = a2; a3 = a3;

  printf("fdctest commands are:\n");
  
  for (i=0; i<ncommands; i++)
    printf(" %s", command_table[i].name);
  printf("\n");
}

/*----------------------------------------------------------------------*/

void 
QuitCom(char *com, char *a1, char *a2, char *a3)
{
  com = com; a1 = a1; a2 = a2; a3 = a3;
  finished = 1;
}

/*----------------------------------------------------------------------*/

void EnableMotorCom(char *com, char *a1, char *a2, char *a3)
{
  DiscReq req;
  int     part = DEFAULTPART;

  com = com; a2 = a2; a3 = a3;

  if (a1[0]) sscanf(a1, "%d", &part);

  req.DevReq.Request   = FF_EnableMotor;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = part;

  /* rely on knowledge that driver is synchronous */
  Operate(dcb, &req);

  if (req.DevReq.Result < 0) 
    PrintFault("enable motor", (int)req.DevReq.Result);
}

/*----------------------------------------------------------------------*/

void DisableMotorCom(char *com, char *a1, char *a2, char *a3)
{
  DiscReq req;
  int     part = DEFAULTPART;

  com = com; a2 = a2; a3 = a3;

  if (a1[0]) sscanf(a1, "%d", &part);

  req.DevReq.Request   = FF_DisableMotor;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = part;

  /* rely on knowledge that driver is synchronous */
  Operate(dcb, &req);

  if (req.DevReq.Result < 0) 
    PrintFault("disable motor", (int)req.DevReq.Result);
}

/*----------------------------------------------------------------------*/

void ReadIdCom(char *com, char *a1, char *a2, char *a3)
{
  DiscReq req;
  int     part = DEFAULTPART;

  com = com; a2 = a2; a3 = a3;

  if (a1[0]) sscanf(a1, "%d", &part);

  req.DevReq.Request   = FF_ReadId;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = part;

  /* rely on knowledge that driver is synchronous */
  Operate(dcb, &req);

  if (req.DevReq.Result < 0) 
    PrintFault("read id", (int)req.DevReq.Result);
}

/*----------------------------------------------------------------------*/

void RecalibrateCom(char *com, char *a1, char *a2, char *a3)
{
  DiscReq req;
  int     part = DEFAULTPART;

  com = com; a2 = a2; a3 = a3;

  if (a1[0]) sscanf(a1, "%d", &part);

  req.DevReq.Request   = FF_Recalibrate;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = part;

  /* rely on knowledge that driver is synchronous */
  Operate(dcb, &req);

  if (req.DevReq.Result < 0) 
    PrintFault("recalibrate", (int)req.DevReq.Result);
}

/*----------------------------------------------------------------------*/

void SeekCom(char *com, char *a1, char *a2, char *a3)
{
  DiscReq req;
  int     part = DEFAULTPART;
  int	  cyl   = 0;

  com = com; a2 = a2; a3 = a3;

  if (a1[0]) sscanf(a1, "%d", &cyl);
  if (a2[0]) sscanf(a2, "%d", &part);

  printf("seeking to cyl %d on partition %d\n", cyl, part);

  req.DevReq.Request   = FF_Seek;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = part;
  req.Pos	       = cyl;   /* Not the same as read or write */

  /* rely on knowledge that driver is synchronous */
  Operate(dcb, &req);

  if (req.DevReq.Result < 0) 
    PrintFault("seek", (int)req.DevReq.Result);
}

/*----------------------------------------------------------------------*/

void ReadCom(char *com, char *a1, char *a2, char *a3)
{
  DiscReq req;
  int     part = DEFAULTPART;
  int     pos  = 1;
  int	  size = DEFAULTXFERSIZE;
  word    start, end;

  com = com; a2 = a2; a3 = a3;

  if (a1[0]) sscanf(a1, "%d", &pos);
  if (a2[0]) sscanf(a2, "%d", &size);
  if (a3[0]) sscanf(a3, "%d", &part);

  printf("reading %d blocks (%d bytes) from offset %d on partition %d\n",
         size, size*ADDRESSING, pos, part);

  req.DevReq.Request   = FG_Read;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = part;
  req.Pos	       = pos;
  req.Size	       = size;
  req.Buf	       = (byte *)0x740000; /* Use screen memory! */

  /* rely on knowledge that driver is synchronous */
  start = _cputime();
  Operate(dcb, &req);
  end   = _cputime();

  if (req.DevReq.Result < 0) 
    PrintFault("read", (int)req.DevReq.Result);
  else
    printf("read %ld bytes\n", req.Actual);

  printf("time %ld ms (%ld bytes/sec)\n", (end - start)*10,
	 (req.Actual * 100)/(end - start));
}

/*----------------------------------------------------------------------*/

void WriteCom(char *com, char *a1, char *a2, char *a3)
{
  DiscReq req;
  int     part = DEFAULTPART;
  int     pos  = 1;
  int	  size = DEFAULTXFERSIZE;
  word    start, end;

  com = com; a2 = a2; a3 = a3;

  if (a1[0]) sscanf(a1, "%d", &pos);
  if (a2[0]) sscanf(a2, "%d", &size);
  if (a3[0]) sscanf(a3, "%d", &part);

  printf("writing %d blocks (%d bytes) from offset %d on partition %d\n",
         size, size*ADDRESSING, pos, part);

  req.DevReq.Request   = FG_Write;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = part;
  req.Pos	       = pos;
  req.Size	       = size;
  req.Buf	       = (byte *)0x740000; /* Use screen memory! */

  /* rely on knowledge that driver is synchronous */
  start = _cputime();
  Operate(dcb, &req);
  end   = _cputime();

  if (req.DevReq.Result < 0) 
    PrintFault("write", (int)req.DevReq.Result);
  else
    printf("wrote %ld bytes\n", req.Actual);

  printf("time %ld ms (%ld bytes/sec)\n", (end - start)*10,
	 (req.Actual * 100)/(end - start));
}

/*----------------------------------------------------------------------*/

void ReadFileCom(char *com, char *a1, char *a2, char *a3)
{
  printf("not written yet\n");
}

/*----------------------------------------------------------------------*/

void WriteFileCom(char *com, char *a1, char *a2, char *a3)
{
  DiscReq req;
  char    *filename = a1;
  FILE    *s;
  int     part = DEFAULTPART;
  int     blockPos = 0;

  com = com; a2 = a2; a3 = a3;

  if (filename[0] == 0) 
  {
    printf("filename missing\n");
    return;
  }

  s = fopen(filename, "rb");
  if (s == 0)
  {
    printf("Can't open '%s' for input\n", filename);
    return;
  }

  printf("writing file '%s' to disc\n", filename);

  /* Write enough buffers to encompass entire file */
  while (!feof(s))
  {
    int got = fread(buffer, 1, BUFSIZE, s);

    printf("read %d bytes\n", got);

    req.DevReq.Request   = FG_Write;
    req.DevReq.Action    = Action;
    req.DevReq.SubDevice = part;
    req.Pos	         = blockPos;
    req.Size	         = BUFBLOCKS;
    req.Buf	         = buffer;

    Operate(dcb, &req);  /* rely on knowledge that driver is synchronous */

    if (req.DevReq.Result < 0) 
      PrintFault("writefile", (int)req.DevReq.Result);
    else
      printf("wrote %ld bytes\n", req.Actual);

    blockPos += BUFBLOCKS;
  }

  fclose(s);
}

/*----------------------------------------------------------------------*/

void FormatCom(char *com, char *a1, char *a2, char *a3)
{
  FormatReq req;
  int       part = DEFAULTPART;
  int 	    startCyl  = 0;
  int	    endCyl    = 79;
  int	    cylSkew   = 1; /* Seems to give best results */

  com = com; a2 = a2; a3 = a3;

  if (a1[0]) sscanf(a1, "%d", &startCyl);
  if (a2[0]) sscanf(a2, "%d", &endCyl);
  if (a3[0]) sscanf(a3, "%d", &cylSkew);

  printf("formatting cylinders %d to %d with cyl skew %d on partition %d\n",
         startCyl, endCyl, cylSkew, part);

  req.DevReq.Request   = FG_Format;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = part;
  req.StartCyl	       = startCyl;
  req.EndCyl	       = endCyl;
  req.Interleave       = 1;
  req.TrackSkew        = 0;
  req.CylSkew          = cylSkew;

  /* rely on knowledge that driver is synchronous */
  Operate(dcb, &req);

  if (req.DevReq.Result < 0) 
    PrintFault("format", (int)req.DevReq.Result);
  else
    printf("formatting complete\n");
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Repeat a simple command a large number of times to facilitate 	*/
/* looking at the FDC interface with an oscilloscope.			*/
/*----------------------------------------------------------------------*/
void RepeatCom(char *com, char *a1, char *a2, char *a3)
{
  FormatReq req;
  int 	    times = 50000;
  int	    i;

  com = com; a2 = a2; a3 = a3;

  if (a1[0]) sscanf(a1, "%d", &times);

  printf("repeating command %d times\n", times);

  req.DevReq.Request   = FF_SimpleCommand;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = DEFAULTPART;

  for (i = 0; i < times; ++i)
    Operate(dcb, &req);

  printf("done\n");
}

/*----------------------------------------------------------------------*/

void RattleCom(char *com, char *a1, char *a2, char *a3)
{
  DiscReq req;
  int     part = DEFAULTPART;
  int	  nSeeks = 100;
  int	  i;

  com = com; a2 = a2; a3 = a3;

  if (a1[0]) sscanf(a1, "%d", &nSeeks);
  if (a2[0]) sscanf(a2, "%d", &part);

  printf("doing %d random seeks on partition %d\n", nSeeks, part);

  req.DevReq.Request   = FF_Seek;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = part;

  for (i = 0; i < nSeeks; ++i)
  {
    req.Pos = ((word)rand() >> 1) % NCYLS; /* Not the same as read or write */
    Operate(dcb, &req); /* rely on knowledge that driver is synchronous */

    if (req.DevReq.Result < 0) 
      PrintFault("seek", (int)req.DevReq.Result);
  }
}

/*----------------------------------------------------------------------*/

void OscCom(char *com, char *a1, char *a2, char *a3)
{
  DiscReq req;
  int     part = DEFAULTPART;
  int	  nSeeks = 100;
  int	  i;

  com = com; a2 = a2; a3 = a3;

  if (a1[0]) sscanf(a1, "%d", &part);

  printf("doing oscillating seeks on partition %d\n", part);

  req.DevReq.Request   = FF_Seek;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = part;

  for (i = NCYLS-1; i != 0; i--)
  {
    req.Pos = i; /* Not the same as read or write */
    Operate(dcb, &req); /* rely on knowledge that driver is synchronous */

    if (req.DevReq.Result < 0) 
      PrintFault("seek", (int)req.DevReq.Result);

    req.Pos = NCYLS-i; /* Not the same as read or write */
    Operate(dcb, &req); /* rely on knowledge that driver is synchronous */

    if (req.DevReq.Result < 0) 
      PrintFault("seek", (int)req.DevReq.Result);
  }
}

/*----------------------------------------------------------------------*/
/* Bash the drive with a long sequence of operations			*/

void RunCom(char *com, char *a1, char *a2, char *a3)
{
  DiscReq req;
  int     part = DEFAULTPART;
  int	  nRuns = 10;
  int	  i,j;

  com = com; a3 = a3;

  if (a1[0]) sscanf(a1, "%d", &nRuns);
  if (a2[0]) sscanf(a2, "%d", &part);

  printf("doing running seeks on partition %d\n", part);

  req.DevReq.Request   = FF_Seek;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = part;

 for( j = 0 ; j < nRuns ; j++)
 {
  for (i = NCYLS-1; i != 0; i--)
  {
    req.Pos = i; /* Not the same as read or write */
    Operate(dcb, &req); /* rely on knowledge that driver is synchronous */

    if (req.DevReq.Result < 0) 
      PrintFault("seek", (int)req.DevReq.Result);
  }
  for (i = 0 ; i < NCYLS ; i++)
  {

    req.Pos = i; /* Not the same as read or write */
    Operate(dcb, &req); /* rely on knowledge that driver is synchronous */

    if (req.DevReq.Result < 0) 
      PrintFault("seek", (int)req.DevReq.Result);
  }
 }
}

/*----------------------------------------------------------------------*/
/* Bash the drive with a long sequence of operations			*/

void BashSeek()
{
#ifdef completed
  DiscReq req;

  req.DevReq.Request   = FF_Seek;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = part;
  req.Pos              = RansUpTo((word)rand() >> 1) % NCYLS; /* Not the same as read or write */
    Operate(dcb, &req); /* rely on knowledge that driver is synchronous */

    if (req.DevReq.Result < 0) 
      PrintFault("seek", (int)req.DevReq.Result);
  }
#endif
}

void BashCom(char *com, char *a1, char *a2, char *a3)
{
  DiscReq req;
  int     part = DEFAULTPART;
  int     times = 1000;
  int	  i;
  int	  size = DEFAULTXFERSIZE;

  com = com; a2 = a2; a3 = a3;
#ifdef completed

  if (a1[0]) sscanf(a1, "%d", &nOps);
  if (a2[0]) sscanf(a2, "%d", &part);

  printf("doing %d random disc operations on partition %d\n", times, part);

  srand(_cputime()); /* Initialise random number generator */

  for (i = 0; i < times; ++i)
  {
    int op = RandUpTo(2);

    switch (op)
    {
    case 0: BashRead();  break;
    case 1: BashWrite(); break;
    case 2: BashSeek();  break;
    }

    if (i % 100 == 0)
    {
      printf("\nafter %d operations:\n", i);
      PrintErrors();
    }
  }
#endif
}

/*----------------------------------------------------------------------*/

void GetSizeCom(char *com, char *a1, char *a2, char *a3)
{
  DiscReq req;
  int     part = DEFAULTPART;

  com = com; a2 = a2; a3 = a3;
  if (a1[0]) sscanf(a1, "%d", &part);

  req.DevReq.Request   = FG_GetSize;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = part;

  /* rely on knowledge that driver is synchronous */
  Operate(dcb, &req);

  if (req.DevReq.Result < 0) 
    PrintFault("get size", (int)req.DevReq.Result);
  else
    printf("size of partition %d is %ld\n", part, req.DevReq.Result);
}

/*----------------------------------------------------------------------*/

void ErrorsCom(char *com, char *a1, char *a2, char *a3)
{
  DiscReq     req;
  ErrorCounts e;
  int         part = DEFAULTPART;

  com = com; a2 = a2; a3 = a3;
  if (a1[0]) sscanf(a1, "%d", &part);

  req.DevReq.Request   = FF_ReadErrCounts;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = part;
  req.Buf	       = &e;

  /* rely on knowledge that driver is synchronous */
  Operate(dcb, &req);

  if (req.DevReq.Result < 0) 
    PrintFault("error counts", (int)req.DevReq.Result);
  else
  {
    printf("%d soft errors, %d hard errors\n",	e.softErrors, e.hardErrors);

    if (e.notReady)	
      printf("not ready:          %4d\n", e.notReady);	
    if (e.equipmentCheck)	
      printf("equipment check:    %4d\n", e.equipmentCheck);	
    if (e.dataError)	
      printf("data error:         %4d\n", e.dataError);
    if (e.overrun)	
      printf("overrun:            %4d\n", e.overrun);
    if (e.noData)	
      printf("no data:            %4d\n", e.noData);
    if (e.notWritable)	
      printf("not writable:       %4d\n", e.notWritable);
    if (e.missingAddressMark)	
      printf("missing addr mark:  %4d\n", e.missingAddressMark);
    if (e.controlMark)	
      printf("control mark:       %4d\n", e.controlMark);
    if (e.crcError)
      printf("CRC error:          %4d\n", e.crcError);
    if (e.wrongCylinder)	
      printf("wrong cylinder:     %4d\n", e.wrongCylinder);
    if (e.badCylinder)	
      printf("bad cylinder:       %4d\n", e.badCylinder);
    if (e.missingAddrMarkInData) 
      printf("missing AM in data: %4d\n", e.missingAddrMarkInData);
    if (e.seekNotComplete)
      printf("seek not complete:  %4d\n", e.seekNotComplete);
    if (e.seekToWrongCylinder)	
      printf("seek to wrong cyl:  %4d\n", e.seekToWrongCylinder);
    if (e.unknown)
      printf("unknown: %4d\n", e.unknown);
  }
}

/*----------------------------------------------------------------------*/

void 
Error(char *string)
{
 extern int errno;
 printf("%s, error %x\n",string, errno);
}

/*----------------------------------------------------------------------*/

void Action(DCB *dcb, DiscReq *req)
{
  dcb = dcb; req = req;
  /* printf("Action called back\n"); */
}

/*----------------------------------------------------------------------*/

#ifdef prototype
unsigned char 
ReadStatusReg(DCB *dcb, int chanNum, int regNum)
{
  DiscReq req;

  req.DevReq.Request   = FS_GetStatusReg;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = chanNum;
  req.Size    	       = regNum;

  /* rely on knowledge that driver is synchronous */
  Operate(dcb, &req);

  if (req.DevReq.Result < 0) PrintFault("read status", (int)req.DevReq.Result);
  return (unsigned char)req.Actual;
}

/*----------------------------------------------------------------------*/

unsigned char 
ReadControlReg(DCB *dcb, int chanNum, int regNum)
{
  FdcReq req;

  req.DevReq.Request   = FS_GetControlReg;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = chanNum;
  req.Size    	       = regNum;

  /* rely on knowledge that driver is synchronous */
  Operate(dcb, &req);

  if (req.DevReq.Result < 0) PrintFault("read status", (int)req.DevReq.Result);
  return (unsigned char)req.Actual;
}
#endif

/*----------------------------------------------------------------------*/

void PrintFault(char *prefix, int code)
{
  char mess[256];

  /* Fault(code, mess, 256);
  printf("%s: %s\n", prefix, mess); */

  printf("%s: fault %x", prefix, code);
  printf("\n");
}

/*----------------------------------------------------------------------*/

void BuildCommandTable(void)
{
  AddCommand("help",	 HelpCom);
  AddCommand("q",	 QuitCom);
  AddCommand("motoron",  EnableMotorCom);
  AddCommand("motoroff", DisableMotorCom);
  AddCommand("seek",	 SeekCom);
  AddCommand("rattle",   RattleCom);
  AddCommand("osc",      OscCom);
  AddCommand("run",      RunCom);
  AddCommand("getsize",	 GetSizeCom);
  AddCommand("read",	 ReadCom);
  AddCommand("write",	 WriteCom);
  AddCommand("readid",	 ReadIdCom);
  AddCommand("recal",	 RecalibrateCom);
  AddCommand("format",	 FormatCom);
  AddCommand("repeat",	 RepeatCom);
  AddCommand("errors",	 ErrorsCom);
  AddCommand("bash",     BashCom);
  AddCommand("writefile",WriteFileCom);
  AddCommand("readfile", ReadFileCom);
}

/*----------------------------------------------------------------------*/

/* End of fdctest.c */
