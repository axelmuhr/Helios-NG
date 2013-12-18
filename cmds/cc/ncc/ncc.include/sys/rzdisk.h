
/*
 *	11/9/89	(ULTRIX)	@(#)rzdisk.h	1.1.1.2
 */

/************************************************************************
 *									*
 *			Copyright (c) 1984,86,87,88 by			*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/************************************************************************
 *
 * rzdisk.h	12-Sep-88
 *
 * SCSI Disk Utility Include File
 *
 * Modification history:
 *
 * 25-Oct-89    Janet Schank
 *      Added define for NO_DEFECTS.  Added the io_uxfer struct.
 *      Modified the mode_sel_sns_params struct and added the
 *      mode_sel_sns_data struct.  Added page_code_5 struct.
 *      Added the union rzdisk_params.
 *
 * 06-Feb-89	Alan Frechette
 *	Added in page 8 support.
 *
 * 12-Sep-88	Alan Frechette
 *	Created this utility for the maintainence of scsi disks.
 *
 ************************************************************************/
/* Special ioctl's for SCSI DISKS */
#define SCSI_FORMAT_UNIT 	_IOW('p', 4, struct format_params)
#define SCSI_REASSIGN_BLOCK 	_IOW('p', 5, struct reassign_params)
#define SCSI_READ_DEFECT_DATA 	_IOW('p', 6, struct read_defect_params)
#define SCSI_VERIFY_DATA 	_IOW('p', 7, struct verify_params)
#define SCSI_MODE_SELECT 	_IOW('p', 8, struct mode_sel_sns_params)
#define SCSI_MODE_SENSE 	_IOWR('p', 9, struct mode_sel_sns_params)
#define SCSI_GET_SENSE 		_IOR('p', 10, struct extended_sense)
#define SCSI_GET_INQUIRY_DATA 	_IOR('p', 11, struct inquiry_info)
#define SCSI_READ_LONG 		_IOW('p', 12, struct io_uxfer)
#define SCSI_WRITE_LONG 	_IOW('p', 13, struct io_uxfer)

/* Maximum defects to handle and return value for subroutine calls */
#define MAXDEFECTS		256
#define MAXPARAMSPACE		sizeof(struct mode_sel_sns_params)
#define SUCCESS			0
#define ERROR			-1

/* Possible ways to return the defect list for READ DEFECT */
#define BLK_FORMAT		0x0
#define BFI_FORMAT		0x4
#define PHY_FORMAT		0x5

/* Possible ways to format a disk for FORMAT UNIT */
#define VENDOR_DEFECTS		0
#define KNOWN_DEFECTS		1
#define	NO_DEFECTS		2

/* Possible page values for MODE SELECT and MODE SENSE */
#define CURRENT_VALUES		0
#define CHANGED_VALUES		1
#define DEFAULT_VALUES		2
#define SAVED_VALUES		3

/* Possible error conditions returned on failed commands */
#define NO_ERROR		0
#define SOFT_ERROR		1
#define HARD_ERROR		2
#define FATAL_ERROR		3

/* INQUIRY DATA Information */
struct inquiry_info {
    u_char	perfdt;		/* Peripheral Deice Type	      */
    u_char	devtq:7;	/* Device Type Qualifier	      */
    u_char	rmb:1;		/* Removable Media Bit		      */
    u_char	version;	/* Version			      */
    u_char		:8;	/* Reserved			      */
    u_char	addlen;		/* Additional Length		      */
    u_char	rsvd[3];	/* Reserved			      */
    u_char	vndrid[8];	/* Vendor ID (ASCII)		      */
    u_char	prodid[16];	/* Product ID (ASCII)		      */
    u_char	revlvl[4];	/* Revision level (ASCII)	      */
    u_char	revdata[8];	/* Revision data (ASCII)	      */
};

