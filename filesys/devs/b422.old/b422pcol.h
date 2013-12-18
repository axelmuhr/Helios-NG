

	/****************************************************************/
	/*								*/
	/* b422pcol.h	Protocol definitions for B422 INMOS SCSI TRAM.	*/
	/*		Copyright (C) INMOS Limited 1990.		*/
	/*								*/
	/* Date		Ver	Author		Comment			*/
	/* ----		---	------		-------			*/
	/* 20-Mar-90	1.0	Mike Burrow	Original		*/
	/*								*/
	/****************************************************************/


/* Definition of occam-like types */
/**********************************/

typedef short	INT16;
typedef int	INT32;
typedef int	INT64; /* not 64 bits */
typedef byte	BOOL;


/* SCSI command arrays */
/***********************/

typedef	BYTE	SCSI_CMND_6[SCSI_6];
typedef BYTE	SCSI_CMND_10[SCSI_10];
typedef BYTE	NULL_BUFFER[NULL_SIZE];


void output_pcol(BYTE, ...);
void input_tag(BYTE, BYTE *);
void input_pcol(BYTE, BYTE, ...);

int  b422_init(WORD);
void b422_reset(WORD, BYTE);
void b422_term(WORD);

WORD squirt(BYTE *, WORD);

BYTE getbyte(INT32, BYTE);

