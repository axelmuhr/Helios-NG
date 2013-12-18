/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1990
*******************************************************************************
******************************************************************************/

/************************************************************************

   DIAGDEFS.H

*************************************************************************/

#define	INFO_PALETTE	05
#define	RETRY_PALETTE	06

#define	MARKETING 	TRUE

#define	FULL_TEST	1

#define DISPLAY   	TRUE
#define NO_DISPLAY	FALSE

#define	CONTENTION_OFFSET	0x07F0
 
#define	TX_THROTTLE_MAX	9999
 
/* I/O space definitions */
#define	STRT_BIO	0x200			/* first available base io */
#define	LAST_BIO	0x3e0			/* last available base io */
#define	DEF_BIO		0x280			/* default base io address */
#define	IO_LEN		0x20			/* length of io addrs in card */

/* System type definitions */
#define	E_TYP		1			/* 8003E type board */
#define	EA_TYP		2			/* 8003EA type (micro-chan) */
#define	EB_TYP		3			/* 8003EB type */
#define	INT_TYP		4			/* 8023E type (intelligent) */

/* LAN type definitions */
#define	STAR_TYPE	0			/* ethernet type */
#define	ETHER_TYPE	1			/* starlan type */
#define	INT_ETHER_TYPE	2			/* intelligent ethernet type */

#define	SPACE		0x20

#define NODE_ADDR_IO	0x08

#define	INIT_PARAM_FIELDS	9
#define	RESP_PARAM_FIELDS	3
#define	RESP_CHK_METHOD_PARAM	0
#define	RESP_QUIET_ERR_PARAM	1
#define	RESP_AUTO_CLR_PARAM	2
#define	INIT_PATTERN_PARAM	0
#define	INIT_ITERATE_PARAM	1
#define	INIT_FRAME_LEN_PARAM	2
#define	INIT_CHK_METHOD_PARAM	3
#define	INIT_DADDR_PARAM	4
#define	INIT_MAX_RETRY_PARAM	5
#define	INIT_TIMEOUT_PARAM	6
#define	INIT_QUIET_ERR_PARAM	7
#define	INIT_AUTO_CLR_PARAM	8

#define	XTR_ENGR_FIELDS		15
#define	SEND_ONLY_PARAM		0
#define	RECEIVE_ONLY_PARAM	1
#define	QCK_MODE_PARAM		2
#define	CONTEND_690_PARAM	3
#define	FORCE_16BIT_PARAM	4
#define	MANUAL_INIT_NIC_PARAM	5
#define	PROMISCUOUS_PARAM	6
#define	TX_THROTTLE_PARAM	7
#define	SRC_SAP_PARAM		8
#define	DST_SAP_PARAM		9
#define	LOOPING_MODE_PARAM	10
#define	BLOCK_RCV_MODE_PARAM	11
#define	LARGE_FRAMES_PARAM	12
#define	RSPND_CRC_ERRS_PARAM	13
#define	ACCEPT_ERR_PKT_PARAM	14

extern	char	*programMesgTable[];
