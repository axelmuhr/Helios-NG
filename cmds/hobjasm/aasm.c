/*-> aasm/c
 * Title:               The top level module in the world
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "asm.h" 
#include "constant.h"
#include "errors.h" 
#include "expr.h"
#include "formatio.h" 
#include "getline.h"
#include "globvars.h"
#include "initdir.h"
#include "iface.h" 
#include "listing.h" 
#include "nametype.h"
#include "osdepend.h" 
#include "symbol.h" 
#include "tables.h" 
#include "tokens.h" 
#include "vars.h" 
#include "version.h"

#include <stdlib.h>
#include <ctype.h>

/*---------------------------------------------------------------------------*/

static char tempLine[MaxLineLength + 1] ;

BOOLEAN notFound ;

/*---------------------------------------------------------------------------*/

int main(int argc,char **argv)
{
 Init_osd() ;           /* Operating System Dependant initialisation */
 Init_Asm() ;           /* assembler initialisation */
 Init_Expression() ;    /* expression initialisation (operator precedence) */
 Init_InitDir() ;       /* assembler directive declarations */
 Init_Variables() ;     /* global variable initialisation */

 /* Display the program title string and version number */
 WriteChs(Version) ;
 WriteChs(" (") ;
 WriteChs(__DATE__) ;
 WriteCh(' ') ; 
 WriteChs(__TIME__) ;
 WriteChs(")\\N") ;

 InitSymbolTable() ;    /* declare and initialise (empty) the symbol table */

 if (GetInterface(argc,argv,tempLine))
  {
   /* Gathering cross-reference information is only useful if we are going
    * to "print" the information.
    */
   if ((printState == 0) && (xrefOn == 1))
    xrefOn = 0 ;

   PageModeOff() ;                      /* non-paging output stream */

   inFile = FALSE ;
   Asm(tempLine) ;                      /* Just pass in the code file name */

   if ((printState == 1) && (xrefOn == 1))
    SymbolDump() ;                      /* display the gathered symbol info */

   CloseErrorStream() ;                 /* closedown the error output file */
   exit((totalErrors > 0) ? 1 : 0) ;
  }
 else
  printf("Bad commands, use -help for help\n") ;

 return(0) ;
}

/*---------------------------------------------------------------------------*/
/* End aasm/c */
