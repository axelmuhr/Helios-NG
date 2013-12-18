/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1989
*******************************************************************************
******************************************************************************/

/************************************************************************

this file implements the user interface for the sample driver

*************************************************************************/

#include	"ieee8023.h"
#include	"intrface.h"
#include	"cvars.h"
#include	"params.h"
#include	"board_id.h"
#include	"sdrvvars.h"

#include	<stdio.h>
#include	<conio.h>

char	for_color = WHITE;
char	back_color = BLUE;

/**************************************************************************

this handles all keystrokes from the user

**************************************************************************/
handle_key_strokes ()
{
    int    hex;			/* for converting keyboard input to hex */
    int    input;		/* input from the keyboard */
    int    index;		/* general indexing variable */
    int    field;		/* the current highlighted field */
    int    subfield;		/* the current subfield in a hex string */

    field = 0;			/* highlight first field to start */
    subfield = 0;		/* first digit of hex strings */
    while ((input = getch()) != ESCAPE)	/* ESCAPE returns and exits */
    {
        switch (input)
        {
            case EXT_ASCII:		/* was it an extended ascii char??? */
                input = getch();	/* yes so get the real code */
                switch (input)
                {
                    case UP_CURSOR:			/* goto prev field */
			display_field (field, FALSE);
                        field--;
                        if (field == SRC_ADDR_FIELD)
                            field--;
                        if (field < 0)
                            field = num_of_fields - 1;
                        subfield = 0;
			display_field (field, TRUE);
                        break;
                    case DOWN_CURSOR:			/* goto next field */
			display_field (field, FALSE);
                        field++;
                        if (field == SRC_ADDR_FIELD)
                            field++;
                        if (field > num_of_fields - 1)
                            field = 0;
                        subfield = 0;
			display_field (field, TRUE);
                        break;
                    default:			/* only up/down arrows */
                        break;
                }
                break;
            case RETURN:			/* execute */
                drive_board();			/* become initiator/responder */
                break;
            case SPACE:				/* change param value */
		switch (field)
                {
                    case MODE_FIELD:		/* initiator/responder */
			if (mode)
			    mode = 0;
			else
			    mode = 1;
                        break;
                    case ITERATION_FIELD:	/* number of packets to xmit */
                        iterations *= 10;
                        if (iterations == 0)
                            iterations = 1;
                        if (iterations > 10000000)
                            iterations = 0;
                        break;
                    case FRAME_LENGTH_FIELD:	/* frame length of xmit pkt */
                        frame_len *= 2;
			if (frame_len == 2048)
			    frame_len = 1518;
                        if (frame_len > 1518)
                            frame_len = 64;
                        break;
                    default:
                        break;
                }
                display_field (field, TRUE);
                break;
            default:	/* allow user to enter some parameter values */
		/* this is entering the destination address for pkts */
                if (field == RESPONDER_FIELD && isxdigit(input))
                {
                    responder_found = 1;
                    position_cursor (6, 60 + subfield + (subfield / 2));
                    set_color (FOR_BASE + back_color, BACK_BASE + for_color); 
                    hex = char_to_hex (input);
                    index = subfield / 2;
                    if (subfield % 2)
                        resp_addr[index] = (resp_addr[index] & 0xf0) | hex;
                    else
                        resp_addr[index] = (resp_addr[index] & 0x0f)|(hex << 4);
                    printf ("%1c", toupper(input));
                    subfield++;
                    if (subfield > 11)
                        subfield = 0;
                    position_cursor (6, 60 + subfield + (subfield / 2));
                    set_color (FOR_BASE + for_color, BACK_BASE + back_color);
                }
		/* this is entering the frame length */
                else if ((field == FRAME_LENGTH_FIELD) && (isdigit(input)))
                {
                    frame_len = 0;
                    frame_len = input - '0';
                    display_field (field, TRUE);
                    input = getch();
                    while (isdigit (input))
                    {
                        frame_len = (frame_len * 10) + input - '0';
                        if (frame_len > 1518)
                            frame_len = 0;
                        display_field (field, TRUE);
                        input = getch();
                    }
                    ungetch (input);
                    if (frame_len < 64)
                        frame_len = 64;
                    display_field (field, TRUE);
                }
                break;
        }
    }
}

