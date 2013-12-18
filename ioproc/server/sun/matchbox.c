
/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      matchbox.c                                                      --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.9 20/1/94\ Copyright (C) 1994, Perihelion Distributed Software */

#define Linklib_Module

#include "../helios.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>

#include <thread.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>

extern "C"
{
#include <tsplib.h>
}

#define ServerDebug_(x)		printf x; putchar ('\n')
/* #define ServerDebug_(x)	ServerDebug x */

thread_t	ReadThread;

#define MAX_ERRORS	10

int	LinkDaemonAlive;
#define ld_alive_	LinkDaemonAlive

unsigned char	LdByte;
int		LdSemaphore;
int		LdStop;
int		LdAcknowledge;

#define ld_byte_	LdByte
#define ld_semaphore_	LdSemaphore
#define ld_ack_		LdAcknowledge
#define ld_stop_	LdStop

#define MBOX_TIMEOUT	1
#define MBOX_RETRIES	5

/*
#define LINKD_DEBUG
#define MBOX_DEBUG
*/

void * link_daemon (void * fd) {
        int	mbox_fd = *((int*)fd);
 	unsigned char	b;

	int	tsp_result; 	int	errors = 0;

#ifdef LINKD_DEBUG
 	printf ("Link Daemon starting (It's alive I tell you!!!)\n");

	printf ("Link Daemon - mbox_fd = %d\n");
#endif

	if (tsp_reset (mbox_fd) == -1) 
	{
		printf ("Link Daemon - failed to reset MatchBox (Oh well, press on regardless)\n");
	}

	ld_alive_ = 1;
	while (ld_alive_)
	{

		b = '\0';
        	tsp_result = tsp_read (mbox_fd, (void *)(&b), 1, 1);
#ifdef LINKD_DEBUG
		printf ("Link Daemon - tsp_result = %d\n", tsp_result);
#endif

		if (tsp_result == -1)
		{
			printf ("Link Daemon read error (%d)\n", errors);

			if (errors == MAX_ERRORS)
			{
				printf ("Link Daemon dying, too many errors (I'm out of here ...)\n");
				break;
			}
			errors++;
		}
		else if (tsp_result == 0)
		{
#ifdef LINKD_DEBUG
			printf ("Link Daemon nothing to read (It's all gone dark)\n");
#endif
		}
		else
		{
#ifdef LINKD_DEBUG
			printf ("Link Daemon read 0x%02x from link\n", b);
#endif

			/* place byte in global space and signal that I've found it */
			ld_byte_ = b;
			ld_semaphore_ = 1;

			/* wait for IO server to notice */
			while (ld_semaphore_ != 0 && ld_alive_)
				;

			ld_ack_ = 0;
			ld_byte_ = '\0';

			while (ld_stop_ == 1 && ld_alive_)
				;
#ifdef LINKD_DEBUG
			printf ("Link Daemon continuing (onwards into the great wild yonder ...)\n");
#endif
		}
	}

	ld_alive_ = 0;
	ld_ack_ = 2;	/* acknowledge death of daemon */

#ifdef LINKD_DEBUG
	printf ("Link Daemon dying (I'm melltinnggg ...)\n");
#endif
	return fd;
}
		

#define link_fd (int)(Matchbox_links[current_link].fildes)

/*
 * Transtech Matchbox specifics
 */

#define MATCHBOX_MAX_LINKS	4

#define MBOX_DEFAULT_SCSI_CONTROLLER		0
#define MBOX_DEFAULT_SCSI_ID			5
#define MBOX_DEFAULT_SITE			0

PRIVATE Trans_link Matchbox_links[MATCHBOX_MAX_LINKS];

