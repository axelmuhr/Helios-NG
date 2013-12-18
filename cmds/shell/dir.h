/**
*
* Title:  Dir.h
*
* $Header: /hsrc/cmds/shell/RCS/dir.h,v 1.1 1990/08/23 15:51:15 james Exp $
*
**/

#define MAXNAMELEN 32
#define DIRBLKSIZE 512

struct direct
{
  u_long d_ino;
  u_short d_reclen;
  u_short d_namelen;
  char d_name[MAXNAMELEN + 1];
};

typedef struct _dirdesc
{
  int dd_fd;
  long dd_loc;
  long dd_size;
  char dd_buf[DIRBLKSIZE];
} DIR;

