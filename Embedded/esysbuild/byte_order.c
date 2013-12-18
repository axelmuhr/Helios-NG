
/*
 * byte_order.c
 *
 *	Functions and data structures used to determine the byte ordering 
 * differences between the host machine and the processor for which the
 * nucleus is being built for.
 *
 */

#include <stdio.h>

#include "defs.h"
#include "externs.h"

#define TEST_VALUE	0x12345678

#define LITTLE_ENDIAN	0x78
#define BIG_ENDIAN	0x12

struct byte_order_str
{
	char		processor_name[12];
	unsigned char	byte_order_result;
};

struct byte_order_str processor_byte_order[4] =
{
	{"TRAN", LITTLE_ENDIAN},
	{"ARM", LITTLE_ENDIAN},		/* ??? */
	{"C40", LITTLE_ENDIAN},		/* ??? */
	{"", 0}
};


/*
 * host_byte_order ():
 *	Simple function to check the byte order of the host machine.
 * Write a long, and then check the value at byte 0.
 */
int host_byte_order ()
{
	long		n = TEST_VALUE;
	unsigned char *	b = (unsigned char *)(&n);

	sysbuild_debug ("host_byte_order () - returning 0x%02x\n", b[0]);

	return b[0];
}

int check_byte_order (char *	proc_name)
{
	int	i;

	/* find element in the array of known processors */
	for (i = 0; processor_byte_order[i].byte_order_result != 0; i++)
	{
		if (strequ_(processor_byte_order[i].processor_name, proc_name))
		{
#ifdef NEVER
			unsigned char host_order = host_byte_order ();
			unsigned char proc_order = processor_byte_order[i].byte_order_result;
			sysbuild_debug ("host_order = 0x%02x, proc_order = 0x%02x", host_order, proc_order);
#endif
			return (host_byte_order () != processor_byte_order[i].byte_order_result);
		}
	}

	sysbuild_warning ("Failed to find processor information for %s, assuming same byte order", proc_name);

#ifdef JUST_TESTING
	/* Little undocumented feature to test new processors */
	if (get_data (T_SWAP_BYTES))
	{
		sysbuild_warning ("Wait a minute, how did you know about swap_bytes.  I assume you know what your doing");

		return 1;
	}
#endif
	return 0;
}
