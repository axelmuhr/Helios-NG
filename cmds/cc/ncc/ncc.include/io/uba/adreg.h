
/* @(#)adreg.h	1.2	(ULTRIX)	1/30/89	*/
/*	adreg.h	6.1	83/07/29	*/
/*	adreg.h		modified for 2752 -- Robert Chesler */

struct addevice
{
	short int ad_csr;			/* Control status register */
	short int ad_data;			/* Data Buffer Register */
	short int ad_wcr;			/* DMA Word Count Register */
	short int ad_car;			/* DMA Current Address Reg */
};
#define ad_dmaext ad_data			/* DMA Extension Register */

#define AD_CHAN		ADIOSCHAN
#define AD_READ		ADIOGETW
#define	ADIOSCHAN	_IOW('a', 0, int)		/* set channel */
#define	ADIOGETW	_IOR('a', 1, int)		/* read one word */

/*
 * Unibus CSR register bits
 */

#define AD_START		01		/* bit 0  */
#define AD_DMA_MODE		02		/* bit 1  */
#define AD_DMA_MULT		04		/* bit 2  */
#define AD_AINC			010		/* bit 3  */
#define AD_CLOCK		020		/* bit 4  */
#define AD_DMA_DONE		040		/* bit 5  */
#define AD_IENABLE		0100		/* bit 6  */
#define AD_DONE 		0200		/* bit 7  */
/*	Channel				bits 	 8 - 11	  */
/*	Gain Select 1,2,4,8		bits 	12 - 13	  */
#define AD_ERRINT		0400		/* bit 14 */
#define AD_ERROR		01000		/* bit 15 */
