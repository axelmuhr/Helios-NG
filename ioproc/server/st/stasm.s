/*-------------------------------------------------------------------------
/--                                                                      --
/--          H E L I O S   I N P U T / O U T P U T   S E R V E R	 --
/--          ---------------------------------------------------	 --
/--                                                                      --
/--             Copyright (C) 1987, Perihelion Software Ltd.             --
/--                        All Rights Reserved.                          --
/--                                                                      --
/--      localasm.s                                                      --
/--                                                                      --
/--         This module contains all the 68000 assembler needed for the  --
/--                                                                      --
/--         ST version of the IO server.                                 --
/--                                                                      --
/--	Author:  BLV 22/5/88						 --
/--                                                                      --
/------------------------------------------------------------------------*/
/* SccsId: 3.7 14/4/89 Copyright (C) 1987, Perihelion Software Ltd.		 */


/*-------------------------------------------------------------------------
/--                                                                      --
/--      ncolib                                                          --
/--                                                                      --
/--         Basic coroutine library functions, for use with Mark         --
/--                                                                      --
/--         Williams C on the Atari ST. Ported from a version for the    --
/--                                                                      --
/--         Amiga by Nick Garnett.                                       --
/--                                                                      --
/------------------------------------------------------------------------*/

co_sp      = 0          / offsets within a structure
co_parent  = 4
co_func    = 8
co_size    = 12
co_magic   = 14
co_SIZEOF  = 16         / size of the structure

magic_value = 7654

/ external routines used
.globl   get_mem_,free_mem_

/ these allow access from C
.globl   InitCo_,CreateCo_,CallCo_,WaitCo_,DeleteCo_
.globl   CurrentCo_


CreateCo_:
      movea.l 4(sp),a0         / function
      move.l  8(sp),d0         / NB a long, not an int

/==============================================================================
/ coroutine = CreateCo( function,size )
/    d0                   a0     d0
/
/ Creates and adds a co-routine with the required stacksize and start function.
/ size is in bytes
/==============================================================================
CreateCo:  movem.l ${d3-d7,a3-a6},-(sp)       / save all registers
           movea.l  a0,a3              / stash start routine address
           add.l    $co_SIZEOF,d0      / add coroutine structure size
           move.w   d0,-(sp)
	   jsr      get_mem_            / get the memory from the system
           clr.l    d2                 / clear leading bits
	   move.w   (sp)+,d2
           tst.l    d0                 / if any is left
           bne.s    L2                         / got it with no problems
           movem.l  (sp)+,${d3-d7,a3-a6}       / else, restore registers
           rts                                 / return with d0 = 0

L2:        movea.l  d0,a1              / a1 = new coroutine base
           movea.l  CurrentCo_,a0      / a0 = current coroutine
           move.w   d2,co_size(a1)     / set stack size
           move.w   $magic_value,co_magic(a1)
           move.l   a0,co_parent(a1)   / set parent to current co
           move.l   a3,co_func(a1)     / set function pointer
           add.l    a1,d2              / d2 = sp of new co-routine
           move.l   a1,CurrentCo_      / make new coroutine current
           move.l   sp,co_sp(a0)       / save sp, parent is now suspended
           movea.l  d2,sp              / swap to new stack
           move.l   a1,d0              / return new coroutine address
           movea.l  a1,a3              / and save it for us too
L1:        bsr.s    WaitCo             / waitco back to parent
           movea.l   co_func(a3),a0    / a0 = initial function
           jsr      (a0)               / call it (Arg in d0)
           bra.s    L1                 / and loop forever if it returns

CallCo_:
           movea.l  4(sp),a0
           move.l   8(sp),d0           / NB a long, not an int

/==============================================================================
/ Result = CallCo( coroutine,arg )
/   d0                 a0    d0
/
/ Starts up a coroutine that was just created or did a WaitCo to return an arg.
/==============================================================================
CallCo:     movea.l a0,a1              / stash entering address
            movea.l CurrentCo_,a0      / a0 = current coroutine
            move.l  a0,co_parent(a1)   / adopt the coroutine

/ coming here we assume:
/ D0 = return/argument value
/ A0 = coroutine we are leaving
/ A1 = coroutine we are entering
coenter:     movem.l ${d3-d7,a3-a6},-(sp)      / save this coroutines registers

             cmpi.w  $magic_value,co_magic(a0)
             bne     coroutines_crashed
             cmpi.w  $magic_value,co_magic(a1)
             bne     coroutines_crashed
             
             move.l  sp,co_sp(a0)              / save stack pointer
             movea.l co_sp(a1),sp              / move to new coroutine stack
             movem.l (sp)+,${d3-d7,a3-a6}      / restore registers
             move.l  a1,CurrentCo_             / set new current coroutine
             rts                               / and go into it

WaitCo_:
             move.l   4(sp),d0       / NB a long, not an int

