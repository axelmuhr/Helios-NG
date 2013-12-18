#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "iasm.h"

#define HASHSIZE 100

Symbol *hashtable[HASHSIZE];

int hash(char *s, int l)
{
        char *p;
        uint32 h = 0, g;

        for( p = s ; l-- ; p++ )
        {
                h = (h << 4) + *p;
                if( (g = (h & 0xf0000000)) != 0 )
                {
                        h ^= (g >> 24);
                        h ^= g;
                }
        }
        return (int)(h % HASHSIZE);
}

Symbol *lookup(char *s, int l, bool enter, STYPE type)
{
   Symbol **symp = &hashtable[hash(s,l)];
   Symbol *sym;
   char savech = s[l];

   s[l] = '\0';
   while( (sym = *symp) != NULL )
   {
      if( strcmp( sym->name, s ) == 0 )
         break;
      symp = &sym->cdr;
   }

   if( (sym == NULL) && enter )
   {
      sym = *symp = newsymbol(s,l);
      sym->cdr = NULL;
      sym->symtype = type;
      sym->symflags = 0;
   }
   s[l] = savech;
   return sym;
}

Symbol *newsymbol(char *s, int l)
{
   Symbol *sym = aalloc(sizeof(Symbol)+l);
   memset(sym, 0, sizeof(Symbol));
   memcpy(sym->name, s,l);
   sym->name[l] = '\0';
   return sym;
}

void resolvesym(Symbol *sym, int32 val, STYPE type)
{
   if( !(sym->symtype == type || sym->symtype == S_NULL ))
   {
      error("Invalid symbol resolution");
      return;
   }
   if( (pass == 2) && (sym->symtype != S_SET) )
   {
      if( sym->symv.symval != val || sym->symtype != type )
         error("Phasing error on symbol %s",sym->name);
   }
   else
   {
      sym->symv.symval = val;
      sym->symtype = type;
   }
   sym->symflags |= passevalflag;
}

#define MAXINSARGS 3

void newinstruction(char *name, uint32 base, uint32 modifiers, int nargs, ...)
{
   va_list ap;
   Symbol *sym = lookup(name,strlen(name), 1, S_INSTR);
   InstrInfo *ins = aalloc(sizeof(InstrInfo)+(MAXINSARGS-1)*sizeof(uint32));
   int i;

   sym->symflags |= sf_invisible;

   va_start(ap, nargs);

   resolvesym(sym, (int32)ins, S_INSTR);

   ins->base        = base;
   ins->modifiers   = modifiers;
   ins->nargs       = nargs;
   for( i = 0; i<MAXINSARGS; i++)
      ins->argmodes[i] = va_arg(ap,uint32);
   va_end(ap);
}

typedef struct insinit {
   char   *name;
   uint32  base;
   uint32  modifiers;
   int     nargs;
   uint32  argmodes[MAXINSARGS];
} insinit;

