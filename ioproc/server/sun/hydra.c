/*------------------------------------------------------------------------
--                                                                      --
--         H E L I O S   I N P U T / O U T P U T   S E R V E R          --
--         ---------------------------------------------------          --
--                                                                      --
--         Copyright (C) 1989, Perihelion Software Ltd.                 --
--                       All Rights Reserved.                           --
--                                                                      --
--  hydra.c                                                             --
--                                                                      --
--  Author:  BLV 8/6/89                                                 --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: hydra.c,v 1.18 1993/09/28 12:24:05 bart Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.       			*/

/**
*** This program supports access to links over the ethernet, using 
*** TCP/IP sockets.
**/

/*{{{  header files, declarations, etc. */
#define Daemon_Module

/**
*** Different levels of debugging may be compiled in.
***
*** 0 -> no debugging
*** 1 -> limited debugging only
*** 2 -> most debugging
*** 3 -> everything, lots of debugging even when idle
**/
#define debug 0

#ifdef ARMBSD
#include "helios.h"
#else
#include "../helios.h"
#endif
#include "../sccs.h"

#if (0)
	/* was needed for RISCiX 1.1 */
#include <pwd.h>
char *cuserid(buf)
char *buf;
{ struct passwd *pw = getpwuid(getuid());
  strcpy(buf, pw->pw_name);
  return(buf);
}
#endif

/*@@@ 16.05.90 */
#define TIMEOUT_VALUE 100  /* gives 10 seconds for inmos compatible links */
/**
*** The link daemon is linked with module sun/linklib.c and any
*** hardware-specific modules, e.g. sun/kpar.c on a Sun386,
*** b011.c & b014.c & b016.c & niche.c on Sun3 and Sun4, etc.
*** Access to the link I/O functions is by indirecting through a set
*** of pointers, which are set up by the resetlnk() routine in
*** module sun/linklib.c
**/
extern int (*open_link_fn)();
extern void (*free_link_fn)();
int  (*rdrdy_fn)();
int  (*wrrdy_fn)();
int  (*byte_to_link_fn)();
int  (*byte_from_link_fn)();
int  (*send_block_fn)();
int  (*fetch_block_fn)();
void (*reset_fn)();
void (*analyse_fn)();

/**
*** To keep the code slightly more legible, these macros indirect
*** through the above function pointers.
**/
#define open_link         (*open_link_fn)
#define free_link         (*free_link_fn)
#define rdrdy             (*rdrdy_fn)
#define wrrdy             (*wrrdy_fn)
#define byte_to_link      (*byte_to_link_fn)
#define byte_from_link    (*byte_from_link_fn)
#define send_block        (*send_block_fn)
#define fetch_block       (*fetch_block_fn)
#define reset_processor   (*reset_fn)
#define analyse_processor (*analyse_fn)

/**
*** Various variables are needed to link with sun/linklib.c, these
*** are in the header file server.h in the I/O Server.
**/
int                Server_Mode = Mode_Daemon;
jmp_buf            exit_jmpbuf;
int                current_link = 0;
int                number_of_links = 1;
int                transputer_site = 0;
PRIVATE Trans_link first_link;
Trans_link         *link_table = &(first_link);
bool               C40HalfDuplex = FALSE;
word               target_processor = Processor_C40;
char	           *bootstrap;
word               bootsize;
word               debugflags	= 0;

/**
*** Here are various variables specific to Hydra. Hydra can have two
*** different types of connections, Server connections and Debugging
*** connections from Hydramon. At present I allow for 64 of the former,
*** and 4 for the latter. The table con_table contains details of the
*** Server connections, e.g. the user name and location. The table
*** debug_table does something similar for the Debugging connections,
*** but less information is needed. These limits are rather generous
*** given the probable amount of available hardware and the restricted
*** number of file descriptors available, but never mind.
**/
#define MaxConnections 64         /* Maximum number of link conections       */
                                  /* Telmat SM90 has up to 48 link adapters  */
                                  /* which is the current record */
#define Max_debug_connections 4   /* allow up to 4 hydramon sessions */

socket_msg con_table[MaxConnections];  /* to hold all the connections */

typedef struct debug_connection {
        WORD   fildes;
        WORD   flags;
} debug_connection;
debug_connection debug_table[Max_debug_connections];
#define Debugtab_Free 0x0001

/**
*** Here is a buffer for holding messages. 512K is a bit excessive,
*** but Hydra will only run on Unix boxes with plenty of virtual
*** memory. 64K is not enough because the buffer may be used to hold
*** a system image larger than a Helios message.
**/
UBYTE big_buffer[512 * 1024]; 

/**
*** Here are some variables to hold the connection and socket details.
*** family_type can be AF_UNIX or AF_INET. For Unix sockets, socket_name
*** will contain the name of the socket extracted from the configuration
*** file. last_connection holds the last time when a Server connected.
*** During the initial bootstrap phase there is so much link traffic that
*** Hydra can get overloaded, so a delay is enforced between connections.
*** This delay is obtained from the configuration file, defaulting to
*** 10 seconds. Finally, my_socket is the socket on which Hydra waits
*** for new connections.
**/
PRIVATE int family_type = AF_UNIX;
PRIVATE char *socket_name;
PRIVATE WORD last_connection = 0;
PRIVATE WORD connection_delay;
PRIVATE int my_socket;

/**
*** This may be set inside a signal routine
**/
PRIVATE int Special_Exit = 0;

/**
*** Like the I/O Server, Hydra reads in a configuration file.
*** The default file name is hydra.con, but this may be overwritten
*** using a command-line argument.
**/
/*@@@ 16.05.90 */
PRIVATE char *configname = "hydra.con";


/**
*** Various function prototypes.
**/
PRIVATE int  fn( read_config, (char *));
PRIVATE void fn( tidy_config, (void));
PRIVATE void fn( create_socket, (void));
PRIVATE void fn( main_loop, (void));
PRIVATE void fn( initialise_signals, (void));
void fn( socket_broken, (int));
/*}}}*/
/*{{{  main() */

/**
*** And at long last some code. First the jump buffer is initialised so
*** that an exit is possible at all times. Next the copyright message
*** is displayed. The arguments are processed to look for a
*** configuration file, and the configuration file is read in.
***
*** Next the Unix signal mechanism is effectively disabled to stop
*** nasties from happening. In addition an alarm signal is set to go
*** off at regular intervals, aborting any blocked link I/O. 
***
*** The routine resetlnk() in sun/linklib.c initialises the link
*** hardware. It determines which links are available and stores
*** this data in link_table. All the links are set to unused if they
*** appear to exist but cannot be opened, or free if they can be
*** opened succesfully - in the latter case the link is actually
*** free_link'ed, to allow other programs access to it while Hydra
*** is running. After resetlnk() all the connections are set to -1,
*** indicating that nobody owns the link, and the debug connections
*** are also initialised.
***
*** create_socket() installs the program as a daemon, which can be
*** accessed by clients. It sets my_socket to a value which can
*** be used to wait for incoming connections. Then the daemon detaches
*** itself from the current process group, i.e. it turns itself into
*** a true daemon. If this succeeds
*** the daemon can print out a success message, and enter its
*** main loop. At present there is no way to exit the main loop,
*** or to restart the daemon, but this may prove useful in the
*** future.
**/
int main(argc, argv)
int argc;
char *argv[];
{ int  i;
  char *curr_arg;
  int  return_code;
  int childpid;

  if ((return_code = setjmp(exit_jmpbuf)) ne 0) goto endpoint;
  printf("Helios Link Daemon - Hydra. %s%s",SccsId1, SccsId5);

  for (i=1; i<argc; i++)
    { curr_arg = argv[i];
      if (curr_arg[0] eq '-')
       if ((curr_arg[1] eq 'C') || (curr_arg[1] eq 'c'))
        { if (curr_arg[2] ne '\0')
           configname = &(curr_arg[2]);
          elif (++i < argc)
           configname = argv[i];
        }
    }

/*@@@ 16.05.90 */
  if (!read_config(configname)) {
	  printf ("usage : hydra [ -c <pathname of configuration file>]\n");
	  goto endpoint;
  }

  connection_delay = get_int_config("connection_delay");
  if (connection_delay eq Invalid_config)
   connection_delay = 10;  /* 10 second default */

  initialise_signals();

  resetlnk();

  for (i = 0; i < number_of_links; i++)
   link_table[i].connection = -1;

  for (i = 0; i < Max_debug_connections; i++)
   debug_table[i].flags = Debugtab_Free;

  create_socket();

  printf("Hydra: running on %d link%s.\n", number_of_links,
	 (number_of_links != 1) ? "s" : "");

  switch(fork())
	{
	case 0:		/* child */
#if (TR5 || i486V4 || HP9000)
		setpgrp();	/* new session */
#else
		setpgrp(getpid(), 0);	/* new session */
#endif
		setuid(geteuid());	/* run as set-uid bit says */
		setgid(getegid());	/* run as set-gid bit says */
		break;

	case -1:	/* can't fork */
		printf("Hydra : can't fork daemon, continuing\n");
		break;

	default:
		exit(0);
	}

  main_loop();

  if (family_type eq AF_UNIX)
    unlink(socket_name);

  /* Release all links here */

endpoint:
  tidy_config();

  return(return_code);
}
/*}}}*/
/*{{{  signal handling */

