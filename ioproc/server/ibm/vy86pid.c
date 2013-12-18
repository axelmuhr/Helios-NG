/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1993, Perihelion Software Ltd.           --
--                          All Rights Reserved.                        --
--                                                                      --
--  vy86pid.c                                                           --
--                                                                      --
--  Author:  BLV 9/3/93                                                 --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id# */
/* Copyright (C) 1993, Perihelion Software Ltd. 			*/

/**
*** This module contains the PC link I/O routines for the VY86PID boards which
*** use RS232 lines for communication. 
**/

#include "../helios.h"

#define ITERATIONS	10000
static ComsPort *port;
static RS232	*rs232;

/**
*** Reset is a no-op (unless somebody wires up OUT1 or OUT2 appropriately)
**/
void vy86pid_reset(void)
{
}

/**
*** Analyse is impossible since the VY86PID board does not have a transputer.
**/
void vy86pid_analyse(void)
{
}

/**
*** All the data transfer routines involve setting up a full interrupt
*** driven transfer and polling until the transfer finishes.
**/
int vy86pid_byte_to_link(int x)
{
	int	i;
	word	result;
	char	buf[4];

	buf[0] = x;
	RS232_send(port, 1L, buf);

	for (i = 0; i < ITERATIONS; i++)
	{
		result = RS232_pollwrite(port);
		if (result >= 0L)
			break;
		elif (result == -1L)
			return(0);
	}
	RS232_abortwrite(port);
	return(1);
}

int vy86pid_byte_from_link(char *where)
{
	int	i;
	word	result;

	RS232_receive(port, 1L, where);  
	for (i = 0; i < ITERATIONS; i++)
	{
		result = RS232_pollread(port);
		if (result >= 0L)
			break;
		elif (result == -1L)
			return(0);
	}
	RS232_abortread(port);
	return(1);
}

int vy86pid_fetch_block(unsigned int count, char *buf, int timeout)
{
	word	result;
	word	iterations;
	uint	got;
	uint	prev_got;
	
	
	RS232_receive(port, (word) count, buf);

	got = 0;
	
	do
	  {
	    prev_got   = got;
	    iterations = ITERATIONS + (1600L * (word)count);
	    
	    while (iterations-- > 0L)
	      {
		result = RS232_pollread(port);
		if (result >= 0L)
			break;
		elif (result == -1L)
			return(0);
	      }
	    
	    got = rs232->incount;
	  }
	while (got > prev_got);
	
	result = RS232_abortread(port);
	return(count - (unsigned int) result);
}

int vy86pid_send_block(unsigned int count, char *buf, int timeout)
{
	word	result;
	word	iterations;

	iterations = ITERATIONS + (1600L * (word) count);

	RS232_send(port, (word) count, buf);
	while (iterations-- > 0)
	{
		result = RS232_pollwrite(port);
		if (result >= 0L)
			break;
		elif (result == -1L)
			return(0);
	}
	result = RS232_abortwrite(port);
	return(count - (unsigned int) result);
}

/**
*** rdrdy() is slightly complicated. The root processor may have sent
*** some excess data which would be held in the overflow buffer.
*** If not there may a character already in the receiver buffer register,
*** which involves checking the bottom bit of the Line Status register.
**/
int vy86pid_rdrdy(void)
{
	int		 x;

	if (rs232->overflow_count)
		return(1);

	return(0);
}

/**
*** wrrdy() simply involves checking the TXEmpty flag which is maintained
*** by the lower level RS232 code.
**/
int vy86pid_wrrdy(void)
{
	if (rs232->flags & RS232_TXEmpty)
		return(1);
	else
		return(0);
}

