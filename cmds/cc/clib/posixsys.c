/*------------------------------------------------------------------------------
--                 P O S I X   A N S I   C   I N T E R F A C E                --
--                 -------------------------------------------                --
--                                                                            --
--            Copyright (C) 1987 - 1992 Perihelion Software Ltd.              --
--                      All rights Reserved                                   --
--                                                                            --
--      This file contains various routines needed as an interface between    --
--                                                                            --
--      the ANSI C library of the Norcroft compiler and Posix.                --
--                                                                            --
--                                                                            --
--      Author: NHG 19/5/88 from Heliosys BLV 2/12/87                         --
--                                                                            --
------------------------------------------------------------------------------*/
/* $Id: posixsys.c,v 1.13 1993/08/11 13:35:47 bart Exp $ */

#include <helios.h>
#include <posix.h>
#include <signal.h>

#include "norcrosys.h"       /* get the machine specific details */
#include "sysdep.h"

#include <time.h>
#include <stdio.h>                   /*the usual include files */
#include <stdlib.h>
#include <string.h>

#include <setjmp.h>
#include <fcntl.h>
#include <syslib.h>
#include <nonansi.h>

#include <syslib.h>
#include <codes.h>

extern void IOdebug(const char *, ...);

extern FILE *fdopen(FILEHANDLE fd, char *mode);

static void SignalHandler(void);
#if !defined(__TRAN)
static void SyncSigHandler(int signal, void *data);
#endif

/* ---------------------------------------------------------------------------
     The routines needed are as follows :
       1) clock(), time() - as per the spec
       2) remove(), rename(), getenv(), system() - also as per spec
       3) _sys_open(), _sys_istty_(), _sys_seek_(), _sys_flen_(), _sys_write_(),
          _sys_read_(), _sys_close_() - routines assumed present by the ANSI
          library to interface with the I/O system
       4) _main() - routine used to initialise the system
       5) _mapstore(), _backtrace() - debugging routines
       6) _sys_msg() - used to send fatal messages to the screen

------------------------------------------------------------------------------*/

/* 1) clock() and time() 				*/
                 /* CPU time in whatever units it uses	*/


static word start_cputime = 0;

clock_t clock()
{  
   return (clock_t)(_cputime() - start_cputime);
}

/* ---------------------------------------------------------------------------*/
/*  remove(), rename(), getenv(), and system() - remove and rename map to Helios
    calls, getenv and system are not supported at present                     */

int remove(const char *name)
{
	return unlink((char *)name);
}

#if 0
int system(const char *string)
{ string = string;
  _sysdie("unimplemented system()");
}
#endif

/*----------------------------------------------------------------------------*/
/* _mapstore() and _backtrace() - debugging facilities                        */

#ifdef __TRAN	/* doesn't use these functions, but for backwards compat... */
void _mapstore()
{ _sysdie("Unimplemented _mapstore()");
}

void _backtrace(int why, int *address, int *fp, int *sp, int *pc)
{ why = why; address = address; fp = fp; sp = sp; pc = pc;

  _sysdie("Unimplemented _backtrace()");
}

void _postmortem()
{ _sysdie("Unimplemented _postmortem()\n");
}
#endif

/*----------------------------------------------------------------------------*/
/* _sysmsg() - this is used to send messages to the screen when things start to
   collapse, _error_recursion is used in case sending the message causes a
   further failure.                                                           */


static int _error_recursion;

void _sys_msg(const char *s)
{ if (!_error_recursion)
    { _error_recursion = 1;           /* not too fatal an error */
      (void) fprintf(stderr, "\n%s\n", s);
      _error_recursion = 0;
    }
  else                     /* things must be pretty bad - send a message */
    { IOdebug("%s\n", s);  /* directly to the screen */
     return;
    }
}


/*----------------------------------------------------------------------------*/
/* _main() - this should be called when the program has been loaded 	      */

#ifdef STACKEXTENSION
#define SIGHANDLERSTACK	800
#else
#define	SIGHANDLERSTACK 2000
#endif

#ifdef NOT_NEEDED
static int exit_code;
#endif