/**************************************************************************

this redraws the screen after the color has been changed

***************************************************************************/
redraw_screen ()
{
	set_color (for_color + FOR_BASE, back_color + BACK_BASE);
	clear_screen ();
	draw_box ();
	write_header ();
	show_keys ();
	show_options ();
}

/**************************************************************************

this handles all screen initializations

***************************************************************************/
setup_screen ()
{
	reset_wrap ();
	printf ("%c[%dm", ESCAPE, BOLD);
	set_color (for_color + FOR_BASE, back_color + BACK_BASE);
	clear_screen ();
	draw_box ();
	write_header ();
	show_keys ();
	show_options ();
	show_stats ();
	display_field (MODE_FIELD, TRUE);
}

/**************************************************************************

this undoes everything done by 'setup_screen ()'

***************************************************************************/
unset_screen ()
{
	set_wrap ();
	printf ("%c[%dm", ESCAPE, ALL_OFF);
	set_color (FOR_BASE + WHITE, BACK_BASE + BLACK);
	clear_screen ();
}

/****************************************************************************

this resets cursor wrap around at end of line

*****************************************************************************/
reset_wrap ()
{
	printf ("%c[=7l", ESCAPE);
}

/****************************************************************************

this sets cursor wrap around at end of line

*****************************************************************************/
set_wrap ()
{
	printf ("%c[=7h", ESCAPE);
}

/****************************************************************************

this clears the screen

*****************************************************************************/
clear_screen ()
{
	printf ("%c[2J", ESCAPE);
}

/****************************************************************************

this sets the screen color

*****************************************************************************/
set_color (frgrnd, bckgrnd)
char	frgrnd;
char	bckgrnd;
{
	printf ("%c[%d;%dm", ESCAPE, frgrnd, bckgrnd);
}

/****************************************************************************

this draws the box around the screen

*****************************************************************************/
draw_box()
{
	int	count,
		row,
		column;

	position_cursor (1, 1);
	printf ("%c", TOP_LFT_CORNER);
	for (count = 0; count < 78; count++)
		printf ("%c", HORIZ_BAR);
	printf ("%c", TOP_RGT_CORNER);
	position_cursor (25, 1);
	printf ("%c", BTM_LFT_CORNER);
	for (count = 0; count < 78; count++)
		printf ("%c", HORIZ_BAR);
	printf ("%c", BTM_RGT_CORNER);
	for (row = 2; row < 25; row++)
	{
		column = 1;
		position_cursor (row, column);
		printf ("%c", VERT_BAR);
		column = 80;
		position_cursor (row, column);
		printf ("%c", VERT_BAR);
	}
	position_cursor (3, 1);
	printf ("%c", LFT_CONNECT);
	for (count = 0; count < 78; count++)
		printf ("%c", HORIZ_BAR);
	printf ("%c", RGT_CONNECT);
	position_cursor (22, 1);
	printf ("%c", LFT_CONNECT);
	for (count = 0; count < 78; count++)
		printf ("%c", HORIZ_BAR);
	printf ("%c", RGT_CONNECT);
}

/****************************************************************************

this writes the header into the proper location

*****************************************************************************/
write_header ()
{
	for_color = YELLOW;
	set_color (for_color + FOR_BASE, back_color + BACK_BASE);
	position_cursor (2, 3);
	printf ("%s", header);
	printf (" %s", version);
	position_cursor (2, 53);
	printf ("WESTERN DIGITAL CORP. 1990");
	for_color = WHITE;
	set_color (for_color + FOR_BASE, back_color + BACK_BASE);
}

/****************************************************************************

this positions the cursor to the passed row and column 

*****************************************************************************/
position_cursor (row, column)
int	row;
int	column;
{ 
	printf ("%c[%d;%dH", ESCAPE, row, column);
}

/***************************************************************************

this writes the valid keys into the box

****************************************************************************/
show_keys ()
{
	for_color = YELLOW;
	set_color (for_color + FOR_BASE, back_color + BACK_BASE);
	position_cursor (23, 3);
	printf ("ESC = quit   Arrows = move     Space = change   Return = Run");
	position_cursor (24, 3);
	printf ("                      cursor           value                ");
	for_color = WHITE;
	set_color (for_color + FOR_BASE, back_color + BACK_BASE);
}

