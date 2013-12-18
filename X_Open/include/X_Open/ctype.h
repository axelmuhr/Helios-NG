/******************************************************************************
**
**
**	FILE		:	X_Open/ctype.h
**
**
**	DESCRIPTION	:	X-Open header file : <ctype.h>
**				already exists under Helios
**
*****************************************************************************/


#define	_toupper(c)	((c) - 'a' + 'A')
#define	_tolower(c)	((c) - 'A' + 'a')


extern	int toascii(int c);
extern	int isascii(int c);

