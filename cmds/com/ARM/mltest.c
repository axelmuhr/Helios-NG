/* $Header$ */
/* $Source$ */
/************************************************************************/ 
/* mltest.c - Tests of ARM Helios microlink interface.			*/
/*									*/
/* Copyright 1990 Active Book Company Ltd., Cambridge, England		*/
/*									*/
/* Author: Brian Knight, December 1990					*/
/************************************************************************/

/*
 * $Log$
 */
 
/*----------------------------------------------------------------------*/

#include <stdio.h>
#include <codes.h>
#include <stdlib.h>
#include <syslib.h>
#include <process.h>
#include <fault.h>
#include <string.h>
#include <posix.h>
#include <helios.h>
#include <abcARM/ABClib.h>

/*----------------------------------------------------------------------*/

#define MAXCOM  25 	/* Max number of commands     */

/*#define MAXMSGSIZE       (1 + 1 + 32)*/ /* Longest message in protocol */
#define MAXMSGSIZE       (1 + 1 + 64) /* Longest message in protocol */
#define NBUFS	16	/* Number of buffers for received messages	*/

#define sleep(seconds) Delay((seconds) * 1000000)

#define P(s) printf(s); fflush(stdout)

/*----------------------------------------------------------------------*/
/* Microlink message formats						*/

#define MLHDR_FORMAT	0x80	/* Format field				*/
#define MLHDR_LONGEXT	0x80	/* Long or extended message		*/
#define MLHDR_SHORT	0x00	/* Short message			*/

#define MLHDR_LONGTYPE	0xFC	/* Type field of long/ext message	*/
#define MLHDR_SHORTTYPE	0xF8	/* Type field of short message		*/

#define MLHDR_SHORTDATA	0x07	/* Data field of short message		*/

#define MLHDR_LENCODE	0x03	/* Length code field of long msg hdr	*/
#define MLHDR_LEN1	0x00	/* 1 byte follows			*/
#define MLHDR_LEN2	0x01	/* 2 bytes follow			*/
#define MLHDR_LEN4	0x02	/* 4 bytes follow			*/
#define MLHDR_LENEXT	0x03	/* Extended message: length in next byte */

/*#define MLEXT_LEN	0x1F*/	/* Length code field in 2nd byte of ext msg */
#define MLEXT_LEN	0x3F	/* Length code field in 2nd byte of ext msg */

/*----------------------------------------------------------------------*/

/* Structure used in circular buffer below. It allows the handler to	*/
/* record that messages were discarded because the circular buffer	*/
/* was full.								*/

typedef struct Buffer
{
  int   lostAfterThis;	 /* Number of messages discarded after this one */
  ubyte buf[MAXMSGSIZE]; /* Received message 		  		*/
} Buffer;
  
/* Structure used to pass information to message handler functions.	*/
/* The circular buffer is empty when nextPut == nextGet, full when	*/
/* nextPut is the slot before nextGet.					*/
/* Only the handler increments nextPut, and only the printing process	*/
/* increments nextGet. The semaphore is HardenedSignalled each time	*/
/* a new slot is filled.						*/

typedef struct HandlerArg
{
  Semaphore sem;	     /* To be HardenedSignalled when msg received   */
  int	    nextPut;	     /* Next circular buffer slot to be filled 	    */
  int 	    nextGet;	     /* Next circular buffer slot to be emptied	    */
  Buffer    buffers[NBUFS];  /* Circular buffer of received message buffers */
} HandlerArg;

/*----------------------------------------------------------------------*/

struct command
{
  char *name;
  void (*proc)();
} command_table[MAXCOM];

static int 	nCommands = 0;
static int	finished = 0;
static ubyte    *buffer; 

static HandlerArg handlerArg;       /* Shared between registered handlers */
static word       lastRxHandle = 0; /* Handle from last ML_SetUpRx	*/

/*----------------------------------------------------------------------*/

/* Forward and external references */

static void PrintFault(char *prefix, word code);
static void BuildCommandTable(void);
static void AddCommand(char *name, void (*proc)());
void (*LookupCommand(char *name))();
void PrintMessage(ubyte *buf);
void HandlerProcess(void);

