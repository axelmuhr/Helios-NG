/**
*
* Title:  Helios Debugger - Disassembler.
*
* Author: Andy England
*
* Date:   September 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#include "tla.h"

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/disasm.c,v 1.3 1992/10/27 15:44:18 nickc Exp $";

  /* direct functions */
PRIVATE char *directfns[] =
{
  "j",
  "ldlp",
  "pfix",
  "ldnl",
  "ldc",
  "ldnlp",
  "nfix",
  "ldl",
  "adc",
  "call",
  "cj",
  "ajw",
  "eqc",
  "stl",
  "stnl",
  "opr"
};

PRIVATE char *oper[] =
{
  /* one byte operations */
  "rev",
  "lb",
  "bsub",
  "endp",
  "diff",
  "add",
  "gcall",
  "in",
  "prod",
  "gt",
  "wsub",
  "out",
  "sub",
  "startp",
  "outbyte",
  "outword",

  /* two byte operations */
  "seterr",
  0,
  "resetch",
  "csub0",
  0,
  "stopp",
  "ladd",
  "stlb",
  "sthf",
  "norm",
  "ldiv",
  "ldpi",
  "stlf",
  "xdble",
  "ldpri",
  "rem",

  "ret",
  "lend",
  "ldtimer",
  0,
  0,
  0,
  0,
  0,
  0,
  "testerr",
  "testpranal",
  "tin",
  "div",
  0,
  "dist",
  "disc",

  "diss",
  "lmul",
  "not",
  "xor",
  "bcnt",
  "lshr",
  "lshl",
  "lsum",
  "lsub",
  "runp",
  "xword",
  "sb",
  "gajw",
  "savel",
  "saveh",
  "wcnt",

  "shr",
  "shl",
  "mint",
  "alt",
  "altwt",
  "altend",
  "and",
  "enbt",
  "enbc",
  "enbs",
  "move",
  "or",
  "csngl",
  "ccnt1",
  "talt",
  "ldiff",

  "sthb",
  "taltwt",
  "sum",
  "mul",
  "sttimer",
  "stoperr",
  "cword",
  "clrhalterr",
  "sethalterr",
  "testhalterr",
  "dup",
  "move2dinit",
  "move2dall",
  "move2dnonzero",
  "move2dzero",
  0,

  0,
  0,
  0,
  "unpacksn",
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  "postnormsn",
  "roundsn",
  0,
  0,

  0,
  "ldinf",
  "fmul",
  "cflerr",
  "crcword",
  "crcbyte",
  "bitcnt",
  "bitrevword",
  "bitrevnbits",
  0,
  0,
  0,
  0,
  0,
  0,
  0,

  0,
  "wsubdb",
  "fpldnldbi",
  "fpchkerr",
  "fpstnldb",
  0,
  "fpldnlsni",
  "fpadd",
  "fpstnlsn",
  "fpsub",
  "fpldnldb",
  "fpmul",
  "fpdiv",
  0,
  "fpldnlsn",
  "fpremfirst",

  "fpremstep",
  "fpnan",
  "fpordered",
  "fpnotfinite",
  "fpgt",
  "fpeq",
  "fpi32tor32",
  0,
  "fpi32tor64",
  0,
  "fpb32tor64",
  0,
  "fptesterr",
  "fprtoi32",
  "fpstnli32",
  "fpldzerosn",

  "fpldzerodb",
  "fpint",
  0,
  "fpdup",
  "fprev",
  0,
  "fpldnladddb",
  0,
  "fpldnlmuldb",
  0,
  "fpldnladdsn",
  "fpentry",
  "fpldnlmulsn",
  0,
  0,
  0
};

/* three byte operations */
/* -- crf : 07/08/91 - not used -
PRIVATE char *oper3[] =
{
  0,
  "fpusqrtfirst",
  "fpusqrtstep",
  "fpusqrtlast",
  "fpurp",
  "fpurm",
  "fpurz",
  "fpur32tor64",
  "fpur64tor32",
  "fpuexpdec32",
  "fpuexpinc32",
  "fpuabs",
  0,
  "fpunoround",
  "fpuchki32",
  "fpuchki64"
};
*/

/**
*** The various transputer opcodes
**/

#define f_pfix  0x2
#define f_nfix  0x6
#define f_call  0x9
#define f_j     0x0
#define f_cj    0xa
#define f_ldc   0x4
#define f_ldnl  0x3
#define f_ldnlp 0x5
#define f_stnl  0xe
#define f_ldl   0x7
#define f_stl   0xd
#define f_ldlp  0x1

PUBLIC UBYTE *disasm(DEBUG *debug, UBYTE *addr)
{
  UBYTE byte = 0;
  WORD function = 0;
  WORD operand = 0;

  forever
  {
    byte = peekbyte(debug, addr++);
    function = (UWORD)byte >> 4;
    operand = (operand << 4) | ((UWORD)byte & 0x0F);
    switch ((int)function)
    {
      case f_nfix:
      operand = ~operand;

      case f_pfix:
      break;

      default:
      return addr;
    }
  }
  if (0 <= function AND function <= 0x0E)
  {
    dprintf(debug->display, "%-5s %8lx", directfns[function], operand);
  }
  else if (0 <= operand AND operand <= 0xAC AND oper[operand] != 0)
  {
    dprintf(debug->display, "%-5s", oper[operand]);
  }
  else dprintf(debug->display, "UNKNOWN %2lx %8lx", function, operand);
  return addr;
}

#else /* not __TRAN */
#endif /* not __TRAN */
