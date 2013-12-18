
	/*	@(#) trans.h		T3000-1.2 3/15/89	*/

/*
 *	TELMAT Informatique	(C) 1988
 */

#define LINT_ARGS 1       /* Enable Compiler Parameter Checking */
#define LINK_READ_OFFSET	0x00	/* registers offsets on ITFTP32 */
#define LINK_WRITE_OFFSET       0x04
#define LINK_OUT_STATUS_OFFSET  0x08
#define LINK_IN_STATUS_OFFSET   0x0C
#define LINK_STATUS_OFFSET    	0x80
#define LINK_STATUS_IT_OFFSET  	0x81
#define LINK_RESET_IT0_OFFSET 	0xC0
#define LINK_RESET_IT1_OFFSET 	0xC8

#define D0	0x01			/* DATA bits */
#define D1	0x02
#define D2	0x04
#define D3	0x08
#define D4	0x10
#define D5	0x20
#define D6	0x40
#define D7	0x80
#define D8	0x100
#define D9	0x200

#define MAXLNK	5
#define RESET_COUNT              20000	/* 10000 DG */ /* delay to reset */
#define ANALYSE_COUNT            20000	/* 10000 DG */ /* delay to set analyse */
#define MAX_LINK_TIME            64000	/* 32000 DG */ /* delay to read on link adapter */
/* ecriture */
#define LNDEFTRY	20	/* max 20 ticks */
#define LNSUPW		20	/* 20 essais */
/* */
#define PRITRANS	26
struct LINK_RW {
		unsigned char	link_read;
		unsigned char	bour1[3];
		unsigned char	link_write;
		unsigned char	bour2[3];
		unsigned char	link_out_status;
		unsigned char	bour3[3];
		unsigned char	link_in_status;
		unsigned char	bour4[0x73];
		unsigned char	link_status;
		unsigned char	link_status_it;
		unsigned char	bour5[0x3E];
		unsigned char	link_reset_it0;
		unsigned char	bour6[7];
		unsigned char	link_reset_it1;
};

struct link {
	unsigned short  ln_state;
	char		ln_error;
	struct LINK_RW *ln_rw;
	unsigned int    ln_ctimeout;	/* current timeout (0 = no timeout) */
	int		ln_nbtry;	/* for write */
	struct	proc   *ln_rsel;
	struct	proc   *ln_wsel;
} link[];

#define LNOPEN		0x1
#define LNAERROR	0x2
#define LNSUP		0x4
#define LNRSLEEP	0x8	/* sleep on read */
#define LNWSLEEP	0x10	/* sleep on write */
#define	LNRCOLL		0x20	/* collision in read select */
#define	LNWCOLL		0x40	/* collision in write select */
#define LNABORT		0x80	/* timeout occurs when sup  */
#define LNSSLEEP	0x100	/* sleep on select */
#define LNERROR		0x200	/* transputer error */

#define	RESET_ANALYSE	(('l'<<8)|1)
#define	LNRESET_ANALYSE	(('l'<<8)|1)
#define RESET_ROOT	(('l'<<8)|2)
#define LNRESET_ROOT	(('l'<<8)|2)
#define ERROR_ON	(('l'<<8)|3)
#define LNERROR_ON	(('l'<<8)|3)
#define ERROR_OFF	(('l'<<8)|4)
#define LNERROR_OFF	(('l'<<8)|4)
#define LNSETTIMEOUT	(('l'<<8)|5)
#define LNGETTIMEOUT	(('l'<<8)|6)
#define LNSETNBTRY	(('l'<<8)|7)
#define LNGETNBTRY	(('l'<<8)|8)
#define LNTESTREAD	(('l'<<8)|9)
#define LNTESTWRITE	(('l'<<8)|10)
#define LNTESTERROR	(('l'<<8)|11)