/**
*** Signal handling. Some signals will cause an exit. The timer signal is
*** used to abort blocked link I/O.
**/
PRIVATE void exit_interrupt()
{ Special_Exit = 1;
}

PRIVATE void alarm_handler()
{
}

PRIVATE void initialise_signals()
{
#ifdef   SIGHUP
  signal(SIGHUP   , func(exit_interrupt));
#endif
#if (SM90)
#else
#ifdef   SIGINT
  signal(SIGINT   , func(exit_interrupt));
#endif
#endif
#ifdef   SIGQUIT
/*  signal(SIGQUIT  , func(exit_interrupt));   leave a back door for safety */
/* kill -3 hydra will give a core dump */
#endif  
#ifdef   SIGILL
  signal(SIGILL   , SIG_IGN);
#endif
#ifdef   SIGTRAP
  signal(SIGTRAP  , SIG_IGN);
#endif
#ifdef   SIGEMT
  signal(SIGEMT   , SIG_IGN);
#endif
#ifdef   SIGFPE
  signal(SIGFPE   , SIG_IGN);
#endif
#ifdef   SIGBUS
  signal(SIGBUS   , SIG_IGN);
#endif
#if 0
				/* leave this one open */
#ifdef   SIGSEGV
  signal(SIGSEGV  , SIG_IGN);
#endif
#endif
#ifdef   SIGKILL
  signal(SIGKILL  , func(exit_interrupt));
#endif
#ifdef   SIGSYS
  signal(SIGSYS   , SIG_IGN);
#endif
#ifdef   SIGPIPE
  signal(SIGPIPE  , SIG_IGN);
#endif
#ifdef   SIGALRM
#if (SM90 || TR5)
#else
  signal(SIGALRM  , func(alarm_handler));
  alarm(2);   /* force an alarm signal every couple of seconds, this */
              /* unblocks any blocking I/O */
#endif
#endif
#ifdef   SIGTERM
  signal(SIGTERM  , func(exit_interrupt));
#endif
#ifdef   SIGURG
  signal(SIGURG   , SIG_IGN);
#endif
#ifdef   SIGIO
  signal(SIGIO    , SIG_IGN);
#endif
#ifdef   SIGUSR1
  signal(SIGUSR1  , SIG_IGN);
#endif
#ifdef   SIGUSR2
  signal(SIGUSR2  , SIG_IGN);
#endif
#ifdef   SIGABRT
  signal(SIGABRT  , SIG_IGN);
#endif
#ifdef   SIGSTOP
  signal(SIGSTOP  , func(exit_interrupt));
#endif
#ifdef   SIGTSTP
  signal(SIGTSTP  , func(exit_interrupt));
#endif
#ifdef   SIGCONT
  signal(SIGCONT  , SIG_IGN);
#endif
#ifdef   SIGCHLD
  signal(SIGCHLD  , SIG_IGN);
#endif
#ifdef   SIGTTIN
  signal(SIGTTIN  , SIG_IGN);
#endif
#ifdef   SIGTTOU
  signal(SIGTTOU  , SIG_IGN);
#endif
#ifdef   SIGXCPU
  signal(SIGXCPU  , SIG_IGN);
#endif
#ifdef   SIGXFSZ
  signal(SIGXFSZ  , SIG_IGN);
#endif
#ifdef   SIGVTALRM
  signal(SIGVTALRM, SIG_IGN);
#endif
#ifdef   SIGPROG
  signal(SIGPROF  , SIG_IGN);
#endif
#ifdef   SIGWINCH
  signal(SIGWINCH , SIG_IGN);
#endif
#ifdef   SIGLOST
  signal(SIGLOST  , SIG_IGN);
#endif
}
/*}}}*/
/*{{{  create the daemon's socket */

/**
*** This routine creates a socket on which the link daemon waits for
*** incoming requests. The socket may be a Unix one or an internet
*** one, and is a TCP/IP stream socket.
**/
PRIVATE void create_socket()
{
  char *family_name = get_config("family_name");
  int family_type = AF_UNIX;

  socket_name = get_config("socket_name");

  if (family_name eq (char *) NULL)
   family_name = "AF_UNIX";

  if (!mystrcmp(family_name, "AF_UNIX"))
   family_type = AF_UNIX;
  elif(!mystrcmp(family_name, "AF_INET"))
   family_type = AF_INET;
  else
   { printf("Hydra : unknown family type %s in configuration file\n",
            family_name);
     longjmp(exit_jmpbuf, 1);
   }

  my_socket = socket(family_type, SOCK_STREAM, 0);
  if (my_socket < 0)
   { perror("Hydra : failed to create socket");
     longjmp(exit_jmpbuf, 1);
   }

  if (family_type eq AF_UNIX)
   { struct sockaddr_un addr;
     addr.sun_family = AF_UNIX;
     strcpy(addr.sun_path,
            (socket_name eq (char *) NULL) ? "hydra.skt" : socket_name);
     if (bind(my_socket, &addr, 
          strlen(addr.sun_path) + sizeof(addr.sun_family) ) eq -1)
      { perror("Hydra : failed to bind socket");
        longjmp(exit_jmpbuf, 1);
      }     
   }
  else
   { struct sockaddr_in addr;
     struct servent     *sp;

     sp = getservbyname("hydra", "tcp");
     if (sp eq NULL)
      { printf("Hydra has not been allocated a socket port on this machine.\n");
        printf("Please edit the file /etc/services\n");
        longjmp(exit_jmpbuf, 1);
      }

     bzero((char *) &addr, sizeof(struct sockaddr_in));
     addr.sin_addr.s_addr   = INADDR_ANY;
     addr.sin_port   = sp->s_port;
     if (bind(my_socket, &addr, sizeof(struct sockaddr_in)) eq -1)
      { perror("Hydra : failed to bind socket");
        longjmp(exit_jmpbuf, 1);
      }
   }


  setsockopt(my_socket, SOL_SOCKET, SO_REUSEADDR, 10, 0);
  
  if (listen(my_socket, 5) ne 0)
   { perror("Hydra : failed to listen on socket");
     longjmp(exit_jmpbuf, 1);
   }
}
/*}}}*/
/*{{{  main loop */
/**
*** This is the main loop of the link daemon. It waits for new connections,
*** data ready on any of the links currently in use, requests from any of
*** the I/O Servers currently connected, or requests from Hydramon. Life is
*** made rather complicated by the fact that not all link interfaces support
*** select and that not all systems support microsecond timeouts.
***
*** The first job is to examine all possible link devices. Links that are not
*** currently in use by Hydra or that should never be used by Hydra are
*** ignored. Also if the I/O Server has been informed already that data
*** is available then there is no point in testing this link again until the
*** I/O Server has sent an appropriate request. If the link supports select()
*** then the appropriate file descriptor is added to the select() mask.
*** Otherwise the select() timeout is reduced to allow polling and the link
*** is checked for data.
***
*** In addition to the links it is necessary to check the sockets from all
*** connected I/O Servers, the socket used to accept new connections, and
*** any debug connections from Hydramon. All of these are supported by
*** select().
***
*** Once the select() has returned it is possible to check which sources of
*** data are ready and invoke appropriate handlers.
**/