/==============================================================================
/ Arg = WaitCo( arg )
/ d0            d0
/
/ Returns control back to the parent with required argument/return code in d0.
/ Arg will eventually be returned when the coroutine doing the WaitCo is called
/ again with CallCo(coroutine,ARG) or ResumeCo(coroutine,ARG)
/==============================================================================
WaitCo:
      movea.l   CurrentCo_,a0         / a0 = current coroutine
      movea.l   co_parent(a0),a1      / a1 = parent
      clr.l     co_parent(a0)         / become an orphan
      bra.s     coenter               / go do switch

DeleteCo_:
      movea.l 4(sp),a0

/==============================================================================
/ success = DeleteCo( coroutine )
/   d0                 a0
/
/ Deletes the stack area being used by a coroutine that is no longer needed.
/==============================================================================
DeleteCo:
      cmpa.l   $0,a0               / in case we are passed a 0
      beq      DeleteFail          / we were, quit now

      cmpi.w    $magic_value,co_magic(a0)
      bne      coroutines_crashed
      move.l   a0,-(sp)
      jsr      free_mem_               / free it using C's free()
      addq     $4,sp
DeleteFail:
      rts


/==============================================================================
/ Success = InitCo()
/   d0
/
/ Initialises a root co-routine that never goes away.  It corresponds directly
/ to the main level of the program and is really just a list header for all
/ other co-routines that get started.  The memory allocation could go in the
/ main allocator in Init() but I've left it here for clarity.
/==============================================================================
InitCo_: 
      move.l    a6,-(sp)
      moveq.l   $co_SIZEOF,d0                / get root co-routine
      move.w   d0,-(sp)
      jsr      get_mem_                       / obtain the memory  
      move.w   (sp)+,d2
      tst.l    d0
      beq.s    L20                           / it didn't work so fail
      movea.l  d0,a0
      move.l   a0,CurrentCo_                 / and sometimes here
      move.l   a0,co_parent(a0)              / I'm my own parent
      move.w   $magic_value,co_magic(a0)
      moveq.l  $-1,d0                        / return success
L20:  movea.l   (sp)+,a6
      rts



cotext1:
.ascii "*** Internal error : coroutine stack overflow detected.\n\r\000"
.even
cotext2:
.ascii "***                : attempting to exit safely.\n\r\000"
.even

			/ If the coroutine library gets corrupted it jumps here
coroutines_crashed:
     pea	cotext1		/ display message 1
     move.w     $9,-(sp)
     trap       $1
     addq.l     $6,sp
     pea	cotext2		/ and message 2
     move.w     $9,-(sp)
     trap       $1
     addq.l	$6,sp

     clr.w      -(sp)		/ and exit
     trap       $1

CurrentCo_: .long  1

	
/ ------------------------------------------------------------------------
/ events
/ ======
/
/	Interrupt routines to handle keyboard and mouse events.
/	Other devices (joystick, midi) may be added later.
/
/	Author: BLV	04/02/88
/
/ ------------------------------------------------------------------------

/
/ Since I dislike large quantities of assembler, my interrupt routines tend to
/ call C routines in module st/local.c as soon as possible. This may not
/ be as efficient as possible, and there are limits to what I can do in the
/ C routines, but it still makes my life a lot easier.
/

	.globl	mouse_event_		/ the C routine called by ...
	.globl	mouse_int_		/ the assembler interrupt

	.globl	keyboard_event_		/ ditto for the keyboard
	.globl	keyboard_int_
	.globl	keyboard_reset_		/ for when the ACIA overruns

/
/ The mouse interrupt routine only handles mouse movements. It is
/ installed by BIOS Initmous() and I specify that the mouse buttons should
/ be key events, because I can find no way of catching button up and down
/ in any other way.
/
/ The mouse code has been taken from Gem's catch.s, and is magic as far as
/ I am concerned.
/
	.even
mouse_int_:
	movea.l	4(sp),a0
	moveq	$0,d0			/ clear the top bytes
	moveq	$0,d1			/ of x and y values
	move.b	1(a0),d0		/ Load the x coordinate
	ext.w	d0			/ as an int (16 bit)
	move.b	2(a0),d1		/ Similarly for the y coordinate
	ext.w	d1
	movem.w ${d0-d1},-(sp)		/ Push as parameters to mouse_event

	jsr	mouse_event_
	addq.l	$4,sp
	rts

/
/ Much of the following code has been lifted from the ST's bios listing,
/ and appears to work happily which is all I am worried about.
/

keyboard	=	0xfffc00
comstat		=	0
iodata		=	2

	.even

keyboard_int_:
	movea.l	$keyboard,a1		/ point to keyboard register base

	move.b	comstat(a1),d2		/ grab device status
	btst	$7,d2			/ make sure it was an interrupt request
	beq	aciaexit		/ nope...it's empty
	btst	$0,d2			/ see if receiver buffer is full
	beq	mk1			/ nope...it's empty

	movem.l	${d2,a1},-(sp)		/ save the old ACIA status
	clr.w	d2
	move.b	iodata(a1),d2		/ grab data byte from acia data register
	move.w	d2,-(sp)
	jsr	keyboard_event_
	addq.l	$2,sp
	movem.l	(sp)+,${d2,a1}		/ restore ACIA status