void _main(int (*main)(int argc, char **argv))
{ 
	int		argc   = 0;
	char		**argv1, **argv = (char **) NULL;
#if !defined(__TRAN)
	extern void SetException( VoidFnPtr, void *, ... );
#endif	

	start_cputime = _cputime();

	_error_recursion = 1;    		/* not safe to do a printf */

	argv1 = _posix_init();

	Fork(SIGHANDLERSTACK,SignalHandler,0);

	Delay( 0 );	/* deschedule ourselves so that the Posix execption handler has a chance to start up */
	
#if !defined(__TRAN)
	/* for processors that can generate syncronous hardware traps */
	/* set the tasks exception handling vector */
	/* The C40 cannot generate hardware signals, but the C compiler */
	/* support code can in some cases generate SIGFPE or SIGSTK */
	/* This Exception vector allows us to raise a signal from the */
	/* kernel */
	SetException(SyncSigHandler,NULL,0);
#endif

	memclr(_iob, sizeof(_iob)); 	/* clear table containing streams */

	fdopen(0,"r");		/* stdin */
	fdopen(1,"w");		/* stdout */
	fdopen(2,"w");		/* stderr */
	argv = argv1;
	for (argc = 0; *argv1 != (char *)NULL; argc++, argv1++);

	_error_recursion = 0;      /* IO initialised, so safe to do a printf */

	atexit(_terminateio);			/* cause io termination on exit */

	exit(main(argc, argv));
}

#define _IOAPPEND 0x8000        /* must seek to eof before any write     */
#define _IOFUNNY  0x10000	/* for MSDOS files			 */

FILE *fdopen(FILEHANDLE fh, char *mode)
{
    int i;

    /* first check that the fd is valid */
    if( fcntl(fh,F_GETFD) == -1 ) return 0;
    
    for (i=0; i<_SYS_OPEN; i++)
    {   FILE *iob = &_iob[i];
        if (!(iob->_flag & _IOREAD+_IOWRITE))  /* if not open then try it */
	{
		int flag, openmode;
		    switch (*mode++)
		    {   default:  return(NULL);               /* mode is incorrect */
		        case 'r': flag = _IOREAD;  openmode = 0; break;
		        case 'w': flag = _IOWRITE; openmode = 4; break;
		        case 'a': flag = _IOWRITE | _IOAPPEND;
		                                   openmode = 8; break;
		    }
		    for (;;)
		    {   switch (*mode++)
		        {
		    case '+':   flag |= _IOREAD+_IOWRITE, openmode |= 2;
		                continue;
		    case 'b':   flag |= _IOBIN, openmode |= 1;
		                continue;
		        }
		        break;
		    }

		    if( ( ( fdstream(fh)->Flags & Flags_MSdos ) != 0 ) && 
		    	((openmode & 0x1) == 0)) flag |= _IOFUNNY;

		    iob->_flag = flag;
		    iob->_file = fh;
				    /* BLV - so that I can check for buffer */
		    iob->_sysbase = NULL;

		    if (openmode & 8) fseek(iob, 0L, SEEK_END);  /* a or a+             */

		    return iob;
	}
    }
    return 0;   /* no more i/o channels allowed for */

}

int fileno(FILE *stream)
{
	return stream->_file;
}


/* The following routines must be compiled with stack checking OFF */
#ifdef __TRAN
# pragma -s1
#elif defined (__ARM) || defined(__C40)
# pragma no_check_stack
#else
# error "Pragma required to turn off stack checking"
#endif

#ifdef __TRAN
void _stack_error(Proc *p)
{
	IOdebug("Clib stack error in %s at %x",p->Name,&p);
	raise(SIGSTAK);
}
#endif

static void SignalHandler(void)
{
  extern void _posix_exception_handler( void );

  _posix_exception_handler();	
}

#if !defined(__TRAN)
/* syncronous signal handler */
static void SyncSigHandler(int signal, void *data)
{
	raise(signal);
}
#endif

#ifdef __TRAN
void StackCheck(void)
{
	int call_stack;
	int vec_stack[3];
	
	if( &call_stack < (&vec_stack[0]+40) ) _stack_error(NULL);
}
#endif



/* posixsys.c */
