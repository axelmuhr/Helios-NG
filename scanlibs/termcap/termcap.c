/* Author : 	Alex Schuilenburg
 * 
 * Date :	4 Feb '91
 * 
 * Library of functions for access to termcap
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termcap.h>
#ifdef __HELIOS
#include <attrib.h>
#include <unistd.h>
#endif

static char TTRUE[]="\1";
#ifdef FULL_TGOTO
static char OOPS[]="OOPS";
#endif
static char pad_char = '\0'; 
static char tgotobuf[32];
#ifdef NAKED_ANSI
static char *ansi[]={	"am ",	TTRUE,
			"bs",	TTRUE,
			"ce",	"\x1b[K",
			"cl",	"50\x1b[;H\x1b[2J",
			"cm",	"\x1b[%i%d;%dH",
			"co",	"#80",
			"ho",	"\x1b[H",
			"li",	"#24",
			"nd",	"\x1b[1C",
			"pt",	TTRUE,
			"up",	"\x1b[1A",
			"\0",	"\0"
		    };
#else
#ifdef SUN_ANSI
static char *ansi[]={	"al",	"1*\x1b[1L",
			"am",	TTRUE,
			"bs",	TTRUE,
			"cd",	"\x1b[J",
			"ce",	"\x1b[K",
			"cl",	"\x1b[2J",
			"cm",	"\x1b[%i%2;%2H",
			"co",	"#80",
			"dc",	"\x1b[1P",
			"dl",	"1*\x1b[1M",
			"dn",	"\x1b[1B",
			"ei",	"\x1b[4l",
			"ho",	"\x1b[H",
			"im",	"\x1b[4h",
			"li",	"#24",
			"mi",	TTRUE,
			"nd",	"\x1b[1C",
			"as",	"\x1b[10m",
			"ae",	"\x1b[11m",
			"ms",	TTRUE,
			"pt",	TTRUE,
			"se",	"\x1b[0m",
			"so",	"\x1b[7m",
			"up",	"\x1b[1A",
			"vs",	"\x1b[>4h",  
			"ve",	"\x1b[>4l",
			"kb",	"\08",
			"ku",	"\x1b[A",
			"kd",	"\x1b[B",
			"kl",	"\x1b[D",
			"kr",	"\x1b1C", 
			"kh",	"\x1b[H",
			"kn",	"#8",
			"k1",	"\x1bOS",
			"k2",	"\x1bOT",
			"k3",	"\x1bOU",
			"k4",	"\x1bOV",
			"k5",	"\x1bOW",
			"l6",	"blue",
			"l7",	"red",
			"l8",	"white",
			"k6",	"\x1bOP",
			"k7",	"\x1bOQ",
			"k8",	"\x1bOR",
			"sr",	"\x1bM",
			"is",	"\x1b<\x1b[>1;2;3;4;5;6;7;8;9l\x1b[0m\x1b[11m\x1b[?7h",
			"\0",	"\0"
		    };
#else
/* Helios ANSI definition  */
static char *ansi[]={
#ifdef CSI
			"al",	"\x9b""L",	/* Add line		*/
			"bs",	TTRUE,		/* Backspace possible	*/
			"cd",	"\x9b""J",	/* Clear to end display	*/
			"ce",	"\x9b""K",	/* Clear to end of line	*/
			"cl",	"\x9b""1;1H\x9b""J",	/* Clear screen	*/
			"cm",	"\x9b""%i%2;%2H",	/* Cursor move	*/
			"co",	"#80",		/* Columns on screen	*/
			"dc",	"\x9b""P",	/* Delete Character	*/
			"dl",	"\x9b""M",	/* Delete Line		*/
			"do",	"\x9b""1B",	/* Cursor Down		*/
			"li",	"#24",		/* Lines on screen	*/
			"nd",	"\x9b""1C",	/* Cursor Right		*/
			"up",	"\x9b""1A",	/* Cursor Up 		*/
			"sf",	"\x9b""1T",	/* Scroll forwards	*/
			"sr",	"\x9b""1S",	/* Scroll backwards	*/

			"se",	"\x9b""0m",	/* End rendition mode	*/
			"so",	"\x9b""7m",	/* Begin rendition mode	*/
#else
			"al",	"\x1b[L",	/* Add line		*/
			"bs",	TTRUE,		/* Backspace possible	*/
			"cd",	"\x1b[J",	/* Clear to end display	*/
			"ce",	"\x1b[K",	/* Clear to end of line	*/
			"cl",	"\x1b[1;1H\x1b[J",	/* Clear screen	*/
			"cm",	"\x1b[%i%2;%2H",	/* Cursor move	*/
			"co",	"#80",		/* Columns on screen	*/
			"dc",	"\x1b[P",	/* Delete Character	*/
			"dl",	"\x1b[M",	/* Delete Line		*/
			"do",	"\x1b[1B",	/* Cursor Down		*/
			"li",	"#24",		/* Lines on screen	*/
			"nd",	"\x1b[1C",	/* Cursor Right		*/
			"up",	"\x1b[1A",	/* Cursor Up 		*/
			"sf",	"\x1b[1T",	/* Scroll forwards	*/
			"sr",	"\x1b[1S",	/* Scroll backwards	*/

			"se",	"\x1b[0m",	/* End rendition mode	*/
			"so",	"\x1b[7m",	/* Begin rendition mode	*/
#endif
			"kb",	"\08",		/* Backspace Key	*/
			"kh",	"\x9b""H",	/* Home Key		*/
			"ku",	"\x9b""A",	/* Up Arrow		*/
			"kd",	"\x9b""B",	/* Down Arrow		*/
			"kr",	"\x9b""C",	/* Right Arrow		*/
			"kl",	"\x9b""D",	/* Left Arrow		*/
			"&8",	"\x9b""1z",	/* Undo	Key		*/
			"@7",	"\x9b""2z",	/* End Key		*/
			"kH",	"\x9b""2z",	/* End Key		*/
			"kI",	"\x9b""@",	/* Insert Key		*/
			"kN",	"\x9b""4z",	/* PageDown Key		*/
			"kP",	"\x9b""3z",	/* PageUp Key		*/
			"%1",	"\x9b""?~",	/* Help	Key		*/

			"kn",	"#10",		/* Num of Function Keys	*/
			"k0",	"\x9b""9~",	/* Function Key 10	*/
			"k1",	"\x9b""0~",	/* Function Key 1	*/
			"k2",	"\x9b""1~",	/* Function Key 2	*/
			"k3",	"\x9b""2~",	/* Function Key 3	*/
			"k4",	"\x9b""3~",	/* Function Key 4	*/
			"k5",	"\x9b""4~",	/* Function Key 5	*/
			"k6",	"\x9b""5~",	/* Function Key 6	*/
			"k7",	"\x9b""6~",	/* Function Key 7	*/
			"k8",	"\x9b""7~",	/* Function Key 8	*/
			"k9",	"\x9b""8~",	/* Function Key 9	*/
			"k;",	"\x9b""9~",	/* Function Key 10	*/
			"l1",	"F1",		/* Label 1		*/
			"l2",	"F2",		/* Label 2		*/
			"l3",	"F3",		/* Label 3		*/
			"l4",	"F4",		/* Label 4		*/
			"l5",	"F5",		/* Label 5		*/
			"l6",	"F6",		/* Label 6		*/
			"l7",	"F7",		/* Label 7		*/
			"l8",	"F8",		/* Label 8		*/
			"l9",	"F9",		/* Label 9		*/
			"l0",	"F10",		/* Label 10		*/
			"\0",	"\0"
		    };
