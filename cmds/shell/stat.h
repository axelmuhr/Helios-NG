/* $Header: /hsrc/cmds/shell/RCS/stat.h,v 1.1 1990/08/23 16:07:24 james Exp $ */

typedef short dev_t;
typedef short ino_t;
typedef short off_t;
typedef short time_t;

struct stat
{
  dev_t st_dev;
  ino_t st_ino;
#ifdef OLDCODE
  u_short st_mode;
#else
  u_long st_mode;
#endif
  short st_nlink;
  short st_uid;
  short st_gid;
  dev_t st_rdev;
  off_t st_size;
  time_t st_atime;
  int st_spare1;
  time_t st_mtime;
  int st_spare2;
  time_t st_ctime;
  int st_spare3;
  long blksize;
  long st_blocks;
  long st_spare4[2];
};

#define S_IFDIR 0040000
#define S_IFREG 0100000
#define S_IEXEC 0000100

