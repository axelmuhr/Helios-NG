/*------------------------------------------------------------------------
--                                                                      --
--                     P O S I X    L I B R A R Y			--
--                     --------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- scandb.c								--
--                                                                      --
--	Simple database handling routines.				--
--                                                                      --
--	Author:  NHG 1/6/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* Copyright (C) 1987, Perihelion Software Ltd.				*/
/* RCSId: $Id: scandb.c,v 1.5 1993/07/12 10:22:24 nickc Exp $ */

#include "pposix.h"
#include <stdarg.h>
#include <string.h>

extern unsigned long swap_long(unsigned long a,unsigned long b);
extern unsigned short swap_short(unsigned short a,unsigned long b);


typedef  int (*IntFnPtr)();	/* pointer to int function	*/

#define	EXACT	1
#define	WILD	2
#define ASSIGN	3
#define IGNORE	4
#define ALIAS	5

#define BSIZ 128

struct DBINFO
{
	int		u[32];		/* user static area		*/

	char		*list[32];	/* argv style list		*/
	char		strbuf[512];	/* strings buffer		*/
	
	char		buf[BSIZ];	/* file buffer			*/
	int		pos;		/* pos in buffer		*/
	int		upb;		/* end of buffer		*/
	int		fd;		/* file	descriptor		*/
	int		unread;		/* last unread char		*/
	int		stayopen;	/* db stayopen flag		*/
};

extern struct DBINFO *dbinfo;

static void initdb()
{
	if( dbinfo == NULL )
	{
		dbinfo = (struct DBINFO *)Malloc(sizeof(struct DBINFO));	
		memset(dbinfo,0,sizeof(struct DBINFO));
		
		dbinfo->fd = -1;
		dbinfo->unread = -1;
	}
}


/* 
mode == 0	dont stay open, rewind if open
	1	stay open, rewind if open
	2	dont stay open, dont rewind if open
*/
extern int opendb(char *name, int mode)
{	
	CHECKSIGS();
	if( dbinfo == NULL ) initdb();

	if( dbinfo->fd == -1 )
	{
		dbinfo->fd = open(name,O_RDONLY);
		dbinfo->pos = dbinfo->upb = 0;
	}
	else if( mode != 2 ) lseek(dbinfo->fd,0,0);

	dbinfo->stayopen = dbinfo->stayopen || (mode==1);
	
	CHECKSIGS();
	return dbinfo->fd==-1?100:0;
}

/*
mode == 0	force close
	1	close iff !stayopen
*/
extern void closedb(int mode)
{
	CHECKSIGS();
	if( mode==0 || !dbinfo->stayopen )
	{
		if( dbinfo->fd >= 0 ) close(dbinfo->fd);
		dbinfo->fd = -1;
		dbinfo->pos = dbinfo->upb = 0;
		dbinfo->stayopen = FALSE;
	}
	CHECKSIGS();
}

static int rdch(void)
{
	int c = dbinfo->unread;
	
	if( c != -1 ) 
	{
		dbinfo->unread = -1;
		return c;
	}
	
	do {
		if( dbinfo->pos >= dbinfo->upb )
		{
			if( dbinfo->fd < 0 ) return -1;
			dbinfo->upb = read(dbinfo->fd,dbinfo->buf,BSIZ);
			if( dbinfo->upb <= 0 ) 
			{
				closedb(0);
				return '\n';
			}
			dbinfo->pos = 0;
		}
		c = dbinfo->buf[dbinfo->pos++];
	} while( c == '\r' );
	
	/* ^Z marks end of file for MSDOS (yuk!) */
	if( c == 0x1A ) return -1;
	else return c;
}

static void unrdch(int c)
{
	dbinfo->unread = c;
}

static int nextch(void)
{
	int c;
	
	for( c = rdch(); c > 0 ; c = rdch() )
	{
		if( c == '#' ) until( c == '\n' || c == -1 ) c = rdch();
		if( c == ' ' || c == '\t' ) continue;
		break;
	}
	return c;
}


static int getint(int c, int base, int (*rdch)(),int (*unrdch)(),void *info)
{
	int val = 0;
	int d;
	int sign = 1;
		
	if( c == '-' )
	{
		c = rdch(info);
		sign = -1;
	}

	if( c == '0' )
	{
		c = rdch(info);
		if( c == 'x' ) 
		{
			c = rdch(info);
			base = 16;
		}
		else base = 8;
	}
	
	for(;;)
	{
		if( '0' <= c && c <= '7' ) d = c - '0';
		elif( base == 10 && (c == '8' || c == '9') ) d = c - '0';
		elif( base == 16 && 'a' <= c && c <= 'f' ) d = c - 'a' + 10;
		elif( base == 16 && 'A' <= c && c <= 'A' ) d = c - 'A' + 10;
		else break;
		if( d >= base ) break;
		val = val*base + d;
		c = rdch(info);
	}
	unrdch(c,info);

	return val*sign;
}

