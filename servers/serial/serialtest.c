/* $Header: /hsrc/servers/serial/RCS/serialtest.c,v 1.1 1991/05/31 15:07:15 paul Exp $ */
/* $Source: /hsrc/servers/serial/RCS/serialtest.c,v $ */
/************************************************************************/ 
/* serialtest.c - Tests of ARM Helios serial server		 	*/
/*									*/
/* Copyright (C) 1990 Active Book Company Ltd., Cambridge, England	*/
/*									*/
/* Author: Brian Knight, 17th May 1990					*/
/************************************************************************/

#include <stdio.h>
#include <codes.h>
#include <stdlib.h>
#include <fault.h>
#include <string.h>
#include <process.h>
#include <attrib.h>
#include <gsp.h>
#include <ioevents.h>

#define DEVNAME		   "/rs232/default"
#define MAXCOM                25	/* Max number of commands */
#define BUFSIZE	           10000
#define DEFAULTSAMPLESIZE     20
#define DEFAULTLISTENSIZE     64
#define DEFAULTMOUSESIZE       9
#define LISTENSTACKSIZE	    2000	
#define WAITFOREVER	      -1

#define sleep(seconds) Delay((seconds) * 1000000)

/*----------------------------------------------------------------------*/

struct command
{
  char *name;
  void (*proc)();
} command_table[MAXCOM];


static int ncommands = 0;
static int finished = 0;
static unsigned char *buffer; 
static Stream *stream;

/* Scaling for stylus position */
static int stylusXMin =  2100;
static int stylusXMax = 12300;
static int stylusYMin =  1600;
static int stylusYMax =  7960;

/*----------------------------------------------------------------------*/

/* Forward references */

void Error(char *mess);
int SerialRead(Stream *stream, unsigned char *buf, int len, int timeout);
int SerialWrite(Stream *stream, unsigned char *buf, int len);
void PrintFault(char *prefix, int code);
void (*LookupCommand(char *name))(void);
void BuildCommandTable(void);
void AddCommand(char *name, void (*proc)());

/*----------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  Object *obj;

  printf("Serialtest 1.18 (%s)\n", __DATE__);
  argc = argc; argv = argv;
  buffer = (unsigned char *)Malloc(BUFSIZE);
  if (buffer == 0)
    { printf("failed to get buffer\n"); exit(1); }

  /* Open() doesn't seem to work with a NULL context, so we have to do	*/
  /* a Locate() first.							*/
  obj = Locate(CurrentDir, DEVNAME);
  if (obj == NULL) { printf("can't locate %s\n", DEVNAME); exit(1); }
  stream = Open(obj, NULL, O_ReadWrite);
  if (stream == 0) { printf("open of %s failed\n", DEVNAME); exit(1); }

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
  Close(stream);
  return 0;
}

/*----------------------------------------------------------------------*/

void AddCommand(char *name, 
		 void (*proc)(char *com, char *a1, char *a2, char *a3))
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

void (*LookupCommand(name))()
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


void HelpCom(char *com, char *a1, char *a2, char *a3)
{
  int i;
  com = com; a1 = a1; a2 = a2; a3 = a3;

  printf("Serialtest commands are:\n");
  
  for (i=0; i<ncommands; i++)
    printf(" %s", command_table[i].name);
  printf("\n");
}

/*----------------------------------------------------------------------*/

void QuitCom(char *com, char *a1, char *a2, char *a3)
{
  com = com; a1 = a1; a2 = a2; a3 = a3;
  finished = 1;
}

/*----------------------------------------------------------------------*/

/* Fork a process to print out whatever arrives on the receiver */

void ListenProc(int bufSize)
{
  printf("listening with buffer size %d\n", bufSize);
  
  for (;;)
  {
    int j;
  
    for (j=0; j<bufSize; ++j) buffer[j] = 0;

    if (SerialRead(stream, buffer, bufSize, WAITFOREVER) != bufSize)
      { Error("read error"); }

    for (j=0; j<bufSize; ++j)
    {
      char ch = buffer[j];
    
      if (ch == 13)
	printf("\n");
      else if ((ch < ' ') || (ch > 127))
        printf("[%x]", ch);
      else
        printf("%c", ch);
    }
    fflush(stdout);
  }
}

void ListenCom(char *com, char *a1, char *a2, char *a3)
{
  int bufSize;
  com = com; a2 = a2; a3 = a3;

  bufSize = DEFAULTLISTENSIZE;
  if (a1[0]) sscanf(a1, "%d", &bufSize);
  if (Fork(LISTENSTACKSIZE, ListenProc, sizeof(bufSize), bufSize) == 0)
    { PrintFault("Fork", 0); }
}

