/* stdio.c: ANSI draft (X3J11 Oct 86) library code, section 4.9 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.17e */

/* Incorporate _sys_tmpnam_ idea from WGD */

/* _pos of a file is now a long, and we avoid warnings by two casts         */
/* of this to (int) in the calls to _sys_seek().                            */
/* N.B. I am trying to factor out machine dependence via calls to           */
/* routines like _sys_read_ which can be implemented as _osgbpb or          */
/* NIOP as required.  This file SHOULD therefore be machine independent     */

/* the #include <stdio.h> imports macros getc/putc etc.  Note that we
   must keep the two files in step (more details in ctype.c).
*/
#include "hostsys.h"   /* _sys_alloc() etc */
#include <stdio.h>     /* macros for putc, getc, putchar, getchar */
#include <string.h>    /* for memcpy etc */
#include <stddef.h>    /* for size_t etc */
#include <stdlib.h>    /* for free() */
#include <time.h>      /* for time() for tmpfil() */

#ifndef _SYS_OPEN
/* Temp feature to cope with upgrade path of compiler */
#define _SYS_OPEN SYS_OPEN
#endif

FILE _iob[_SYS_OPEN];
#define LINBUFSIZ 132
/* Note that the real name for this variable is _errno, and the macro    */
/* errno is defined in math.h and stddef.h                               */
volatile int errno;                              /* error code           */

/* Explanation of the _IOxxx bits:                                       */
/* IOFP1 says that the file is positioned at the end of the current      */
/* buffer (as when performing sequential reads), while IOFP2 indicates   */
/* that its position is at the start of the current buffer (as in the    */
/* usual case when writing a file). When the relevant bit is not set     */
/* and a transfer is needed _sys_seek() is used to reposition the file.  */
/* Now extra bit _IOSEEK indicating that repositioning of the file       */
/* is required before performing I/O                                     */
/* I now use IOFP1 and IOFP2 to check the restriction                    */
/* that read and write operations must be separated by fseek/fflush.     */

/* N.B. bits up to (and including 0xfff) in <stdio.h>                    */
#define _IOFP1    0x1000        /* last op was a read                    */
#define _IOFP2    0x2000        /* last op was a write                   */
#define _IOPEOF   0x4000        /* 'pending' EOF                         */
#define _IOAPPEND 0x8000        /* must seek to eof before any write     */
#define _IODEL    0xdead0000    /* for safety check 16 bits              */
#define _IODELMSK 0xffff0000

/* first functions for macros in <stdio.h>  */
int (fgetc)(FILE *stream) { return getc(stream); }
int (fputc)(int ch, FILE *stream) { return putc(ch, stream); }
int (getc)(FILE *stream) { return getc(stream); }
int (getchar)() { return getchar(); }
int (putc)(int ch, FILE *stream) { return putc(ch, stream); }
int (putchar)(int ch) { return putchar(ch); }
int (feof)(FILE *stream) { return feof(stream); }
int (ferror)(FILE *stream) { return ferror(stream); }

/* put this here too */
void clearerr(FILE *stream)
{   /* we should do more in 'clearerr' resetting _pos _ptr and _cnt      */
    stream->_flag &= ~(_IOEOF+_IOERR+_IOPEOF);
}

#define seterr(stream) ((stream)->_flag |= _IOERR)

int setvbuf(FILE *stream, char *buf, int type, size_t size)
{   int flags = stream -> _flag;
    unsigned char *ubuf = (unsigned char *) buf;   /* costs nothing */
    if (!(flags & _IOREAD+_IOWRITE) || (flags & _IONBF+_IOFBF+_IOLBF+_IOSBF))
        return 1;             /* failure - not open, or already buffered */
    switch (type)
    {   default: return 1;    /* failure */
        case _IONBF:
            ubuf = stream->_lilbuf;
            size = 1;
            break;
        case _IOLBF:
        case _IOFBF:
            if (size-1 >= 0xffffff) return 1;  /* unsigned! */
            break;
    }
    stream -> _ptr = stream -> _base = ubuf;
    stream -> _bufsiz = size;
    stream -> _flag |= type;
    return 0;                 /* success */
}

void setbuf(FILE *stream, char *buf)
{   (void) setvbuf(stream, buf, (buf==0 ? _IOFBF : _IONBF), BUFSIZ);
}

int ungetc(int c,FILE *stream)
{   /* made into a fn to evaluate each arg once. */
    /* we should allocate a buffer if no reads so far when called */
    if (c==EOF || stream->_ptr == stream->_base) return EOF;
    (stream->_icnt)++;
/* The next line avoids writing to a scanf pseudo-stream (maybe write-      */
/* protected string).  Scanf guarantees to only ungetc() last getc()'d char */
    if (stream->_flag & _IOSTRG) return (unsigned char)(--stream->_ptr,c);
    return *--(stream->_ptr) = c;
}

