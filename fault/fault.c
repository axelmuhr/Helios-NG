/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- fault.c								--
--                                                                      --
--	Library to extract error from faults database.			--
--                                                                      --
--	Author:  NHG 7/3/89						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 	%W%	%G% Copyright (C) 1989, Perihelion Software Ltd.*/


#include <helios.h>	/* standard header */

#define __in_fault 1	/* flag that we are in this module */

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <string.h>
#include <codes.h>
#include <syslib.h>
#include <errno.h>
#include <fault.h>

static char *skipname(char *p);
static char *skipspace(char *p);
static int strtonum(char *s);
static void rdline(FDB *fdb, char *buf);
static int rdch(FDB *fdb);
static void addhex(char *text, word val);
static word addint(char *s, word i);

/*--------------------------------------------------------
-- fdbopen						--
--							--
-- Open faults database					--
--							--
--------------------------------------------------------*/

extern FDB *fdbopen(string name)
{
	Object *o = NULL;
	Stream *s = NULL;

	FDB *fdb = New(FDB);

	if( fdb == NULL ) return NULL;

	fdb->pos = 0;	
	fdb->upb = 0;	
	
	o = Locate(NULL,name);
	
	if( o == NULL ) goto fail;
	
	s = Open(o,NULL,O_ReadOnly);
	
	if( s == NULL ) goto fail;
	
	fdb->stream = s;
	
	Close(o);
	
	return fdb;
fail:
	if( o != NULL ) Close(o);
	if( s != NULL ) Close(s);
	if( fdb != NULL ) Free(fdb);
	return NULL;
}

/*--------------------------------------------------------
-- fdbclose						--
--							--
-- Close faults database				--
--							--
--------------------------------------------------------*/

extern void fdbclose(FDB *fdb)
{
	Close(fdb->stream);
	Free(fdb);
}

/*--------------------------------------------------------
-- fdbrewind						--
--							--
-- Rewind faults database				--
--							--
--------------------------------------------------------*/

extern void fdbrewind(FDB *fdb)
{
	fdb->pos = 0;
	fdb->upb = 0;
	Seek(fdb->stream,0,0);
}

/*--------------------------------------------------------
-- fdbfind						--
--							--
-- Search faults database for a given code		--
--							--
--------------------------------------------------------*/

extern int fdbfind(FDB *fdb, char *Class, word code, char *text, word tsize)
{
	char *line = (char *)Malloc(100);
	char *codename, *codenum, *codemsg, *p;
	char *Classname, *Classmask, *Classpfix;
	bool foundclass = FALSE;
	int codeval, Classbits = 0;

	forever
	{
		rdline(fdb,line);

		switch( line[0] )
		{
		case '\0':		/* EOF		*/
			goto nomsg;
		case '\n':		/* blank line 	*/
		case '#':		/* comment	*/
			continue;
		case '!':		/* Class name	*/
			if( foundclass ) goto nomsg;
			if( line[1] == '!' ) continue;
			
			p = skipspace(line+1);
			Classname = p;
			p = skipname(p);
			p = skipspace(p);
			Classmask = p;
			p = skipname(p);
			p = skipspace(p);
			Classpfix = p;
			p = skipname(p);
			Classbits = strtonum(Classmask);
			
			if( strcmp(Classname,Class) == 0 ) foundclass = TRUE;

			break;
			
		default:		/* code line	*/
			if( !foundclass ) continue;

			p = skipspace(line);
			codename = p;
			p = skipname(p);
			p = skipspace(p);
			codenum = p;
			p = skipname(p);
			p = skipspace(p);
			codemsg = p;
			while( *p != 0 && *p != '\n' ) p++;
			*p = 0;

			codeval = strtonum(codenum);

			if( codeval == (code & Classbits) )
			{
				if( codemsg[0] == 0 ) codemsg = codename;
				if( (word)strlen(codemsg) + (word)strlen(text) < tsize )
				{
					strcat(text,codemsg);
				}
				Free(line);
				return strlen(text);
			}
			
		}
	}	

nomsg:
	strcpy(line,"<");
	strcat(line,Class);
	strcat(line,": 0x");
	addhex(line,code & Classbits);
	strcat(line,">");

	if( (word)strlen(line) + (word)strlen(text) < tsize ) strcat(text,line);
	
	Free(line);
	return strlen(text);
}

/*--------------------------------------------------------
-- Fault						--
--							--
-- Decode Helios error/function code			--
--							--
--------------------------------------------------------*/

