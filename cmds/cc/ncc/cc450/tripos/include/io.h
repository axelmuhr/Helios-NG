
/* <unix.io.h>: Simulation of Unix open/write etc in terms of <stdio.h> */
/* version 0.01: created AM 18-oct-86. */

#ifndef __unix_io_h
#define __unix_io_h

/* note that the following macro definitions rely on the internal       */
/* details of the NorCroft C <stdio> implementation.                    */

/* flags for OPEN */
#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWRITE 2

/* generates small code per module: */
static char _open_mode[][4] = {"r", "w", "r+"};
static FILE *_open_temp;
#define write(fd,buf,siz) fwrite(buf, 1, siz, &_iob[fd])
#define read(fd,buf,siz) fread(buf, 1, siz, &_iob[fd])
#define open(name,mode) \
  ((_open_temp = fopen(name,_open_mode[mode]))!=0 ? _open_temp - _iob : -1)
#define creat(name,prot) \
  ((_open_temp = fopen(name,"w"))!=0 ? _open_temp - _iob : -1)
#define close(fd) fclose(&_iob[fd])
#define lseek(fd,pos,org) fseek(&_iob[fd],pos,org)
extern int _fisatty(FILE *);
#define isatty(fd) _fisatty(&_iob[fd])
#define unlink(file) remove(file)
#define getpid() 1

#endif

/* end of <unix.io.h> */
