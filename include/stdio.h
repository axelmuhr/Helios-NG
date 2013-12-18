/* stdio.h: ANSI draft (X3J11 Oct 86) library header, section 4.9 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.06a - SccsId: %W% %G% */
/* RcsId: $Id: stdio.h,v 1.6 1993/07/06 13:30:25 nickc Exp $ */

#ifndef __stdio_h
#define __stdio_h

/* N.B. it is far from clear that stdio.h should be defining these types: */
#ifndef size_t
#  define size_t unsigned int   /* see <stddef.h> */
#endif

#if defined(__ARM) || defined(__C40)
typedef char *__va_list[1];       /* see <stdarg.h> */
#else
# ifndef __va_list_defined
typedef char *va_list[1] ;	/* see <stdarg.h> */
# define __va_list_defined
# endif /* __va_list_defined */
#endif /* __ARM || __C40 */

#ifndef NULL
/* I think that it is legitimate for <stdio.h> to define NULL since many */
/* routines declared here return a null pointer in certain cases.        */
#define NULL 0
#endif

extern int errno;		/* actually in POSIX library */

typedef struct _fpos_t
{ unsigned long lo;             /* add hi one day */
} fpos_t;

typedef struct _FILE
{ unsigned char *_ptr;
  int _icnt;      /* two separate _cnt fields so we can police ...        */
  int _ocnt;      /* ... restrictions that read/write are fseek separated */
  int _flag;
#ifdef __system_io
  unsigned char *_base;
  sysbase *_sysbase;		/* BLV - used for MSdos files */
  FILEHANDLE _file;
  int _pos;
  int _bufsiz;
  int _signature;
  unsigned char _lilbuf[1];
#else
  int :32,:32,:32,:32,:32,:32,:32;
#endif
} FILE;

#ifdef __system_io
# define _IOREAD   0x01
# define _IOWRITE  0x02
# define _IOBIN    0x04
# define _IOSTRG   0x08
# define _IOSEEK   0x10
# define _IOSBF   0x800
#endif
#define _IOEOF     0x40
#define _IOERR     0x80
#define _IOFBF    0x100
#define _IOLBF    0x200
#define _IONBF    0x400

#define BUFSIZ 1024
#define EOF      (-1)
#define L_tmpnam 32
#define L_ctermid 50

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

/* _SYS_OPEN defines a limit on the number of open files that is imposed
   by this C library */
#define _SYS_OPEN 16
#define TMP_MAX 1000000000

extern FILE _iob[_SYS_OPEN]; /* BLV - used to be _iob[], but this gave an
                                error */
#define stdin  (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])
#define stddbg (&_iob[2])

extern int remove(char const * filename);
extern int rename(char const * old_name, char const * new_name);
extern FILE *tmpfile(void);
extern char *tmpnam(char *s);

extern int fclose(FILE *stream);
extern int fflush(FILE *stream);
extern FILE *fopen(char const * filename, char const * mode);
extern FILE *freopen(char const * filename, char const * mode, FILE *stream);
extern void setbuf(FILE *stream, char *buf);
extern int setvbuf(FILE *stream, char *buf, int mode, size_t size);

#pragma -v1   /* hint to the compiler to check f/s/printf format */
extern int printf(const char *format, ...);
extern int fprintf(FILE *stream, const char *format, ...);
extern int sprintf(char *s, const char *format, ...);
#pragma -v2   /* hint to the compiler to check f/s/scanf format */
extern int scanf(const char *format, ...);
extern int fscanf(FILE *stream, const char *format, ...);
extern int sscanf(const char *s, const char *format, ...);
extern int _scanf(const char *format, ...);
extern int _fscanf(FILE *stream, const char *format, ...);
extern int _sscanf(const char *s, const char *format, ...);
#pragma -v0   /* back to default */

#if defined(__ARM) || defined(__C40)
extern int vprintf(const char *format, __va_list arg);
extern int vfprintf(FILE *stream, const char *format, __va_list arg);
extern int vsprintf(char *s, const char *format, __va_list arg);
extern int _vfprintf(FILE *stream, const char *format, __va_list arg);
#else
extern int vprintf(const char *format, va_list arg);
extern int vfprintf(FILE *stream, const char *format, va_list arg);
extern int vsprintf(char *s, const char *format, va_list arg);
extern int _vfprintf(FILE *stream, const char *format, va_list arg);
#endif /* __ARM || __C40 */
#define _VFPRINTF_AVAILABLE

extern int fgetc(FILE *stream);
extern char *fgets(char *s, int n, FILE *stream);
extern int fputc(int c, FILE *stream);
extern int fputs(char const *s, FILE *stream);
extern int _fillbuf(FILE *stream);
#define getc(p) (--((p)->_icnt) >= 0 ? *((p)->_ptr)++ : _fillbuf(p))
extern int (getc)(FILE *stream);
#define getchar() getc(stdin)
extern int (getchar)(void);
extern char *gets(char *s);
extern int _flushbuf(int ch, FILE *stream);
#define putc(ch, p) (--((p)->_ocnt) >= 0 ? (*((p)->_ptr)++ = (ch)) : _flushbuf(ch,p))
extern int (putc)(int c, FILE *stream);
#define putchar(ch) putc(ch, stdout)
extern int (putchar)(int c);
extern int puts(char const *s);
extern int ungetc(int c, FILE *stream);
#ifndef _POSIX_SOURCE
#ifdef _BSD
extern int putw(int w, FILE *stream);
extern int getw(FILE *stream);
extern FILE *popen(char *, char *);
extern int pclose(FILE *);
#endif
#endif

extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t fwrite(void const * ptr, size_t size, size_t count, FILE *stream);

extern int fgetpos(FILE *stream, fpos_t *pos);
extern int fseek(FILE *stream, long int offset, int whence);
extern int fsetpos(FILE *stream, const fpos_t *pos);
extern long int ftell(FILE *stream);
extern void rewind(FILE *stream);

extern void clearerr(FILE *stream);

#define feof(stream) ((stream)->_flag & _IOEOF)
extern int (feof)(FILE *stream);
#define ferror(stream) ((stream)->_flag & _IOERR)
extern int (ferror)(FILE *stream);
#ifndef _POSIX_SOURCE
#	ifdef _BSD
#	define perror bsd_perror
#	endif
#endif
extern void perror(const char *s);

extern int fileno(FILE *);		/* Clib stream -> POSIX fd convert   */
extern FILE *fdopen(int fd, char *mode);/* POSIX fd -> Clib FILE * 	     */

#endif

/* end of stdio.h */
