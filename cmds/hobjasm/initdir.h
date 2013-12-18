/* -> initdir/h
 * Title:               Directive initialisation
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef initdir_h
#define initdir_h

#include "nametype.h"
#include "tables.h"

/*---------------------------------------------------------------------------*/

typedef enum {
              TIF,                      /* [ */
              TELSE,                    /* | */
              TFI,                      /* ] */
              TMNOTE,                   /* ! */
              THASH,                    /* # */
              TSTAR,                    /* * */
              TEQUAL,                   /* = */
              TPERC,                    /* % */
              TAMP,                     /* & */
              THAT,                     /* ^ */
              TEND,
              TLIB,                     /* generate library code */
              TLNK,
              TGET,
#if 1 /* binary include support */
	      TBGET,			/* include binary data into image */
#endif
              TOPT,
              TTTL,
              TSUBTTL,
              TRN,
              TWHILE,
              TWEND,
              TMACRO,
              TMEXIT,
              TMEND,
              TGBLA,
              TGBLL,
              TGBLS,
              TLCLA,
              TLCLL,
              TLCLS,
              TSETA,
              TSETL,
              TSETS,
              TASSERT,
              TROUT,
              TALIGN,
              TLTORG,
              TDCW,
              TFN,
              TDCFS,
              TDCFD,
              TNOFP,
              TCN,
              TCP,
              TIMPORT,
	      TLABREF,
              TEXPORT,
              TEXTERN,
              TDATA,
              TINIT,
              TCODE,
              TLABEL,
              TMODULE,
              TCOMMON,
              TIMSIZE,
              TMODNUM,
              TMODOFF,
              TOFFSET,
              TDUMMY
             } DirectiveNumber ;

/*---------------------------------------------------------------------------*/

void InitDirectives(void) ;
/* This does the name table initialisation because the crappy language
 * doesn't do it properly itself and doesn't allow manifest tables to
 * be constructed at compile time
 */

BOOLEAN OneCharacterDirective(char *line,CARDINAL *lineIndex,DirectiveNumber *directiveNumber) ;
/* Spot one character directives and return the number
 * lineIndex is returned past the directive if successful,
 * otherwise before it
 */

BOOLEAN NameDirective(DirectiveNumber *directiveNumber,Name name) ;
/* Spot name directives and return the number
 * lineIndex is returned past the directive if successful,
 * otherwise before it
 */

BOOLEAN DirectiveSyntax(DirectiveNumber number,char ch,BOOLEAN symbolFound) ;
/* Returns true <=> directive syntax ok
 * otherwise false and error message already produced
 */

BOOLEAN AreaAttrName(char *line,CARDINAL *index,CARDINAL *result) ;

void Init_InitDir(void) ;

extern BOOLEAN allowDirInCond[TDUMMY] ;
extern BOOLEAN disallowDirOutsideArea[TDUMMY] ;

#endif

/*---------------------------------------------------------------------------*/
/* EOF initdir/h */