/*----------------------------------------------------------------------*/

/* Fork a process to print out whatever arrives on the receiver */

void HexListenProc(int bufSize, int timeout)
{
  int col = 0;

  printf("listening with buffer size %d, timeout %d\n", bufSize, timeout);
  
  for (;;)
  {
    int j, got;
  
    for (j=0; j<bufSize; ++j) buffer[j] = 0;

    got = SerialRead(stream, buffer, bufSize, timeout);
    if (got != bufSize)
    { 
      if (timeout != 0) Error("read error"); 
      printf("requested %d bytes, got %d\n", bufSize, got);
    }

    for (j=0; j<got; ++j)
    {
      char ch = buffer[j];
    
      printf(" %02x", ch);
      col += 3;
      if (col > 76) { printf("\n"); col = 0; }
    }
    fflush(stdout);
  }
}

void HexListenCom(char *com, char *a1, char *a2, char *a3)
{
  int bufSize, timeout;
  com = com; a3 = a3;

  bufSize = DEFAULTLISTENSIZE;
  if (a1[0]) sscanf(a1, "%d", &bufSize);
  timeout = WAITFOREVER;
  if (a2[0]) sscanf(a2, "%d", &timeout);
  if (Fork(LISTENSTACKSIZE, HexListenProc, 2*sizeof(int), bufSize, 
	   timeout) == 0)
    { PrintFault("Fork", 0); }
}

/*----------------------------------------------------------------------*/

/* Fork a process to listen to a 2 button serial mouse */

void Mouse2Proc(int bufSize, int timeout)
{
  printf("listening with buffer size %d, timeout %d\n", bufSize, timeout);
  
  for (;;)
  {
    int j, got;

    for (j=0; j<bufSize; ++j) buffer[j] = 0;

    got = SerialRead(stream, buffer, bufSize, timeout);
    if (got != bufSize)
    { 
      if (timeout != 0) Error("read error"); 
      printf("requested %d bytes, got %d\n", bufSize, got);
      return;
    }

    for (j=0; j<got; ++j)
    {
      char ch = buffer[j] & 0x7F;
    
      if (ch & 0x40) printf("\n");
      printf(" %02x", ch);
    }
    fflush(stdout);
  }
}

void Mouse2Com(char *com, char *a1, char *a2, char *a3)
{
  int bufSize, timeout;
  com = com; a3 = a3;

  bufSize = DEFAULTMOUSESIZE;
  if (a1[0]) sscanf(a1, "%d", &bufSize);
  timeout = WAITFOREVER;
  if (a2[0]) sscanf(a2, "%d", &timeout);
  if (Fork(LISTENSTACKSIZE, Mouse2Proc, 2*sizeof(int), bufSize, 
	   timeout) == 0)
    { PrintFault("Fork", 0); }
}

/*----------------------------------------------------------------------*/

/* Fork a process to listen to a 3 button Mouse Systems serial mouse */

void Mouse3Proc(int bufSize, int timeout)
{
  int x = 0, y = 0;
  unsigned char flags;
  signed char x1, x2, y1, y2;

  printf("listening with buffer size %d, timeout %d\n", bufSize, timeout);
  
  /* Get in phase with the 5-byte packets */
  do
  {
    SerialRead(stream, buffer, 1, WAITFOREVER);
  } while ((buffer[0] & 0xF0) != 0x80);

  SerialRead(stream, buffer, 4, WAITFOREVER); /* Swallow rest of packet */
  
  for (;;)
  {
    int j, got;
  
    for (j=0; j<bufSize; ++j) buffer[j] = 0;

    got = SerialRead(stream, buffer, bufSize, timeout);
    if (got != bufSize)
    { 
      if (timeout != 0) Error("read error"); 
      printf("requested %d bytes, got %d\n", bufSize, got);
      return;
    }

    for (j=0; j<got; ++j)
    {
      char ch = buffer[j];
    
      if ((ch & 0xF0) == 0x80) printf("\r");
      printf(" %02x", ch);
    }

    flags = buffer[0];
    x1    = buffer[1];
    y1    = buffer[2];
    x2    = buffer[3];
    y2    = buffer[4];

    x += x1 + x2; y += y1 + y2;
    printf("  x %5d  y %5d", x, y);
    fflush(stdout);
  }
}

