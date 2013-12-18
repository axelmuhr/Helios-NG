/*
-- tinybiff is a *highly* simplified biff daemon, intended only for use on 
-- single-user systems. The program simply waits for notification from 
-- /helios/bin/mail that mail has arrived, and then writes the name of the 
-- recipient and the time to the controlling console.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <syslib.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <signal.h>

extern int openlog (char *, int, int) ;
extern int syslog (int, char *, ... ) ;
void fatal (char *) ;
void int_sig(void) ;

typedef struct sockaddr SOCK_ADDR ;
Environ *env ;
int s = -1 ;

int main (void)
{
  struct servent *sp ;
  struct sockaddr_in addr ;
  int addr_len = sizeof (addr) ;
  void inform_user (char *) ;

  env = getenviron () ;
  openlog ("tinybiff", LOG_PID, LOG_MAIL) ;
  syslog (LOG_INFO, "(%s) starting", env->Objv [OV_Task]->Name) ;

  {
    struct sigaction act ;
    act.sa_handler = int_sig ;
    act.sa_mask = 0 ;
    act.sa_flags = SA_ASYNC ;
    (void) sigaction(SIGINT, &act, NULL) ;
    (void) sigaction(SIGHUP, &act, NULL) ;
    (void) sigaction(SIGQUIT, &act, NULL) ;
    (void) sigaction(SIGTERM, &act, NULL) ;
  }

  if ((s = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    fatal ("socket()") ;

  if ((sp = getservbyname ("biff", "udp")) == NULL) 
    fatal ("getservbyname()") ;

  memset (&addr, 0, addr_len) ;
  addr.sin_family = AF_INET ; 
  addr.sin_port = sp->s_port ;
  addr.sin_addr.s_addr = INADDR_ANY ;

/* 
-- bind socket to port 
*/
  if (bind (s, (SOCK_ADDR *) &addr, addr_len) < 0) 
    fatal ("bind()") ;

  forever
  {    
    char buf [1024] ;
    char *end_of_name ;
    int nfds ; 
    int readfds = 1 << s ;
    nfds = select (s + 1, &readfds, 0, 0, 0);
    if (nfds <= 0) 
    {
      if (nfds < 0 && errno != EINTR)
        syslog(LOG_WARNING, "select: %m");
      continue;
    }

/* 
-- get a message from socket 
*/
    if ((recvfrom (s, buf, sizeof (buf), 0, (SOCK_ADDR *) &addr, 
         &addr_len)) < 0)
      fatal ("recvfrom()") ;

/*
-- Format of message from /helios/bin/mail: name@len, where
-- name = name of recipient
-- len  = length of message
-- Text of newly arrived message is therefore accessible by reading the first
-- len bytes of the file /helios/local/spool/name. This is not done here ... 
-- instead, a simple notification message is generated
*/  
    if ((end_of_name = strchr (buf, '@')) == NULL)
      fatal ("Bad message format") ;
    *end_of_name = '\0' ;
    inform_user (buf) ;
  }
}

void inform_user (char *name)
{
  time_t timer ;
  struct tm *cal ;
  char msg [256] ;
  Stream *str ;
  Object *window = env->Objv [OV_Console] ;
/*
-- alternatively, can use ctermid() to get pathname for controlling terminal
*/
  time (&timer) ;
  cal = localtime (&timer) ;
  sprintf (msg, "\n%s %s - %s", "Mail has arrived for", name, asctime (cal)) ;
  if ((str = Open (window, (char *) NULL, O_ReadWrite)) == (Stream *) NULL)
    fatal ("Open()") ;
  (void) Write (str, msg, strlen (msg), -1) ;
  (void) Close (str) ;
}

void fatal (char *msg)
{
  syslog (LOG_ERR, "Error: %s - %m", msg) ;
  exit (1) ;
}

void int_sig()
{
  (void) close (s) ;
  syslog (LOG_INFO, "terminated") ;
  exit(0);
}
