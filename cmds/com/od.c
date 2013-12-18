/**
*
* Title:  od
*
* Author: Andy England
*
* Date:   19th January 1988
*
**/

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/od.c,v 1.3 1993/07/12 11:35:38 nickc Exp $";

#include	<stdio.h>
#include	<ctype.h>
#include <stdlib.h>

void od(FILE *file);
void putascii(void);
void putubhex(void);
void putcchar(void);
void putuwdec(void);
void putuldec(void);
void putfloat(void);
void putdouble(void);
void putuwhex(void);
void putulhex(void);
void putswdec(void);
void putsldec(void);
void putuwoct(void);
void putuloct(void);

int width = 16;
int length;
char *buffer;
int nflags = 0;
int field = 14;
char s[100];

struct
{
  void (*func)();
} flag[32] =
{
  putuwoct
};

char *ascii[] =
{
  "nul",
  "soh",
  "stx",
  "etx",
  "eot",
  "enq",
  "ack",
  "bel",
  "bs",
  "ht",
  "nl",
  "vt",
  "ff",
  "cr",
  "so",
  "si",
  "dle",
  "dc1",
  "dc2",
  "dc3",
  "dc4",
  "nak",
  "syn",
  "etb",
  "can",
  "em",
  "sub",
  "esc",
  "fs",
  "gs",
  "rs",
  "us",
  "sp"
};

int main(int argc, char *argv[])
{
  FILE *file;
  int argn;
  char *p;

  for (argn = 1; argn < argc; argn++)
  {
    p = argv[argn];
    if (*p++ == '-')
    {
      while (*p)
      {
        switch (*p++)
        {
          case 'a':
          flag[nflags++].func = putascii;
          field = 16;
          break;

          case 'b':
          flag[nflags++].func = putubhex;
          field = 16;
          break;

          case 'c':
          flag[nflags++].func = putcchar;
          field = 16;
          break;

          case 'd':
          flag[nflags++].func = putuwdec;
          if (field < 12) field = 12;
          break;

          case 'D':
          flag[nflags++].func = putuldec;
          if (field < 11) field = 11;
          break;

          case 'e':
          case 'F':
          flag[nflags++].func = putdouble;
          if (field < 11) field = 11;
          break;

          case 'f':
          flag[nflags++].func = putfloat;
          if (field < 14) field = 14;
          break;

          case 'h':
          case 'x':
          flag[nflags++].func = putuwhex;
          if (field < 10) field = 10;
          break;

          case 'H':
          case 'X':
          flag[nflags++].func = putulhex;
          if (field < 9) field = 9;
          break;

          case 'i':
          flag[nflags++].func = putswdec;
          if (field < 14) field = 14;
          break;

          case 'l':
          case 'I':
          case 'L':
          flag[nflags++].func = putsldec;
          if (field < 12) field = 12;
          break;

          case 'o':
          case 'B':
          flag[nflags++].func = putuwoct;
          if (field < 14) field = 14;
          break;

          case 'O':
          flag[nflags++].func = putuloct;
          if (field < 12) field = 12;
          break;

          case 'w':
          if (isdigit(*p))
          {
            width = 0;
            do width = (10 * width) + (*p - '0'); while (isdigit(*++p));
          }
          else width = 32;
          break;

          default:
          fprintf(stderr, "od: bad flag -%c\n", *(p - 1));
          exit(1);
        }
        if (nflags == 32)
        {
          fprintf(stderr, "od: too many flags\n");
          exit(1);
        }
      }
    }
    else break;
  }

  if (argc > argn + 1)
  {
    fprintf(stderr, "usage: od [ -format ] [ file ]\n");
    exit(1);
  }
  if (argc == argn) od(stdin);
  else
  {
    if ((file = fopen(argv[argc - 1], "rb")) == NULL)
    {
      fprintf(stderr, "Can't open file '%s'\n", argv[argc - 1]);
      exit(1);
    }
    od(file);
  }
  return 0;
}

void od(FILE *file)
{
  int n, c;
  unsigned long offset;

  if ((buffer = (char *)malloc(width)) == NULL)
  {
    fprintf(stderr, "width too big\n");
    exit(1);
  }
  for (offset = 0;; offset += length) 
  {
    for (length = 0; length < width; length++)
    {
      if ((c = fgetc(file)) == EOF) break;
      buffer[length] = c;
    } 
    printf("%07lo ", offset);
    if (length == 0) break;
    for (n = 0; (n == 0) || (n < nflags); n++)
    {
      if (n) putchar('\t');
      (*flag[n].func)();
      putchar('\n');
    }
  }
  putchar('\n');
}

void putascii(void)
{
  int i, c;

  for (i = 0; i < length; i++)
  {
    c = (buffer[i] & 0x7F);
    if (c == 128) sprintf(s, "del");
    else if (c <= 32) sprintf(s, "%3s", ascii[c]);
    else sprintf(s, "%3c", c);
    printf("%*s", (field >> 2), s);
  }
}

