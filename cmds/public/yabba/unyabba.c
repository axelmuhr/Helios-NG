/* Placed into the public domain by Daniel J. Bernstein. */

/* Want to make yourself popular? Figure out a fast Y decoding method.  */

#include <stdio.h>
extern long atol();
extern int getopt();
extern char *optarg;
extern int optind;
#include "bitout.h"
#include "percent.h"
#include "texts.h"
#include "huptrie.h"

static char progname[] = "unyabba";
static char progged[] = "UnY'ed";

int twom1[9] = { 0,1,3,7,15,31,63,127,255 } ;

#define ALPHABET 256

#ifndef NODEMAX
#define NODEMAX (65533)
#endif
#ifndef NODENUM
#define NODENUM NODEMAX
#endif
#ifndef MOD
#define MOD (65536)
#endif

STATICDECLARE(n,p,h,NODEMAX,MOD - 1)
node s[NODEMAX + 1];
hash geth[NODEMAX + 2]; /* aargh */
char gc[NODEMAX + 1];

#define NUMOF(no) node2pos(n,no,NODEMAX)

#define CHECKMAXBITS \
((max == nextbits) && ((bits++), \
  (nextbits = pos2ipos(n,2 * ipos2pos(n,nextbits,NODEMAX),NODEMAX))))

#define ADD(hash,oldnode,node,ch) \
( (void) ( ADDMAX(n,p,h,max,oldnode,hash,node), \
(gc[node2ipos(n,node,NODEMAX)] = ch), \
(geth[node2ipos(n,node,NODEMAX)] = hash), CHECKMAXBITS ) )

#define SUB(ip1,ip2) (ip1 - ip2) /* XXXX: This breaks encapsulation! Ungood. */

static unsigned long savein = 0;
static unsigned long saveout = 0;
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

void fatalinfo(x,ss)
int x;
char **ss;
{
 if (flagverbose) while (*ss)
  {
   fprintf(stderr,*(ss++),NODENUM);
   putc('\n',stderr);
  }
 (void) exit(x);
}

#define PUTERR { if (flagverbose) fprintf(stderr,"%s: fatal: output error\n",progname); \
savein += in; saveout += out; \
if (flagverbose >= 2) goaheadandbeverbose(); (void) exit(2); }