/*----------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  printf("Microlink test (%s %s)\n", __DATE__, __TIME__);

  buffer = (unsigned char *)Malloc(MAXMSGSIZE);
  if (buffer == 0)
    { printf("failed to get buffer\n"); exit(1); }

  BuildCommandTable(); 
  
  /* Start a separate process to announce when a message handler	*/
  /* a message.								*/
  
  InitSemaphore(&handlerArg.sem, 0); /* Process sleeps on this */
  
  if (Fork(5000, HandlerProcess, 0, 0) < 0)
  {
    printf("failed to start child process\n");
    exit(1);
  }

  /* Main command loop */
   
  while (!finished)
  {
    char com[256], a1[256], a2[256], a3[256], a4[256], a5[256], a6[256];
    char line[256];
    int nwords;
    
    printf("> "); fflush(stdout);
    gets(line);
    com[0] = 0; 
    a1[0] = 0; a2[0] = 0; a3[0] = 0; a4[0] = 0; a5[0] = 0; a6[0] = 0;
    nwords = sscanf(line, "%s %s %s %s %s %s %s", com, a1, a2, a3, a4, a5, a6);

    if (nwords > 0)
    {
      void (*proc)() = LookupCommand(com);
      
      if ((int)proc) 
        (*proc)(com, a1, a2, a3, a4, a5, a6);
      else
        printf("Unknown command '%s'\n", com);
    }
  }

  Free(buffer);
  return 0;
}

/*----------------------------------------------------------------------*/

void AddCommand(char *name, 
		void (*proc)(char *com, char *a1, char *a2, char *a3,
			                char *a4, char *a5, char *a6))
{
  struct command *next;

  if (nCommands >= MAXCOM)
  {
    printf("Too many commands (%d)\n", MAXCOM);
    exit(1);
  }
  
  next = &command_table[nCommands++];
  next->name = name;
  next->proc = proc;
}

/*----------------------------------------------------------------------*/

void (*LookupCommand(char *name))()
{
  int i;
  
  for (i=0; i<nCommands; i++)
    if (strcmp(name, command_table[i].name) == 0)
      return command_table[i].proc;
      
  return 0;
}

/*----------------------------------------------------------------------*/

/* Body of the process which waits for handler functions to be called 	*/
void HandlerProcess(void)
{
  for (;;)
  {
    Buffer *bufStruct;

    HardenedWait(&handlerArg.sem); /* Wait for a handler function to run */
    
    /* There is now a buffer to print. (The semaphore alleviates the 	*/
    /* need to check for circular buffer underflow.)			*/

    bufStruct = &handlerArg.buffers[handlerArg.nextGet];

    printf("hdlr got msg: ");
    PrintMessage(bufStruct->buf);

    if (bufStruct->lostAfterThis > 0)
    {
      printf("***** %d messages discarded (buffer full) *****\n", 
	     bufStruct->lostAfterThis);
      bufStruct->lostAfterThis = 0;
    }

    if (++handlerArg.nextGet >= NBUFS) handlerArg.nextGet = 0; 
  }
}

/*********************************************************************/
/* Command procs						     */
/*********************************************************************/


void HelpCom(char *com, char *a1, char *a2, char *a3,
	                char *a4, char *a5, char *a6)
{
  int i;

  printf("mltest commands are:\n");
  
  for (i=0; i<nCommands; i++)
    printf(" %s", command_table[i].name);
  printf("\n");
}

/*----------------------------------------------------------------------*/

void QuitCom(char *com, char *a1, char *a2, char *a3,
	                char *a4, char *a5, char *a6)
{
  finished = 1;
}

/*----------------------------------------------------------------------*/