/* REQUEST SENSE Extended Sense Format */
struct extended_sense {
    	u_char	errcode:4;	/* Error Code			      */
    	u_char	errclass:3;	/* Error Class			      */
    	u_char	valid:1;	/* Valid			      */
    	u_char	segnum;		/* Segment Number		      */
    	u_char	snskey:4;	/* Sense Key			      */
    	u_char		:1;	/* Reserved			      */
    	u_char	ili:1;		/* Illegal Length Indicator	      */
    	u_char	eom:1;		/* End of Medium		      */
    	u_char	filmrk:1;	/* Filemark			      */
    	u_char	infobyte3;	/* Information Byte (MSB)	      */
    	u_char	infobyte2;	/* Information Byte 		      */
    	u_char	infobyte1;	/* Information Byte 		      */
    	u_char	infobyte0;	/* Information Byte (LSB)	      */
    	u_char	asl;		/* Additional Sense Length	      */
	u_char rb1[4];		/* Reserved bytes		      */
	u_char asc;		/* Additional Sense Code	      */
	u_char rb2;		/* Reserved byte		      */
	u_char fur;		/* Field Replaceable Unit (FRU)       */
	u_char bp;		/* Bit Pointer info		      */
	u_char fp1;		/* Field Pointer		      */
	u_char fp0;		/* Field Pointer		      */
};

/* READ DEFECT DATA defect list header */
struct form1_defect_list_header {
	u_char	reserved;	/* reserved field	    */
	u_char	format	: 3;	/* defect list format	    */
	u_char	gdl	: 1;	/* grown defect list 	    */
	u_char	mdl	: 1;	/* manufacturers defect list*/
	u_char		: 3;	/* reserved field	    */
	u_char	defect_len1;	/* defect list length	    */
	u_char	defect_len0;	/* defect list length	    */
};

/* FORMAT UNIT defect list header */
struct form2_defect_list_header {
	u_char	reserved;	/* reserved field	    */
	u_char	vu	: 1;	/* vender unique bit	    */
	u_char		: 3;	/* reserved field	    */
	u_char	stpf	: 1;	/* stop format on error bit */
	u_char	dcrt	: 1;	/* disable certification bit*/
	u_char	dpry	: 1;	/* disable primary bit	    */
	u_char	fov	: 1;	/* format options valid bit */
	u_char	defect_len1;	/* defect list length	    */
	u_char	defect_len0;	/* defect list length	    */
};

/* REASSIGN BLOCK defect list header */
struct form3_defect_list_header {
	u_char	reserved1;	/* reserved field	    */
	u_char	reserved2;	/* reserved field	    */
	u_char	defect_len1;	/* Length of bad blocks to reassign */
	u_char	defect_len0;	/* Length of bad blocks to reassign */
};

/* FORMAT UNIT and READ DEFECT DATA defect modes */
struct bytes_from_index_format {
	u_char	 cyl2;		/* cylinder number of defect */
	u_char	 cyl1;		/* cylinder number of defect */
	u_char	 cyl0;		/* cylinder number of defect */
	u_char	 head;		/* head number of defect     */
	u_char	 bfi3;		/* defect bytes from index   */
	u_char	 bfi2;		/* defect bytes from index   */
	u_char	 bfi1;		/* defect bytes from index   */
	u_char	 bfi0;		/* defect bytes from index   */
};

struct block_format {
	u_char	lba3;		/* defect logical block address */
	u_char	lba2;		/* defect logical block address */
	u_char	lba1;		/* defect logical block address */
	u_char	lba0;		/* defect logical block address */
};

struct physical_sector_format {
	u_char	 cyl2;		/* cylinder number of defect */
	u_char	 cyl1;		/* cylinder number of defect */
	u_char	 cyl0;		/* cylinder number of defect */
	u_char	 head;		/* head number of defect     */
	u_char	 sector3;	/* defect sector number	     */
	u_char	 sector2;	/* defect sector number	     */
	u_char	 sector1;	/* defect sector number	     */
	u_char	 sector0;	/* defect sector number	     */
};

/* MODE SENSE and MODE SELECT parameter list header */
struct mode_sel_sns_header {
	u_char	sense_len;	/* mode sense data length    */
	u_char	medium_type;	/* the media type	     */
	u_char		: 7;	/* reserved field	     */
	u_char	wp	: 1;	/* the write protect bit     */
	u_char	blk_des_len;	/* block descriptor length   */
};

/* MODE SENSE and MODE SELECT parameter list block descriptor */
struct block_descriptor {
	u_char   density_code;	/* the density code of media */
	u_char 	 nblks2;	/* total number of blocks    */
	u_char 	 nblks1;	/* total number of blocks    */
	u_char 	 nblks0;	/* total number of blocks    */
	u_char	 reserved;	/* reserved field	     */
	u_char 	 blklen2;	/* the logical block length  */
	u_char 	 blklen1;	/* the logical block length  */
	u_char 	 blklen0;	/* the logical block length  */
};

