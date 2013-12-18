/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1990
*******************************************************************************
******************************************************************************/

extern	char	*OEM_node_addr;
extern	char	*OEM_card_id;

typedef int     BOOLEAN;

typedef	struct	BOARD
    {
	unsigned long	brd_id;
	unsigned int	base_io;
        int	chnl_pos;
#if	defined(DGS)
	int	tst_mode;
	int	e_type;
#endif
    }
    ;

extern	struct	BOARD	board_table[];
extern	int  	board_count;
extern	int  	board_table_index;
extern	int  	old_baseio;
extern	int	allow_boot;

extern	char	model_byte;
extern	char	submodel_byte;

extern	int  	testing_mode;
extern	int  	ethernet_type;

extern    int       far    *contention_ptr;
extern    char      got_serviced;
extern    int         micro_chnl;
extern    int         channel_pos;
extern    int         adapter_num;

extern    int           batch_baseio;
extern    int           s_now_hex;
extern    int           auto_crc;
extern    int           loopback_mode;
extern    int           looping_mode;
extern    int           block_rcv_mode;
extern    int           large_frames;
extern    int           force_crc_errors;
extern    int           accept_err_packets;
extern    int           deadly_event;
extern    char        is_ram_enabled;

extern    int           chosen;
extern    int           lan_index;
extern    char far    *lan_type[];
extern    char far    *on_off_strings[];

extern    int           temp_vals[];

extern    char        configkeylist[];
extern    char        writeregkeylist[];
extern    char        ilbkeylist[];
extern    char        writeposkeylist[];
extern    char        send_message_key_list[];
extern    char        resp_to_message_key_list[];
extern    char        set_params_keys[];
extern    char        bufskeylist[];
extern    char        startkeylist[];
extern    char        show_stats_keys[];
