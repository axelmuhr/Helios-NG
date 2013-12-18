/****************************************************************/
/*                                                              */
/* keyform.c                                                    */
/*                                                              */
/* Generate the keyword table with the instruction and          */
/* directive names.                                             */
/*                                                              */
/****************************************************************/
/* $Id: keyform.c,v 1.11 1994/08/09 16:43:25 al Exp $ */



#include "asm.h"

#define TABSIZE 5000

INT 			dicttab[ GLOBAL_HASHSIZE ];
INT 			ccount[  GLOBAL_HASHSIZE ];
struct keyentry *	ends[    GLOBAL_HASHSIZE ];

UBYTE keytab[ TABSIZE ];

INT keyupb = 4;

FILE *ofd;

#include "hash.c"

#ifdef SWAPKEYTABLE
#define cat1(H) (WORD)((WORD)H<<24)
#define cat2(H,I) (WORD)((((WORD)H<<8)+(WORD)I)<<16)
#define cat3(H,I,J) (WORD)((((WORD)H<<16)+((WORD)I<<8)+(WORD)J)<<8)
#define cat4(H,I,J,K) (WORD)(((WORD)H<<24)+((WORD)I<<16)+((WORD)J<<8)+(WORD)K)
#else
#define cat1(H) (WORD)((WORD)H)
#define cat2(H,I) (WORD)(((WORD)I<<8)+(WORD)H)
#define cat3(H,I,J) (WORD)(((WORD)J<<16)+((WORD)I<<8)+(WORD)H)
#define cat4(H,I,J,K) (WORD)(((WORD)K<<24)+((WORD)J<<16)+((WORD)I<<8)+(WORD)H)
#endif

int
main()
{
  int i;


  printf("KeyForm\n");

  ofd = fopen("keytab.c","w");

  if ( ofd == NULL )
    {
      printf("Cannot open keytab.c\n");
      exit(20);
    }

  /* fprintf(ofd,"#include "ttypes.h"\n\nINT keytab[] =\n{\n"); */
  fprintf(ofd,"\nlong keytab[] =\n{\n");

  for( i = 0; i < GLOBAL_HASHSIZE ; i++)
    {
      dicttab[ i ] = 0;
      ccount[  i ] = 0;
    }

  keyform();
  keyform1();
  keyform2();

  puttab(keytab,keyupb);

  fprintf(ofd,"};\n\nlong dicttab[] = {\n");

  for( i = 0 ; i < GLOBAL_HASHSIZE-1; i++ )
    {
      if( (i % 10) == 0  ) fprintf(ofd,"\n   ");
      fprintf(ofd,"%ld,",dicttab[i]);
    }

  fprintf(ofd,"%ld };\n",dicttab[ GLOBAL_HASHSIZE - 1 ]);

  fprintf(ofd,"\n/* chain lengths: ");

  for( i = 0 ; i < GLOBAL_HASHSIZE; i++ )
    {
      if( (i % 10) == 0  ) fprintf(ofd,"\n   ");
      fprintf(ofd,"%ld,",ccount[i]);
    }

  fprintf(ofd,"\n*/\n\n");

  fclose(ofd);

  exit(0);
}

