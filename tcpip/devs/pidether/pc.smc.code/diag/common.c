/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1989
*******************************************************************************
******************************************************************************/

/******************************************************************************

this module contains tools for implementing send, receive, and echo

******************************************************************************/

#include	"ieee8023.h"
#include	"cvars.h"
#include	"common.h"
#include	"eth_data.h"
#include	<string.h>

#if	defined(ENGR)
#include	"params.h"
#include	"diagdata.h"
#endif

/*****************************************************************************

this manages the request to release a page of the local receive buffer table

****************************************************************************/
update_head()
{
    if (buffers_used)
    {
        head_index++;					/* index next page */
        if (head_index == NUM_OF_BUFFS)			/* handle wrap around */
            head_index = 0;
        my_head = &table_ptr->rcv_table[head_index];	/* point to next page */
        buffers_used--;					/* and update counter */
    }
}

/******************************************************************************

this initializes the local receive table

******************************************************************************/
init_rcv_table ()
{
    short i;
    char far *clear_ptr;

    buffers_used = 0;			/* show empty table */
    head_index = 0;			/* index into rcv table */
    tail_index = -1;			/* will put first pkt in rcv_table[0] */
    my_head = &table_ptr->rcv_table[head_index];	/* point to next page */
    my_tail = NULL;
    clear_ptr = (char far *) table_ptr;	/* clear all space for completeness */
    for (i = 0; i < (sizeof (struct RCV_TBL)); i++)
        *clear_ptr++ = 0xFF;
}

/*************************************************************************

Char_to_hex:   Function that returns an integral hexadecimal value from 0 to 15
	       given a character

Entry:	  The character in question

Exit:	The corresponding hexadecimal value

*****************************************************************************/
/*************************************************************************

this converts a character to a hex digit

**************************************************************************/
int char_to_hex (chr)
char chr;
{
    int ret_val;

    if (islower(chr) || isupper(chr))
        ret_val = ((int) (toupper (chr) - 'A')) + 10;
    else
        ret_val = chr - '0';
    return (ret_val);
}

/*************************************************************************

Str_to_hex:  Function that returns a long integer value given a hexadecimal
	     string

Entry:	 pointer to the beginning of the string

Exit:	 length of the string

*****************************************************************************/

long str_to_hex (str, length)
char *str;
int length;
{
    int  index,
         count,
         int_val;
    long ret_val;
    long multiple;
    char chr;

    ret_val = 0;
    for (index = 0; index < length; index++)
    {
        multiple = 1;
        for (count = 0; count < (length - index - 1); count++)
           multiple *= 16;
        chr = *(str + index);
        if (islower(chr) || isupper(chr))
            int_val = ((int) (toupper (chr) - 'A')) + 10;
        else
            int_val = chr - '0';
     
        ret_val = ret_val + (int_val * multiple);
    }
    return (ret_val);
}

#if	defined(NEVER_USED)

/******************************************************************************

this calculates which multicast bit should be set in the 8390 for the given
	multicast address

******************************************************************************/
unsigned char calc_multicast_bit ()
{
    unsigned long crc;
    int      i,
             j,
             carry;
    unsigned char loc_mult_addr[6];

    for (i = 0; i < 6; i++)
        loc_mult_addr[i] = mult_addr[i];

    crc = 0xFFFFFFFF;
    for (i = 0; i < 6; i++)
    {
        for (j = 0; j < 8; j++)
        {
            carry = ((crc & 0x80000000) ? 1:0) ^ (loc_mult_addr[i] & 0x01);
            crc <<= 1;
            loc_mult_addr[i] >>= 1;
            if (carry)
                crc = ((crc ^ POLYNOMIAL) | carry);
        }
    }
    crc = crc & 0xFF000000;
    crc = crc >> 26;
    return ((unsigned char) crc);
}

#endif
/******************************************************************************

******************************************************************************/
long string_to_hex (str)
char *str;
{
	int	index,
		count,
		int_val,
		length;
	long	ret_val,
		multiple;
	char	chr;

	length = strlen (str);
	ret_val = 0;
	for (index = 0; index < length; index++)
	{
		multiple = 1;
		for (count = 0; count < (length - index - 1); count++)
		multiple *= 16;
		chr = *(str + index);
		if (islower(chr) || isupper(chr))
			int_val = ((int) (toupper (chr) - 'A')) + 10;
		else
			int_val = chr - '0';
	 
		ret_val = ret_val + (int_val * multiple);
	}
	return (ret_val);
}
