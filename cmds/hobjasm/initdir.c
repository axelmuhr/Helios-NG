/* -> initdir/c
 * Title:               Directive initialisation
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "constant.h"
#include "errors.h"
#include "formatio.h"
#include "globvars.h"
#include "initdir.h"
#include "nametype.h"
#include "store.h"
#include "vars.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

typedef enum {
              NoCheck,
              NoLabNoExp,
              NoLabExp,
              LabNoExp,
              LabExp,
              OptLabExp,
              OptLabNoExp
             } SyntaxType ;

typedef struct DirectiveEntry {
                               Name            key ;
                               DirectiveNumber value ;
                              } DirectiveEntry ;

typedef struct AreaEntry {
                          Name     name ;
                          CARDINAL value ;
                         } AreaEntry ;

/* THIS SHOULD BE TUNED TO BE AS SMALL AS POSSIBLE */
#define MaxDir  1024    /* maximum number of directives of the same length */

#define MaxAreaName 9

BOOLEAN allowDirInCond[TDUMMY] ;
BOOLEAN disallowDirOutsideArea[TDUMMY] ;

BOOLEAN         doneInit = FALSE ;
signed char     dirChars[256] ;

#ifdef __NCCBUG
DirectiveEntry *threeLetterDirectives = NULL ;
DirectiveEntry *fourLetterDirectives = NULL ;
DirectiveEntry *fiveLetterDirectives = NULL ;
DirectiveEntry *sixLetterDirectives = NULL ;
#else
DirectiveEntry  threeLetterDirectives[MaxDir] ;
DirectiveEntry  fourLetterDirectives[MaxDir] ;
DirectiveEntry  fiveLetterDirectives[MaxDir] ;
DirectiveEntry  sixLetterDirectives[MaxDir] ;
#endif
AreaEntry areaNames[MaxAreaName] ;

static SyntaxType syntax[TDUMMY] ;

/*---------------------------------------------------------------------------*/

static CARDINAL DirHash(Name name)
{
  switch (name.length) {
    case 3:
    return (name.key[0]-0x40 + 2*(name.key[1]-0x40) + 2*(name.key[2]-0x40)) & (MaxDir-1);

    case 4:
    return ((name.key[0]-0x40) + (name.key[1]-0x40) + 2*(name.key[2]-0x40) + (name.key[3]-0x40)) & (MaxDir-1);

    case 5:
    return (4*(name.key[0]-0x40) + 2*(name.key[3]-0x40) + (name.key[4]-0x40)) & (MaxDir-1);

    case 6:
    return (4 * (name.key[0] - 0x40) + 2 * (name.key[1] - 0x40) + 2 * (name.key[3] - 0x40) + (name.key[4] - 0x40)) & (MaxDir-1) ;

    default:
    return 0;
  };
} /* End DirHash */

/*---------------------------------------------------------------------------*/

static void InitEntry(char *chars,
  DirectiveNumber dirNum, 
  DirectiveEntry *table)
{
  CARDINAL i;
  Name     name;

  name.length = strlen(chars);
  name.key = chars;
  i = DirHash(name);
  while (table[i].key.length != 0) {
    WriteChs("Directive name clash at ");
    WriteChs(chars);
    WriteChs("\\N");
    i++;
  };
  table[i].value = dirNum;
  table[i].key.length = strlen(chars);
  table[i].key.key = mymalloc(table[i].key.length + 1);
  strcpy(table[i].key.key, chars);
} /* End InitEntry */

/*---------------------------------------------------------------------------*/
/* Initialise the directive name table */