mk1:	andi.b	$0x20,d2		/ mask off bits already tested
	beq	aciaexit		/ check the overflow flag
	jsr	keyboard_reset_		/ an overrun occurred, so recover

aciaexit:
	rts

		
/////////////////////////////////////////////////////////////////////////////
/
/ messlib.s
/
/ Defines sendmsg and getmsg for sending arbitrary amounts of data between
/ the transputer and the ST by using Helios message protocols.
/
/  AE 	02-OCT-87	: created
/  TJK  17-OCT-87	: altered error codes & tidies
/  BLV  23-NOV-87	: removed sendmsg and getmsg, redundant
/  BLV  22-MAY-88       : made part of localasm.s, removed getports, initmsg
/
//////////////////////////////////////////////////////////////////////////////

	.globl	GetMsg_
	.globl	PutMsg_

/ define trap 12 opcodes

	GETM	= 3
	PUTM	= 4

////////////////////////////////////////////////////////////////////////////////
/ int GetMsg( &mcb )
/
/ MCB mcb
/
/ Reads a full Helios message into the message control port given
/ as argument. Returns 1 (true) if it worked and 0 (false) otherwise
/
////////////////////////////////////////////////////////////////////////////////
GetMsg_:
	moveq	$GETM,d1	/ GetMsg opcode
	movea.l	4(sp),a0	/ a0 is pointer to mcb
	trap	$12		/ do it
	rts			/ return to caller

////////////////////////////////////////////////////////////////////////////////
/ int PutMsg( &mcb )
/
/ MCB mcb
/
/ Writes a full Helios message from the message control port given
/ as argument. Returns 1 (true) if it worked and 0 (false) otherwise
/
////////////////////////////////////////////////////////////////////////////////

PutMsg_:
	moveq	$PUTM,d1	/ GetMsg opcode
	movea.l	4(sp),a0	/ a0 is pointer to mcb
	trap	$12		/ do it
	rts			/ return to caller

////////////////////////////////////////////////////////////////////////////
/
/  File sxplib.s
/
/  Changes:
/	AE  17-Aug-87	:created
/	AE  01-Oct-87	:rewritten to use DMAC version of trap#8
/	AE  13-Oct-87	:now uses Perihelion/Kuma version of trap#8
/	AE  19-Oct-87	:now includes dbrdint() & dbwrint()
/       BLV 22-May-88   :made part of localasm.s
/
/  Description:
/	Library procedures to interface to a transputer via DMAC & link adapter.
/	These routines are 68000 assembler implementaions of the 'C' routines
/	which were defined in the file xplib.c. All these routines are built
/	upon the primitive communication routines which have been incorporated
/	in the trap#8 routine, as implemented in the file trap1.s.
/
/
/////////////////////////////////////////////////////////////////////////////

	.shri

	.globl	xpwrbyte_
	.globl	xpwrrdy_
	.globl	xprdbyte_
	.globl	xprdrdy_
	.globl	xpwrword_
	.globl	xprdword_
	.globl	xpwrint_
	.globl	xprdint_
	.globl	xpwrdata_
	.globl	xprddata_
	.globl	xpreset_
	.globl	resetlnk_
	.globl	xpanalyse_
	.globl	dbwrword_
	.globl	dbrdword_
	.globl	dbrdint_
	.globl	dbwrint_


/ The trap used to call the direct I/O operations

	TRAPIO	= 8


/ Opcodes for trap operations

	READ	= 1	/ read a byte from the transputer
	WRITE	= 2	/ write a byte to the transputer
	RESET	= 3	/ reset the transputer & the link adapter
	ANALYSE	= 4	/ analyse the transputer
	RESETLNK= 5 	/ reset link adapter
	WRRDY	= 6	/ write ready?
	RDRDY	= 7	/ read ready?


/ define constant returned to caller on successful completion of functio
	SUCCESS	=  1	/ 'C' boolean (non zero value = TRUE)
	FAIL	=  0	/ 'C' boolean 

	.shri


//////////////////////////////////////////////////////////////////////////////
/
/ xpreset
/
/ Resets the transputer and the Perihelion link adapter.
/
/ Synopsis:	void xpreset()
/
//////////////////////////////////////////////////////////////////////////////
xpreset_:
	moveq.l	$RESET,d1
	trap	$TRAPIO
	rts

//////////////////////////////////////////////////////////////////////////////
/
/ xpanalyse
/
/ Analyses the transputer.
/
/ Synopsis:	void xpanalyse()
/
//////////////////////////////////////////////////////////////////////////////
xpanalyse_:
	moveq.l	$ANALYSE,d1
	trap	$TRAPIO
	rts

	.prvd
