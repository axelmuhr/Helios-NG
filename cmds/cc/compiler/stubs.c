/* $Id: stubs.c,v 1.2 1991/08/21 18:02:14 nick Exp $ */
#include "cchdr.h"
#include <stdarg.h>
#include <time.h>
#include "xrefs.h"
#include "util.h"

#include "cg.h"

/* stubs for Norcroft C */

#ifndef COMPILING_ON_XPUTER
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

#ifndef COMPILING_ON_XPUTER
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

int obj_symref() {}
DataXref *dataxrefs;
void obj_init() {}
void obj_header() {}

void mcdep_init() {}
void obj_trailer() {}


FILE *objstream, *asmstream;

/* code generation stubs */



int block_cur, icode_cur;

block_head *top_block;

AvailList *adconlist;
void display_assembly_code() {}
void obj_codewrite() {}
CodeXref *codexrefs;
void branch_round_literals() {}
void setlabel() {}

BindListList *current_env;
void show_entry() {}
void show_code() {}

int min(a,b)
int a,b;
{
	return a<b ? a : b ;
}

#ifndef COMPILING_ON_XPUTER
int tolower(c)
int c;
{
	if( isupper(c) ) return c - 'A' + 'a';
	else return c;
}
#endif
