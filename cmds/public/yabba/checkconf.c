/* Placed into the public domain by Daniel J. Bernstein. */

#include <stdio.h>

extern long atol();

static char introspiel[] = "\
checkconf is a tool to help you configure yabba and whap for your system.\n\
It lets you quickly experiment with options to see what effect they\n\
will have on program size and to make sure that they are appropriate\n\
for your system.\n\
\n\
You can give checkconf any of the following options: -DPTRS, -UPTRS,\n\
-DBZERO, -UBZERO, -DMEMZERO, -UMEMZERO, -DZEROFILLED, -UZEROFILLED,\n\
-DTYPE=short, -DTYPE=int, -DHASHTYPE=short, -DHASHTYPE=int,\n\
-DHASHTYPE=TYPE, -DBITBUFSIZE=%%d, -DRESETNUM=%%d, -DRESETFUZZ=%%d,\n\
-DNODEMAX=%%d, -DNODENUM=NODEMAX, -DNODENUM=%%d, -DMOD=%%d. Here %%d means\n\
any number. These options override CCOPTS in the Makefile. The Makefile\n\
contains more detailed information on each option.\n\
\n\
checkconf -H shows this help screen.\n\
\n\
Be sure to remake checkconf if you change the Makefile.\n\
";

#ifndef BITBUFSIZE
#define BITBUFSIZE 1000
#endif
#ifndef RESETNUM
#define RESETNUM 8192
#endif
#ifndef RESETFUZZ
#define RESETFUZZ 30
#endif
#ifndef NODEMAX
#define NODEMAX 65533
#endif
#ifndef TYPE
#define TYPE short
#endif
#ifndef MOD
#define MOD 65536
#endif

unsigned long bitbufsize = BITBUFSIZE;
unsigned long resetnum = RESETNUM;
long resetfuzz = RESETFUZZ;
unsigned long nodemax = NODEMAX;
#ifndef NODENUM
#define NODENUM NODEMAX
unsigned long nodenum = 0;
#else
unsigned long nodenum = NODENUM;
#endif
unsigned long mod = MOD;
#ifdef PTRS
char ptrs = 'D';
#else
char ptrs = 'U';
#endif
#ifdef BZERO
char cbzero = 'D';
#else
char cbzero = 'U';
#endif
#ifdef MEMZERO
char cmemzero = 'D';
#else
char cmemzero = 'U';
#endif
#ifdef ZEROFILLED
char zerofilled = 'D';
#else
char zerofilled = 'U';
#endif

static int h[100];