keyform()
{

        /* direct functions */
        install("j",            s_direct,       (WORD)0x00);
        install("ldlp",         s_direct,       (WORD)0x10);
        install("pfix",         s_direct,       (WORD)0x20);
        install("ldnl",         s_direct,       (WORD)0x30);
        install("ldc",          s_direct,       (WORD)0x40);
        install("ldnlp",        s_direct,       (WORD)0x50);
        install("nfix",         s_direct,       (WORD)0x60);
        install("ldl",          s_direct,       (WORD)0x70);
        install("adc",          s_direct,       (WORD)0x80);
        install("call",         s_direct,       (WORD)0x90);
        install("cj",           s_direct,       (WORD)0xa0);
        install("ajw",          s_direct,       (WORD)0xb0);
        install("eqc",          s_direct,       (WORD)0xc0);
        install("stl",          s_direct,       (WORD)0xd0);
        install("stnl",         s_direct,       (WORD)0xe0);
        install("opr",          s_direct,       (WORD)0xf0);

        /* one byte operations */
        install("jump0",	s_oper1,	cat1(0x00));
        install("rev",          s_oper1,        cat1(0xf0));
        install("lb",           s_oper1,        cat1(0xf1));
        install("bsub",         s_oper1,        cat1(0xf2));
        install("endp",         s_oper1,        cat1(0xf3));
        install("diff",         s_oper1,        cat1(0xf4));
        install("add",          s_oper1,        cat1(0xf5));
        install("gcall",        s_oper1,        cat1(0xf6));
        install("in",           s_oper1,        cat1(0xf7));
        install("prod",         s_oper1,        cat1(0xf8));
        install("gt",           s_oper1,        cat1(0xf9));
        install("wsub",         s_oper1,        cat1(0xfa));
        install("out",          s_oper1,        cat1(0xfb));
        install("sub",          s_oper1,        cat1(0xfc));
        install("startp",       s_oper1,        cat1(0xfd));
        install("outbyte",      s_oper1,        cat1(0xfe));
        install("outword",      s_oper1,        cat1(0xff));


        /* two byte operations */
        install("seterr",       s_oper2,        cat2(0x21,0xf0));
        install("resetch",      s_oper2,        cat2(0x21,0xf2));
        install("csub0",        s_oper2,        cat2(0x21,0xf3));
        install("stopp",        s_oper2,        cat2(0x21,0xf5));
        install("ladd",         s_oper2,        cat2(0x21,0xf6));
        install("stlb",         s_oper2,        cat2(0x21,0xf7));
        install("sthf",         s_oper2,        cat2(0x21,0xf8));
        install("norm",         s_oper2,        cat2(0x21,0xf9));
        install("ldiv",         s_oper2,        cat2(0x21,0xfa));
        install("ldpi",         s_oper2,        cat2(0x21,0xfb));
        install("stlf",         s_oper2,        cat2(0x21,0xfc));
        install("xdble",        s_oper2,        cat2(0x21,0xfd));
        install("ldpri",        s_oper2,        cat2(0x21,0xfe));
        install("rem",          s_oper2,        cat2(0x21,0xff));

}
keyform1()
{
        install("ret",          s_oper2,        cat2(0x22,0xf0));
        install("lend",         s_oper2,        cat2(0x22,0xf1));
        install("ldtimer",      s_oper2,        cat2(0x22,0xf2));
        install("testlds",      s_oper2,        cat2(0x22,0xf3));        
        install("testlde",      s_oper2,        cat2(0x22,0xf4));        
        install("testldd",      s_oper2,        cat2(0x22,0xf5));        
        install("teststs",      s_oper2,        cat2(0x22,0xf6));        
        install("testste",      s_oper2,        cat2(0x22,0xf7));        
        install("teststd",      s_oper2,        cat2(0x22,0xf8));        
        install("testerr",      s_oper2,        cat2(0x22,0xf9));
        install("testpranal",   s_oper2,        cat2(0x22,0xfa));
        install("tin",          s_oper2,        cat2(0x22,0xfb));
        install("div",          s_oper2,        cat2(0x22,0xfc));
        install("testhardchan", s_oper2,        cat2(0x22,0xfd));
        install("dist",         s_oper2,        cat2(0x22,0xfe));
        install("disc",         s_oper2,        cat2(0x22,0xff));

        install("diss",         s_oper2,        cat2(0x23,0xf0));
        install("lmul",         s_oper2,        cat2(0x23,0xf1));
        install("not",          s_oper2,        cat2(0x23,0xf2));
        install("xor",          s_oper2,        cat2(0x23,0xf3));
        install("bcnt",         s_oper2,        cat2(0x23,0xf4));
        install("lshr",         s_oper2,        cat2(0x23,0xf5));
        install("lshl",         s_oper2,        cat2(0x23,0xf6));
        install("lsum",         s_oper2,        cat2(0x23,0xf7));
        install("lsub",         s_oper2,        cat2(0x23,0xf8));
        install("runp",         s_oper2,        cat2(0x23,0xf9));
        install("xword",        s_oper2,        cat2(0x23,0xfa));
        install("sb",           s_oper2,        cat2(0x23,0xfb));
        install("gajw",         s_oper2,        cat2(0x23,0xfc));
        install("savel",        s_oper2,        cat2(0x23,0xfd));
        install("saveh",        s_oper2,        cat2(0x23,0xfe));
        install("wcnt",         s_oper2,        cat2(0x23,0xff));

        install("shr",          s_oper2,        cat2(0x24,0xf0));
        install("shl",          s_oper2,        cat2(0x24,0xf1));
        install("mint",         s_oper2,        cat2(0x24,0xf2));
        install("alt",          s_oper2,        cat2(0x24,0xf3));
        install("altwt",        s_oper2,        cat2(0x24,0xf4));
        install("altend",       s_oper2,        cat2(0x24,0xf5));
        install("and",          s_oper2,        cat2(0x24,0xf6));
        install("enbt",         s_oper2,        cat2(0x24,0xf7));
        install("enbc",         s_oper2,        cat2(0x24,0xf8));
        install("enbs",         s_oper2,        cat2(0x24,0xf9));
        install("move",         s_oper2,        cat2(0x24,0xfa));
        install("or",           s_oper2,        cat2(0x24,0xfb));
        install("csngl",        s_oper2,        cat2(0x24,0xfc));
        install("ccnt1",        s_oper2,        cat2(0x24,0xfd));
        install("talt",         s_oper2,        cat2(0x24,0xfe));
        install("ldiff",        s_oper2,        cat2(0x24,0xff));

        install("sthb",         s_oper2,        cat2(0x25,0xf0));
        install("taltwt",       s_oper2,        cat2(0x25,0xf1));
        install("sum",          s_oper2,        cat2(0x25,0xf2));
        install("mul",          s_oper2,        cat2(0x25,0xf3));
        install("sttimer",      s_oper2,        cat2(0x25,0xf4));
        install("stoperr",      s_oper2,        cat2(0x25,0xf5));
        install("cword",        s_oper2,        cat2(0x25,0xf6));
        install("clrhalterr",   s_oper2,        cat2(0x25,0xf7));
        install("sethalterr",   s_oper2,        cat2(0x25,0xf8));
        install("testhalterr",  s_oper2,        cat2(0x25,0xf9));
        install("dup",          s_oper2,        cat2(0x25,0xfa));
        install("move2dinit",   s_oper2,        cat2(0x25,0xfb));
        install("move2dall",    s_oper2,        cat2(0x25,0xfc));
        install("move2dnonzero",s_oper2,        cat2(0x25,0xfd));
        install("move2dzero",   s_oper2,        cat2(0x25,0xfe));

        install("unpacksn",     s_oper2,        cat2(0x26,0xf3));
        install("postnormsn",   s_oper2,        cat2(0x26,0xfc));
        install("roundsn",      s_oper2,        cat2(0x26,0xfd));
}
keyform2()
{
        install("ldinf",        s_oper2,        cat2(0x27,0xf1));
        install("fmul",         s_oper2,        cat2(0x27,0xf2));
        install("cflerr",       s_oper2,        cat2(0x27,0xf3));
        install("crcword",      s_oper2,        cat2(0x27,0xf4));
        install("crcbyte",      s_oper2,        cat2(0x27,0xf5));
        install("bitcnt",       s_oper2,        cat2(0x27,0xf6));
        install("bitrevword",   s_oper2,        cat2(0x27,0xf7));
        install("bitrevnbits",  s_oper2,        cat2(0x27,0xf8));
        install("pop",          s_oper2,        cat2(0x27,0xf9));
        install("timerdisableh",s_oper2,        cat2(0x27,0xfa));
        install("timerdisablel",s_oper2,        cat2(0x27,0xfb));
        install("timerenableh", s_oper2,        cat2(0x27,0xfc));
        install("timerenablel", s_oper2,        cat2(0x27,0xfd));
        install("ldmemstartval",s_oper2,        cat2(0x27,0xfe));

        install("wsubdb",       s_oper2,        cat2(0x28,0xf1));
        install("fpldnldbi",    s_oper2,        cat2(0x28,0xf2));
        install("fpchkerr",     s_oper2,        cat2(0x28,0xf3));
        install("fpstnldb",     s_oper2,        cat2(0x28,0xf4));
        install("fpldnlsni",    s_oper2,        cat2(0x28,0xf6));
        install("fpadd",        s_oper2,        cat2(0x28,0xf7));
        install("fpstnlsn",     s_oper2,        cat2(0x28,0xf8));
        install("fpsub",        s_oper2,        cat2(0x28,0xf9));
        install("fpldnldb",     s_oper2,        cat2(0x28,0xfa));
        install("fpmul",        s_oper2,        cat2(0x28,0xfb));
        install("fpdiv",        s_oper2,        cat2(0x28,0xfc));
        install("fpldnlsn",     s_oper2,        cat2(0x28,0xfe));
        install("fpremfirst",   s_oper2,        cat2(0x28,0xff));

        install("fpremstep",    s_oper2,        cat2(0x29,0xf0));
        install("fpnan",        s_oper2,        cat2(0x29,0xf1));
        install("fpordered",    s_oper2,        cat2(0x29,0xf2));
        install("fpnotfinite",  s_oper2,        cat2(0x29,0xf3));
        install("fpgt",         s_oper2,        cat2(0x29,0xf4));
        install("fpeq",         s_oper2,        cat2(0x29,0xf5));
        install("fpi32tor32",   s_oper2,        cat2(0x29,0xf6));
        install("fpi32tor64",   s_oper2,        cat2(0x29,0xf8));
        install("fpb32tor64",   s_oper2,        cat2(0x29,0xfa));
        install("fptesterr",    s_oper2,        cat2(0x29,0xfc));
        install("fprtoi32",     s_oper2,        cat2(0x29,0xfd));
        install("fpstnli32",    s_oper2,        cat2(0x29,0xfe));
        install("fpldzerosn",   s_oper2,        cat2(0x29,0xff));

        install("fpldzerodb",   s_oper2,        cat2(0x2a,0xf0));
        install("fpint",        s_oper2,        cat2(0x2a,0xf1));
        install("fpdup",        s_oper2,        cat2(0x2a,0xf3));
        install("fprev",        s_oper2,        cat2(0x2a,0xf4));
        install("fpldnladddb",  s_oper2,        cat2(0x2a,0xf6));
        install("fpldnlmuldb",  s_oper2,        cat2(0x2a,0xf8));
        install("fpldnladdsn",  s_oper2,        cat2(0x2a,0xfa));
        install("fpentry",      s_oper2,        cat2(0x2a,0xfb));
        install("fpldnlmulsn",  s_oper2,        cat2(0x2a,0xfc));

	install("break",	s_oper2,	cat2(0x2b,0xf1));
	install("clrj0break",	s_oper2,	cat2(0x2b,0xf2));
	install("setj0break",	s_oper2,	cat2(0x2b,0xf3));
	install("testj0break",	s_oper2,	cat2(0x2b,0xf4));	

        /* three byte operations */
	install("start",	s_oper3,	cat3(0x21,0x2f,0xff));
	install("lddevid",	s_oper3,	cat3(0x21,0x27,0xfc));
        install("fpusqrtfirst", s_oper3,        cat3(0x41,0x2a,0xfb));
        install("fpusqrtstep",  s_oper3,        cat3(0x42,0x2a,0xfb));
        install("fpusqrtlast",  s_oper3,        cat3(0x43,0x2a,0xfb));
        install("fpurp",        s_oper3,        cat3(0x44,0x2a,0xfb));
        install("fpurm",        s_oper3,        cat3(0x45,0x2a,0xfb));
        install("fpurz",        s_oper3,        cat3(0x46,0x2a,0xfb));
        install("fpur32tor64",  s_oper3,        cat3(0x47,0x2a,0xfb));
        install("fpur64tor32",  s_oper3,        cat3(0x48,0x2a,0xfb));
        install("fpuexpdec32",  s_oper3,        cat3(0x49,0x2a,0xfb));
        install("fpuexpinc32",  s_oper3,        cat3(0x4a,0x2a,0xfb));
        install("fpuabs",       s_oper3,        cat3(0x4b,0x2a,0xfb));
        install("fpunoround",   s_oper3,        cat3(0x4d,0x2a,0xfb));
        install("fpuchki32",    s_oper3,        cat3(0x4e,0x2a,0xfb));
        install("fpuchki64",    s_oper3,        cat3(0x4f,0x2a,0xfb));

        /* four byte operations */
        install("fpudivby2",    s_oper4,        cat4(0x21,0x41,0x2a,0xfb));
        install("fpumulby2",    s_oper4,        cat4(0x21,0x42,0x2a,0xfb));
        install("fpurn",        s_oper4,        cat4(0x22,0x42,0x2a,0xfb));
        install("fpuseterr",    s_oper4,        cat4(0x22,0x43,0x2a,0xfb));
        install("fpuclrerr",    s_oper4,        cat4(0x29,0x4c,0x2a,0xfb));

        /* Assembler directives */
        install("module",       s_directive,    (WORD)s_module);
        install("global",       s_directive,    (WORD)s_global);
        install("data",         s_directive,    (WORD)s_data);
        install("init",         s_directive,    (WORD)s_init);
        install("align",        s_directive,    (WORD)s_align);
        install("word",         s_directive,    (WORD)s_word);
        install("byte",         s_directive,    (WORD)s_byte);
        install("blkb",         s_directive,    (WORD)s_blkb);
        install("blkw",         s_directive,    (WORD)s_blkw);
        install("common",       s_directive,    (WORD)s_common);
        install("size",         s_directive,    (WORD)s_size);
	install("ref",		s_directive,	(WORD)s_ref);        
}

