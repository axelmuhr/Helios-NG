/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*	dkio.h	6.1	83/07/29	*/
/*
 * Structures and definitions for disk io control commands
 */

/* 
 * disk io control commands 
 */
#define DIOCFMTMAP	_IOW('d', 1, struct fmt_map_info)	/* Format and Map */
#define DIOCVFYSEC	_IOW('d', 2, struct verify_info)	/* Verify sectors */
#define DIOCGETCTLR	_IOR('d', 3, struct ctlr_info)	/* Get ctlr info */
#define DIOCDIAG	_IOWR('d', 4, struct diag_info)	/* Perform diag */
#define DIOCSETDP	_IOW('d', 5, struct device_parameters) /* Set devparams */
#define DIOCNOECC	_IOW('d', 6, int)			/* Disable/Enable ECC */
#define DIOCINITVH	_IOW('d', 7, caddr_t)		/* Init volhdr info */
#define DIOCRDEFECTS	_IOWR('d', 8, struct media_defect) /* Read mediadefects */