void Mouse3Com(char *com, char *a1, char *a2, char *a3)
{
  int bufSize, timeout;
  Attributes attrs;

  com = com; a3 = a3;

  GetAttributes(stream, &attrs);
  RemoveAttribute(&attrs, RS232_IXON);  /* Do not steal incoming XON/XOFF */
  RemoveAttribute(&attrs, RS232_IXOFF); /* Do not issue XON/XOFF	  */
  SetInputSpeed(&attrs, RS232_B1200);
  AddAttribute(&attrs, RS232_Csize_8);
  SetAttributes(stream, &attrs);

  bufSize = 5;
  if (a1[0]) sscanf(a1, "%d", &bufSize);
  timeout = WAITFOREVER;
  if (a2[0]) sscanf(a2, "%d", &timeout);
  if (Fork(LISTENSTACKSIZE, Mouse3Proc, 2*sizeof(int), bufSize, 
	   timeout) == 0)
    { PrintFault("Fork", 0); }
}

/*----------------------------------------------------------------------*/

/* Fork a process to listen to mouse events */

void MouseEventsProc(int timeout)
{
  Port eventPort;

  printf("listening with timeout %d\n", timeout);
  
  eventPort = EnableEvents(stream, Event_Mouse);
  if (eventPort == NullPort)
  {
    printf("EnableEvents failed\n");
    goto MouseEventsExit;
  }

  for (;;)
  {
    BYTE    data[IOCDataMax];
    IOEvent *event;
    MCB     message;
    int     i, got, eventsGot;
    WORD    rc;
  
    message.Data        = &data[0];
    message.MsgHdr.Dest = eventPort;
    message.Timeout     = timeout;

    rc = GetMsg(&message);
    if (rc < 0)
    {
      printf("GetMsg failed %x\n", (int)rc);
      continue; /* Assume it was timeout! */
    }
    got = message.MsgHdr.DataSize;
    eventsGot = got/sizeof(IOEvent);
    printf("got %d bytes (%d events)\n", got, eventsGot);
    event = (IOEvent *)data;

    for (i = 0; i < eventsGot; ++i, ++event)
    {
      printf("X %d, Y %d, Buttons %x\n", event->Device.Mouse.X,
	     event->Device.Mouse.Y, (int)event->Device.Mouse.Buttons);
    }
    fflush(stdout);
  }

MouseEventsExit:
  Close(stream);
}

void MouseEventsCom(char *com, char *a1, char *a2, char *a3)
{
  int timeout;
  Attributes attrs;

  com = com; a3 = a3;

  GetAttributes(stream, &attrs);
  RemoveAttribute(&attrs, RS232_IXON);  /* Do not steal incoming XON/XOFF */
  RemoveAttribute(&attrs, RS232_IXOFF); /* Do not issue XON/XOFF	  */
  SetInputSpeed(&attrs, RS232_B1200);
  AddAttribute(&attrs, RS232_Csize_8);
  SetAttributes(stream, &attrs);

  timeout = WAITFOREVER;
  if (a1[0]) sscanf(a1, "%d", &timeout);
  if (Fork(LISTENSTACKSIZE, MouseEventsProc, sizeof(int), timeout) == 0)
    { PrintFault("Fork", 0); }
}

/*----------------------------------------------------------------------*/

/* Fork a process to listen to stylus events */

void StylusEventsProc(int timeout)
{
  Port eventPort;

  printf("listening with timeout %d\n", timeout);
  
  eventPort = EnableEvents(stream, Event_Stylus);
  if (eventPort == NullPort)
  {
    printf("EnableEvents failed\n");
    goto StylusEventsExit;
  }

  for (;;)
  {
    BYTE    data[IOCDataMax];
    IOEvent *event;
    MCB     message;
    int     i, got, eventsGot;
    WORD    rc;
  
    message.Data        = &data[0];
    message.MsgHdr.Dest = eventPort;
    message.Timeout     = timeout;

    rc = GetMsg(&message);
    if (rc < 0)
    {
      printf("GetMsg failed %x\n", (int)rc);
      continue; /* Assume it was timeout! */
    }
    got = message.MsgHdr.DataSize;
    eventsGot = got/sizeof(IOEvent);
    printf("got %d bytes (%d events)\n", got, eventsGot);
    event = (IOEvent *)data;

    for (i = 0; i < eventsGot; ++i, ++event)
    {
      printf("X %d, Y %d, Buttons %x\n", event->Device.Stylus.X,
	     event->Device.Stylus.Y, (int)event->Device.Stylus.Buttons);
    }
    fflush(stdout);
  }

StylusEventsExit:
  Close(stream);
}

