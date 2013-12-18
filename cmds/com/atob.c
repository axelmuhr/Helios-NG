/* atob: version 4.0
 * stream filter to change printable ascii from "btoa" back into 8 bit bytes
 * if bad chars, or Csums do not match: exit(1) [and NO output]
 *
 *  Paul Rutter		Joe Orost
 *  philabs!per		petsd!joe
 */

/*
 * Hacked by PAB 3/9/88 for Helios and ANSI
 */

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/atob.c,v 1.5 1993/07/12 10:44:40 nickc Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nonansi.h>

#define streq(s0, s1)  strcmp(s0, s1) == 0

#define times85(x)	((((((x<<2)+x)<<2)+x)<<2)+x)

long int Ceor = 0;
long int Csum = 0;
long int Crot = 0;
long int cword = 0;
long int bcount = 0;

void fatal(void);
void decode(int c);
void byteout(int c);

void fatal(void) {
  fprintf(stderr, "atob: Bad format or Csum to atob\n");
  exit(1);
}

#define DE(c) ((c) - (long)'!')

void decode(int c)
{
  if (c == 'z') {
    if (bcount != 0) {
      fatal();
    } else {
      byteout(0);
      byteout(0);
      byteout(0);
      byteout(0);
    }
  } else if ((c >= '!') && (c < ('!' + 85))) {
    if (bcount == 0) {
      cword = DE(c);
      ++bcount;
    } else if (bcount < 4) {
      cword = times85(cword);
      cword += DE(c);
      ++bcount;
    } else {
      cword = times85(cword) + DE(c);
      byteout((int)((cword >> 24) & 255));
      byteout((int)((cword >> 16) & 255));
      byteout((int)((cword >> 8) & 255));
      byteout((int)(cword & 255));
      cword = 0;
      bcount = 0;
    }
  } else {
    fatal();
  }
}

FILE *tmp_file;

void byteout(int c )
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
  putc(c, tmp_file);
}

int main(
  int argc,
  char **argv )
{
  int c;
  long i;
  char buf[100];
  long int n1, n2, oeor, osum, orot;

  if (argc != 1) {
    fprintf(stderr,"atob: bad args to %s\n", argv[0]);
    exit(2);
  }

  if ((tmp_file = tmpfile())== NULL) {
    fprintf(stderr,"atob: Couldnt create tmp file\n");
    exit(2);
  }

  if(freopen(Heliosno(stdout)->Name, "wb", stdout) == NULL)
	{
	fprintf(stderr, "Failed to reopen stdout\n");
	exit(1);
	}

  /*search for header line*/
  for (;;) {
    if (fgets(buf, sizeof buf, stdin) == NULL) {
      fatal();
    }
    if (streq(buf, "xbtoa Begin\n")) {
      break;
    }
  }

  while ((c = getchar()) != EOF) {
    if (c == '\n') {
      continue;
    } else if (c == 'x') {
      break;
    } else {
      decode(c);
    }
  }
  if(scanf("btoa End N %ld %lx E %lx S %lx R %lx\n",
         &n1, &n2, &oeor, &osum, &orot) != 5) {
    fatal();
  }
  if ((n1 != n2) || (oeor != Ceor) || (osum != Csum) || (orot != Crot)) {
    fatal();
  } else {
    /*copy OK tmp file to stdout*/;
    fseek(tmp_file, 0L, 0);
    for (i = n1; --i >= 0;) {
      putchar(getc(tmp_file));
    }
  }
  exit(0);
}