#endif
#endif
#ifdef DEBUG
#include "stdio.h"
extern	FILE		*outf;
#endif

int tgetent(char *bp, char *name)
{
  /* This is a dummy routine of tgetent.  The only terminal definition
   * is that of an ANSI terminal which is stored in static memory.
   */
  return(!strcmp(name,"ansi"));
}

int tgetnum(char *id)
{
  /* Search through ansi for id and return value */
  char **idtab;
#ifdef __HELIOS
  Attributes attr;
  Stream *output = fdstream(fileno(stdout));

  if ((id[0] == 'l') && (id[1] == 'i')) {		/* Capture line check	*/
    if (isatty(fileno(stdout))) {
       GetAttributes( output, &attr );
       return(attr.Min);		/* Window Lines	*/
    }					/* Failure will return std #	*/
  } else if ((id[0] == 'c') && (id[1] == 'o')) {	/* Capture column check	*/
    if (isatty(fileno(stdout))) {
       GetAttributes( output, &attr );
       return(attr.Time);		/* Window Columns	*/
    }					/* Failure will return std #	*/
  }
#endif

  for ( idtab=ansi; **idtab; idtab+=2 )
	if( (*idtab)[0] == id[0] && (*idtab)[1] == id[1] )
	{ idtab++; break; }	

  if (**idtab=='#') { 			/* id found and is a numeric.	*/
      return(atoi((*idtab) + 1)); 	/* past the # of the entry, 	*/
					/* and give the value. 		*/
  } else return(-1);  			/* Not found or not numeric. 	*/
}

int tgetflag(char *id)
{
  /* Search through ansi for presence of id */
  char **idtab;

  for ( idtab=ansi; **idtab; idtab+=2 )
	if( (*idtab)[0] == id[0] && (*idtab)[1] == id[1] )
	{ idtab++; break; }	
  if (*idtab==TTRUE) {  			/* id found and is TTRUE.	*/
    return(1); 				/* Found !!!		 	*/
  } else return(NULL); 			/* Not found !!!	 	*/
}

