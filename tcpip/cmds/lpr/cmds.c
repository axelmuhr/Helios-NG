/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __HELIOS
#ifndef lint
static char sccsid[] = "@(#)cmds.c	5.4 (Berkeley) 6/30/88";
#endif /* not lint */
#else
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/tcpip/cmds/lpr/RCS/cmds.c,v 1.2 1994/03/17 17:06:36 nickc Exp $";
#endif
#endif

/*
 * lpc -- line printer control program -- commands:
 */

#include "lp.h"
#include <sys/time.h>
#ifdef __HELIOS
#include "printcap.h"
#include <syslib.h>

char mode_path [256] ;
int lock_mode ;

void abortpr(int);
void startpr(int);
#endif

/*
 * kill an existing daemon and disable printing.
 */
#ifndef __HELIOS
abort(argc, argv)
  char *argv[];
#else
void abort_ (int argc, char *argv[])
#endif
{
  register int c, status;
  register char *cp1, *cp2;
  char prbuf[100];

  if (argc == 1) {
    printf("Usage: abort {all | printer ...}\n");
    return;
  }
  if (argc == 2 && !strcmp(argv[1], "all")) {
    printer = prbuf;
    while (getprent(line) > 0) {
      cp1 = prbuf;
      cp2 = line;
      while ((c = *cp2++) != 0 && c != '|' && c != ':')
        *cp1++ = c;
      *cp1 = '\0';
      abortpr(1);
    }
    return;
  }
  while (--argc) {
    printer = *++argv;
    if ((status = pgetent(line, printer)) < 0) {
      printf("cannot open printer description file\n");
      continue;
    } else if (status == 0) {
      printf("unknown printer %s\n", printer);
      continue;
    }
    abortpr(1);
  }
}

