/* -> conds/h
 * Title:               The IF, WHILE and structure stack handler
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef conditionals_h
#define conditionals_h

#include "getline.h"
#include "globvars.h"
#include "nametype.h"

/*---------------------------------------------------------------------------*/

extern CARDINAL includedIFs ;
extern CARDINAL rejectedIFs ;
extern CARDINAL includedWHILEs ;
extern CARDINAL rejectedWHILEs ;

typedef enum StructureStackElementType {
                                        ConditionalSSET,
                                        WhileSSET,
                                        GetSSET,
                                        MacroSSET
                                       } StructureStackElementType ;

typedef struct {
                CARDINAL    lineNumber ;
                char       *pointer ;
                ListStatus  state ;
               } WhileElement ;

typedef struct {
                FileReadMode inputMode ; /* Only relevant at top level macro call */
                union {
                       /* case WholeFileLoad */
                       char     *storePointer ;
                       /* case ByteStream */
                       CARDINAL  fileOffset ;
                      } u ;
                CARDINAL     lineNumber ;
                Name         name ; /* This holds the current file or macro name */
                ListStatus   state ;
               } MacroElement ;

typedef struct {
                FileReadMode  inputMode ; /* Only relevant at top level macro call */
                union {
                       /* case WholeFileLoad */
                       char     *storePointer ;
                       /* case ByteStream */
                       CARDINAL  fileOffset ;
                      } u ;
                CARDINAL      lineNumber ;
                Name          fileName ; /* This holds the current file or macro name */
                char         *fileStart ;
                CARDINAL      fileLen ;  /* For the benefit of caching GET files */
               } GetElement ;

typedef struct {
                StructureStackElementType type ;
                union {
                       /* case ConditionalSSET */
                       ListStatus   state ;
                       /* case WhileSSET */
                       WhileElement whileEl ;
                       /* case GetSSET */
                       GetElement   file ;
                       /* case MacroSSET */
                       MacroElement macro ;
                      } u ;
               } StructureStackElement ;

/*---------------------------------------------------------------------------*/

BOOLEAN Stack(StructureStackElement s) ;

BOOLEAN Unstack(StructureStackElement *s) ;

void InitStructureStack(void) ;

void InitErrorAccess(void) ;

BOOLEAN NextMacroElement(StructureStackElement *s) ;

void UnwindToGet(void) ;

#endif

/*---------------------------------------------------------------------------*/
/* EOF conds/h */
