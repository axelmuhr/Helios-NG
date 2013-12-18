/* Placed into the public domain by Daniel J. Bernstein. */

/* This is the source for BOTH coding methods! Y and AP are both here. */
/* Keep this in mind when hacking the source. */

#ifdef WHAP
static char whapwarning[] = "\
WARNING! If you use AP coding except for instruction and amusement,\n\
you may be infringing upon a patent.\n\
";
#endif

#include <stdio.h>
extern long atol();
extern int getopt();
extern char *optarg;
extern int optind;
#include "bitout.h"
#include "percent.h"
#include "texts.h"
#include "huptrie.h"

#ifdef WHAP
static char progname[] = "whap";
static char progged[] = "Whapped";
#else
static char progname[] = "yabba";
static char progged[] = "Y'ed";
#endif

#define ALPHABET 256 /* do not change this! */

#ifndef RESETNUM
#define RESETNUM 8192
#endif
#ifndef RESETFUZZ
#define RESETFUZZ 30 /* XXX: any suggestions? */
#endif
static long resetnum = RESETNUM;
static long resetfuzz = RESETFUZZ; /* when in doubt, pass the buck */

#ifndef NODEMAX
#define NODEMAX (65533) /* up to 65533---could be 65534 for whap */
#endif
#ifndef NODENUM
#define NODENUM NODEMAX
#endif

#ifndef MOD
#define MOD (65536)
#endif

STATICDECLARE(n,p,h,NODEMAX,MOD - 1)
#ifndef WHAP
node s[NODEMAX + 1];
hash geth[NODEMAX + 2]; /* XXX: only because of this is 65533 the default */
#endif

#define NUMOF(no) node2pos(n,no,NODEMAX)

#define CHECKMAXBITS \
((max == nextbits) && ((bits++), \
  (nextbits = pos2ipos(n,2 * ipos2pos(n,nextbits,NODEMAX),NODEMAX))))

#ifndef WHAP
#define ADD(hash,oldnode,node) \
( (void) ( ADDMAX(n,p,h,max,oldnode,hash,node), \
  (geth[node2ipos(n,node,NODEMAX)] = hash), CHECKMAXBITS ) )
#else
#define ADD(hash,oldnode,node) \
( (void) ( ADDMAX(n,p,h,max,oldnode,hash,node), CHECKMAXBITS ) )
#endif

#define SUB(ip1,ip2) (ip1 - ip2) /* XXXX: This breaks encapsulation! Ungood. */

static unsigned long savein = 0;
static unsigned long saveout = 0;
static int flagverbose = 1;

void goaheadandbeverbose()
{
 long per;
      
 per = percent(saveout,savein,10000L);
 if (per == 10000L) /* absolutely ridiculous */
   if (flagverbose == 2)
     fprintf(stderr,"In: %ld chars  Out: %ld chars  %s to: >9999%%\n",
     savein,saveout,progged);
   else
     fprintf(stderr,"In: %ld chars  Out: %ld chars  %s by: <-9899%%\n",
     savein,saveout,progged);
 else
   if (flagverbose == 2)
     fprintf(stderr,"In: %ld chars  Out: %ld chars  %s to: %ld%%\n",
     savein,saveout,progged,per);
   else
     fprintf(stderr,"In: %ld chars  Out: %ld chars  %s by: %ld%%\n",
     savein,saveout,progged,100 - per);
}

int y[55];
int j;
int k;

#define RANDOMBIT ( ( !j ? (j = 54) : --j ), \
( !k ? (k = 54) : --k ), ( y[k] ^= y[j] ) )

