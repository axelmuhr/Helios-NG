/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1989
*******************************************************************************
******************************************************************************/

#include	"common.h"
#include	"ieee8023.h"

#if	defined(ROMDIAG)
#else
typedef	struct	RCV_TBL
	{
		struct	RBUF	rcv_table[NUM_OF_BUFFS];
	}
	;

struct	TBUF	far	*local_tx_buffer;
struct	RBUF 	far	*my_head;
struct	RBUF 	far	*my_tail;
struct	RCV_TBL	far	*table_ptr;
int	head_index;			/* index into table of current head */
int	tail_index;			/* index into table of last pkt rcvd */
int	buffers_used;			/* number of buffers used in table */
#endif

#if	defined(ROMDIAG)
#else
char	llc_dst_sap = 0;
char	llc_src_sap = 0;
char	llc_control_byte = 0;

char        broadcast = FALSE;
char        multicast = FALSE;
#endif

unsigned	try_again_count;
int		TCR_value;
int  manual_crc[4] = {0x1E, 0xD0, 0x21, 0x2F};
int	fifo_hold[8];


