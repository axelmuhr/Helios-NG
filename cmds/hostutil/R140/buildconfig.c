/*> buildconfig.c <*/
/*---------------------------------------------------------------------------*/
/* (c) 1990, Active Book Company, Cambridge, United Kingdom.                 */
/*---------------------------------------------------------------------------*/
/* The "buildconfig" utility constructs a ROM item containing the file 	     */
/* "/rom/helios/etc/config". This is used by the Helios system during        */
/* startup to define certain system parameters. At the moment it is a UNIX   */
/* source, which includes certain HELIOS header files for information.       */
/*---------------------------------------------------------------------------*/

#define __SERIALLINK (1) /* Allow serial line to be used as second link      */

/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "helios.h"
#include "config.h"
#include "queue.h"
#include "message.h"
#include "sem.h"
#include "link.h"

/*---------------------------------------------------------------------------*/
/* flags set in Flags word (from root.h) */

#define Root_Flags_rootnode	0x00000001 /* set if this is rootnode */
#define Root_Flags_special	0x00000002 /* set if this is special nucleus */
#define Root_Flags_ROM		0x00000004 /* set if this is ROMm'ed nucleus */

/*---------------------------------------------------------------------------*/

#define cfile	"ROMconfig"	/* destination binary file */

int main(void)
{
 Config  cdata ;
 FILE   *imf ;
 int     i ;

 printf("buildconfig: ROM configuration data placed in file \"%s\"\n",cfile) ;

 /**** USEFUL **** these values are worth tailoring **************************/

 /* initial size (and increment value) for port table */
 cdata.PortTabSize = 1024 ;

 cdata.MyName = -1 ;		/* use default "/00" */
 cdata.ParentName = -1 ;	/* use default "/IO" */

 /* number of transputer link adaptors connected to the system */
#ifdef __SERIALLINK
 cdata.NLinks = 2 ; /* 2nd link is really the Serial line */
#else
 cdata.NLinks = 1 ;
#endif

 /* Set up the link structures */
 for (i = 0; i < cdata.NLinks; ++i)
 {
  /* Link not connected to IO processor and didn't boot us (no IOdebugs) */
  cdata.LinkConf[i].Flags = Link_Flags_debug;

  /* Initially only link 0 should form part of a network */
  cdata.LinkConf[i].Mode = (i == 0 ? Link_Mode_Intelligent : Link_Mode_Null) ;

  /* This link is not currently communicating */
  cdata.LinkConf[i].State = Link_State_Dead ;

  /* Link ID number */
  cdata.LinkConf[i].Id = i ;
 }

 /* This value should be initialised by the "config" loading program */
 cdata.Date = time(NULL) ; /* seconds since "January 1st 1970 GMT" */
 printf("buildconfig: Date = &%08X\n",(int)cdata.Date) ;

 /**** FIXED **** this information is not used in a ROM started system *******/
 cdata.Incarnation = 1 ;
 cdata.LoadBase = NULL ;	/* initialised by Executive */
 cdata.ImageSize = 0 ;		/* initialised by Executive */

 cdata.FirstProg = 0;		/* slot offset of initial program */
 				/* (0 = default to IVecProcMan) */

 cdata.MemSize = 0 ;		/* initialised by Executive */

 /* ROM loaded configuration */
 cdata.Flags = Root_Flags_rootnode | Root_Flags_special | Root_Flags_ROM ;
 cdata.Spare[0] = 0 ;		/* unused */

 /*-- output the configuration file -----------------------------------------*/

 if ((imf = fopen(cfile,"w")) == NULL)
  {
   fprintf(stderr,"buildconfig: Cannot open output file \"%s\"\n",cfile) ;
   exit(1) ;
  }

 if (fwrite((char *)&cdata,sizeof(Config),1,imf) != 1)
  {
   fprintf(stderr,"buildconfig: Unable to write data to file \"%s\"\n",cfile) ;
   exit(2) ;
  }

 fclose(imf) ;

 return(0) ;
}

/*---------------------------------------------------------------------------*/
/*> EOF buildconfig.c <*/