void initrandom()
{
 int i;

#ifdef RANDINIT
 srand(RANDINIT);
#else
 srand(1);
#endif
 for (i = 0;i < 54;i++)
   y[i] = ((rand()) + (rand() >> 13) + (rand() % 71)) % 2;
 y[54] = 1;
 j = 24;
 k = 0;
 for (i = 0;i < 100;i++)
   (void) RANDOMBIT;
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

main(argc,argv)
int argc;
char *argv[];
{
 register node oldnode;
 register hash oldhash;
 register node curnode;
 register hash curhash;
#ifndef WHAP
 register node matchnode;
 register hash matchhash;
 register node dosnode;
 register node safenode;
#else
 register node lastnode;
 register hash lasthash;
 register node midnode;
 register hash midhash;
#endif
 register int ch;
 register bitnum curbits;
#ifdef WHAP
 register node firstmidnode;
 register node newnode;
#endif
 register ipos max;
 register ipos nextbits;
 register bitnum bits;
 register ipos min;
 register long numin = 0;
 register long nextreset;
 register int flagrandom = 0;
 ipos curmax;
 ipos curnextbits;
 long eff;
 pos smax;

#define PUTERR { \
if (flagverbose) fprintf(stderr,"%s: fatal: output error\n",progname); \
savein += numin; saveout += bit_numout; /*XXX*/ \
if (flagverbose >= 2) goaheadandbeverbose(); (void) exit(2); }


 min = pos2ipos(n,NODENUM - 1,NODEMAX);

  {
   int opt;
   bitword i;

   while ((opt = getopt(argc,argv,"m:v^qQrz:Z:RACHUVW")) != EOF)
     switch(opt)
      {
       case '?': fatalinfo(1,squsage);
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
       case '^': flagverbose = 3; break;
       case 'q': flagverbose = 0; break;
       case 'Q': flagverbose = 1; break;
       case 'r': flagrandom = 1; break;
       case 'R': flagrandom = 0; break;
       case 'z': resetnum = atol(optarg);
		 if (resetnum < 512) resetnum = 512;
		 break;
       case 'Z': resetfuzz = atol(optarg); break;
       case 'A': fatalinfo(1,sqauthor);
       case 'C': fatalinfo(1,sqcopyright);
       case 'H': fatalinfo(1,sqhelp);
       case 'U': fatalinfo(1,squsage);
       case 'V': fatalinfo(1,sqversion);
       case 'W': fatalinfo(1,sqwarranty);
      }
   argv += optind;
   argc -= optind;
  }

#ifdef WHAP
 if (flagverbose)
   fprintf(stderr,whapwarning);
#endif

 if (flagrandom)
   initrandom();
 else
  {
   pos i = ipos2pos(n,min,NODEMAX) + 1;
   bitword q = 1;
   char r = 1;

   while (q <= i / 10)
    {
     q *= 10;
     ++r; /* this could overflow! :-) */
    }
#ifndef WHAP
   putchar(25); putchar(1); putchar(2); putchar(2); putchar(1);
#else
   putchar(23); putchar(8); putchar(1); putchar(16);
#endif
   putchar(r);
#ifndef WHAP
   saveout += 6;
#else
   saveout += 5;
#endif
   while (q)
    {
     putchar(48 + (i / q));
     ++saveout;
     i -= (i / q) * q;
     q /= 10;
    }
  }

 FIRSTHASH(h,MOD - 1)


restart:

 STATICINIT(n,p,h,max,smax,NODEMAX,MOD - 1)

 nextbits = pos2ipos(n,ALPHABET,NODEMAX);
 bits = 8;
 nextreset = 0;
 eff = 0;

#ifndef WHAP
 geth[node2ipos(n,topnode(n,NODEMAX),NODEMAX)] = tophash(h,MOD - 1);
#endif

 for (ch = 0;ch < ALPHABET;++ch)
  {
   curhash = tophash(h,MOD - 1);
   curhash = hc(curhash,ch,MOD - 1);
   ADD(curhash,topnode(n,NODEMAX),curnode);
#ifndef WHAP
   s[node2ipos(n,curnode,NODEMAX)] = topnode(n,NODEMAX);
#endif
  }
 WASTEMAX(n,p,h,max,smax,curnode); CHECKMAXBITS;
 /* leaving space for the clear code, ALPHABET */

#ifndef WHAP
 safenode = ipos2node(n,max,NODEMAX);
#endif

 oldnode = topnode(n,NODEMAX);
 oldhash = tophash(h,MOD - 1);

#ifndef WHAP
 matchnode = topnode(n,NODEMAX);
 matchhash = tophash(h,MOD - 1);
#else
 lastnode = topnode(n,NODEMAX);
 lasthash = tophash(h,MOD - 1);
#endif

#ifdef WHAP
 midhash = lasthash;
 firstmidnode = 0; /* still in tree */
#endif

 curbits = bits;
 curmax = max;
 curnextbits = nextbits;

 for (;;)
  {
   ch = getchar();
   if (ch == EOF)
    {
     if (flagrandom)
      {
       int b;

       if (oldnode != topnode(n,NODEMAX))
	 if (bits_out((NUMOF(oldnode) + 1 < SUB(curmax,curnextbits)) &&
	              RANDOMBIT ?
	              (NUMOF(oldnode) + ipos2pos(n,curmax,NODEMAX) + 1) :
		      (NUMOF(oldnode))
		      ,bits) == EOF)
	   PUTERR
       b = RANDOMBIT; b = b + b;
       b += RANDOMBIT; b = b + b; b += RANDOMBIT; b = b + b;
       b += RANDOMBIT; b = b + b; b += RANDOMBIT; b = b + b;
       b += RANDOMBIT; b = b + b; b += RANDOMBIT; b = b + b;
       b += RANDOMBIT;
       if (bit_fillflush(b) == EOF)
	 PUTERR
      }
     else
      {
       if (oldnode != topnode(n,NODEMAX))
         if (bits_out(NUMOF(oldnode),bits) == EOF)
	   PUTERR
       if (bit_flushbuf() == EOF)
	 PUTERR
      }
     savein += numin;
     saveout += bit_numout;
     /* XXX: test for overflow? */
     if (flagverbose >= 2)
       goaheadandbeverbose();
     (void) exit(0);
    }
   numin++;
   for (;;)
    {
     /* We use some tricks to avoid any need for backtracking. */

#ifndef WHAP
#define MATCHADD { if (dosnode) { ADD(matchhash,matchnode,oldnode); \
s[node2ipos(n,dosnode,NODEMAX)] = oldnode; dosnode = oldnode; } \
else ADD(matchhash,matchnode,dosnode); \
matchnode = s[node2ipos(n,matchnode,NODEMAX)]; \
matchhash = geth[node2ipos(n,matchnode,NODEMAX)]; \
}

/* XXXX: get rid of first if (max != min) ? */
/* XXXX: is the inner DOWNI too slow? */
#define MATCHDOWN if (max != min) { dosnode = 0; \
do { matchhash = hc(matchhash,ch,MOD - 1); \
DOWNI(n,p,h,matchnode,matchhash,oldnode,{break;},MATCHADD) } while(max != min); \
if (dosnode) s[node2ipos(n,dosnode,NODEMAX)] = oldnode; \
matchnode = oldnode; }
/* XXX: Should unroll the loop a bit. */