char *tgetstr(char *id, char **area)
{
  /* Search through ansi for id and return string */
  char **idtab, *copy;
  for ( idtab=ansi; **idtab; idtab+=2 )
	if( (*idtab)[0] == id[0] && (*idtab)[1] == id[1] )
	{ idtab++; break; }	
  if (**idtab && (**idtab!='#')&&(*idtab!=TTRUE)) { /* id found and is a string.	*/
    if (**area) {
       for (copy=*idtab; *copy; copy++){	/* Not NULL, so copy.		*/
           **area = *copy;
           (*area)++;
       }
    }
    return(*idtab);			/* Return pointer to string.	*/
  } else return(NULL); 			/* Not found, return NULL. 	*/
}

#ifdef FULL_TGOTO
void *tpad(int num, char **str, int pad)   /* Internal function to pad 0's  */
{  char buf[20];
   char *ch;

   if (num >= 0) {
      for (ch = buf; num; ch++, pad--) {   	/* Decode to ASCII */
         *ch = (num % 10) + '0';
         num = num / 10;               
      }
      for (;(pad > 0); pad--) {
         **str = '0';	/* Leading Zeros */
         (*str)++;
      }
      do {
         **str = *--ch;	/* Reverse copy */
         (*str)++;
      } while (ch != buf);
      **str = '\0';
   }
}

char *tgoto(char *cm, int destcol, int destline)
{ /*  Parse string from left to right, placing compiled str in tgotobuf */
  char *buf;
  int doline;

  doline = 1;
  for (buf=tgotobuf; *cm; cm++) {
    if (*cm == '%') {
       cm++;
       switch (*cm) {
          case 'c' :		/* %c */
            *buf = doline ? (char) destline : (char) destcol;
            buf++;
            doline = !doline;
            break;
          case 'd' :            /* %d */
            tpad(doline ? destline : destcol, &buf, 0); 
            doline = !doline;
            break;
          case '2' :            /* %2d */
            tpad(doline ? destline : destcol, &buf, 2); 
            doline = !doline;
            break;
          case '3' :            /* %3d */
            tpad(doline ? destline : destcol, &buf, 3); 
            doline = !doline;
            break;
          case '%' :		/* % required */
            *buf = '%';
            buf++;
            break;
          case 'r' :		/* Reverse line and col order (Def:line 1st) */
            doline = !doline;
            break;
          case 'i' :		/* Use 1 origin */
            destcol++;
            destline++;
            break;
          case '>' :      	/* '>xy' if x > value then value += y  */
            cm++;
            if (doline) {
               if (destline > (*cm++ - '0')) destline += (*cm - '0'); }
            else {
               if (destcol > (*cm++ - '0')) destcol += (*cm - '0');   }
            break;
          case 'n' :		/* XOR row and col with 0140 (DM2500) */
            destline ^= 0140;
            destcol ^= 0140;
            break;
          case 'B' :		/* BCD */
            if (doline) 
               destline = (16*(destline/10)) + (destline % 10);
            else 
               destcol = (16*(destcol/10)) + (destcol % 10);
            break;
          case 'D' :		/* Delta Data */
            if (doline) 
               destline = destline - 2*(destline%16);
            else 
               destcol = destcol - 2*(destcol%16);
            break;
          default :		/* Unrecognised !!! */
            return(OOPS);
       }
    } else {
      *buf = *cm;
      buf++;
    }
  }
  *buf = '\0';
  return(tgotobuf);  /* Parse successful, return pointer to buffer area */
}
#else
/* The HELIOS tgoto string will always be returned */
char *tgoto(char *cm, int destcol, int destline)
{ int i;

  destcol++;   destline++;			/* HELIOS works off base 1 */
#ifdef CSI
  tgotobuf[0] = '\x9b';
  i = 1;
#else
  tgotobuf[0] = '\x1b';
  tgotobuf[1] = '[';
  i = 2;
#endif
  if (destline > 9) tgotobuf[i++] = (destline / 10) + '0';
  tgotobuf[i++] = (destline % 10) + '0';
  tgotobuf[i++] = ';';
  if (destcol > 9)  tgotobuf[i++] = (destcol / 10) + '0';
  tgotobuf[i++] = (destcol % 10) + '0';
  tgotobuf[i++] = 'H';
  tgotobuf[i] = '\0';

  return(tgotobuf);
}
#endif

void tputs(register char *cp, int affcnt, int (*outc)(char c))
{ register int pad = 0;
  int i;
  if (cp!=NULL) {
    for (; ((*cp >= '0') && (*cp <= '9')); cp++) {  /* Padding info follows */
      pad = pad*10 + (*cp - '0');
    }
    if (*cp == '*') {		/* Affected by the number of lines */
      pad *= affcnt;
      cp++;			/* Skip to next char */
    }
    for (; *cp; cp++) {
	(*outc)(*cp);
# ifdef DEBUG
           fprintf(outf, "tputs(%x)\n",*cp);
# endif
    }
    if (pad) for (i=0; i < pad; i++) (*outc)(pad_char); /* Delay required */
  }
}


