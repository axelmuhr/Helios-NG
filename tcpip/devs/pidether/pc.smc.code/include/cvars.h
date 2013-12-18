/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1990
*******************************************************************************
******************************************************************************/

#include	"common.h"

typedef	struct	RCV_TBL
	{
		struct	RBUF	rcv_table[NUM_OF_BUFFS];
	}
	;

extern	struct	TBUF	far	*local_tx_buffer;
extern	struct	RBUF 	far	*my_head;
extern	struct	RBUF 	far	*my_tail;
extern	struct	RCV_TBL	far	*table_ptr;
extern	int			head_index;
extern	int			tail_index;
extern	int			buffers_used;

extern  int   tpsr_hold,
       	      tbcr0_hold,
              tbcr1_hold,
              size_hold,
              pstart_hold,
              pstop_hold,
              rsr_hold,
              tsr_hold,
              isr_hold,
              imr_hold,
              rcr_hold,
              local_nxtpkt_ptr;
  
extern    char		llc_dst_sap;
extern    char		llc_src_sap;
extern    char		llc_control_byte;
extern    char		sequence_number;

extern	unsigned	try_again_count;
extern	int		TCR_value;
extern  int		manual_crc[];
extern	int		fifo_hold[];

extern    char		broadcast;
extern    char		multicast;
