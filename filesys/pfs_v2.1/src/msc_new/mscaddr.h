/*
 * $Header: /Chris/00/helios/msc/RCS/mscaddr.h,v 2.0 91/08/21 18:04:31 chris
 * Exp Locker: chris $
 */

/*************************************************************************
**									**
**	            M S C   D I S C   D E V I C E			**
**	            -----------------------------			**
**									**
**		  Copyright (C) 1990, Parsytec GmbH			**
**			 All Rights Reserved.				**
**									**
**									**
** mscaddr.h								**
**									**
**	- Hardware addresses for the MSC board				**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	14/09/90 : C. Fleischer					**
*************************************************************************/

#ifndef	__mscaddr_h
#define	__mscaddr_h

/*----------------------- MSC Hardware addresses -----------------------*/

#define	Xilinx		((uword *) 0x100)
#define XI_Reg0		((uword *) 0x118)
#define XI_Reg1		((uword *) 0x11C)
#define XI_Status	((uword *) 0x118)
#define XI_Interrupt	((uword *) 0x11C)

#define BigLatch	((uword *) 0x8000)

#define	WD_Address	((uword *) 0x100)
#define WD_AuxStatus	((uword *) 0x100)
#define	WD_Register	((uword *) 0x104)

#define	FL_Status	((uword *) 0x100)
#define FL_Data		((uword *) 0x104)
#define FL_Oper		((uword *) 0x108)
#define FL_Contr	((uword *) 0x10C)

/*---------------------------- Xilinx bits -----------------------------*/

#define XB_Interrupt		0x10
#define XB_FlReady		0x20
#define	XB_Word			0x40
#define XB_Timeout		0x80

/*-------------------------- Xilinx Flipflops --------------------------*/

#define RXF_Event	XI_Reg0		/* Hardware Address for XF_Flag	 */
#define RXF_ExtO	XI_Reg0
#define RXF_Scsi	XI_Reg1
#define RXF_Pres	XI_Reg1
#define RXF_Read	XI_Reg1
#define RXF_DBA		XI_Reg1

#define rXF_Event	XI_reg0		/* Hardware Address for XF_Flag	 */
#define rXF_ExtO	XI_reg0
#define rXF_Scsi	XI_reg1
#define rXF_Pres	XI_reg1
#define rXF_Read	XI_reg1
#define rXF_DBA		XI_reg1

#define SXF_Event	0x10		/* Set Flag mask		 */
#define SXF_ExtO	0x20
#define SXF_Scsi	0x10
#define SXF_Pres	0x20
#define SXF_Read	0x40
#define SXF_DBA		0x80

#define CXF_Event	0xE0		/* Clear Flag mask		 */
#define CXF_ExtO	0xD0
#define CXF_Scsi	0xE0
#define CXF_Pres	0xD0
#define CXF_Read	0xB0
#define CXF_DBA		0x70

/*------------------------- WD 3393 registers --------------------------*/

#define WR_OwnID		0x00
#define WR_CDBSize		0x00
#define WR_Control		0x01
#define WR_Timeout		0x02
#define WR_CDB01		0x03
#define WR_CDB02		0x04
#define WR_CDB03		0x05
#define WR_CDB04		0x06
#define WR_CDB05		0x07
#define WR_CDB06		0x08
#define WR_CDB07		0x09
#define WR_CDB08		0x0A
#define WR_CDB09		0x0B
#define WR_CDB10		0x0C
#define WR_CDB11		0x0D
#define WR_CDB12		0x0E
#define	WR_TargetLUN		0x0F
#define WR_CmdPhase		0x10
#define WR_Synch		0x11
#define WR_TCountH		0x12
#define WR_TCountM		0x13
#define WR_TCountL		0x14
#define WR_DestID		0x15
#define WR_SourceID		0x16
#define WR_ScsiStatus		0x17
#define WR_Command		0x18
#define WR_Data			0x19

/*---------------------------- WD 3393 bits ----------------------------*/

#define	WB_Interrupt		0x80	/* Bit 7 of Aux.Status register	 */
#define	WB_LastIgnored		0x40	/* Bit 6 of Aux.Status register	 */
#define	WB_Busy			0x20	/* Bit 5 of Aux.Status register	 */
#define	WB_InProgress		0x10	/* Bit 4 of Aux.Status register	 */
#define	WB_ParityError		0x02	/* Bit 1 of Aux.Status register	 */
#define	WB_DataReady		0x01	/* Bit 0 of Aux.Status register	 */

/*-------------------------- WD 3393 commands --------------------------*/

#define	WC_Reset		0x00
#define	WC_Abort		0x01
#define	WC_AssertATN		0x02
#define	WC_NegateACK		0x03
#define	WC_Disconnect		0x04
#define	WC_SelectATN		0x06
#define	WC_Select		0x07
#define	WC_SelectTransferATN	0x08
#define	WC_SelectTransfer	0x09
#define	WC_Transfer		0x20
#define	WC_TransferSingle	0xA0
#define	WC_ReceiveData		0x11
#define	WC_TranslateAddress	0x18
#endif

/*--- end of mscaddr.h ---*/