temp:
	.byte	0
	.even
	.prvi

//////////////////////////////////////////////////////////////////////////////
/
/ xprdbyte
/
/ Reads a single byte from the transputer.
/
/ Synopsis:	WORD xprdbyte ()
/
//////////////////////////////////////////////////////////////////////////////
xprdbyte_:
	moveq.l	$1,d0		/ d0 contains bytecount
	moveq.l	$READ,d1	/ d1 is an opcode
	movea.l	$buffer,a0	/ pointer to data area
	trap	$TRAPIO		/ perform read operation and store result in d0
	moveq.l	$0,d0		/ clear d0
	move.b	buffer,d0	/ return retrieved data to caller
	rts

//////////////////////////////////////////////////////////////////////////////
/
/ xpwrbyte
/
/ Sends a single byte to the transputer, and returns error flag: -1 = error.
/ 0 = successful termination.
/
/ Synopsis:	WORD xpwrbyte(b)
/ 		WORD b;
/
//////////////////////////////////////////////////////////////////////////////
xpwrbyte_:
	moveq.l	$1,d0		/ d0 is a byte-count
	moveq.l	$WRITE,d1	/ d1 contains opcode for write operation
	move.b	7(sp),buffer	/ store the data
	movea.l	$buffer,a0	/ pointer to data
	trap	$TRAPIO		/ trap causes byte to be written
	rts

//////////////////////////////////////////////////////////////////////////////
/
/ xpwrdata
/
/ Sends 'size' bytes from the buffer pointed to by 'buf' to the transputer. On
/ exit this routine returns an error flag to the caller: 0 = error, 
/ 1 = success.
/
/ Synopsis:	WORD xpwrdata(buf,size)
/ 		UBYTE *buf;
/ 		WORD size;
/
//////////////////////////////////////////////////////////////////////////////
xpwrdata_:
	moveq.l	$WRITE,d1	/ d1 is an opcode
	move.l	8(sp),d0	/ d0 is a byte-count
	movea.l	4(sp),a0	/ a0 is pointer to the data
	trap	$TRAPIO		/ send the data to the transputer	
	rts
	
//////////////////////////////////////////////////////////////////////////////
/
/ xprddata
/
/ Reads 'size' bytes from the buffer pointed to by 'buf' to the transputer. On
/ exit this routine returns an error flag to the caller: 0 = error, 
/ 1 = success.
/
/ Synopsis:	WORD xprddata(buf,size)
/ 		UBYTE *buf;
/ 		WORD size;
/
//////////////////////////////////////////////////////////////////////////////
xprddata_:
	moveq.l	$READ,d1	/ d1 is an opcode
	move.l	8(sp),d0	/ d0 is a byte-count
	movea.l	4(sp),a0	/ a0 is pointer to the data
	trap	$TRAPIO		/ send the data to the transputer	
	rts
	
/////////////////////////////////////////////////////////////////////////////
/
/ xpwrword
/
/ Sends a non-byteswapped long-word of data to the transputer, and returns
/ error flag: 0 = error, 1 = successful termination.
/
/ Synopsis:	WORD xpwrword(data)
/ 		WORD data;
/
/////////////////////////////////////////////////////////////////////////////
xpwrword_:
	move.l	4(sp),buffer	/ retrieve data from stack & store in buffer
	moveq.l	$4,d0		/ d0 is a byte-count
	moveq.l	$WRITE,d1	/ set d1 to opcode for write operation
	movea.l	$buffer,a0	/ a0 is pointer to data
	trap	$TRAPIO		/ send the data to the transputer
	rts

////////////////////////////////////////////////////////////////////////////
/
/ xprdword
/
/ reads a non-byteswapped long-word from the transputer.
/
/ Synopsis:	WORD xprdword()
/
////////////////////////////////////////////////////////////////////////////
xprdword_:
	moveq.l	$READ,d1	/ d1 is an opcode
	moveq.l	$4,d0		/ d0 is a byte-count
	movea.l	$buffer,a0	/ a0 is pointer to data buffer
	trap	$TRAPIO		/ fetch the data into buffer
	move.l	buffer,d0	/ retrieve data from buffer
	rts			/ return data to caller

/////////////////////////////////////////////////////////////////////////////
/
/ xpwrint
/
/ send a byte swapped long word to the transputer, and return error flag to
/ caller: 0 = error, 1 = success.
/
/ Synopsis:	WORD xpwrint(data)
/ 		WORD data;
/
/////////////////////////////////////////////////////////////////////////////
xpwrint_:
	move.l	4(sp),d0	/ retrieve data from stack
	bsr	byteswap	/ byte swap the data
	move.l	d0,buffer	/ store data in buffer
	moveq.l	$4,d0		/ d0 is a byte-count
	moveq.l	$WRITE,d1	/ set d1 to opcode for write operation
	movea.l	$buffer,a0	/ a0 is pointer to data
	trap	$TRAPIO		/ send the data to the transputer
	rts