static int rdint(int base)
{
	int c = nextch();
	return getint(c,base,(IntFnPtr)rdch,(IntFnPtr)unrdch,NULL);
}

static long rd_inet()
{
	long bit[4];
	unsigned long addr = 0;
	int i;
	int c;

	for( i = 0; i < 4; )
	{
		bit[i++] = rdint(10);
		if( (c=nextch()) != '.' ) break;
	}
	
	unrdch(c);
	
	switch( i )
	{
	case 1: addr = bit[0]; break;
	case 2: addr = bit[0] | swap_long(bit[1]<<8,0); break;
	case 3: addr = bit[0] | (bit[1]<<8) | (int)swap_short((short)bit[2],0); break;
	case 4: addr = bit[0] | (bit[1]<<8) | (bit[2]<<16) | (bit[3]<<24); break;
	}

	return addr;
}

static char *rdstr(char *s, int term)
{
	int c = nextch();
	bool defterm = (term==' '||term==0);

	while( defterm?(c!=' ' && c!='\t' && c!='\n'):(c!=term) )
	{
		*s++ = c;
		c = rdch();
	}
	*s++ = 0;

	unrdch(c);
	return s;
}

extern int scandb(char *format, ... )
{
	va_list a;
	int match;
	char *f;
	int c = 0;	
	char *s, *t;
	
	CHECKSIGS();
	for( c = nextch(); c > 0; c = nextch())
	{
		bool matched = TRUE;
		char **matchstr = NULL;
		char *realname = NULL;
		
		s = dbinfo->strbuf;
		
		if( c == '\n' ) continue;
		unrdch(c);

		va_start(a,format);
		for( f = format; *f ; f++ )
		{
			int val;
			int arg;
			int base = 10;
			bool list = FALSE;
			switch( *f )
			{
			case '!': match = EXACT; break;
			case '?': match = WILD; break;
			case '%': match = ASSIGN; break;
			case '$': match = IGNORE; break;
			case '#': match = ALIAS; break;
			
			case '\\': f++;
			default:
				c = rdch();
				if( *f == ' ') {unrdch(c);unrdch(nextch());}
				elif( *f != c ) return 100;
				continue;
			}
			
			if( f[1] == 'l' ) list = TRUE,f++;

			switch( *++f )
			{
			case 'a':
				val = (int)rd_inet();
				arg = va_arg(a,int);
				goto matchint;
								
			case 'x': base = 16;
			case 'd':
				val = rdint(base);
				arg = va_arg(a,int);
			matchint:
				switch( match ) 
				{
				case WILD:
					if( val == 0 ) break;
				case ALIAS:
				case EXACT:
					if( val != arg ) goto nextline;
					break;
					
				case ASSIGN:
					*(int *)arg = val;
					break;
					
				case IGNORE:
					break;
				}
				break;
				
			case 's':
			
				switch( match )
				{
				case WILD:
					c = nextch();
					if( c == '*' ) break;
					unrdch(c);
				case ALIAS:
				case EXACT:
				{
					char **tt;
					tt = va_arg(a,char **);
					t = rdstr(s,f[1]);
					if( strcmp(s,*tt) != 0 ) 
					{
						if( match != ALIAS ) goto nextline;
						realname = s;
						s = t;
						matchstr = tt;
						matched = FALSE;
					}
					break;
				}
					
				case ASSIGN:
					if( list )
					{
						char ***ava = va_arg(a,char ***);
						char **av = dbinfo->list;
						*ava = av;

						for(;;)
						{
							t = rdstr(s,f[1]);
							if( t == s+1 ) break;

							if( matchstr!=NULL && 
							    !matched   &&
							    (strcmp(s,*matchstr)==0))
							{
								matched = TRUE;
								*matchstr = realname;
							}
							*av++ = s;
							s = t;
						}
						*av = 0;
						
						va_end(a);

						if( matched ) return 0;
						goto nextline;
					}
					else
					{
						char **tt = va_arg(a,char **);
						*tt = s;
						t = rdstr(s,f[1]);
						s = t;
						break;
					}
				
				case IGNORE:
					rdstr(s,f[1]);
					break;					
				}
				break;
			} /* end of switch */

		} /* end of format loop */
		
		/* if we get this far all the matches have succeeded */
		
		va_end(a);
		return matched?0:100;
		
	nextline:
		for( c = rdch(); c > 0 && c != '\n'; c = rdch() );
	} /* end of line loop */

	va_end(a);
	CHECKSIGS();
	return 100;
}