void StylusEventsCom(char *com, char *a1, char *a2, char *a3)
{
  int timeout;
  Attributes attrs;

  com = com; a3 = a3;

  GetAttributes(stream, &attrs);
  RemoveAttribute(&attrs, RS232_IXON);  /* Do not steal incoming XON/XOFF */
  RemoveAttribute(&attrs, RS232_IXOFF); /* Do not issue XON/XOFF	  */
  SetAttributes(stream, &attrs);

  timeout = WAITFOREVER;
  if (a1[0]) sscanf(a1, "%d", &timeout);
  if (Fork(LISTENSTACKSIZE, StylusEventsProc, sizeof(int), timeout) == 0)
    { PrintFault("Fork", 0); }
}

/*----------------------------------------------------------------------*/
/* Plot a point at pixel coordinates (x,y)				*/

#define SCREENBASE 0x740000
#define LINEBYTES  256	/* Stride between lines of screen */
#define MAXX	   639
#define MAXY	   399

void Plot(int x, int y)
{
  unsigned char *byteAddr;

  if ((x < 0) || (x > MAXX) || (y < 0) || (y > MAXY)) return;

  byteAddr = (unsigned char *)(SCREENBASE + LINEBYTES*(MAXY-y) + x/8);
  *byteAddr |= 1 << (x & 7);
}

/*----------------------------------------------------------------------*/
/* Draw a line from (x1,y1) to (x2,y2)					*/

void Draw(int x1, int y1, int x2, int y2)
{
  int xDelta = x2 - x1;
  int yDelta = y2 - y1;
  int dx     = (xDelta < 0 ? -1 : 1);
  int dy     = (yDelta < 0 ? -1 : 1);
  int px     = abs(yDelta);
  int py     = abs(xDelta);
  int destx  = x1;
  int desty  = y1;
  int nots;

  Plot(destx, desty);
  if (py > px)
  { 
    int p = py/2;
    for (nots = 1; nots <= py; ++nots)
    {
      destx += dx;
      p -= px;
      if (p < 0)
	{ desty += dy; p += py; }
      Plot(destx, desty);
    } 
  }
  else
  {
    int p = px/2;
    for (nots = 1; nots <= px; ++nots)
    {
      desty += dy;
      p -= py;
      if (p < 0)
	{ destx += dx; p += px; }
      Plot(destx, desty);
    } 
  }
}

/*----------------------------------------------------------------------*/

#define GLITCH 20 /* Jump in x or y of > GLITCH pixels is ignored */

#define REDBUTTON	0x01
#define GREYBUTTON	0x02
#define BLUEBUTTON	0x04
#define BARRELSWITCH	0x08
#define TIPSWITCH   	0x10
#define STYLUSSWITCHES	(BARRELSWITCH | TIPSWITCH)
#define ALLSWITCHES	(REDBUTTON | GREYBUTTON | BLUEBUTTON | BARRELSWITCH | TIPSWITCH)
#define OUTOFPROX	0x40
#define PHASEFLAG	0x80

/* Fork a process to print out whatever arrives from a graphics tablet */

