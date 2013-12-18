/*
 --  -------------------------------------------------------------------------
 --
 --      ISERVER  -  INMOS standard file server
 --
 --      udplink.c
 --
 --      Link module for SubSystems connected via the UDP-LINK series of
 --      products.
 --
 --      Copyright (c) INMOS Limited., 1989, 1990
 --
 --   ------------------------------------------------------------------------
*/

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <assert.h>

#ifdef WIN_TCP

#include <psldef.h>
#include <time.h>
#include <ssdef.h>
#include <descrip.h>
#include <ssdef.h>
#include <winclude/netdb.h>
#include <vms/iodef.h>
#else

#include <sys/time.h>
#include <netdb.h>

#endif                          /* WIN and VMS */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "inmos.h"
#include "iserver.h"
#include "udplink.h"

#define NULL_LINK -1
#define MAXLINE (255)
#define MAX_SEND (1400)

EXTERN int TheLink;
EXTERN BOOL VerboseSwitch;
EXTERN BOOL CocoPops;
EXTERN int errno;

#ifdef WIN_TCP
EXTERN int uerrno;
#define ERRORCODE uerrno
#else
#define ERRORCODE errno
#endif

PRIVATE LINK ActiveLink = NULL_LINK;
PRIVATE BYTE DefaultDevice[64] = "TRANSNET";
PRIVATE unsigned char h_seq = 0;/* host (vax) sequence no */
PRIVATE unsigned char t_seq = 0;/* transputer sequence no. */
PRIVATE struct data_dgram inp;
PRIVATE char *saved_buf = NULL;
PRIVATE int saved_len = 0;
PRIVATE int Result = SUCCEEDED;         /* link error code status */

unsigned char prot_send();
unsigned char peek_send();
int timed_read();
int data_send();
int data_read();