#if ANSI_prototypes
void matchbox_init_link (void)
#else
void matchbox_init_link ()
#endif
{
	word	scsi_controller = get_int_config ("matchbox_scsi_controller");
	word	scsi_id = get_int_config ("matchbox_scsi_id");
	word	scsi_site = get_int_config ("site");

	if (scsi_controller == Invalid_config)
	{
		scsi_controller = MBOX_DEFAULT_SCSI_CONTROLLER;
	}

	if (scsi_id == Invalid_config)
	{
		scsi_id = MBOX_DEFAULT_SCSI_ID;	/* default scsi_id */
	}

	if (scsi_id < 0 || scsi_id > 7)
	{
		ServerDebug_(("Invalid scsi id %d given, assuming %d", scsi_id, MBOX_DEFAULT_SCSI_ID));
	}

	if (scsi_site == Invalid_config)
	{
		scsi_site = MBOX_DEFAULT_SITE;
	}

	if (scsi_site < 0 || scsi_site > 3)
	{
		ServerDebug_(("Invalid site %d given, assuming %d", scsi_site, MBOX_DEFAULT_SITE));
	}

#ifdef MBOX_DEBUG
	ServerDebug_(("matchbox_init_links ()"));
#endif

	number_of_links = MATCHBOX_MAX_LINKS;

	link_table = &(Matchbox_links[0]);
	sprintf (Matchbox_links[scsi_site].link_name, "c%dt%dl%d", scsi_controller, scsi_id, scsi_site);

#ifdef MBOX_DEBUG
	ServerDebug_(("Matchbox_links[%d] = %s\n", scsi_site, Matchbox_links[scsi_site].link_name));
#endif

	Matchbox_links[scsi_site].flags = Link_flags_unused 
				+ Link_flags_uninitialised
				+ Link_flags_firsttime
				+ Link_flags_not_selectable;

	Matchbox_links[scsi_site].connection = -1;
	Matchbox_links[scsi_site].fildes     = -1;
	Matchbox_links[scsi_site].state      = Link_Reset;
	Matchbox_links[scsi_site].ready      = 0;
}

#if ANSI_prototypes
int matchbox_open_link (int tabno)
#else
int matchbox_open_link (tabno)
int    tabno;
#endif
{
	int	tsp_fd;

	ld_alive_ = 0;

	ld_stop_	= 0;
	ld_semaphore_	= 0;
	ld_ack_		= 0;

#ifdef MBOX_DEBUG
	ServerDebug_(("matchbox_open_link (%d)", tabno));

	ServerDebug_(("matchbox_open_link () - opening %s", Matchbox_links[tabno].link_name));
#endif

	if ((tsp_fd = tsp_open (Matchbox_links[tabno].link_name)) == -1)
	{
		ServerDebug_(("matchbox_open_link () - failed to open link"));

		return 0;
	}

	if (tsp_reset (tsp_fd) == -1)
	{
		ServerDebug_(("matchbox_open_link () - failed to reset link"));
	}

#ifdef MBOX_DEBUG
	ServerDebug_(("matchbox_open_link () - opened %d to matchbox", tsp_fd));
#endif
	
	/* start up link daemon */
	if (thr_create (NULL, NULL, link_daemon, (void *)(&tsp_fd),
        	        THR_DETACHED | THR_DAEMON, 
			&ReadThread) != 0)
	{
        	perror ("Failed to create link daemon thread");

	        return 0;
	}

	Matchbox_links[tabno].fildes = tsp_fd;

#ifdef MBOX_DEBUG
	ServerDebug_(("matchbox_open_link () - waiting for link daemon"));
#endif

	sleep (1);

	/* wait for daemon to awaken */
	while (ld_alive_ == 0)
		;

	sleep (1);

#ifdef MBOX_DEBUG
	ServerDebug_(("matchbox_open_link () - link daemon active"));
#endif

	return 1;
}

#if ANSI_prototypes
void matchbox_free_link (int tabno)
#else
void matchbox_free_link (tabno)
int tabno;
#endif
{
	/* kill link daemon */

	ld_alive_ = 0;

	while (ld_ack_ != 2)
		;
	ld_ack_ = 0;

	if (tsp_close ((int)(Matchbox_links[tabno].fildes)) == -1)
	{
		ServerDebug_(("matchbox_free_link () - problems closing link"));
	}

	Matchbox_links[tabno].fildes = -1;
}