void abortpr(int dis)
{
  register FILE *fp;
  struct stat stbuf;
#ifndef __HELIOS
  int pid, fd;
#else
  int fd;
  void upstat(char *) ;
#endif  

  bp = pbuf;
  if ((SD = pgetstr("sd", &bp)) == NULL)
    SD = DEFSPOOL;
  if ((LO = pgetstr("lo", &bp)) == NULL)
    LO = DEFLOCK;
  (void) sprintf(line, "%s/%s", SD, LO);
  printf("%s:\n", printer);

  /*
   * Turn on the owner execute bit of the lock file to disable printing.
   */
  if (dis) {
#ifndef __HELIOS
    if (stat(line, &stbuf) >= 0) {
      if (chmod(line, (stbuf.st_mode & 0777) | 0100) < 0)
        printf("\tcannot disable printing\n");
      else {
        upstat("printing disabled\n");
        printf("\tprinting disabled\n");
      }
    } else if (errno == ENOENT) {
      if ((fd = open(line, O_WRONLY|O_CREAT, 0760)) < 0)
        printf("\tcannot create lock file\n");
      else {
        (void) close(fd);
        upstat("printing disabled\n");
        printf("\tprinting disabled\n");
        printf("\tno daemon to abort\n");
      }
#else
    (void) sprintf (mode_path, "%s/%s", SD, DEFMODE) ;
/*
-- crf : in this instance, if lockmode does not exist, it should be
-- created with printing disabled ... so, test for this first
*/
    if (stat (mode_path, &stbuf) < 0)
    {
      debugf ("cmds: lockmode doesn't exist") ;
      if (set_mode (mode_path, 0760) < 0)
        debugf ("cmds: can't create lockmode") ;
    }
    if ((lock_mode = get_mode (mode_path)) < 0)
      debugf ("cmds: can't get mode") ;
    
    if (stat (line, &stbuf) >= 0) {
      if (set_mode (mode_path, (lock_mode & 0777) | 0100) < 0)
        printf("\tcannot disable printing\n");
      else {
        upstat("printing disabled\n");
        printf("\tprinting disabled\n");
      }
    } else if (errno == ENOENT) {
      if ((fd = open(line, O_WRONLY|O_CREAT, 0760)) < 0)
        printf("\tcannot create lock file\n");
      else {
        (void) close(fd);
        upstat("printing disabled\n");
        printf("\tprinting disabled\n");
        printf("\tno daemon to abort\n");
      }
#endif      
      return;
    } else {
      printf("\tcannot stat lock file\n");
      return;
    }
  }
  /*
   * Kill the current daemon to stop printing now.
   */
  if ((fp = fopen(line, "r")) == NULL) {
    printf("\tcannot open lock file\n");
    return;
  }
#ifndef __HELIOS
  if (!getline(fp) || flock(fileno(fp), LOCK_SH|LOCK_NB) == 0) {
    (void) fclose(fp);  /* unlocks as well */
    printf("\tno daemon to abort\n");
    return;
  }
#else
  debugf ("cmds: flock NOT done (using Locate)") ;
  if ((!getline(fp)) || (Locate (NULL, line) == NULL)) {
    (void) fclose(fp);  /* unlocks as well */
    printf("\tno daemon to abort\n");
    return;
  }
#endif
  
  (void) fclose(fp);
#ifndef __HELIOS
  if (kill(pid = atoi(line), SIGTERM) < 0)
    printf("\tWarning: daemon (pid %d) not killed\n", pid);
  else
    printf("\tdaemon (pid %d) killed\n", pid);
#else
  debugf ("cmds: lpd name = %s", line) ;
#ifdef OLDCODE
  if (kill_task (line, SIGTERM) < 0)
#else
  if (kill_task (line) < 0)
#endif  
    printf("\tWarning: daemon (%s) not killed\n", line);
  else
    printf("\tdaemon (%s) killed\n", line);
#endif  

#ifdef OLDCODE
  {  
    Stream *strm ;
    Object *obj ;
    if ((obj = Locate (NULL, line)) == NULL)
      debugf ("cmds: Locate failed : errno = %d", errno) ;
    debugf ("cmds: Trying to kill kill daemon (verify !!)") ;
    if ((strm = Open (obj, line, O_ReadWrite)) == NULL)
      debugf ("cmds: Open failed : errno = %d", errno) ;
    if (SendSignal (strm, SIGTERM) < 0)
    {
      debugf ("cmds: SendSignal : errno = %d", errno) ;
      printf("\tWarning: daemon (%s) not killed\n", line);
    }
    else
      printf("\tdaemon (%s) killed\n", line);
  }
#endif  
}

/*
 * Write a message into the status file.
 */
void upstat(char *msg)
{
  register int fd;

  bp = pbuf;
  if ((ST = pgetstr("st", &bp)) == NULL)
    ST = DEFSTAT;
  (void) sprintf(line, "%s/%s", SD, ST);
  umask(0);

#ifndef __HELIOS
  fd = open(line, O_WRONLY|O_CREAT, 0664);
  if (fd < 0 || flock(fd, LOCK_EX) < 0) {
#else
  fd = open(line, O_WRONLY|O_CREAT|O_TRUNC, 0664);
  if (fd < 0 || f_lock(fd, LOCK_EX) < 0) {
#endif
    printf("\tcannot create status file\n");
    return;
  }

#ifndef __HELIOS
  (void) ftruncate(fd, 0);
#else
  debugf ("implicit ftruncate()") ;
#endif  

  if (msg == (char *)NULL)
    (void) write(fd, "\n", 1);
  else
    (void) write(fd, msg, strlen(msg));
  (void) close(fd);
#ifdef __HELIOS
  (void) f_lock(fd, LOCK_UN) ;
#endif  
}

/*
 * Remove all spool files and temporaries from the spooling area.
 */
void clean(int argc, char *argv[])
{
  register int c, status;
  register char *cp1, *cp2;
  char prbuf[100];
#ifdef __HELIOS
  void cleanpr(void);
#endif

  if (argc == 1) {
    printf("Usage: clean {all | printer ...}\n");
    return;
  }
  if (argc == 2 && !strcmp(argv[1], "all")) {
    printer = prbuf;
    while (getprent(line) > 0) {
      cp1 = prbuf;
      cp2 = line;
      while ((c = *cp2++) != 0 && c != '|' && c != ':')
        *cp1++ = c;
      *cp1 = '\0';
      cleanpr();
    }
    return;
  }
  while (--argc) {
    printer = *++argv;
    if ((status = pgetent(line, printer)) < 0) {
      printf("cannot open printer description file\n");
      continue;
    } else if (status == 0) {
      printf("unknown printer %s\n", printer);
      continue;
    }
    cleanpr();
  }
}

#ifndef __HELIOS
select(d)
struct direct *d;
#else
int select_ (struct direct *d)
#endif
{
  int c = d->d_name[0];

  if ((c == 't' || c == 'c' || c == 'd') && d->d_name[1] == 'f')
    return(1);
  return(0);
}

/*
 * Comparison routine for scandir. Sort by job number and machine, then
 * by `cf', `tf', or `df', then by the sequence letter A-Z, a-z.
 */
int sortq(struct direct **d1, struct direct **d2)
{
  int c1, c2;

  if ((c1 = strcmp((*d1)->d_name + 3, (*d2)->d_name + 3)) != 0)
    return(c1);
  c1 = (*d1)->d_name[0];
  c2 = (*d2)->d_name[0];
  if (c1 == c2)
    return((*d1)->d_name[2] - (*d2)->d_name[2]);
  if (c1 == 'c')
    return(-1);
  if (c1 == 'd' || c2 == 'c')
    return(1);
  return(-1);
}

/*
 * Remove incomplete jobs from spooling area.
 */
void cleanpr()
{
  register int i, n;
  register char *cp, *cp1, *lp;
  struct direct **queue;
  int nitems;
#ifdef __HELIOS
  void unlinkf(char *);
#endif

  bp = pbuf;
  if ((SD = pgetstr("sd", &bp)) == NULL)
    SD = DEFSPOOL;
  printf("%s:\n", printer);

  for (lp = line, cp = SD; (*lp++ = *cp++) != 0; )
    ;
  lp[-1] = '/';

#ifndef __HELIOS
  nitems = scandir(SD, &queue, select, sortq);
#else
  debugf ("cmds: scan_dir()") ;
  nitems = scan_dir(SD, &queue, &select_, &sortq);
#endif
  
  if (nitems < 0) {
    printf("\tcannot examine spool directory\n");
/*
-- crf : free up memory (in case there was some Mallocing)
*/    
    debugf ("cmds: scan_freeing (1)") ;
    if (scan_free (queue) < 1)
      fatal ("Error freeing memory") ;

    return;
  }
  if (nitems == 0)
    return;
  i = 0;
  do {
    cp = queue[i]->d_name;
    if (*cp == 'c') {
      n = 0;
      while (i + 1 < nitems) {
        cp1 = queue[i + 1]->d_name;
        if (*cp1 != 'd' || strcmp(cp + 3, cp1 + 3))
          break;
        i++;
        n++;
      }
      if (n == 0) {
        strcpy(lp, cp);
        unlinkf(line);
      }
    } else {
      /*
       * Must be a df with no cf (otherwise, it would have
       * been skipped above) or a tf file (which can always
       * be removed).
       */
      strcpy(lp, cp);
      unlinkf(line);
    }
       } while (++i < nitems);
/*
-- crf : free up memory
*/    
    debugf ("cmds: scan_freeing (2)") ;
    if (scan_free (queue) < 1)
      fatal ("Error freeing memory") ;
}

void unlinkf(char *name)
{
  if (unlink(name) < 0)
    printf("\tcannot remove %s\n", name);
  else
    printf("\tremoved %s\n", name);
}

/*
 * Enable queuing to the printer (allow lpr's).
 */
void enable(int argc, char *argv[])
{
  register int c, status;
  register char *cp1, *cp2;
  char prbuf[100];
#ifdef __HELIOS
  void enablepr(void);
#endif

  if (argc == 1) {
    printf("Usage: enable {all | printer ...}\n");
    return;
  }
  if (argc == 2 && !strcmp(argv[1], "all")) {
    printer = prbuf;
    while (getprent(line) > 0) {
      cp1 = prbuf;
      cp2 = line;
      while ((c = *cp2++) != 0 && c != '|' && c != ':')
        *cp1++ = c;
      *cp1 = '\0';
      enablepr();
    }
    return;
  }
  while (--argc) {
    printer = *++argv;
    if ((status = pgetent(line, printer)) < 0) {
      printf("cannot open printer description file\n");
      continue;
    } else if (status == 0) {
      printf("unknown printer %s\n", printer);
      continue;
    }
    enablepr();
  }
}

void enablepr()
{
  struct stat stbuf;

  bp = pbuf;
  if ((SD = pgetstr("sd", &bp)) == NULL)
    SD = DEFSPOOL;
  if ((LO = pgetstr("lo", &bp)) == NULL)
    LO = DEFLOCK;
  (void) sprintf(line, "%s/%s", SD, LO);
  printf("%s:\n", printer);

  /*
   * Turn off the group execute bit of the lock file to enable queuing.
   */
#ifndef __HELIOS
  if (stat(line, &stbuf) >= 0) {
    if (chmod(line, stbuf.st_mode & 0767) < 0)
      printf("\tcannot enable queuing\n");
    else
      printf("\tqueuing enabled\n");
  }
#else
/*
-- crf: Note - if lock is not there, cannot enable
*/
  (void) sprintf(mode_path, "%s/%s", SD, DEFMODE);
  if (stat(line, &stbuf) >= 0) 
  {
    if ((lock_mode = get_mode (mode_path)) < 0)
    {
/*
-- crf : if lock_mode < 0, file doesn't exist. This is OK - it will be
-- created when the mode is set.
*/      
      debugf ("cmds: enable - can't get mode") ;
      lock_mode = ENABLE_MODE ;
    }
    if (set_mode (mode_path, lock_mode & 0767) < 0)
      printf("\tcannot enable queuing\n");
    else
      printf("\tqueuing enabled\n");
  }
#endif  
}

/*
 * Disable queuing.
 */
void disable (int argc, char *argv[])
{
  register int c, status;
  register char *cp1, *cp2;
  char prbuf[100];
#ifdef __HELIOS
  void disablepr(void);
#endif

  if (argc == 1) {
    printf("Usage: disable {all | printer ...}\n");
    return;
  }
  if (argc == 2 && !strcmp(argv[1], "all")) {
    printer = prbuf;
    while (getprent(line) > 0) {
      cp1 = prbuf;
      cp2 = line;
      while ((c = *cp2++) != 0 && c != '|' && c != ':')
        *cp1++ = c;
      *cp1 = '\0';
      disablepr();
    }
    return;
  }
  while (--argc) {
    printer = *++argv;
    if ((status = pgetent(line, printer)) < 0) {
      printf("cannot open printer description file\n");
      continue;
    } else if (status == 0) {
      printf("unknown printer %s\n", printer);
      continue;
    }
    disablepr();
  }
}

void disablepr()
{
  register int fd;
  struct stat stbuf;

  bp = pbuf;
  if ((SD = pgetstr("sd", &bp)) == NULL)
    SD = DEFSPOOL;
  if ((LO = pgetstr("lo", &bp)) == NULL)
    LO = DEFLOCK;
  (void) sprintf(line, "%s/%s", SD, LO);
  printf("%s:\n", printer);
  /*
   * Turn on the group execute bit of the lock file to disable queuing.
   */
#ifndef __HELIOS   
  if (stat(line, &stbuf) >= 0) {
    if (chmod(line, (stbuf.st_mode & 0777) | 010) < 0)
      printf("\tcannot disable queuing\n");
    else
      printf("\tqueuing disabled\n");
  } else if (errno == ENOENT) {
    if ((fd = open(line, O_WRONLY|O_CREAT, 0670)) < 0)
      printf("\tcannot create lock file\n");
    else {
      (void) close(fd);
      printf("\tqueuing disabled\n");
    }
#else
  (void) sprintf (mode_path, "%s/%s", SD, DEFMODE) ;
/*
-- crf : in this instance, if lockmode does not exist, it should be
-- created with queuing disabled ... so, test for this first
*/
  if (stat (mode_path, &stbuf) < 0)
  {
    debugf ("cmds: lockmode doesn't exist") ;
    if (set_mode (mode_path, 0670) < 0)
      debugf ("cmds: can't create lockmode") ;
  }
  if ((lock_mode = get_mode (mode_path)) < 0)
    debugf ("cmds: can't get mode") ;
    
  if (stat (line, &stbuf) >= 0) {
    if (set_mode (mode_path, (lock_mode & 0777) | 010) < 0)
      printf("\tcannot disable queuing\n");
    else
      printf("\tqueuing disabled\n");
  } else if (errno == ENOENT) {
    if ((fd = open(line, O_WRONLY|O_CREAT, 0670)) < 0)
      printf("\tcannot create lock file\n");
    else {
      (void) close(fd);
      printf("\tqueuing disabled\n");
    }
#endif    
    return;
  } else
    printf("\tcannot stat lock file\n");
}

/*
 * Disable queuing and printing and put a message into the status file
 * (reason for being down).
 */
void down(int argc, char *argv[])
{
  register int c, status;
  register char *cp1, *cp2;
  char prbuf[100];
#ifdef __HELIOS
  void putmsg(int, char **);
#endif

  if (argc == 1) {
    printf("Usage: down {all | printer} [message ...]\n");
    return;
  }
  if (!strcmp(argv[1], "all")) {
    printer = prbuf;
    while (getprent(line) > 0) {
      cp1 = prbuf;
      cp2 = line;
      while ((c = *cp2++) != 0 && c != '|' && c != ':')
        *cp1++ = c;
      *cp1 = '\0';
      putmsg(argc - 2, argv + 2);
    }
    return;
  }
  printer = argv[1];
  if ((status = pgetent(line, printer)) < 0) {
    printf("cannot open printer description file\n");
    return;
  } else if (status == 0) {
    printf("unknown printer %s\n", printer);
    return;
  }
  putmsg(argc - 2, argv + 2);
}

void putmsg(int argc, char **argv)
{
  register int fd;
  register char *cp1, *cp2;
  char buf[1024];
  struct stat stbuf;

  bp = pbuf;
  if ((SD = pgetstr("sd", &bp)) == NULL)
    SD = DEFSPOOL;
  if ((LO = pgetstr("lo", &bp)) == NULL)
    LO = DEFLOCK;
  if ((ST = pgetstr("st", &bp)) == NULL)
    ST = DEFSTAT;
  printf("%s:\n", printer);
  /*
   * Turn on the group execute bit of the lock file to disable queuing and
   * turn on the owner execute bit of the lock file to disable printing.
   */
  (void) sprintf(line, "%s/%s", SD, LO);
#ifndef __HELIOS
  if (stat(line, &stbuf) >= 0) {
    if (chmod(line, (stbuf.st_mode & 0777) | 0110) < 0)
      printf("\tcannot disable queuing\n");
    else
      printf("\tprinter and queuing disabled\n");
  } else if (errno == ENOENT) {
    if ((fd = open(line, O_WRONLY|O_CREAT, 0770)) < 0)
      printf("\tcannot create lock file\n");
    else {
      (void) close(fd);
      printf("\tprinter and queuing disabled\n");
    }
#else
  (void) sprintf (mode_path, "%s/%s", SD, DEFMODE) ;
/*
-- crf : in this instance, if lockmode does not exist, it should be
-- created with printing & queuing disabled ... so, test for this first
*/
  if (stat (mode_path, &stbuf) < 0)
  {
    debugf ("cmds: lockmode doesn't exist") ;
    if (set_mode (mode_path, 0770) < 0)
      debugf ("cmds: can't create lockmode") ;
  }
  if ((lock_mode = get_mode (mode_path)) < 0)
    debugf ("cmds: can't get mode") ;
    
  if (stat (line, &stbuf) >= 0) {
    if (set_mode (mode_path, (lock_mode & 0777) | 0110) < 0)
      printf("\tcannot disable printer and queuing\n");
    else
      printf("\tprinter and queuing disabled\n");
  } else if (errno == ENOENT) {
    if ((fd = open(line, O_WRONLY|O_CREAT, 0770)) < 0)
      printf("\tcannot create lock file\n");
    else {
      (void) close(fd);
      printf("\tprinter and queuing disabled\n");
    }
#endif    
    return;
  } else
    printf("\tcannot stat lock file\n");
  /*
   * Write the message into the status file.
   */
  (void) sprintf(line, "%s/%s", SD, ST);

#ifndef __HELIOS
  fd = open(line, O_WRONLY|O_CREAT, 0664);
  if (fd < 0 || flock(fd, LOCK_EX) < 0) {
    printf("\tcannot create status file\n");
    return;
  }
  
  (void) ftruncate(fd, 0);
  if (argc <= 0) {
    (void) write(fd, "\n", 1);
    (void) close(fd);
    return;
  }
  cp1 = buf;
  while (--argc >= 0) {
    cp2 = *argv++;
    while (*cp1++ = *cp2++)
      ;
    cp1[-1] = ' ';
  }
  cp1[-1] = '\n';
  *cp1 = '\0';
  (void) write(fd, buf, strlen(buf));
  (void) close(fd);

#elif defined OLDCODE
{
  FILE *stream ; 
  stream = fopen (line, "w") ;
/*
-- crf : using fopen to avoid use of ftruncate
*/  
#ifndef __HELIOS
  if (fd < 0 || flock(fd, LOCK_EX) < 0) {
    printf("\tcannot create status file\n");
    return;
  }
#endif  
  debugf ("cmds: flock NOT done (2)") ;
  if (stream == NULL) {
    printf("\tcannot create status file\n");
    return;
  }
  
  if (argc <= 0) {
    (void) fputs ("\n", stream) ;
    (void) fclose(stream);
    return;
  }
  cp1 = buf;
  while (--argc >= 0) {
    cp2 = *argv++;
    while (*cp1++ = *cp2++)
      ;
    cp1[-1] = ' ';
  }
  cp1[-1] = '\n';
  *cp1 = '\0';
  (void) fputs (buf, stream) ;
  (void) fclose(stream) ;
}
#else
#ifndef __HELIOS
  fd = open(line, O_WRONLY|O_CREAT, 0664);
  if (fd < 0 || flock(fd, LOCK_EX) < 0) {
#else
  fd = open(line, O_WRONLY|O_CREAT|O_TRUNC, 0664);
  if (fd < 0 || f_lock(fd, LOCK_EX) < 0) {
#endif  
    printf("\tcannot create status file\n");
    return;
  }

#ifndef __HELIOS
  (void) ftruncate(fd, 0);
#else
  debugf ("implicit ftruncate()") ;
#endif  
  if (argc <= 0) {
    (void) write(fd, "\n", 1);
    (void) close(fd);
#ifdef __HELIOS
    (void) f_lock(fd, LOCK_UN) ;
#endif    
    return;
  }
  cp1 = buf;
  while (--argc >= 0) {
    cp2 = *argv++;
    while ((*cp1++ = *cp2++) != NULL)
      ;
    cp1[-1] = ' ';
  }
  cp1[-1] = '\n';
  *cp1 = '\0';
  (void) write(fd, buf, strlen(buf));
  (void) close(fd);
#ifdef __HELIOS
  (void) f_lock(fd, LOCK_UN) ;
#endif    
#endif  
}

/*
 * Exit lpc
 */
void quit(int argc, char *argv[])
{
  exit(0);
}

/*
 * Kill and restart the daemon.
 */
void restart (int argc, char *argv[])
{
  register int c, status;
  register char *cp1, *cp2;
  char prbuf[100];

  if (argc == 1) {
    printf("Usage: restart {all | printer ...}\n");
    return;
  }
  if (argc == 2 && !strcmp(argv[1], "all")) {
    printer = prbuf;
    while (getprent(line) > 0) {
      cp1 = prbuf;
      cp2 = line;
      while ((c = *cp2++) != 0 && c != '|' && c != ':')
        *cp1++ = c;
      *cp1 = '\0';

/*
-- crf : suspected bug in original sources !!
-- This code is quite nasty. tbuf (in printcap.c) is a pointer to the
-- current line of the printcap file. OK. The call getprent(line) above
-- causes tbuf to point to line. Fine. However, abortpr() uses line for
-- a variety of purposes (i.e. to hold the name of the lock file and the
-- name of the previous daemon (extracted from the lock file)). Which
-- means that tbuf likewise points to this information. startpr() calls
-- pgetstr() to extract the name of the printer spool directory from
-- the current printcap entry (which it thinks is pointed at by tbuf). But
-- tbuf is now actually pointing at the name of the pevious printer daemon -
-- so it fails to get the name of the spool directory and sets it to the
-- default. So, I will save line before calling abortpr(), and then restore it
-- before calling startpr().
*/
#ifndef __HELIOS
      abortpr(0);
      startpr(0);
#else      
      {
        char temp [sizeof line] ;
        strcpy (temp, line) ;
        abortpr(0);
        strcpy (line, temp) ;
        startpr(0);
      }
#endif      
    }
    return;
  }
  while (--argc) {
    printer = *++argv;
    if ((status = pgetent(line, printer)) < 0) {
      printf("cannot open printer description file\n");
      continue;
    } else if (status == 0) {
      printf("unknown printer %s\n", printer);
      continue;
    }
#ifndef __HELIOS    
    abortpr(0);
    startpr(0);
#else      
    {
      char temp [sizeof line] ;
      strcpy (temp, line) ;
      abortpr(0);
      strcpy (line, temp) ;
      startpr(0);
    }
#endif      
  }
}

/*
 * Enable printing on the specified printer and startup the daemon.
 */
void start(int argc, char *argv[])
{
  register int c, status;
  register char *cp1, *cp2;
  char prbuf[100];

  if (argc == 1) {
    printf("Usage: start {all | printer ...}\n");
    return;
  }
  if (argc == 2 && !strcmp(argv[1], "all")) {
    printer = prbuf;
    while (getprent(line) > 0) {
      cp1 = prbuf;
      cp2 = line;
      while ((c = *cp2++) != 0 && c != '|' && c != ':')
        *cp1++ = c;
      *cp1 = '\0';
      startpr(1);
    }
    return;
  }
  while (--argc) {
    printer = *++argv;
    if ((status = pgetent(line, printer)) < 0) {
      printf("cannot open printer description file\n");
      continue;
    } else if (status == 0) {
      printf("unknown printer %s\n", printer);
      continue;
    }
    startpr(1);
  }
}

void startpr(int enable)
{
  struct stat stbuf;
  extern int startdaemon (char *) ;

  bp = pbuf;
  if ((SD = pgetstr("sd", &bp)) == NULL)
    SD = DEFSPOOL;
  if ((LO = pgetstr("lo", &bp)) == NULL)
    LO = DEFLOCK;
  (void) sprintf(line, "%s/%s", SD, LO);
  printf("%s:\n", printer);

  /*
   * Turn off the owner execute bit of the lock file to enable printing.
   */
#ifndef __HELIOS
  if (enable && stat(line, &stbuf) >= 0) {
    if (chmod(line, stbuf.st_mode & (enable==2 ? 0666 : 0677)) < 0)
      printf("\tcannot enable printing\n");
    else
      printf("\tprinting enabled\n");
  }
#else

  (void) sprintf (mode_path, "%s/%s", SD, DEFMODE);
  if (enable && stat(line, &stbuf) >= 0) 
  {
    if ((lock_mode = get_mode (mode_path)) < 0)
    {
/*
-- crf : if lock_mode < 0, file doesn't exist. This is OK - it will be
-- created when the mode is set.
*/      
      debugf ("cmds: enable : can't get mode") ;
      lock_mode = ENABLE_MODE ;
    }
    
    if (set_mode (mode_path, 
                  lock_mode & (enable==2 ? 0666 : 0677)) < 0)
      printf("\tcannot enable printing\n");
    else
      printf("\tprinting enabled\n");
  }
#endif  
  if (!startdaemon(printer))
    printf("\tcouldn't start daemon\n");
  else
    printf("\tdaemon started\n");
}

/*
 * Print the status of each queue listed or all the queues.
 */
void status(int argc, char *argv[])
{
  register int c, status;
  register char *cp1, *cp2;
  char prbuf[100];
#ifdef __HELIOS  
  void prstat(void);
#endif

  if (argc == 1) {
    printer = prbuf;
    while (getprent(line) > 0) {
      cp1 = prbuf;
      cp2 = line;
      while ((c = *cp2++) != 0 && c != '|' && c != ':')
        *cp1++ = c;
      *cp1 = '\0';
      prstat();
    }
    return;
  }
  while (--argc) {
    printer = *++argv;
    if ((status = pgetent(line, printer)) < 0) {
      printf("cannot open printer description file\n");
      continue;
    } else if (status == 0) {
      printf("unknown printer %s\n", printer);
      continue;
    }
    prstat();
  }
}

/*
 * Print the status of the printer queue.
 */
void prstat()
{
  struct stat stbuf;
  register int fd, i;
  register struct direct *dp;
  DIR *dirp;

  bp = pbuf;
  if ((SD = pgetstr("sd", &bp)) == NULL)
    SD = DEFSPOOL;
  if ((LO = pgetstr("lo", &bp)) == NULL)
    LO = DEFLOCK;
  if ((ST = pgetstr("st", &bp)) == NULL)
    ST = DEFSTAT;
  printf("%s:\n", printer);
  (void) sprintf(line, "%s/%s", SD, LO);
#ifndef __HELIOS
  if (stat(line, &stbuf) >= 0) {
    printf("\tqueuing is %s\n",
      (stbuf.st_mode & 010) ? "disabled" : "enabled");
    printf("\tprinting is %s\n",
      (stbuf.st_mode & 0100) ? "disabled" : "enabled");
#else
  (void) sprintf (mode_path, "%s/%s", SD, DEFMODE);
  if (stat(line, &stbuf) >= 0) 
  {
    if (stat (mode_path, &stbuf) < 0)
    {
      debugf ("cmds: lockmode doesn't exist") ;
      if (set_mode (mode_path, ENABLE_MODE) < 0)
        debugf ("cmds: can't set mode") ;
    }
    if ((lock_mode = get_mode (mode_path)) < 0)
      debugf ("cmds: can't get mode") ;
    else
    {
      printf("\tqueuing is %s\n",
        (lock_mode & 010) ? "disabled" : "enabled");
      printf("\tprinting is %s\n",
        (lock_mode & 0100) ? "disabled" : "enabled");
    }
#endif      
  } else {
    printf("\tqueuing is enabled\n");
    printf("\tprinting is enabled\n");
  }
  if ((dirp = opendir(SD)) == NULL) {
    printf("\tcannot examine spool directory\n");
    return;
  }
  i = 0;
  while ((dp = readdir(dirp)) != NULL) {
    if (*dp->d_name == 'c' && dp->d_name[1] == 'f')
      i++;
  }
  closedir(dirp);
  if (i == 0)
    printf("\tno entries\n");
  else if (i == 1)
    printf("\t1 entry in spool area\n");
  else
    printf("\t%d entries in spool area\n", i);
  fd = open(line, O_RDONLY);
#ifndef __HELIOS
  if (fd < 0 || flock(fd, LOCK_SH|LOCK_NB) == 0) {
    (void) close(fd);  /* unlocks as well */
    printf("\tno daemon present\n");
    return;
  }
#else
  debugf ("cmds: flock NOT done (using Locate)") ;
  if ((fd < 0) || (Locate (NULL, line) == NULL)) {
    (void) close(fd);  /* unlocks as well */
    printf("\tno daemon present\n");
    return;
  }
#endif  

  (void) close(fd);
  putchar('\t');
  (void) sprintf(line, "%s/%s", SD, ST);
  fd = open(line, O_RDONLY);
  if (fd >= 0) {
#ifndef __HELIOS
    (void) flock(fd, LOCK_SH);
#else
    (void) f_lock(fd, LOCK_SH);
#endif    
    while ((i = read(fd, line, sizeof(line))) > 0)
      (void) fwrite(line, 1, i, stdout);
    (void) close(fd);  /* unlocks as well */
#ifdef __HELIOS
    (void) f_lock(fd, LOCK_UN) ;
#endif    
  }
#ifdef __HELIOS
/*
-- crf : if no status file, print cr
*/  
  else
  {
    (void) fwrite("\r", 1, 1, stdout);
  }
#endif  
}

/*
 * Stop the specified daemon after completing the current job and disable
 * printing.
 */
void stop(int argc, char *argv[])
{
  register int c, status;
  register char *cp1, *cp2;
  char prbuf[100];
#ifdef __HELIOS
  void stoppr(void);
#endif

  if (argc == 1) {
    printf("Usage: stop {all | printer ...}\n");
    return;
  }
  if (argc == 2 && !strcmp(argv[1], "all")) {
    printer = prbuf;
    while (getprent(line) > 0) {
      cp1 = prbuf;
      cp2 = line;
      while ((c = *cp2++) != 0 && c != '|' && c != ':')
        *cp1++ = c;
      *cp1 = '\0';
      stoppr();
    }
    return;
  }
  while (--argc) {
    printer = *++argv;
    if ((status = pgetent(line, printer)) < 0) {
      printf("cannot open printer description file\n");
      continue;
    } else if (status == 0) {
      printf("unknown printer %s\n", printer);
      continue;
    }
    stoppr();
  }
}

void stoppr()
{
  register int fd;
  struct stat stbuf;

  bp = pbuf;
  if ((SD = pgetstr("sd", &bp)) == NULL)
    SD = DEFSPOOL;
  if ((LO = pgetstr("lo", &bp)) == NULL)
    LO = DEFLOCK;
  (void) sprintf(line, "%s/%s", SD, LO);
  printf("%s:\n", printer);

  /*
   * Turn on the owner execute bit of the lock file to disable printing.
   */
#ifndef __HELIOS
  if (stat(line, &stbuf) >= 0) {
    if (chmod(line, (stbuf.st_mode & 0777) | 0100) < 0)
      printf("\tcannot disable printing\n");
    else {
      upstat("printing disabled\n");
      printf("\tprinting disabled\n");
    }
  } else if (errno == ENOENT) {
    if ((fd = open(line, O_WRONLY|O_CREAT, 0760)) < 0)
      printf("\tcannot create lock file\n");
    else {
      (void) close(fd);
      upstat("printing disabled\n");
      printf("\tprinting disabled\n");
    }
#else

  (void) sprintf (mode_path, "%s/%s", SD, DEFMODE) ;
/*
-- crf : in this instance, if lockmode does not exist, it should be
-- created with printing disabled ... so, test for this first
*/
  if (stat (mode_path, &stbuf) < 0)
  {
    debugf ("cmds: lockmode doesn't exist") ;
    if (set_mode (mode_path, 0760) < 0)
      debugf ("cmds: can't create lockmode") ;
  }
  if ((lock_mode = get_mode (mode_path)) < 0)
    debugf ("cmds: can't get mode") ;
    
  if (stat (line, &stbuf) >= 0) {
    if (set_mode (mode_path, (lock_mode & 0777) | 0100) < 0)
      printf("\tcannot disable printing\n");
    else {
      upstat("printing disabled\n");
      printf("\tprinting disabled\n");
    }
  } else if (errno == ENOENT) {
    if ((fd = open(line, O_WRONLY|O_CREAT, 0760)) < 0)
      printf("\tcannot create lock file\n");
    else {
      (void) close(fd);
      upstat("printing disabled\n");
      printf("\tprinting disabled\n");
    }
#endif
  } else
    printf("\tcannot stat lock file\n");
}

struct  queue **queue;
int  nitems;
time_t  mtime;

/*
 * Put the specified jobs at the top of printer queue.
 */
void topq (int argc, char *argv[])
{
#ifndef __HELIOS
  register int n, i;
  register char *cfname;
#else
  register int i;
#endif  
  struct stat stbuf;
  int status, changed;
#ifdef __HELIOS
  int doarg(char *) ;
#endif

  if (argc < 3) {
    printf("Usage: topq printer [jobnum ...] [user ...]\n");
    return;
  }

  --argc;
  printer = *++argv;
  status = pgetent(line, printer);
  if (status < 0) {
    printf("cannot open printer description file\n");
    return;
  } else if (status == 0) {
    printf("%s: unknown printer\n", printer);
    return;
  }
  bp = pbuf;
  if ((SD = pgetstr("sd", &bp)) == NULL)
    SD = DEFSPOOL;
  if ((LO = pgetstr("lo", &bp)) == NULL)
    LO = DEFLOCK;
  printf("%s:\n", printer);

  if (chdir(SD) < 0) {
    printf("\tcannot chdir to %s\n", SD);
    return;
  }
  nitems = getq(&queue);
  if (nitems == 0)
    return;
  changed = 0;
  mtime = queue[0]->q_time;
  for (i = argc; --i; ) {
    if (doarg(argv[i]) == 0) {
      printf("\tjob %s is not in the queue\n", argv[i]);
      continue;
    } else
      changed++;
  }
  for (i = 0; i < nitems; i++)
    free(queue[i]);
  free(queue);
  if (!changed) {
    printf("\tqueue order unchanged\n");
    return;
  }
  /*
   * Turn on the public execute bit of the lock file to
   * get lpd to rebuild the queue after the current job.
   */
#ifndef __HELIOS
  if (changed && stat(LO, &stbuf) >= 0)
    (void) chmod(LO, (stbuf.st_mode & 0777) | 01);
#else

  if (changed && stat(LO, &stbuf) >= 0)
  {
/*
-- crf : in this instance, if lockmode does not exist, it should be
-- created with public execute bit on ... I've chosen to also enable
-- printing and queuing in the creation of the file ...
*/
    if (stat (DEFMODE, &stbuf) < 0)
    {
      debugf ("cmds: lockmode doesn't exist") ;
      if (set_mode (DEFMODE, 0661) < 0)
        debugf ("cmds: can't create") ;
    }
    if ((lock_mode = get_mode (DEFMODE)) < 0)
      debugf ("cmds: can't get mode") ;

    if (set_mode (DEFMODE, (lock_mode & 0777) | 01) < 0)
      debugf ("cmds: can't set mode") ;
  }
#endif    
} 

/*
 * Reposition the job by changing the modification time of
 * the control file.
 */
int touch(struct queue *q)
{
  struct timeval tvp[2];

  tvp[0].tv_sec = tvp[1].tv_sec = --mtime;
  tvp[0].tv_usec = tvp[1].tv_usec = 0;
  return(utimes(q->q_name, tvp));
}

/*
 * Checks if specified job name is in the printer's queue.
 * Returns:  negative (-1) if argument name is not in the queue.
 */
int doarg(char *job)
{
  register struct queue **qq;
  register int jobnum, n;
  register char *cp, *machine;
  int cnt = 0;
  FILE *fp;

  /*
   * Look for a job item consisting of system name, colon, number 
   * (example: ucbarpa:114)  
   */
  if ((cp = index(job, ':')) != NULL) {
    machine = job;
    *cp++ = '\0';
    job = cp;
  } else
    machine = NULL;

  /*
   * Check for job specified by number (example: 112 or 235ucbarpa).
   */
  if (isdigit(*job)) {
    jobnum = 0;
    do
      jobnum = jobnum * 10 + (*job++ - '0');
    while (isdigit(*job));
    for (qq = queue + nitems; --qq >= queue; ) {
      n = 0;
      for (cp = (*qq)->q_name+3; isdigit(*cp); )
        n = n * 10 + (*cp++ - '0');
      if (jobnum != n)
        continue;
      if (*job && strcmp(job, cp) != 0)
        continue;
      if (machine != NULL && strcmp(machine, cp) != 0)
        continue;
      if (touch(*qq) == 0) {
        printf("\tmoved %s\n", (*qq)->q_name);
        cnt++;
      }
    }
    return(cnt);
  }
  /*
   * Process item consisting of owner's name (example: henry).
   */
  for (qq = queue + nitems; --qq >= queue; ) {
    if ((fp = fopen((*qq)->q_name, "r")) == NULL)
      continue;
    while (getline(fp) > 0)
      if (line[0] == 'P')
        break;
    (void) fclose(fp);
    if (line[0] != 'P' || strcmp(job, line+1) != 0)
      continue;
    if (touch(*qq) == 0) {
      printf("\tmoved %s\n", (*qq)->q_name);
      cnt++;
    }
  }
  return(cnt);
}

/*
 * Enable everything and start printer (undo `down').
 */
void up (int argc, char *argv[])
{
  register int c, status;
  register char *cp1, *cp2;
  char prbuf[100];

  if (argc == 1) {
    printf("Usage: up {all | printer ...}\n");
    return;
  }
  if (argc == 2 && !strcmp(argv[1], "all")) {
    printer = prbuf;
    while (getprent(line) > 0) {
      cp1 = prbuf;
      cp2 = line;
      while ((c = *cp2++) != 0 && c != '|' && c != ':')
        *cp1++ = c;
      *cp1 = '\0';
      startpr(2);
    }
    return;
  }
  while (--argc) {
    printer = *++argv;
    if ((status = pgetent(line, printer)) < 0) {
      printf("cannot open printer description file\n");
      continue;
    } else if (status == 0) {
      printf("unknown printer %s\n", printer);
      continue;
    }
    startpr(2);
  }
}