/////////////////////////////////////////////////////////////////////////////
/
/ xprdint
/
/ read a byte swapped long word from the transputer
/
/ Synopsis:	WORD xprdint()
/
//////////////////////////////////////////////////////////////////////////////
xprdint_:
	moveq.l	$READ,d1	/ d1 is an opcode
	moveq.l	$4,d0		/ d0 is a byte-count
	movea.l	$buffer,a0	/ a0 is pointer to data buffer
	trap	$TRAPIO		/ fetch the data into buffer
	move.l	buffer,d0	/ retrieve data from buffer
	bsr	byteswap	/ byteswap contents of d0
	rts			/ return data (d0) to caller

/////////////////////////////////////////////////////////////////////////////
/
/ xprdrdy
/
/ This reads a status register to determine whether the transputer is
/ ready to send data.
/
/ Synopsis:
/	WORD xprdrdy()
/
/////////////////////////////////////////////////////////////////////////////
xprdrdy_:
	moveq.l	$RDRDY,d1	/ d1 now contains an opcode
	trap	$TRAPIO		/ returns TRUE or FALSE
	rts

////////////////////////////////////////////////////////////////////////////
/
/ xpwrrdy
/
/ This is a function which was originally designed for use with the Kuma
/ transputer interface, where it read a status register to determine whether
/ or not the transputer was ready to rx (?) data. With the DMAC this function
/ is no longer necessary, although this function can still be used.
/ 
/ Synopsis:
/	WORD xpwrrdy()
/
///////////////////////////////////////////////////////////////////////////
xpwrrdy_:
	moveq.l	$WRRDY,d1	/ d1 now contains an opcode
	trap	$TRAPIO		/ returns TRUE or FALSE
	rts

////////////////////////////////////////////////////////////////////////////
/
/ dbrdword
/
/ Use transputer's boot or analyse mode to read a word of data from the xptr.
/ When the transputer is reset (or analysed) it is possible to instruct the
/ transputer to return the contents of any memory location by firstly sending
/ a '1' down a link, and then the address of the memory location which is to 
/ be examined. This routine implements this operation and returns the retrieved
/ data to the caller.
/
/ Synopsis:
/	WORD dbrdword(address)
/	WORD address
/
////////////////////////////////////////////////////////////////////////////
dbrdword_:
	move.l	$1,-(sp)	/ push parameter to xpwrbyte onto stack
	jsr	xpwrbyte_	/ call the procedure
	addq.l	$4,sp		/ clean up stack

	move.l	4(sp),-(sp)	/ retrieve address from stack & push on stack
	jsr	xpwrint_	/ call the procedure
	addq.l	$4,sp		/ clean up stack

	jsr	xprdword_	/ d0=xprdword();
	rts

/////////////////////////////////////////////////////////////////////////////
/
/ dbwrword
/
/ When the transputer has been reset or analysed it is possible to 'poke'
/ data into the transputers memory by sending a '0' to the transputer, followed
/ by two words which represent an address and the data which is to be stores at
/ that address. This routine allows this operation to be performed from within
/ 'C' programs.
/
/ Synopsis:
/	void dbwrword(address,data)
/	WORD address, data;
/
////////////////////////////////////////////////////////////////////////////
dbwrword_:
	move.l	$0,-(sp)	/ push parameter to xpwrbyte onto stack
	jsr	xpwrbyte_	/ call the procedure
	addq.l	$4,sp		/ clean up stack

	move.l	4(sp),-(sp)	/ push 'address' onto stack
	jsr	xpwrint_	/ call procedure xpwrint
	addq.l	$4,sp		/ clean up stack

	move.l	8(sp),-(sp)	/ push 'data' onto stack
	jsr	xpwrword_	/ call procedure xpwrword
	addq.l	$4,sp		/ clean up stack
	rts
	
////////////////////////////////////////////////////////////////////////////
/
/ byteswap
/
/ This subroutine reverses the order of the bytes in reg. d0. Thus if the msb
/ occupies the lowest mem address, and lsb the highest (68000 arrangement) then
/ the result will be that the lsb occupies the lowest memory and the msb the
/ highest (i.e compatible with transputer) and vice versa.
/
/       On entry:
/               d0 = 32 bits data
/       on exit:
/               d0 = 32 bits of byte swapped data
/
////////////////////////////////////////////////////////////////////////////
byteswap:       
        ror     $8,d0           / rotate two lowest bytes in d0
        swap    d0              / move lower word to upper word posn
        ror     $8,d0           / rotate two lowest bytes (formerly the highest)
        rts

////////////////////////////////////////////////////////////////////////////
/
/ resetlnk
/
/ Resets the Perihelion link adapter.
/
/ Synopsis:
/	void resetlnk();
/
////////////////////////////////////////////////////////////////////////////
resetlnk_:
	moveq.l	$RESETLNK,d1
	trap	$TRAPIO
	rts

