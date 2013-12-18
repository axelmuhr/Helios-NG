
/* stdio.h: ANSI draft (X3J11 Oct 86) library header, section 4.9 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.06c */

#ifndef __stdio_h
#define __stdio_h

#define __LIB_VERSION 160       /* 1.60, but int for PP inequality test */

/* N.B. it is far from clear that stdio.h should be defining these types: */
#ifndef size_t
#  define size_t unsigned int   /* see <stddef.h> */
#endif

#ifndef __va_list_defined
typedef char *__va_list[1];       /* see <stdarg.h> */
#define __va_list_defined
#endif

#ifndef NULL
/* I think that it is legitimate for <stdio.h> to define NULL since many */
/* routines declared here return a null pointer in certain cases.        */
#  define NULL 0                /* see <stddef.h> */
#endif

#ifndef errno
/* Perhaps ansi will legitamise this too (see next draft)                */
extern volatile int _errno;
#define errno _errno
#endif

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
  FILEHANDLE _file;
  long _pos;
  int _bufsiz;
  int _signature;
/* The next line used to read -
  unsigned char _lilbuf[1];
   but if we allow structures to be short word aligned then
   then _lilbuf will only be padded to 16 bits instead of
   32 which assumed by the !__system_io case. (CGS 7-sep-88)
*/
  unsigned char _lilbuf[4];
#else
  int :32,:32,:32,:32,:32,:32;
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

#define BUFSIZ   4096
#define EOF      (-1)
#define L_tmpnam 20

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* It is not clear to me what value OPEN_MAX should have, so I will
   err in the cautious direction - ANSI requires it to be at least 8 */
#define OPEN_MAX 8
/* _SYS_OPEN defines a limit on the number of open files that is imposed
   by this C library */
#define _SYS_OPEN 16
#define TMP_MAX 1000000000

extern FILE _iob[];

#define stdin  (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])

extern int remove(const char *filename);
extern int rename(const char *old, const char *new);
extern FILE *tmpfile(void);
extern char *tmpnam(char *s);

extern int fclose(FILE *stream);
extern int fflush(FILE *stream);
extern FILE *fopen(const char *filename, const char *mode);
extern FILE *freopen(const char *filename, const char *mode, FILE *stream);
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
#pragma -v0   /* back to default */
extern int vprintf(const char *format, __va_list arg);
extern int vfprintf(FILE *stream, const char *format, __va_list arg);
extern int vsprintf(char *s, const char *format, __va_list arg);

extern int fgetc(FILE *stream);
extern char *fgets(char *s, int n, FILE *stream);
extern int fputc(int c, FILE *stream);
extern int fputs(const char *s, FILE *stream);
extern int _fillbuf(FILE *stream);
#define getc(p) \
    (--((p)->_icnt) >= 0 ? *((p)->_ptr)++ : _fillbuf(p))
extern int (getc)(FILE *stream);
#define getchar() getc(stdin)
extern int (getchar)(void);
extern char *gets(char *s);
extern int _flushbuf(int ch, FILE *stream);
#define putc(ch, p) \
    (--((p)->_ocnt) >= 0 ? (*((p)->_ptr)++ = (ch)) : _flushbuf(ch,p))
extern int (putc)(int c, FILE *stream);
#define putchar(ch) putc(ch, stdout)
extern int (putchar)(int c);
extern int puts(const char *s);
extern int ungetc(int c, FILE *stream);

extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);

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
extern void perror(const char *s);

#endif

/* end of stdio.h */
