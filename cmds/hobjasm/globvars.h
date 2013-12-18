/* -> globvars/h
 * Title:               The global variables
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef globalvars_h
#define globalvars_h

/*---------------------------------------------------------------------------*/

#include "constant.h"

typedef enum EXCEPTION {
                        None,
                        EndOfInput,
                        StackOverflow,
                        StackUnderflow,
                        StackErr,
                        FileNotFound
                       } EXCEPTION ;

typedef enum {
              ListPC,
              ListCondPC,
              ListSETPC,
              ListMacExpPC,
              ListMacCallPC,
              ListMendPC,
              ListOptsPC
             } PrintControl ;

typedef int ListStatus ; /* SET OF PrintControl */

typedef enum {
              FixedVCT,
              RelativeVCT
             } VCType ;

typedef struct {
                CARDINAL offset ;
                CARDINAL reg ;
               } RelativeVC ;

typedef struct {
                VCType   type ;
                union    {
                          /* Case FixedVCT */
                          CARDINAL   offset ;

                          /* Case RelativeVCT */
                          RelativeVC relativeVC ;
                         } u ;
               } VariableCounter ;

extern CARDINAL programCounter ;
extern CARDINAL fileSize ;
extern CARDINAL maxCache ;

extern VariableCounter variableCounter;

extern int     pass ; /* 1, 2 */

extern int     macroLevel ; /* [0..255] */

extern BOOLEAN printState ;
extern BOOLEAN abortFlag ;
extern BOOLEAN inMacroDef ;
extern BOOLEAN terseState ;
extern BOOLEAN allowFP ;
extern BOOLEAN hadFP ;
extern BOOLEAN xrefOn ;
extern BOOLEAN caching ;
extern BOOLEAN module ;
extern BOOLEAN closeExec ;

extern BOOLEAN allowUndefinedSymbols ;
extern BOOLEAN librarycode ;
extern BOOLEAN clmake_def ;
extern BOOLEAN clmake_SMT ;
extern BOOLEAN traceon ;

extern ListStatus listStatus ;
extern ListStatus nextListState ;

extern EXCEPTION exception ;

/* This is the limit we impose on the size of the source file cache */
#define MaxCache    0x800000 /* 8 Mbytes */

/*---------------------------------------------------------------------------*/

#endif  /* globalvars_h */

/*---------------------------------------------------------------------------*/
/* EOF globvars/h */