void TabletProc(int bufSize)
{
  Attributes attrs;
  int samples =  0;
  int cleared = 1;
  int lastPixelX = 0, lastPixelY = 0;
  unsigned char xOn = 0x11; /* XON character */

  printf("listening with buffer size %d packets\n", bufSize);
  bufSize *= 5;

  GetAttributes(stream, &attrs);
  RemoveAttribute(&attrs, RS232_IXON);  /* Do not steal incoming XON/XOFF */
  RemoveAttribute(&attrs, RS232_IXOFF); /* Do not issue XON/XOFF	  */
  SetAttributes(stream, &attrs);
  printf("%c\n", 12); /* clear screen */

  /* Send an XON just in case the digitiser is stuck */
  SerialWrite(stream, &xOn, 1);

  for (;;)
  {
    int j;

    *(int *)0x75C000 = 0xFFFF0000;
    if (SerialRead(stream, buffer, bufSize, WAITFOREVER) != bufSize)
      { Error("read error"); return; }
    *(int *)0x75C000 = 0x0000FFFF;

    /* Print the complete packets from this buffer */
    j=0; 
    while ((j < (bufSize-4)) && ((buffer[j] & PHASEFLAG) == 0)) ++j;

    for (; j < (bufSize-4); j += 5)
    {
      if (buffer[j] & PHASEFLAG)
      {
	int flags = buffer[j];
        int x = (buffer[j+2] << 7) + buffer[j+1];
        int y = (buffer[j+4] << 7) + buffer[j+3];
	int validpoint = (x != 0x3FFF) && (y != 0x3FFF);
	int pixelX = ((x - stylusXMin)*640)/(stylusXMax - stylusXMin);
	int pixelY = 399 - ((y - stylusYMin)*400)/(stylusYMax - stylusYMin);

	if ((flags & ALLSWITCHES) == ALLSWITCHES)
	{
          if (!cleared) printf("\f"); /* Clear screen */
	  cleared = 1;
	}
	else
	  cleared = 0;

	/* Print coords occasionally (slow!) */
        if (++samples%50 == 0)
	{
	  printf("\r%02x %5d %5d ", buffer[j], x, y);
	  printf(" %02x %02x %02x %02x", 
		 buffer[j+1], buffer[j+2], buffer[j+3], buffer[j+4]);
	  if (flags & REDBUTTON)    printf(" red") ;   else printf("    ");
	  if (flags & GREYBUTTON)   printf(" grey");   else printf("     ");
	  if (flags & BLUEBUTTON)   printf(" blue");   else printf("     ");
	  if (flags & TIPSWITCH)    printf(" tip") ;   else printf("    ");
	  if (flags & BARRELSWITCH) printf(" barrel"); else printf("       ");
	  if (flags & OUTOFPROX)    printf("     ");   else printf(" prox");
	}

	if (flags & BLUEBUTTON)
	{
	  /* Smooth the stream of points */
	  int dx = pixelX - lastPixelX;
	  int dy = pixelY - lastPixelY;
	  
	  if (dx > GLITCH)
	    dx = GLITCH;
	  else if (dx < -GLITCH)
	    dx = -GLITCH;

	  if (dy > GLITCH)
	    dy = GLITCH;
	  else if (dy < -GLITCH)
	    dy = -GLITCH;

	  pixelX = (5*lastPixelX + dx)/5;
	  pixelY = (5*lastPixelY + dy)/5;
	}

	/* Turn on one word of the screen if in proximity */
	if (flags & OUTOFPROX)
	  *(int *)0x758E4C = 0; /* Out of prox. */
	else
	  *(int *)0x758E4C = -1; /* In prox. */

	/* Draw only if tip or barrel switch is pressed */
	if (validpoint && (flags & STYLUSSWITCHES))
	{
	  if (flags & GREYBUTTON)
	    Draw(lastPixelX, lastPixelY, pixelX, pixelY);
	  else
	    Plot(pixelX, pixelY);
	}

	lastPixelX = pixelX; lastPixelY = pixelY;
      }
    }
    fflush(stdout);
  }

  GetAttributes(stream, &attrs);
  AddAttribute(&attrs, RS232_IXON);  /* Interpret incoming XON/XOFF */
  AddAttribute(&attrs, RS232_IXOFF); /* Issue XON/XOFF              */
  SetAttributes(stream, &attrs);
}

void TabletCom(char *com, char *a1, char *a2, char *a3)
{
  int bufSize;
  com = com; a2 = a2; a3 = a3;

  bufSize = 10;
  if (a1[0]) sscanf(a1, "%d", &bufSize);
  if (Fork(LISTENSTACKSIZE, TabletProc, sizeof(bufSize), bufSize) == 0)
    { PrintFault("Fork", 0); }
}

/*----------------------------------------------------------------------*/

void ReadCom(char *com, char *a1, char *a2, char *a3)
{
  int j, sample;
  com = com; a2 = a2; a3 = a3;

  sample = DEFAULTSAMPLESIZE;
  if (a1[0]) sscanf(a1, "%d", &sample);

  for (j=0; j<sample; ++j) buffer[j] = 0;

  if (SerialRead(stream, buffer, sample, WAITFOREVER) != sample)
    { Error("read error"); }

  {
    int j;
    for (j=0; j<10; ++j) printf(" %2x", buffer[j]);
    printf("\n");
  }
}

/*----------------------------------------------------------------------*/

#define DEFDATASIZE 2000 /* about 2 seconds at 9600 baud */

void WriteCom(char *com, char *a1, char *a2, char *a3)
{
  char *data;
  char defData[DEFDATASIZE+1];
  int  len, i;
  com = com; a2 = a2; a3 = a3;

  for (i=0; i<DEFDATASIZE; ++i) defData[i] = 0 /* ' ' + (i % 95)*/;
  defData[DEFDATASIZE] = 0;
  if (a1[0])
    { data = a1; len = strlen(data); }
  else
    { data = defData; len = DEFDATASIZE; }

  printf("writing %d bytes\n", len);
  if (SerialWrite(stream, (unsigned char *)data, len) != len)
    { Error("write error"); }
}