insinit instructions[] =
{
   {"ld",      0x00000000, I_X,         3, {M_REGOFF|M_ITYPE|M_S1FLD, M_SRC2, M_RDEST}},
   {"st",      0x0c000000, I_X,         3, {M_SRC1NI, M_REGOFF|M_SPLITOFFSET|M_NOREG,M_SRC2}},
   {"ixfr",    0x08000000, 0,           2, {M_SRC1NI, M_FRDEST}},
   {"fld",     0x20000000, I_Y|I_OLT,   3, {M_REGOFF|M_AUTOINC|M_ITYPE|M_S1FLD, M_SRC2, M_FRDEST}},
   {"pfld",    0x60000000, I_Z|I_OLT,   3, {M_REGOFF|M_AUTOINC|M_ITYPE|M_S1FLD, M_SRC2, M_FRDEST}},
   {"fst",     0x28000000, I_Y|I_OLT,   3, {M_FRDEST, M_REGOFF|M_AUTOINC|M_ITYPE|M_NOREG|M_S1FLD, M_SRC2}},
   {"pst",     0x3c000000, I_D|I_OLT,   2, {M_FRDEST, M_REGOFF|M_AUTOINC|M_ITYPE|M_NOREG|M_S1FLD, M_SRC2}},
   {"addu",    0x80000000, 0,           3, {M_SRC1, M_SRC2, M_RDEST}},
   {"adds",    0x90000000, 0,           3, {M_SRC1, M_SRC2, M_RDEST}},
   {"subu",    0x88000000, 0,           3, {M_SRC1, M_SRC2, M_RDEST}},
   {"subs",    0x98000000, 0,           3, {M_SRC1, M_SRC2, M_RDEST}},
   {"shl",     0xa0000000, 0,           3, {M_SRC1, M_SRC2, M_RDEST}},
   {"mov",     0xa0000000, 0,           2, {M_SRC2, M_RDEST}},
   {"nop",     0xa0000000, 0,           0},
   {"shr",     0xa8000000, 0,           3, {M_SRC1, M_SRC2, M_RDEST}},
   {"shra",    0xb8000000, 0,           3, {M_SRC1, M_SRC2, M_RDEST}},
   {"shrd",    0xb0000000, 0,           3, {M_SRC1, M_SRC2, M_RDEST}},
   {"fnop",    0xb0000000, 0,           0},
   {"trap",    0x44000000, 0,           3, {M_CONST|M_ITYPE, M_SRC2, M_RDEST}},
   {"intovr",  0x4c000004, 0,           0, {}},
   {"and",     0xc0000000, 0,           3, {M_SRC1, M_SRC2, M_RDEST}},
   {"andh",    0xc8000000, 0,           3, {M_CONST|M_ITYPE, M_SRC2, M_RDEST}},
   {"andnot",  0xd0000000, 0,           3, {M_SRC1, M_SRC2, M_RDEST}},
   {"andnoth", 0xd8000000, 0,           3, {M_CONST|M_ITYPE, M_SRC2, M_RDEST}},
   {"or",      0xe0000000, 0,           3, {M_SRC1, M_SRC2, M_RDEST}},
   {"orh",     0xe8000000, 0,           3, {M_CONST|M_ITYPE, M_SRC2, M_RDEST}},
   {"xor",     0xf0000000, 0,           3, {M_SRC1, M_SRC2, M_RDEST}},
   {"xorh",    0xf8000000, 0,           3, {M_CONST|M_ITYPE, M_SRC2, M_RDEST}},
   {"br",      0x68000000, 0,           1, {M_LBROFF}},
   {"bc",      0x70000000, I_T,         1, {M_LBROFF}},
   {"bnc",     0x78000000, I_T,         1, {M_LBROFF}},
   {"bte",     0x58000000, 0,           3, {M_SRC1S, M_SRC2, M_SBROFF}},
   {"btne",    0x50000000, 0,           3, {M_SRC1S, M_SRC2, M_SBROFF}},
   {"bla",     0xb4000000, 0,           3, {M_SRC1NI, M_SRC2, M_SBROFF}},
   {"call",    0x6c000000, 0,           1, {M_LBROFF}},
   {"calli",   0x4c000002, 0,           1, {M_SRC1NI}},
   {"bri",     0x40000000, 0,           1, {M_SRC1NI}},
   {"flush",   0x34000000, I_OLT|I_FLUSH, 2, {M_REGOFF|M_AUTOINC|M_SRC1,M_SRC2}},
   {"ld.c",    0x30000000, I_CM,        2, {M_CTRLREG|M_S2FLD,M_RDEST}},
   {"st.c",    0x38000000, I_CM,        2, {M_SRC1NI,M_CTRLREG|M_S2FLD}},
   {"lock",    0x4c000001, 0,           0, {}},
   {"unlock",  0x4c000007, 0,           0, {}},
   {"fmul",    0x48000020, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"pfmul",   0x48000420, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"pfmul3",  0x48000024, I_DD|I_DIM,  3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"fmlow",   0x48000021, I_DD|I_DIM,  3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"frcp",    0x48000022, I_P|I_DIM,   2, {M_FSRC2,M_FRDEST}},
   {"frsqr",   0x48000023, I_P|I_DIM,   2, {M_FSRC2,M_FRDEST}},
   {"fadd",    0x48000030, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"pfadd",   0x48000430, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"fsub",    0x48000031, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"pfsub",   0x48000431, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"pfgt",    0x48000034, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"pfle",    0x480000b4, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"pfeq",    0x48000035, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"fix",     0x48000032, I_P|I_DIM,   2, {M_FSRC1,M_FRDEST}},
   {"pfix",    0x48000432, I_P|I_DIM,   2, {M_FSRC1,M_FRDEST}},
   {"ftrunc",  0x4800003a, I_P|I_DIM,   2, {M_FSRC1,M_FRDEST}},
   {"pftrunc", 0x4800043a, I_P|I_DIM,   2, {M_FSRC1,M_FRDEST}},
   {"r2p1",    0x48000400, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"r2pt",    0x48000401, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"r2ap1",   0x48000402, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"r2apt",   0x48000403, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"i2p1",    0x48000404, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"i2pt",    0x48000405, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"i2ap1",   0x48000406, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"i2apt",   0x48000407, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"rat1p2",  0x48000408, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"m12apm",  0x48000409, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"ra1p2",   0x4800040a, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"m12ttpa", 0x4800040b, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"iat1p2",  0x4800040c, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"m12tpm",  0x4800040d, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"ia1p2",   0x4800040e, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"m12tpa",  0x4800040f, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"r2s1",    0x48000410, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"r2st",    0x48000411, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"r2as1",   0x48000412, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"r2ast",   0x48000413, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"i2s1",    0x48000414, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"i2st",    0x48000415, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"i2as1",   0x48000416, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"i2ast",   0x48000417, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"rat1s2",  0x48000418, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"m12asm",  0x48000419, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"ra1s2",   0x4800041a, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"m12ttsa", 0x4800041b, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"iat1s2",  0x4800041c, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"m12tsm",  0x4800041d, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"ia1s2",   0x4800041e, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"m12tsa",  0x4800041f, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mr2p1",   0x48000000, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mr2pt",   0x48000001, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mr2mp1",  0x48000002, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mr2mpt",  0x48000003, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mi2p1",   0x48000004, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mi2pt",   0x48000005, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mi2mp1",  0x48000006, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mi2mpt",  0x48000007, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mrmt1p2", 0x48000008, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mm12mpm", 0x48000009, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mrm1p2",  0x4800000a, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mm12ttpm",0x4800000b, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mimt1p2", 0x4800000c, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mm12tpm", 0x4800000d, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mim1p2",  0x4800000e, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
/*   {"mm12tpm", 0x4800000f, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}}, */
   {"mr2s1",   0x48000010, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mr2st",   0x48000011, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mr2ms1",  0x48000012, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mr2mst",  0x48000013, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mi2s1",   0x48000014, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mi2st",   0x48000015, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mi2ms1",  0x48000016, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mi2mst",  0x48000017, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mrmt1s2", 0x48000018, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mm12msm", 0x48000019, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mrm1s2",  0x4800001a, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mm12ttsm",0x4800001b, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mimt1s2", 0x4800001c, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mm12tsm", 0x4800001d, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"mim1s2",  0x4800001e, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
/*   {"mm12tsm", 0x4800001f, I_P|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}}, */
   {"fisub",   0x4800004d, I_W|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"pfisub",  0x4800044d, I_W|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"fiadd",   0x48000049, I_W|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"pfiadd",  0x48000449, I_W|I_DIM,   3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"fzchks",  0x4800005f, I_DIM,       3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"pfzchks", 0x4800045f, I_DIM,       3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"fzchkl",  0x48000057, I_DIM,       3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"pfzchkl", 0x48000457, I_DIM,       3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"faddp",   0x48000050, I_DIM,       3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"pfaddp",  0x48000450, I_DIM,       3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"pfaddz",  0x48000051, I_DIM,       3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"faddz",   0x48000451, I_DIM,       3, {M_FSRC1,M_FSRC2,M_FRDEST}},
   {"form",    0x4800005a, I_DIM,       2, {M_FSRC1,M_FRDEST}},
   {"pform",   0x4800045a, I_DIM,       2, {M_FSRC1,M_FRDEST}},
   {"fxfr",    0x48000040, I_DIM,       2, {M_FSRC1,M_RDEST}},
   {"get",     PS_GET,     I_PSEUDO,    1, {M_STRING} },
   {"include", PS_GET,     I_PSEUDO,    1, {M_STRING} },
   {"set",     PS_SET,     I_PSEUDO,    1, {M_CONST} },
   {"equ",     PS_EQU,     I_PSEUDO,    1, {M_CONST} },
   {"dc",      PS_DC,      I_PSEUDO|I_X|I_Y, -1, {M_CONST|M_STRING}},
   {"nolist",  PS_NOLIST,  I_PSEUDO,    0},
   {"list",    PS_LIST,    I_PSEUDO,    1, {M_CONST|M_OPTIONAL}},
   {"equr",    PS_EQUR,    I_PSEUDO,    1, {M_IREG|M_FREG|M_CTRLREG|M_SHFT0}},
   {"if",      PS_IF,      I_PSEUDO,    1, {M_CONST}},
   {"ifd",     PS_IFD,     I_PSEUDO,    1, {M_CONST}},
   {"ifnd",    PS_IFND,    I_PSEUDO,    1, {M_CONST}},
   {"else",    PS_ELSE,    I_PSEUDO,    0},
   {"endc",    PS_ENDC,    I_PSEUDO,    0},
   {"error",   PS_ERROR,   I_PSEUDO,    1, {M_STRING} },
   {"ds",      PS_DS,      I_PSEUDO|I_X|I_Y, 1, {M_CONST} },
   {"global",  PS_GLOBAL,  I_PSEUDO,    1, {M_CONST} },
   {"data",    PS_DATA,    I_PSEUDO|I_X|I_Y, 1, { M_CONST} },
   {"init",    PS_INIT,    I_PSEUDO,    0},
   {"module",  PS_MODULE,  I_PSEUDO, 1, {M_CONST} },
   {"xlabel",  PS_XLABEL,  I_PSEUDO,   -1, {M_CONST} },
   {"xdata",   PS_XDATA,   I_PSEUDO,   -1, {M_CONST} }
};

void initialise_instructions(void)
{
   insinit *ins;
   int nins = sizeof(instructions)/sizeof(insinit);

   for(ins = instructions; nins--; ins++)
   {
      newinstruction(ins->name, ins->base, ins->modifiers, ins->nargs,
                        ins->argmodes[0],ins->argmodes[1],ins->argmodes[2]);
   }
}

void newregister(char *name, STYPE type, int32 num)
{
   Symbol *sym = lookup(name, strlen(name), 1, type);
   sym->symflags |= sf_invisible;
   resolvesym(sym, num, type);
}

void initialise_registers(void)
{
   newregister("r0", S_IREGISTER, 0);
   newregister("r1", S_IREGISTER, 1);
   newregister("r2", S_IREGISTER, 2);
   newregister("r3", S_IREGISTER, 3);
   newregister("r4", S_IREGISTER, 4);
   newregister("r5", S_IREGISTER, 5);
   newregister("r6", S_IREGISTER, 6);
   newregister("r7", S_IREGISTER, 7);
   newregister("r8", S_IREGISTER, 8);
   newregister("r9", S_IREGISTER, 9);
   newregister("r10", S_IREGISTER, 10);
   newregister("r11", S_IREGISTER, 11);
   newregister("r12", S_IREGISTER, 12);
   newregister("r13", S_IREGISTER, 13);
   newregister("r14", S_IREGISTER, 14);
   newregister("r15", S_IREGISTER, 15);
   newregister("r16", S_IREGISTER, 16);
   newregister("r17", S_IREGISTER, 17);
   newregister("r18", S_IREGISTER, 18);
   newregister("r19", S_IREGISTER, 19);
   newregister("r20", S_IREGISTER, 20);
   newregister("r21", S_IREGISTER, 21);
   newregister("r22", S_IREGISTER, 22);
   newregister("r23", S_IREGISTER, 23);
   newregister("r24", S_IREGISTER, 24);
   newregister("r25", S_IREGISTER, 25);
   newregister("r26", S_IREGISTER, 26);
   newregister("r27", S_IREGISTER, 27);
   newregister("r28", S_IREGISTER, 28);
   newregister("r29", S_IREGISTER, 29);
   newregister("r30", S_IREGISTER, 30);
   newregister("r31", S_IREGISTER, 31);

   newregister("f0", S_FREGISTER, 0);
   newregister("f1", S_FREGISTER, 1);
   newregister("f2", S_FREGISTER, 2);
   newregister("f3", S_FREGISTER, 3);
   newregister("f4", S_FREGISTER, 4);
   newregister("f5", S_FREGISTER, 5);
   newregister("f6", S_FREGISTER, 6);
   newregister("f7", S_FREGISTER, 7);
   newregister("f8", S_FREGISTER, 8);
   newregister("f9", S_FREGISTER, 9);
   newregister("f10", S_FREGISTER, 10);
   newregister("f11", S_FREGISTER, 11);
   newregister("f12", S_FREGISTER, 12);
   newregister("f13", S_FREGISTER, 13);
   newregister("f14", S_FREGISTER, 14);
   newregister("f15", S_FREGISTER, 15);
   newregister("f16", S_FREGISTER, 16);
   newregister("f17", S_FREGISTER, 17);
   newregister("f18", S_FREGISTER, 18);
   newregister("f19", S_FREGISTER, 19);
   newregister("f20", S_FREGISTER, 20);
   newregister("f21", S_FREGISTER, 21);
   newregister("f22", S_FREGISTER, 22);
   newregister("f23", S_FREGISTER, 23);
   newregister("f24", S_FREGISTER, 24);
   newregister("f25", S_FREGISTER, 25);
   newregister("f26", S_FREGISTER, 26);
   newregister("f27", S_FREGISTER, 27);
   newregister("f28", S_FREGISTER, 28);
   newregister("f29", S_FREGISTER, 29);
   newregister("f30", S_FREGISTER, 30);
   newregister("f31", S_FREGISTER, 31);

   newregister("fir", S_CREGISTER, 0);
   newregister("psr", S_CREGISTER, 1);
   newregister("dirbase", S_CREGISTER, 2);
   newregister("db", S_CREGISTER, 3);
   newregister("fsr", S_CREGISTER, 4);
   newregister("epsr", S_CREGISTER, 5);
}

void predefine(char *name, int32 val, STYPE t)
{
   Symbol *sym = lookup(name, strlen(name), 1,t );
   resolvesym(sym, val, t);
}

void syminit(void)
{  if( pass == 1 )
   {  initialise_instructions();
      initialise_registers();
   }
   predefine("LO_CODEONLY",LO_CODEONLY,S_EQU);
   predefine("LO_NOTMACRO",LO_NOTMACRO,S_EQU);
}