/* OpenLink
	- opens a link to the transputer (i.e the server)

	effects  : returns either
			ER_LINK_BAD     (link already open)
			ER_LINK_CANT    (transputer doesn't respond)
			ER_LINK_SOFT    (link software failure)
		   or a LINK value      (success)
*/
PUBLIC LINK
OpenLink (Name)
BYTE *Name;
{
  struct sockaddr_in sin, rsin;
  struct hostent *hp, *gethostbyname ();
  int s, i, param_count;
  int xa,xb,xc,xd;
  char buf[1024];
  union {
    unsigned long l_addr;
    unsigned char addr[4];
  } address;

  if (ActiveLink != NULL_LINK) {
    return (ER_LINK_BAD);
  };

  if ((*Name == 0) || (Name == NULL)) {
    strcpy (Name, DefaultDevice);
  };

  s = socket (AF_INET, SOCK_DGRAM, 0);

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl (INADDR_ANY);
  sin.sin_port = htons (0);
  if (bind (s, (struct sockaddr *) & sin, (short int) sizeof (sin)) < 0) {
    DEBUG (( "cant bind, err=%d\n", ERRORCODE ));
    return (ER_LINK_CANT);
  };

#ifdef WIN_TCP
  if ((hp = gethostbyname (Name)) == 0) {
#else
  if ((hp = gethostbyname (Name)) == NULL) {
#endif
    param_count = sscanf(Name, "%d.%d.%d.%d", &xa, &xb, &xc, &xd);
    address.addr[0] = (unsigned char) xa;             
    address.addr[1] = (unsigned char) xb;
    address.addr[2] = (unsigned char) xc;
    address.addr[3] = (unsigned char) xd;
    if (param_count == 4) {
      rsin.sin_addr.s_addr = address.l_addr;
    } else {
      DEBUG(( "bad internet address\n" ));
      return (ER_LINK_BAD);
    };
  } else {               /* Name is a textual name */
    bcopy ((char *) hp->h_addr, (char *) &rsin.sin_addr, hp->h_length);
  };
  rsin.sin_family = AF_INET;
  rsin.sin_port = htons (HOSTLINK_PORT);

  DEBUG (( "remote  addr is %s\n", inet_ntoa (rsin.sin_addr) ));

  if (connect (s, (struct sockaddr *) & rsin, sizeof (rsin)) < 0) {
    DEBUG (( "cant connect, err = %d\n", ERRORCODE ));
    return (ER_LINK_CANT);
  };                                         

  /* start sequence numbers as zero */
  h_seq = 0;
  t_seq = 0;

  /* send a SYN frame, receiving an ACK indicates success */
  if (prot_send (s, rp_SYN, h_seq, 2, TRANS_TIMEOUT) != rp_ACK) {
    close (s);
    h_seq = (h_seq + 1) & 0x007f;
    return (ER_LINK_CANT);
  } else {
    ActiveLink = s;
    h_seq = (h_seq + 1) & 0x007f;
    return (ActiveLink);
  };
}

/* CloseLink
	- closes the link between the transputer & the host

	effects  : returns either
			ER_LINK_BAD     (link wasn't open)
			ER_LINK_CANT    (transputer doesn't respond)
			ER_LINK_SOFT    (link software failure)
			SUCCEED         (success)
*/
PUBLIC int CloseLink (LinkId)
LINK LinkId;
{
  if (LinkId != ActiveLink) {
    return (ER_LINK_BAD);
  };      
  
  /* terminate the connection to the transputer */
  if (prot_send (ActiveLink, rp_RST, h_seq, 2, TRANS_TIMEOUT) != rp_ACK) {
    return (Result);
  } else {
    close (ActiveLink);
    ActiveLink = NULL_LINK;
    return (SUCCEEDED);     /* return success value */
  };
}

/* ReadLink
	- tries to read Count bytes from the link!

	effects : tries to read rp_MAX_DATA_SIZE bytes from the link,
		  returns either
			ER_LINK_BAD     (link not open)
			ER_LINK_CANT    (transputer doesn't respond)
			ER_LINK_SOFT    (link software failure)
			the number of bytes (up to the amount requested
			 in "Count") successfully read
*/
PUBLIC int ReadLink (LinkId, Buffer, Count, Timeout)
LINK LinkId;
char *Buffer;
unsigned int Count;
int Timeout;
{
  int got;

  if (LinkId != ActiveLink) {
    return (ER_LINK_BAD);
  };
	
  if (Count < 1) {
    return (ER_LINK_CANT);
  };

  if (Count > rp_MAX_DATA_SIZE) {       /* restrict size of read to maximum */
    Count = rp_MAX_DATA_SIZE;
  };

  if (saved_len <= 0) {                 /* no buffered data, so read some more data */
    saved_len = data_read (ActiveLink, &saved_buf, rp_MAX_DATA_SIZE,
			   Count, (short int) Timeout);
    if (saved_len < 0) {
      DEBUG (( "ReadLink - returning %d\n", got ));
      return (saved_len);
    };
  };
  if (saved_len >= Count) {             /* if read more than wanted, just return wanted */
    got = Count;
  } else {
    got = saved_len;                    /* if read less than wanted, return read */
  };
  memcpy (Buffer, saved_buf, got);
  saved_len = saved_len - got;
  saved_buf = saved_buf + got;
  DEBUG (( "ReadLink - returning %d\n", got ));
  return (got);
}

/* WriteLink
	- tries to write data to the link!

	effects  : tries to write up to "Count" bytes to the link.
		   returns either
			ER_LINK_BAD     (link wasn't open)
			ER_LINK_CANT    (transputer doesn't respond)
			ER_LINK_SOFT    (link software failure)
		   or number of bytes sucessfully sent
*/
PUBLIC int WriteLink (LinkId, Buffer, Count, Timeout)
LINK LinkId;
char *Buffer;
short int Count;
int Timeout;
{
  short int i, rem, status;

  rem = Count;

  if (LinkId != ActiveLink) {
    return (ER_LINK_BAD);
  };
  
  if (rem < 1) {
    return (ER_LINK_CANT);
  };

  while (rem > 0) {
    i = (rem > rp_MAX_DATA_SIZE) ? rp_MAX_DATA_SIZE : rem;
    status = data_send (ActiveLink, rp_DATA, Buffer, i);
    if (!status) {
      return (Result);
    };
    Buffer += i;
    rem -= i;
  }
  return (Count);
}

/* ResetLink
	- reset the transputer's subsystem

	effects  : returns either
			ER_LINK_BAD     (link wasn't open)
			ER_LINK_CANT    (transputer doesn't respond)
			ER_LINK_SOFT    (link software failure)
			SUCCEDDED
*/
PUBLIC int ResetLink (LinkId)
LINK LinkId;
{
  int code;
  
  if (LinkId != ActiveLink) {
    return (ER_LINK_BAD);
  };

  /* reset the transputer's subsystem */
  code = prot_send (ActiveLink, rp_RESET, h_seq, 0, TRANS_TIMEOUT);
  if (code == rp_NACK) {
    return (ER_LINK_BAD);
  };
  if (code != rp_ACK) {
    return (Result);     
  } else {
    h_seq = (h_seq + 1) & 0X007f;
    return (SUCCEEDED);     /* return success value */
  };
}

/* AnalyseLink
	effects  : sets analyse mode on the transputer
		   returns either
			ER_LINK_BAD     (link wasn't open)
			ER_LINK_CANT    (transputer doesn't respond)
			ER_LINK_SOFT    (link software failure)
			SUCCEEDED
*/
PUBLIC int AnalyseLink (LinkId)
LINK LinkId;
{
  int code;
  
  if (LinkId != ActiveLink) {
    return (ER_LINK_BAD);
  };

  code = prot_send (ActiveLink, rp_ANALYSE, h_seq, 0, TRANS_TIMEOUT);
  if (code == rp_NACK) {
    return (ER_LINK_BAD);
  };
  if (code != rp_ACK) {
    return (Result);
  } else {
    h_seq = (h_seq + 1) & 0x007f;
    return (SUCCEEDED);     /* return success value */
  };
}

/* PeekLink
	- tries to peek 'words' words of data from the transputer

	modifies : Result
	effects  : returns either
			ER_LINK_BAD     (link wasn't open)
			ER_LINK_CANT    (transputer doesn't respond)
			ER_LINK_SOFT    (link software failure)
			0               (no data could be peeked in the available time)
		   or number of bytes successfully peeked
*/              
PUBLIC int PeekLink (LinkId, Buffer, address, words)
LINK LinkId;
char *Buffer;
u_long address;
short int words;
{
  struct prot_dgram resp;
  int code, tries, data_ok, bytes_returned;                         
  unsigned char next_t_seq, next_h_seq;

  if (LinkId != ActiveLink) {
    return (ER_LINK_BAD);
  };
  
  next_t_seq = (t_seq + 1) &0x007f;
  next_h_seq = (h_seq + 1) &0x007f;

  for (tries = 1; tries < MAX_RETRIES; tries++) {
    code = peek_send (ActiveLink, h_seq, address, words);
    if (code == rp_NACK) {
      h_seq = next_h_seq;
      return (ER_LINK_BAD);
    };
    if (code != rp_ACK) {
      h_seq = next_h_seq;
      return (Result);
    } else {
      data_ok = timed_read (ActiveLink, (char *) &inp, sizeof(inp),
			    timeout_period * tries);
      if (data_ok >= 2 && (inp.op == rp_DATA && inp.seq == t_seq)) {
	h_seq = next_h_seq;
	t_seq = next_t_seq;
	resp.op = rp_ACK;
	resp.seq = inp.seq;
	resp.len = 0;
	if (send (ActiveLink, (char *) &resp, sizeof(resp), 0) < 0) {
	  DEBUG (("*fatal error : PeekLink[1] - send failed=%d\n", ERRORCODE ));
	  return (ER_LINK_SOFT); 
	}
	bytes_returned = ntohs(inp.len);
	DEBUG(( "PeekLink :returning %d\n",bytes_returned ));
	memcpy (Buffer, inp.data, bytes_returned);
	return (bytes_returned);
      } else {                                                          
	if (data_ok >= 2 && inp.op != rp_ACK && inp.op != rp_NACK &&
	    inp.op != rp_CONT && inp.op != rp_ERROR) {
	  resp.op = rp_ACK;
	  resp.seq = inp.seq;
	  resp.len = 0;
	  if (send (ActiveLink, (char *) &resp, sizeof (resp), 0) < 0) {  
	    DEBUG (("*fatal error : PeekLink[2] - send failed=%d\n", ERRORCODE ));
	    return (ER_LINK_SOFT);
	  };
	};
      };                                     
    };
  };
  t_seq = next_t_seq;
  h_seq = next_h_seq;
  return (ER_LINK_CANT);
};

/* TestError
	- tests the error state of the transputer

	modifies : h_seq (incremented)
	effects  : tests the error state of the transputer
		   returns either
			ER_LINK_BAD     (link wasn't open)
			ER_LINK_CANT    (transputer doesn't respond)
			ER_LINK_SOFT    (link software failure)
			0               (error flag not set)
			1               (error flag set)
*/
PUBLIC int TestError (LinkId)
LINK LinkId;
{
  struct prot_dgram reply;
  struct error_dgram resp, error_inq;
  int tries, reply_ok, timed_out;

  if (LinkId != ActiveLink) {
    return (ER_LINK_BAD);
  };
  
  error_inq.op = rp_ERROR;
  error_inq.seq = h_seq;

  for (tries = 1; tries < MAX_RETRIES; tries++) {
    if (send (ActiveLink, (char *) &error_inq, sizeof (error_inq), 0) < 0) {
      DEBUG (("*fatal error : TestError[1] - send failed =%d\n", ERRORCODE ));
      return (ER_LINK_SOFT);
    } else {
      reply_ok = (timed_read (ActiveLink, (char *) &resp, sizeof (resp),
		  timeout_period * tries) > 0);
      if (reply_ok > 0) {
	if (resp.op == rp_NACK) {
	  return (ER_LINK_BAD);
	};
	if (resp.op == rp_ERROR) {
	  /* reply obtained, so return error status of transputer */
	  h_seq = (h_seq + 1) & 0x007f;
	  return (int) (resp.status);
	} else {
	  if (resp.op != rp_ACK && resp.op != rp_NACK && resp.op != rp_CONT &&
	      resp.op != rp_ERROR) {
	    reply.op = rp_ACK;
	    reply.seq = resp.seq;
	    reply.len = htons ((short int) 0);
	    reply.timeout = htons ((short int) 0);
	    if (send (ActiveLink, (char *) &reply, sizeof (reply), 0) < 0) {
	      DEBUG (("*fatal error : TestError[2] - send failed =%d\n", ERRORCODE ));
	      return (ER_LINK_SOFT); 
	    };
	  };
	};
      };
    };
  };
  /* failed */
  h_seq = (h_seq + 1) &0x007f;
  return (ER_LINK_CANT);
};

/* TestRead
	- test the read operation
	effects : returns either
			ER_LINK_BAD     (link wasn't open)
			0               (link might not be able to read)
*/
PUBLIC int TestRead (LinkId)
LINK LinkId;
{
  if (LinkId != ActiveLink) {
    return (ER_LINK_BAD);
  } else {
    return (0);
  };
}

/* TestWrite
	- test the write operation
	effects : returns either
			ER_LINK_BAD     (link wasn't open)
			0               (link might not be able to write)
*/
PUBLIC int TestWrite (LinkId)
LINK LinkId;
{
  if (LinkId != ActiveLink) {
    return (ER_LINK_BAD);
  } else {
    return (0);
  };
}

/* TestLink
	- tests the link is operational

	effects  : tests the link status. returns either
			ER_LINK_BAD     (link wasn't open)
			ER_LINK_CANT    (link communication failed)
			SUCCEEDED       (link is ok)
*/
PUBLIC TestLink (LinkId)
LINK LinkId;
{
  int status;
  
  if (LinkId != ActiveLink) {
    return (ER_LINK_BAD);
  };
  
  status = prot_send (ActiveLink, rp_ACTIVE, h_seq, 0, TRANS_TIMEOUT);
  if (status == rp_NACK) {
    return (ER_LINK_BAD);
  };
  if (status == rp_TIMEOUT) {
    return (Result);
  } else {
    h_seq = (h_seq + 1) &0x007f;
    return (SUCCEEDED);
  };
};

/************************ INTERNAL FUNCTIONS ******************************/

/* timeout functions */

#ifdef WIN_TCP
/*
 * Timeout functions under WIN TCP
 * Broken select(2) call
 *
 */


/* timed_read
 *
 *   effects  : returns the number of bytes successfully read
 *              or zero if timeout after time in 'ms' milliseconds
 */

#define time_event (unsigned long int) 1  /* Event flags and masks */
#define read_event (unsigned long int) 2
#define read_mask  (unsigned long int) 4
#define event_mask (unsigned long int) 6

#define IO$_RECEIVE IO$_READVBLK  /* See WIN/TCP reference manual */

PRIVATE int timed_read (socket, buf, buf_size, ms)
LINK socket;
char *buf;
int buf_size;
long ms;
{
  int bytes_read = 0;
  unsigned long int flags;
  int status;               /* status is consistantly ignored */
  int vms_delta_time[2];    /* To store our backdoor version of VMS time */
  int read_iosb[2];         /* i/o status block */

  vms_delta_time[1] = -1;
  vms_delta_time[0] = ms * -10000; /* convert ms to VMS 100 nS intervals */

  /* Queue read */
  status = SYS$QIO(read_event, socket, IO$_RECEIVE, read_iosb, 0, 0,
		   buf, buf_size, 0, 0, 0, 0);
  if (status != SS$_NORMAL) {
    DEBUG (("*fatal error : timed_read - Abnormal QIO status %d\n", status ));
    return (ER_LINK_SOFT);
  };
	
  /* Set up timer */
  status = SYS$SETIMR(time_event, &vms_delta_time, 0, 0);
  if (status != SS$_NORMAL) {
    DEBUG (("*fatal error : timed_read - Abnormal timer status %d\n", status ));
    return (ER_LINK_SOFT);
  };
	
  /* Wait for read or timer event flag top be set */
  status = SYS$WFLOR(read_event, event_mask);
  if (status != SS$_NORMAL) {
    DEBUG (("*fatal error : timed_read - Abnormal WFLOR status %d\n", status ));
    return (ER_LINK_SOFT);
  };
	
  /* Findout which event flag was set */
  status = SYS$READEF(read_event, &flags);
  if ((status != SS$_WASSET) && (status != SS$_WASCLR)) {
    DEBUG (("*fatal error : timed_read - Abnormal READEF status %d\n", status ));
    return (ER_LINK_SOFT);
  };

  if ((flags & read_mask) == read_mask)  { /* It was a read, not a timeout */
    int fnresult = read_iosb[0] & 0XFFFF;

    status = SYS$CANTIM(0, PSL$C_USER);  /* cancel timer */

    /* Extract length of data from i/o status block */
    bytes_read = (read_iosb[0] & 0XFFFF0000) >> 16;
  } else {
    status = SYS$CANCEL(socket);
  }
  return(bytes_read);
}


#else                           /* WIN_TCP (not VMS) */

PRIVATE int timed_read (s, buf, len, millisec)
int s;
char *buf;
int len;
long int millisec;
{
  int fdwidth;
  fd_set readfds;
  struct timeval timeout;

  assert (s < FD_SETSIZE);

  timeout.tv_sec = millisec / 1000;
  timeout.tv_usec = (millisec % 1000) * 1000;

  fdwidth = s + 1;

  FD_ZERO (&readfds);
  FD_SET (s, &readfds);

  if (select (fdwidth, &readfds, (fd_set *) NULL, (fd_set *) NULL, &timeout) > 0) {
    return (read (s, buf, len));
  } else {
    return (0);
  };
}

#endif                          /* WIN_TCP */

/* prot_send
	- send a protocol frame to the transputer & obtain a reply

	modifies : Result
	effects  : sends a frame to the transputer and returns either
		   ACK, NACK or TIMEOUT (TIMEOUT occurs if an ACK or
		   NACK of the correct sequence number is received
		   within the timeout period. the timeout parameter
		   passed is sent to the transputer & used for its own
		   timeout checking. If TIMEOUT is returned, inspect
		   Result for status value.
*/
PRIVATE unsigned char prot_send (s, op, seq, len, timeout)
LINK s;
unsigned char op, seq;
short int len;
short int timeout;
{
  struct prot_dgram requ, resp;
  int tries, reply_ok, numbytes, timeval1, timeval2;

  requ.op = op;
  requ.seq = seq;
  requ.len = htons ((short int) len);
  requ.timeout = htons ((short int) timeout);

  /*
   * make upto MAX_RETRIES tries to successfully send a frame to the
   * transputer, and get either an ACK or NACK with the correct sequence
   * number back
   */
  Result = SUCCEEDED;
  for (tries = 1; tries < MAX_RETRIES; tries++) {
    /* send request packet */
    if (send (s, (char *) &requ, sizeof (requ), 0) < 0) {
      DEBUG (( "*fatal error : prot_send - send failed =%d\n", ERRORCODE ));
      Result = ER_LINK_SOFT;
      return (rp_TIMEOUT);
    } else {
      /* request sent ok, so get reply */
      time (&timeval1);
      reply_ok = FALSE;
      
      while (!reply_ok) {
	numbytes = timed_read (s, (char *) &resp, sizeof (resp),
			       timeout_period * tries);
	if (numbytes > 0) {                /* got a reply */
	  reply_ok = TRUE;
	  if  ((resp.op == rp_ACK || resp.op == rp_NACK) && (resp.seq == seq)) {
	    return (resp.op);             
	  };
	};
	time (&timeval2);
	if ((timeval2-timeval1) >= QUIETTIMEOUT) {
	  reply_ok = TRUE;
	};
      };
    };
  };
  /* failed */
  Result = ER_LINK_CANT;
  return (rp_TIMEOUT);
}

/* peek_send
	- send a peek frame to the transputer & obtain a reply

	modifies : Result
	effects  : sends a frame to the transputer and returns either
		   ACK, NACK or TIMEOUT (TIMEOUT occurs if an ACK or
		   NACK of the correct sequence number is received
		   within the timeout period. the timeout parameter
		   passed is sent to the transputer & used for its own
		   timeout checking. if TIMEOUT is returned, inspect
		   Result for status value.
*/               
PRIVATE unsigned char peek_send (s, seq, address, numwords)
LINK s;
unsigned char seq;
u_long address;
short int numwords;
{
  struct peek_dgram requ;
  struct prot_dgram resp;
  int tries, reply_ok, numbytes, timeval1, timeval2;

  requ.op = rp_PEEK;
  requ.seq = h_seq;
  requ.address = htonl(address);
  requ.numwords = htons(numwords);
  /*
   * make upto MAX_RETRIES tries to successfully send a frame to the
   * transputer, and get either an ACK or NACK with the correct sequence
   * number back
   */
  Result = SUCCEEDED;
  for (tries = 1; tries < MAX_RETRIES; tries++) {
    /* send request packet */
    if (send (s, (char *) &requ, sizeof (requ), 0) < 0) {
      DEBUG (("*fatal error : peek_send - send failed =%d\n", ERRORCODE ));
      Result == ER_LINK_SOFT;
      return (rp_TIMEOUT);
    } else {
      /* request sent ok, so get reply */
      reply_ok = FALSE;
      time (&timeval1);
      while (!reply_ok) {
	numbytes = timed_read (s, (char *) &resp, sizeof (resp),
			       timeout_period * tries);
	if (numbytes > 0) {
	  reply_ok = TRUE;
	  if ((resp.op == rp_ACK || resp.op == rp_NACK) && (resp.seq == seq)) {
	    return (resp.op);
	  };
	};
	time (&timeval2); /* if X seconds elapsed cant contact transputer */
	if ((timeval2-timeval1) >= QUIETTIMEOUT) {
	  reply_ok = TRUE;
	};
      }; 
    };
  };
  /* failed */
  Result = ER_LINK_CANT;
  return (rp_TIMEOUT);
}

/* data_send
	- send data to the transputer & obtain a reply
	modifies : Result
	effects   : returns either
			TRUE    (data send successfully)
			FALSE   (send failed -> status returned in Result)
*/
PRIVATE int data_send (s, op, buff, len)
LINK s;
unsigned char op;
char *buff;
short int len;
{
  struct data_dgram requ;
  struct prot_dgram resp, reply;
  int tries, count, rcount, reply_ok, tresult, timed_out;

  Result = SUCCEEDED;
    
  memcpy (requ.data, buff, len);
  requ.op = op;
  requ.seq = h_seq;
  requ.len = htons ((short int) len);

  DEBUG (( "data_send: sending %d\n", len ));

  len += rp_DATA_HEADER;

  for (tries = 1; tries < MAX_RETRIES; tries++) {
    /* send request packet */
    if (send (s, (char *) &requ, sizeof (requ), 0) < 0) {
      Result = ER_LINK_SOFT;
      return (FALSE);
    } else {
      reply_ok = FALSE;
      timed_out = FALSE;
      while (!timed_out && !reply_ok) {
	tresult = timed_read (s, (char *) &resp, sizeof (resp),
			      timeout_period * tries);
	timed_out = (tresult == 0);
	reply_ok = (!timed_out && (resp.op == rp_ACK || resp.op == rp_NACK || 
		     resp.op == rp_CONT || resp.op == rp_SUSP) && resp.seq == h_seq);
	if (reply_ok && resp.op == rp_NACK) {
	  Result = ER_LINK_BAD;
	  return (Result);
	};
	if (!timed_out && resp.op == rp_DATA) {
	  DEBUG (( "received DATA frame in write\n"));
	  reply.op = rp_ACK;
	  reply.seq = resp.seq;
	  reply.len = 0;
	  if (send (s, (char *) &reply, sizeof (reply), 0) < 0) {
	    DEBUG (( "*fatal error : data_send[2] - send failed =%d\n", ERRORCODE ));
	    Result = ER_LINK_SOFT;
	    return (FALSE);
	  };
	};
      };
      if (reply_ok) {
	if ((resp.op == rp_SUSP) && (resp.seq == h_seq)) {
	  reply_ok = FALSE;
	  timed_out = FALSE;
	  while (!timed_out && !reply_ok) {
	    rcount = timed_read (s, (char *) &resp, sizeof (resp),
				 timeout_period * tries);
	    timed_out = (rcount == 0);                   
	    if (!timed_out && resp.seq == h_seq && resp.op == rp_ACK) {
	      reply_ok = TRUE;
	      h_seq = (h_seq + 1) &0x007f;
	      return (TRUE);
	    };
	  };
	  tries = 1;    /* transputer didn't do request so retry*/
	} else {
	  if (resp.op == rp_ACK) {
	    h_seq = (h_seq + 1) & 0x007f;
	    return (TRUE);
	  } else {
	    DEBUG (( "Invalid sp frame received %d -- Please report\n", resp.op ));
	    h_seq = (h_seq + 1) & 0x007f;
	    Result = ER_LINK_CANT;
	    return (FALSE);
	  };
	};
      };
    };
  };
  /* failed */
  h_seq = (h_seq + 1) & 0x007f;
  Result = ER_LINK_CANT;
  return (FALSE);
}         

/* data_read
	- read data from transputer

	effects  : returns either
		    ER_LINK_BAD     (link wasn't open)
		    ER_LINK_CANT    (link communication failed)
		    ER_LINK_SOFT    (link software failure)
		    0               (no data could be read in available time)
		   or number of bytes successfully read
*/
PRIVATE int data_read (s, buff, maxlen, len, timeout)
LINK s;
char **buff;
short int maxlen;
short int len;
short int timeout;
{
  struct prot_dgram resp;
  int tries, data_ok, code;
  unsigned char next_t_seq, next_h_seq;

  next_t_seq = (t_seq + 1) & 0x007f;
  next_h_seq = (h_seq + 1) & 0x007f;

  Result = SUCCEEDED;
  for (tries = 1; tries < MAX_RETRIES; tries++) {
    code = prot_send (s, rp_REQ, h_seq, len, timeout);
    if (code == rp_NACK) {
      h_seq = next_h_seq;
      return (ER_LINK_BAD);
    };
    if (code == rp_TIMEOUT) {
      h_seq = next_h_seq;
      return (Result);
    } else {
      data_ok = timed_read (s, (char *) &inp, sizeof (inp),
			    timeout_period * tries);
      if (data_ok >= 2 && inp.op == rp_DATA && inp.seq == t_seq) {
	h_seq = next_h_seq;
	t_seq = next_t_seq;
	resp.op = rp_ACK;
	resp.seq = inp.seq;
	resp.len = 0;
	if (send (s, (char *) &resp, sizeof (resp), 0) < 0) {
	  DEBUG (( "*fatal error : data_read[1] - send failed =%d\n", ERRORCODE ));
	  return (ER_LINK_SOFT);
	};
	DEBUG (( "input len = %d\n", len ));
	*buff = (char *) &(inp.data[0]);
	DEBUG (( "data_read: returning %d\n", ntohs (inp.len) ));
	return (ntohs (inp.len));
      } else {
	if ((data_ok >=2) && (inp.op != rp_ACK) && (inp.op != rp_NACK)&&
	   (inp.op != rp_CONT) && (inp.op != rp_ERROR)) {
	  resp.op = rp_ACK;
	  resp.seq = inp.seq;
	  resp.len = 0;
	  if (send (s, (char *) &resp, sizeof (resp), 0) < 0) {
	    DEBUG (( "*fatal error : data_read[2] - send failed =%d\n", ERRORCODE ));
	    return (ER_LINK_SOFT);
	  };
	};
      };
    };
  };
  return (ER_LINK_CANT);           
}

/*
 *  Eof
   */ 