static int _writebuf(unsigned char *buf, int len, FILE *stream)
{   int w;
    FILEHANDLE fh = stream->_file;
    int flag = stream->_flag;
    if (flag & _IOSEEK)
    {   if (_sys_seek_(fh, (int)stream->_pos) < 0)
        {   seterr(stream);
            return EOF;
        }
        stream->_flag = (flag &= ~_IOSEEK);
    }
    w = _sys_write_(fh, buf, len, flag);
    stream->_pos += len - (w & 0x7fffffffL);
    if (w!=0)    /* AM: was (w<0) but trap unwritten chars as error too */
    {   seterr(stream);
        return(EOF);
    }
    return 0;
}

int _flushbuf(int ch, FILE *stream)
{   int flag = stream -> _flag;
    if ((flag & _IOERR+_IOSTRG+_IOWRITE+_IOFP1) != _IOWRITE)
    {   stream->_ocnt = 0;                        /* 2^31 is big but finite */
        seterr(stream);
        return EOF;
    }
/* the next conditional code is ACN's view of that APPEND means seek to     */
/* EOF after EVERY fflush, not just initially.  Hmm, ANSI really should     */
/* clarify - the problem is perhaps that we wish to seek to EOF after       */
/* fflush after read, but not after fflush after write?                     */
    if ((flag & (_IOFP2+_IOSEEK+_IOAPPEND)) == _IOAPPEND)
    {   /* first write to APPEND file after FFLUSH, but not FSEEK nor       */
        /* fopen (does its own FSEEK)                                       */
        fseek(stream, 0L, SEEK_END);
        flag = stream->_flag;
    }
    stream->_flag = (flag |= _IOFP2);             /* we are writing         */
    if ((flag & _IOFBF+_IOSBF+_IONBF+_IOLBF) == 0)
    {   if (_sys_istty_(stream->_file))           /* terminal - unbuffered  */
#ifdef LINE_BUFFERED_TTYIO
            stream->_ptr = stream -> _base = _sys_alloc(LINBUFSIZ),
            stream->_bufsiz = LINBUFSIZ,
            stream->_flag |= (flag |= _IOLBF);
#else
            stream->_ptr = stream->_base = stream->_lilbuf,
            stream->_bufsiz = 1,
            stream->_flag = (flag |= _IONBF);
#endif
        else
            /* allocate default system buffer */
            stream->_ptr = stream->_base = _sys_alloc(BUFSIZ),
            stream->_bufsiz = BUFSIZ,
            stream->_flag |= (flag |= _IOSBF);
    }
    if (flag & _IOFBF+_IOSBF)    /* system or user buffer */
    {   unsigned char *buff = stream->_base;
        int count = stream->_ptr - buff;
        if (count != 0)
        {   if (_writebuf(buff, count, stream)) return EOF;
        }
        stream->_ptr = buff+1;
        stream->_ocnt = stream->_bufsiz - 1;
        return (*buff = ch);
    }
    else     /* no buffer (i.e. 1 char private one) or line buffer */
    {   unsigned char *buff = stream->_base;
        int count;
        *stream->_ptr++ = ch;   /* always room */
        count = stream->_ptr - buff;
        if ((flag & _IONBF) ||
               (unsigned char)ch == '\n' || count >= stream->_bufsiz )
        {   stream->_ptr = buff;
            stream->_ocnt = 0;                 /* 2^31 is big but finite */
            return _writebuf(buff, count, stream) ? EOF : (unsigned char)ch;
        }
        return (unsigned char)ch;
    }
}

int fflush(FILE *stream)
{
    if ((stream->_flag & _IOERR+_IOWRITE) != _IOWRITE) return EOF;
    /* N.B. really more to do here for ANSI input stream */
    if (stream->_flag & _IOFP2)
    {   /* only write if dirty buffer - this avoids problems with
           writing to a file opened in append (or read+write) mode
           when only input has been done since the last fflush/fseek.
        */
        unsigned char *buff = stream->_base;
        if (stream->_ptr != buff)       /* nothing to do */
        {   if (_writebuf(buff, stream->_ptr - buff, stream)) return EOF;
        }
        stream->_ptr = buff;
/* the next line forces a call to _fillbuf/_flushbuf on next putc/getc -    */
/* this is necessary since change of direction may happen.                  */
        stream->_ocnt = 0;
        stream->_flag &= ~_IOFP2;
    }
    return 0;
}