void InitDirectives(void)
{
 if (doneInit)
  return ;

 InitEntry("END",    TEND,    threeLetterDirectives) ;
 InitEntry("LIB",    TLIB,    threeLetterDirectives) ;
 InitEntry("LNK",    TLNK,    threeLetterDirectives) ;
 InitEntry("GET",    TGET,    threeLetterDirectives) ;
 InitEntry("OPT",    TOPT,    threeLetterDirectives) ;
 InitEntry("TTL",    TTTL,    threeLetterDirectives) ;
 InitEntry("DCB",    TEQUAL,  threeLetterDirectives) ;
 InitEntry("DCW",    TDCW,    threeLetterDirectives) ;
 InitEntry("DCD",    TAMP,    threeLetterDirectives) ;
 InitEntry("EQU",    TSTAR,   threeLetterDirectives) ;
#if 1 /* binary include support */
 InitEntry("BGET",   TBGET,   fourLetterDirectives) ;
#endif
 InitEntry("SUBT",   TSUBTTL, fourLetterDirectives) ;
 InitEntry("WEND",   TWEND,   fourLetterDirectives) ;
 InitEntry("MEND",   TMEND,   fourLetterDirectives) ;
 InitEntry("GBLA",   TGBLA,   fourLetterDirectives) ;
 InitEntry("GBLL",   TGBLL,   fourLetterDirectives) ;
 InitEntry("GBLS",   TGBLS,   fourLetterDirectives) ;
 InitEntry("LCLA",   TLCLA,   fourLetterDirectives) ;
 InitEntry("LCLL",   TLCLL,   fourLetterDirectives) ;
 InitEntry("LCLS",   TLCLS,   fourLetterDirectives) ;
 InitEntry("SETA",   TSETA,   fourLetterDirectives) ;
 InitEntry("SETL",   TSETL,   fourLetterDirectives) ;
 InitEntry("SETS",   TSETS,   fourLetterDirectives) ;
 InitEntry("ROUT",   TROUT,   fourLetterDirectives) ;
 InitEntry("CODE",   TCODE,   fourLetterDirectives) ;
 InitEntry("DATA",   TDATA,   fourLetterDirectives) ;
 InitEntry("INIT",   TINIT,   fourLetterDirectives) ;
 InitEntry("DCFS",   TDCFS,   fourLetterDirectives) ;
 InitEntry("DCFD",   TDCFD,   fourLetterDirectives) ;
 InitEntry("NOFP",   TNOFP,   fourLetterDirectives) ;
 InitEntry("ALIGN",  TALIGN,  fiveLetterDirectives) ;
 InitEntry("WHILE",  TWHILE,  fiveLetterDirectives) ;
 InitEntry("MACRO",  TMACRO,  fiveLetterDirectives) ;
 InitEntry("MEXIT",  TMEXIT,  fiveLetterDirectives) ;
 InitEntry("LTORG",  TLTORG,  fiveLetterDirectives) ;
 InitEntry("LABEL",  TLABEL,  fiveLetterDirectives) ;
 InitEntry("MODULE", TMODULE, sixLetterDirectives) ;
 InitEntry("COMMON", TCOMMON, sixLetterDirectives) ;
 InitEntry("IMSIZE", TIMSIZE, sixLetterDirectives) ;
 InitEntry("MODNUM", TMODNUM, sixLetterDirectives) ;
 InitEntry("MODOFF", TMODOFF, sixLetterDirectives) ;
 InitEntry("OFFSET", TOFFSET, sixLetterDirectives) ;
 InitEntry("ASSERT", TASSERT, sixLetterDirectives) ;
 InitEntry("IMPORT", TIMPORT, sixLetterDirectives) ;
 InitEntry("LABREF", TLABREF, sixLetterDirectives) ;
 InitEntry("EXPORT", TEXPORT, sixLetterDirectives) ;
 InitEntry("EXTERN", TEXTERN, sixLetterDirectives) ;

 doneInit = TRUE ;
} /* End InitDirectives */

/*---------------------------------------------------------------------------*/

BOOLEAN OneCharacterDirective(char *line,
  CARDINAL        *lineIndex,
  DirectiveNumber *directiveNumber)
/*
Spot one character directives and return the number
lineIndex is returned past the directive if successful,
otherwise before it
*/
{
  char ch;
  if (line[*lineIndex] == Dot) {
    *directiveNumber = -1;
    return TRUE;
    /* Indicate we've found a style changing directive */
  };
  if (!termin[line[*lineIndex+1]]) return FALSE;
  ch = line[*lineIndex];
  if (dirChars[ch] >= 0) {
    *directiveNumber = dirChars[ch];
    (*lineIndex)++;/*Past directive*/
    while (line[*lineIndex] == Space) (*lineIndex)++;
    return TRUE;
  }; /* if */
  return FALSE;
} /* End OneCharacterDirective */

