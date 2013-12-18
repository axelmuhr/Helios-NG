#ifndef __old
#include <time.h>
#include "globals.h"
#include "defs.h"
#include <stdarg.h>
#include "xrefs.h"
#include "util.h"
#include "cg.h"
#include "mcdep.h"
#else
#include <stdarg.h>
#include <time.h>
#include "cchdr.h"
#include "cg.h"
#include "xrefs.h"
#include "util.h"
#define int32 int
#endif

/* stubs for Norcroft C */

#ifndef __STDC__
static int fpargs[20];

int _vfprintf(fd,str,a) 
FILE  *fd;
char *str;
va_list a;
{
	char *s = str;
	int i = 0;

	while( *s != 0 ) 
		if( *s++ == '%' )
			if( *s != '%' )
				fpargs[i++] =  va_arg(a,int);

/*	fprintf(fd,"\nvfprintf =%s= =%s= \n",str,a0); 	*/
/*	fprintf(fd,"str = %8x a = %8x %8x\n",str,a,a0);  */
	fprintf(fd,str,fpargs[0],fpargs[1],fpargs[2],fpargs[3],fpargs[4],
		       fpargs[5],fpargs[6],fpargs[7],fpargs[8],fpargs[9]);
}
#endif
#if 0
int _vfprintf(fd,str,a) 
FILE *fd;
char *str;
int *a;
{
	fprintf(fd,"\nvfprintf =%s= =%s= \n",str,a[0]); 
	fprintf(fd,str,a[0],a[1],a[2],a[3],a[4]);
}
#endif

#ifndef __STDC__
void memcpy(d,s,n) 
char *s,*d;
int n;
{
	while( n-- ) *d++ = *s++;
}

void memset(a,v,n) 
char *a;
int v,n;
{
	while( n-- ) *a++ = v;
}
#endif

int annotations = 0;

#ifndef __STDC__
static char *tt = "<unset>";

char *ctime() { return tt; }
time_t time() { return 0; }
clock_t clock() { return 0 ;}
#endif

DataXref *dataxrefs;

#ifdef __old
void obj_init() {}
void obj_header() {}

void mcdep_init() {}
void obj_trailer() {}

#endif

int32 obj_symref(Symstr *s, int flags, int32 loc) {return 0;}


FILE *objstream, *asmstream;

#ifndef __old

int32 config;

bool mcdep_config_option(char s1, char s2[]) { return NO; }

#endif

/* code generation stubs */

int block_cur, icode_cur;

void display_assembly_code(Symstr *s) {}
void obj_codewrite(Symstr *s) {}
CodeXref *codexrefs;
void branch_round_literals(LabelNumber *m) {}
void setlabel(LabelNumber *l) {}

BindListList *current_env;
void show_entry() {}
void show_code() {}

int min(a,b)
int a,b;
{
	return a<b ? a : b ;
}

#ifndef __STDC__
int tolower(c)
int c;
{
	if( isupper(c) ) return c - 'A' + 'a';
	else return c;
}
#endif

#ifdef CC420
int32 cse_debugcount;

void config_init( void )
{
	config	= CONFIG_HAS_MULTIPLY;
}

KW_Status mcdep_keyword
(
	const char 	*key,
	int		*argp,
	char		**argv
)
{
	return KW_NONE;
}


#endif
