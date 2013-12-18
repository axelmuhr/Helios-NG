/*
 *
 * Copyright (c) 1988, 1989, 1990, 1991 by Perihelion Software Ltd.
 * 
 * All rights reserved
 *
 */

/* header for use of transputer bytblt call */

#ifndef BYTBLT_H
#define BYTBLT_H

#ifdef __TRAN

#ifndef __nonansi_h
#include <nonansi.h>
#endif

extern int	_operate( int op, ... );

#define T8MOVE( dest, source, len )	_operate( 0x4A, len, dest, source )

#define bytblt1d_( src, dst, width )    T8MOVE( dst, src, width )

#define bytbltall_( src, dst, srcstride, dststride, height, width ) \
	_operate( 0x5b, height, dststride, srcstride ),	\
	_operate( 0x5c, width, dst, src )

#define bytbltnonzero_( src, dst, srcstride, dststride, height, width ) \
	_operate( 0x5b, height, dststride, srcstride ),	\
	_operate( 0x5d, width, dst, src )

#define bytbltzero_( src, dst, srcstride, dststride, height, width )	\
	_operate( 0x5b, height, dststride, srcstride ),	\
	_operate( 0x5e, width, dst, src )

#else
#define T8MOVE( dest, source, len )	memmove( dest, source, len )

#define bytblt1d_( src, dst, width )    T8MOVE( dst, src, width )

#endif

#endif /* BYTBLT_H */
