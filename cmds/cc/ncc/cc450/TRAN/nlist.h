/* @(#)19	1.7  com/inc/sys/nlist.h, bos, bos320 6/16/90 00:33:06 */
/*	com/inc/sys/nlist.h, bos, bos320 - 90/06/16 - 00:33:06  	*/
/*
 * COMPONENT_NAME: SYSPROC 
 *
 * ORIGIN: 3, 27 
 *
 * Copyright International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */
#ifndef	_H_NLIST 
#define _H_NLIST	1

/********************************************************************** 
 *
 *	COFF Extended Object File Format:
 *		nlist.h
 *	Derived from AT&T UNIX System V Release 2.0 COFF
 *
 *	Structure similar to original COFF
 *
 *********************************************************************/

/* symbol table entry structure */

struct nlist
{
	union
	{
		char	*_n_name;	/* symbol name */
	} _n;
	long		n_value;	/* value of symbol */
	short		n_scnum;	/* section number */
	unsigned short	n_type;		/* type and derived type */
	char		n_sclass;	/* storage class */
	char		n_numaux;	/* number of aux. entries */
};

#ifndef	n_name
#define	n_name		_n._n_name
#endif	/* n_name */

#endif /* _H_NLIST */
