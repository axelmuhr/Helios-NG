/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--         Copyright (C) 1993, Perihelion Distributed Software Ltd.     --
--                        All Rights Reserved.                          --
--                                                                      --
--      vy86pid.c                                                       --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: vy86pid.c,v 1.7 1994/07/04 14:53:22 tony Exp $ */
/* Copyright (C) 1989, Perihelion Distributed Software Ltd.   */

#define Linklib_Module

#include "../helios.h"

#ifdef SUN3
#include <termio.h>	/* ??? */
#endif

#include <sys/ttold.h>

#ifndef O_NOCTTY
#define O_NOCTTY 0
#endif

#define link_fd (int)(link_table[current_link].fildes)

/**
*** The vy86pid boards are accessed via serial lines, /dev/ttya etc.
*** The exact names of these serial lines vary from machine to machine.
*** I allow for upto ten of them. The user will have to specify a
*** site in the host.con file to get the right tty device.
**/

#if (ARMBSD)
#define VY86PID_Max_Link 1
#else
#define VY86PID_Max_Link 10
#endif

#if SOLARIS
# define VY86PID_MAX_RETRIES	512
#else
# define VY86PID_MAX_RETRIES	16
#endif

PRIVATE Trans_link	VY86PID_links[VY86PID_Max_Link];
PRIVATE struct termios	saved_termios;
PRIVATE struct termios	current_termios;

/* #define DEBUG_RETRIES */

#ifdef DEBUG_RETRIES
long	InitReadRetries, InitWriteRetries;

long	MaxReadRetries, MaxWriteRetries;

long	ReadAttempts, WriteAttempts;
long	ReadRetries, WriteRetries;
#endif

void vy86pid_init_link()
{
	int i;
  
	number_of_links = VY86PID_Max_Link;
	link_table = &(VY86PID_links[0]);

#ifdef DEBUG_RETRIES
	InitReadRetries = InitWriteRetries = MaxReadRetries = MaxWriteRetries = -1;

	ReadAttempts = WriteAttempts = ReadRetries = WriteRetries = 0;
#endif

	if (Server_Mode eq Mode_Daemon)
	{
		ServerDebug("Hydra: the link daemon cannot support the vy86pid board.");
		/* longjmp(exit_jmpbuf, 1); */
		longjmp_exit;
	}
	
	for (i = 0; i < VY86PID_Max_Link; i++)
	{
#if (SUN3 || SUN4)
		sprintf(VY86PID_links[i].link_name, "/dev/tty%c", i + 'a');
#endif
#if (HP9000)
		sprintf(VY86PID_links[i].link_name, "/dev/tty%02d", i);
#endif
#if (RS6000)
		sprintf(VY86PID_links[i].link_name, "/dev/tty%d", i);
#endif
#if (ARMBSD)
		sprintf(VY86PID_links[i].link_name, "/dev/lserial");
#endif
		VY86PID_links[i].flags      = Link_flags_unused +
			Link_flags_uninitialised + Link_flags_firsttime;
		VY86PID_links[i].connection = -1;
		VY86PID_links[i].fildes     = -1;
		VY86PID_links[i].state      = Link_Reset;
		VY86PID_links[i].ready      = 0;
	}

		/* Discard any non-existant tty devices.		*/
	for ( ; number_of_links >= 0; number_of_links--)
	{
		struct stat buf;

		if (stat(link_table[number_of_links-1].link_name, &buf) eq 0)
			break;		/* OK, found the last known site */
		if (errno ne ENOENT)	/* Appears to exist, but not currently usable */
			break;
	}
}