PRIVATE void fn( accept_connection, (void));
PRIVATE void fn( accept_debug_connection, ( WORD, socket_msg *));
PRIVATE void fn( from_debug, (int));
PRIVATE void fn( from_transputer, (int));
PRIVATE void fn( from_server, (int));
PRIVATE void fn( message_from_transputer, (int));
PRIVATE void fn( message_from_server, (int));
PRIVATE jmp_buf again;

PRIVATE void main_loop()
{ fd_set rd_mask;
  int    i;
  int    max_fd;
  struct timeval select_time;

  setjmp(again);

  forever
   { FD_ZERO(&rd_mask);
     max_fd = -1;

     select_time.tv_sec  = 0;
     select_time.tv_usec = 500000;   /* half a second delay */

     for (i = 0; i < number_of_links; i++)
      if (!(link_table[i].flags & (Link_flags_free | Link_flags_unused)))
       {
         unless(link_table[i].flags & Link_flags_datareadysent)
          {
            if (link_table[i].flags & Link_flags_not_selectable)
             {
               select_time.tv_usec = 1000;   /* need to poll the link */
               current_link = i;
               if (rdrdy())
                {
                  if (link_table[i].flags & Link_flags_messagemode)
                   message_from_transputer(i);
                  else
                   from_transputer(i);
                }
             }
            else
             { FD_SET(link_table[i].fildes, &rd_mask);
               if (link_table[i].fildes > max_fd) max_fd = link_table[i].fildes;
             }
          }

	 FD_SET(link_table[i].connection, &rd_mask);
	 if (link_table[i].connection > max_fd) max_fd=link_table[i].connection;
       }

#if SM90
     select_time.tv_sec = 1;
#endif

     FD_SET(my_socket, &rd_mask);
     if (my_socket > max_fd) max_fd = my_socket;

     for (i = 0; i < Max_debug_connections; i++)
      if (!(debug_table[i].flags & Debugtab_Free))
       { FD_SET(debug_table[i].fildes, &rd_mask);
         if (debug_table[i].fildes > max_fd) max_fd = debug_table[i].fildes;
       }

     i = select(max_fd+1, &rd_mask, NULL, NULL, &select_time);
#if (debug > 2)
     printf("Hydra : select returned %d\n", i);
#endif

#if (TR5)	/* le select n'a pas d'effet sur rd_mask su TR5 */
     if ( i == 0 )FD_ZERO(&rd_mask);	/* timeout select case */
#endif

     if (Special_Exit) return;
     if (i < 0)
      { if (errno ne EINTR) perror("Hydra : select failed");
        continue;
      }

     if (FD_ISSET(my_socket, &rd_mask))
         accept_connection();

     for (i = 0; i < Max_debug_connections; i++)
      if (!(debug_table[i].flags & Debugtab_Free))
       if (FD_ISSET(debug_table[i].fildes, &rd_mask))
         from_debug(i);
        
     for (i = 0; i < number_of_links; i++)
      if (!(link_table[i].flags & (Link_flags_free | Link_flags_unused)))
       {
  	  unless(link_table[i].flags & Link_flags_not_selectable)
           if (FD_ISSET(link_table[i].fildes, &rd_mask))
            {
              if (link_table[i].flags & Link_flags_messagemode)
               message_from_transputer(i);
              else
               from_transputer(i);
            }

         if (FD_ISSET(link_table[i].connection, &rd_mask))
	  {
	    if (link_table[i].flags & Link_flags_messagemode)
	     message_from_server(i);
	    else
             from_server(i);
          }
       }
   }     
}
/*}}}*/
/*{{{  accept new connection */

/**
*** This routine is called when the main select() indicates that some
*** new client is attempting to connect to the daemon. First there is
*** an accept() call, which should never fail because we know that
*** something is trying to connect. Next there is a read of a socket
*** message. This structure is defined in the header structs.h, and
*** contains lots of info. First, the request may be a debug connection
*** rather than an I/O Server connection, which is handled by a
*** separate routine. Note that the data transferred in socket_msg uses
*** the transputer's byte ordering, i.e. little-endian, which will have
*** to be swapped on 68000 and sparc machines using the normal swap()
*** macro. If the request is not a debug connection it comes from an
*** I/O Server, and the fun really starts.
***
*** The bootstrap phase of a transputer is very link-intensive. It
*** involves the nucleus, posix library, C library, init, network
*** server, task force manager, shell, and possibly a couple of other
*** programs. This will be noticeable by other users, because their
*** response time will increase. To avoid the worst of the problems
*** Hydra will only accept new connections every few seconds, the
*** exact amount being determined by the entry connection_delay in
*** the configuration file hydra.con . The I/O Server contains code to
*** print out suitable delay messages, and to retry a number of times.
***
*** Once past this hurdle, the request may be for any link. The routine
*** looks for a free link and tries to open it. If this succeeds the
*** link is now owned by that person. The link state is modified to
*** reflect this, and the connection table is filled in with the
*** owner's name, location, etc. A suitable reply message is generated.
*** If none of the links are currently available, an error message is
*** returned.
***
*** The alternative is that the request is for a specific link. If this
*** link is available, fine. Otherwise an error message is generated.
***
*** Note that if no link is available the connection is closed.
**/

