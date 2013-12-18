/*****************************************************************************
 * A program for adding or changing environment variable values for MSDOS.
 * The "set" command provided by command.com is very limited.  It fails to
 * provide the ability to use quotation marks and escape characters and
 * octal/hex constants in the value definition.  Setenv provides these
 * abilities.
 *
 * Usage notes:
 *
 *	setenv <symbol> = <value>
 *
 *	<symbol> ::= legal MSDOS environment symbol.  Lower case converted
 *		     to uppercase.
 *
 *	<value>  ::= environment symbol value in one of three forms:
 *
 *		     * No quotation marks.  The value is the literal string
 *		       of characters starting IMMEDIATELY after the equal
 *		       sign and extending to the end-of-line.
 *
 *		     * Single quotation marks (').  The value is the literal
 *		       string enclosed in quotation marks.
 *
 *		     * Double quotation marks (").  The value is the string
 *		       enclosed in double quotation marks.  Backslash escape
 *		       constructions are processed -- this includes the usual
 *		       C language constructions such as \n for newline and
 *		       \r for carriage return plus octal and hexadecimal
 *		       constants (\ddd & \0xdd, respectively).
 *****************************************************************************/

/*****************************************************************************
 * Based on a program by Alan J Myrvold (ajmyrvold@violet.waterloo.edu)
 *
 * WARNING WARNING WARNING - virtually no error checking is done !!
 *                           use at own risk !!
 *
 * This program by Larry A. Shurr (las@cbema.ATT.COM)
 *
 * I added checking for env seg overrun, so now it's a little more robust.
 *****************************************************************************/

/*****************************************************************************
 *
 * Notes by Alan J Myrgold:
 *
 * Technical information : A program's PSP contains a pointer at
 * offset 44 (decimal) to a COPY of the parent's environment.
 * The environment is a set of strings of the form NAME=value,
 * each terminated by a NULL byte.
 * An additional NULL byte marks the end of the environment.
 * The environment area is ALWAYS paragraph aligned
 * i.e. on a 16 byte boundary.
 *
 * Searching backwards from the PSP, I consistently find
 * two copies of the envronment area.
 *
 * The program : finds the two areas
 *               reads one into memory
 *               udpates the specified environment variable
 *               writes updated environment to parent environment
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <process.h>
#include <conio.h>
#include <dos.h>

#define FALSE 0
#define TRUE  1

struct mcb {				/* MSDOS Memory Control Block */
  unsigned char tag4D;			/* Tag field must = 0x4D */
  unsigned int  next;			/* Segment base for next block */
  unsigned int  size;			/* Memory block size in paragraphs */
};


unsigned env_size = 0;			/* Maintain size of environment */
/***************************************************************************/
int env_size_bytes(unsigned env_seg)
/* Determine the length of the environment area in bytes */
{
    int n;

    n = 0;
    while (peekb(env_seg,n) != 0) {
          while (peekb(env_seg,n) != 0) n++;
          n++;
    }
    return(n);
}
/***************************************************************************/
int env_size_strings(unsigned env_seg)
/* Determine how many strings are in the environment area */
{
    int k,n;

    k = n = 0;
    while (peekb(env_seg,n) != 0) {
          k++;
          while (peekb(env_seg,n) != 0) n++;
          n++;
    }
    return(k);
}
/***************************************************************************/
int peek_cmp(unsigned seg1,unsigned seg2,int nbytes)
/* A trivial compare routine for segement aligned data items */
{
   int i;

   for (i = 0; (i < nbytes) && (peekb(seg1,i) == peekb(seg2,i)); i++);
   return(i == nbytes);
}
/***************************************************************************/
void find_env(unsigned seg_ray[2])
{
    unsigned psp_seg,copy_of_seg,env_seg;
    int k,n;

/* Find first copy of environment */
    psp_seg = _psp;
    copy_of_seg = peek(psp_seg,44);

/* Set return value to non-garabage */
    seg_ray[0] = seg_ray[1] = copy_of_seg;

/* Search back to find 2 copies of environment */
    env_size = n = env_size_bytes(copy_of_seg);
    env_seg = copy_of_seg - 1;
    for (k = 0; (env_seg != 0) && (k < 2); k++) {
          while ((env_seg != 0) &&
                 (peek_cmp(copy_of_seg,env_seg,n) == 0)) {
                     env_seg--;
          }
          if (env_seg != 0) {
              seg_ray[k] = env_seg;
              env_seg--;
          }
    }

/* If not found, display error message and abort */
    if (k != 2) {
       fprintf(stderr,"Two copies of the environment were not found\n");
       exit(-1);
    }
}
/***************************************************************************/
void read_env(unsigned env_seg,int *k,char ***s,char ***t)
/* Read environment into a malloc'd array of malloc'd strings */
{
  int i,j,n;

  env_size = env_size_bytes(env_seg);

  *k = env_size_strings(env_seg);
  *s = (char **) malloc((*k)*sizeof(char *));
  *t = (char **) malloc((*k)*sizeof(char *));

  n = 0;
  for (i = 0; i < *k; i++) {
    for (j = 0; peekb(env_seg,n+j) != '='; j++);
    (*s)[i] = (char *) malloc(j+1);
    for (j = 0; peekb(env_seg,n+j) != '='; j++)
      ((*s)[i])[j] = peekb(env_seg,n+j);
    ((*s)[i])[j] = 0;
    n += j + 1;
    for (j = 0; peekb(env_seg,n+j) != 0; j++);
    (*t)[i] = (char *) malloc(j+1);
    for (j = 0; peekb(env_seg,n+j) != 0; j++)
      ((*t)[i])[j] = peekb(env_seg,n+j);
    ((*t)[i])[j] = 0;
    n += j + 1;
  }
}
/***************************************************************************/
void write_env(unsigned env_seg, int k, char **s, char **t)
/* Write the environment back out to memory */
{
  int i,j,n;

  struct mcb far *tmcb = (struct mcb far *)((long)(env_seg-1) << 16);

  if (tmcb->tag4D == 0x4D) {
    unsigned env_seg_siz = tmcb->size << 4;
    if (env_size < env_seg_siz) {
      for (n = i = 0; i < k; i++) {
        char *si = s[i];
        char *ti = t[i];
	for (j = 0; si[j] != 0; j++) pokeb(env_seg,n++,si[j]);
	pokeb(env_seg,n++,'=');
	for (j = 0; ti[j] != 0; j++) pokeb(env_seg,n++,ti[j]);
	pokeb(env_seg,n++,0);
      }
      pokeb(env_seg,n,0);
    } else {
      fprintf(stderr,"Insufficient space in environment\n");
      exit(-1);
    }
  } else {
    fprintf(stderr,"Environment memory control block trashed\n");
    exit(-1);
  }
}
/***************************************************************************/
char *get_env_var(int k,char **s,char **t,char *var)
/* Return the value of the environment variable or NULL if not found */
{
    char *val;
    int i;

    val = NULL;
    for (i = 0; i < k; i++) if (stricmp(s[i],var) == 0) val = t[i];

    return(val);
}