void TxCom(char *com, char *a1, char *a2, char *a3, 
	              char *a4, char *a5, char *a6)
{
  int i;
  ubyte txBuf[MAXMSGSIZE];
  int header = 0x82; /* Long message by default */
  int data1  = 0x01;
  int data2  = 0x02;
  int data3  = 0x03;
  int data4  = 0x04;
  word res;

  if (a1[0]) sscanf(a1, "%x", &header);
  if (a2[0]) sscanf(a2, "%x", &data1);
  if (a3[0]) sscanf(a3, "%x", &data2);
  if (a4[0]) sscanf(a4, "%x", &data3);
  if (a5[0]) sscanf(a5, "%x", &data4);

  txBuf[0] = header & 0xFF;
  txBuf[1] = data1 & 0xFF;
  txBuf[2] = data2 & 0xFF;
  txBuf[3] = data3 & 0xFF;
  txBuf[4] = data4 & 0xFF;

  for (i = 5; i < MAXMSGSIZE; ++i) txBuf[i] = i; /* put known values in buf */

  PrintMessage(txBuf);
  res = ML_Transmit(txBuf);
  if (res < 0) PrintFault("ML_Transmit", res);
}

/*----------------------------------------------------------------------*/

void TxManyCom(char *com, char *a1, char *a2, char *a3, 
	                  char *a4, char *a5, char *a6)
{
  int i;
  int times = 100;	/* Number of times to send message */
  ubyte txBuf[MAXMSGSIZE];
  int header = 0x82; /* Long message by default */
  int data1  = 0x01;
  int data2  = 0x02;
  int data3  = 0x03;
  int data4  = 0x04;
  word res;

  if (a1[0]) sscanf(a1, "%d", &times);
  if (a2[0]) sscanf(a2, "%x", &header);
  if (a3[0]) sscanf(a3, "%x", &data1);
  if (a4[0]) sscanf(a4, "%x", &data2);
  if (a5[0]) sscanf(a5, "%x", &data3);
  if (a6[0]) sscanf(a6, "%x", &data4);

  txBuf[0] = header & 0xFF;
  txBuf[1] = data1 & 0xFF;
  txBuf[2] = data2 & 0xFF;
  txBuf[3] = data3 & 0xFF;
  txBuf[4] = data4 & 0xFF;

  for (i = 5; i < MAXMSGSIZE; ++i) txBuf[i] = i; /* put known values in buf */

  printf("sending %d times: ", times);
  PrintMessage(txBuf);

  for (i = 0; i < times; ++i)
  {
    txBuf[5] = i & 0xFF; /* Increment the byte after the last settable one */
    res = ML_Transmit(txBuf);
    if (res < 0) 
      { PrintFault("ML_Transmit", res); break; }
  }
}

/*----------------------------------------------------------------------*/

/* Function used in all message handlers				*/
/* It relies on running indivisibly wrt other handlers and the printing	*/
/* process.								*/

void HandlerFunc(ubyte *buf, HandlerArg *arg)
{
  int i;
  int nextNextPut = arg->nextPut + 1;
  if (nextNextPut >= NBUFS) nextNextPut = 0;

  if (nextNextPut == arg->nextGet)
  {
    int prevPut = arg->nextPut - 1;

    if (prevPut < 0) prevPut = NBUFS - 1;
    ++arg->buffers[prevPut].lostAfterThis; /* Record discarded message */
  }
  else
  {
    ubyte *dest = arg->buffers[arg->nextPut].buf;

    /* Copy message into buffer in HandlerArg structure */
    for (i = 0; i < MAXMSGSIZE; ++i)
      *dest++ = *buf++;

    arg->nextPut = nextNextPut;
    HardenedSignal(&arg->sem); /* Signal only when new slot used */
  }
}

/* Register a new handler for received messages of a particular type	*/
/* This needs to allocate a new handler structure each time.		*/

void RegHandlerCom(char *com, char *a1, char *a2, char *a3, 
		              char *a4, char *a5, char *a6)
{
  ML_MsgHandler *handler;
  int		type = 0x80;
  word		res;

  if (a1[0]) sscanf(a1, "%x", &type);
  
  handler = (ML_MsgHandler *)Malloc(sizeof(ML_MsgHandler));
  if (handler == 0)
    { printf("Malloc failed\n"); return; }
    
  printf("setting up handler (at 0x%x) for message type 0x%02x\n",
  	 (int)handler, type);
  	 
  handler->msgType = type & 0xFF;
  handler->func    = HandlerFunc; /* Shared by all handlers		*/
  handler->arg     = &handlerArg; /* Shared (unsafely!) by all handlers */
  
  res = ML_RegisterHandler(handler);
  if (res < 0)
    PrintFault("ML_RegisterHandler", res);
}