PRIVATE void accept_connection()
{ int fil;
  struct sockaddr new_sockaddr;
  int temp, i;
  socket_msg con;
  WORD now = time((time_t *) NULL);

  temp = sizeof(struct sockaddr);

#if (debug > 0)
  printf("Hydra : accepting connection\n");
#endif
 
  fil = accept(my_socket, &new_sockaddr, &temp);
  if (fil < 0)
   { perror("Hydra : failed to accept connection on socket");
     return;
   }

  if (read(fil, (BYTE * ) &con, sizeof(socket_msg))
        < sizeof(socket_msg))
   { perror("Hydra : failed to read connection packet");
     shutdown(fil, 2);
     close(fil);
     return;
   }

  temp = swap(con.fnrc);

  if (temp eq Debug_Connection)
   { accept_debug_connection( fil, &con);
     return;
   }

  if ((now - last_connection) < connection_delay)
   { con.fnrc = swap(Hydra_Busy);
#if (debug > 0)
     printf("Hydra : busy, %d seconds since last connection\n",
            now - last_connection);
#endif
     if (write(fil, (BYTE *) &con, sizeof(socket_msg))
         < sizeof(socket_msg))
       perror("Hydra : failed to reply to Server");
     shutdown(fil, 2);
     close(fil);
     return;
   }

  if (temp eq Any_Link)
   {
#if (debug > 0)
     printf("Hydra : connecting to any link\n");
#endif 
     for (i = 0; i < number_of_links; i++)
      if (link_table[i].flags & Link_flags_free)
       { if ((*open_link_fn)(i) eq 0) continue;  /* not available */
#if (debug > 0)
         printf("Hydra : connected to link %d\n", i);
#endif
         con.fnrc = swap(i);
         if (write(fil, (BYTE *) &con, sizeof(socket_msg))
              < sizeof(socket_msg))
          { perror("Hydra : failed to reply to Server");
            shutdown(fil, 2);
            close(fil);
            return;
          }
         link_table[i].flags &= ~(Link_flags_free | Link_flags_uninitialised |
                                  Link_flags_firsttime);
         link_table[i].state = Link_Reset;
         link_table[i].connection = fil;
         memcpy((BYTE *) &(con_table[i]), (BYTE *) &con,
          sizeof(socket_msg));
         last_connection = now;
         return;
       }

#if (debug > 0)
     printf("Hydra : no link available\n");
#endif
     con.fnrc = swap(Link_Unavailable);
     if (write(fil, (BYTE *) &con, sizeof(socket_msg)) 
         < sizeof(socket_msg))
      perror("Hydra : failed to reply to Server");
     shutdown(fil, 2);
     close(fil);
     return;
   }
       
  if ((temp < 0) || (temp > number_of_links))
   { con.fnrc = swap(Invalid_Link);
#if (debug > 0)
     printf("Hydra : invalid link %d requested\n", temp);
#endif
     if (write(fil, (BYTE *) &con, sizeof(socket_msg))
          < sizeof(socket_msg))
      perror("Hydra : failed to reply to server");
     shutdown(fil, 2);
     close(fil);
     return;
   }

  if (link_table[temp].flags & Link_flags_free)
   { if (open_link_fn(temp) eq 0)
      goto fail;
#if (debug > 0)
     printf("Hydra : connected to link %d as requested\n", temp);
#endif
     if (write(fil, (BYTE *) &con, sizeof(socket_msg)) 
         < sizeof(socket_msg))
      { perror("Hydra : failed to reply to server");
        shutdown(fil, 2);
        close(fil);
        return;
      }
     memcpy((BYTE *) &(con_table[temp]), (BYTE *) &con, 
            sizeof(socket_msg));
     link_table[temp].flags &= ~(Link_flags_free | Link_flags_uninitialised |
                                 Link_flags_firsttime);
     link_table[temp].connection = fil;
     last_connection = now;
     return;
   }

fail:
#if (debug > 0)
  printf("Hydra : requested link %d unavailable\n", temp);
#endif
  con.fnrc = swap(Link_Unavailable);
  if (write(fil, (BYTE *) &con, sizeof(socket_msg)) 
      < sizeof(socket_msg))
   perror("Hydra : failed to reply to server");
  shutdown(fil, 2);
  close(fil); 
}

/**
*** This routine is called from accept_connection() above, if the
*** new connection is a debugging one from Hydramon. If a slot is
*** available in the debugging table, fine.
**/
PRIVATE void accept_debug_connection(fildes, con)
WORD fildes;
socket_msg *con;
{ int i;

#if (debug > 0)
  printf("Hydra : accepting debug connection\n");
#endif

  for (i = 0; i < Max_debug_connections; i++)
   if (debug_table[i].flags & Debugtab_Free)
    { con->fnrc  = swap(1);
      con->extra = swap(number_of_links);
      if (write(fildes, (BYTE *) con, sizeof(socket_msg))
          < sizeof(socket_msg))
       { perror("Hydra : failed to reply to hydramon");
         shutdown(fildes, 2);
         close(fildes);
         return;
       }
      debug_table[i].flags &= ~Debugtab_Free;
      debug_table[i].fildes = fildes;
      return;
    }

#if (debug > 0)
  printf("Hydra : all debug connections used\n");
#endif
  con->fnrc  = swap(Link_Unavailable);
  if (write(fildes, (BYTE *) con, sizeof(socket_msg))
      < sizeof(socket_msg))
    perror("Hydra : failed to reply to hydramon");
  shutdown(fildes, 2);
  close(fildes);
}
/*}}}*/
/*{{{  new protocol between root processor and I/O Server */

/**
*** If the root processor is trying to send some data then the I/O Server
*** should be informed. Hydra reads either a byte or a word from the link,
*** depending on the hardware, and includes this in the message. This avoids
*** problems with certain device drivers and improves the latency.
**/

PRIVATE void from_transputer(link)
int link;
{ Hydra_Message message;

  current_link = link;

  if (link_table[link].flags & Link_flags_word)
   { 
     if (fetch_block(4, message.Extra.Buf, TIMEOUT_VALUE))
     	return;
     link_table[link].last_send = *((int *) message.Extra.Buf);
#if (Debug > 1)
     printf("Hydra: sending DataReadyWord\n");
#endif
     message.FnRc	= swap(Hydra_DataReadyWord);
   }
  else
   {
     if (byte_from_link(message.Extra.Buf))
     	return;
     link_table[link].last_send = message.Extra.Buf[0];
#if (Debug > 1)
     printf("Hydra: sending DataReadyByte\n");
#endif
     message.FnRc	= swap(Hydra_DataReadyByte);
   }

  socket_write(link, (BYTE *) &message, sizeof(Hydra_Message));

  link_table[link].flags |= Link_flags_datareadysent;

#if (Debug > 2)
  { struct timeval xyz;
    gettimeofday(&xyz, NULL);
    printf("Hydra: sent dataready at %d.%06d\n", xyz.tv_sec, xyz.tv_usec);
  }
  message.FnRc	= swap(Hydra_Nop);
  socket_write(link, (BYTE *) &message, sizeof(Hydra_Message));
#endif
}

/**
*** The I/O Server can send a number of different requests for interacting
*** with the link device. 
**/

