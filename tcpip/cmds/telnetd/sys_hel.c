#include <helios.h>
#include <codes.h>
#include <signal.h>
#include <errno.h>
#include <posix.h>
#include <message.h>
#include <gsp.h>
#include <string.h>

int fchmod(int fd, int mode)
{
	fd =fd, mode = mode;
	return (-1);
}

void vhangup(void)
{
	(void) kill(0, SIGHUP);
}

int setpgrp(int uid, int gid)
{
	uid = uid, gid = gid;
	return (-1);
}

/* keep in match with window.h */
#define FG_Reinit	0x00001fd0	/* Reinitialise Terminal tables	*/

int SendTerminalType ( char *path, char *terminaltype)
{
    MCB		m;
    word	e;
    word	Control[IOCMsgMax];
    byte	Data[IOCDataMax];
    Port	reply;
  
    unless (terminaltype && *terminaltype && path && *path)
    	return -1;
    
    terminaltype = strchr(terminaltype, '=');
    unless (terminaltype && terminaltype[1])
    	return -1;
    	
    reply = NewPort ();
    InitMCB ( &m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, FC_GSP + FG_Reinit );
    m.Control = Control;
    m.Data = Data; 	   
    MarshalCommon (&m, Null(Object), path);          
    MarshalString (&m, &terminaltype[1]);

    e = PutMsg ( &m );
    if ( e != Err_Null )
    {
    	errno = EPIPE;
    	return -1;
    }

    InitMCB ( &m, MsgHdr_Flags_preserve, reply, NullPort, 0 );
    m.Timeout = MaxInt;

    e = GetMsg ( &m );
    FreePort ( reply );
 
    e = m.MsgHdr.FnRc;
    if ( e == FC_GSP + SS_Window + FG_Reinit)
	return 0;
    elif ( e == FC_GSP + SS_Window + FG_Reinit + 1)
    {
    	errno = EINVAL;
    	return -1;
    }
    else
    {
    	errno = EPERM;
    	return -1;
    }
}