int _fillbuf(FILE *stream)
{   int w;
    unsigned char *buff;
    FILEHANDLE fh;
    int flag = stream->_flag;
/* note that sscanf (q.v.) requires this next line to yield EOF */
    if ((flag & (_IOEOF+_IOERR+_IOSTRG+_IOREAD+_IOPEOF+_IOFP2)) != _IOREAD)
    {   stream->_icnt = 0;                      /* 2^31 is big but finite */
        if (flag & _IOEOF+_IOPEOF)
            /* writing ok after EOF read according to ansi */
            stream->_flag = flag & ~(_IOFP1+_IOPEOF) | _IOEOF;
        else seterr(stream);
        return(EOF);
    }
    stream->_flag = (flag |= _IOFP1);           /* we are reading         */
    if ((flag & _IOFBF+_IOSBF+_IONBF+_IOLBF) == 0)
    {
            /* allocate default system buffer */
            stream->_ptr = stream->_base = _sys_alloc(BUFSIZ),
            stream->_bufsiz = BUFSIZ,
            stream->_flag = (flag |= _IOSBF);
    }
    buff = stream->_base;
    stream->_pos += stream->_ptr - buff;     /* add buf size for ftell() */
    fh = stream->_file;
    if (flag & _IOSEEK)
    {   if (_sys_seek_(fh, (int)stream->_pos) < 0)
        {   seterr(stream);
            return EOF;
        }
        stream->_flag = (flag &= ~_IOSEEK);
    }
    w = _sys_read_(fh, buff, stream->_bufsiz, flag);
    if (w<0)     /* this deals with operating systems with 'early' eof */
    {   stream->_flag |= _IOPEOF;
        w = w & 0x7fffffff;
    }
    w = stream->_bufsiz - w;
    if (w==0)    /* this deals with operating systems with 'late' eof  */
    {   stream->_flag |= _IOEOF;                /* is this case independent? */
        stream->_flag &= ~_IOFP1;               /* writing OK after EOF read */
        stream->_icnt = 0;
        stream->_ptr = buff;  /* just for fun - NB affects ftell() - check */
        return(EOF);
    }
    else
    {   stream->_icnt = w-1;
        stream->_ptr = buff+1;
        return(buff[0]);
    }
}

static int _fillb2(FILE *stream)
{   if (_fillbuf(stream) == EOF) return EOF;
    stream->_icnt++;
    stream->_ptr--;
    return 0;
}

int fclose(FILE *stream)
{   /* MUST be callable on a closed file - if stream clr then no-op. */
    FILEHANDLE fh = stream->_file;
    unsigned char *buff = stream->_base;
    int flag = stream->_flag;
    if (!(flag & _IOREAD+_IOWRITE)) return 1;   /* already closed        */
    if (!(flag & _IOSTRG))                      /* from _fopen_string    */
    {   fflush(stream);
        _sys_close_(fh);                        /* close real file       */
        if (flag & _IOSBF) free(buff);          /* free buffer if system */
        if ((flag & _IODELMSK) == _IODEL)
        {   char name[L_tmpnam];
            _sys_tmpnam_(name, stream->_signature);
            remove(name);                 /* delete the file if possible */
        }
    }
    memclr(stream, sizeof(FILE));
    return 0;                               /* success */
}

#if defined AMIGA
static FILE *_fdreopen(FILEHANDLE fh, const char *mode, FILE *iob)
{
    int flag, openmode;        /* nasty magic numbers for openmode */
    fclose(iob);
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
/* The only difference between this and freopen is the
   in freopen there follows a _sys_open; in this routine
   the FILEHANDLE fh is assumed to be open in the
   required mode already */

    iob->_flag = flag;
    iob->_file = fh;
    if (openmode & 8) fseek(iob, 0L, SEEK_END);  /* a or a+             */
    return iob;
}
#endif

FILE *freopen(const char *name, const char *mode, FILE *iob)
{
/* The use of modes "r+", "w+" and "a+" is not fully thought out   */
/* yet, in that calls to _flushbuf may write back stuff that was   */
/* loaded by _fillbuf and thereby corrupt the file.                */
/* This is now just about fixed given the ANSI restriction that    */
/* calls to getc/putc must be fflush/fseek separated.              */
    FILEHANDLE fh;
    int flag, openmode;        /* nasty magic numbers for openmode */
    fclose(iob);
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
    if ((fh = _sys_open(name, openmode)) == NONHANDLE) return NULL;
    iob->_flag = flag;
    iob->_file = fh;
    if (openmode & 8) fseek(iob, 0L, SEEK_END);  /* a or a+             */
    return iob;
}

