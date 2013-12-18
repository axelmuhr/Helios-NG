/* btoa: version 4.0
 * stream filter to change 8 bit bytes into printable ascii
 * computes the number of bytes, and three kinds of simple checksums
 * incoming bytes are collected into 32-bit words, then printed in base 85
 *  exp(85,5) > exp(2,32)
 * the ASCII characters used are between '!' and 'u'
 * 'z' encodes 32-bit zero; 'x' is used to mark the end of encoded data.
 *
 *  Paul Rutter		Joe Orost
 *  philabs!per		petsd!joe
 *
 *  WARNING: this version is not compatible with the original as sent out
 *  on the net.  The original encoded from ' ' to 't'; which cause problems
 *  with some mailers (stripping off trailing blanks).
 */

/*
 * Hacked by PAB 3/9/88 for Helios and ANSI
 */

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/btoa.c,v 1.4 1993/07/12 10:50:45 nickc Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <nonansi.h>

#define MAXPERLINE 78

long int Ceor = 0;
long int Csum = 0;
long int Crot = 0;

long int ccount = 0;
long int bcount = 0;
long int cword;

#define EN(c)	(int) ((c) + '!')

void encode(int c);
void wordout(long cword);
void charout(int c);

void encode(int c)
{
  Ceor ^= c;
  Csum += c;
  Csum += 1;
  if ((Crot & 0x80000000)) {
    Crot <<= 1;
    Crot += 1;
  } else {
    Crot <<= 1;
  }
  Crot += c;

  cword <<= 8;
  cword |= c;
  if (bcount == 3) {
    wordout(cword);
    bcount = 0;
  } else {
    bcount += 1;
  }
}

void wordout(long cword)
{
  if (cword == 0) {
    charout('z');
  } else {
    int tmp = 0;
    
    if(cword < 0) {	/* Because some don't support unsigned long */
      tmp = 32;
      cword = cword - (long)(85 * 85 * 85 * 85 * 32);
    }
    if(cword < 0) {
      tmp = 64;
      cword = cword - (long)(85 * 85 * 85 * 85 * 32);
    }
    charout(EN((cword / (long)(85 * 85 * 85 * 85)) + tmp));
    cword %= (long)(85 * 85 * 85 * 85);
    charout(EN(cword / (85 * 85 * 85)));
    cword %= (85 * 85 * 85);
    charout(EN(cword / (85 * 85)));
    cword %= (85 * 85);
    charout(EN(cword / 85));
    cword %= 85;
    charout(EN(cword));
  }
}

void charout(int c)
{
  putchar(c);
  ccount += 1;
  if (ccount == MAXPERLINE) {
    putchar('\n');
    ccount = 0;
  }
}

int main(
  int argc,
  char **argv )
{
  int c;
  long n;

  if (argc != 1) {
    fprintf(stderr,"btoa: bad args to %s\n", argv[0]);
    exit(2);
  }

  if(freopen(Heliosno(stdin)->Name, "rb", stdin) == NULL)
	{
	fprintf(stderr, "Failed to reopen stdin\n");
	exit(1);
	}

  printf("xbtoa Begin\n");
  n = 0;
  while ((c = getchar()) != EOF) {
    encode(c);
    n += 1;
  }
  while (bcount != 0) {
    encode(0);
  }
  /* n is written twice as crude cross check*/
  printf("\nxbtoa End N %ld %lx E %lx S %lx R %lx\n", n, n, Ceor, Csum, Crot);
  exit(0);
}
