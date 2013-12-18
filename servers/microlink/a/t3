/* $Source: /server/usr/users/b/charles/world/microlink/RCS/source/test/test3.c,v $ */
/* $Header: test3.c,v 1.2 91/01/14 17:21:16 charles Locked $ */

/*------------------------------------------------------------------------*/
/*                                         source/microlink/test/test3.c  */
/*------------------------------------------------------------------------*/

/* This test file is to allow the user to interactively send microlink    */
/*   messages and receive them using the "/microlink/general" server.     */
/* It allows multiple streams to this service to be opened.               */

/*------------------------------------------------------------------------*/
/*                                                          Header Files  */
/*------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdlib.h>
#include <syslib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <process.h>
#include <codes.h>
#include <fault.h>

#include <time.h>

#include "microlink/general.h"

/*------------------------------------------------------------------------*/
/*                                                                Macros  */
/*------------------------------------------------------------------------*/

#define GENNAME    "/microlink/general"
#define MAXSTREAMS 10

/*------------------------------------------------------------------------*/
/*                                                         Event records  */
/*------------------------------------------------------------------------*/

/* A linked list of all enabled events is set-up.                         */

typedef struct EvtRec
{  struct EvtRec *nxt;
   Port evp; 
   int  typ;
   int  req;      /* Kill-request flag        */
   Semaphore ack; /* Kill-request acknowledge */
} EvtRec;

EvtRec *evts = NULL;

/*------------------------------------------------------------------------*/
/*                                        Command table type description  */
/*------------------------------------------------------------------------*/

typedef struct ComEnt
{  char                  *cmdName;
   int  (*cmdFunc)(Stream*,char*);
   char                  *cmdDesc;
} ComEnt;

/*------------------------------------------------------------------------*/
/*                                               Look-ahead declarations  */
/*------------------------------------------------------------------------*/

       int  main           (int argc,char **argv);
static int  doHelp         (Stream *stm,char *ptr);
static int  doComment      (Stream *stm,char *ptr);
static int  doQuit         (Stream *stm,char *ptr);
static int  doIncludeFile  (Stream *stm,char *ptr);
static int  doTxRx         (Stream *stm,char *ptr);
static int  doEnableEvent  (Stream *stm,char *ptr);
static int  doDisableEvent (Stream *stm,char *ptr);
static int  doShow         (Stream *stm,char *ptr);
static void removeSpace    (char **ptr);
static int  ispfx          (char **ptr,char *cmd);
static void EventHandler   (EvtRec *er);
static void PrintMessage   (GeneralMessageBlock *msg);
static void PrintError     (word cd);

/*------------------------------------------------------------------------*/
/*                                                     The command table  */
/*------------------------------------------------------------------------*/

ComEnt comTab[] = 
{  { "?",      doHelp         , "Print command table"                     },
   { ";",      doComment      , "Add comment (to transcript)"             },
   { "<",      doIncludeFile  , "Input commands from a file"              },
   { "q",      doQuit         , "Quit this test program"                  },
   { "en",     doEnableEvent  , "Enable an event (give type)"             },
   { "dis",    doDisableEvent , "Disable an event (give type) 0=>all"     },
   { "txrx",   doTxRx         , "Packet xch (specify packet and rx type)" },
   { "show",   doShow         , "Show which events are enabled"           },
   { NULL,     NULL           , NULL                                      }
};

/*------------------------------------------------------------------------*/
/*                                                Transcript file-stream  */
/*------------------------------------------------------------------------*/

FILE *tf = NULL;
char b[1000];
#define outf(l) { sprintf l ; printf("%s",b); if(tf) fprintf(tf,"%s",b); }

Semaphore printSem;
int promptPresent;

/*------------------------------------------------------------------------*/
/*                                                     Indirection stack  */
/*------------------------------------------------------------------------*/

typedef struct FilRec
{  FILE *fh;
   struct FilRec *nxt;
} FilRec;

FilRec *fstk=NULL;

/*------------------------------------------------------------------------*/
/*                                                             main(...)  */
/*------------------------------------------------------------------------*/

