/************************************************************************/
/*                                                                      */
/* File: xplib.c                                                        */
/*                                                                      */
/* Changes:                                                             */
/*      NHG  18-May-87  : Created                                       */
/*                                                                      */
/* Description:                                                         */
/*      Library of procedures to interface to a Kuma Transputer Box     */
/*                                                                      */
/*                                                                      */
/* Copyright (c) 1987, Perihelion Software Ltd. All Rights Reserved.    */
/************************************************************************/

#include <helios.h>
#include <link.h>
#include <config.h>
#include <codes.h>

void xpanalyse();
WORD xprdword();
WORD xprdint();
WORD linkno = -1;
WORD WordSize = 4;

LinkInfo linkinfo;

bool Parsytec = TRUE;

bool linkdead = FALSE;

WORD xpinit(int l, int isT2)
{
	word e;
        LinkConf c;

	linkno = l;
	
	if( isT2 ) WordSize = 2;
	
	LinkData(l,&linkinfo);
		
        c.Id = linkno;
        c.Mode = Link_Mode_Dumb;
        c.State = 0;
        c.Flags = 0;
        
        if((e = Configure(c)) != Err_Null ) error("cannot reconfigure link %d: %x",linkno,e);
        
        if((e = AllocLink(linkno)) != Err_Null ) error("cannot allocate link %d: %x",linkno,e);
}

WORD xptidy()
{
	word e;
	LinkConf c = *(LinkConf *)&linkinfo;

	if( linkno == -1 ) return;
		
        if((e = FreeLink(linkno)) != Err_Null ) error("cannot free link %d: %x",linkno,e);
        
        if((e = Configure(c)) != Err_Null ) error("cannot reconfigure link %d: %x",linkno,e);       
}

WORD dbrdword(address)
WORD address;
{
	if( linkno == -1 ) return *(word *)address;
        xpwrbyte(1L);
        xpwrint(address);        /* byteswap the address */
        return xprdword();
}

WORD dbrdint(address)
WORD address;
{
	if( linkno == -1 ) return *(word *)address;
        xpwrbyte(1L);
        xpwrint(address);        /* byteswap the address */
        return xprdint();
}

void dbwrword(address,data)
WORD address,data;
{
	if( linkno == -1 ) { *(word *)address = data; return; }
	
        xpwrbyte(0L);
        xpwrint(address);        /* bytespap the address */
        xpwrword(data);          /* but not the data */
}

void dbwrint(address,data)
WORD address,data;
{
	if( linkno == -1 ) { *(word *)address = data; return; }	
	
        xpwrbyte(0L);
        xpwrint(address);        /* bytespap the address */
        xpwrint(data);           /* and the data */
}

xpwrbyte(b)
WORD b;
{
	word e;
	if( linkno == -1 || linkdead ) return;
	e = LinkOut(1,linkno,&b,OneSec);
	if( e != 0 ) linkdead = TRUE;
if( e != 0 ) IOdebug("LinkOut failed: %x",e);
}

WORD xpwrrdy()
{
        return TRUE;
}

WORD xprdbyte()
{
	word e;
	word b = 0;
	if( linkno == -1 || linkdead ) return 0;
	e = LinkIn(1,linkno,&b,OneSec);
	if( e != 0 ) linkdead = TRUE;
if( e != 0 ) IOdebug("LinkIn %d failed: %x",linkno,e);	
	return b;
}

WORD xprdrdy()
{
        return TRUE;
}


xpwrword(data)
WORD data;
{
	word e;
	if( linkno == -1 || linkdead ) return;
	e = LinkOut(WordSize,linkno,&data,OneSec);	
	if( e != 0 ) linkdead = TRUE;
if( e != 0 ) IOdebug("LinkOut %d failed: %x",linkno,e);	
}

WORD xprdword()
{
	word e;
        WORD data = 0;
       	if( linkno == -1 || linkdead ) return 0;
	e = LinkIn(WordSize,linkno,&data,OneSec);
	if( e != 0 ) linkdead = TRUE;
if( e != 0 ) IOdebug("LinkIn %d failed: %x",linkno,e);	
        return data;
}

xpwrint(data)
WORD data;
{
	if( linkno == -1 || linkdead ) return;
        xpwrword(data);
}

WORD xprdint()
{
	if( linkno == -1 || linkdead ) return 0;
	return xprdword();
}

void xpwrdata(buf,size)
UBYTE *buf;
WORD size;
{
	word e;
	if( linkno == -1 || linkdead ) return;
        e = LinkOut(size,linkno,buf,OneSec);
	if( e != 0 ) linkdead = TRUE;
if( e != 0 ) IOdebug("LinkOut %d failed: %x",linkno,e);        
}

void xpreset()
{
	word e;
	if( linkno == -1 ) return;
	xpanalyse();
#if 0
	e = SoftReset( linkno );
if( e != 0 ) IOdebug("SoftReset %d failed: %x",linkno,e);        	
	return;
#endif
}

void xpanalyse()
{
	if( Parsytec )
	{
		static WORD link[4] = { 1, 2, 4, 8 };
		WORD	*reset = 0x000000C0;

		if( linkno == -1 ) return;

IOdebug("pa_reset %d",linkno);	
		*reset = 0;
		*reset = 1;
		*reset = 2;
		*reset = 3;
	
		*reset = link[linkno];
	
		Delay( 1000 );
	
		*reset = 0;
	}
	linkdead = FALSE;
	return;
}

/*  -- End of xplib.c -- */