PRIVATE void from_server(link)
int link;
{ Hydra_Message message;
  int		size;
  
  current_link = link;

  socket_read(link, (BYTE *) &message, sizeof(Hydra_Message));
  message.FnRc	= swap(message.FnRc);
  size		= swap(message.Extra.Size);

#if (debug > 1)
  printf("hydra: got request %x, size %d, flags & sent %x \n", message.FnRc, size, link_table[link].flags & Link_flags_datareadysent);
#endif

  switch(message.FnRc)
   {
   	case Hydra_ResetRequest :
   		reset_processor();
   		message.FnRc = swap(Hydra_ResetAck);
   		socket_write(link, (BYTE *) &message, sizeof(Hydra_Message));
		link_table[link].flags &= ~Link_flags_datareadysent;
   		break;

   	case Hydra_AnalyseRequest :
   		analyse_processor();
   		message.FnRc = swap(Hydra_AnalyseAck);
   		socket_write(link, (BYTE *) &message, sizeof(Hydra_Message));
		link_table[link].flags &= ~Link_flags_datareadysent;
   		break;

   	case Hydra_ReadRequest :
			/* If a byte/word has been sent already, allow for this.	*/
		if (link_table[link].flags & Link_flags_datareadysent)
			size -= (link_table[link].flags & Link_flags_word) ? 4 : 1;

			/* Is there anything still to read ?				*/
		if (size > 0)
		{
			/* Yes, try to read it.						*/
			if (fetch_block(size, big_buffer, TIMEOUT_VALUE))
			{
				message.FnRc = swap(Hydra_Broken);
				socket_write(link, (BYTE *) &message, sizeof(Hydra_Message));
			}
			else
			{
				message.FnRc = swap(Hydra_ReadAck);
				socket_write(link, (BYTE *) &message, sizeof(Hydra_Message));
				socket_write(link, (BYTE *) big_buffer, size);
			}
		}
			/* No, the I/O Server has already received the required		*/
			/* amount of data.						*/
		else
		{
			message.FnRc = swap(Hydra_ReadAck);
			socket_write(link, (BYTE *) &message, sizeof(Hydra_Message));
		}
		link_table[link].flags &= ~Link_flags_datareadysent;
		break;

	case Hydra_WriteRequest :
		socket_read(link, big_buffer, size);
		if (send_block(size, big_buffer, TIMEOUT_VALUE))
			message.FnRc = swap(Hydra_Broken);
		else
			message.FnRc = swap(Hydra_WriteAck);
		socket_write(link, (BYTE *) &message, sizeof(Hydra_Message));
		break;

	case Hydra_Done :
		shutdown(link_table[link].connection, 2);
		close(link_table[link].connection);
		link_table[link].connection = -1;
		free_link(link);
		link_table[link].flags |= (Link_flags_free + Link_flags_uninitialised);
		link_table[link].state = Link_Reset;
		link_table[link].flags &= ~Link_flags_datareadysent;
		return;

	case Hydra_WriteByte :
		if (byte_to_link(message.Extra.Buf[0]))
			message.FnRc = swap(Hydra_Broken);
		else
			message.FnRc = swap(Hydra_WriteAck);
		socket_write(link, (BYTE *) &message, sizeof(Hydra_Message));
		break;
		
	case Hydra_WriteWord :
		if (send_block(4, message.Extra.Buf, TIMEOUT_VALUE))
			message.FnRc = swap(Hydra_Broken);
		else
			message.FnRc = swap(Hydra_WriteAck);
		socket_write(link, (BYTE *) &message, sizeof(Hydra_Message));
		break;

	case Hydra_ReadByte :
			/* If a byte has been sent already, just clear the flag		*/
		if ((link_table[link].flags & Link_flags_datareadysent) eq 0)
		{
			if (byte_from_link(message.Extra.Buf))
				message.FnRc = swap(Hydra_Broken);
			else
				message.FnRc = swap(Hydra_ReadAck);
			socket_write(link, (BYTE *) &message, sizeof(Hydra_Message));
		}
		link_table[link].flags &= ~Link_flags_datareadysent;
		break;

	case Hydra_ReadWord :
			/* Read word : this is complicated because it may not be	*/
			/* a word-based link. e.g. transputer peek and poke. Also Hydra	*/
			/* and the I/O Server may have different byte orderings.	*/
		if ((link_table[link].flags & Link_flags_word) &&
		    (link_table[link].flags & Link_flags_datareadysent))
		{
			/* The word has been sent already, no-op			*/
		}
		else
		{
			if (link_table[link].flags & Link_flags_datareadysent)
			{
					/* One byte already sent, three to go		*/
				if (fetch_block(3, &(message.Extra.Buf[1]), TIMEOUT_VALUE))
					message.FnRc	= swap(Hydra_Broken);
				else
					message.FnRc	= swap(Hydra_ReadAck);
			}
			else
			{
					/* nothing sent, four to go			*/
				if (fetch_block(4, message.Extra.Buf, TIMEOUT_VALUE))
					message.FnRc	= swap(Hydra_Broken);
				else
					message.FnRc	= swap(Hydra_ReadAck);
			}
			socket_write(link, (BYTE *) &message, sizeof(Hydra_Message));
		}
		link_table[link].flags &= ~Link_flags_datareadysent;
		break;				

		/* Switch from new protocol to old message passing mode	*/
	case Hydra_MessageMode :
		link_table[link].flags |= Link_flags_messagemode;
		if (message.Extra.Size)	/* I/O Server's C40HalfDuplex flag	*/
			link_table[link].flags |= Link_flags_halfduplex;
		message.FnRc	= swap(Hydra_MessageAck);
		socket_write(link, (BYTE *) &message, sizeof(Hydra_Message));
		link_table[link].state = Link_Running;
		if (link_table[link].flags & Link_flags_datareadysent)
			message_from_transputer(link);
		break;
					
	default :
		printf("Hydra: unexpected request 0x%08x from I/O Server on link %d\n",
			message.FnRc, link);
		socket_broken(link);
		link_table[link].flags &= ~Link_flags_datareadysent;
   }

	/* If the request was a write request and the datareadysent flag is set	*/
	/* then the I/O Server will have received a DataReady packet and the	*/
	/* write acknowledgement packet. It is now necessary to send another	*/
	/* packet to wake up the select(). Sadly this means that if the I/O	*/
	/* Server is sending a message while the root processor is trying to	*/
	/* send one as well then there could be four extra socket operations.	*/
	/* Sigh.								*/
	if (link_table[link].flags & Link_flags_datareadysent)
	{
		message.FnRc = swap(Hydra_Nop);
		socket_write(link, (BYTE *) &message, sizeof(Hydra_Message));
	}
}
/*}}}*/
/*{{{  message protocol between root processor and I/O Server */
/**
*** This code copes with full Helios messages going to and from the link.
*** It has the intelligence needed to cope with particular protocol bytes
*** etc. This makes it more efficient than the higher level protocol:
*** a message from the root processor to the I/O Server is handled by a
*** single socket write rather than four separate request/reply pairs.
*** The code is rather convoluted because of the need to handle both
*** byte- and word-based links.
**/

static void message_from_transputer(link)
int link;
{
	int	 protocol_word;
	bool	 word_link;
	int	*int_ptr	= (int *) big_buffer;
	MCB	 mcb;
	int	 size;
	
	if (link_table[link].flags & Link_flags_word)
		word_link = TRUE;
	else
		word_link = FALSE;
		
	if (link_table[link].flags & Link_flags_datareadysent)
	{
		/* one byte/word has been read from the link already.	*/
		link_table[link].flags &= ~Link_flags_datareadysent;
		protocol_word	= link_table[link].last_send;
	}
	else
	{
		/* fetch a byte/word from the link.			*/
		if (word_link)
			fetch_block(4, &protocol_word, TIMEOUT_VALUE);
		else
		{
			byte_from_link(big_buffer);
			protocol_word	= ((int)big_buffer[0]) & 0x000000FF;
		}
	}

		/* store the protocol byte/word in the buffer		*/
	if (word_link)
	{
		*((int *) big_buffer)	= protocol_word;
		protocol_word		= swap(protocol_word);
	}
	else
		big_buffer[3]	= protocol_word;

	switch(protocol_word)
	{
	case	Proto_Write :
			/* Store the poked value in the link table for	*/
			/* subsequent peeks, but do not pass it on to	*/
			/* the I/O Server.				*/
		if (fetch_block(8, &(big_buffer[4]), TIMEOUT_VALUE)) goto fail;
		link_table[link].last_send	= *((int *)&(big_buffer[8]));
		break;

	case	Proto_Read :
			/* Get the address and ignore it. Return the inverse	*/
			/* of the last value poked.				*/
		if (fetch_block(4, &(big_buffer[4]), TIMEOUT_VALUE)) goto fail;
		link_table[link].last_send = ~link_table[link].last_send;
		if (send_block(4, &(link_table[link].last_send), TIMEOUT_VALUE)) goto fail;
		break;			

	case	Proto_Msg :
/*{{{  receive message */
			if (link_table[link].flags & Link_flags_halfduplex)
			{
				if (word_link)
				{
					protocol_word	= swap(Proto_Go);
					if (send_block(4, &protocol_word, TIMEOUT_VALUE)) goto fail;
				}
				else
					if (byte_to_link(Proto_Go)) goto fail;
			}

			if (fetch_block(16, &(big_buffer[4]), TIMEOUT_VALUE)) goto fail;
				/* Work out the MCB values			*/
			{
				int	*x	= (int *) &mcb;
				*x++ = swap(int_ptr[1]);
				*x++ = swap(int_ptr[2]);
				*x++ = swap(int_ptr[3]);
				*x++ = swap(int_ptr[4]);
			}
#if (debug > 1)
			printf("Hydra: message from root processor on site %d, FnRc %x\n", link, mcb.MsgHdr.FnRc);
			printf("     : dsize %d, csize %d, flags 0x%02x, dest 0x%08x, reply 0x%08x\n",
				mcb.MsgHdr.DataSize, mcb.MsgHdr.ContSize, mcb.MsgHdr.Flags, mcb.MsgHdr.Dest, mcb.MsgHdr.Reply);
#endif
			/* Sort out the locations of the control and data vectors.	*/
		mcb.Control	= (int *) &(big_buffer[20]);
		size		= mcb.MsgHdr.ContSize * 4;
		mcb.Data	= (byte *) &(big_buffer[20 + size]);
		if (size > 0)
		 	if (fetch_block(size, mcb.Control, TIMEOUT_VALUE)) goto fail;
		size		= mcb.MsgHdr.DataSize;
		if (word_link)
		{
			size += (mcb.MsgHdr.Flags & MsgHdr_Flags_bytealign);
			size  = (size + 3) & ~3;
		}
		if (fetch_block(size, mcb.Data, TIMEOUT_VALUE)) goto fail;

		size += 16 + (mcb.MsgHdr.ContSize * 4);

		if (word_link)
			socket_write(link, big_buffer, size + 4);
		else
			socket_write(link, &(big_buffer[3]), size + 1);
/*}}}*/
		break;

	case	Proto_Info :
		if (word_link)
		{
			if (fetch_block(8, big_buffer, TIMEOUT_VALUE)) goto fail;
		}
		else
		{
			if (fetch_block(11, big_buffer, TIMEOUT_VALUE)) goto fail;
		}
		
		int_ptr[0] = swap(0xF0F0F0F0);
		int_ptr[1] = swap(0x00000100);
		int_ptr[2] = swap(0x00000001);
		if (send_block(12, big_buffer, TIMEOUT_VALUE)) goto fail;
		break;

	default:
		/* Ignore any junk bytes.				*/
		break;
	}

	return;	
fail:
#if 0
	  fprintf(stderr, "Hydra: communication failure from site %d, disconnecting user.\n", link);
	  socket_broken(link);
#else
	/* Do nothing, leave it for the user to sort things out.	*/
#if (debug > 1)
	  fprintf(stderr, "Hydra: communication failure from site %d.\n", link);
#endif	
	return;
#endif
}

