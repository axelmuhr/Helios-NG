/* $Source: /server/usr/users/b/charles/world/microlink/RCS/source/test/test4.c,v $ */
/* $Header: test4.c,v 1.2 91/01/31 13:51:32 charles Locked $ */

/*------------------------------------------------------------------------*/
/*                                         source/microlink/test/test4.c  */
/*------------------------------------------------------------------------*/

/* This ia a test-harness for the 'phone line interface functions.        */

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

#include "microlink/phone.h"

/*------------------------------------------------------------------------*/
/*                                        Command table type description  */
/*------------------------------------------------------------------------*/

typedef struct ComEnt
{  char                  *cmdName;
   int          (*cmdFunc)(char*);
   char                  *cmdDesc;
} ComEnt;

/*------------------------------------------------------------------------*/
/*                                                  Static Phone Channel  */
/*------------------------------------------------------------------------*/

/* The following phone channel pointer refers to the currently opened     */
/*   phone channel, or is NULL if no channel is open.                     */

Phone_Channel *pc = NULL;

/*------------------------------------------------------------------------*/
/*                                Static dead-man's handle communication  */
/*------------------------------------------------------------------------*/

/* The static 'deadmanKillRequest' is set to request the deadman's handle */
/*   thread to stop functioning, whereupon the semaphore 'deadmanAck'     */
/*   will be signalled by the deadman's handle thread. In normal running  */
/*   the static 'deadmanKillRequest' will be zero if there is a dead-     */
/*   man's-handle thread running, or one otherwise.                       */

word      DeadmanKillReq;
Semaphore DeadmanAck;

/*------------------------------------------------------------------------*/
/*                                               Look-ahead declarations  */
/*------------------------------------------------------------------------*/

       int     main                 (int argc,char **argv);
static int     doHelp               (char *ptr);
static int     doComment            (char *ptr);
static int     doQuit               (char *ptr);
static int     doIncludeFile        (char *ptr);
static void    removeSpace          (char **ptr);
static int     ispfx                (char **ptr,char *cmd);
static void    PrintError           (word cd);
static word    ReadTime             (char **lnk);
static void    PrintTime            (word tm);
static void    PrintResult          (word res);
static int     doOpenChannel        (char *ptr);
static int     doStartDeadman       (char *ptr);
static void    DeadmanThread        (word tmo);
static int     doFinishDeadman      (char *ptr);
static int     doPollRing           (char *ptr);
static int     doPollDial           (char *ptr);
static int     doPollOnHook         (char *ptr);
static int     doWaitRing           (char *ptr);
static int     doTakeOffHook        (char *ptr);
static int     doPutOnHook          (char *ptr);
static int     doSelectPulse        (char *ptr);
static int     doSelectTone         (char *ptr);
static int     doSelectPabx         (char *ptr);
static int     doSelectPstn         (char *ptr);
static int     doSetPabxNumber      (char *ptr);
static int     doDialNumber         (char *ptr);
static int     doWaitDial           (char *ptr);
static int     doCloseChannel       (char *ptr);

/*------------------------------------------------------------------------*/
/*                                                     The command table  */
/*------------------------------------------------------------------------*/

