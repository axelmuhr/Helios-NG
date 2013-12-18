/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1990
*******************************************************************************
******************************************************************************/

extern    char        pattern[];

extern    char        node_addr[];
extern    char        null_addr[];
extern    char        mult_addr[];
extern    char        resp_addr[];

extern    char      *baseio_strings[];
extern    int       baseio_vals[];
extern    int       baseio_index;
extern    int       baseio;
extern    int       installed_baseio;
extern    char      *RAMbase_strings[];
extern    long      RAMbase_vals[];
extern    int       RAMbase_index;
extern    long      RAMbase;
extern    long      installed_RAMbase;
extern    char      *RAMsize_strings[];
extern    long      RAMsize_vals[];
extern    int       RAMsize_index;
extern    int       onboard_RAMsize_index;
extern    long      RAMsize;
extern    long      installed_RAMsize;
extern    char      *ROMbase_strings[];
extern    long      ROMbase_vals[];
extern    int       ROMbase_index;
extern    long      ROMbase;
extern    long      installed_ROMbase;
extern    char      *ROMsize_strings[];
extern    long      ROMsize_vals[3];
extern    int       ROMsize_index;
extern    long      ROMsize;
extern    long      installed_ROMsize;
extern    char      *irq_strings[];
extern    int       irq_vals[];
extern    int       irq_index;
extern    int       irq;
extern    int       interrupt_status;
extern    int       installed_irq;
extern    char      *fifo_strings[];
extern    int       fifo_vals[];
extern    int       fifo_index;
extern    int       fifo_depth;
extern    int       installed_fifo_depth;
extern    long      board_id;
extern    int       board_index;
extern    char far  *board_strings[];

#define	BLANK_STRING		0
#define	ETHERNET_STRING		1
#define	STARLAN_STRING		2
#define	WD8003WT_STRING		3
#define	WD8003W_STRING		4
#define	WD8003EB_STRING		5
#define	WD8003ETA_STRING	6
#define	WD8003STA_STRING	7
#define	WD8003EA_STRING		8
#define	WD8003SHA_STRING	9
#define	WD8003WA_STRING		10
#define	WD8013EBT_STRING	11
#define	WD8013EB_STRING		12
#define	WD8013EW_STRING		13
#define	WD8003EP_STRING		14
#define	WD8013W_STRING		15
#define	WD8003EPA_STRING	16
#define WD8003WPA_STRING	17

extern    char          batch;
extern    char          batch_type;
extern    char          responder_found;
extern    unsigned long iterations;
extern    int           frame_len;
extern    unsigned      max_retry;
extern    unsigned      rcv_timeout;
extern    int           auto_clr_index;
extern    int           cpu_id_index;
extern    char far      *cpu_id_strings[];
extern    int           send_only_flag;
extern    int           receive_only_flag;
extern    int           quick_mode;
extern    int           contend_690_mode;
extern    char          contend_690_val;
extern    int           manual_init_nic;
extern    int           nic_initialized;
extern    int           promiscuous_mode;
extern   unsigned int   tx_throttle;

#define	STAT_LEN	23	/* number of statistics used */
#define	PC_STATS	3	/* PC statistics */
#define	INT_TX_STATS	9
#define	INT_RX_STATS	11

/*--- statistics counters ---*/
extern unsigned long frm_sent;       /* total # data frames sent */
extern unsigned long bc_sent;        /* # broadcast frames sent */
extern unsigned long mc_sent;        /* # multicast frames sent */
extern unsigned long collisions;     /* # collisions encountered */
extern unsigned long aborts;         /* # aborted transmits */
extern unsigned long lost_crs;     /* # lost CRS errors on transmission */
extern unsigned long underruns;      /* # transmit underruns */
extern unsigned long cd_hearts;      /* # cd heartbeat errors */
extern unsigned long owc_errs;       /* # out of window collisions */
extern unsigned long frm_rcvd;       /* total # data frames rcvd */
extern unsigned long bc_rcvd;        /* # broadcast frames received */
extern unsigned long mc_rcvd;        /* # multicast frames received */
extern unsigned long crc_errs;       /* # CRC errors on received frames */
extern unsigned long align_errs;   /* # algnmnt err on received frames */
extern unsigned long overruns;       /* # receive overruns */
extern unsigned long missed;         /* # missed packets */
extern unsigned long rcvr_disabled;  /* # receiver disabled errors */
extern unsigned long deferring;      /* # deferring */
extern unsigned long runt_packets;   /* # runt packets */ 
extern unsigned long bad_nxt_pkt;    /* # bad next packet ptr */ 
extern unsigned long overlapped;     /* # tx overlapped */
extern unsigned long frm_rtry;       /* # transmit retries */
extern unsigned long check_errs;    /* # miscompares or check sum errors */ 

extern unsigned long collision_table[];

extern    int         vector_no;
extern    long        prv_vector;	/* to hold the previous int vector */
extern    int         save_mask;
extern    unsigned    intreg,
                      maskreg;

#if	defined(ATDGS)
#else
extern    int         mode;
#endif
extern    char        *non_stop_string;
extern    char        *chk_strings [];

#define	NULL_METHOD	0
#define	COMPARE_METHOD	1
#define	CHECKSUM_METHOD	2
#define	NUM_OF_METHODS	3
#define	CHECKSUM_VALUE	0xFF

extern    int         check_method;
extern    int         byp_index;
extern    int         upd_index;
extern    int         xmt_pending;

extern    char        *author_string;
extern    char        *coauthor_string;
extern    char        feature_info_byte_1;