/*---------------------------------------------------------------------------*/

BOOLEAN NameDirective(DirectiveNumber *directiveNumber, Name name)
/*
Spot name directives and return the number
lineIndex is returned past the directive if successful,
otherwise before it
*/
{
  CARDINAL index;

  /*Now name contains the suspected directive*/
  switch (name.length) {
    case 2:
    if (name.key[1] == 'N') {
      if (name.key[0] == 'R') {
        *directiveNumber = TRN;
        return TRUE;
      } else if ((name.key[0] == 'F') && allowFP) {
        *directiveNumber = TFN;
        hadFP = TRUE;
        return TRUE;
      } else if (name.key[0] == 'C') {
        *directiveNumber = TCN;
        return TRUE;
      }; /* if */
    } else if ((name.key[0] == 'C') && (name.key[1] == 'P')) {
      *directiveNumber = TCP;
      return TRUE;
    }; /* if */
    break;
    
    case 3:
    index = DirHash(name);
    { DirectiveEntry *entry = &threeLetterDirectives[index];
      if ((entry->key.length == 3) &&
        (memcmp(entry->key.key, name.key, 3) == 0)) {
        *directiveNumber = entry->value;
        return TRUE;
      }; /* if */
    };
    break;

    case 4:
    index = DirHash(name);
    { DirectiveEntry *entry = &fourLetterDirectives[index];
      if ((entry->key.length == 4) &&
        (memcmp(entry->key.key, name.key, 4) == 0)) {
        *directiveNumber = entry->value;
        if (allowFP || ((*directiveNumber != TDCFS) &&
          (*directiveNumber != TDCFD))) {
          if ((*directiveNumber == TDCFS) || (*directiveNumber == TDCFD))
            hadFP = TRUE;
          return TRUE;
        }; /* if */
      }; /* if */
    };
    break;

    case 5:
    index = DirHash(name);
    { DirectiveEntry *entry = &fiveLetterDirectives[index];
      if ((entry->key.length == 5) &&
        (memcmp(entry->key.key, name.key, 5) == 0)) {
        *directiveNumber = entry->value;
        return TRUE;
      }; /* if */
    };
    break;

    case 6:
    index = DirHash(name);
    { DirectiveEntry *entry = &sixLetterDirectives[index];
      if ((entry->key.length == 6) &&
        (memcmp(entry->key.key, name.key, 6) == 0)) {
        *directiveNumber = entry->value;
        return TRUE;
      }; /* if */
    };
  }; /* case */
  return FALSE;
} /* End NameDirective */

/*---------------------------------------------------------------------------*/

BOOLEAN AreaAttrName(char *line, CARDINAL *index, CARDINAL *result)
{
  Name     name;
  CARDINAL i,
           j,
           oldIndex;

  while (line[*index] == Space) (*index)++;
  oldIndex = *index;
  if (SymbolTest(line, index, &name)) {
    while (line[*index] == Space) { (*index)++; }; /* while */
    for (i = 0; i < MaxAreaName; i++) {
      if (areaNames[i].name.length == name.length) {
        j = 0;
        while ((j < name.length) &&
          (name.key[j] == areaNames[i].name.key[j])) {
          j++;
        }; /* while */
        if (j == name.length) {
          *result = areaNames[i].value;
          return TRUE;
        }; /* if */
      }; /* if */
    }; /* for */
  }; /* if */
  *index = oldIndex;
  return FALSE;
} /* End AreaAttrName */

/*---------------------------------------------------------------------------*/

