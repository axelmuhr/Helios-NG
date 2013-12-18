/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1989
*******************************************************************************
******************************************************************************/

#define	TRUE		-1
#define	FALSE		0

#include	"misc.h"

#if	defined(ROMDIAG)
#else
char        mult_addr[6] = { 0x01, 0, 0xC0, 0, 0, 0x01 };
char        resp_addr[6] = { 0, 0, 0xC0, 0, 0, 0 };
#endif
char        node_addr[6] = { 0, 0, 0xC0, 0, 0, 0 };	/* these must start */
char        null_addr[6] = { 0, 0, 0xC0, 0, 0, 0 };	/*     on even word */

#if	defined(ROMDIAG)
#else
  char      pattern [20] = "ABCDEFGHIJKLMNOPQRST";
/*
  char      pattern [20] = { 0x2a, 0x73, 0x97, 0xc6, 0xde, 0xda, 0x24, 0x0e,
			0x7d, 0xea, 0x2c, 0xf2, 0x28, 0x0c, 0x02, 0x7f,
			0x00, 0x00, 0x00, 0x00 };
*/

  char      *baseio_strings[NUM_BASEIOS] =
				  { "  200", "  220", "  240", "  260", 
                                    "  280", "  2A0", "  2C0", "  2E0",
                                    "  300", "  320", "  340", "  360",
                                    "  380", "  3A0", "  3C0", "  3E0",
				    "  800", " 1800", " 2800", " 3800",
				    " 4800", " 5800", " 6800", " 7800",
				    " 8800", " 9800", " A800", " B800",
				    " C800", " D800", " E800", " F800" };
  unsigned int	baseio_vals[NUM_BASEIOS] =
				  { 0x200, 0x220, 0x240, 0x260,
                                    0x280, 0x2a0, 0x2c0, 0x2e0,
                                    0x300, 0x320, 0x340, 0x360,
                                    0x380, 0x3a0, 0x3c0, 0x3e0,
				    0x800, 0x1800, 0x2800, 0x3800,
				    0x4800, 0x5800, 0x6800, 0x7800,
				    0x8800, 0x9800, 0xa800, 0xb800,
				    0xc800, 0xd800, 0xe800, 0xf800 };
  int       baseio_index = D_BASEIO_INDEX;
#endif
  unsigned int	baseio = D_BASEIO_VAL;
#if	defined(ROMDIAG)
#else
  unsigned int	installed_baseio = D_BASEIO_VAL;
  char      *RAMbase_strings[NUM_RAMBASES] = { 
                                     "  A0000", "  A2000", "  A4000", "  A6000",
                                     "  A8000", "  AA000", "  AC000", "  AE000",
                                     "  B0000", "  B2000", "  B4000", "  B6000",
                                     "  B8000", "  BA000", "  BC000", "  BE000",
                                     "  C0000", "  C2000", "  C4000", "  C6000",
                                     "  C8000", "  CA000", "  CC000", "  CE000",
                                     "  D0000", "  D2000", "  D4000", "  D6000",
                                     "  D8000", "  DA000", "  DC000", "  DE000",
                                     "  E0000", "  E2000", "  E4000", "  E6000",
                                     "  E8000", "  EA000", "  EC000", "  EE000",
                                     "  F0000", "  F2000", "  F4000", "  F6000",
                                     "  F8000", "  FA000", "  FC000", "  FE000"
                                   };
  long      RAMbase_vals[NUM_RAMBASES] = {
                                 0xA0000000, 0xA2000000, 0xA4000000, 0xA6000000,
                                 0xA8000000, 0xAA000000, 0xAC000000, 0xAE000000,
                                 0xB0000000, 0xB2000000, 0xB4000000, 0xB6000000,
                                 0xB8000000, 0xBA000000, 0xBC000000, 0xBE000000,
                                 0xC0000000, 0xC2000000, 0xC4000000, 0xC6000000,
                                 0xC8000000, 0xCA000000, 0xCC000000, 0xCE000000,
                                 0xD0000000, 0xD2000000, 0xD4000000, 0xD6000000,
                                 0xD8000000, 0xDA000000, 0xDC000000, 0xDE000000,
                                 0xE0000000, 0xE2000000, 0xE4000000, 0xE6000000,
                                 0xE8000000, 0xEA000000, 0xEC000000, 0xEE000000,
                                 0xF0000000, 0xF2000000, 0xF4000000, 0xF6000000,
                                 0xF8000000, 0xFA000000, 0xFC000000, 0xFE000000
                               };
  int       RAMbase_index = D_RAMBASE_INDEX;