void putubhex(void)
{
  int i;

  for (i = 0; i < length; i++)
  {
    sprintf(s, "%03o", buffer[i]);
    printf("%*s", (field >> 2), s);
  }
}

void putcchar(void)
{
  int i, c;

  for (i = 0; i < length; i++)
  {
    switch (c = buffer[i])
    {
      case '\0':
      sprintf(s, " \\0");
      break;

      case '\b':
      sprintf(s, " \\b");
      break;

      case '\f':
      sprintf(s, " \\f");
      break;

      case '\n':
      sprintf(s, " \\n");
      break;

      case '\r':
      sprintf(s, " \\r");
      break;

      case '\t':
      sprintf(s, " \\t");
      break;

      default:
      if isprint(c) sprintf(s, "%3c", c);
      else sprintf(s, "%03o", c);
      break;
    }
    printf("%*s", (field >> 2), s);
  }
}

void putuwdec(void)
{
  int i;
#ifdef SHORT
  unsigned short w;
#else
  unsigned long w;
#endif

  for (i = 0; i < (length + 1) >> 1; i++)
  {
#ifdef SHORT
    w = ((unsigned short *)buffer)[i];
#else
    w = ((unsigned long *)buffer)[i / 2];
    if (i & 1) w >>= 16;
    else w &= 0xFFFF;
#endif
    sprintf(s, "%05lu", w);
    printf("%*s", (field >> 1), s);
  }
}

void putuldec(void)
{
  int i;
  unsigned long l;

  for (i = 0; i < (length + 3) >> 2; i++)
  {
    l = ((unsigned long *)buffer)[i];
    sprintf(s, "%010lu", l);
    printf("%*s", field, s);
  }
}

void putdouble(void)
{
#ifdef FLOAT
  int i;
  double d;

  for (i = 0; i < (length + 7) >> 3; i++)
  {
    d = ((double *)buffer)[i];
    sprintf(s, "%22.14lf", d);
    printf("%*s", field << 1, s);
  }
#endif
}
 
void putfloat(void)
{
#ifdef FLOAT
  int i;
  float f;

  for (i = 0; i < (length + 3) >> 2; i++)
  {
    f = ((float *)buffer)[i];
    sprintf(s, "%14.7f", f);
    printf("%*s", field, s);
  }
#endif
} 

void putuwhex(void)
{
  int i;
#ifdef SHORT
  unsigned short w;
#else
  unsigned long w;
#endif

  for (i = 0; i < (length + 1) >> 1; i++)
  {
#ifdef SHORT
    w = ((unsigned short *)buffer)[i];
#else
    w = ((unsigned long *)buffer)[i / 2];
    if (i & 1) w >>= 16;
    else w &= 0xFFFF;
#endif
    sprintf(s, "%04lx", w);
    printf("%*s", (field >> 1), s);
  }
}
 
void putulhex(void)
{
  int i;
  unsigned long l;

  for (i = 0; i < (length + 3) >> 2; i++)
  {
    l = ((unsigned long *)buffer)[i];
    sprintf(s, "%08lx", l);
    printf("%*s", field, s);
  }
}
 
void putswdec(void)
{
  int i;
#ifdef SHORT
  short w;
#else
  long w;
#endif

  for (i = 0; i < (length + 1) >> 1; i++)
  {
#ifdef SHORT
    w = ((short *)buffer)[i];
#else
    w = ((long *)buffer)[i / 2];
    if (i & 1) w >>= 16;
    else w &= 0xFFFF;
#endif
    sprintf(s, "%6ld", w);
    printf("%*s", (field >> 1), s);
  }
}
 
void putsldec(void)
{
  int i;
  long l;

  for (i = 0; i < (length + 3) >> 2; i++)
  {
    l = ((long *)buffer)[i];
    sprintf(s, "%11ld", l);
    printf("%*s", field, s);
  }
}
 
void putuwoct(void)
{
  int i;
#ifdef SHORT
  unsigned short w;
#else
  unsigned long w;
#endif

  for (i = 0; i < (length + 1) >> 1; i++)
  {
#ifdef SHORT
    w = ((unsigned short *)buffer)[i];
#else
    w = ((unsigned long *)buffer)[i / 2];
    if (i & 1) w >>= 16;
    else w &= 0xFFFF;
#endif
    sprintf(s, "%06lo", w);
    printf("%*s", (field >> 1), s);
  }
}
 
void putuloct(void)
{
  int i;
  unsigned long l;

  for (i = 0; i < (length + 3) >> 2; i++)
  {
    l = ((unsigned long *)buffer)[i];
    sprintf(s, "%011lo", l);
    printf("%*s", field, s);
  }
}
 