/*----------------------------------------------------------------------*/

void RemHandlerCom(char *com, char *a1, char *a2, char *a3, 
		              char *a4, char *a5, char *a6)
{
  int  addr;
  word res;
  
  if (a1[0] == 0)
    { printf("must give handler address\n"); return; }
    
  sscanf(a1, "%x", &addr);
  
  res = ML_DetachHandler((ML_MsgHandler *)addr);
  if (res < 0)
    PrintFault("ML_RemHandler", res);
}

/*----------------------------------------------------------------------*/

void ResetCom(char *com, char *a1, char *a2, char *a3, 
	                 char *a4, char *a5, char *a6)
{
  ML_Reset();
}

/*----------------------------------------------------------------------*/

void SetUpRxCom(char *com, char *a1, char *a2, char *a3, 
		           char *a4, char *a5, char *a6)
{
  int  type = 0x84;
  word res;

  if (a1[0]) sscanf(a1, "%x", &type);

  printf("setting up rx for msg type 0x%x\n", type);
  res = ML_SetUpRx(buffer, type);
  if (res < 0)
    PrintFault("ML_SetUpRx", res);
  else
  {
    lastRxHandle = res;
    printf("handle 0x%lx\n", res);
  }
}

/*----------------------------------------------------------------------*/

void WaitRxCom(char *com, char *a1, char *a2, char *a3, 
	                  char *a4, char *a5, char *a6)
{
  word handle = lastRxHandle;
  word res;
  word timeout = -1;

  if (a1[0]) sscanf(a1, "%d", &timeout);

  printf("waiting on handle 0x%lx, timeout %ld\n", handle, timeout);

  res = ML_WaitForRx(handle, timeout);
  if (res < 0)
    PrintFault("ML_WaitForRx", res);
  else
    PrintMessage(buffer);
}

/*----------------------------------------------------------------------*/

/* Print details of microlink message in the supplied buffer */

void PrintMessage(ubyte *buf)
{
  int   i, len;
  ubyte hdr = buf[0];

  if ((hdr & MLHDR_FORMAT) == MLHDR_SHORT)
  {
    printf("short:");
    len = 1; 
  }
  else
  {
    if ((hdr & MLHDR_LENCODE) == MLHDR_LENEXT)
      printf("extended:");
    else
      printf("long:");

    switch (hdr & MLHDR_LENCODE)
    {
    case MLHDR_LEN1:   len = 2; break;
    case MLHDR_LEN2:   len = 3; break;
    case MLHDR_LEN4:   len = 5; break;
    case MLHDR_LENEXT: len = 2 + (buf[1] & MLEXT_LEN) + 1; break;
    }
  }

  for (i = 0; i < len; ++i) printf(" %02x", buf[i]);
  printf("\n");
}

/*----------------------------------------------------------------------*/

void PrintFault(char *prefix, word code)
{
  char *m;
  /* char mess[256]; */

  /* Fault(code, mess, 256); */
  /* printf("%s: %s\n", prefix, mess); */

  /* This should not really be done like this, but Fault seems to use	*/
  /* enormous amounts of stack.						*/
  printf("%s: ", prefix);
  switch(code)
  {
    default:
      printf(" fault %lx", code); return;
  }
  printf("%s\n", m);
}

/*----------------------------------------------------------------------*/

void BuildCommandTable(void)
{
  AddCommand("help",	HelpCom);
  AddCommand("q",	QuitCom);
  AddCommand("reghand",	RegHandlerCom);
  AddCommand("remhand", RemHandlerCom);
  AddCommand("reset",	ResetCom);
  AddCommand("setrx",	SetUpRxCom);
  AddCommand("tx",	TxCom);
  AddCommand("txmany",	TxManyCom);
  AddCommand("waitrx",	WaitRxCom);
}

/*----------------------------------------------------------------------*/

/* End of mltest.c */
