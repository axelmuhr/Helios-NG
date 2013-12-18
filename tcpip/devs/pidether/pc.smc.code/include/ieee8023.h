/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1990
*******************************************************************************
******************************************************************************/

#define	PACKET_DATA_SIZE	0x800	/* allow 2K packets for ENGR */
#define	PACKET_OVERHEAD		30

typedef	struct	RBUF
	{
		char	rcv_status;
		char	next_ptr;
		char	count0;
		char	count1;
		char	daddr[6];
		char	saddr[6];
		char	len1;
		char	len0;
		char	dsap;
		char	ssap;
		char	control;
		char	data[PACKET_DATA_SIZE];
	}
	;

typedef	struct	TBUF
	{
		char	d_addr[6];
		char	s_addr[6];
		char	len_1;
		char	len_0;
		char	d_sap;
		char	s_sap;
		char	cntrl;
		char	info[PACKET_DATA_SIZE];
	}
	;