main(argc,argv)
int argc;
char *argv[];
{
 register pos i;
 register node matchnode;
 register hash matchhash;
 register node dosnode;
 register node oldnode;
 register ipos max;
 register ipos nextbits;
 register bitnum bits;
 register bitword curw = 0;
 register bitnum curbb = 0;
 register int ch;
 register ipos min;
 register unsigned long in = 0;
 register unsigned long out = 0;
 register node curnode;
 register int firstch;
 pos smax;

 min = pos2ipos(n,NODENUM - 1,NODEMAX);

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
		 min = pos2ipos(n,i - 1,NODEMAX);
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

 if (!flagrandom)
  {
   bitword i = 0;
   int r;

   if ((getchar() != 25)
     ||(getchar() != 1)
     ||(getchar() != 2)
     ||(getchar() != 2)
     ||(getchar() != 1)
     ||((r = getchar()) == EOF))
    {
     if (flagverbose) fprintf(stderr,"%s: fatal: input not in right format\n",progname);
     (void) exit(3);
    }
   in += 6;
   while (r)
    {
     if (((ch = getchar()) == EOF) || (ch < 48) || (ch > 57))
      {
       if (flagverbose) fprintf(stderr,"%s: fatal: input not in right format\n",progname);
       (void) exit(3);
      }
     ++in; /* XXX: check for overflow */
     i = i * 10 + (ch - 48);
     --r;
    }
   if (i != ipos2pos(n,min,NODEMAX) + 1)
    {
     if (flagverbose) fprintf(stderr,"%s: fatal: input has -m %ld, I have -m %ld\n"
	     ,progname,(long) i,(long) ipos2pos(n,min,NODEMAX) + 1);
     (void) exit(4);
    }
  }

 FIRSTHASH(h,MOD - 1)

restart:

 STATICINIT(n,p,h,max,smax,NODEMAX,MOD - 1)

 nextbits = pos2ipos(n,ALPHABET,NODEMAX);
 bits = 8;

 geth[node2ipos(n,topnode(n,NODEMAX),NODEMAX)] = tophash(h,MOD - 1);

 for (ch = 0;ch < ALPHABET;++ch)
  {
   ADD(hc(tophash(h,MOD - 1),ch,MOD - 1),topnode(n,NODEMAX),curnode,ch);
   s[node2ipos(n,curnode,NODEMAX)] = topnode(n,NODEMAX);
  }
 WASTEMAX(n,p,h,max,smax,curnode); CHECKMAXBITS;
 /* leaving space for the clear code, ALPHABET */

 matchnode = topnode(n,NODEMAX);
 matchhash = tophash(h,MOD - 1);

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
       savein += in; saveout += out;
       if (flagverbose >= 2)
	 goaheadandbeverbose();
       (void) exit(0);
      }
   if ((ch = getchar()) == EOF)
    {
     savein += in; saveout += out;
     if (flagverbose >= 2)
       goaheadandbeverbose();
     (void) exit(0);
    }
   i = curw + ((ch & twom1[bits - curbb]) << curbb);
   curw = ch >> (bits - curbb);
   curbb = 8 - (bits - curbb);
   ++in; /* XXX: check for overflow */

   /* XXX: flagpedantic to control whether we make this test? */
   if (i > ipos2pos(n,max,NODEMAX))
     if (flagrandom)
       i -= ipos2pos(n,max,NODEMAX) + 1;
     else
      {
       if (flagverbose) fprintf(stderr,"%s: fatal: input corrupt at byte %ld\n",progname,in);
       (void) exit(5);
      }

   if (i == ALPHABET) /* clear */
    {
     savein += in;
     saveout += out;
     /* XXX: test for overflow? */
     in = 0;
     out = 0;
     CLEARHASH(h,MOD - 1)
     goto restart;
    }
   curnode = pos2node(n,i,NODEMAX);

   while (curnode != topnode(n,NODEMAX))
    {
     ch = gc[node2ipos(n,curnode,NODEMAX)];
     curnode = s[node2ipos(n,curnode,NODEMAX)];
#ifdef BRAINDAMAGED
     if (((char) ch) == (char) 255)
       putchar((char) ch);
     else
#endif
     if (putchar((char) ch) == EOF)
       PUTERR
     ++out; /* XXX: check for overflow */

#define MATCHADD { if (matchnode == topnode(n,NODEMAX)) firstch = ch; \
if (dosnode) { ADD(matchhash,matchnode,oldnode,firstch); \
s[node2ipos(n,dosnode,NODEMAX)] = oldnode; dosnode = oldnode; } \
else ADD(matchhash,matchnode,dosnode,firstch); \
matchnode = s[node2ipos(n,matchnode,NODEMAX)]; \
firstch = gc[node2ipos(n,matchnode,NODEMAX)]; \
matchhash = geth[node2ipos(n,matchnode,NODEMAX)]; \
}

/* XXXX: get rid of first if (max != min) ? */
/* XXX: is DOWNI too slow? */
#define MATCHDOWN if (max != min) { dosnode = 0; \
do { matchhash = hc(matchhash,ch,MOD - 1); \
DOWNI(n,p,h,matchnode,matchhash,oldnode,{break;},MATCHADD) } while(max!=min);\
if (matchnode == topnode(n,NODEMAX)) firstch = ch; \
if (dosnode) s[node2ipos(n,dosnode,NODEMAX)] = oldnode; \
matchnode = oldnode; }
/* XXX: Should unroll the loop a bit. */
     MATCHDOWN

    }

#ifdef notdef
/*XXX: -d code */
#endif

  }
}
