/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1989
*******************************************************************************
******************************************************************************/

/******************************************************************************

these are variable definitions specific to the sample driver

******************************************************************************/
char	*send_resp_strings[2] = { "Initiate", "Respond" };
int	num_of_fields;

char	version[] = "7.00";
char	engr_ver[] = "@(#) Version 7.00a\0$";

char	clear_string[] = "                                                                              ";
#define	ESC_STR_LEN 	26
char	esc_str[] = "Press <ESC> to continue...";

char	header[] = "WD LAN Adapter Sample Driver";

int	update_counter;
int	micro_chnl;
int	channel_pos;
