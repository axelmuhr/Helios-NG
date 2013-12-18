/*
 * Name:	MicroGnuEmacs
 *		VAX/VMS terminal I/O.
 *		o 16-Apr-86 ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 *		  Turn off TTSYNC so ^S and ^Q are sent to program.
 *		  To get this back, compile with -DFLOWCONTROL
 *		o 10-Jul-86 ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 *		  Add setttysize(), typeahead() and panic() for Gnu v30
 *		o 21-Jul-87 ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 *		  Use transmit speed from iosb instead of receive, which
 *		  is only valid if different from the transmit speed...
 */
#include	"def.h"

#include	<stsdef.h>
#include	<ssdef.h>
#include	<descrip.h>
#include	<iodef.h>
#include	<ttdef.h>
#include	<tt2def.h>

#define	NIBUF	128			/* Probably excessive.		*/
#define	NOBUF	512			/* Not too big for 750/730.	*/
#define	EFN	0			/* Event flag			*/

char	obuf[NOBUF];			/* Output buffer		*/
int	nobuf;				/* # of bytes in above		*/
char	ibuf[NIBUF];			/* Input buffer			*/
int	nibuf;				/* # of bytes in above		*/
int	ibufi;				/* Read index			*/
int	oldmode[3];			/* Old TTY mode bits		*/
int	newmode[3];			/* New TTY mode bits		*/
short	iochan;				/* TTY I/O channel		*/
int	nrow;				/* Terminal size, rows.		*/
int	ncol;				/* Terminal size, columns.	*/
short	ospeed;				/* Terminal output speed	*/
					/* for termcap library		*/

/*
 * This routines gets called once, to set up the
 * terminal channel.
 * On VMS we find the translation of the SYS$COMMAND:
 * logical name, assign a channel to it, and set it raw.
 */

