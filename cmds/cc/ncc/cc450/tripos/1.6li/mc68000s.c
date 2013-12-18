/* mc68000sys.c:  Copyright (C) C. Selwyn                               */
/* Very AMIGA/TRIPOS/NEWTRIPOS specific routines.                                        */
/* version 0.01 */

/* AM 14-dec-86: created. */

#include "hostsys.h"                            /* things like _initio() */
#include <stddef.h>                             /* for NULL              */
#include <stdio.h>                              /* for EOF               */
#include <stdlib.h>                             /* for exit()            */
#include <ctype.h>                              /* for isprint()         */
#include <time.h>                               /* for clock()           */
#include <string.h>                             /* for memset            */
#include <signal.h>                             /* for SIGINT            */

typedef struct MemBlock {
      struct MemBlock *nextblock;
      size_t           memsize;
} MemBlock;

static struct MemBlock *_mem_list;
extern int initpkt;
#ifdef AMIGA
static FILEHANDLE _initial_input;
static FILEHANDLE _initial_output;
#endif
/* get/free here for now */

void _mem_uninit()
{
    MemBlock *q = _mem_list;
    MemBlock *nextq;

    for(; q != NULL; q = nextq )
    {   nextq = q->nextblock;
        nfreemem(q, q->memsize);
    }
}

void _mem_init()
{
    _mem_list = NULL;
}

void *malloc(size_t size)
{   MemBlock *p;
    if (size >= 0x00fffff8u ||
        (p = (void *)nallocmem(size+sizeof(MemBlock),MEMF_PUBLIC)) == 0)
             return 0;
    p->nextblock = _mem_list;
    _mem_list = p;
    p->memsize = size+sizeof(MemBlock);
    
    return ((void *)(p+1));
}

void free(void *p)
{   MemBlock *q = (MemBlock *)((MemBlock *)p - 1);
    MemBlock *lq = (MemBlock *)((char *)(&_mem_list))-offsetof(MemBlock,nextblock);

    for( ; lq->nextblock != q; lq = lq->nextblock)
        if( lq->nextblock == NULL) return;

    lq->nextblock = lq->nextblock->nextblock;
    nfreemem((void *)q,q->memsize);
}

void *realloc(void *ptr, size_t size)
{
   void *node;
   if( size == 0 ) return(NULL);

   node = (void *)malloc(size);
   if ( node == NULL) return(NULL);

   memcpy(node, ptr, size);
   free( ptr );
   return( node );
}

/* timing things... */

static int _time0[3];

static void _clock_init()                /* private - for initialisation */
{
   ndatestamp(_time0);      /* For now */
}

/* Exported... */

clock_t clock()
{  int t[3];
   clock_t n;

   ndatestamp(t);

   n  = (t[0]-_time0[0])*24*60;                    /* Mins since start */
   n  = (n+(t[1]-_time0[1]))*60*CLK_TCK;           /* CLK_TCKs */
   n += (t[2]-_time0[2])*CLK_TCK/TICKS_PER_SECOND; /* CLK_TCKs */
   return (n);
}

time_t time(time_t *timer)
/* this version gives the UNIX result of secs since 1-jan-1970 */
{
   time_t r;
   int t[3];
   ndatestamp(t);
   r = ( (t[0]+365*8+2)*24*60 + t[1])*60+t[2]/50 ;
   if( timer != NULL ) *timer = r;
   return( r );
}

static int _saved_interrupt;
int _interrupts_off = 1;    /* Public */

#ifdef AMIGA
static void _checkbreak(void)
{
   if( nsetsignal(0,0x3000) & 0x3000)
   {
      if( _interrupts_off )
         _saved_interrupt = SIGINT;
      else
         raise(SIGINT);
   }
}
#else
static void _checkbreak(void)
{ }
#endif

void _raise_stacked_interrupts(void)
{  int s;
   _interrupts_off = 0;
   if( (s = _saved_interrupt) != 0 )
   {  _saved_interrupt = 0;
      raise(s);
   }
}

/* system dependent I/O routines ... */

/* _io_fatalerr is the (temporary) error handler for (unhandled) NIOP       */
/*  errors.  Note that NIOP calls disturb _io_r0 and _io_emsg so use WTO    */
/* (also in case stderr error).                                             */
static void _io_fatalerr(int cc)
{  cc = cc;
   printf("Fatal Error");
   exit(10);
}

FILEHANDLE _sys_open(const char *name, int openmode)
/* openmode is coded: a = 8, w = 4, + = 2, b = 1 */
{
#ifdef TRIPOS
    static int modtab[12] = { /* r */ MODE_OLDFILE, /* rb*/ -MODE_OLDFILE,
                              /* r+*/ MODE_UPDATE,  /* rb+*/ -MODE_UPDATE,
                              /* w */ MODE_NEWFILE, /* wb*/ -MODE_NEWFILE,
                              /* w+*/ MODE_NEWFILE, /* wb+*/ -MODE_NEWFILE,
                              /* a */ MODE_UPDATE,  /* ab*/ -MODE_UPDATE,
                              /* a+*/ MODE_UPDATE,  /* ab+*/ -MODE_UPDATE };
    return(nopen(name,modtab[openmode]));
#else /* AMIGA | NEWTRIPOS*/
    static int modtab[6] = { /* r */ MODE_OLDFILE, /* r+ */ MODE_OLDFILE,
                             /* w */ MODE_NEWFILE, /* w+ */ MODE_NEWFILE,
                             /* a */ MODE_NEWFILE, /* a+ */ MODE_NEWFILE };
    _checkbreak();
    return(nopen(name,modtab[openmode>>1]));
#endif
}