BOOLEAN DirectiveSyntax(DirectiveNumber number, char ch, BOOLEAN symbolFound)
/*
Returns true <=> directive syntax ok
otherwise false and error message already produced
*/
{
  switch (syntax[number]) {
    case NoLabNoExp:
    case LabNoExp:
    case OptLabNoExp:
    if ((ch != CommentSymbol) && (ch != CR)) {
      Warning(SynAfterDir);
      return FALSE;
    }; /* if */
    /*No check if expression expected*/
  };
  switch (syntax[number]) {
    case NoLabNoExp:
    case NoLabExp:
    if (symbolFound) {
      Warning(IllLineStart);
      return FALSE;
    }; /* if */
    break;

    case LabNoExp:
    case LabExp:
    if (!symbolFound) {
      Warning(LabMiss);
      return FALSE;
    }; /* if */
  
    /*Doesn't matter whether we have a symbol or not*/
  }; /* case */
  return TRUE;
} /* End DirectiveSyntax */

/*---------------------------------------------------------------------------*/

void Init_InitDir(void)
{
 CARDINAL i ;
 for (i = 0; i <= 255; i++)
  dirChars[i] = -1 ;
 dirChars['['] = TIF ;
 dirChars['|'] = TELSE ;
 dirChars[']'] = TFI ;
 dirChars['!'] = TMNOTE ;
 dirChars['#'] = THASH ;
 dirChars['*'] = TSTAR ;
 dirChars['='] = TEQUAL ;
 dirChars['%'] = TPERC ;
 dirChars['&'] = TAMP ;
 dirChars['^'] = THAT ;

#ifdef __NCCBUG
 threeLetterDirectives = (DirectiveEntry *)mymalloc(MaxDir * sizeof(DirectiveEntry)) ;
 fourLetterDirectives = (DirectiveEntry *)mymalloc(MaxDir * sizeof(DirectiveEntry)) ;
 fiveLetterDirectives = (DirectiveEntry *)mymalloc(MaxDir * sizeof(DirectiveEntry)) ;
 sixLetterDirectives = (DirectiveEntry *)mymalloc(MaxDir * sizeof(DirectiveEntry)) ;
#endif

 for (i = 0; i < MaxDir; i++)
  {
   threeLetterDirectives[i].key.length = 0 ;
   fourLetterDirectives[i].key.length = 0 ;
   fiveLetterDirectives[i].key.length = 0 ;
   sixLetterDirectives[i].key.length = 0 ;
  } ; /* for */
 for (i = TIF; i < TDUMMY; i++)
  {
   allowDirInCond[i] = FALSE ;
   disallowDirOutsideArea[i] = TRUE ;
  } ;

  allowDirInCond[TIF] = TRUE ;
  allowDirInCond[TELSE] = TRUE ;
  allowDirInCond[TFI] = TRUE ;
  allowDirInCond[TEND] = TRUE ;
  allowDirInCond[TLIB] = TRUE ;
  allowDirInCond[TLNK] = TRUE ;
  allowDirInCond[TWHILE] = TRUE ;
  allowDirInCond[TWEND] = TRUE ;
  allowDirInCond[TMACRO] = TRUE ;
  allowDirInCond[TMEND] = TRUE ;
  disallowDirOutsideArea[TEND] = FALSE ;
  disallowDirOutsideArea[TLIB] = FALSE ;
  disallowDirOutsideArea[TLNK] = FALSE ;
  disallowDirOutsideArea[TGET] = FALSE ;
#if 1 /* binary include support */
  disallowDirOutsideArea[TBGET] = FALSE ;
#endif
  disallowDirOutsideArea[TTTL] = FALSE ;
  disallowDirOutsideArea[TSUBTTL] = FALSE ;
  disallowDirOutsideArea[TASSERT] = FALSE ;
  disallowDirOutsideArea[TOPT] = FALSE ;
  disallowDirOutsideArea[TMACRO] = FALSE ;
  disallowDirOutsideArea[TMEND] = FALSE ;
  disallowDirOutsideArea[TMEXIT] = FALSE ;
  disallowDirOutsideArea[TWEND] = FALSE ;
  disallowDirOutsideArea[TWHILE] = FALSE ;
  disallowDirOutsideArea[TGBLA] = FALSE ;
  disallowDirOutsideArea[TGBLL] = FALSE ;
  disallowDirOutsideArea[TGBLS] = FALSE ;
  disallowDirOutsideArea[TLCLA] = FALSE ;
  disallowDirOutsideArea[TLCLL] = FALSE ;
  disallowDirOutsideArea[TLCLS] = FALSE ;
  disallowDirOutsideArea[TSETA] = FALSE ;
  disallowDirOutsideArea[TSETL] = FALSE ;
  disallowDirOutsideArea[TSETS] = FALSE ;
  disallowDirOutsideArea[TIMPORT] = FALSE ;
  disallowDirOutsideArea[TLABREF] = FALSE ;
  disallowDirOutsideArea[TEXPORT] = FALSE ;
  disallowDirOutsideArea[TEXTERN] = FALSE ;
  disallowDirOutsideArea[TIF] = FALSE ;
  disallowDirOutsideArea[TFI] = FALSE ;
  disallowDirOutsideArea[TELSE] = FALSE ;
  disallowDirOutsideArea[TRN] = FALSE ;
  disallowDirOutsideArea[TFN] = FALSE ;
  disallowDirOutsideArea[TCN] = FALSE ;
  disallowDirOutsideArea[TCP] = FALSE ;
  disallowDirOutsideArea[TSTAR] = FALSE ;
  disallowDirOutsideArea[THAT] = FALSE ;
  disallowDirOutsideArea[THASH] = FALSE ;
  disallowDirOutsideArea[TMNOTE] = FALSE ;
  disallowDirOutsideArea[TNOFP] = FALSE ;
  disallowDirOutsideArea[TMODULE] = FALSE ;
  syntax[TIF] = NoLabExp ;
  syntax[TELSE] = NoLabNoExp ;
  syntax[TFI] = NoLabNoExp ;
  syntax[TMNOTE] = NoLabExp ;
  syntax[THASH] = OptLabExp ;
  syntax[TSTAR] = LabExp ;
  syntax[TEQUAL] = OptLabExp ;
  syntax[TPERC] = OptLabExp ;
  syntax[TDCW] = OptLabExp ;
  syntax[TAMP] = OptLabExp ;
  syntax[THAT] = NoLabExp ;
  syntax[TEND] = NoCheck ;
  syntax[TLIB] = NoLabNoExp ;
  syntax[TLNK] = NoCheck ;
  syntax[TGET] = NoLabExp ;
#if 1 /* binary include support */
  syntax[TBGET] = NoLabExp ;
#endif
  syntax[TOPT] = NoLabExp ;
  syntax[TTTL] = NoLabExp ;
  syntax[TSUBTTL] = NoLabExp ;
  syntax[TRN] = LabExp ;
  syntax[TFN] = LabExp ;
  syntax[TCN] = LabExp ;
  syntax[TCP] = LabExp ;
  syntax[TWHILE] = NoLabExp ;
  syntax[TWEND] = NoLabNoExp ;
  syntax[TMACRO] = NoLabNoExp ;
  syntax[TMEXIT] = NoLabNoExp ;
  syntax[TMEND] = NoLabNoExp ;
  syntax[TGBLA] = NoLabExp ;
  syntax[TGBLL] = NoLabExp ;
  syntax[TGBLS] = NoLabExp ;
  syntax[TLCLA] = NoLabExp ;
  syntax[TLCLL] = NoLabExp ;
  syntax[TLCLS] = NoLabExp ;
  syntax[TSETA] = LabExp ;
  syntax[TSETL] = LabExp ;
  syntax[TSETS] = LabExp ;
  syntax[TASSERT] = NoLabExp ;
  syntax[TROUT] = OptLabNoExp ;
  syntax[TALIGN] = NoLabExp ;
  syntax[TLTORG] = NoLabNoExp ;
  syntax[TIMPORT] = NoLabExp ;
  syntax[TEXPORT] = NoLabExp ;
  syntax[TEXTERN] = LabNoExp ;  /* new special directive */
  syntax[TDCFS] = OptLabExp ;
  syntax[TDCFD] = OptLabExp ;
  syntax[TNOFP] = NoLabNoExp ;
}

/*---------------------------------------------------------------------------*/
/* EOF initdir/c */