/**
*** Opening a link means opening the appropriate tty device. Non-blocking mode
*** is used and I do not want the VY86PID board to become the controlling terminal.
*** Various ioctl() calls are needed to set up the tty correctly. Some of these calls
*** are almost certainly machine-specific.
**/
#if ANSI_prototypes
int vy86pid_open_link (int tabno)
#else
int vy86pid_open_link(tabno)
int tabno;
#endif
{
	int		 j;
	char		*config_option;

/*	printf ("vy86pid_open_link () - opening %s\n", link_table[tabno].link_name); */
	
	link_table[tabno].fildes = open(link_table[tabno].link_name, O_RDWR | O_NDELAY | O_NOCTTY);
	if (link_table[tabno].fildes eq -1)
		return(0);

		/* Get exclusive use of this tty line.				*/
	if (ioctl((int)(link_table[tabno].fildes), TIOCEXCL, 0) < 0)
		goto fail;
		
		/* Do not pass this file descriptor on to child processes.	*/
	fcntl((int)(link_table[tabno].fildes), F_SETFD, 1);

		/* Ignore the DCD line.						*/
	j = 1;
	if (ioctl((int)(link_table[tabno].fildes), TIOCSSOFTCAR, &j) < 0)
		goto fail;

#ifdef TCIOFLUSH
		/* Clear out anything still in the buffers. Ideally this would	*/
		/* be followed by a reset.					*/
	if (ioctl((int)(link_table[tabno].fildes), TCFLSH, TCIOFLUSH) < 0)
		goto fail;
#endif

	if (ioctl((int)(link_table[tabno].fildes), TCGETS, &current_termios) < 0)
		goto fail;
	memcpy(&saved_termios, &current_termios, sizeof(struct termios));
	
	current_termios.c_iflag		|= IGNBRK;
	current_termios.c_iflag 	&= ~(BRKINT | IGNPAR | PARMRK | INPCK | ISTRIP | INLCR |
				     IGNCR | ICRNL | IUCLC | IXON | IXANY | IXOFF | IMAXBEL);
	current_termios.c_oflag		 = 0;	/* too many options to disable explicitly */
	current_termios.c_cflag		|= (CS8 | CREAD | CLOCAL);
	current_termios.c_cflag		&= ~(CBAUD | CSTOPB | PARENB | HUPCL | CIBAUD | CRTSCTS);
	current_termios.c_lflag		 = 0;

		/* non-canonical input is used, and the following parameters impose	*/
		/* a two second timeout on read operations. In practice the device is	*/
		/* opened with O_NDELAY so these parameters are probably irrelevant.	*/
	current_termios.c_cc[VMIN]	 = 0;
	current_termios.c_cc[VTIME]	 = 20;

		/* The initial baud rate should be 9600 for the PISD protocol.	*/
		/* This is updated later on by a call to vy86pid_set_baudrate().*/
	current_termios.c_cflag	|= B9600;
	

	if (ioctl((int)(link_table[tabno].fildes), TCSETS, &current_termios) < 0)
		goto fail;

		/* After the termios options have been set I update the		*/
		/* baud rate in the data structure. This can then be set up	*/
		/* in vy86pid_set_baudrate() below.				*/
	config_option = get_config("vy86pid_baudrate");
	if (config_option == NULL)
		current_termios.c_cflag	|= B9600;
	else
	{
		if (!mystrcmp(config_option, "9600"))
			current_termios.c_cflag |= B9600;
		elif (!mystrcmp(config_option, "19200"))
			current_termios.c_cflag |= B19200;
		elif (!mystrcmp(config_option, "38400"))
			current_termios.c_cflag |= B38400;
		else
		{
			ServerDebug("invalid host.con entry for vy86pid_baudrate, should be 9600, 19200 or 38400");
			/* longjmp(exit_jmpbuf, 1); */
			longjmp_exit;
		}
	}
	
/*	printf ("vy86pid_open_link () - succeeded\n"); */
	return(1);

fail:
/*	printf ("vy86pid_open_link () - failed\n"); */
		/* This tty line could not be initialised.			*/
	close((int)(link_table[tabno].fildes));
	link_table[tabno].fildes = -1;
	return(0);
} 

/**
*** Once the PISD protocols have been used to download the nucleus the
*** baud rate should be updated to whatever host.con says, to match the
*** comms code in the nucleus.
**/
#if ANSI_prototypes
void vy86pid_set_baudrate(word	rate,
			  bool	fifo)
#else
void vy86pid_set_baudrate(rate, fifo)
word rate;
bool fifo;	/* something bart calls in tload.c, but apparently doesn't use */
#endif
{
	current_termios.c_cflag &= ~CBAUD;
	switch(rate)
	{
	case 9600 :
		current_termios.c_cflag |= B9600; break;
	case 19200 :
		current_termios.c_cflag |= B19200; break;
	case 38400 :
		current_termios.c_cflag |= B38400; break;
	default:
		ServerDebug("vy86pid_set_baudrate, internal error, illegal baud rate %d", rate);
		/* longjmp(exit_jmpbuf, 1); */
		longjmp_exit;
	}
	
	ioctl((int)(link_table[current_link].fildes), TCSETS, &current_termios);
}

/**
*** Enable hardware handshaking if appropriate.
**/
void vy86pid_setup_handshake()
{
	if (get_config("vy86pid_handshake") != NULL)
	{
	  int	j;
	  current_termios.c_cflag |= CRTSCTS;
	  current_termios.c_cflag &= ~CLOCAL;
	  ioctl((int)(link_table[current_link].fildes), TCSETS, &current_termios);
	  j = 1;
	  ioctl((int)(link_table[current_link].fildes), TIOCSSOFTCAR, &j);
	}

}
#if 0
/**
*** Also, the bootstrap code should be able to determine the baud rate
*** to update the monitor running in the board.
**/
word vy86pid_get_baudrate()
{
	switch(current_termios.c_cflag & CBAUD)
	{
	case B9600 : return(9600);
	case B19200: return(19200);
	case B38400: return(38400);
	default:
		ServerDebug("VY86PID support, internal error, the baud rate has been changed.");
		/* longjmp(exit_jmpbuf, 1); */
		longjmp_exit;
	}
}
#endif

