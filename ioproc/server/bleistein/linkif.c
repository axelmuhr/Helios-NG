/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1990, Bleistein Rohde Sytemtechnik GmbH    --
--                        All Rights Reserved.                          --
--                                                                      --
--      linklibadd.c                                                    --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90\ Copyright (C) 1990 Bleistein Rohde Sytemtechnik GmbH */

#define Linklib_Module

#include "../helios.h"

#define FAIL 1
#define SUCCESS 0

void brs_init_link()
{ number_of_links = 1;
  strcpy(link_table[0].link_name, "/dev/la");
}

int  fn( OpenLink,       (char *));
int  fn( CloseLink,      (int));
int  fn( ReadLink,       (int, BYTE *, int, int));
int  fn( WriteLink,      (int, BYTE *, int, int));
int  fn( ResetLink,      (int));
int  fn( AnalyseLink,    (int));
int  fn( TestRead,       (int));
int  fn( TestWrite,      (int));


static UBYTE TestReadbuf[1];	/* analogue to ... in linklib.c */
static int TestReadflag=FALSE;


int OpenLink(dev)
char *dev;
{
  int fd;
  struct flock lck;
  
  if ((fd=open(dev, O_RDWR))==-1)
    {
      printf("BRS Error : OpenLink() failed.\r\n");
      return(-1);
    }
  
  lck.l_type=F_RDLCK;
  lck.l_whence=0;
  lck.l_start=0L;
  lck.l_len=0L;

  (void)fcntl(fd, F_GETLK, &lck);
  
  if (lck.l_type!=F_UNLCK)
    {
      printf("BRS Error : Linkdevice already locked.\r\n");
      (void)close(fd);
      return(-1);
    }

  lck.l_type=F_RDLCK;
  lck.l_whence=0;
  lck.l_start=0L;
  lck.l_len=0L;

  if (fcntl(fd, F_SETLK, &lck)==-1)
    {
      printf("BRS Error : LockLink() failed.\r\n");
      (void)close(fd);
      return(-1);
    }

  return (fd);
}

int CloseLink(fd)
int fd;
{
  struct flock lck;

  lck.l_type=F_UNLCK;
  lck.l_whence=0;
  lck.l_start=0L;
  lck.l_len=0L;

  if (fcntl(fd, F_SETLK, &lck)==-1)
    {
      printf("BRS Error : UnlockLink() failed.\r\n");
      return(-1);
    }

  if ((close(fd))==-1)
    {
      printf("BRS Error : CloseLink() failed.\r\n");
      return (-1);
    }

  return (SUCCESS);
}

int ReadLink(link_fd, buf, amount, timeout)
int link_fd; 
BYTE *buf;
int amount; 
int timeout;
{
  int res, count;
  
  count=0;

  if (TestReadflag==TRUE)
    {
      *buf=(BYTE)TestReadbuf[0];
      TestReadflag=FALSE;

      buf++;
      amount--;
      count++;
    }
  
  if (amount > 0)
    {
      if (((res=read(link_fd, buf, amount)))==-1)
	{
	  printf("BRS Error : ReadLink() failed.\r\n");
	  return (res);
	}
      
    }

  return (res+count);
}

int WriteLink(link_fd, buf, amount, timeout)
int link_fd; 
BYTE *buf;
int amount;
int timeout;
{
  int res;
  
  if ((res=(write(link_fd, buf, amount)))==-1)
    {
      printf("BRS Error : WriteLink() failed.\r\n");
    }

  return (res);
}

int ResetLink(link_fd)
int link_fd;
{
  if ((ioctl(link_fd, 0, NULL))==-1)
    {
      printf("BRS Error : Reset() failed.\r\n");
      return (FAIL);
    }

  /*C  INFORMATION: parameters 0 and NULL are kernel dummies up to now. */
  
  return (SUCCESS);
}

int AnalyseLink(link_fd)
int link_fd;
{
#ifdef BRSNOTYET
  /* involves...
  asserting the analyse line on the transputer
  short delay
  asserting the reset   line on the transputer
  delay
  releasing the reset   line on the transputer
  releasing the analyse line on the transputer
  */;
#else
  printf("BRS Information : AnalyseLink() not implemented yet.\r\n");
  return (FAIL);
#endif
  
}

int TestRead(link_fd)
int link_fd;
{
  int res;
  
  if (TestReadflag==FALSE)
    {
      if (((res=read(link_fd, TestReadbuf, 1)))==-1)
	{
	  printf("BRS Error : ReadLink() failed.\r\n");
	}
      else
	{
	  if (res==1)
	    {
	      TestReadflag=TRUE;
	    }
	}
    }
  else
    {
      res=1;
    }
  
  return (res);
}

int TestWrite(link_fd)
int link_fd;
{
  printf("BRS Information : TestWrite() not implemented yet.\r\n");
  return (FAIL);
}