ComEnt comTab[] = 
{
   { "?",      doHelp         , "Print command table"                     },
   { ";",      doComment      , "Add comment (to transcript)"             },
   { "<",      doIncludeFile  , "Input commands from a file"              },
   { "q",      doQuit         , "Quit this test program"                  },
   
   { "open",   doOpenChannel  , "Open a channel to phone interface"       },
   { "dms",    doStartDeadman , "Re-start and refresh dead man's handle"  },
   { "dmf",    doFinishDeadman, "Stop refreshing dead man's handle"       },
   { "pring",  doPollRing     , "Poll to see if phone ringing"            },
   { "ponhk",  doPollOnHook   , "Poll to see if phone is on-hook"         },
   { "pdial",  doPollDial     , "Poll to see if phone is dialling"        },
   { "wring",  doWaitRing     , "Wait for phone to ring (with timeout)"   },
   { "toffh",  doTakeOffHook  , "Take phone off hook"                     },
   { "ponh",   doPutOnHook    , "Put phone on-hook"                       },
   { "tone",   doSelectTone   , "Select tone-dialling"                    },
   { "pulse",  doSelectPulse  , "Select pulse-dialling"                   },
   { "pabx",   doSelectPabx   , "Enable use of PABX prefix string"        },
   { "pstn",   doSelectPstn   , "Select PSTN : no PABX prefix"            },
   { "spabx" , doSetPabxNumber, "Set the PABX prefix string"              },
   { "dialn",  doDialNumber   , "Dial a number"                           },
   { "wdial",  doWaitDial     , "Wait for dial-finished (with timeout)"   },
   { "close",  doCloseChannel , "Close current phone channel"             },

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
{  char    inpl[1000];
   char          *ptr;
   char          *tfn;
   char      tft[256];
   int           argn;
   char          *arg;
   ComEnt        *cmd;

   printf
   (  "Test program for microlink phone line interface. %s %s\n",
      __DATE__,__TIME__
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
   } else printf("No transcript of this session will be written\n");
   
   pc = NULL;
   DeadmanKillReq = 1;
   InitSemaphore(&DeadmanAck,0);

   for(;;)
   {  Signal(&printSem); Wait(&printSem);
      if(fstk)
      {  if(pc) { outf((b,"< ")); } else { outf((b,"unopened < ")); }
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
      {  if(pc) { outf((b,"> "));  } else { outf((b,"unopened > ")); }
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
         {  if((*cmd->cmdFunc)(lap)==1) goto finishProgram;
            break;
         }
      }
      if(!cmd->cmdName) { outf((b,"Bad command name : Use '?' for help\n")); }
   }

   finishProgram:
   if(pc) Phone_CloseChannel(pc);
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                           doHelp(...)  */
/*------------------------------------------------------------------------*/

static int doHelp(char *ptr)
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

static int doComment(char *ptr)
{  ptr = NULL; 
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                           doQuit(...)  */
/*------------------------------------------------------------------------*/

static int doQuit(char *ptr)
{  ptr = NULL;
   return 1;
}

/*------------------------------------------------------------------------*/
/*                                                    doIncludeFile(...)  */
/*------------------------------------------------------------------------*/

static int doIncludeFile(char *ptr)
{  FilRec *rc;

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
/*                                                       PrintError(...)  */
/*------------------------------------------------------------------------*/

static void PrintError(word cd)
{  char buf[256];

   *buf = 0;
   Fault(cd,buf,255);
   outf((b,"Error description : %s\n",buf));
}

/*------------------------------------------------------------------------*/
/*                                                         ReadTime(...)  */
/*------------------------------------------------------------------------*/

static word ReadTime(char **lnk)
{  char *ptr;
   word tm;

   ptr = *lnk;

   tm = (int)strtol(ptr,NULL,10);
   removeSpace(&ptr);

   if(ispfx(&ptr,"mS"))
   {  tm*=1000;
      ptr++;
   } else if(ispfx(&ptr,"s")||ispfx(&ptr,"S"))
   {  tm*=1000*1000;
      ptr++;
   } else if(ispfx(&ptr,"m")||ispfx(&ptr,"M"))
   {  tm *= 60*1000*1000;
      ptr++;
   }

   *lnk = ptr;
   return tm;
}

/*------------------------------------------------------------------------*/
/*                                                        PrintTime(...)  */
/*------------------------------------------------------------------------*/

static void PrintTime(word tm)
{  outf((b,"%duS : %.3fmS : %.3fs : %.3fm",
           tm,tm/1000.0,tm/(1000.0*1000.0),tm/(60.0*1000.0*1000.0)
       ));
}

/*------------------------------------------------------------------------*/
/*                                                      PrintResult(...)  */
/*------------------------------------------------------------------------*/

static void PrintResult(word res)
{  
   switch(res)
   {  case Phone_Ok:
         outf((b,"Operation sucessful\n"));
         break;
      case Phone_Dead:
         outf((b,"Dead Man's Timeout Expired\n"));
         break;
      case Phone_Timedout:
         outf((b,"Operation timed-out\n"));
         break;
      case Phone_UnBlocked:
         outf((b,"Another process un-blocked this channel\n"));
         break;
      case Phone_Failed:
         outf((b,"Operation failed\n"));
         break;
      case Phone_BadNum:
         outf((b,"Bad number\n"));
         break;
      case Phone_RejectedNum:
         outf((b,"Number was rejected\n"));
         break;
      default:
         outf((b,"Fault number 0x%08X received.\n",res));
         PrintError(res);
         break;
   }
}

/*------------------------------------------------------------------------*/
/*                                                    doOpenChannel(...)  */
/*------------------------------------------------------------------------*/

static int doOpenChannel(char *ptr)
{  word err;

   ptr = NULL;
   if(pc!=NULL)
   {  outf((b,"Channel already open\n"));
      return 0;
   }
   
   pc = Phone_OpenChannel(NULL,&err);
   if(pc==NULL) PrintResult(err);
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                   doStartDeadman(...)  */
/*------------------------------------------------------------------------*/

static int doStartDeadman(char *ptr)
{  word tmo;

   if(DeadmanKillReq==0)
   {  outf((b,"Deadman's handle thread is already running\n"));
      return 0;
   }
   
   if(pc==NULL)
   {  outf((b,"Channel is not open\n"));
      return 0;
   }

   removeSpace(&ptr);
   tmo = ReadTime(&ptr);
   outf((b,"Deadman's handle period requested : "));
   PrintTime(tmo);
   
   if(tmo<=100*1000||tmo>10*1000*1000)
   {  outf((b,"Bad timeout: Must be between 1/10th sec and 10 secs"));
      return 0;
   }
   
   DeadmanKillReq = 1;
   Fork(2000,DeadmanThread,4,tmo);
   return 0;
}
   
/*------------------------------------------------------------------------*/
/*                                                    DeadmanThread(...)  */
/*------------------------------------------------------------------------*/

static void DeadmanThread(word tmo)
{  word res;

   for(;;)
   {  Delay(tmo);
      if(DeadmanKillReq) break;
      res = Phone_RefreshDeadmanTimeout(pc);
      if(res!=Phone_Ok)
      {  Wait(&printSem);
         if(promptPresent) { outf((b,"\n")); }
         PrintResult(res);
         Signal(&printSem);
      }
      if(DeadmanKillReq) break;
   }
   Signal(&DeadmanAck);
}

/*------------------------------------------------------------------------*/
/*                                                  doFinishDeadman(...)  */
/*------------------------------------------------------------------------*/

static int doFinishDeadman(char *ptr)
{  
   if(DeadmanKillReq==1)
   {  outf((b,"There is no deadman's handle thread running\n"));
      return 0;
   }
   
   outf((b,"Requesting termination of dead-man's-handle thread\n"));
   DeadmanKillReq = 1;
   Wait(&DeadmanAck);
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                       doPollRing(...)  */
/*------------------------------------------------------------------------*/

static int doPollRing(char *ptr)
{  word res;
   
   if(pc==NULL)
   {  outf((b,"Phone channel not opened\n"));
      return 0;
   }
   
   ptr = NULL;
   res = Phone_IsRinging(pc);
   if(res!=Phone_No&&res!=Phone_Yes)
   {  PrintResult(res);
      return 0;
   }

   if(res==Phone_No)
   {  outf((b,"Phone is not ringing\n"));
   } else
   {  outf((b,"Phone is ringing\n"));
   }
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                     doPollOnHook(...)  */
/*------------------------------------------------------------------------*/

static int doPollOnHook(char *ptr)
{  word res;
   
   if(pc==NULL)
   {  outf((b,"Phone channel not opened\n"));
      return 0;
   }
   
   ptr = NULL;
   res = Phone_IsOnHook(pc);
   if(res!=Phone_No&&res!=Phone_Yes)
   {  PrintResult(res);
      return 0;
   }

   if(res==Phone_No)
   {  outf((b,"Phone is on-hook\n"));
   } else
   {  outf((b,"Phone is off-hook\n"));
   }
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                       doPollDial(...)  */
/*------------------------------------------------------------------------*/

static int doPollDial(char *ptr)
{  word res;
   
   if(pc==NULL)
   {  outf((b,"Phone channel not opened\n"));
      return 0;
   }
   
   ptr = NULL;
   res = Phone_IsDialling(pc);
   if(res!=Phone_No&&res!=Phone_Yes)
   {  PrintResult(res);
      return 0;
   }

   if(res==Phone_No)
   {  outf((b,"Phone is dialling\n"));
   } else
   {  outf((b,"Phone is not dialling\n"));
   }
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                       doWaitRing(...)  */
/*------------------------------------------------------------------------*/

static int doWaitRing(char *ptr)
{  word tmo,res;

   if(pc==NULL)
   {  outf((b,"There is no phone channel open\n"));
      return 0;
   }
   
   removeSpace(&ptr);
   tmo = ReadTime(&ptr);
   outf((b,"Timeout requested to wait for `phone ring : "));
   PrintTime(tmo);
   
   if(tmo<=100*1000||tmo>4*60*1000*1000)
   {  outf((b,"Bad timeout : Must be between 1/10th sec and 4 mins\n"));
      return 0;
   }
   
   outf((b,"Waiting for phone to ring ...\n"));
   res = Phone_AwaitRing(pc,tmo);
   PrintResult(res);
   return 0;
}
 
/*------------------------------------------------------------------------*/
/*                                                    doTakeOffHook(...)  */
/*------------------------------------------------------------------------*/

static int doTakeOffHook(char *ptr)
{  word res;

   if(pc==NULL)
   {  outf((b,"There is no phone channel open\n"));
      return 0;
   }
   
   ptr = NULL;
   res = Phone_TakeOffHook(pc);
   PrintResult(res);
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                      doPutOnHook(...)  */
/*------------------------------------------------------------------------*/

static int doPutOnHook(char *ptr)
{  word res;

   if(pc==NULL)
   {  outf((b,"There is no phone channel open\n"));
      return 0;
   }
   
   ptr = NULL;
   res = Phone_PutOnHook(pc);
   PrintResult(res);
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                    doSelectPulse(...)  */
/*------------------------------------------------------------------------*/

static int doSelectPulse(char *ptr)
{  word res;

   if(pc==NULL)
   {  outf((b,"There is no open `phone channel\n"));
      return 0;
   }
   
   ptr = NULL;
   res = Phone_SelectPulseDialling(pc);
   PrintResult(res);
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                     doSelectTone(...)  */
/*------------------------------------------------------------------------*/

static int doSelectTone(char *ptr)
{  word res;

   if(pc==NULL)
   {  outf((b,"There is no open `phone channel\n"));
      return 0;
   }
   
   ptr = NULL;
   res = Phone_SelectToneDialling(pc);
   PrintResult(res);
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                     doSelectPabx(...)  */
/*------------------------------------------------------------------------*/

static int doSelectPabx(char *ptr)
{  word res;

   if(pc==NULL)
   {  outf((b,"There is no open `phone channel\n"));
      return 0;
   }
   
   ptr = NULL;
   res = Phone_SelectPabx(pc);
   PrintResult(res);
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                     doSelectPstn(...)  */
/*------------------------------------------------------------------------*/

static int doSelectPstn(char *ptr)
{  word res;

   if(pc==NULL)
   {  outf((b,"There is no open `phone channel\n"));
      return 0;
   }
   
   ptr = NULL;
   res = Phone_SelectPstn(pc);
   PrintResult(res);
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                  doSetPabxNumber(...)  */
/*------------------------------------------------------------------------*/

static int doSetPabxNumber(char *ptr)
{  word res;

   removeSpace(&ptr);
   res = Phone_SetPabxNumber(pc,ptr);
   PrintResult(res);
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                     doDialNumber(...)  */
/*------------------------------------------------------------------------*/

static int doDialNumber(char *ptr)
{  word res;

   removeSpace(&ptr);
   res = Phone_DialNumber(pc,ptr);
   PrintResult(res);
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                       doWaitRing(...)  */
/*------------------------------------------------------------------------*/

static int doWaitDial(char *ptr)
{  word tmo,res;

   if(pc==NULL)
   {  outf((b,"There is no phone channel open\n"));
      return 0;
   }
   
   removeSpace(&ptr);
   tmo = ReadTime(&ptr);
   outf((b,"Timeout requested to wait for `phone ring : "));
   PrintTime(tmo);
   
   if(tmo<=100*1000||tmo>4*60*1000*1000)
   {  outf((b,"Bad timeout : Must be between 1/10th sec and 4 mins\n"));
      return 0;
   }
   
   outf((b,"Waiting for phone to ring ...\n"));
   res = Phone_AwaitRing(pc,tmo);
   PrintResult(res);
   return 0;
}

/*------------------------------------------------------------------------*/
/*                                                   doCloseChannel(...)  */
/*------------------------------------------------------------------------*/

static int doCloseChannel(char *ptr)
{  word res;

   res = Phone_CloseChannel(pc);
   PrintResult(res); pc = NULL;
   return 0;
}
