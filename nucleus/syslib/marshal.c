/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--            Copyright (C) 1987,1990, Perihelion Software Ltd.         --
--                        All Rights Reserved.                          --
--                                                                      --
-- syslib/marshal.c							--
--                                                                      --
--	Parameter marshalling routines plus a few related(ish) things.	--
--									--
--                                                                      --
--	Author:  NHG 16/8/87						--
--		 NHG 03/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: marshal.c,v 1.6 1993/07/09 13:36:47 nickc Exp $ */

#define _in_XXXX

#include "sys.h"

Capability DefaultCap = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static unsigned getbit(char c, string bitchars);

/*--------------------------------------------------------
-- InitMCB						--
--							--
-- Initialise an MCB in preparation for Marshalling.	--
-- 							--
--------------------------------------------------------*/

PUBLIC void InitMCB(MCB *mcb, byte flags, Port dest, Port reply, word fnrc)
{
	*(word *)mcb		= 0;
	mcb->MsgHdr.Flags	= flags;
	mcb->MsgHdr.Dest	= dest;
	mcb->MsgHdr.Reply	= reply;
	mcb->MsgHdr.FnRc	= fnrc;
	mcb->Timeout		= IOCTimeout;
}

/*--------------------------------------------------------
-- Marshalxxxx						--
--							--
-- Routines for marshalling various types of object	--
-- into a message.					--
-- 							--
--------------------------------------------------------*/

PUBLIC void
MarshalString(
	      MCB *	mcb,
	      string	str )
{
	word offset = mcb->MsgHdr.DataSize;
	word stlen = 0;
	word csize = mcb->MsgHdr.ContSize;
	byte *t;
	if( str == (string)NULL || *str == '\0' )
	{
		MarshalWord(mcb,-1);
		return;
	}
	for( t = &mcb->Data[offset] ; *str ; *t++ = *str++, stlen++ );
	*t = '\0';
	mcb->Control[csize]   = offset;
	mcb->MsgHdr.ContSize  = (unsigned char) (csize + 1);
	mcb->MsgHdr.DataSize += (unsigned short)(stlen + 1);
}

PUBLIC void MarshalData(MCB *mcb, word size, byte *data)
{
	word offset = mcb->MsgHdr.DataSize;
/*	memcpy(&mcb->Data[offset],data,size); */
	{ word i = size;
	  byte *t = &mcb->Data[offset];
	  while( i-- ) *t++ = *data++;
	}
	mcb->MsgHdr.DataSize += (unsigned short)size;
}

PUBLIC void
MarshalWord(
	    MCB *	mcb,
	    word	wd )
{
	word csize = mcb->MsgHdr.ContSize;
	mcb->Control[csize]  = wd;
	mcb->MsgHdr.ContSize = (unsigned char)(csize + 1);
}

PUBLIC void
MarshalOffset( MCB * mcb )
{
	word csize = mcb->MsgHdr.ContSize;
	word hdr = *((word *)mcb);
	word offset = hdr & 0xFFFF;
	offset = (offset+3) & ~3;
	mcb->Control[csize] = offset;
	*((word *)mcb) = (hdr & ~0xffff) + offset;
	mcb->MsgHdr.ContSize = (unsigned char)(csize + 1);
}

PUBLIC void
MarshalCap(
	   MCB *	mcb,
	   Capability *	cap )
{
	MarshalWord(mcb,((word *)cap)[0]);
	MarshalWord(mcb,((word *)cap)[1]);	
}

PUBLIC void MarshalDate(MCB *mcb, Date date)
{
	MarshalWord(mcb,(word)date);
}

PUBLIC void
MarshalCommon(
	      MCB *	mcb,
	      Object *	object,
	      string	name )
{
	if( name != NULL && *name == '@' ) 
	{
		/* the name string contains an encoded capability	*/
		/* treat this as the Object				*/
		Capability cap;
		name = EncodeCapability(name,&cap);
		MarshalString(mcb,name);
		MarshalWord(mcb,-1L);
		MarshalWord(mcb,1L);
		MarshalCap(mcb,&cap);
	}
	elif( object == Null(Object) || 
		( name != NULL && *name == c_dirchar ) )
	{
		/* If there is no context object, or the path name is 	*/
		/* absolute do not send a context string or a capability*/
		MarshalWord(mcb,-1L);		/* no context string	*/
		MarshalString(mcb,name);	/* set pathname		*/
		MarshalWord(mcb,1L);		/* start past '/'	*/
		MarshalCap(mcb,&DefaultCap);	/* default capability	*/
	}
	else
	{
		MarshalString(mcb,object->Name);
		MarshalString(mcb,name);
		MarshalWord(mcb,1L);
		MarshalCap(mcb,&object->Access);
	}
}