/* MODE SENSE and MODE SELECT page codes */
struct page_code_1 {
	u_char	pgcode	: 6;
	u_char	rsrv	: 1;
	u_char	ps	: 1;
	u_char 	pglength;
	u_char	flags;
	u_char	retry_count;
	u_char	correct_span;
	u_char	head_offset;
	u_char 	data_strobe;
	u_char	recovery_time;
};

struct page_code_2 {
	u_char	pgcode	: 6;
	u_char	rsrv	: 1;
	u_char	ps	: 1;
	u_char 	pglength;
	u_char	bus_fratio;
	u_char	bus_eratio;
	u_char 	bus_inactive1;
	u_char 	bus_inactive0;
	u_char 	disconn_time1;
	u_char 	disconn_time0;
	u_char 	conn_time1;
	u_char 	conn_time0;
	u_char 	reserved1;
	u_char 	reserved2;
};

struct page_code_3 {
	u_char	 pgcode	: 6;
	u_char	 rsrv	: 1;
	u_char	 ps	: 1;
	u_char 	 pglength;
	u_char	 tpz1;
	u_char	 tpz0;
	u_char	 aspz1;
	u_char	 aspz0;
	u_char	 atpz1;
	u_char	 atpz0;
	u_char	 atpv1;
	u_char	 atpv0;
	u_char   spt1;
	u_char   spt0;
	u_char   bps1;
	u_char   bps0;
	u_char   interleave1;
	u_char   interleave0;
	u_char   track_skew1;
	u_char   track_skew0;
	u_char   cylinder_skew1;
	u_char   cylinder_skew0;
	u_char	 flags;
	u_char 	 reserved1;
	u_char 	 reserved2;
	u_char 	 reserved3;
};

struct page_code_4 {
	u_char	 pgcode	: 6;
	u_char	 rsrv	: 1;
	u_char	 ps	: 1;
	u_char 	 pglength;
	u_char   ncyl2;
	u_char   ncyl1;
	u_char   ncyl0;
	u_char	 nheads;
	u_char   wprecomp2;
	u_char   wprecomp1;
	u_char   wprecomp0;
	u_char   rwc2;
	u_char   rwc1;
	u_char   rwc0;
	u_char	 dsr1;
	u_char	 dsr0;
	u_char 	 lzc2;
	u_char 	 lzc1;
	u_char 	 lzc0;
	u_char 	 reserved1;
	u_char 	 reserved2;
	u_char 	 reserved3;
};

struct page_code_5 {
	u_char	 pgcode	: 6;
	u_char	 rsrv1	: 1;
	u_char	 ps	: 1;
	u_char 	 pglength;
	u_char	xfer_rate1;
	u_char	xfer_rate0;
	u_char	num_heads;
	u_char	sec_per_trk;
	u_char	db_per_physec1;
	u_char	db_per_physec0;
	u_char	num_cyl1;
	u_char	num_cyl0;
	u_char	swpc1;
	u_char	swpc0;
	u_char	srwcc1;
	u_char	srwcc0;
	u_char	drv_stp_rate1;
	u_char	drv_stp_rate0;
	u_char	drv_sp_width;
	u_char	hd_stl_del1;
	u_char	hd_stl_del0;
	u_char	mtr_on_del;
	u_char	mtr_off_del;
	u_char	rsrv3	: 5;
	u_char	mo	: 1;
	u_char	ssn	: 1;
	u_char	trdy	: 1;
	u_char	sp_cyl	: 5;
	u_char	rsrv4	: 3;
	u_char	wpc_lvl;
	u_char	hl_del;
	u_char	hul_del;
	u_char	p2_def	: 4;
	u_char	p34_def	: 4;
	u_char	p1_def	: 4;
	u_char	p4_def	: 4;
	u_char	med_rr1;
	u_char	med_rr0;
	u_char	reserved1;
	u_char	reserved2;
};

struct page_code_8 {
	u_char	 pgcode	: 6;
	u_char	 rsrv	: 1;
	u_char	 ps	: 1;
	u_char 	 pglength;
	u_char 	 rc	: 1;
	u_char 	 ms	: 1;
	u_char 	 wce	: 1;
	u_char	 	: 5;
	u_char	 wrp	: 4;
	u_char	 drrp	: 4;
	u_char	 dpftl1;
	u_char	 dpftl0;
	u_char	 minpf1;
	u_char	 minpf0;
	u_char	 maxpf1;
	u_char	 maxpf0;
	u_char	 maxpfc1;
	u_char	 maxpfc0;
};

