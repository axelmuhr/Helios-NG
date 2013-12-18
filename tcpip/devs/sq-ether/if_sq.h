/* if_sq.h */

/* ----------------------------------------------------------- */

#define	ETHERHDR	14 
#define	ETHERMTU	1500
#define ETHERMIN	(60-ETHERHDR)
#define ETHERPKTSIZE	2000		/* buffer size, plenty */

/* ----------------------------------------------------------- */


/*   General constants   */

#define		LOW_BYTE		0x00FF
#define		HIGH_BYTE		0xFF00
#define		LOW_HALF		0xFFFF
#define		BYTES_PER_INT		4
#define		TWO_READ_DMA_TCT	2
#define		ONE_WRITE_DMA_TCT	1
#define		ONE_FOR_ROUND_UP 	1
#define		ONE_FOR_BAD_LAST_INT	1
#define		MAXUNIT			6

/* ----------------------------------------------------------- */


/*  Integer-  and  Byte-offsets  of data buffers  */

/*		Offset		Value	Meaning of Subfield	*/
/*--------------------------------------------------------------*/

#define		BO_L		 0	/*  Length  from BO_DA until
					    last byte in bytes	*/
#define		BO_SH		 4	/*  4 byte SEEQ header	*/
#define		BO_PP_HI	 4	/*  Packet pointer high	*/
#define		BO_PP_LO	 5	/*  Packet pointer low	*/
#define		BO_T_COM	 6	/*  SEEQ transmit command byte */
#define		BO_HD_ST	 6	/*  SEEQ header status    byte */
#define		BO_T_ST		 7	/*  SEEQ transmit status  byte */
#define		BO_FR_ST	 7	/*  SEEQ header frame status   */
#define		BO_DA		 8	/*  Destination address	*/
#define		BO_SA		14	/*  Source address	*/
#define		BO_ET		20	/*  Ethernet type	*/
#define		BO_DATA		22	/*  Higher level data	*/
#define		BO_PN		24	/*  Package number	*/

#define		IO_L		BO_L    >> 2
#define		IO_SH		BO_SH   >> 2
#define		IO_SA		BO_SA   >> 2


/* ----------------------------------------------------------- */



/*  Bit masks of SEEQ 8005   COMMAND   register  */

#define		SQ_EN_DMA_INT	0x0001
#define		SQ_EN_RX_INT	0x0002
#define		SQ_EN_TX_INT	0x0004
#define		SQ_EN_BUF_WIN	0x0008
#define		SQ_DMA_INT_ACK	0x0010
#define		SQ_RX_INT_ACK	0x0020
#define		SQ_TX_INT_ACK	0x0040
#define		SQ_BUF_WIN_ACK	0x0080
#define		SQ_SET_DMA_ON	0x0100
#define		SQ_SET_RX_ON	0x0200
#define		SQ_SET_TX_ON	0x0400
#define		SQ_SET_DMA_OFF	0x0800
#define		SQ_SET_RX_OFF	0x1000
#define		SQ_SET_TX_OFF	0x2000
#define		SQ_FIFO_READ	0x4000
#define		SQ_FIFO_WRITE	0x8000



/* ----------------------------------------------------------- */



/*  Bit masks of SEEQ 8005   STATUS   register  */

#define		SQ_DMA_INT_EN	0x0001
#define		SQ_RX_INT_EN	0x0002
#define		SQ_TX_INT_EN	0x0004
#define		SQ_BUF_WIN_EN	0x0008
#define		SQ_DMA_INT	0x0010
#define		SQ_RX_INT	0x0020
#define		SQ_TX_INT	0x0040
#define		SQ_BUF_WIN_INT	0x0080
#define		SQ_DMA_ON	0x0100
#define		SQ_RX_ON	0x0200
#define		SQ_TX_ON	0x0400
#define		SQ_FIFO_FULL	0x2000
#define		SQ_FIFO_EMPTY	0x4000
#define		SQ_FIFO_DIR	0x8000



/* ----------------------------------------------------------- */



/*  Bit masks of SEEQ 8005   CONFIGURATION 1   register  */

#define		SQ_STA_0_EN	0x0100
#define		SQ_STA_1_EN	0x0200
#define		SQ_STA_2_EN	0x0400
#define		SQ_STA_3_EN	0x0800
#define		SQ_STA_4_EN	0x1000
#define		SQ_STA_5_EN	0x2000



/* ----------------------------------------------------------- */



/*  Bit masks of SEEQ 8005   CONFIGURATION 2   register  */

#define		SQ_BYTE_SWAP	0x0001
#define		SQ_AUTO_U_REA	0x0002
#define		SQ_CRC_ERR_EN	0x0008
#define		SQ_DRIB_ERR_EN	0x0010
#define		SQ_SHORT_FR_EN	0x0020
#define		SQ_SLOT_TIME	0x0040
#define		SQ_XMIT_NO_PRE	0x0080
#define		SQ_ADDR_LENG	0x0100
#define		SQ_RECV_CRC	0x0200
#define		SQ_XMIT_NO_CRC	0x0400
#define		SQ_LOOP_BACK	0x0800
#define		SQ_WATCH_T_DIS	0x1000
#define		SQ_RESET	0x8000



