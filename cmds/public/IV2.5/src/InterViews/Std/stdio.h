/* 
 * C++ interface to C Standard I/O library
 */

#ifndef stdio_h

/* change the names to protect the type-checking */
#define fopen stdio_h_fopen
#define fdopen stdio_h_fdopen
#define freopen stdio_h_fdopen
#define popen stdio_h_popen
#define tmpfile stdio_h_tmpfile
#define ftell stdio_h_ftell
#define rewind stdio_h_rewind
#define setbuf stdio_h_setbuf
#define setbuffer stdio_h_setbuffer
#define setlinebuf stdio_h_setlinebuf
#define fgets stdio_h_fgets
#define gets stdio_h_gets
#define ctermid stdio_h_ctermid
#define cuserid stdio_h_cuserid
#define tempnam stdio_h_tempnam
#define tmpnam stdio_h_tmpnam
#define sprintf stdio_h_sprintf

/* use the standard C header file */
#include "//usr/include/stdio.h"

/* change the names back */
#undef fopen
#undef fdopen
#undef freopen
#undef popen
#undef tmpfile
#undef ftell
#undef rewind
#undef setbuf
#undef setbuffer
#undef setlinebuf
#undef fgets
#undef gets
#undef ctermid
#undef cuserid
#undef tempnam
#undef tmpnam
#undef sprintf

/* just in case standard header file didn't */
#ifndef stdio_h
#define stdio_h
#endif

extern FILE* fopen(const char* name, const char* type);
extern FILE* fdopen(int fd, const char* type);
extern FILE* freopen(const char* name, const char* type, FILE*);
extern int fclose(FILE*);
extern FILE* popen(const char* command, const char* type);
extern int pclose(FILE*);

extern int fread(char*, unsigned int, int, FILE*);
extern int fwrite(const char*, unsigned int, int, FILE*);
extern int fseek(FILE*, long, int);
extern long ftell(const FILE*);
extern int fflush(FILE*);
extern void rewind(FILE*);

extern char* fgets(char*, int, FILE*);
extern char* gets(char*);
extern int puts(const char*);
extern int fputs(const char*, FILE*);
extern int getw(FILE*);
extern int fgetc(FILE*);
extern int putw(int, FILE*);
extern int fputc(int, FILE*);
extern int ungetc(int, FILE*);

extern int printf(const char* ...);
extern int fprintf(FILE*, const char* ...);
extern int sprintf(char*, const char* ...);
extern int scanf(const char* ...);
extern int fscanf(FILE*, const char* ...);
extern int sscanf(char*, const char* ...);

#if !defined(clearerr)
extern void clearerr(FILE*);
#endif

extern void setbuf(FILE*, char*);

extern int _filbuf(FILE*);
extern int _flsbuf(unsigned, FILE*);

extern FILE* tmpfile();
extern char* ctermid(char*);
extern char* cuserid(char*);
extern char* tempnam(char*, char*);
extern char* tmpnam(char*);

#endif