int main(int argc,char **argv)
{  Stream        *stm;
   Object        *obj;
   char    inpl[1000];
   char          *ptr;
   char          *tfn;
   char      tft[256];
   int           argn;
   char          *arg;
   ComEnt        *cmd;

   printf
   (  "Test program for \"%s\" driver. %s %s\n",
      GENNAME,__DATE__,__TIME__
   );
   InitSemaphore(&printSem,0);
   promptPresent = 0;
   fstk = NULL;
   
   tfn = tft;
   sprintf(tft,"%s.log",argv[0]);
   for(argn=1;argn<argc;argn++)
   {  int err;
      err = 0;
      arg = argv[argn];
      if(*arg=='-')
      {  if(arg[1]=='t') tfn=arg+2;
         else err=1;
      } else err=1;
      if(err==1)
      {  printf("Format %s [-f<transcript-file-name]\n",argv[0]);
         exit(1);
      }
   }
   
   if(*tfn)
   {  time_t tim;
      printf("Transcript file name : %s\n",tfn);
      tf = fopen(tfn,"wb");
      if(tf==NULL)
      {  printf("Could not open transcript file\n");
         printf("Error %d ",errno); perror("");
         exit(1);
      }
      tim = time(NULL);
      fprintf(tf,"Transcript of %s test-session\n",argv[0]);
      fprintf
      (  tf,"This version of %s was compiled on %s %s\n",
         argv[0],__DATE__,__TIME__
      );
      fprintf
      (tf,"Helios thinks this test session commenced on %s\n",ctime(&tim));
      fprintf(tf,"\n-------------------\n");
   } else
   {  printf("No transcript file to use\n");  
   }
   
   obj = Locate(NULL,GENNAME);
   if(obj==NULL)
   {  outf((b,"Could not locate '%s'\n",GENNAME));
      exit(1);
   }
   
   stm = Open(obj,NULL,O_ReadWrite);
   if(stm==NULL)
   {  outf((b,"Could not open '%s'\n",GENNAME));
      exit(1);
   }

   for(;;)
   {  Signal(&printSem); Wait(&printSem);
      if(fstk)
      {  outf((b,"< "));
         if(fgets(inpl,1000,fstk->fh)==NULL)
         {  FilRec *rc;
            rc = fstk; fstk=rc->nxt;
            fclose(rc->fh);
            free(rc);
            outf((b,"; ***** File completed *****\n"));
            continue;
         }
         {  char *ptr;
            for(ptr=inpl;*ptr&&*ptr!='\n';ptr++);
            if(*ptr=='\n') *ptr=0;
         }
         outf((b,"%s\n",inpl));
      } else
      {  outf((b,"> ")); 
         fflush(stdout);
         promptPresent = 1;
         Signal(&printSem); gets(inpl); Wait(&printSem);
         promptPresent = 0;
         if(tf) fprintf(tf,"%s\n",inpl);
      }
      ptr = inpl;
      removeSpace(&ptr);
      if(*ptr==0) continue;
      for(cmd=comTab;cmd->cmdName;cmd++)
      {  char *lap;
         lap = ptr;
         if(ispfx(&lap,cmd->cmdName)&&*lap<=32)
         {  if((*cmd->cmdFunc)(stm,lap)==1) goto finishProgram;
            break;
         }
      }
      if(!cmd->cmdName) { outf((b,"Bad command name : Use '?' for help\n")); }
   }

   finishProgram:
   Close(obj);
   Close(stm);
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                           doHelp(...)  */
/*------------------------------------------------------------------------*/

static int doHelp(Stream *stm,char *ptr)
{  ComEnt *cmd;
   int i,wdth;
   
   ptr = NULL; /* Ignore */
   outf((b,"Help command for this test utility\n\n"));
   outf((b,"Command are : \n"));
   wdth = 0;
   for(cmd=comTab;cmd->cmdName;cmd++)
      if(strlen(cmd->cmdName)>wdth) wdth = strlen(cmd->cmdName);
   for(cmd=comTab;cmd->cmdName;cmd++)
   {  outf((b,"%s",cmd->cmdName));
      for(i=strlen(cmd->cmdName);i<wdth+2;i++) { outf((b," ")); }
      outf((b,"%s\n",cmd->cmdDesc));
   }
   outf((b,"\n"));
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                        doComment(...)  */
/*------------------------------------------------------------------------*/

static int doComment(Stream *stm,char *ptr)
{  stm = NULL;
   ptr = NULL;
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                           doQuit(...)  */
/*------------------------------------------------------------------------*/

static int doQuit(Stream *stm,char *ptr)
{  stm = NULL;
   ptr = NULL;
   return 1;
}

/*------------------------------------------------------------------------*/
/*                                                    doIncludeFile(...)  */
/*------------------------------------------------------------------------*/

static int doIncludeFile(Stream *stm,char *ptr)
{  FilRec *rc;

   stm = NULL;
   removeSpace(&ptr);
   rc = (FilRec*)calloc(1,sizeof(FilRec));
   if(rc==NULL)
   {  outf((b,"No memory\n"));
      return 1;
   }
   rc->nxt = fstk;
   rc->fh  = fopen(ptr,"rb");
   if(rc->fh==NULL)
   {  outf((b,"Could not open \"%s\" for reading\n",ptr));
      free(rc);
      return 0;
   }
   outf((b,"Including file \"%s\"\n",ptr));
   fstk = rc;
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                      removeSpace(...)  */
/*------------------------------------------------------------------------*/

static void removeSpace(char **ptr)
{  for(;**ptr<=32&&**ptr;(*ptr)++); }

/*------------------------------------------------------------------------*/
/*                                                           doTxRx(...)  */
/*------------------------------------------------------------------------*/

static int doTxRx(Stream *stm,char *ptr)
{  GeneralMlkRequest   rq;
   GeneralMlkReply     rp;
   word               res;
   int                  i;
   int                 bs;

   rq.txFlag = 1;
   for(i=0;i<34;i++) rq.msg.byt[i]=0;
   i = 0;
   bs = 10;
   while(1)
   {  removeSpace(&ptr);
      if(*ptr=='x'||*ptr=='X') { ptr++; bs=16; continue; }
      if(*ptr=='d'||*ptr=='D') { ptr++; bs=10; continue; }
      if(!isdigit(*ptr))
      {  if(bs==10) break;
         if(!((*ptr>='a'&&*ptr<='f')||(*ptr>='A'&&*ptr<='F'))) break;
      }
      if(i==34)
      {  outf((b,"More than 34 bytes in this message\n"));
         return 0;
      }
      rq.msg.byt[i] = (ubyte)strtol(ptr,&ptr,bs);
      i++;
   }

   if(*ptr!=';')
   {  if(*ptr==0)
      {  outf((b,"Type of return packet expected\n"));
         return 0;
      }
      outf((b,"Unexpected character 0x%02X in command line\n",*ptr));
      return 0;
   }
   
   ptr++;
   i=0;
   while(1)
   {  removeSpace(&ptr);
      if(*ptr=='x'||*ptr=='X') { ptr++; bs=16; continue; }
      if(*ptr=='d'||*ptr=='D') { ptr++; bs=10; continue; }
      if(!isdigit(*ptr))
      {  if(bs==10) break;
         if(!((*ptr>='a'&&*ptr<='f')||(*ptr>='A'&&*ptr<='F'))) break;
      }
      rq.rxType = (int)strtol(ptr,&ptr,bs);
      i=1;
      break;
   }
   
   if(i==0)
   {  outf((b,"Receive message type not specified\n"));
      return 0;
   }
   
   outf((b,"Message to be transmitted : \n"));
   
   PrintMessage(&rq.msg);
   
   outf
   ((b,"Expecting return message type %d = 0x%02X\n",rq.rxType,rq.rxType));

   res = SetInfo(stm,(void*)&rq,sizeof(GeneralMlkRequest));
   if(res!=0)
   {  outf((b,"Error number 0x%08X occured during SetInfo()\n",res));
      PrintError(res);
      return 0;
   }

   res = GetInfo(stm,(void*)&rp);
   if(res!=0)
   {  outf((b,"Error number 0x%08X occured during GetInfo()\n",res));
      PrintError(res);
      return 0;
   }

   outf
   (( b,
      "Return status is %d whilst return code is 0x%08X\n",
      rp.status,rp.code
   ));

   outf((b,"Returned message ...\n"));
   
   PrintMessage(&rp.msg);

   return 0;   
}

/*------------------------------------------------------------------------*/
/*                                                    doEnableEvent(...)  */
/*------------------------------------------------------------------------*/

static int doEnableEvent(Stream *stm,char *ptr)
{  int              evType;
   int                i,bs;
   EvtRec    *er,**el,*nev;

   i=0;
   while(1)
   {  removeSpace(&ptr);
      if(*ptr=='x'||*ptr=='X') { ptr++; bs=16; continue; }
      if(*ptr=='d'||*ptr=='D') { ptr++; bs=10; continue; }
      if(!isdigit(*ptr))
      {  if(bs==10) break;
         if(!((*ptr>='a'&&*ptr<='f')||(*ptr>='A'&&*ptr<='F'))) break;
      }
      evType = (int)strtol(ptr,&ptr,bs);
      i=1;
      break;
   }
   
   if(evType<=0)
   {  outf((b,"The event type must be positive\n"));
      return 0;
   }

   if(i==0)
   {  outf((b,"Expected an event type\n"));
      return 0;
   }

   if(*ptr!=0)
   {  outf((b,"Format : en <event-type>\n"));
      return 0;
   }
   
   nev = (EvtRec*)calloc(1,sizeof(EvtRec));
   if(nev==NULL)
   {  outf((b,"Out of memory"));
      return 0;
   }
   
   for(el=&evts;(er=*el)!=NULL;el=&er->nxt);
   *el = nev;
   
   nev->nxt = NULL;
   nev->req = 0;
   nev->typ = evType;
   nev->evp = EnableEvents(stm,evType);
   InitSemaphore(&nev->ack,0);
   
   outf((b,"Event with type 0x%02X enabled\n",evType));
   
   Fork(2000,EventHandler,4,nev);

   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                   doDisableEvent(...)  */
/*------------------------------------------------------------------------*/

static int doDisableEvent (Stream *stm,char *ptr)
{  int              evType;
   int                i,bs;
   EvtRec     *er,**el,*en;

   i=0;
   while(1)
   {  removeSpace(&ptr);
      if(*ptr=='x'||*ptr=='X') { ptr++; bs=16; continue; }
      if(*ptr=='d'||*ptr=='D') { ptr++; bs=10; continue; }
      if(!isdigit(*ptr))
      {  if(bs==10) break;
         if(!((*ptr>='a'&&*ptr<='f')||(*ptr>='A'&&*ptr<='F'))) break;
      }
      evType = (int)strtol(ptr,&ptr,bs);
      i=1;
      break;
   }

   if(evType<0)
   {  outf((b,"The event type must be positive (or zero for 'all events')\n"));
      return 0;
   }

   if(i==0)
   {  outf((b,"Expected an event type\n"));
      return 0;
   }

   if(*ptr!=0)
   {  outf((b,"Format : dis <event-type>\n"));
      return 0;
   }
   
   if(evType!=0)
   {  for(el=&evts;(er=*el)!=NULL;el=&er->nxt) if(er->typ==evType) break;
      if(er==NULL)
      {  outf((b,"No event of type 0x%02X has been enabled\n",evType));
         return 0;
      }
      outf((b,"Diabling event of type 0x%02X ...\n",evType));
      EnableEvents(stm,-(word)evType);  /* Disable events on port       */
      er->req = 1;                /* Send kill-request to event handler */
      AbortPort                   /* Unblock handler process            */
      (  er->evp,                 /* ... continued ...                  */
         EC_Error|EG_Timeout      /* ... continued ...                  */
      );                          /* ... continued                      */
      Signal(&printSem);          /* Allow handler to print messages    */
      Wait(&er->ack);             /* Wait for handler to die            */
      Wait(&printSem);            /* Regain right to print messages     */
      outf((b,"... Event Disabled and event process killed\n"));
      *el = er->nxt;              /* Unlink event record from list      */
      free(er);                   /* Free event record                  */
   } else
   {  outf((b,"Disabling all events ...\n"));
      EnableEvents(stm,0);
      for(er=evts;er;er=en)
      {  outf((b,"Disabling event of type 0x%02X\n",er->typ));
         er->req = 1;                /* Send kill-request to event handler */
         AbortPort                   /* Unblock handler process            */
         (  er->evp,                 /* ... continued ...                  */
   	         EC_Error|EG_Timeout /* ... continued ...                  */
         );                          /* ... continued                      */
         Signal(&printSem);          /* Allow handler to print messages    */
         Wait(&er->ack);             /* Wait for handler to die            */
         Wait(&printSem);            /* Regain right to print messages     */
         outf((b,"... Event Disabled and event process killed\n"));
         en = er->nxt;               /* Unlink event record from list      */
         free(er);                   /* Free event record                  */
      }
      evts = NULL;
   }
   
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                     EventHandler(...)  */
/*------------------------------------------------------------------------*/

static void EventHandler(EvtRec *er)
{  GeneralEvent          ge;
   MCB                  ecb;
   word                 res;
   GeneralMessageBlock *msg;

   for(;;)
   {  ecb.MsgHdr.Dest =  er->evp;
      ecb.Data        = (byte*) &ge;
      ecb.Control     = NULL;
      ecb.Timeout     =   -1;
      if(er->req) break;
      res = GetMsg(&ecb);
      if(er->req) break;
      if(res<0)
      {  Wait(&printSem);
         if(promptPresent) { outf((b,"\n")); }
         outf
         (( b,"Event %3d : GetMsg() returns with 0x%08X. Aborting\n",
            er->typ,res
         ));
         PrintError(res);
         Signal(&printSem);
         break;
      }
      if((res=ecb.MsgHdr.FnRc)<0)
      {  Wait(&printSem);
         if(promptPresent) { outf((b,"\n")); }
         outf
         (( b,"Event %3d : GetMsg() returns with 0x%08X. Aborting\n",
            er->typ,res
         ));
         PrintError(res);
         Signal(&printSem);
         break;
      }
      Wait(&printSem);
      if(promptPresent) { outf((b,"\n")); }
      outf((b,"Event 0x%02X : ",er->typ));
      msg = &ge.Device.microlink;
      
      PrintMessage(msg);

      Signal(&printSem);
   }

   Signal(&er->ack);
}

/*------------------------------------------------------------------------*/
/*                                                           doShow(...)  */
/*------------------------------------------------------------------------*/

static int doShow(Stream *stm,char *ptr)
{  EvtRec *er;
   int cnt;

   stm = NULL;
   ptr = NULL;

   outf((b,"The following event types are enabled : \n"));

   for(cnt=0,er=evts;er;er=er->nxt,cnt++) { outf((b,"0x%02X\n",er->typ)); }

   if(cnt==0) { outf((b,"(no events)\n")); }
   else       { outf((b,"(total of %d events enabled)\n",cnt)); }

   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                            ispfx(...)  */
/*------------------------------------------------------------------------*/

static int ispfx(char **ptr,char *cmd)
{  char *lap;

   for(lap=*ptr;*cmd&&*cmd==*lap;cmd++,lap++);
   if(*cmd) return 0;
   *ptr = lap;
   return 1;
}

/*------------------------------------------------------------------------*/
/*                                                     PrintMessage(...)  */
/*------------------------------------------------------------------------*/

static void PrintMessage(GeneralMessageBlock *msg)
{  int i,byts;

   byts = 1;
   if(msg->byt[0]&0x80)
   {  switch(msg->byt[0]&0x03)
      {  case 0:
            byts = 2; break;
         case 1:
            byts = 3; break;
         case 2:
            byts = 5; break;
         case 3:
            byts = msg->byt[1]+2;
            if(byts>34) byts=32;
      }
   }

   outf((b,"x  : "));
   for(i=0;i<byts;i++) outf((b,"  %02X ",msg->byt[i]));
   outf((b,"\n"));

   outf((b,"d  : "));
   for(i=0;i<byts;i++) outf((b,"%+4d ",(signed char)msg->byt[i]));
   outf((b,"\n"));

   outf((b,"ud : "));
   for(i=0;i<byts;i++) outf((b,"%4d ",(unsigned char)msg->byt[i]));
   outf((b,"\n"));

}

/*------------------------------------------------------------------------*/
/*                                                       PrintError(...)  */
/*------------------------------------------------------------------------*/

static void PrintError(word cd)
{  char buf[256];

   *buf = 0;
   Fault(cd,buf,255);
   outf((b,"Error description : %s\n",buf));
}