/*----------------------------------------------------------------------*/

Attribute AttrFromName(char *s)
{
  Attribute attr;

  if      (strcmp(s, "ParEnb")         == 0) attr = RS232_ParEnb;
  else if (strcmp(s, "ParOdd")         == 0) attr = RS232_ParOdd;
  else if (strcmp(s, "InPck")          == 0) attr = RS232_InPck;
  else if (strcmp(s, "IgnPar")         == 0) attr = RS232_IgnPar;
  else if (strcmp(s, "ParMrk")         == 0) attr = RS232_ParMrk;
  else if (strcmp(s, "Istrip")         == 0) attr = RS232_Istrip;
  else if (strcmp(s, "IXON")           == 0) attr = RS232_IXON;
  else if (strcmp(s, "IXOFF")  	       == 0) attr = RS232_IXOFF;
  else if (strcmp(s, "IgnoreBreak")    == 0) attr = RS232_IgnoreBreak;
  else if (strcmp(s, "BreakInterrupt") == 0) attr = RS232_BreakInterrupt;
  else if (strcmp(s, "HupCl")          == 0) attr = RS232_HupCl;
  else if (strcmp(s, "CLocal")         == 0) attr = RS232_CLocal;
  else if (strcmp(s, "Csize_5")        == 0) attr = RS232_Csize_5;
  else if (strcmp(s, "Csize_6")        == 0) attr = RS232_Csize_6;
  else if (strcmp(s, "Csize_7")        == 0) attr = RS232_Csize_7;
  else if (strcmp(s, "Csize_8")        == 0) attr = RS232_Csize_8;
  else if (strcmp(s, "Cstopb")         == 0) attr = RS232_Cstopb;
  else
  {
    if (strcmp(s, "") != 0) printf("unknown attribute '%s'\n", s);
    attr = 0;
  }

  return attr;
}

/*----------------------------------------------------------------------*/

void PrintAttributes(Attributes *attrs)
{
  printf("input speed %ld, output speed %ld\n", GetInputSpeed(attrs),
	 GetOutputSpeed(attrs));
  if (IsAnAttribute(attrs, RS232_ParEnb))  printf(" RS232_ParEnb");
  if (IsAnAttribute(attrs, RS232_ParOdd))  printf(" RS232_ParOdd");
  if (IsAnAttribute(attrs, RS232_InPck))   printf(" RS232_InPck");
  if (IsAnAttribute(attrs, RS232_IgnPar))  printf(" RS232_IgnPar");
  if (IsAnAttribute(attrs, RS232_ParMrk))  printf(" RS232_ParMrk");
  if (IsAnAttribute(attrs, RS232_Istrip))  printf(" RS232_Istrip");
  if (IsAnAttribute(attrs, RS232_IXON))    printf(" RS232_IXON");
  if (IsAnAttribute(attrs, RS232_IXOFF))   printf(" RS232_IXOFF");
  if (IsAnAttribute(attrs, RS232_IgnoreBreak)) printf(" RS232_IgnoreBreak");
  if (IsAnAttribute(attrs, RS232_BreakInterrupt))
    printf(" RS232_BreakInterrupt");
  if (IsAnAttribute(attrs, RS232_HupCl))   printf(" RS232_HupCl");
  if (IsAnAttribute(attrs, RS232_CLocal))  printf(" RS232_CLocal");
  if (IsAnAttribute(attrs, RS232_Csize_5)) printf(" RS232_Csize_5");
  if (IsAnAttribute(attrs, RS232_Csize_6)) printf(" RS232_Csize_6");
  if (IsAnAttribute(attrs, RS232_Csize_7)) printf(" RS232_Csize_7");
  if (IsAnAttribute(attrs, RS232_Csize_8)) printf(" RS232_Csize_8");
  if (IsAnAttribute(attrs, RS232_Cstopb))  printf(" RS232_Cstopb");

}

/*----------------------------------------------------------------------*/

void SetAttrCom(char *com, char *a1, char *a2, char *a3)
{
  Attributes attrs;
  word       res;

  com = com;
  res = GetAttributes(stream, &attrs);
  if (res < 0)
    { PrintFault("GetAttributes failed", (int)res); return; }

  AddAttribute(&attrs, AttrFromName(a1));
  AddAttribute(&attrs, AttrFromName(a2));
  AddAttribute(&attrs, AttrFromName(a3));

  res = SetAttributes(stream, &attrs);
  if (res < 0)
    { PrintFault("SetAttributes failed", (int)res); return; }
}