/***************************************************************************

this clears the space where the available keys are shown
	used for displaying an error with 'my_alert'

****************************************************************************/
clear_keys ()
{
	position_cursor (23, 2);
	printf ("%s", clear_string);
	position_cursor (24, 2);
	printf ("%s", clear_string);
}

/***************************************************************************

this writes the statistics descriptors to the screen

****************************************************************************/
show_stats ()
{
	for_color = YELLOW;
	set_color (for_color + FOR_BASE, back_color + BACK_BASE);
	position_cursor (10, 5);
	printf ("Frames Sent");
	position_cursor (10, 42);
	printf ("Frames Received");
	for_color = WHITE;
	set_color (for_color + FOR_BASE, back_color + BACK_BASE);
}

/****************************************************************************

this places all field names on the screen

****************************************************************************/
show_options ()
{
	int	field;

	for_color = YELLOW;
	set_color (for_color + FOR_BASE, back_color + BACK_BASE);
	position_cursor (5, 5);
	printf ("MODE");
	position_cursor (6, 5);
	printf ("ITERATIONS");
	position_cursor (7, 5);
	printf ("FRAME LENGTH");
	position_cursor (5, 35);
	printf ("LOCAL NETWORK ADDRESS");
	position_cursor (6, 35);
	printf ("REMOTE NETWORK ADDRESS");
	for_color = WHITE;
	set_color (for_color + FOR_BASE, back_color + BACK_BASE);
	for (field = 0; field < num_of_fields; field++)
		display_field (field, FALSE);
}

/**************************************************************************

this displays the fields values in the main menu screen

**************************************************************************/
display_field (field, rev_flag)
int	field;
int	rev_flag;
{
	int	count;

	if (rev_flag)	/* this flags reverse video */
	{
		printf ("%c[%dm", ESCAPE, ALL_OFF);
		set_color (FOR_BASE + back_color, BACK_BASE + for_color); 
	}

	switch (field)
	{
		case MODE_FIELD:
			position_cursor (5, 20);
			printf ("%8s", send_resp_strings[mode]);
			position_cursor (5, 20);
			break;
		case ITERATION_FIELD:
			position_cursor (6, 20);
			if (iterations == 0)
				printf ("%8s", non_stop_string);
			else
				printf ("%8ld", iterations);
			position_cursor (6, 20);
			break;
		case FRAME_LENGTH_FIELD:
			position_cursor (7, 20);
			printf ("%8d", frame_len);
			position_cursor (7, 20);
			break;
		case SRC_ADDR_FIELD: 
			position_cursor (5, 60);
			for (count = 0; count < 6; count++)
			{
				printf ("%02X",node_addr[count] & 0xFF);
				printf (" ");
			}
			position_cursor (5, 60);
			break;
		case RESPONDER_FIELD:
			show_responder_addr ();
			break;
		default:
			break;
	}
	printf ("%c[%dm", ESCAPE, BOLD);
	set_color (FOR_BASE + for_color, BACK_BASE + back_color);
}

/*************************************************************************


**************************************************************************/
show_responder_addr ()
{
	int	count;

	position_cursor (6, 60);
	for (count = 0; count < 6; count++)
	{
		printf ("%02X",resp_addr[count] & 0xFF);
		printf (" ");
	}
	position_cursor (6, 60);
}
	
/*************************************************************************


**************************************************************************/
my_alert (string_ptr)
char	*string_ptr;
{
	int	input;
	int	length;
	char	*temp_ptr;

	temp_ptr = string_ptr;
	length = 0;
	while (*temp_ptr != '\0')
	{
		length++;
		temp_ptr++;
	}
	clear_keys ();
	position_cursor (23, ((SCREEN_WIDTH - length)/2));
	printf ("%s", string_ptr);
	printf ("%c", BELL);
	position_cursor (24, ((SCREEN_WIDTH - ESC_STR_LEN)/2));
	printf ("%s", esc_str);
	while ((input = getch ()) != ESCAPE)
		;
	show_keys ();
}