PUBLIC void MarshalObject(MCB *m, Object *o)
{
	string name = o->Name;
	MarshalOffset(m);
	MarshalData(m,sizeof(Capability),(byte *)&o->Access);
	MarshalData(m,(word)strlen(name)+1,name);
}

PUBLIC void MarshalStream(MCB *m, Stream *s)
{
	string name = s->Name;
	word mode = s->Flags;		/*  & Flags_Mode; */
	MarshalOffset(m);
	MarshalData(m,sizeof(word),(byte *)&mode);
	MarshalData(m,sizeof(word),(byte *)&s->Pos);
	MarshalData(m,sizeof(Capability),(byte *)&s->Access);
	MarshalData(m,(word)strlen(name)+1,name);
}

/*--------------------------------------------------------
-- DefaultCapability					--
--							--
-- Set/Get default Capability				--
--							--
--------------------------------------------------------*/

void DefaultCapability(Capability *New, Capability *old)
{
	if( old != NULL ) *old = DefaultCap;

	if( New != NULL ) DefaultCap = *New;
}

/*--------------------------------------------------------
-- EncodeMatrix						--
--							--
-- Encode an access matrix from a string of chars.	--
--							--
--------------------------------------------------------*/

Matrix EncodeMatrix(string s, word type)
{
	Matrix matrix = 0;
	AccMask mask = 0;
	word shift = 0;
	string bitchars = getbitchars(type);

	forever
	{
		if( *s == '\0' || *s == c_matchar )
		{
			matrix |= (word)mask<<shift;
			mask = 0;
			shift += 8;
			if( *s == '\0' ) return matrix;
		}
		else mask |= getbit(*s,bitchars);
		s++;
	}

	return 0;
}

static unsigned getbit(char c, string bitchars)
{
	unsigned bit = 0;
	while( *bitchars )
		if( c == *bitchars ) return 1<<bit;
		else bitchars++,bit++;
	return 0;
}

string getbitchars(word type)
{
	switch( type & Type_Flags )
	{
	case 0:
	case Type_Directory:	return DirChars;
	default:
	case Type_Stream:	return FileChars;
	}
}

/*--------------------------------------------------------
-- DecodeMask						--
--							--
-- Decode the access mask and generate a string of	--
-- chars.						--
--							--
--------------------------------------------------------*/

string DecodeMask(string s, AccMask mask, string bitchars)
{
	int i;
	
	for( i = 0; i < 8 ; i++ )
		if( mask & (1<<i) ) *s++ = bitchars[i];
		else *s++ = '-';

	return s;
}

/*--------------------------------------------------------
-- DecodeMatrix						--
--							--
-- Decode the access matrix and generate an appropriate	--
-- string of chars.					--
--							--
--------------------------------------------------------*/

PUBLIC void DecodeMatrix(string s, Matrix matrix, word type)
{
	string bitchars = getbitchars(type);

	s = DecodeMask(s, (AccMask)matrix, bitchars);
	*s++ = c_matchar;
	s = DecodeMask(s, (AccMask)(matrix>>8), bitchars);
	*s++ = c_matchar;
	s = DecodeMask(s, (AccMask)(matrix>>16), bitchars);
	*s++ = c_matchar;
	s = DecodeMask(s, (AccMask)(matrix>>24), bitchars);
	*s++ = '\0';
}

/*--------------------------------------------------------
-- DecodeCapability					--
-- EncodeCapability					--
--							--
-- Encode and decode capabilities stored as strings	--
--							--
--------------------------------------------------------*/

char *DecodeCapability(char *s, Capability *cap)
{
	int i;
	char *c = (char *)cap;
	
	*s++ = '@';

	for( i = 0; i < 8; i++ )
	{
		*s++ = (c[i]&0xf) + 'a';
		*s++ = (c[i]>>4) + 'a';
	}

	*s = '\0';

	return s;
}

char *EncodeCapability(char *s,Capability *cap)
{
	int i;
	char *c = (char *)cap;
	
	s++;	/* skip '@' */

	for( i = 0; i < 8; i++ )
	{
		char x = *s++ - 'a';
		c[i] = 	x | ( (*s++ - 'a') << 4 );
	}

	return s;
}

/*--------------------------------------------------------
-- splitname						--
--							--
-- Copy chars out of str into pfix until either a null 	--
-- is found or ch is encountered. Returns pos in str 	--
-- of char after ch. This is used for splitting up file	--
-- names.						--
-- 							--
--------------------------------------------------------*/
int splitname(char *pfx, char ch, char *str)
{
	int ptr = 0;
	int pptr = 0;
	while( str[ptr] != ch )
	{
		if( str[ptr] == '\0' ) { pfx[pptr] = '\0'; return ptr; }
		pfx[pptr] = str[ptr];
		if( pptr < NameMax-1 ) pptr++;
		ptr++;
	}
	pfx[pptr] = '\0';
	return ptr+1;
}

/* end of marshal.c */