static void message_from_server(link)
{
	bool	word_link;
	int	protocol_word;
	MCB	mcb;
	int	size;
	
	if (link_table[link].flags & Link_flags_word)
		word_link	= TRUE;
	else
		word_link	= FALSE;

	if (word_link)
	{
		socket_read(link, big_buffer, 4);
		protocol_word = swap(*((int *) big_buffer));
	}
	else
	{
		socket_read(link, &(big_buffer[3]), 1);
		protocol_word = ((int)big_buffer[3]) & 0x00FF;
	}

	switch(protocol_word)
	{
	case	Proto_RemoteReset :
		reset_processor();
		link_table[link].state	= Link_Reset;
		link_table[link].flags &= ~Link_flags_messagemode;
		break;

	case	Proto_RemoteAnalyse :
		analyse_processor();
		link_table[link].state	= Link_Reset;
		link_table[link].flags &= ~Link_flags_messagemode;
		break;

	case	Proto_Close :
		shutdown(link_table[link].connection, 2);
		close(link_table[link].connection);
		link_table[link].connection = -1;
		free_link(link);
		link_table[link].flags |= (Link_flags_free + Link_flags_uninitialised);
		link_table[link].state = Link_Reset;
		link_table[link].flags &= ~(Link_flags_datareadysent | Link_flags_messagemode);
		return;

	case	Proto_Go :
			/* Half duplex protocol support, already handled by Hydra	*/
		break;

	case	Proto_Msg :
/*{{{  receive message */
		socket_read(link, &(big_buffer[4]), 16);
		{	/* Work out the MCB values.				*/
			int	*x		= (int *) &mcb;
			int	*int_ptr	= (int *) big_buffer;
			*x++ = swap(int_ptr[1]);
			*x++ = swap(int_ptr[2]);
			*x++ = swap(int_ptr[3]);
			*x++ = swap(int_ptr[4]);
		}
#if (debug > 1)
			printf("Hydra: message to root processor on site %d, FnRc %x\n", link, mcb.MsgHdr.FnRc);
			printf("     : dsize %d, csize %d, flags 0x%02x, dest 0x%08x, reply 0x%08x\n",
				mcb.MsgHdr.DataSize, mcb.MsgHdr.ContSize, mcb.MsgHdr.Flags, mcb.MsgHdr.Dest, mcb.MsgHdr.Reply);
#endif
			/* Sort out the locations of the control and data vectors.	*/
		mcb.Control	= (int *) &(big_buffer[20]);
		size		= mcb.MsgHdr.ContSize * 4;
		mcb.Data	= (byte *) &(big_buffer[20 + size]);
		if (size > 0)
		 	socket_read(link, mcb.Control, size);
		size		= mcb.MsgHdr.DataSize;
		if (word_link)
		{
			size += (mcb.MsgHdr.Flags & MsgHdr_Flags_bytealign);
			size  = (size + 3) & ~3;
		}
		socket_read(link, mcb.Data, size);

		size += 16 + (mcb.MsgHdr.ContSize * 4);

		if (word_link)
		{
			if (send_block(size + 4, big_buffer, TIMEOUT_VALUE)) goto fail;
		}
		else
		{
			if (send_block(size + 1,  &(big_buffer[3]), TIMEOUT_VALUE)) goto fail;
		}
/*}}}*/
		break;

	default:
		/* Ignore any junk bytes.				*/
		break;
	}

	return;
fail:
#if 0
	fprintf(stderr, "Hydra: communication failure to site %d, disconnecting user.\n", link);
	socket_broken(link);
#else
	/* Do nothing, leave it for the user to sort things out.	*/
#if (debug > 1)
	  fprintf(stderr, "Hydra: communication failure to site %d.\n", link);
#endif	
	return;
#endif
}

