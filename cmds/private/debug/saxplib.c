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

WORD xprdword();
WORD xprdint();

WORD dbrdword(address)
WORD address;
{
	return *(word *)address;
}

WORD dbrdint(address)
WORD address;
{
        return *(word *)address;
}

void dbwrword(address,data)
WORD address,data;
{
	*(word *)address = data;
}

void dbwrint(address,data)
WORD address,data;
{
	*(word *)address = data;
}

xpwrbyte(b)
WORD b;
{
}

WORD xpwrrdy()
{
	return TRUE;
}

WORD xprdbyte()
{
	return 0;
}

WORD xprdrdy()
{
        return TRUE;
}


xpwrword(data)
WORD data;
{
	return;
}

WORD xprdword()
{
        return 0;
}

xpwrint(data)
WORD data;
{

}

WORD xprdint()
{
        WORD data = 0;
        return data;
}

void xpwrdata(buf,size)
UBYTE *buf;
WORD size;
{
}

void xpreset()
{

}

void xpanalyse()
{

}

/*  -- End of xplib.c -- */