#if ANSI_prototypes
void matchbox_reset (void)
#else
void matchbox_reset ()
#endif
{
	/*
	 * New, improved reset.
	 * 
	 * I can't simply reset the device as the link daemon is reading it.
 	 * Hence I first close the link, thus killing the daemon, and then
	 * open the link again, creating a new daemon.
	 */

#ifdef MBOX_DEBUG
	ServerDebug_(("matchbox_reset ()"));
#endif

	/* close the link */
	matchbox_free_link (current_link);

	/* reopen the link, which resets it */
	if (matchbox_open_link (current_link) == 0)
	{
		ServerDebug_(("matchbox_reset () - failed to reset the device"));

		Matchbox_links[current_link].fildes = -1;
	}
}

#if ANSI_prototypes
void matchbox_analyse (void)
#else
void matchbox_analyse ()
#endif
{
	if (tsp_analyse (link_fd))
	{
		ServerDebug_(("matchbox_analyse () - problems analysing matchbox"));
	}
}

#if ANSI_prototypes
int matchbox_rdrdy (void)
#else
int matchbox_rdrdy ()
#endif
{
#ifdef MBOX_DEBUG
	ServerDebug_(("matchbox_rdrdy ()"));
#endif
	/*
	 * Possible problems here with the daemon writing to the semaphore,
	 * just as I'm about to read it.
	 */

	/*
	 * First, is there a byte pending from the link daemon ?
	 */
	if (ld_semaphore_ == 1)
	{
#ifdef MBOX_DEBUG
	ServerDebug_(("matchbox_rdrdy () - byte pending, returning 1"));
#endif
		return 1;
	}

	/*
	 * Second, is the daemon currently stopped ?
	 *	If so start it going again ...
	 */
	if (ld_stop_ == 1)
	{
#ifdef MBOX_DEBUG
		ServerDebug_(("matchbox_rdrdy () - resetting ld_stop_"));
#endif
		ld_stop_ = 0;
	}

	/*
	 * ... and return false.
	 */
#ifdef MBOX_DEBUG
	ServerDebug_(("matchbox_rdrdy () - returning 0"));
#endif
	return  0;
}

#if ANSI_prototypes
int matchbox_wrrdy (void)
#else
int matchbox_wrrdy ()
#endif
{
	/* MatchBox is always read to receive data ? */
	return 1;
}

#if ANSI_prototypes
int matchbox_byte_to_link (int w)
#else
int matchbox_byte_to_link (w)
int    w;
#endif
{
	unsigned char	b = (unsigned char)(w);

	int	write_res;

	write_res = tsp_write (link_fd, (void *)(&b), 1, 0);

	if (write_res == -1)
	{
		ServerDebug_(("matchbox_byte_to_link () - failed to write 0x%02x to matchbox", b));

		return 1;
	}

	return 0;
}

#if ANSI_prototypes
int matchbox_byte_from_link (UBYTE * b)
#else
int matchbox_byte_from_link (b)
UBYTE *    b;
#endif
{
#ifdef MBOX_DEBUG
	ServerDebug_(("matchbox_byte_from_link () - waiting for link daemon"));
#endif

	/* wait for link daemon to retrieve a byte */
	while (ld_semaphore_ != 1)
		;

#ifdef MBOX_DEBUG
	ServerDebug_(("matchbox_byte_from_link () - link daemon read something"));

	ServerDebug_(("matchbox_byte_from_link () - ld_byte_ = 0x%02x", ld_byte_));
#endif

	*b = (UBYTE)(ld_byte_);

#ifdef MBOX_DEBUG
	ServerDebug_(("matchbox_byte_from_link () - *b = 0x%02x", *b));
#endif

	/* start up daemon again */
	ld_stop_ = 1;
	ld_semaphore_ = 0;

	return 0;
}


unsigned long	TotalNano	= 0;
unsigned long	TotalBytes	= 0;

#if ANSI_prototypes
int matchbox_send_block (int    amount,
             BYTE * buf,
             int    timeout)
