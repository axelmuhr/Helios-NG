/* -> globvars/c
 * Title:               The global variables
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "globvars.h"
#include "asmvars.h"

/*---------------------------------------------------------------------------*/

CARDINAL        programCounter ;
CARDINAL        fileSize ;
CARDINAL        maxCache ;

VariableCounter variableCounter ;

int             pass ; /* 1, 2 */

int             macroLevel ; /* [0..255] */

BOOLEAN         printState = FALSE ;
BOOLEAN         abortFlag = FALSE ;
BOOLEAN         inMacroDef = FALSE ;
BOOLEAN         terseState = TRUE ;
BOOLEAN         allowFP ;
BOOLEAN         hadFP ;
BOOLEAN         xrefOn = FALSE ;
BOOLEAN         caching = TRUE ;                /* cache source files */
BOOLEAN         module = FALSE ;
BOOLEAN         noInitArea ;
BOOLEAN         keepingAll ;
BOOLEAN         area_is_code ;
BOOLEAN         closeExec = FALSE ;

BOOLEAN         allowUndefinedSymbols = FALSE ;
BOOLEAN         librarycode = FALSE ;
BOOLEAN         clmake_def = FALSE ;    /* command line "-Makedef" option */
#if 1
BOOLEAN         clmake_SMT = TRUE ;     /* command line "-nosmt" for FALSE */
#else
BOOLEAN         clmake_SMT = FALSE ;    /* command line "-SMT" option */
#endif

BOOLEAN         traceon = FALSE ;       /* command line "-TRace" option */

ListStatus      listStatus ;
ListStatus      nextListState ;

EXCEPTION       exception = None ;

CARDINAL        stringOffset ;
int             symbolId ;
SegmentType     segment_type ;
CARDINAL        code_size ;
CARDINAL        data_size ;
CARDINAL        bss_size ;

HOF_header      heliosHdr ;     /* Helios global object information */

/*---------------------------------------------------------------------------*/
/* EOF globvars/c */