FILE *fopen(const char *name, const char *mode)
{   int i;
    for (i=3; i<_SYS_OPEN; i++)
    {   FILE *stream = &_iob[i];
        if (!(stream->_flag & _IOREAD+_IOWRITE))  /* if not open then try it */
            return(freopen(name, mode, stream));
    }
    return 0;   /* no more i/o channels allowed for */
}

FILE *_fopen_string_file(const char *data, int length)
{
/* open a file that will read data from the given string argument        */
/* The declaration of this function in #include "hostsys.h" suggests     */
/* that this function is of type (void *), so this definition will lead  */
/* to a warning message.                                                 */
    int i;
    for (i=3; i<_SYS_OPEN; i++)
    {   FILE *stream = &_iob[i];
        if (!(stream->_flag & _IOREAD+_IOWRITE))  /* if not open then try it */
        {
            fclose(stream);
            stream->_flag = _IOSTRG+_IOREAD;
            stream->_ptr = stream->_base = (unsigned char *)data;
            stream->_icnt = length;
            return stream;
        }
    }
    return 0;   /* no more i/o channels allowed for */
}

int _fisatty(FILE *stream)   /* not in ANSI, but related needed for ML */
{   if ((stream->_flag & _IOREAD) && _sys_istty_(stream->_file)) return 1;
    return 0;
}

/* initialisation/termination code... */


void _initio(char *f1,char *f2,char *f3)
{
    char v[128];
    memclr(_iob, sizeof(_iob));
    /* In the next lines DO NOT use standard I/O for error msgs (not open yet)
       Moreover, open in this order so we do not create/overwrite output if
       input does not exist. */
#ifdef AMIGA
    if ( f3 == NULL ) f3 = "*";
#endif
    if (freopen(f3, "w", stderr) == 0)
        sprintf(v,"Couldn't write %s", f3), _sys_msg(v), exit(1);
#ifdef AMIGA
    if ( f1 == NULL )
      _fdreopen(ninput(), "r", stdin);
    else
#endif
    if (freopen(f1, "r", stdin) == 0)
        sprintf(v,"Couldn't read %s", f1), _sys_msg(v), exit(1);
#ifdef AMIGA
    if ( f2 == NULL )
      _fdreopen(noutput(), "w", stdout);
    else
#endif
    if (freopen(f2, "w", stdout) == 0)
        sprintf(v,"Couldn't write %s", f2), _sys_msg(v), exit(1);
}

void _terminateio()
{   int i;
    for (i=3; i<_SYS_OPEN; i++) fclose(&_iob[i]);
    /* for cowardice do stdin, stdout, stderr last (in that order) */
    for (i=0; i<3; i++) fclose(&_iob[i]);
}


/* now the less machine dependent functions ... */

char *fgets(char *s, int n, FILE *stream)
{   char *a = s;
    if (n <= 1) return NULL;                  /* best of a bad deal */
    do { int ch = getc(stream);
         if (ch == EOF)                       /* error or EOF       */
         {   if (s == a) return NULL;         /* no chars -> leave  */
             if (ferror(stdin)) a = NULL;
             break; /* add NULL even if ferror(), spec says 'indeterminate' */
         }
         if ((*s++ = ch) == '\n') break;
       }
       while (--n > 1);
    *s = 0;
    return a;
}        

char *gets(char *s)
{   char *a = s;
    for (;;)
    {    int ch = getc(stdin);
         if (ch == EOF)                       /* error or EOF       */
         {   if (s == a) return NULL;         /* no chars -> leave  */
             if (ferror(stdin)) a = NULL;
             break; /* add NULL even if ferror(), spec says 'indeterminate' */
         }
         if (ch == '\n') break;
         *s++ = ch;
    }
    *s = 0;
    return a;
}        

int fputs(const char *s, FILE *stream)
{
    char c;
    while((c = *s++) != 0)
        if(putc(c, stream)==EOF) return(EOF);
    return(0);
}

int puts(const char *s)
{
    char c;
    while ((c = *s++) != 0)
       if (putchar(c) == EOF) return EOF;
    return putchar('\n');
}

/* _read improved to use _fillbuf and block moves.  Optimisation
   to memcpy too if word move.  Still possible improvments avoiding copy
   but I don't want to do these yet because of interactions
   (e.g. _pos of a file).   N.B.  _read is not far from unix 'read' */