/* ----------------------------------------------------------- */



/*  Buffer Code Bits   (for configuration register 2)  */

#define		BC_STA_0	0x0
#define		BC_STA_1	0x1
#define		BC_STA_2	0x2
#define		BC_STA_3	0x3
#define		BC_STA_4	0x4
#define		BC_STA_5	0x5
#define		BC_PROM		0x6
#define		BC_TEA		0x7
#define		BC_LPB		0x8
#define		BC_INT_VEC	0x9



/* ----------------------------------------------------------- */



/*   Constants  defining the partitioning of the LPB   */

#define		SQ_LPB_L	0x10000
#define		SQ_LPB_START	0x0000
#define		SQ_REA		0x00FF



/* ----------------------------------------------------------- */



/*  Bits of register  *etn_ffs  */

#define		SQ_DREQ_BIT	0x1	/*  Read only:   DREQ line of 8005 */
#define		SQ_FF_DMA	0x1	/*  Write only:  DMA mode          */
#define		SQ_FF_DWR	0x2	/*  Write only:  DMA Write mode    */
#define		SQ_FF_IDAP	0x4	/*  Write only:  Init. PAL DAP     */



/* ----------------------------------------------------------- */


#define		STA_0_ADDR		0
#define		STA_1_ADDR		6
#define		STA_2_ADDR		12
#define		STA_3_ADDR		18
#define		STA_4_ADDR		24
#define		STA_5_ADDR		30



/* ----------------------------------------------------------- */



/*  Bit masks for SEEQ 8005 packet headers  */

#define		SQ_RCVHDR	4
#define		SQ_XMITHDR	4
#define		SQ_RCVDONE	0x8000
#define		SQ_DATA_FOLLOWS	0x20
#define		SQ_ERRMSK	0x0F

#define		SQ_IERRORS	16

/* Transmitter error codes */

#define		SQ_XMIT16COL	0x04
#define		SQ_XMIT1COL	0x02
#define		SQ_XMITBBL	0x01

/* Receiver error codes */

#define		SQ_SHORT	0x08
#define		SQ_DRIBBLE	0x04
#define		SQ_CRC		0x02
#define		SQ_2BIG		0x01
#define		SQ_RCVOVR	0x0F



/* ----------------------------------------------------------- */


/*   Pointer to memory mapped registers   */

#define	 pery_base	0x0  
#define	 sq_base	pery_base  +  0x0200

/*---------------  Configuration constants  -------------------*/

#define	conf_addr_mm		1
#define	conf_loop_back 		0 
#define	conf_watch_dis		0 
#define	conf_byte_swap		0 
#define	conf_auto_rea		0 
#define	conf_acc_crc_err	1 
#define	conf_acc_drib_err	1 
#define	conf_acc_short_fr	1 


/*---------------  Definitions to make the sources readable  ----------------*/

#define sq_tea		(dcb->sq_gl.gl_tea)
#define sq_rec_area_l	(dcb->sq_gl.gl_rec_area_l)
#define sq_xmit_area_l	(dcb->sq_gl.gl_xmit_area_l)
#define sq_tea_addr	(dcb->sq_gl.gl_tea_addr)
#define sq_com_reg	(dcb->sq_gl.gl_com_reg)
#define sq_xmit_start	(dcb->sq_gl.gl_xmit_start)
#define sq_rec_start	(dcb->sq_gl.gl_rec_start)
#define sq_rcv_was_on	(dcb->sq_gl.gl_rcv_was_on)

/*-----------------------------------------------------------------------*/

#define	etn_ffs		(dcb->sq_reg.reg_etn_ffs)
#define	dma_win_base 	(dcb->sq_reg.reg_dma_win_base)
#define	tct_write_addr	(dcb->sq_reg.reg_tct_write_addr)
#define	tct_read_addr	(dcb->sq_reg.reg_tct_read_addr)
#define	sq_command	(dcb->sq_reg.reg_command)
#define	sq_status	(dcb->sq_reg.reg_status)
#define	sq_conf_1	(dcb->sq_reg.reg_conf_1)
#define	sq_conf_2	(dcb->sq_reg.reg_conf_2)
#define	sq_rea		(dcb->sq_reg.reg_rea)
#define	sq_buf_win	(dcb->sq_reg.reg_buf_win)
#define	sq_rp		(dcb->sq_reg.reg_rp)
#define	sq_tp		(dcb->sq_reg.reg_tp)
#define	sq_dma_ad	(dcb->sq_reg.reg_dma_ad)


/* end of sq_if.h */

