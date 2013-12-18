/*
 *
 * Copyright (c) 1988, 1989, 1990 by Perihelion Software Ltd.
 * 
 * All rights reserved.
 */

/*
 * quick implimentaion of the database package provided by UNIX
 * (see DBM(3X) )
 *
 * NC 14/04/88
 */

#ifndef _dbm_h
#define _dbm_h

typedef struct _datum
{
  unsigned char	*	dptr;
  int			dsize;
} datum;

extern int		dbminit(  char * file_name );
extern datum		fetch(    datum key );
extern int		store(    datum key, datum contents );
extern int		delete(   datum key );
extern datum		firstkey( void );
extern datum		nextkey(  datum key );
extern int		dbmclose( void );

#endif /* _dbm_h */