/*----------------------------------------------------------------------*/

void UnsetAttrCom(char *com, char *a1, char *a2, char *a3)
{
  Attributes attrs;
  word       res;

  com = com;
  res = GetAttributes(stream, &attrs);
  if (res < 0)
    { PrintFault("GetAttributes failed", (int)res); return; }

  RemoveAttribute(&attrs, AttrFromName(a1));
  RemoveAttribute(&attrs, AttrFromName(a2));
  RemoveAttribute(&attrs, AttrFromName(a3));

  res = SetAttributes(stream, &attrs);
  if (res < 0)
    { PrintFault("SetAttributes failed", (int)res); return; }
}

/*----------------------------------------------------------------------*/

void ShowAttrsCom(char *com, char *a1, char *a2, char *a3)
{
  Attributes attrs;
  word       res;

  com = com; a1 = a1; a2 = a2; a3 = a3;
  res = GetAttributes(stream, &attrs);
  if (res < 0)
    { PrintFault("GetAttributes failed", (int)res); return; }
  PrintAttributes(&attrs);
}

/*----------------------------------------------------------------------*/

int ValidNumber(char *s, int *value)
{
  int  v = 0;
  char ch = *s++;
  
  if (ch == 0) return 0; /* Don't accept null string */

  do
  {
    if ((ch < '0') || (ch > '9')) return 0;
    v = (v * 10) + ch - '0';
    ch = *s++;
  } while (ch != 0);

  *value = v;
  return 1;
}

/*----------------------------------------------------------------------*/

/* Converts the text name of a baud rate to an RS232 speed attribute.	*/
/* Returns 0 if invalid or unsupported speed.				*/

int SpeedToAttr(char *s, Attribute *attr)
{
  int speed;

  if (!ValidNumber(s, &speed)) return 0;

  switch (speed)
  {
    case    50: *attr = RS232_B50;	break;
    case    75: *attr = RS232_B75;	break;
    case   110: *attr = RS232_B110;	break;
    case   134: *attr = RS232_B134;	break;
    case   150: *attr = RS232_B150;	break;
    case   200: *attr = RS232_B200;	break;
    case   300: *attr = RS232_B300;	break;
    case   600: *attr = RS232_B600;	break;
    case  1200: *attr = RS232_B1200;	break;
    case  1800: *attr = RS232_B1800;	break;
    case  2400: *attr = RS232_B2400;	break;
    case  4800: *attr = RS232_B4800;	break;
    case  9600: *attr = RS232_B9600;	break;
    case 19200: *attr = RS232_B19200;	break;
    case 38400: *attr = RS232_B38400;	break;

    default: return 0;
  }

  return 1;
}

/*----------------------------------------------------------------------*/

void SetInSpeedCom(char *com, char *a1, char *a2, char *a3)
{
  Attributes attrs;
  word       speedAttr;
  word       res;

  com = com; a2 = a2; a3 = a3;

  if (!SpeedToAttr(a1, &speedAttr))
    { printf("invalid speed '%s'\n", a1); return; }

  res = GetAttributes(stream, &attrs);
  if (res < 0)
    { PrintFault("GetAttributes failed", (int)res); return; }

  SetInputSpeed(&attrs, speedAttr);

  res = SetAttributes(stream, &attrs);
  if (res < 0)
    { PrintFault("SetAttributes failed", (int)res); return; }
}

/*----------------------------------------------------------------------*/

void SetOutSpeedCom(char *com, char *a1, char *a2, char *a3)
{
  Attributes attrs;
  word       speedAttr;
  word       res;

  com = com; a2 = a2; a3 = a3;

  if (!SpeedToAttr(a1, &speedAttr))
    { printf("invalid speed '%s'\n", a1); return; }

  res = GetAttributes(stream, &attrs);
  if (res < 0)
    { PrintFault("GetAttributes failed", (int)res); return; }

  SetOutputSpeed(&attrs, speedAttr);

  res = SetAttributes(stream, &attrs);
  if (res < 0)
    { PrintFault("SetAttributes failed", (int)res); return; }
}

/*----------------------------------------------------------------------*/

/* Fork idle processes which update a count on the screen */

void CountProc(int procNum)
{
  int i, r = rand();

  for (i=0; ; ++i)
  {
    *(int *)(0x756000 + 0x100*procNum) = i;
    if ((i & 0xFFF) == 0)
    {
      Delay((word)r & (word)0xFFFF);
      r = rand();
    }
  }
}