word vy86pid_get_configbaud()
{
	char *baudrate = get_config("vy86pid_baudrate");

	if (baudrate == NULL)
		return 9600L;
	elif (!mystrcmp(baudrate, "9600"))
		return 9600L;
	elif (!mystrcmp(baudrate, "19200"))
		return 19200L;
	elif (!mystrcmp(baudrate, "38400"))
		return 38400L;
	else
	{
		ServerDebug("invalid host.con entry for vy86pid_baudrate, should be 9600, 19200 or 38400");
		/* longjmp(exit_jmpbuf, 1); */
		longjmp_exit;
	}
}

/**
*** releasing the link means restoring the termios values, just to be polite, and
*** closing the file descriptor.
**/
#if ANSI_prototypes
void vy86pid_free_link(int	tabno)
#else
void vy86pid_free_link(tabno)
int tabno;
#endif
{
	if (link_table[tabno].fildes != -1)
	{
		(void) ioctl((int)(link_table[tabno].fildes), TCSETS, &saved_termios);
		 close((int)(link_table[tabno].fildes));
	}
	link_table[tabno].fildes = -1;

#ifdef DEBUG_RETRIES
	printf ("initial read retries: %d, initial write retries: %d\n", InitReadRetries, InitWriteRetries);

	printf ("maximum read retries: %d, maximum write retries: %d\n", MaxReadRetries, MaxWriteRetries);

	printf ("read attempts: %d, read retries: %d\n", ReadAttempts, ReadRetries);
	printf ("write attempts: %d, write retries: %d\n", WriteAttempts, WriteRetries);

	if (ReadAttempts == 0)
	{
		printf ("average read retries: NO READS ATTEMPTED, ");
	}
	else
	{
		printf ("average read retries: %d, ", ReadRetries/ReadAttempts);
	}

	if (WriteAttempts == 0)
	{
		printf ("average write retries: NO WRITES ATTEMPTED\n");
	}
	else
	{
		printf ("average write retries: %d\n", WriteRetries/WriteAttempts);
	}
#endif
}

/**
*** Resetting a processor is a no-op, unless at some future point one of the
*** handshake or spare lines is used as a reset signal. Analyse is not possible
*** since the vy86pid board does not have a transputer.
**/
void vy86pid_reset_processor()
{
}

void vy86pid_analyse_processor()
{
}

/**
*** rdrdy() can be implemented using a select.
**/
int vy86pid_rdrdy( )
{
	fd_set		rdmask;	
	struct timeval	timelim;

	FD_ZERO(&rdmask);
	FD_SET(link_fd,&rdmask);
	timelim.tv_sec  = 0;
	timelim.tv_usec = 1;
	if(select(link_fd + 1, &rdmask, NULL, NULL, &timelim) < 1)
		return(FALSE);
	return(TRUE);
}

/**
*** There is an ioctl() to determine the number of characters still buffered up
*** in the output queue. If there are any characters still buffered up then
*** the root processor may have stopped receiving, so wrrdy() is set to failure.
**/
int vy86pid_wrrdy( )
{
	int	x	= 0;

	if (ioctl(link_fd, TIOCOUTQ, &x) < 0)
		return(FALSE);
	if (x > 0)
		return(FALSE);
	return(TRUE);
}

/**
*** byte_from_link() involves a simple read(), with a couple of retries to allow for
*** signals etc.
**/
#if ANSI_prototypes
int vy86pid_byte_from_link(UBYTE *	where)
#else
int vy86pid_byte_from_link(where)
UBYTE	*where;
#endif
{
	int	retries;

#ifdef DEBUG_RETRIES
	ReadAttempts++;
#endif

	for (retries = 0; retries < VY86PID_MAX_RETRIES; retries++)
	{
#ifdef DEBUG_RETRIES
		ReadRetries++;
#endif

		if (read(link_fd, where, 1) > 0)
		{
#ifdef DEBUG_RETRIES
			if (InitReadRetries == -1) InitReadRetries = ReadRetries;

			if (MaxReadRetries < retries) MaxReadRetries = retries;
#endif

			return(0);
		}
	}

#ifdef DEBUG_RETRIES
	if (InitReadRetries == -1) InitReadRetries = ReadRetries;
	if (MaxReadRetries < retries) MaxReadRetries = retries;
#endif

	return(1);
}

/**
*** byte_to_link() is almost identical.
**/