ttopen()
{
	struct	dsc$descriptor	idsc;
	struct	dsc$descriptor	odsc;
	char	oname[40];
	int	iosb[2];
	int	status;

	odsc.dsc$a_pointer = "SYS$COMMAND";
	odsc.dsc$w_length  = strlen(odsc.dsc$a_pointer);
	odsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	odsc.dsc$b_class   = DSC$K_CLASS_S;
	idsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	idsc.dsc$b_class   = DSC$K_CLASS_S;
	do {
		idsc.dsc$a_pointer = odsc.dsc$a_pointer;
		idsc.dsc$w_length  = odsc.dsc$w_length;
		odsc.dsc$a_pointer = &oname[0];
		odsc.dsc$w_length  = sizeof(oname);
		status = LIB$SYS_TRNLOG(&idsc, &odsc.dsc$w_length, &odsc);
		if (status!=SS$_NORMAL && status!=SS$_NOTRAN)
			exit(status);
		if (oname[0] == 0x1B) {
			odsc.dsc$a_pointer += 4;
			odsc.dsc$w_length  -= 4;
		}
	} while (status == SS$_NORMAL);
	status = SYS$ASSIGN(&odsc, &iochan, 0, 0);
	if (status != SS$_NORMAL)
		exit(status);
	status = SYS$QIOW(EFN, iochan, IO$_SENSEMODE, iosb, 0, 0,
			  oldmode, sizeof(oldmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		exit(status);

	nrow = (oldmode[1]>>24) & 0xFF;		/* Terminal length.	*/
	if (nrow > NROW)
		nrow = NROW;
	ncol = (oldmode[0]>>16) & 0xFFFF;	/* Width.		*/
	if (ncol > NCOL)
		ncol = NCOL;
	ospeed = (iosb[0]>>16) & 0xFF;		/* Speed (for termcap)	*/
	newmode[0] = oldmode[0];		/* Only in version 4.	*/
#ifdef	FLOWCONTROL
	newmode[1] = oldmode[1] | TT$M_NOECHO | TT$M_TTSYNC;
#else
	newmode[1] = (oldmode[1] | TT$M_NOECHO) & ~TT$M_TTSYNC;
#endif
	newmode[2] = oldmode[2] | TT2$M_PASTHRU;
	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  newmode, sizeof(newmode), 0, 0, 0, 0);

	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		exit(status);
}

/*
 * This function gets called just
 * before we go back home to the command interpreter.
 * On VMS it puts the terminal back in a reasonable state.
 */
ttclose()
{
	int	status;
	int	iosb[2];

	ttflush();
	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
	         oldmode, sizeof(oldmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		exit(status);
	status = SYS$DASSGN(iochan);
	if (status != SS$_NORMAL)
		exit(status);
}

/*
 * Write a character to the display.
 * On VMS, terminal output is buffered, and
 * we just put the characters in the big array,
 * after checking for overflow.
 */
ttputc(c)
{
	if (nobuf >= NOBUF)
		ttflush();
	obuf[nobuf++] = c;
}

/*
 * This function does the real work of
 * flushing out buffered I/O on VMS. All
 * we do is blast out the block with a write call. No status
 * checking is done on the write, because there isn't anything
 * clever that can be done, and because you will see the
 * error as a messed up screen.
 */
ttflush()
{
	int	iosb[2];

	if (nobuf != 0) {
		SYS$QIOW(EFN, iochan, IO$_WRITELBLK|IO$M_NOFORMAT,
		iosb, 0, 0, obuf, nobuf, 0, 0, 0, 0);
		nobuf = 0;
	}
}

/*
 * Read a character from the terminal,
 * performing no editing and doing no echo at all.
 * More complex in VMS that almost anyplace else,
 * which figures.
 */
ttgetc()
{
	int	status;
	int	iosb[2];
	int	term[2];

	term[0] = 0;
	term[1] = 0;
	while (ibufi >= nibuf) {
		ibufi   = 0;
		status = SYS$QIOW(EFN, iochan, IO$_READLBLK|IO$M_TIMED,
			 iosb, 0, 0, ibuf, NIBUF, 0, term, 0, 0);
		if (status != SS$_NORMAL)
			continue;
		status = iosb[0] & 0xFFFF;
		if (status!=SS$_NORMAL && status!=SS$_TIMEOUT)
			continue;
		nibuf = (iosb[0]>>16) + (iosb[1]>>16);
		if (nibuf == 0) {
			status = SYS$QIOW(EFN, iochan, IO$_READLBLK,
				iosb, 0, 0, ibuf, 1, 0, term, 0, 0);
			if (status != SS$_NORMAL)
				continue;
			if ((iosb[0]&0xFFFF) != SS$_NORMAL)
				continue;
			nibuf = (iosb[0]>>16) + (iosb[1]>>16);
		}
	}
	return (ibuf[ibufi++] & 0xFF);
}

/*
 * Internal check for new terminal size.
 * Do this *before* setting terminal modes, so
 * the size changes are kept when.
 */
ckttysize()
{
	int status, mode[3], iosb[2], wid, len;

	status = SYS$QIOW(EFN, iochan, IO$_SENSEMODE, iosb, 0, 0,
			  mode, sizeof(mode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		panic("ckttsize: can't sense terminal modes!");

	/* save new page length */
	len = (mode[1] >> 24) & 0xFF;
	oldmode[1] = (oldmode[1] & 0x00FFFFFF) | (len << 24);
	newmode[1] = (newmode[1] & 0x00FFFFFF) | (len << 24);

	/* save new page width */
	wid = (mode[0] >> 16) & 0xFF;
	oldmode[0] = (oldmode[0] & 0x0000FFFF) | (wid << 16);
	newmode[0] = (newmode[0] & 0x0000FFFF) | (wid << 16);
}

/*
 * Tell Emacs how big the terminal is now,
 * making sure the page size is in the range
 * 1..NROW.
 */

setttysize()
{
	nrow = (newmode[1]>>24) & 0xFF;		/* Length.		*/
	if (nrow > NROW)
		nrow = NROW;

	ncol = (newmode[0]>>16) & 0xFFFF;	/* Width.		*/
	if (ncol > NCOL)
		ncol = NCOL;
}

/*
 * Return the number of characters in the
 * typeahead buffer.
 */

typeahead()
{
	int	status, SYS$QIOW(), iosb[2], mode[2];

	status = SYS$QIOW(EFN, iochan, IO$_SENSEMODE|IO$M_TYPEAHDCNT,
			iosb, 0, 0, mode, sizeof(mode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		exit(status);
	return (mode[0] & 0xFFFF);	/* # characters in typeahead buf */
}

#ifndef	NO_DPROMPT
/*
 * Attempt to read for one character.  Return TRUE if the
 * read times out after 2 seconds, return immediately with
 * FALSE if the user enters something.
 */
ttwait()
{
	int	status;
	int	iosb[2];
	int	term[2];

	term[0] = 0;		        /* no termination char for read */
	term[1] = 0;
	while (ibufi >= nibuf) {	/* anything in the buffer?	*/
		ibufi   = 0;		/* nope, read 1 char w/timeout	*/
		status = SYS$QIOW(EFN, iochan, IO$_READLBLK|IO$M_TIMED,
			 iosb, 0, 0, ibuf, 1, 2, term, 0, 0);
		if (status != SS$_NORMAL)
			continue;		/* did read succeed ?	     */
		status = iosb[0] & 0xFFFF;	/* yes, get secondary status */
		if (status!=SS$_NORMAL && status!=SS$_TIMEOUT)
			continue;		/* try again if bad	     */
		nibuf = (iosb[0]>>16) + (iosb[1]>>16);/* store # chars read  */
		if (status == SS$_TIMEOUT)
			return (TRUE);		/* the read timed out	     */
	}
	return (FALSE);				/* read did not time out     */
}
#endif

/*
 * Just exit, as quickly as we can.
 */

panic(s)
char *s;
{
	fprintf(stderr,"panic: %s\n\n", s);
	ttclose();		/* set the terminal back to sane state... */
	exit(SS$_ABORT);	/* go away! */
}