#define SAFEMATCHDOWN { if (curnode > safenode) /*XXX: AARGH!*/ \
{ MATCHDOWN break; } }
#endif

#ifdef WHAP
#define MIDDOWNF lastnode = firstmidnode; \
WASTEMAX(n,p,h,max,smax,firstmidnode); CHECKMAXBITS; firstmidnode = 0;

#define MIDADDF ADD(midhash,0,midnode); firstmidnode = midnode;

#define MIDADDM newnode = midnode; ADD(midhash,newnode,midnode);

#define MIDDOWN if (max != min) { midhash = hc(midhash,ch,MOD - 1); \
if (!firstmidnode) { \
DOWN(n,p,h,lastnode,midhash,firstmidnode,MIDDOWNF,MIDADDF) \
} else { MIDADDM } }

#define SAFEMATCHDOWN { MIDDOWN break; }
#endif

     curhash = hc(oldhash,ch,MOD - 1);
     DOWN(n,p,h,oldnode,curhash,curnode,SAFEMATCHDOWN,{;})

/* Sheer hell for your optimizer. [grin] */

     if (flagrandom)
      {
       if (bits_out((NUMOF(oldnode) + 1 < SUB(curmax,curnextbits)) &&
		    RANDOMBIT ?
                    (NUMOF(oldnode) + ipos2pos(n,curmax,NODEMAX) + 1):
		    (NUMOF(oldnode))
	            ,curbits) == EOF)
	 PUTERR
       curmax = max;
       curnextbits = nextbits;
      }
     else if (bits_out(NUMOF(oldnode),curbits) == EOF)
       PUTERR

     curbits = bits;

#ifdef WHAP
     if (firstmidnode)
       setparent(p,firstmidnode,lastnode);
     /* hence adding entire tree from neutral down onto lastnode */
#endif

     if (max == min)
      {
       if (numin >= nextreset)
	{
	 nextreset = numin + resetnum;
	 /* XXX: this isn't accurate, we don't know how many bits... */
	 if ((eff * ((bit_numout + 2 * bit_bufsize) / 16)) < numin + resetfuzz)
	   eff = numin / ((bit_numout + 2 * bit_bufsize) / 16);
	 else
	  {
	   savein += numin;
	   saveout += bit_numout;
	   /* XXX: test for overflow? */
	   numin = 0;
	   bit_numout = 0;

	   if (flagrandom)
	    { /*XXX*/
	     if (bits_out((ALPHABET + 1 < SUB(max,nextbits)) && RANDOMBIT ?
			  (ALPHABET + ipos2pos(n,max,NODEMAX) + 1) :
			  ALPHABET,bits) == EOF)
	       PUTERR
	    }
	   else
	     if (bits_out(ALPHABET,bits) == EOF)
	       PUTERR

           CLEARHASH(h,MOD - 1)

	   (void) ungetc(ch,stdin);
	   --savein;
	   /* XXX: Shouldn't have this goto at all. */
	   goto restart;
	  }
	}
      }
     else
      {
#ifndef WHAP
       safenode = ipos2node(n,max,NODEMAX);
       /* if max is min, s's past the old safenode might be out of dict */
       /* so changing safenode here wouldn't be safe */
#else
       lastnode = oldnode;
       lasthash = oldhash;

       midhash = lasthash;

       firstmidnode = 0;
#endif
      }
     oldnode = topnode(n,NODEMAX);
     oldhash = tophash(h,MOD - 1);
    }
   oldnode = curnode;
   oldhash = curhash;
  }
}