struct page_code_37 {
	u_char	pgcode	: 6;
	u_char	rsrv	: 1;
	u_char	ps	: 1;
	u_char 	pglength;
	u_char	spinup	: 1;
	u_char		: 7;
	u_char  reserved[22];
};

/* MODE SELECT and MODE SENSE parameters */
struct mode_sel_sns_params {
	caddr_t			   msp_addr;	/* Address of data (pages) */
	u_char			   msp_pgcode;  /* The pages to send/get   */
	u_char 			   msp_pgctrl;	/* The values to send/get  */
	u_char 			   msp_length;  /* The length to send/get  */
	u_char 			   msp_setps;  	/* Flag to set/unset PS bit*/
};

/* MODE SELECT and MODE SENSE data */
/* NOTE: structure size must not exceed 255 bytes (msp_length is a byte) */
struct mode_sel_sns_data {
	struct mode_sel_sns_header ms_hdr;	/* Mode sense/select header*/
	struct block_descriptor	   ms_desc;	/* Block descriptor info   */
	u_char			   ms_pages[243]; /* Space for all pages   */
};

/* REASSIGN BLOCK parameters */
struct reassign_params {
	struct 	form3_defect_list_header rp_header;
	u_char	rp_lbn3;		/* The LBN of the bad block */
	u_char	rp_lbn2;		/* The LBN of the bad block */
	u_char	rp_lbn1;		/* The LBN of the bad block */
	u_char	rp_lbn0;		/* The LBN of the bad block */
};

/* FORMAT UNIT parameters */
struct format_params {
	u_char	fp_format;		/* The format mode         */
	u_char	fp_pattern;		/* The format pattern      */
	u_char	fp_interleave;		/* The interleave factor   */
	u_char	fp_length;		/* The defect list length  */
	u_char	fp_defects;		/* VENDOR or KNOWN defects */
	u_char	*fp_addr;		/* The defect list address */
};

/* READ DEFECT DATA parameters */
struct read_defect_params {
	u_char	rdp_format;		/* The format mode         */
	u_short rdp_alclen;		/* The allocation length   */
	u_char	*rdp_addr;		/* The defect list address */
};

/* User I/O transfer requests for ioctl calls. */
struct io_uxfer {
	int	io_cnt;			/* byte count to transfer to user */
	u_char	*io_addr;		/* users address */
	u_char	io_cdb[12];		/* for a SCSI CDB */
};

/* FORMAT UNIT and READ DEFECT DATA defect descriptors */
struct defect_descriptors {
	union {
		struct form1_defect_list_header	rdd_hdr;
		struct form2_defect_list_header	fu_hdr;
	} dd_header;
	union {
		struct bytes_from_index_format	bfi[MAXDEFECTS];
		struct block_format		blk[MAXDEFECTS];
		struct physical_sector_format	phy[MAXDEFECTS];
	} dd_defects;
#define BFI	dd_defects.bfi
#define BLK	dd_defects.blk
#define PHY	dd_defects.phy
};

/* VERIFY DATA parameters */
struct verify_params {
	u_long	vp_lbn;			/* The beginning LBN to verify */
	u_short	vp_length;		/* The # of blocks to verify   */
};

/*
 * Tells the SCSI driver how much space to allocate for
 * the vairous parameter structures.
 */

union	rzdisk_params {
	struct	mode_sel_sns_params	rp_ms;
	struct	reassign_params		rp_rb;
	struct	format_params		rp_fp;
	struct	read_defect_params	rp_rd;
	struct	verify_params		rp_vp;
};

/*
 * Request Sense Sense Key Codes.
 */
#define SC_NOSENSE	0x00	
#define SC_RECOVERR	0x01	
#define SC_NOTREADY	0x02	
#define	SC_MEDIUMERR	0x03	
#define SC_HARDWARE	0x04	
#define SC_ILLEGALREQ	0x05	
#define SC_UNITATTEN	0x06	
#define SC_DATAPROTECT	0x07	
#define SC_BLANKCHK	0x08	
#define	SC_VNDRUNIQUE	0x09
#define	SC_COPYABORTD	0x0a
#define SC_ABORTEDCMD	0x0b	
#define	SC_EQUAL	0x0c
#define SC_VOLUMEOVFL	0x0d	
#define SC_MISCOMPARE	0x0e	
#define	SC_RESERVED	0x0f
