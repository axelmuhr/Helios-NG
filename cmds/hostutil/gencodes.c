/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- gencodes.c								--
--                                                                      --
--	program to generate header files from fault database		--
--                                                                      --
--	Author:  NHG 7/3/89						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 	@(#)gencodes.c	1.1	14/3/89 Copyright (C) 1989, Perihelion Software Ltd.*/


/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <stdio.h>
#if defined(R140) || defined(SUN4) || defined(HP) || defined(RS6000)
# include <strings.h>
#else
# include <string.h>
# include <stdlib.h>
# include <posix.h>
#endif

#if defined(__HELIOS)
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <fcntl.h>

#define FDBBUFMAX	2000

typedef struct FDB {
	int	stream;
	int	pos;
	int	upb;
	char	buf[FDBBUFMAX];
} FDB;

#if !defined(R140) && !defined(SUN4) && !defined(HP) && !defined(RS6000)
static char *skipname(char *p);
static char *skipspace(char *p);
static int strtonum(char *s);
static void rdline(FDB *fdb, char *buf);
static int rdch(FDB *fdb);

#if defined(__HELIOS)
static void tfprintf(FILE *f, char *fmt, ... );
#else
static void tfprintf(FILE *f, char *fmt, long va_alist );
#endif

#else
static char *skipname();
static char *skipspace();
static int strtonum();
static void rdline();
static int rdch();
extern char *malloc();
static void tfprintf();
#endif

/*--------------------------------------------------------
-- fdbopen						--
--							--
-- Open faults database					--
--							--
--------------------------------------------------------*/

extern FDB *fdbopen(name)
char * name;
{
	int s;

	FDB *fdb = (FDB *)malloc(sizeof(FDB));

	if( fdb == NULL ) return NULL;

	fdb->pos = 0;	
	fdb->upb = 0;	
	
	s = open(name,O_RDONLY);
	
	if( s < 0 ) goto fail;
	
	fdb->stream = s;
	
	return fdb;
fail:
	if( fdb != NULL ) free(fdb);
	return NULL;
}

/*--------------------------------------------------------
-- fdbclose						--
--							--
-- Close faults database				--
--							--
--------------------------------------------------------*/

extern void fdbclose(fdb)
FDB *fdb;
{
	close(fdb->stream);
	free(fdb);
}

/*--------------------------------------------------------
-- fdbrewind						--
--							--
-- Rewind faults database				--
--							--
--------------------------------------------------------*/

extern void fdbrewind(fdb)
FDB *fdb;
{
	fdb->pos = 0;
	fdb->upb = 0;
	lseek(fdb->stream,0,0);
}

/*--------------------------------------------------------
-- fdbscan						--
--							--
--							--
--------------------------------------------------------*/

extern void fdbscan(name)
char *name;
{
#if 0
	char *line = malloc(1000);
	char *classline = malloc(1000);
#else
	static char line [1000];
	static char classline [1000];
#endif
	char *codename, *codenum, *codemsg, *p;
	char *classmask, *classpfix;
	int classbits;
	FDB *fdb = fdbopen(name);
	FILE *f = stdout;
	int classshift;
	int copying = 0;
	
	if( fdb == NULL ) 
	{
		fprintf(stderr,"cannot open %s\n",name);
		return;
	}
	
	for(;;)
	{
		rdline(fdb,line);
/*printf("%s",line);*/
		if( copying )
		{
			if( line[0] == '#' && line[1] == '}' )
			{ copying = 0; continue; }
			fprintf(f,"%s",line);
			continue;			
		}
		switch( line[0] )
		{
		case '\0':		/* EOF		*/
			return;
			
		case '\n':		/* blank line 	*/
			fprintf(f,"\n");
			break;
			
		case '#':		/* comment	*/
			switch( line[1] )
			{
			case '#':
				fprintf(f,"%s",line+1);
				break;
			case '!':
				p = line+2;
				while( *p != '\n' ) p++;
				*p = 0;
				if( f != stdout ) fclose(f);
				if( (f = fopen(line+2,"w")) == NULL )
				{
					fprintf(stderr,"Cannot open %s\n",line+2);
					return;
				}
				break;
			case '{':
				copying = 1;
				break;
			default:
				p = line+1;
				while( *p != '\n' ) p++;
				*p = 0;
				tfprintf(f,"/*%s\t\t\t\t\t*/\n", line+1);
				break;
			}
			break;
		case '!':		/* class name	*/
			if( line[1] == '!' ) continue;

			(void)strcpy(classline,line);			
			p = skipspace(classline+1);
			p = skipname(p);
			p = skipspace(p);
			classmask = p;
			p = skipname(p);
			p = skipspace(p);
			classpfix = p;
			p = skipname(p);
			classbits = strtonum(classmask);
			classshift = 0;
			while(((classbits>>classshift) & 1) == 0) classshift++;

			if( classpfix[0] != 0 )
			{
				tfprintf(f,"#define\t%sMask\t%s\n",classpfix,classmask);
				tfprintf(f,"#define\t%sShift\t%d\n",classpfix,classshift);
			}
			break;
			
		default:		/* code line	*/
			p = skipspace(line);
			codename = p;
			p = skipname(p);
			p = skipspace(p);
			codenum = p;
			p = skipname(p);
			p = skipspace(p);
			codemsg = p;
			while( *p != '\n' && *p != 0 ) p++;
			*p = 0;

			if( *codemsg )
				tfprintf(f,"#define\t%s%s\t%s\t/*\t%s\t*/\n",
					classpfix,codename,codenum,codemsg);
			else tfprintf(f,"#define\t%s%s\t%s\n",
					classpfix,codename,codenum);
		}
	}	

}

static int tabtab[] = { 8, 20, 12, 3, 29, 2, 8 };

#if defined(__HELIOS)
static void tfprintf(FILE *f, char *fmt, ...)
{
#else
static void tfprintf(f, fmt, va_alist )
FILE *f;
char *fmt;
va_dcl
{
#endif
	va_list a;
	static char fbuf[100];
	int n = 0;
	int pos = 0;
	int tabn = 0;
	int i;
	int percent = 0;

#if defined(__HELIOS)
	va_start(a, fmt);
#else	
	va_start(a);
#endif
	
	for(;;)
	{
		char c = *fmt++;
		if( c == 0 || c == '\t' || c == '%')
		{
			int newpos;
			fbuf[n] = 0;
			if( percent ) 
			{
				static char pbuf[100];
				sprintf(pbuf,fbuf,va_arg(a,int));
				fputs(pbuf,f);
				pos += strlen(pbuf);
			}
			else { fputs(fbuf,f); pos += n; }
			if( c == '%' )
			{
				n = 1;
				fbuf[0] = c;
				percent = 1;
				continue;
			}
			else percent = 0;
			if( c == 0 ) break;
			newpos = 0;
			for( i = 0; i <= tabn; i++ ) newpos += tabtab[i];
			tabn++;
			while( pos < newpos )
			{
#if 1
				int nexttab = (pos+8)/8;
				nexttab *= 8;
				if( nexttab <= newpos ) 
				{ putc('\t',f); pos = nexttab; }
				else
#endif
				{ putc(' ',f); pos++; }
			}
			n = 0;
		}
		else fbuf[n++] = c;
	}
	va_end(a);
}


static char *skipname(p)
char *p;
{
	while( *p != ' ' && *p != '\t' && *p != '\n' && *p != 0 ) p++;
	if( *p == 0 ) return p;
	*p++ = 0;
	return p;
}

static char *skipspace(p)
char *p;
{
	while( *p == ' ' || *p == '\t' ) p++;
	return p;
}

static int strtonum(s)
char *s;
{
	int radix = 10;
	int val = 0;
	
	while( *s )
	{
		char c = *s++;
		int digit;
		
		if( c == 'x' ) { radix = 16; continue; }
		
		if( '0' <= c && c <= '9' ) digit = c - '0';
		else if( radix == 16 && ('a' <= c && c <= 'f') ) digit = c - 'a' + 10;
		else if( radix == 16 && ('F' <= c && c <= 'F') ) digit = c - 'A' + 10;

		val = val * radix + digit;
	}
	
	return val;
}

static void rdline(fdb,buf)
FDB *fdb; char *buf;
{
	int pos = 0;
	int c = 0;

	for(;;)
	{
		c = rdch(fdb);
		switch( c )
		{
		case -1: goto done;
		case '\r': continue;
		case '\t':
			do { buf[pos++] = ' '; } while(pos % 8);
			break;
		default:
			buf[pos++] = c; 
		}
		if( c == '\n' ) break;
	}
done:
	buf[pos] = 0;
}

static int rdch(fdb)
FDB *fdb;
{
	if( fdb->upb == -1 ) return -1;
	if( fdb->pos == fdb->upb )
	{
		int size = read(fdb->stream,fdb->buf,FDBBUFMAX);
		if( size <= 0 ) 
		{
			fdb->upb = -1;
			return -1;
		}
		fdb->upb = size;
		fdb->pos = 0;
	}
	return fdb->buf[fdb->pos++];
}


int main(argc,argv)
int argc; char **argv;
{
	if( argc < 2 ) 
	{
		printf("usage: gencodes fdb\n");
		exit(1);
	}
	
	fdbscan(argv[1]);
	return 0;
}


/* -- End of gencodes.c */