/*}}}*/
/*{{{  debug protocol for use with hydramon */
/**
*** This routine deals with requests coming from a debug connection.
*** The structure debug_msg, described in structs.h, is used for the
*** incoming requests. The structure socket_msg is used for the reply.
*** The requests may be :
***   Close, finish this debugging session
***   Disconnect, abort an I/O Server on a particular link, i.e. change
***    a link from used to free mode
***   Use, try to change a link from unused mode to free mode
***   Free, try to change a link from free to used mode.
**/
PRIVATE void from_debug(no)
int no;
{ debug_msg  msg;
  socket_msg reply;
  int fildes = debug_table[no].fildes;
  int link;

#if (debug  > 0)
  printf("Hydra : message from hydramon\n");
#endif

  if (read(fildes, (BYTE *) &msg, sizeof(debug_msg))
      < sizeof(debug_msg))
   { perror("Hydra : failed to read hydramon request");
     shutdown(fildes, 2);
     close(fildes);
     debug_table[no].flags |= Debugtab_Free;
     return;
   }

  if (swap(msg.fnrc) eq Debug_Close)
   { shutdown(fildes, 2);
     close(fildes);
     debug_table[no].flags |= Debugtab_Free;
     return;
   }

  link = swap(msg.link);
  if ((link < 0) || (link >= number_of_links))
   { reply.fnrc = swap(Invalid_Link);
     if (write(fildes, (BYTE *) &reply, sizeof(socket_msg))
         < sizeof(socket_msg))
      { perror("Hydra : failed to reply to hydramon request");
        shutdown(fildes, 2);
        close(fildes);
        debug_table[no].flags |= Debugtab_Free;
      }
     return;
   }

/**
*** Disconnect : only possible if the link is not free and in use,
*** the processor is reset, the connection is closed which should abort
*** the I/O Server, the link is freed, and the link flags are set.
**/
  if (swap(msg.fnrc) eq Debug_Disconnect)
   {
     if (!(link_table[link].flags & (Link_flags_free | Link_flags_unused) ))
      { current_link = link;
        reset_processor();
        shutdown(link_table[link].connection, 2);
        close(link_table[link].connection);
        link_table[link].connection = -1;
        free_link(link);
        link_table[link].flags |= (Link_flags_free | Link_flags_uninitialised);
        link_table[link].state = Link_Reset;
        link_table[link].connection = -1;
      }
     return;
   }

/**
*** Use request, only possible if the link is currently unused, it
*** involves attempting to open the link. If that fails the link
*** remains unused. Otherwise the link is set to free.
**/
  if (swap(msg.fnrc) eq Debug_Use)
   { if (link_table[link].flags & Link_flags_unused)
      { if (open_link(link))
         { free_link(link);
           link_table[link].flags &= ~(Link_flags_unused |
                                       Link_flags_firsttime);
           link_table[link].flags |= (Link_flags_free |
                                      Link_flags_uninitialised);
           link_table[link].state = Link_Reset;
           link_table[link].connection = -1;
         }
      }
     return;
   }

/**
*** Free : this is only allowed if the link is currently free. If it is
*** unused Free is a no-op, if it is currently in use Free is not allowed.
**/
  if (swap(msg.fnrc) eq Debug_Free)
   { if (link_table[link].flags & Link_flags_unused)
      return;   /* no-op */
     if (!(link_table[link].flags & Link_flags_free))
      return;   /* should disconnect first */
     link_table[link].flags &= ~Link_flags_free;
     link_table[link].flags |= Link_flags_unused;
     return;
   }

/**
*** BLV - addition, 25.2.93, allow hydramon to terminate Hydra
**/
   
  if (swap(msg.fnrc) eq Debug_Exit)
   exit(0);

/**
*** Send back info on the link.
**/
  memcpy((BYTE *) &reply, (BYTE *) &(con_table[link]), sizeof(socket_msg));
  strncpy(reply.linkname, link_table[link].link_name, 31);
  reply.linkname[31]= '\0';
  reply.fnrc = swap(link_table[link].state);
  reply.extra = swap(link_table[link].flags);
  if (write(fildes, (BYTE *) &reply, sizeof(socket_msg))
      < sizeof(socket_msg))
   { perror("Hydra : failed to reply to hydramon request");
     shutdown(fildes, 2);
     close(fildes);
     debug_table[no].flags |= Debug_Free;
   }
}
/*}}}*/
/*{{{  hydra.con handling */

/*------------------------------------------------------------------------
--                                                                      --
-- Configuration file support                                           --
--                                                                      --
------------------------------------------------------------------------*/

/**
*** Here is a copy of the configuration file support, normally held
*** in module server.c. Reading in the configuration file also needs
*** the linked list support, further down. Finally there are some
*** utilities such as swap(), ServerDebug(), and socket I/O routines.
*** All of this is very boring.
**/

                        /* This is the list used to hold the configuration. */
PRIVATE List config_hdr;
typedef struct { Node node;
                 char name[1];         /* space for byte '\0' */
} config;
PRIVATE int config_initialised = 0;

PRIVATE BYTE misc_buffer1[512];

PRIVATE int read_config(config_name)
char *config_name;
{ FILE   *conf;
  register char   *buff = (char *) &(misc_buffer1[0]);
  register int    length;
  config *tempnode;
  char   *temp;
  int    error=0;

  InitList(&config_hdr);

  config_initialised++;                /* There may be memory to be freed */

                                             /* open the configuration file */
  conf = fopen(config_name, "r");
  if (conf eq NULL)
    {
      printf("Hydra : unable to open configuration file %s\r\n", config_name);
      return(0);
    }

                                      /* and read it in, one line at a time */
  while(fgets(buff, 255, conf) ne NULL)
     { while (isspace(*buff)) buff++;
       length = strlen(buff);
                        /* fgets() leaves a newline character in the buffer */
       if ((buff[length-1] eq '\n') || (buff[length-1] eq '\r'))
         buff[--length] = '\0';
       if (length eq 0) continue;     /* blank line */
       if (buff[0] eq '#') continue;  /* comment */

       tempnode = (config *) malloc(sizeof(config) + length);
       if (tempnode eq (config *) NULL)
         { printf("Hydra : insufficient memory on host machine for configuration.\r\n");
           fclose(conf);
           return(0); 
         }
       strcpy(tempnode->name, buff);
       AddTail((Node *) tempnode, &config_hdr);
       buff = &(misc_buffer1[0]);
     }

  fclose(conf);

  temp = get_config("HOST");           /* There must be an entry for HOST */
  if (temp eq NULL)
    { printf("Hydra : error in configuration file, missing entry for HOST\r\n");
      error = 1;
    }

  temp = get_config("BOX");                         /* And an entry for BOX */
  if (temp eq NULL)
    { printf("Hydra : error in configuration file : missing entry for BOX.\r\n");
      error = 1;
    }

  if (error)
    { printf("\nPlease edit the configuration file %s.\r\n", config_name);
      return(0); 
    }

  return(1);
}

PRIVATE void tidy_config()
{ if (config_initialised) FreeList(&config_hdr);
}

        /* The following code allows the server to examine the   */
        /* configuration file. For example, to get the entry for */
        /* HOST, use a call get_config("host"). If there is      */
        /* an entry HOST=XXX get_config() returns a pointer to   */
        /* XXX. The function is fairly easy to implement using a */
        /* list Wander.                                          */
        /* If no entry is found the routine returns NULL.        */
PRIVATE char *test_name(node, name)
config *node;
char   *name;
{ register char *config_name = &(node->name[0]);

                                  /* compare name and config_name */
  while ((*name ne '\0') && (*config_name ne '\0'))
   { if ( ToUpper(*name) ne ToUpper(*config_name) )
       return(FALSE);
     name++; config_name++;
   }
                    /* succeeded ? */
  while (isspace(*config_name)) config_name++;

  if ((*name eq '\0') && (*config_name eq '\0')) return(config_name);

  if ((*name eq '\0') && (*config_name++ eq '='))
    {   while (isspace(*config_name)) config_name++;
        return(config_name);
    }
  else
    return(FALSE);
}

char *get_config(str)
char *str;
{ word result = Wander(&config_hdr, (WordFnPtr) func(test_name), str);
  if (result eq FALSE)
    return(NULL);
  else
    return((char *) result);
}

WORD get_int_config(str)
char *str;
{ register char *result = get_config(str);
  WORD base = 10L, value = 0L, sign = 1L;
  WORD temp;

  if (result eq NULL)
    return(Invalid_config);

  while (isspace(*result)) result++;
  if (*result eq '-') { sign = -1L; result++; }
  if (*result eq '0')
    { result++;
      if (*result eq 'x' || *result eq 'X')
        { result++; base = 16L; }
      else
       base = 8L;
    }
  if (*result eq '-') { sign = -1L; result++; }


#if (UNIX || PC)
  while (isxdigit(*result))    /* Use this routine if it is available */
#else
  while ( isdigit(*result) || ('a' <= *result && *result <= 'f') ||
          ('A' <= *result && *result <= 'F'))
#endif
   { 
     switch (*result)
      { case '0' : case '1' : case '2' : case '3' : case '4' :
        case '5' : case '6' : case '7' : case '8' : case '9' :
                      temp = (WORD) (*result - '0'); break;

        case 'a' : case 'b' : case 'c' : case 'd' : case 'e' :
        case 'f' :    temp = 10L + (WORD) (*result - 'a'); break;

        case 'A' : case 'B' : case 'C' : case 'D' : case 'E' :
        case 'F' :    temp = 10L + (WORD) (*result - 'A'); break;
      }

     if (temp >= base) break;
     value = (base * value) + temp;
     result++;
   }

 return(value * sign);
}
/*}}}*/
/*{{{  utilities */