int _read(char *ptr, int nbytes, FILE *stream)
{   int i = nbytes;
    do
    {   if (i <= stream->_icnt)
        {   memcpy(ptr, stream->_ptr, i);
            stream->_icnt -= i; stream->_ptr += i;
            return nbytes;
        }
        else
        {   memcpy(ptr, stream->_ptr, stream->_icnt);
            ptr += stream->_icnt, i -= stream->_icnt;
            stream->_ptr += stream->_icnt, stream->_icnt = -1; /* for _pos */
        }
    } while (_fillb2(stream) != EOF);
    return nbytes-i;
/*
    for (i=0; i<nbytes; i++)
    {   if ((ch = getc(stream)) == EOF) return i;
        *ptr++ = ch;
    }
    return nbytes;
*/
}

size_t fread(void *ptr, size_t itemsize, size_t count, FILE *stream)
{    /* ANSI spec says EOF and ERR treated the same as far as fread
      * is concerned and that the number of WHOLE items read is returned.
      */
    return itemsize == 0 ? 0   /* slight ansi irrationality */
                         : _read(ptr, itemsize*count, stream) / itemsize;
}

int _write(const char *ptr, int nbytes, FILE *stream)
{   int i;
    for(i=0; i<nbytes; i++)
        if (putc(*ptr++, stream) == EOF) return 0;
        /* H&S say 0 on error */
    return nbytes;
}

size_t fwrite(const void *ptr, size_t itemsize, size_t count, FILE *stream)
{
/* The comments made about fread apply here too */
    return itemsize == 0 ? count
                         : _write(ptr, itemsize*count, stream) / itemsize;
}

/* back to machine dependent functions */

#define _ftell(stream) ((stream)->_pos + (stream)->_ptr - (stream)->_base)

long int ftell(FILE *stream)
{
    return _ftell(stream);
}

/* The treatment of files that can be written to seems complicated in fseek */

int fseek(FILE *stream, long int offset, int whence)
{
    FILEHANDLE fh = stream->_file;
    unsigned char *buff = stream->_base;

    if (!(stream->_flag & _IOREAD+_IOWRITE) || _sys_istty_(fh))
        return(2);                              /* fseek impossible  */

    switch(whence)
    {
case SEEK_SET:
        break;                                  /* relative to file start */
case SEEK_CUR:
        offset += _ftell(stream);               /* relative seek */
        break;
case SEEK_END:
        {   int filelen, filepos;
            filelen = _sys_flen_(fh);           /* length of this file      */
            if (filelen<0)                      /* failed to read length    */
            {   seterr(stream);
                return 1;
            }
            filepos = _ftell(stream);
            if (filepos>filelen)                /* only possible on write   */
                filelen = filepos;              /* allow for stuff buffered */
            offset += filelen;                  /* relative to end of file  */
        }
        break;
default:
        return(2);                              /* illegal operation code   */
    }

    fflush(stream);                             /* may be over-keen         */
    stream->_flag = stream->_flag & ~(_IOEOF | _IOFP1 | _IOFP2 | _IOPEOF)
                                  | _IOSEEK;    /* clear EOF condition      */
    stream->_icnt = stream->_ocnt = 0;
    stream->_pos = offset;
    stream->_ptr = buff;
    return 0;
}

void rewind(FILE *stream)
{
    fseek(stream, 0L, SEEK_SET);
    clearerr(stream);
}

/* the following routines need to become the main entry I suppose          */
int fgetpos(FILE *stream, fpos_t *pos)
{  pos->lo = ftell(stream);
   return 0;
}

int fsetpos(FILE *stream, const fpos_t *pos)
{  fseek(stream, pos->lo, SEEK_SET);
   return 0;
}

static char _tmp_file_name[L_tmpnam];
static int _tmp_file_ser = 0;

char *tmpnam(char *a)
{
/* Obtaining an unique name is tolerably nasty - what I do here is       */
/* derive the name (via _sys_tmpnam_())                                   */
/* from an integer that is constructed out of a serial number combined   */
/* with the current clock setting. An effect of this is that the file    */
/* name can be reconstructed from a 32-bit integer for when I want to    */
/* delete the file.                                                      */
    int signature = ((int)time(NULL) << 8) | (_tmp_file_ser++ & 0xff);
    if (a==NULL) a = _tmp_file_name;
    _sys_tmpnam_(a, signature);
    return a;
}

FILE *tmpfile()
{
    char name[L_tmpnam];
    int signature = ((int)time(NULL) << 8) | (_tmp_file_ser++ & 0xff);
    FILE *f;
    _sys_tmpnam_(name, signature);
    f = fopen(name, "w+b");
    if (f)
    {   f->_flag |= _IODEL;
        f->_signature = signature;
    }
    return f;
}

/* end of stdio.c */