/////////////////////////////////////////////////////////////////////////////
/
/ dbwrint
/
/ When the transputer has been reset or analysed it is possible to 'poke'
/ data into the transputers memory by sending a '0' to the transputer, followed
/ by two words which represent an address and the data which is to be stores at
/ that address. This routine allows this operation to be performed from within
/ 'C' programs.
/
/ Synopsis:
/	void dbwrint(address,data)
/	WORD address, data;
/
////////////////////////////////////////////////////////////////////////////
dbwrint_:
	move.l	$0,-(sp)	/ push parameter to xpwrbyte onto stack
	jsr	xpwrbyte_	/ call the procedure
	addq.l	$4,sp		/ clean up stack

	move.l	4(sp),-(sp)	/ push 'address' onto stack
	jsr	xpwrint_	/ call procedure xpwrint
	addq.l	$4,sp		/ clean up stack

	move.l	8(sp),-(sp)	/ push 'data' onto stack
	jsr	xpwrint_	/ call procedure xpwrint
	addq.l	$4,sp		/ clean up stack
	rts

////////////////////////////////////////////////////////////////////////////
/
/ dbrdint
/
/ Use transputer's boot or analyse mode to read a word of data from the xptr.
/ When the transputer is reset (or analysed) it is possible to instruct the
/ transputer to return the contents of any memory location by firstly sending
/ a '1' down a link, and then the address of the memory location which is to 
/ be examined. This routine implements this operation and returns the retrieved
/ data to the caller in a byteswapped format.
/
/ Synopsis:
/	WORD dbrdint(address)
/	WORD address
/
////////////////////////////////////////////////////////////////////////////
dbrdint_:
	move.l	$1,-(sp)	/ push parameter to xpwrbyte onto stack
	jsr	xpwrbyte_	/ call the procedure
	addq.l	$4,sp		/ clean up stack

	move.l	4(sp),-(sp)	/ retrieve address from stack & push on stack
	jsr	xpwrint_	/ call the procedure
	addq.l	$4,sp		/ clean up stack

	jsr	xprdint_	/ d0=xprdint();
	rts

	.prvd
buffer:
	.blkl	2	/ reserve 2 long words of memory as a buffer



		
/////////////////////////////////////////////////////////////////////////////
/
/ RS232 support
/
/ Assembler routines for accessing the rs232 device on the ST.
/
/ Author : BLV, 15.12.88
/
//////////////////////////////////////////////////////////////////////////////

/ Routines accessed from C
.globl	RS232_enable_interrupts_, RS232_disable_interrupts_
.globl	RS232_setattrib_, RS232_writech_, RS232_dobreak_
.globl  RS232_checkCTS_
.globl	RS232_arg1_,RS232_arg2_

RS232_arg1_:	.word	1
RS232_arg2_:	.word	1

/ C routines accessed via the interrupt routine
.globl	RS232_gotcha_, RS232_sendcha_, RS232_error_, RS232_modem_
.globl	RS232_gotCTS_

/ addresses of the interrupt routines
CTS_int		=	0x0108
TXerr_int	=	0x0124
TXempty_int	=	0x0128
RXerr_int	=	0x012C
RXfull_int	=	0x0130
Ring_int	=	0x0138

/ addresses for the MFP chip
MFP_PORTS	=	0xFFFA01
MFP_AE		=	0xFFFA03
MFP_IERA	=	0xFFFA07
MFP_IERB	=	0xFFFA09
MFP_ISRA	=	0xFFFA0F
MFP_ISRB	=	0xFFFA11
MFP_IMRA	=	0xFFFA13
MFP_IMRB	=	0xFFFA15
MFP_TCDCR	=	0xFFFA1D
MFP_TDDR	=	0xFFFA25
MFP_control	=	0xFFFA29
MFP_rxstat	=	0xFFFA2B
MFP_txstat	=	0xFFFA2D
MFP_data	=	0xFFFA2F

/ space to store the old interrupt vectors and masks
old_CTS_int:		.long	1
old_TXerr_int:		.long	1
old_TXempty_int:	.long	1
old_RXerr_int:		.long	1
old_RXfull_int:		.long	1
old_Ring_int:		.long	1
old_MFP_AE:		.byte	1
old_MFP_IERA:		.byte	1
old_MFP_IERB:		.byte	1
old_MFP_IMRA:		.byte	1
old_MFP_IMRB:		.byte	1
old_MFP_TCDCR:		.byte	1
old_MFP_TDDR:		.byte	1
old_MFP_control:	.byte	1

.even