/*------------------------------------------------------------------------
--                                                                      --
--  Lists                                                               --
--                                                                      --
--      Usual linked list library, but containing only the              --
--      functions which I can be bothered to use.                       --
------------------------------------------------------------------------*/

/* create and initialise a list header, returning 0 to indicate failure */
List *MakeHeader()
{ List *newptr;

  newptr = (List *) malloc( sizeof(List) );
  if (newptr ne NULL)
    InitList(newptr);

  return(newptr);
}

/* initialise a list header to the empty list */
void InitList(listptr)
List *listptr;
{ listptr->head  = (Node *) &(listptr->earth);
  listptr->earth = NULL;
  listptr->tail  = (Node *) listptr;
} 

/* add a node to the beginning of the list */
Node *AddHead(node, header)
Node *node;
List *header;
{ node->next = header->head;
  node->prev = (Node *)header;
  header->head = node;
  (node->next)->prev = node;
  return(node);
}

Node *AddTail(node, header)
Node *node;
List *header;
{ node->prev = header->tail;
  header->tail = node;
  node->next = (Node *) &(header->earth);
  (node->prev)->next = node;
  return(node);
}

Node *listRemove(node)
Node *node;
{ (node->prev)->next = node->next;
  (node->next)->prev = node->prev;
  return(node);
}

Node *NextNode(node)
Node *node;
{ return(node->next);
}

word TstList(header)
List *header;
{ if ((header->head)->next eq NULL) return(FALSE);
  return(TRUE);
}

#if (ANSI_prototypes && !AMIGA)
#if (RS6000 || HP9000)
void WalkList(List *list, VoidFnPtr fun, ...)
#else
void WalkList(list, fun, ...)
List *list;
VoidFnPtr fun;
#endif
{ Node *node, *node2;
  WORD arg1, arg2;
  va_list args;

  va_start(args, fun);
  arg1 = va_arg(args, WORD);
  arg2 = va_arg(args, WORD);
  va_end(args);

  for (node=list->head; node->next ne NULL; node=node2)
    { node2 = node->next;
      (*fun)(node, arg1, arg2);
    }
}

#if (RS6000 || HP9000)
WORD Wander(List *list, WordFnPtr fun, ...)
#else
word Wander(list, fun, ...)
List *list;
WordFnPtr fun;
#endif
{ Node *node, *node2;
  word result, arg1, arg2;
  va_list args;

  va_start(args, fun);
  arg1 = va_arg(args, WORD);
  arg2 = va_arg(args, WORD);
  va_end(args);

  result = FALSE;

  for (node=list->head; (node->next ne NULL) && !result;
       node = node2)
    { node2 = node->next;
      result = (*fun)(node, arg1, arg2);
    }

  return(result);
}
#else
void WalkList(list, fun, arg1, arg2)
List *list;
VoidFnPtr fun;
WORD arg1, arg2;
{ Node *node, *node2;

  for (node=list->head; node->next ne NULL; node=node2)
    { node2 = node->next;
      (*fun)(node, arg1, arg2);
    }
}

word Wander(list, fun, arg1, arg2)
List *list;
WordFnPtr fun;
WORD arg1, arg2;
{ Node *node, *node2;
  word result;
  result = FALSE;

  for (node=list->head; (node->next ne NULL) && !result;
       node = node2)
    { node2 = node->next;
      result = (*fun)(node, arg1, arg2);
    }

  return(result);
}

#endif /* ANSI_prototypes */

                /* This routine just frees every node in the list */
void FreeList(list)
List *list;
{ Node *node, *node2;

  for (node = list->head; node->next ne NULL; node = node2)
    { node2 = node->next;
      free(Remove(node));
    }
}

                          /* this routine is needed by the debugger */
void PreInsert(next, node)
Node *next, *node;
{ node->next = next;
  node->prev = next->prev;
  next->prev = node;
  node->prev->next = node;
}

          /* This routine is used to maintain the order of nodes in WaitingCo */
          /* It puts the node just after the list node given                  */
void PostInsert(node, before)
Node *node, *before;
{ node->next       = before->next;
  node->prev       = before;
  before->next     = node;
  node->next->prev = node;
}

          /* This routine returns the number of nodes in a list. */
WORD ListLength(header)
List *header;
{ Node *node;
  WORD result = 0L;

  for (node = header->head; node->next ne (Node *) NULL; node = node->next)
    result++;

  return(result);
}


/*-------------------------------------------------------------------------
--                                                                       --
--   The usual utilities                                                 --
--                                                                       --
-------------------------------------------------------------------------*/

int ToUpper(x)
int x;
{ return(islower(x) ? toupper(x) : x);
}

word mystrcmp(ms1, ms2)
char *ms1, *ms2;
{ register char *s1 = ms1, *s2 = ms2; 
  for (;;)
   { if (*s1 eq '\0')
       return((*s2 eq '\0') ? 0L : -1L);
     elif (*s2 eq '\0')
       return(1L);
     elif(ToUpper(*s1) < ToUpper(*s2))
       return(-1L);
     elif(ToUpper(*s1) > ToUpper(*s2))
       return(1L);
     else
       { s1++; s2++; }
   }
}

#if swapping_needed
#if (ST || TRIPOS || SUN3 || SUN4 || SM90 || TR5 || RS6000 || HP9000)
                  /* for the 68000's + SPARC's byte ordering */
                  /* on the Amiga it is written in assembler */
word swap(a)
word a;
{ register word b = 0;
  int i;

  if (a eq 0L) return(0L);

  for (i=0; i<4; i++)
    { b <<= 8; b |= (a & 0xFF); a >>= 8; }

  return(b);
}
#endif     /* 68000 + SPARC */
#endif

#if ANSI_prototypes

#if (RS6000 || HP9000)
void ServerDebug(char *text, ...)
#else
void ServerDebug(text, ...)
char *text;
#endif
{
  va_list args;

  va_start(args, text);

  printf("Hydra : ");
  vprintf(text, args);

  va_end(args);
}

#else

void ServerDebug(text, a, b, c, d, e)
char *text;
int a, b, c, d, e;
{ printf("Hydra : ");
  printf(text, a, b, c, d, e); putchar('\n');
}

#endif /* ANSI_prototypes */

/**
*** writing to and reading from sockets. I know at all times how much data to
*** read from a socket, and the other side knows how much I am writing. Hence
*** the following code guarantees delivery of that amount of data,
*** and who cares about the socket's buffering.
**/

void socket_broken(link)
int link;
{ current_link = link;
  reset_processor();
  free_link(link);
  link_table[link].flags |= Link_flags_free;
  link_table[link].state =  Link_Reset;
  close(link_table[link].connection); 
  link_table[link].connection = -1;
  printf("Hydra : I/O Server on link %d has been disconnected.\n", link);
  longjmp(again, 1);
}

void socket_write(link, buf, amount)
int link;
BYTE *buf;
int amount;
{ int written = 0;
  int signals = 0;
  int fildes = link_table[link].connection;
  
  while (written < amount)
   { int x = write(fildes, &(buf[written]), amount - written);
     if (x <= 0)
      { if ((errno eq EINTR) && (signals < 5))
         { signals++; continue; }
        socket_broken(link);
        return;
      }
     written += x;
   }
}

void socket_read(link, buf, amount)
int link;
BYTE *buf;
int  amount;
{ int read_so_far = 0;
  int signals = 0;
  int fildes = link_table[link].connection;
  
  while (read_so_far < amount)
   { int x = read(fildes, &(buf[read_so_far]), amount - read_so_far);
     if (x <= 0)
      { if ((errno eq EINTR) && (signals < 5))
         { signals++; continue; }
        socket_broken(link);
        return;
      }
     read_so_far += x;
   }
}
/*}}}*/