extern void Fault(word code, char *msg, word msize)
{
	FDB *fdb;

	fdb = fdbopen("/helios/etc/faults");
	
	if( fdb == NULL ) 
	{
		strcpy(msg,"Cannot open fault database");
		return;
	}
	
	if( code < 0 )
	{
		strcpy(msg,"From ");
		fdbfind(fdb,"SubSystem",code,msg,msize);
		strcat(msg,": ");
		fdbfind(fdb,"ErrorClass",code,msg,msize); strcat(msg,", ");
		fdbfind(fdb,"GeneralError",code,msg,msize); strcat(msg," ");
		if( code&0x8000 ) fdbfind(fdb,"ObjectCode",code,msg,msize);
		else {
			if( (code&EG_Mask) == EG_Exception )
			{
				if( (code & 0x0000ff00) == EE_Signal )
				{
					strcat(msg,"Signal: ");
					fdbfind(fdb,"Signal",code,msg,msize);
				}
				else fdbfind(fdb,"Exception",code,msg,msize);
			}
			else
			{
				word c = code&0xffff;
				if( c == 0 ) strcat(msg,"0");
				else addint(msg,c);
				if( (code&EG_Mask) == EG_CallBack )
					strcat(msg," secs");
			}
		}
	}
	else
	{
		if( code <= MAX_PERROR )
		{
			strcpy(msg,"Posix error: ");
			fdbfind(fdb,"Posix",code,msg,msize);
		}
		else
		{
			strcpy(msg,"Function for ");
			fdbfind(fdb,"SubSystem",code,msg,msize);
			strcat(msg,": ");
			fdbfind(fdb,"FunctionClass",code,msg,msize); strcat(msg," ");
			fdbfind(fdb,"GeneralFunction",code,msg,msize); strcat(msg," ");
			if ( (code & 0xf) ) fdbfind(fdb,"SubFunction",code,msg,msize);
		}
	}

	fdbclose(fdb);
}

/*--------------------------------------------------------
-- perror						--
--							--
-- get message for posix error code.			--
--							--
--------------------------------------------------------*/
/*--------------------------------------------------------
--							--
-- Support routines					--
--							--
--------------------------------------------------------*/


static char *skipname(char *p)
{
	while( *p != ' ' && *p != '\t' && *p != '\n' && *p != 0 ) p++;
	if( *p == 0 ) return p;
	*p++ = 0;
	return p;
}

static char *skipspace(char *p)
{
	while( *p == ' ' || *p == '\t' ) p++;
	return p;
}

static int strtonum(char *s)
{
	int radix = 10;
	int val = 0;
	
	while( *s )
	{
		char c = *s++;
		int digit = 0;
		
		if( c == 'x' ) { radix = 16; continue; }
		
		if( '0' <= c && c <= '9' ) digit = c - '0';
		elif( radix == 16 && ('a' <= c && c <= 'f') ) digit = c - 'a' + 10;
		elif( radix == 16 && ('F' <= c && c <= 'F') ) digit = c - 'A' + 10;

		val = val * radix + digit;
	}
	
	return val;
}

static word addint(char *s, word i)
{	
	word len;

	if( i == 0 ) return strlen(s);

	len = addint(s,i/10);
  
	s[len] = (char)(i%10) + '0';
  
	s[len+1] = '\0';

	return len+1;
}

static void addhex(char *text, word val)
{
	word i;
	char *digits = "0123456789abcdef";
	text += strlen(text);
	for( i = 7; i >= 0; i-- )
	{
		word d = (val>>(i*4))&0xf;
		*text++ = digits[d];
	}
	*text = 0;
}

static void rdline(FDB *fdb, char *buf)
{
	forever
	{
		int c = rdch(fdb);
		if( c == -1 )
		{
			break;
		}
		if( c == '\r' ) continue;
		*buf++ = c;
		if( c == '\n' ) break;	
	}
	*buf = 0;
}

static int rdch(FDB *fdb)
{
	if( fdb->upb == -1 ) return -1;
	if( fdb->pos == fdb->upb )
	{
		word size = Read(fdb->stream,fdb->buf,FDBBUFMAX,-1);
		fdb->upb = size;
		if( size == -1 ) 
		{
			return -1;
		}
		fdb->pos = 0;
	}
	return fdb->buf[fdb->pos++];
}

/* -- End of fault.c */