/===============================================================================
/ void RS232_enable_interrupts(void)
/
/ Save the old interrupt vectors and install my own. Also save the
/ interrupt masks.
/
/===============================================================================
RS232_enable_interrupts_:

	lea	CTS_int,a0
	move.l	(a0),old_CTS_int	
	lea	my_CTS_int,a1
	move.l	a1,(a0)
	
	lea	TXerr_int,a0
	move.l	(a0),old_TXerr_int	
	lea	my_TXerr_int,a1
	move.l	a1,(a0)
	
	lea	TXempty_int,a0
	move.l	(a0),old_TXempty_int	
	lea	my_TXempty_int,a1
	move.l	a1,(a0)
	
	lea	RXerr_int,a0
	move.l	(a0),old_RXerr_int	
	lea	my_RXerr_int,a1
	move.l	a1,(a0)
	
	lea	RXfull_int,a0
	move.l	(a0),old_RXfull_int	
	lea	my_RXfull_int,a1
	move.l	a1,(a0)

	lea	Ring_int,a0
	move.l	(a0),old_Ring_int
	lea	my_Ring_int,a1
	move.l	a1,(a0)

	lea	MFP_IERA,a0
	move.b	(a0),d0
	move.b	d0,old_MFP_IERA
	or.b	$0x5E,d0
	move.b	d0,(a0)

	lea	MFP_IERB,a0
	move.b	(a0),d0
	move.b	d0,old_MFP_IERB
	or.b	$0x04,d0
	move.b	d0,(a0)

	lea	MFP_IMRA,a0
	move.b	(a0),d0
	move.b	d0,old_MFP_IMRA
	or.b	$0x5E,d0
	move.b	d0,(a0)

	lea	MFP_IMRB,a0
	move.b	(a0),d0
	move.b	d0,old_MFP_IMRB
	or.b	$0x04,d0
	move.b	d0,(a0)

	move.b	MFP_AE,old_MFP_AE
	move.b	MFP_control, old_MFP_control
	move.b	MFP_TCDCR, old_MFP_TCDCR
	move.b	MFP_TDDR, old_MFP_TDDR

	rts

/===============================================================================
/ void RS232_disable_interrupts(void)
/
/ Undo all the damage done by enable_interrupts
/
/===============================================================================
RS232_disable_interrupts_:
	move.l	old_CTS_int, CTS_int
	move.l	old_TXerr_int, TXerr_int
	move.l	old_TXempty_int, TXempty_int
	move.l	old_RXerr_int, RXerr_int
	move.l	old_RXfull_int, RXfull_int
	move.l	old_Ring_int, Ring_int
	move.b	old_MFP_AE, MFP_AE
	move.b	old_MFP_IERA, MFP_IERA
	move.b	old_MFP_IERB, MFP_IERB
	move.b	old_MFP_IMRA, MFP_IMRA
	move.b	old_MFP_IMRB, MFP_IMRB
	move.b	old_MFP_control, MFP_control
	move.b	old_MFP_TCDCR, MFP_TCDCR
	move.b	old_MFP_TDDR, MFP_TDDR

	rts

/===============================================================================
/ void RS232_setattrib(int UCR, int baud)
/
/ set the MFP control register and the baud rate. The baud rate is held in
/ the bottom three bits of the TCDCR register and all of the TDDR register.
/ Hence I zap all of TDDR, extract the old value of TCDCR, and set the
/ bottom three bits according to the top byte of the argument.
/
/===============================================================================
RS232_setattrib_:
	move.w	RS232_arg1_,d0
	lea	MFP_control,a0
	move.b	d0,(a0)

	move.w	RS232_arg2_,d0
	lea	MFP_TDDR,a0
	move.b	d0,(a0)
	lsr.w	$8,d0
	move.b	MFP_TCDCR,d1
	and.b	$0xF8,d1
	or.b	d0,d1
	move.b	d1,MFP_TCDCR

	rts

/===============================================================================
/ void RS232_writech()
/
/ This routine is called when I need to put a byte into the transmission
/ register. The byte will be held in the global rs232_arg1. Unfortunately
/ the routine may be called either from normal mode or from supervisor mode,
/ which makes life slightly complicated.
/
/===============================================================================
RS232_writech_:

	move.w	sr,d1
	btst	$13,d1
	bne	super	/ if set, we are in supervisor mode already
	pea	super
	move.w	$38,-(sp)
	trap	$14
	addq.l	$6,sp
	rts
	
super:
	move.w	RS232_arg1_,d0
	lea	MFP_data,a0
	move.b	d0,(a0)
	rts

/===============================================================================
/ void RS232_dobreak(void)
/
/ generate a break event, by poking the tx status register
/
/===============================================================================
RS232_dobreak_:
	lea	MFP_txstat,a0
	move.b	(a0),d0
	bset	$3,d0
	move.b	d0,(a0)

	move.l	$10000,d1
break_wait:
	dbra	d1,break_wait

	bclr	$3,d0
	move.b	d0,(a0)

	rts

