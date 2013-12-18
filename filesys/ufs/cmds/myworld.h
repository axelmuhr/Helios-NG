/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 25 August 1992                                               */
/* File: myworld.h                                                    */
/* 
 * The headers when using myworld.
 *
 * $Id: myworld.h,v 1.1 1992/09/16 10:01:43 al Exp $
 * $Log: myworld.h,v $
 * Revision 1.1  1992/09/16  10:01:43  al
 * Initial revision
 *
 */

#ifndef MYWORLD
#define MYWORLD
#define open(n,m)	MyOpen(n,m)
#define read(f,b,n)	MyRead(f,b,n)
#define write(f,b,n)	MyWrite(f,b,n)
#define close(f)	MyClose(f)
#define lseek(f,o,w)	MyLseek(f,o,w)
#define ioctl(f,c,a)	MyIoctl(f,c,a)
#define stat(p,b)	MyStat(p,b)

/* The prototypes */
int MyWorld(int flag);
int MyOpen(char *name, int mode);
int MyClose(int fd);
long MyLseek(int fd, int offset, int whence);
int MyIoctl(int fd, int cmd, caddr_t args);
int MyRead(int fd, char *buf, unsigned int num);
int MyWrite(int fd, char *buf, unsigned int num);
int MyStat(char *path, struct stat *buf);

#endif