#endif
  long      RAMbase = D_RAMBASE_VAL;
#if	defined(ROMDIAG)
#else
  long      installed_RAMbase = D_RAMBASE_VAL;
  char      *RAMsize_strings[NUM_RAMSIZES] =
				{ "  8K", "  16K", "  32K" , "  64K" };
  long      RAMsize_vals[NUM_RAMSIZES] =
				 { 0x2000, 0x4000, 0x8000, 0x10000 };
  int       RAMsize_index = D_RAMSIZE_INDEX;
  int       onboard_RAMsize_index = D_RAMSIZE_INDEX;
#endif
  long      RAMsize = D_RAMSIZE_VAL;
#if	defined(ROMDIAG)
#else
  long      installed_RAMsize = D_RAMSIZE_VAL;
  char      *ROMbase_strings[NUM_ROMBASES] =
			{ "  C0000", "  C2000", "  C4000", "  C6000",
                          "  C8000", "  CA000", "  CC000", "  CE000",
			  "  D0000", "  D2000", "  D4000", "  D6000",
                          "  D8000", "  DA000", "  DC000", "  DE000", };
  long      ROMbase_vals[NUM_ROMBASES] =
			{ 0xC0000000, 0xC2000000, 0xC4000000, 0xC6000000,
                          0xC8000000, 0xCA000000, 0xCC000000, 0xCE000000,
			  0xD0000000, 0xD2000000, 0xD4000000, 0xD6000000,
                          0xD8000000, 0xDA000000, 0xDC000000, 0xDE000000 };
  int       ROMbase_index = D_ROMBASE_INDEX;
#endif
  long      ROMbase = D_ROMBASE_VAL;
#if	defined(ROMDIAG)
#else
  long      installed_ROMbase = D_ROMBASE_VAL;
  char      *ROMsize_strings[NUM_ROMSIZES] =
			{ "  Disabled", "   8K", "  16K", "  32K", "  64K" };
  long      ROMsize_vals[NUM_ROMSIZES] =
			{ 0, 0x2000, 0x4000, 0x8000, 0x10000 };
  int       ROMsize_index = D_ROMSIZE_INDEX;
#endif
  long      ROMsize = D_ROMSIZE_VAL;
#if	defined(ROMDIAG)
#else
  long      installed_ROMsize = D_ROMSIZE_VAL;
  char      *irq_strings[NUM_IRQS] = { "  2", "  3", "  4",  "  5",
                                "  6", "  7", "  10", "  11", "  14", "  15" };
  int       irq_vals[NUM_IRQS] = { 2, 3, 4, 5, 6, 7, 10, 11, 14, 15 };
  int       irq_index = D_IRQ_INDEX;
#endif
  int       irq = D_IRQ_VAL;
  int       interrupt_status = 1;
#if	defined(ROMDIAG)
#else
  int       installed_irq = D_IRQ_VAL;
  char      *fifo_strings[4] = { "  2 Bytes", "  4 Bytes",
                                 "  8 Bytes", "  12 Bytes" };
  int       fifo_vals[4] = { 2, 4, 8, 12 };
  int       fifo_index = 2;
#endif
  int       fifo_depth = 8;
#if	defined(ROMDIAG)
#else
  int       installed_fifo_depth= 8;
#endif
  long      board_id = 0;