main(argc,argv)
int argc;
char *argv[];
{
 int sshort = sizeof(short);
 int sint = sizeof(int);
 int slong = sizeof(long);
 int sptr = sizeof(char *);
 int st = sizeof(unsigned TYPE);
#ifndef HASHTYPE
#define HASHTYPE TYPE
 int shasht = 0;
#else
 int shasht = sizeof(HASHTYPE);
#endif
 char *foo = 0;
 int i;
 int flag0;
 char c;
 int flagfill;

 while (*++argv)
  {
   if (!strcmp(*argv,"-DPTRS")) ptrs = 'D';
   else if (!strcmp(*argv,"-UPTRS")) ptrs = 'U';
   else if (!strcmp(*argv,"-DBZERO")) cbzero = 'D';
   else if (!strcmp(*argv,"-UBZERO")) cbzero = 'U';
   else if (!strcmp(*argv,"-DMEMZERO")) cmemzero = 'D';
   else if (!strcmp(*argv,"-UMEMZERO")) cmemzero = 'U';
   else if (!strcmp(*argv,"-DZEROFILLED")) zerofilled = 'D';
   else if (!strcmp(*argv,"-UZEROFILLED")) zerofilled = 'U';
   else if (!strcmp(*argv,"-DTYPE=short")) st = sshort;
   else if (!strcmp(*argv,"-DTYPE=int")) st = sint;
   else if (!strcmp(*argv,"-DHASHTYPE=short")) shasht = sshort;
   else if (!strcmp(*argv,"-DHASHTYPE=int")) shasht = sint;
   else if (!strcmp(*argv,"-DHASHTYPE=TYPE")) shasht = 0;
   else if (!strncmp(*argv,"-DBITBUFSIZE=",13)) bitbufsize = atol(*argv + 13);
   else if (!strncmp(*argv,"-DRESETNUM=",11)) resetnum = atol(*argv + 11);
   else if (!strncmp(*argv,"-DRESETFUZZ=",12)) resetfuzz = atol(*argv + 12);
   else if (!strncmp(*argv,"-DNODEMAX=",10)) nodemax = atol(*argv + 10);
   else if (!strcmp(*argv,"-DNODENUM=NODEMAX")) nodenum = 0;
   else if (!strncmp(*argv,"-DNODENUM=",10)) nodenum = atol(*argv + 10);
   else if (!strncmp(*argv,"-DMOD=",6)) mod = atol(*argv + 6);
   else if (!strcmp(*argv,"-H"))
    {
     fprintf(stderr,introspiel);
     exit(1);
    }
   else
    {
     fprintf(stderr,
	     "checkconf: fatal: argument %s unrecognized. checkconf -H for help.\n"
	     ,*argv);
     exit(1);
    }
  }

 if (shasht == 0)
   shasht = st;
 if (nodenum == 0)
   nodenum = nodemax;

 flagfill = 1;
 for (i = 0;i < sizeof(h) / sizeof(h[0]);i++)
   if (h[i])
     flagfill = 0;

 printf("\n");
 printf("Sizes:  short %d  int %d  long %d  pointer %d\n",sshort,sint,slong,sptr);
 printf("Internal representation of the NULL pointer:  ");
 flag0 = 1;
 for (i = 0;i < sptr;i++)
  {
   c = ((char *) (&foo))[i];
   printf("%d ",c);
   if (c)
     flag0 = 0;
  }
 if (flag0)
   printf("(all zeros)\n");
 else
   printf("(not all zeros)\n");
 printf("\n");

 printf("major config: -%cPTRS  -DBITBUFSIZE=%ld  -DNODEMAX=%ld  -DMOD=%ld\n"
	,ptrs,bitbufsize,nodemax,mod
	);
 printf("type config: TYPE size %ld  HASHTYPE size %ld\n"
	,st
	,shasht
	);
 printf("minor config: -DRESETNUM=%ld  -DRESETFUZZ=%ld  -DNODENUM=%ld\n"
	,resetnum,resetfuzz,nodenum
	);
 printf("system config: -%cBZERO  %s-%cMEMZERO%s  -%cZEROFILLED\n"
	,cbzero
	,(cbzero == 'U' ? "" : "(")
	,cmemzero
	,(cbzero == 'U' ? "" : ", not relevant with -DBZERO)")
	,zerofilled
	);

/* printf("\n"); */
 printf("Array use:\n");
 if (ptrs == 'D')
   printf("  whap:   total %ld  huptrie data %ld hash %ld  bitbuf %ld\n"
	  ,2 * sptr * (nodemax + 1) + mod * sptr + (bitbufsize + 1) * 2 * st
	  ,2 * sptr * (nodemax + 1)
	  ,mod * sptr
	  ,(bitbufsize + 1) * 2 * st
	  );
 else
   printf("  whap:   total %ld  huptrie data %ld %ld hash %ld  bitbuf %ld\n"
	  ,2 * st * (nodemax + 1) + mod * st + (bitbufsize + 1) * 2 * st
	  ,st * (nodemax + 1)
	  ,st * (nodemax + 1)
	  ,mod * st
	  ,(bitbufsize + 1) * 2 * st
	  );
 printf("  unwhap: total %ld  outarray %ld  outpos %ld\n"
	,nodemax + st * nodemax
	,nodemax
	,st * nodemax
	);

 if (ptrs == 'D')
   printf("  yabba:    tot %ld  ht d %ld %ld %ld h %ld  bb %ld\n"
	  ,2 * sptr * (nodemax + 1) + sptr * (nodemax + 1) + shasht * (nodemax + 2) + mod * sptr + (bitbufsize + 1) * 2 * st
	  ,2 * sptr * (nodemax + 1)
	  ,sptr * (nodemax + 1)
	  ,shasht * (nodemax + 2)
	  ,mod * sptr
	  ,(bitbufsize + 1) * 2 * st
	  );
 else
   printf("  yabba:    tot %ld  ht d %ld %ld %ld %ld h %ld  bb %ld\n"
	  ,st * (nodemax + 1) + st * (nodemax + 1) + st * (nodemax + 1) + shasht * (nodemax + 2) + mod * st + (bitbufsize + 1) * 2 * st
	  ,st * (nodemax + 1)
	  ,st * (nodemax + 1)
	  ,st * (nodemax + 1)
	  ,shasht * (nodemax + 2)
	  ,mod * st
	  ,(bitbufsize + 1) * 2 * st
	  );

 if (ptrs == 'D')
   printf("  unyabba:  tot %ld  ht d %ld %ld %ld %ld h %ld\n"
	  ,2 * sptr * (nodemax + 1) + sptr * (nodemax + 1) + shasht * (nodemax + 2) + (nodemax + 1) + mod * sptr
	  ,2 * sptr * (nodemax + 1)
	  ,sptr * (nodemax + 1)
	  ,shasht * (nodemax + 2)
	  ,(nodemax + 1)
	  ,mod * sptr
	  );
 else
   printf("  unyabba:  tot %ld  ht d %ld %ld %ld %ld %ld h %ld\n"
	  ,st * (nodemax + 1) + st * (nodemax + 1) + st * (nodemax + 1) + shasht * (nodemax + 2) + (nodemax + 1) + mod * st
	  ,st * (nodemax + 1)
	  ,st * (nodemax + 1)
	  ,st * (nodemax + 1)
	  ,shasht * (nodemax + 2)
	  ,(nodemax + 1)
	  ,mod * st
	  );

#define MAX(foo) ((unsigned long) ((unsigned foo) (-1))) /*XXX*/

 printf("\n");
 printf("Validity checks:\n");
 if (mod & (mod - 1))
 printf("MOD is a power of 2: FAILED!\n");
 if ((nodenum < 512) || (nodenum > nodemax))
 printf("NODENUM (default c-size) is between 512 and NODEMAX: FAILED!\n");
 if ( ( (st == sshort) ? MAX(short)
     : ( (st == sint) ? MAX(int) : MAX(TYPE) ) ) < nodemax + 2 )
 printf("NODEMAX (maximum c-size) + 2 fits into TYPE: FAILED!\n");
 if ( ( (shasht == sshort) ? MAX(short)
     : ( (shasht == sint) ? MAX(int) : MAX(HASHTYPE) ) ) < mod - 1 )
 printf("MOD - 1 fits into HASHTYPE: FAILED!\n");
 if (MAX(int) < bitbufsize + 3)
 printf("BITBUFSIZE + 3 fits into int: FAILED!\n");
 if (ptrs == 'U')
 if (!flag0)
 if ((cbzero == 'D') || (cmemzero == 'D') || (zerofilled == 'D'))
 printf("Under -DPTRS, if NULL isn't 0, must -UBZERO -UMEMZERO -UZEROFILLED: FAILED!\n");

 printf("\n");
 printf("Range sanity checks:\n");
 if (mod < 512) printf("MOD should be at least 512\n");
 if (bitbufsize < 128) printf("BITBUFSIZE should be at least 128\n");
 if (nodemax < 1024) printf("NODEMAX should be at least 1024\n");
 if (resetnum < 3 * bitbufsize)
   printf("RESETNUM should probably be at least BITBUFSIZE * 3\n");
 if (((long)resetfuzz < -(long)(bitbufsize / 10))
   ||(resetfuzz > bitbufsize / 10))
   printf("RESETFUZZ should probably be at most BITBUFSIZE/10 in absolute value\n");
 if (st > sint)
   printf("TYPE should probably fit into an int\n");
 if (shasht > sint)
   printf("HASHTYPE should probably fit into an int\n");

 printf("\n");
 printf("System requirements:\n");
 if (cbzero == 'D')
   printf("bzero() must be available\n");
 else if (cmemzero == 'D')
   printf("memset() must be available\n");
 if (zerofilled == 'D')
  {
   printf("static arrays must be filled with null bytes upon entry to main()\n");
   if (!flagfill)
     printf("  wait! that doesn't seem to be true here...\n");
  }

 printf("\n");
 exit(0);
}