void ProcsCom(char *com, char *a1, char *a2, char *a3)
{
  int nProcs, n;
  com = com; a2 = a2; a3 = a3;

  nProcs = 10;
  if (a1[0]) sscanf(a1, "%d", &nProcs);

  for (n=0; n<nProcs; ++n)
    if (Fork(LISTENSTACKSIZE, CountProc, sizeof(n), n) == 0)
      { PrintFault("Fork", 0); }
}

/*----------------------------------------------------------------------*/
/* Commands to adjust the stylus scaling parameters */

void PrintScaling()
{
  printf("xmin %d, xmax %d, ymin %d, ymax %d\n", 
	 stylusXMin, stylusXMax, stylusYMin, stylusYMax);
}

void XMinCom(char *com, char *a1, char *a2, char *a3)
{
  int new = stylusXMin;

  com = com; a2 = a2; a3 = a3;
  if (a1[0]) sscanf(a1, "%d", &new);

  if (new == stylusXMin)
    printf("XMin unchanged\n");
  else
    stylusXMin = new;

  PrintScaling();
}

void XMaxCom(char *com, char *a1, char *a2, char *a3)
{
  int new = stylusXMax;

  com = com; a2 = a2; a3 = a3;
  if (a1[0]) sscanf(a1, "%d", &new);

  if (new == stylusXMax)
    printf("XMax unchanged\n");
  else
    stylusXMax = new;

  PrintScaling();
}

void YMinCom(char *com, char *a1, char *a2, char *a3)
{
  int new = stylusYMin;

  com = com; a2 = a2; a3 = a3;
  if (a1[0]) sscanf(a1, "%d", &new);

  if (new == stylusYMin)
    printf("YMin unchanged\n");
  else
    stylusYMin = new;

  PrintScaling();
}

void YMaxCom(char *com, char *a1, char *a2, char *a3)
{
  int new = stylusYMax;

  com = com; a2 = a2; a3 = a3;
  if (a1[0]) sscanf(a1, "%d", &new);

  if (new == stylusYMax)
    printf("YMax unchanged\n");
  else
    stylusYMax = new;

  PrintScaling();
}

/*----------------------------------------------------------------------*/

void Error(char *string)
{
 extern int errno;
 printf("%s, error %x\n",string, errno);
}

/*----------------------------------------------------------------------*/

int SerialRead(Stream *stream, unsigned char *buf, int len, int timeout)
{
  int res = (int)Read(stream, (BYTE *)buf, len, timeout);

  if (res < 1)
  {
    printf("result from Read() is %d\n", res);
    return 0;
  }
  else
    return res;
}

/*----------------------------------------------------------------------*/

int SerialWrite(Stream *stream, unsigned char *buf, int len)
{
  int res = (int)Write(stream, (BYTE *)buf, len, -1);

  if (res < 1)
    printf("result from Write() is %d\n", res);

  return res;
}

/*----------------------------------------------------------------------*/

void PrintFault(char *prefix, int code)
{
  /* char mess[256]; */

  /* Fault(code, mess, 256); */
  /* printf("%s: %s\n", prefix, mess); */

  printf("%s: ", prefix);
  switch(code)
  {
    default:
      printf(" fault %x", code); break;
  }
  printf("\n");
}

/*----------------------------------------------------------------------*/

void BuildCommandTable(void)
{
  AddCommand("help",	    HelpCom);
  AddCommand("q",	    QuitCom);
  AddCommand("listen",	    ListenCom);
  AddCommand("hexlisten",   HexListenCom);
  AddCommand("tablet",	    TabletCom);
  AddCommand("mouse2",	    Mouse2Com); /* 2-button microsoft mouse */
  AddCommand("mouse3",	    Mouse3Com); /* 3-button Mouse Systems mouse */
  AddCommand("mouseev",	    MouseEventsCom);
  AddCommand("stylusev",    StylusEventsCom);
  AddCommand("read",	    ReadCom);
  AddCommand("write",	    WriteCom);
  AddCommand("setattr",     SetAttrCom);
  AddCommand("unsetattr",   UnsetAttrCom);
  AddCommand("showattrs",   ShowAttrsCom); 
  AddCommand("setinspeed",  SetInSpeedCom);
  AddCommand("setoutspeed", SetOutSpeedCom);
  AddCommand("procs",	    ProcsCom); /* Nothing to do with serial lines! */
  AddCommand("xmin",        XMinCom);
  AddCommand("xmax",        XMaxCom);
  AddCommand("ymax",        YMaxCom);
  AddCommand("ymin",        YMinCom);
}

/*----------------------------------------------------------------------*/

/* End of serialtest.c */