#if ANSI_prototypes
int vy86pid_byte_to_link(int	x)
#else
int vy86pid_byte_to_link(x)
int	x;
#endif
{
	int	retries;
	char	buf[4];

#ifdef DEBUG_RETRIES
	WriteAttempts++;
#endif

	buf[0]	= x;
	for (retries = 0; retries < VY86PID_MAX_RETRIES; retries++)
	{
#ifdef DEBUG_RETRIES
		WriteRetries++;
#endif

		if (write(link_fd, buf, 1) > 0)
		{
#ifdef DEBUG_RETRIES
			if (InitWriteRetries == -1) InitWriteRetries = WriteRetries;
			if (MaxWriteRetries < retries) MaxWriteRetries = retries;
#endif
			return(0);
		}
	}


#if (SUN4)
	tcdrain(link_fd);
#endif
#if (SUN3)
	ioctl(link_fd, TCSBRK, 1);
#endif

#ifdef DEBUG_RETRIES
	if (InitWriteRetries == -1) InitWriteRetries = WriteRetries;
	if (MaxWriteRetries < retries) MaxWriteRetries = retries;
#endif

	return(1);
}

/**
*** send_block() and fetch_block() are essentially the same as
*** socket_read() and socket_write(). If any data at all is read
*** or written then the current read or write is not treated as a
*** failure, to cope with the very low speed of the device.
**/
#if ANSI_prototypes
int vy86pid_send_block(int	amount,
		       char *	buf,
		       int	timeout)
#else
int vy86pid_send_block(amount, buf, timeout)
int	amount;
char *	buf;
int	timeout;
#endif
{
	int	written_so_far	= 0;
	int	retries		= 0;

#ifdef DEBUG_RETRIES
	WriteAttempts++;
#endif
	
	while (written_so_far < amount)
	{
		int	x;
		x	= write(link_fd, &(buf[written_so_far]), amount - written_so_far);

		if (x > 0)
		{
			written_so_far	+= x;
			continue;
		}
		elif ((x == 0) || ((x < 0) && (errno == EINTR)))
		{
			if (++retries < VY86PID_MAX_RETRIES)
			{
#ifdef DEBUG_RETRIES
				WriteRetries++;
#endif
				continue;
			}
			else
			{
#ifdef DEBUG_RETRIES
				if (InitWriteRetries == -1) InitWriteRetries = WriteRetries;
				if (MaxWriteRetries < retries) MaxWriteRetries = retries;
#endif
				return(amount - written_so_far);
			}
		}
		else
		{
#ifdef DEBUG_RETRIES
			if (InitWriteRetries == -1) InitWriteRetries = WriteRetries;
			if (MaxWriteRetries < retries) MaxWriteRetries = retries;
#endif
			return(amount - written_so_far);
		}
	}

#if (SUN4)
	tcdrain(link_fd);
#endif
#if (SUN3)
	ioctl(link_fd, TCSBRK, 1);
#endif

#ifdef DEBUG_RETRIES
	if (InitWriteRetries == -1) InitWriteRetries = WriteRetries;
	if (MaxWriteRetries < retries) MaxWriteRetries = retries;
#endif

	return(0);
}

#if ANSI_prototypes
int vy86pid_fetch_block(int	amount,
			char *	buf,
			int	timeout)
#else
int vy86pid_fetch_block(amount, buf, timeout)
int	amount;
char *	buf;
int	timeout;
#endif
{
	int	read_so_far	= 0;
	int	retries		= 0;

#ifdef DEBUG_RETRIES
	ReadAttempts++;
#endif	

	while (read_so_far < amount)
	{
		int	x;
		x	= read(link_fd, &(buf[read_so_far]), amount - read_so_far);

		if (x > 0)
		{
			read_so_far	+= x;
			continue;
		}
		elif ((x == 0) || ((x < 0) && (errno == EINTR)))
		{
			if (++retries < VY86PID_MAX_RETRIES)
			{
#ifdef DEBUG_RETRIES
				ReadRetries++;
#endif
				continue;
			}
			else
			{
#ifdef DEBUG_RETRIES
				if (InitReadRetries == -1) InitReadRetries = ReadRetries;
				if (MaxReadRetries < retries) MaxReadRetries = retries;
#endif
				return(amount - read_so_far);
			}
		}
		else
		{
#ifdef DEBUG_RETRIES
			if (InitReadRetries == -1) InitReadRetries = ReadRetries;
			if (MaxReadRetries < retries) MaxReadRetries = retries;
#endif
			return(amount - read_so_far);
		}
	}

#ifdef DEBUG_RETRIES
	if (InitReadRetries == -1) InitReadRetries = ReadRetries;
	if (MaxReadRetries < retries) MaxReadRetries = retries;
#endif

	return(0);
}