/**
*** Initialisation. This involves the following stages:
***
*** a) figuring out which com port to use (com1 and com2 are supported)
*** b) initialising the port, rs232 and portbase statics
*** c) set up an attributes structure. This involves further host.con
***    options for the baud rate and for the hardware handshake lines.
*** d) call the actual port initialisation routine.
**/
void vy86pid_init_link(void)
{
	char		*port_option;
	Attributes	 *attr;
	
	port	= &(RS232_coms[VY86PID_Port]);
	rs232	= &(RS232_table[port->id]);
	attr	= &(port->attr);

	InitAttributes(  attr);
	AddAttribute(	 attr, RS232_IgnPar);
	RemoveAttribute( attr, RS232_ParMark);
	RemoveAttribute( attr, RS232_InPck);
	RemoveAttribute( attr, RS232_IXON);
	RemoveAttribute( attr, RS232_IXOFF);
	RemoveAttribute( attr, RS232_Istrip);
	AddAttribute(	 attr, RS232_IgnoreBreak);
	RemoveAttribute( attr, RS232_BreakInterrupt);
	RemoveAttribute( attr, RS232_Cstopb);
	RemoveAttribute( attr, RS232_Cread);
	RemoveAttribute( attr, RS232_ParEnb);
	RemoveAttribute( attr, RS232_ParOdd);
	RemoveAttribute( attr, RS232_HupCl);		 
	AddAttribute(    attr, RS232_Csize_8);
	AddAttribute(    attr, RS232_CLocal);	/* hardware handshaking must be	*/
						/* disabled initially.		*/

		/* The initial speed should be 9600 for communication with	*/
		/* the on-board monitor. This is switched later on to whatever	*/
		/* host.con says.						*/
	SetInputSpeed(attr, RS232_B9600);

	RS232_configure(port);

		/* Update the baudrate field					*/
	port_option = get_config("vy86pid_baudrate");
	if (port_option == NULL)
		SetInputSpeed( attr, RS232_B9600);
	else
	{
		if (!mystrcmp(port_option, "9600"))
			SetInputSpeed( attr, RS232_B9600);
		elif (!mystrcmp(port_option, "19200"))
			SetInputSpeed( attr, RS232_B19200);
		elif (!mystrcmp(port_option, "38400"))
			SetInputSpeed( attr, RS232_B38400);
		elif (!mystrcmp(port_option, "56000"))
			SetInputSpeed( attr, RS232_B56000);
		else
		{
			ServerDebug("invalid host.con entry for vy86pid_baudrate, should be 9600, 19200, 38400 or 56000");
			longjmp(exit_jmpbuf, 1);
		}
	}
}

/**
*** Routines to interrogate and update the baud rate.
**/
void vy86pid_set_baudrate(word rate, bool fifoon)
{
	if (rate == 9600L)
		SetInputSpeed(&(port->attr), RS232_B9600);
	elif (rate == 19200L)
		SetInputSpeed(&(port->attr), RS232_B19200);
	elif (rate == 38400L)
		SetInputSpeed(&(port->attr), RS232_B38400);
	elif (rate == 56000L)
		SetInputSpeed(&(port->attr), RS232_B56000);
	else
	{
		ServerDebug("vy86pid_set_baudrate, internal error, illegal baud rate %ld", rate);
		longjmp(exit_jmpbuf, 1);
	}

	RS232_configure(port);

	if (get_config("vy86pid_use_fifo") != NULL)
	{
		RS232_control_fifo(port, fifoon);
	}
}

#if 0
word vy86pid_get_baudrate(void)
{
	word	x = GetInputSpeed(&(port->attr));

	if (x == RS232_B9600)
		return(9600L);
	elif (x == RS232_B19200)
		return(19200L);
	elif (x == RS232_B38400)
		return(38400L);
	elif (x == RS232_B56000)
		return(56000L);
	else
	{
		ServerDebug("vy86pid support, internal error, the baud rate has been changed");
		longjmp(exit_jmpbuf, 1);
	}
}
#endif

word vy86pid_get_configbaud(void)
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
	elif (!mystrcmp(baudrate, "56000"))
		return 56000L;
	else
	{
		ServerDebug("invalid host.con entry for vy86pid_baudrate, should be 9600, 19200, 38400 or 56000");
		longjmp(exit_jmpbuf, 1);
	}
}

/**
*** Routine to enable use of hardware handshaking lines. This only
*** happens if there is a vy86pid_handshake option in the host.con
*** file. Hardware handshaking cannot be enabled until after the
*** nucleus has been downloaded and the bootrom has transferred
*** control to the kernel, since the bootrom does not understand
*** about hardware handshaking.
**/
void vy86pid_setup_handshake(void)
{
	if (get_config("vy86pid_handshake") != NULL)
	{
		RemoveAttribute(&(port->attr), RS232_CLocal);
		RS232_configure(port);
	}
}