int _sys_write_(FILEHANDLE fh, unsigned char *buf, int len, int mode)
{
    _checkbreak();
    return(nwrite(fh, buf, len) != len);
}

int _sys_read_(FILEHANDLE fh, unsigned char *buf, int len, int mode)
{
    _checkbreak();
    return(len-nread(fh,buf,len));
}

int _sys_close_(FILEHANDLE fh)
{
#ifdef AMIGA
    if( fh == _initial_input ) return;
    if( fh == _initial_output ) return;
#endif
    _checkbreak();
    nclose(fh);
}

int _sys_seek_(FILEHANDLE fh, int n)
{
    _checkbreak();
    return(nseek(fh,n,-1));
}

int _sys_flen_(FILEHANDLE fh)
{
   int cp;
    _checkbreak();
   cp = nseek(fh,0,1);

   return( nseek(fh,cp,-1));
}

int _sys_istty_(FILEHANDLE fh)
{
    _checkbreak();
   return( nisinteractive(fh));
}

static int _error_recursion;
void _sys_msg(const char *s)
{   /* write out s carefully for intimate system use.                      */
    if ((stderr->_flag & _IOWRITE) && !_error_recursion)
    {   _error_recursion = 1;
        fprintf(stderr, "\n%s\n", s);
        _error_recursion = 0;
    }
    else
    {   nwrite(stdout->_file,(unsigned char *)"\n", 1);
        nwrite(stdout->_file,(unsigned char *)s, strlen(s));
        nwrite(stdout->_file,(unsigned char *)"\n", 1);
    }
}


int remove(const char *pathname)
{
    _checkbreak();
    return !ndeletefile(pathname);
}

int rename(const char *old, const char *new)
{
    _checkbreak();
   return !nrenamefile(old,new);
}

char *getenv(const char *name)
{
/* This is pretty unsatisfactory, but I do not know what more to do!     */
    name = name;  /* stop warning for pointless procedure */
    return NULL;
}

int system(const char *string)
{
    if (string==NULL) return 0;
    _checkbreak();
    _sysdie("system() unimplemented");
}

static void *top_of_stack;          /* needs rework for stack extension */

static void *_codebase, *_codelimit; /* For profile option */
extern void _count(void);
#ifdef NEWTRIPOS
extern LONG *_DOSBase;
#endif
#ifdef AMIGA
extern struct Library *_DOSBase;
#endif

extern void _initexcepts(void);

void _main(char *argstr,int argl)
{   char ch, *s = argstr;
    int argc = 0, i = 0, curarg = 0, len = argl;
#ifdef AMIGA
    char *stdinfile = NULL;
    char *stdoutfile = NULL;
    char *stderrfile = NULL;
#else  /* TRIPOS | NEWTRIPOS */
    char *stdinfile = "*";
    char *stdoutfile = "*";
    char *stderrfile = "*";
#endif

    static char *argv[20+1];  /* from ACN's arm code ...                   */
    static char args[400];    /* ... should use a primitive alloc.         */
    _error_recursion = 0;
#ifdef TRIPOS
    if( initpkt ) tr_initio();
#endif

#ifdef AMIGA
    _DOSBase = nopenlibrary("dos.library",0);
/* The next two lines save the current input and output file
   handles so that we don't close them in _sys_close */
    _initial_input = ninput();
    _initial_output = noutput();
#endif
#ifdef NEWTRIPOS
    _DOSBase = FindDOS();
#endif
#ifdef never
    top_of_stack = topofstack;
    _codebase = codebase;
    _codelimit = codelimit;
#endif

    _signal_init();         /* had better be done pretty early           */
    _exit_init();           /* must happen before exit() can be called   */
    _clock_init();          /* set Cpu time zero point                   */
    _mem_init();

#ifdef AMIGA
    _initexcepts();
#endif
/*    _initalloc(0,0);                            / * as had the allocator  */
    argv[argc++] = "";      /* cannot yet determine name                 */
    do
    {   ch = (len-- > 0 ? *s++ : 0);
        if (ch == 0 || ch == ' ' || ch == '\n')
        {   if (i != curarg)
            {   args[i++] = 0;
                switch (args[curarg])
                {   default:  if (argc < 20) argv[argc++] = &args[curarg];
                              break;
                    case '<': if (args[curarg+1]) stdinfile = &args[curarg+1];
                              break;
                    case '>': if (args[curarg+1])
                              {   if(args[curarg+1] != '>')
                                    stdoutfile = &args[curarg+1];
                                  else
                                    if (args[curarg+2])
                                       stderrfile = &args[curarg+2];
                              }
                              break;
                }
                curarg = i;
            }
        }
        else args[i++] = ch;
    } while (ch != 0 && i < 400);
    argv[argc] = 0;      /* for ANSI spec */
    _initio(stdinfile, stdoutfile, stderrfile);
/* SIGINT events are not safe until about now.                           */
    _raise_stacked_interrupts();       /* enable SIGINT                  */
    exit(main(argc, argv));
}

void _mapstore()
{
   fprintf(stderr,"Mapstore requested\n");
}

void _backtrace(int why, int *address, int *fp, int *sp, int *pc)
{
   fprintf(stderr,"Backtrace requested\n");
   exit(1);
}

void _write_profile(char *filename)
{
   fprintf(stderr,"Profile option not yet implemented\n");
}

#if defined TRIPOS || defined NEWTRIPOS
void _postmortem(void)
{
   _sys_msg("Postmortem dump requested - not yet implemented\n");
   exit(1);
}
#endif
/* end of mc68000sys.c */