#else
int matchbox_send_block (amount, buf, timeout)
int    amount;
BYTE *     buf;
int    timeout;
#endif
{
	int	byte_count;
	int	written_so_far = 0;
	int	retries = 0;

#ifdef MBOX_DEBUG
	ServerDebug_(("matchbox_send_block (%d, buf, timeout)", amount));

	ServerDebug_(("matchbox_send_block () - using fd %d", link_fd));
#endif
	timeout = timeout;

#ifdef MBOX_DEBUG
	if (amount == 4)
	{
		ServerDebug_(("matchbox_send_block () - [0x%02x, 0x%02x, 0x%02x, 0x%02x]", buf[0], buf[1], buf[2], buf[3]));
	}
#endif

	while (written_so_far < amount)
	{
		byte_count = tsp_write (link_fd, (void *)(&(buf[written_so_far])), amount - written_so_far, MBOX_TIMEOUT);

		if (byte_count == -1)
		{
			ServerDebug_(("matchbox_send_block () - write failed with -1"));

			return 1;
		}
		else if (byte_count == 0)
		{
			if (retries == MBOX_RETRIES)
			{
				ServerDebug_(("matchbox_send_block () - too many retries, failed to write after %d bytes", written_so_far));

				return 1;
			}
			else
			{
				ServerDebug_(("matchbox_send_block () - failed to write any bytes, retrying"));

				retries++;
			}
		}
		else
		{
#ifdef MBOX_DEBUG
			ServerDebug_(("matchbox_send_block () - successfully wrote %d bytes", byte_count));
#endif

			retries = 0;
			written_so_far += byte_count;
		}
	}

	return 0;
}

#if ANSI_prototypes
int matchbox_fetch_block (int    amount,
             BYTE * buf,
             int    timeout)
#else
int matchbox_fetch_block (amount, buf, timeout)
int    amount;
BYTE *     buf;
int    timeout;
#endif
{
	int	byte_count;
	int	read_so_far;
	int	retries = 0;

	timeout = timeout;

	/* wait to retrieve first byte from link daemon */

#ifdef MBOX_DEBUG
	ServerDebug_(("matchbox_fetch_block () - waiting for link daemon"));
#endif

	if (ld_stop_ == 1)
	{
		/*
		 *  Daemon currently stopped, so I can read all the bytes from the
		 *  link myself
		 */
		read_so_far = 0;
	}
	else
	{
		/*
		 * Daemon is running, so wait until it gets a byte
		 */
		while (ld_semaphore_ != 1)
			;

		/*
		 * Daemon is now stopped on ld_semaphore, so retrieve the byte it
		 * read, and read the rest myself.
		 */
#ifdef MBOX_DEBUG
		ServerDebug_(("matchbox_fetch_block () - link daemon retrieved 0x%02x", ld_byte_));
#endif
		buf[0] = (BYTE)ld_byte_;
		read_so_far = 1;
	}

	while (read_so_far < amount)
	{
		byte_count = tsp_read (link_fd, (void *)(&(buf[read_so_far])), amount - read_so_far, MBOX_TIMEOUT);

		if (byte_count == -1)
		{
			ServerDebug_(("matchbox_fetch_block () - read failed with -1"));

			return 1;
		}
		else if (byte_count == 0)
		{
			if (retries == MBOX_RETRIES)
			{
				ServerDebug_(("matchbox_fetch_block () - too many retries, failed to read after %d bytes", read_so_far));

				/* restart the daemon */
				ld_stop_ = 0;
				ld_semaphore_ = 0;

				return 1;
			}
			else
			{
				ServerDebug_(("matchbox_fetch_block () - failed to read any bytes, retrying"));

				retries++;
			}
		}
		else
		{
#ifdef MBOX_DEBUG
			ServerDebug_(("matchbox_fetch_block () - successfully read %d bytes", byte_count));
#endif

			retries = 0;
			read_so_far += byte_count;
		}
	}

	/*
	 * Make sure the daemon comes out of the semaphore block, but stop
	 * it from reading another byte - rdrdy () will start it off again
	 * if necessary.
	 */
	ld_stop_ = 1;		/* make sure daemon doesn't start reading yet and ... */
	ld_semaphore_ = 0;	/* ... tell it that we've got the byte 		      */

#ifdef MBOX_DEBUG
	if (amount == 4)
	{
		ServerDebug_(("[0x%02x, 0x%02x, 0x%02x, 0x%02x]", buf[0], buf[1], buf[2], buf[3]));
	}
#endif
	return 0;
}