#if	defined(ROMDIAG)
#else
  int       board_index = 0;
  char far  *board_strings[18] = { "        ", "Ethernet", "StarLAN ",
                                   "8003WT  ", "8003W   ", "8003EB  ",
                                   "8003ET/A", "8003ST/A",
                                   "8003E/A ", "8003SH/A", "8003W/A ",
                                   "8013EBT ", "8013EP  ", "8013EW  ",
                                   "8003EP  ", "8013W   ", "8013EP/A",
				   "8013WP/A" };

  char        batch = FALSE;
  char        batch_type = 0;
  char        responder_found = FALSE;
  unsigned long iterations = 1000;
  int         frame_len = 64;

  unsigned    max_retry = 2;
  unsigned    rcv_timeout = 3;
  int         auto_clr_index = 1;
#endif
#if	defined(ENGR)
  int       cpu_id_index = 0;
  char far  *cpu_id_strings[13] = { "        ",
                                    "8086/8088",
                                    "80186/80188",
                                    "80286",
                                    "80386" };
  int         send_only_flag = 0;
  int         receive_only_flag = 0;
  int         quick_mode = 0;
  int         contend_690_mode = 0;
  char        contend_690_val = 0x22;
  int         manual_init_nic = 0;
  int         nic_initialized = 0;
  int         promiscuous_mode = 0;
  unsigned int   tx_throttle = 0;
#endif

#if	defined(ROMDIAG)
#else
  /**** statistics counters ****/
  unsigned long frm_sent = 0;       /* total # data frames sent */
  unsigned long bc_sent = 0;        /* # broadcast frames sent */
  unsigned long mc_sent = 0;        /* # multicast frames sent */
  unsigned long collisions = 0;     /* # collisions encountered */
  unsigned long aborts = 0;         /* # aborted transmits */
  unsigned long lost_crs = 0;      /* # lost CRS errors on transmission */
  unsigned long underruns = 0;      /* # transmit underruns */
  unsigned long cd_hearts = 0;      /* # cd heartbeat errors */
  unsigned long owc_errs = 0;       /* # out of window collisions */
  unsigned long frm_rcvd = 0;       /* total # data frames rcvd */
  unsigned long bc_rcvd = 0;        /* # broadcast frames received */
  unsigned long mc_rcvd = 0;        /* # multicast frames received */
  unsigned long crc_errs = 0;       /* # CRC errors on received frames */
  unsigned long align_errs = 0;    /* # algnmnt errs on received frames */
  unsigned long overruns = 0;       /* # receive overruns */
  unsigned long missed = 0;         /* # missed packets */
  unsigned long rcvr_disabled = 0;  /* # receiver disabled errors */
  unsigned long deferring = 0;      /* # deferring */
  unsigned long runt_packets = 0;   /* # runt packets */ 
  unsigned long bad_nxt_pkt = 0;    /* # reinitialize NIC */ 
  unsigned long overlapped = 0;     /* # tx overlapped */
  unsigned long frm_rtry = 0;       /* # transmit retries */
  unsigned long check_errs = 0;    /* # miscompares or check sum errors */ 
#endif

#if	defined(ENGR)
  unsigned long collision_table[17]; /* array of collisions counters */
#endif

  int         vector_no;
  long        prv_vector;		/* to hold the previous int vector */
  int         save_mask;
  unsigned    intreg,
              maskreg;

#if	defined(ROMDIAG)
#else
  int         mode = 0;
  char        *non_stop_string = "Non-stop";
  char        *chk_strings [3] =  { "None   ", "Compare", "Chcksum" };

  int         check_method = FALSE;
  int         byp_index = FALSE;
  int         upd_index = FALSE;
  int         xmt_pending = FALSE;
#endif

#if	defined(DGS)
#else
char	*author_string = "George Alexander Kalwitz, 1991";
char	*coauthor_string = "Michael J. Steiger, 1991";
#endif
char    feature_info_byte_1;
