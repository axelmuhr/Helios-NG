/* Placed into the public domain by Daniel J. Bernstein. */

static char whapwarning[] = "\
WARNING! If you use AP coding except for instruction and amusement,\n\
you may be infringing upon a patent.\n\
";

#include <stdio.h>
extern long atol();
extern int getopt();
extern char *optarg;
extern int optind;
#include "percent.h"
#include "texts.h"

static char progname[] = "unwhap";
static char progged[] = "Unwhapped";

#ifndef TYPE
#define TYPE short
#endif

typedef TYPE bitnum;
typedef unsigned TYPE bitword;

int twom1[9] = { 0,1,3,7,15,31,63,127,255 } ;

#ifndef NODEMAX
#define NODEMAX (65533)
#endif
#ifndef NODENUM
#define NODENUM NODEMAX
#endif
static char outarray[NODEMAX]; /* first 256 spots unused, boo hoo hoo */
static bitword outstart[NODEMAX]; /* first 256 spots unused, boo hoo hoo */

#define MAXPLUS max++; oamax++; osmax++; \
if (max == nextbits) { bits++; nextbits <<= 1; }

static unsigned long savein;
static unsigned long saveout;
static int flagverbose = 1;
static int flagrandom = 0;

void goaheadandbeverbose()
{
 long per = percent(savein,saveout,10000L);

 if (per == 10000L) /* absolutely ridiculous */
   fprintf(stderr,"In: %ld chars  Out: %ld chars  %s from: >9999%%\n",
           savein,saveout,progged);
 else
   fprintf(stderr,"In: %ld chars  Out: %ld chars  %s from: %ld%%\n",
	   savein,saveout,progged,per);
}

void fatalinfo(x,s)
int x;
char **s;
{
 if (flagverbose) while (*s)
  {
   fprintf(stderr,*(s++),NODENUM);
   putc('\n',stderr);
  }
 (void) exit(x);
}

#define PUTERR { if (flagverbose) fprintf(stderr,"%s: fatal: output error\n",progname); \
savein = in; saveout = out; \
if (flagverbose >= 2) goaheadandbeverbose(); (void) exit(2); }

main(argc,argv)
int argc;
char *argv[];
{
 register bitword i;
 register char *j;
 register bitword outb;
 register bitword max;
 register bitword nextbits;
 register bitnum bits;
 register bitword curw = 0;
 register bitnum curbb = 0;
 register int ch;
 register char *oamax;
 register bitword *osmax;
 register char *oai;
 register bitword min = NODENUM - 1; /* maximum decompressor size - 1 */
 register unsigned long in = 0;
 register unsigned long out = 0;

  {
   int opt;
   bitword i;

   while ((opt = getopt(argc,argv,"m:vqQrRACHUVW")) != EOF)
     switch(opt)
      {
       case '?': fatalinfo(1,unsqusage);
       case 'm': i = atol(optarg);
		 if ((i < 512) || (i > NODEMAX))
		  {
		   if (flagverbose) fprintf(stderr,
		      "%s: fatal: mem size out of range: must be between 512 and %ld\n",
		      progname,(long) NODEMAX);
		   (void) exit(1);
		  }
		 min = i - 1;
		 break;
       case 'v': flagverbose = 2; break;
       case 'q': flagverbose = 0; break;
       case 'Q': flagverbose = 1; break;
       case 'r': flagrandom = 1; break;
       case 'R': flagrandom = 0; break;
       case 'A': fatalinfo(1,unsqauthor); break;
       case 'C': fatalinfo(1,unsqcopyright); break;
       case 'H': fatalinfo(1,unsqhelp); break;
       case 'U': fatalinfo(1,unsqusage); break;
       case 'V': fatalinfo(1,unsqversion); break;
       case 'W': fatalinfo(1,unsqwarranty); break;
      }
   argv += optind;
   argc -= optind;
  }

 if (flagverbose)
   fprintf(stderr,whapwarning);

 if (!flagrandom)
  {
   bitword i = 0;
   int r;

   if ((getchar() != 23)
     ||(getchar() != 8)
     ||(getchar() != 1)
     ||(getchar() != 16)
     ||((r = getchar()) == EOF))
    {
     if (flagverbose) fprintf(stderr,"%s: fatal: input not in right format\n",progname);
     (void) exit(3);
    }
   in += 5;
   while (r)
    {
     if (((ch = getchar()) == EOF) || (ch < 48) || (ch > 57))
      {
       if (flagverbose) fprintf(stderr,"%s: fatal: input not in right format\n",progname);
       (void) exit(3);
      }
     ++in;
     i = i * 10 + (ch - 48);
     --r;
    }
   if (i != min + 1)
    {
     if (flagverbose) fprintf(stderr,"%s: fatal: input has -m %ld, I have -m %ld\n"
	     ,progname,(long) i,(long) (min + 1));
     (void) exit(4);
    }
  }

#define MINIT max = 256; oamax = outarray + 256; osmax = outstart + 256; \
outb = 256; nextbits = 256; bits = 8; \
while (max >= nextbits) { bits++; nextbits <<= 1; }
/* of course, max = 255 plus one for table clearing */

 MINIT

 for (;;)
  {
   /* assumes bits >= 8 */
   while (curbb + 8 < bits) /* could be an if, when bits is < 16 */
     if ((ch = getchar()) != EOF)
      {
       curw += ch << curbb;
       curbb += 8;
       ++in; /* XXX: check for overflow */
      }
     else
      {
       savein = in; saveout = out;
       if (flagverbose >= 2)
	 goaheadandbeverbose();
       (void) exit(0);
      }
   if ((ch = getchar()) == EOF)
    {
     savein = in; saveout = out;
     if (flagverbose >= 2)
       goaheadandbeverbose();
     (void) exit(0);
    }
   i = curw + ((ch & twom1[bits - curbb]) << curbb);
   curw = ch >> (bits - curbb);
   curbb = 8 - (bits - curbb);
   ++in; /* XXX: check for overflow */

   /* XXX: flagpedantic to control whether we make this test? */
   if (i > max)
     if (flagrandom)
       i -= max + 1;
     else
      {
       if (flagverbose) fprintf(stderr,"%s: fatal: input corrupt at byte %ld\n",progname,in);
       (void) exit(5);
      }

   if (i < 257) /* 256 is special */
     if (i != 256)
      {
       if (max != min)
	{
         MAXPLUS
	 *osmax = outb;
	 *oamax = i;
         outb = max;
	}
#ifdef BRAINDAMAGED
       if (i == 255)
	 putchar((char) i);
       else
#endif
       if (putchar((char) i) == EOF)
         PUTERR
       ++out; /* XXX: check for overflow */
      }
     else
      {
       /* here's where blocking stuff should go */
       MINIT
       /* wow, that was easy */
      }
   else
    {
     oai = outarray + i;
     j = outarray + outstart[i] - 1;
     while ((j != oai) && (max != min))
      {
       MAXPLUS
       *oamax = *++j;
#ifdef BRAINDAMAGED
       if (*oamax == (char) 255)
	 putchar(*oamax);
       else
#endif
       if (putchar(*oamax) == EOF)
	 PUTERR
       ++out; /* XXX: check for overflow */
       *osmax = outb;
      }
     while (j != oai)
      {
       ++j;
#ifdef BRAINDAMAGED
       if (*j == (char) 255)
	 putchar(*j);
       else
#endif
       if (putchar(*j) == EOF)
	 PUTERR
       ++out; /* XXX: check for overflow */
      }
     outb = max - (i - outstart[i]);
    }
#ifdef notdef
/*XXX: -d code */
#endif
  }
}
