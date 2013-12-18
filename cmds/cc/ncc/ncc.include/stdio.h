#ifndef FILE

#include <stdarg.h>
/* IMPORT va_list FROM <stdarg.h> */

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned size_t;
#endif

typedef unsigned fpos_t;

#ifndef NULL
#define NULL ((void *)0)
#endif

#define _IOFBF     00000
#define _IOLBF     00200
#define _IONBF     00004
#define BUFSIZ  1024
#define EOF  (-1)

#define _NFILE  64   /* should equal NOFILE in /sys/h/param.h */
#define FOPEN_MAX     _NFILE

#define FILENAME_MAX  255
#define P_tmpdir        "/usr/tmp/"
#define L_tmpnam        (sizeof(P_tmpdir)+15)
#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2
#define _N_STATIC_IOBS 3
extern struct _iobuf {
  int     _cnt;
  char    *_ptr;
  char    *_base;
  int     _bufsiz;
  short   _flag;
  char    _file;
} _iob[_N_STATIC_IOBS];
#define FILE  struct _iobuf
#define stdin   (&_iob[0])
#define stdout  (&_iob[1])
#define stderr  (&_iob[2])
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
extern int fprintf(FILE *stream, const char *format, ...);
extern int fscanf(FILE *stream, const char *format, ...);
extern int printf(const char *format, ...);
extern int scanf(const char *format, ...);
#ifndef SYSTEM_FIVE
  extern char *sprintf(char *s, const char *format, ...);
#else
  extern int sprintf(char *s, const char *format, ...);
#endif
extern int sscanf(const char *s, const char *format, ...);
extern int vfprintf(FILE *stream, const char *format, va_list arg);
extern int vprintf(const char *format, va_list arg);
extern int vsprintf(char *s, const char *format, va_list arg);
extern int fgetc(FILE *stream);
extern char *fgets(char *s, int n, FILE *stream);
extern int fputc(int c, FILE *stream);
extern int fputs(const char *s, FILE *stream);
extern int getc(FILE *stream);
extern int _filbuf(FILE *stream);
extern int getchar(void);
extern char *gets(char *s);
extern int putc(int c, FILE *stream);
extern int _flsbuf(unsigned int c, FILE *stream);
extern int putchar(int c);
extern int puts(const char *s);
extern int ungetc(int c, FILE *stream);
extern size_t fread(void *ptr, size_t sizeofPtr, size_t nmemb, FILE *stream);
extern size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
extern int fgetpos(FILE *stream, fpos_t *pos);
extern int fseek(FILE *stream, long int offset, int whence);
extern int fsetpos(FILE *stream, const fpos_t *pos);
extern long int ftell(FILE *stream);
extern void rewind(FILE *stream);
extern void clearerr(FILE *stream);
#define feof(p)         (((p)->_flag&_IOEOF)!=0)
#define ferror(p)       (((p)->_flag&_IOERR)!=0)
extern void perror(const char *s);



#define _IOREAD    00001
#define _IOWRT     00002
#define _IOMYBUF   00010
#define _IOEOF     00020
#define _IOERR     00040
#define _IOSTRG    00100
#define _IORW      00400
#define _IOAPPEND  01000


#define getc(p)  (--(p)->_cnt>=0 ? *(p)->_ptr++&0377 : _filbuf(p))
#define getchar()  getc(stdin)
#define putc(x,p)  (  \
  --(p)->_cnt >= 0  ?  \
    ((int)(*(p)->_ptr++=(unsigned)(x)))  :  \
    _flsbuf((unsigned)(x),p))
#define putchar(x)      putc(x,stdout)
#define fileno(p)       ((p)->_file)

#define L_ctermid       9
#define L_cuserid       9


/* #define TMP_MAX       ? */

extern char *ctermid(char *s);
extern char *cuserid(char *s);
extern FILE *fdopen(int fildes, char *type);
/* extern int feof(FILE *stream); */
/* extern int ferror(FILE *stream); */
/* extern int fileno(FILE *stream); */
extern int getw(FILE *stream);
extern int pclose(FILE *stream);
extern FILE *popen(char *command, char *type);
extern int putw(int w, FILE *stream);
extern void setbuffer(FILE *stream, char *buf, int size);
extern void setlinebuf(FILE *stream);
extern char *tempnam(char *dir, char *pfx);


#endif