/***************************************************************************/
void set_env_var(int *k,char ***s,char ***t,char *var,char *val)
/* Set a new or existing environment variable to a new value */
{
  int i,done;

  done = 0;
  for (i = 0; i < *k; i++) {
    if (stricmp((*s)[i],var) == 0) {
      /* Existing variable */
      done = 1;
      env_size -= strlen((*t)[i]);
      free((*t)[i]);
      (*t)[i] = (char *) malloc(1+strlen(val));
      strcpy((*t)[i],val);
      env_size += strlen((*t)[i]);
    }
  }

  if (!done) {
    /* New environment variable */
    (*k)++;
    *s = realloc(*s,(*k)*sizeof(char *));
    *t = realloc(*t,(*k)*sizeof(char *));
    (*s)[*k-1] = (char *) malloc(1+strlen(var));
    strcpy((*s)[*k-1],var);
    strupr((*s)[*k-1]);
    (*t)[*k-1] = (char *) malloc(1+strlen(val));
    strcpy((*t)[*k-1],val);
    /* Length of name  + length of '=' + length of value + length of '\0' */
    env_size += (strlen((*s)[*k-1]) + 1 + strlen((*t)[*k-1]) + 1);
  }
}
/***************************************************************************/
void show_env(int k,char **s,char **t)
/* Display the array of environment strings */
{
   int i;
   for (i = 0; i < k; i++) printf("%s=%s\n",s[i],t[i]);
}
/***************************************************************************/
void get_cmdline(char *cmd)
/* Read raw command line text into string buffer */
{
    char far *pcmd;

    int idx,odx;

    pcmd = (char far *)((long)_psp << 16) + 128L;

    for (idx = *pcmd++, odx = 0; idx > 0; idx--, odx++) {
      cmd[odx] = *pcmd++;
    }

    cmd[odx] = '\0';
}
/***************************************************************************/
char_in(char ch, char *set)
/* Determine if a character is in a set of characters */
{
  do {
    if (ch == *set) return(TRUE);
  } while ((int)*(++set));
  return(FALSE);
}
/***************************************************************************/
char get_num(char *cmd, int *pidx)
/* Interpret octal or hexadecimal constant in string */
{
  int   accum  = 0;
  char  ch;
  int   f_scan = TRUE;
  int   idx    = *pidx;
  int   limit;
  char *nch    = cmd+idx;
  char *och    = nch+1;
  int   radix;

#define HEXDIG "0123456789ABCDEFabcdef"

  if (*nch == '0' && char_in(*och,"xX") && char_in(*(och+1),HEXDIG)) {
    radix = 16;
    limit = 2;
    och += 1;
  } else {
    radix = 8;
    limit = 3;
    och = nch;
  }

  while (limit-- > 0 && f_scan) {

    f_scan = FALSE;

    while ((int)(*nch)) *nch++ = *och++;

    nch = cmd+idx;
    och = nch+1;

    switch (ch = *nch) {
      case '0' :
      case '1' :
      case '2' :
      case '3' :
      case '4' :
      case '5' :
      case '6' :
      case '7' :
      case '8' :
      case '9' :
	if (ch == 9 && radix == 8) break;
	accum = accum * radix + (int)(ch - '0');
        f_scan = TRUE;
	break;
      case 'A' :
      case 'B' :
      case 'C' :
      case 'D' :
      case 'E' :
      case 'F' :
	if (radix == 8) break;
	accum = accum * radix + (int)(ch - 'A') + 10;
        f_scan = TRUE;
	break;
      case 'a' :
      case 'b' :
      case 'c' :
      case 'd' :
      case 'e' :
      case 'f' :
	if (radix == 8) break;
	accum = accum * radix + (int)(ch - 'a') + 10;
        f_scan = TRUE;
	break;
      default  : break;
    }
  }

  *pidx = idx;
  return(accum);
}
/***************************************************************************/
get_escape(char *cmd, int *pidx, char quote)
/* Interpret escape'd (i.e., '\' (backslash) character */
{
  int   idx = *pidx;

  if (quote == '"') {
    char *nch = cmd+idx;
    char *och = nch+1;
    char *xch = nch;
    while ((int)(*nch)) *nch++ = *och++;
    switch (*xch) {
      case 'a' : *xch = '\a'; break;
      case 'b' : *xch = '\b'; break;
      case 'c' : *xch = '\c'; break;
      case 'd' : *xch = '\d'; break;
      case 'e' : *xch = '\e'; break;
      case 'f' : *xch = '\f'; break;
      case 'g' : *xch = '\g'; break;
      case 'h' : *xch = '\h'; break;
      case 'i' : *xch = '\i'; break;
      case 'j' : *xch = '\j'; break;
      case 'k' : *xch = '\k'; break;
      case 'l' : *xch = '\l'; break;
      case 'm' : *xch = '\m'; break;
      case 'n' : *xch = '\n'; break;
      case 'o' : *xch = '\o'; break;
      case 'p' : *xch = '\p'; break;
      case 'q' : *xch = '\q'; break;
      case 'r' : *xch = '\r'; break;
      case 's' : *xch = '\s'; break;
      case 't' : *xch = '\t'; break;
      case 'u' : *xch = '\u'; break;
      case 'v' : *xch = '\v'; break;
      case 'w' : *xch = '\w'; break;
      case 'x' : *xch = '\x'; break;
      case 'y' : *xch = '\y'; break;
      case 'z' : *xch = '\z'; break;
      case '0' :
      case '1' :
      case '2' :
        *xch = get_num(cmd, &idx);
        break;
    }
  }

  *pidx = idx + 1;
}
/***************************************************************************/
get_qvalue(char *cmd, int idx, char quote, char **value)
/* Extract a quoted value part from command line */
{
  char ch;
  int  f_esc;

  *value = cmd + (++idx);

  do {
    while ((int)(ch = cmd[idx]) && ch != '\\' && ch != quote) idx++;
    if (!(int)ch) return(-1);
    if (ch == '\\') {
      f_esc = TRUE;
      get_escape(cmd, &idx, quote);
    } else f_esc = FALSE;
  } while (f_esc);

  cmd[idx] = '\0';

  for (idx += 1; (int)(ch = cmd[idx]) && ch == ' '; idx++);

  if ((int)ch) return(-1);

  return(0);
}
/***************************************************************************/
get_parm(char **name, char **value)
/* Extract environment symbol name and value from command line */
{
    char ch;
    int  idx;
    int  sdx;

    static char cmd[128];

    get_cmdline(cmd);

    for (idx = 0; (int)(ch = cmd[idx]) && ch  == ' '; idx++);

    if (!(int)ch) return(1);

    *name = cmd + idx;

    for (; (int)(ch = cmd[idx]) && ch != '=' && ch != ' '; idx++);

    if (!(int)ch) return(-1);

    if (ch == ' ') {
      cmd[idx] = '\0';
      for (idx += 1; (int)(ch = cmd[idx]) && ch != '='; idx++);
    } else cmd[idx] = '\0';

    if (!(int)ch) return(-1);

    for (sdx = (idx += 1); (int)(ch = cmd[idx]) && ch == ' '; idx++);

    /*if (!(int)ch) return(-1);*/

    switch (ch) {
      case '"'  : return(get_qvalue(cmd, idx, '"', value));
      case '\'' : return(get_qvalue(cmd, idx, '\'', value));
      default   : *value = cmd + sdx; break;
    }

    return(0);
}
/***************************************************************************/
main(int argc,char **argv)
{
    unsigned env_seg[2];
    char **s,**t;
    int k;
    long now;
    struct tm *local;

    static char *name = NULL, *value = NULL;

    switch (get_parm(&name,&value)) {

    case -1:

	fprintf(stderr,"Invalid symbol definition syntax\n");
	exit(-1);

    case 0:

        /* Find and read environment */

        find_env(env_seg);
        read_env(env_seg[1],&k,&s,&t);

	/* Set the variable <name> to <value> */

	set_env_var(&k,&s,&t,name,value);

	/* Update caller's environment */

	write_env(env_seg[0],k,s,t);

	break;

    case 1:

	fprintf(stderr,"Usage: setenv <symbol name> = <value>\n");
	break;
    }

    return(0);
}
/***************************************************************************/

