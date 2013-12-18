
/* <nonansi.norcrosys.h>: Internal functions for use within run-time        */
/* system of Norcroft C compiler  (deadly eh?).                             */
/* version 0.02a */

#ifndef __nonansi_norcrosys_h
#define __nonansi_norcrosys_h

#define memclr( s, n )	memset( s, 0, n )

extern int		_interrupts_off;
extern void		_raise_stacked_interrupts( void );
extern void *		_fopen_string_file( const char * data, int length );  /* really FILE * but I do not want to include stdio.h */
extern void		_postmortem( void );
extern void		_mapstore( void );
extern void		_sysdie( const char * s );
extern void		_initalloc( char *, char * );
extern void		_initio( char *, char *, char * );
extern void		_terminateio( void );
extern void		_signal_init( void );
extern void		_exit_init( void );

#if !defined size_t && !defined __size_t
#define __size_t 1
typedef unsigned int size_t;  /* see <stddef.h> */
#endif

extern void *		_sys_alloc( size_t n );
extern void		_sys_msg( const char * );
extern void		_exit( int n );

#define __system_io 1

#ifdef RISCOS
typedef int FILEHANDLE;
#define TTYFILENAME ":tt"
extern int _osbyte(int a, int x, int y, int c);
extern int _oswrch(int ch);
extern int _osbget(int fh);
extern int _osbput(int ch, int fh);
extern int _osgbpb(int op, int fh, void *base, int len, int extra);
extern int _osgbpb1(int op, int fh,
    struct _osgbpb1_control_block {void *base; int len; int extra;} *z);
extern int _osrdch(void);
extern int _osword(int op, int *data);
extern int _osfind(int op, char *name);
extern int _osfile(int op, const char *name, int loadaddr, int execaddr,
                                             int startaddr, int endaddr);
extern int _osfile1(int op, const char *name,
    struct _osfile1_control_block {int load; int exec; int start; int end;} *z);
extern int _osargs(int op, int fh, int arg);
extern int _oscli(const char *s);
extern int _ttywrite(unsigned char *buf, unsigned int len, int flag);
extern int _ttyread(unsigned char *buff, int size, int flag);
#endif /* RISCOS */

#ifdef HELIOS
typedef Stream *	FILEHANDLE;  /* pointer to a Helios Stream structure */
#endif

#ifdef POSIX
typedef int 		FILEHANDLE;		/* Posix fd */
#endif


#ifndef eq
#define eq 		==
#define ne		!=
#endif

typedef struct sysbase
  {
    int  file_pos;
    int  used;
    int  read;
    char data[ 1 ];
  }
sysbase;

#ifdef __C40
extern int 			_sdiv10( int );
extern unsigned int		_udiv10( unsigned int );
#define _kernel_sdiv10( x )	_sdiv10( x )
#define _kernel_sdiv( x, y )	((y) / (x))
#define _kernel_udiv10( v )	_udiv10( v )
#else
#define _kernel_sdiv10( x ) 	((x) / 10)
#define _kernel_sdiv( x, y )	((y) / (x))
#define _kernel_udiv10( v )	((unsigned)((v)) / 10)
#endif

#endif /* __nonansi_norcrosys_h */

/* end of nonansi.norcrosys.h */
