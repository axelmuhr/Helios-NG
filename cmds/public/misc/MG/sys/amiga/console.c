/*
 * These functions are taken directly from the
 * console.device chapter in the Amiga V1.1
 * ROM Kernel Manual.
 */
#include <exec/types.h>
#include <exec/io.h>
#include <devices/console.h>
#include <libraries/dos.h>
#include <intuition/intuition.h>

extern	LONG	OpenDevice();
extern	LONG	DoIO();
extern	LONG	SendIO();

/*
 * Open a console device, given a read request
 * and a write request message.
 */

int OpenConsole(writerequest,readrequest,window)
struct IOStdReq *writerequest;
struct IOStdReq *readrequest;
struct Window *window;
{
	LONG error; 
	writerequest->io_Data = (APTR) window;
	writerequest->io_Length = (ULONG) sizeof(*window);
	error = OpenDevice("console.device", 0L, writerequest, 0L);

	/* clone required parts of the request */
	if (readrequest) {
		readrequest->io_Device = writerequest->io_Device;
		readrequest->io_Unit   = writerequest->io_Unit;
	}
	return((int) error);
}

/*
 * Output a single character	
 * to a specified console
 */ 

int ConPutChar(request,character)
struct IOStdReq *request;
char character;
{
#ifdef	V11
	register int x;
#endif
	request->io_Command = CMD_WRITE;
	request->io_Data = (APTR)&character;
	request->io_Length = (ULONG)1;
	DoIO(request);
	/* caution: read comments in manual! */
	return(0);
}
 
/*
 * Output a NULL-terminated string of
 * characters to a console
 */ 

int ConPutStr(request,string)
struct IOStdReq *request;
char *string;
{
#ifdef	V11
	register int x;
#endif
	request->io_Command = CMD_WRITE;
	request->io_Data = (APTR)string;
	request->io_Length = (LONG)-1;
	DoIO(request);
	return(0);
}

/*
 * Write out a string of predetermined
 * length to the console
 */
 
int ConWrite(request,string,len)
struct IOStdReq *request;
char *string;
int len;
{
#ifdef	V11
	register int x;
#endif
	request->io_Command = CMD_WRITE;
	request->io_Data = (APTR)string;
	request->io_Length = (LONG)len;
	DoIO(request);
	return(0);
}

/*
 * Queue up a read request 
 * to a console
 */

int QueueRead(request,whereto)
struct IOStdReq *request;
char *whereto;
{
#ifdef	V11
	register int x;
#endif
	request->io_Command = CMD_READ;
	request->io_Data = (APTR)whereto;
	request->io_Length = (LONG)1;
	SendIO(request);
	return(0);
}

