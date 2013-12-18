/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1990
*******************************************************************************
******************************************************************************/

unsigned int         tpsr_hold,
            tbcr0_hold,
            tbcr1_hold,
            size_hold,
            pstart_hold = 0x08,
            pstop_hold,
            rsr_hold,
            tsr_hold,
            isr_hold,
            imr_hold,
            rcr_hold,
            local_nxtpkt_ptr;

#if	defined(ROMDIAG)
#else
int  far    *node_addr_ptr;
int  far    *tx_ptr;
#endif