/===============================================================================
/ CTS handling : this is ridiculously complicated because of the ST hardware.
/ In particular I can only trigger interrupts on one edge, so I have to
/ change this edge every time I get an interrupt. Ofcourse this could leave
/ things confused if I get two edge changes before I can handle it, so at
/ regular intervals whilst accessing the serial line I call checkCTS to
/ ensure that the world is consistent. This routine is also called from the
/ CTS interrupt routine.
/
/===============================================================================
my_CTS_int:
	movem.l	${d0-d2,a0-a2},-(sp)

	jsr	RS232_checkCTS_

	bclr	$2,MFP_ISRB
	movem.l	(sp)+,${d0-d2,a0-a2}
	rte

RS232_checkCTS_:
	move.b	MFP_PORTS,d0	/ get the current CTS status
	and.b	$0x04,d0	/ and mask off the CTS bit
	beq	CTShigh		/ 0 -> input is high... Typical STism
	move.w	$0,-(sp)	
	jsr	RS232_gotCTS_	/ Inform C of the current level
	addq	$2,sp
	bset	$2,MFP_AE	/ set AE reg to low->high transition
	rts

CTShigh:
	move.w	$1,-(sp)	/ CTS is high
	jsr	RS232_gotCTS_	/ Inform C
	addq	$2,sp
	bclr	$2,MFP_AE	/ set AE reg to high->low transition
	rts

/===============================================================================
/ my_TXerr_int
/
/ Interrupt routine to deal with transmission errors. These are ignored because
/ I do not know what to do with them. Instead I allow transmission of the next
/ character.
/
/===============================================================================
my_TXerr_int:
	movem.l	${d0-d2,a0-a2},-(sp)

	jsr	RS232_sendcha_

	bclr	$1,MFP_ISRA
	movem.l	(sp)+,${d0-d2,a0-a2}
	rte

/===============================================================================
/ my_TXempty_int
/
/ Interrupt routine for handling empty transmit register. This calls a C
/ routine which can then invoke another assembler routine to put another
/ byte in the register.
/
/===============================================================================
my_TXempty_int:
	movem.l	${d0-d2,a0-a2},-(sp)

	jsr	RS232_sendcha_

	bclr	$2,MFP_ISRA
	movem.l	(sp)+,${d0-d2,a0-a2}
	rte

/===============================================================================
/ my_RXerr_int
/
/ Interrupt routine to deal with errors on incoming data. The RX status
/ register is extracted and passed to a C routine which can interpret it.
/
/===============================================================================
my_RXerr_int:
	movem.l	${d0-d2,a0-a2},-(sp)

	clr.l	d0
	move.b	MFP_rxstat,d0
	move.w	d0,-(sp)
	jsr	RS232_error_
	addq	$2,sp

	bclr	$3,MFP_ISRA
	movem.l	(sp)+,${d0-d2,a0-a2}
	rte

/===============================================================================
/ my_RXfull_int
/
/ Interrupt routine to deal with received data. The data is read and passed
/ to a C routine which can buffer it.
/
/===============================================================================
my_RXfull_int:
	movem.l	${d0-d2,a0-a2},-(sp)

	clr.l	d0
	move.b	MFP_data,d0
	move.w	d0,-(sp)
	jsr	RS232_gotcha_

	addq	$2,sp
	bclr	$4,MFP_ISRA
	movem.l	(sp)+,${d0-d2,a0-a2}
	rte

/===============================================================================
/ my_Ring_int
/
/ Interrupt routine to deal with modem interrupts.
/
/===============================================================================
my_Ring_int:
	movem.l	${d0-d2,a0-a2},-(sp)

	jsr	RS232_modem_

	bclr	$6,MFP_ISRA
	movem.l	(sp)+,${d0-d2,a0-a2}
	rte


/===============================================================================
/ Raw disk support - interface to trap 5
/
/ WORD *raw_size(int drive)
/ WORD raw_io(int op, int drive, word offset, int size, byte *buff)
/  ( offset = 1st sector no., size = no. of sectors
/
/===============================================================================

.globl raw_io_, raw_size_

raw_size_:
	movem.l ${d3-d7,a3-a6},-(sp)       / save all registers

	moveq.l	$0,d0			/ opcode 0 = partition status
	move.b	40(sp),d1		/ first arg = drive id
	trap	$5			/ pray it is installed
	tst.b	d0			/ check result
	beq	size_fail		/ 0->failure, return NULL
	move.l	a0,d0			/ return ptr to alloc table
size_fail:		
        movem.l  (sp)+,${d3-d7,a3-a6}       / restore registers
	rts

raw_io_:
	movem.l	${d3-d7,a3-a6},-(sp)      / save all registers

	move.w	40(sp),d0                  / first arg  = opcode
	move.w	42(sp),d1		  / second arg = drive
	move.l	44(sp),d2		  / third arg  = sector offset
	move.w	48(sp),d3		  / fourth arg = number of sectors
	movea.l	50(sp),a0		  / fifth arg  = buffer

	trap $5				  / do the work
					  / return with success/failure

	movem.l	(sp)+,${d3-d7,a3-a6}	  / restore world
	rts
