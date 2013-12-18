/******************************************************************************
**
**
**	FILE		:	X_Open/misc.h
**
**
**	DESCRIPTION	:	For X-Open functions not in header files
**
**
******************************************************************************/



extern	char *crypt(char *key, char *salt);
extern	void setkey(char *key);
extern	void encrypt(char *block, int edflag); 
		/* WARNING Max Size of block 64 characters */

extern	void swab(char *src, char *dest, int nbytes);