install(name,type,value)
char *name;
int type;
WORD value;
{
        UWORD h = (hash(name) % GLOBAL_HASHSIZE);
        struct keyentry *k = (struct keyentry *)(&keytab[keyupb]);

        printf("Install: %ld %s %lu %d %lx\n",keyupb,name,h,type,value);
        k->next = 0;
        k->type = type;
        k->value = value;

#if !defined(SWAPKEYTABLE) && !defined(__DOS386)
	{
	int to = 3;
	char *p = name;
	char *q = k->name;

	printf("swapping %s\n", name);
	while(*p)
		{
		q[to--] = *p++;
		if(to < 0)
			{
			to = 3;
			q += 4;
			}
		}
	while(to >= 0)
		q[to--] = 0;
	}
#else
        strcpy((k->name),name);
#endif

        if( dicttab[h] == 0 ) dicttab[h] = keyupb;
        else ends[h]->next = keyupb;

        ends[h] = k;
        ccount[h]++;

        keyupb += sizeof(struct keyentry) + strlen(name);

        keyupb = (keyupb + 3) & (~3);
}

puttab(tab,upb)
INT *tab;
INT upb;
{
        int i;
        upb = upb / 4;
        for( i = 0 ; i < upb ; i++ )
        {
                if( (i % 8) == 0 ) fprintf(ofd,"\n");
                fprintf(ofd,"0x%08lxL,",tab[i]);
        }
        fprintf(ofd,"0x%08lxL",tab[upb]);
}

