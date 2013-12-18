/*------------------------------------------------------------------------
--                                                                      --
--         H E L I O S   I N P U T / O U T P U T   S E R V E R          --
--         ---------------------------------------------------          --
--                                                                      --
--         Copyright (C) 1989, Perihelion Software Ltd.                 --
--                       All Rights Reserved.                           --
--                                                                      --
--  hydramon.c                                                          --
--                                                                      --
--  Author:  BLV 8/6/89                                                 --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: hydramon.c,v 1.7 1993/03/23 15:15:47 bart Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.       			*/

/**
*** This program is used in conjunction with the link  daemon Hydra,
*** to provide various debugging options.
**/

#define Demon_Module

#ifdef ARMBSD
#include "helios.h"
#else
#include "../helios.h"
#endif

jmp_buf exit_jmpbuf;
PRIVATE int family_type = AF_UNIX;
PRIVATE char *configname = "hydra.con";
PRIVATE char *socket_name;
PRIVATE int  fn( read_config, (char *));
PRIVATE void fn( tidy_config, (void));
PRIVATE void fn( make_connection, (void));
PRIVATE void fn( main_loop, (void));
int number_of_links = 0;
PRIVATE int my_socket;
int current_link = 0;

/**
*** First, set up a jump buffer to allow abnormal exits. Next check the
*** arguments for something link -C myhydra.con, specifying the
*** configuration file to be used. The default configuration file is
*** hydra.con. The configuration file is read in, using the standard
*** code. Then Hydramon attempts to connect to the link daemon Hydra,
*** to start a debugging session. The debugging session itself is held
*** in main_loop(). Finally there is some tidying to be done.
**/
int main(argc, argv)
int argc;
char *argv[];
{ int i;
  char *curr_arg;

  if (setjmp(exit_jmpbuf)) goto endpoint;

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

  if (!read_config(configname)) goto endpoint;

  make_connection();

  main_loop();

  if (family_type eq AF_UNIX)
    unlink(socket_name);

endpoint:

  tidy_config();

}

/**
*** This is the code to connect to Hydra. It should be similar 
*** to the remote init_link() routine in sun/linklib.c
**/
PRIVATE void make_connection()
{
  char *family_name = get_config("family_name");
  char *socket_name = get_config("socket_name");
  char *host_name   = get_config("hydra_host");
  int family_type = AF_UNIX;
  socket_msg con;

  if (family_name eq (char *) NULL)
   family_name = "AF_UNIX";

  if (!mystrcmp(family_name, "AF_UNIX"))
   family_type = AF_UNIX;
  elif(!mystrcmp(family_name, "AF_INET"))
   family_type = AF_INET;
  else
   { printf("Hydramon : unknown family type %s in configuration file\n",
            family_name);
     longjmp(exit_jmpbuf, 1);
   }

  my_socket = socket(family_type, SOCK_STREAM, 0);
  if (my_socket < 0)
   { perror("Hydramon : failed to create socket");
     longjmp(exit_jmpbuf, 1);
   }

  if (family_type eq AF_UNIX)
   { struct sockaddr_un addr;
     addr.sun_family = AF_UNIX;
     strcpy(addr.sun_path, (socket_name eq (char *) NULL) ?
            "hydra.skt" : socket_name);
     if (connect(my_socket, &addr, 
          strlen(addr.sun_path) + sizeof(addr.sun_family) ) eq -1)
      { perror("Hydramon : failed to connect to socket");
        longjmp(exit_jmpbuf, 1);
      }     
   }
  else
   { struct sockaddr_in addr;
     struct servent     *sp;
     struct hostent     *hp;

     if (host_name eq NULL)
      { printf("Missing entry in hydra.con file for hydra_host.\r\n");
        longjmp(exit_jmpbuf, 1);
      }

     hp = gethostbyname(host_name);
     if (hp eq NULL)
      { printf("Unable to get a network address for %s\r\n", host_name);
        longjmp(exit_jmpbuf, 1);
      }
 
     sp = getservbyname("hydra", "tcp");
     if (sp eq NULL)
      { printf("Hydra has not been allocated a socket port on this machine.\n");
        printf("Please edit the file /etc/services\n");
        longjmp(exit_jmpbuf, 1);
      }

     bzero((char *) &addr, sizeof(struct sockaddr_in));

     bcopy(hp->h_addr, (char *) &addr.sin_addr, hp->h_length);
     addr.sin_family = hp->h_addrtype;
     addr.sin_port   = sp->s_port;
     if (connect(my_socket, &addr, sizeof(struct sockaddr_in)) eq -1)
      { perror("Hydramon : failed to connect to socket");
        longjmp(exit_jmpbuf, 1);
      }
   }

  con.fnrc = swap(Debug_Connection);
  if (write(my_socket, (BYTE *) &con, sizeof(socket_msg))
      < sizeof(socket_msg))
   { perror("Hydramon : failed to write to Hydra");
     longjmp(exit_jmpbuf, 1);
   }

  if (read(my_socket, (BYTE *) &con, sizeof(socket_msg))
      < sizeof(socket_msg))
   { perror("Hydramon : failed to read reply from Hydra");
     longjmp(exit_jmpbuf, 1);
   }

  if (swap(con.fnrc) eq Link_Unavailable)
   { printf("Hydra has no spare connections.\r\n");
     longjmp(exit_jmpbuf, 1);
   }
  number_of_links = swap(con.extra);
}

/**
*** This is the main link. Every time around the loop it obtains
*** information about the current link from Hydra, displays it, and
*** asks for a command. Links can be free, unused, or owned by somebody.
*** If owned, the link can be reset, booting, or running, and the name
*** and location of the owner should be displayed. Then a little menu
*** is printed out, a request is accepted and acted upon, and we are
*** back to the start of the loop.
**/

PRIVATE void fn( do_help, (void));

PRIVATE void main_loop()
{ debug_msg msg;
  socket_msg reply;
  int flags, state;
  int x;

  forever
   { msg.fnrc = swap(Debug_Info);
     msg.link = swap(current_link);
     if (write(my_socket, (BYTE *) &msg, sizeof(debug_msg))
         < sizeof(debug_msg))
      { perror("Hydramon : failed to write to hydra");
        longjmp(exit_jmpbuf, 1);
      }
     if (read(my_socket, (BYTE *) &reply, sizeof(socket_msg))
         < sizeof(socket_msg))
      { perror("Hydramon : failed to read hydra reply");
        longjmp(exit_jmpbuf, 1);
      }
     if (swap(reply.fnrc) eq Invalid_Link)
      { current_link = (current_link > 0) ? current_link - 1 : 0;
        continue;
      }

     printf("Site %d (%s) : ", current_link, reply.linkname);
     flags = swap(reply.extra); state = swap(reply.fnrc);
     if (flags & Link_flags_free)
      printf("free");
     elif (flags & Link_flags_unused)
      printf("unused");
     else
      { if (state eq Link_Reset)
         printf("reset");
        elif (state eq Link_Booting)
         printf("booting");
        else
         printf("running");
        printf(", owned by %s @ %s", reply.userid, reply.hostname);
      }
     printf("\n(Top, Bottom, Next, Prev, Disconnect, Use, Free, Help, Quit) ? ");
     fflush(stdout);
     x = getchar();

     switch(x)
      { case 't' :
        case 'T' : current_link = 0; break;
        case 'b' :
        case 'B' : current_link = number_of_links - 1; break;
	case 'n' :
        case 'N' : current_link = (current_link < (number_of_links - 1)) ?
                     current_link + 1 : current_link;
		   break;
	case 'p' :
	case 'P' : current_link = (current_link > 0) ? current_link - 1 : 0;
		   break;
	case 'q' :
	case 'Q' : { msg.fnrc = swap(Debug_Close);
	             write(my_socket, (BYTE *) &msg, sizeof(debug_msg));
	             longjmp(exit_jmpbuf, 1);
      		   }

	case '\\' :{ msg.fnrc = swap(Debug_Exit);
	             write(my_socket, (BYTE *) &msg, sizeof(debug_msg));
	             longjmp(exit_jmpbuf, 1);
      		   }
	case 'd' :
	case 'D' : { msg.fnrc = swap(Debug_Disconnect);
	             msg.link = swap(current_link);
                     if (write(my_socket, (BYTE *) &msg, sizeof(debug_msg))
                         < sizeof(debug_msg))
                      { perror("Hydramon : failed to write to hydra");
                        longjmp(exit_jmpbuf, 1);
           	      }
      		     break;
                   }
	case 'f' :
	case 'F' : { msg.fnrc = swap(Debug_Free); 
	     	     msg.link = swap(current_link);
                     if (write(my_socket, (BYTE *) &msg, sizeof(debug_msg))
                         < sizeof(debug_msg))
                      { perror("Hydramon : failed to write to hydra");
                        longjmp(exit_jmpbuf, 1);
                      }
                     break;
                   }
	case 'u' :
	case 'U' : { msg.fnrc = swap(Debug_Use);
                     msg.link = swap(current_link);
                     if (write(my_socket, (BYTE *) &msg, sizeof(debug_msg))
                      < sizeof(debug_msg))
                      { perror("Hydramon : failed to write to hydra");
                        longjmp(exit_jmpbuf, 1);
                      }
                     break;
                   }
        case 'h' :
	case 'H' : do_help(); break;
      } 
    for ( ; x ne '\n'; x = getchar());
  }
}

PRIVATE char *text1 = "\n\
Hydramon is a utility program that interacts with the Helios link daemon\n\
Hydra. This link daemon allows access to transputer links from remote\n\
hosts. At any one time Hydra has control over one or more links, some\n\
of which can be in use, while others are free or not accessible.\n\
\n\
Hydramon displays the current state of one of the links controlled by\n\
Hydra. It is possible to step through the various links one by one\n\
with the n(ext) and p(revious) commands. Alternatively, the t(op) command\n\
moves immediately to the first link, and the b(ottom) command moves to\n\
the last link.\n\
\n\
A link can be in one of three states. It can be FREE, which means that the\n\
link is accessible via Hydra and nobody is using it at the moment. It can\n\
be UNUSED, which means that the link is not currently accessible via Hydra\n\
and nobody is using it. Alternatively it can be in use by somebody, in\n\
which case the user's name and location will be shown.\n\
\n\
There are a number of reasons why a link might be UNUSED. First, it may not\n\
be listed in the hydra.con configuration file as a useable link. Second,\n\
Hydra may have been unable to access it when it started up, probably\n\
indicating an error somewhere in the configuration. Third, the link may\n\
have been changed from FREE to UNUSED with this program. A link listed\n\
as FREE may not actually be useable at present. Some application may be\n\
accessing the link directly rather than going via the link daemon Hydra\n\
and this will not be detected until Hydra attempts to use that link on\n\
behalf of some application.\n\
";

PRIVATE char *text2 = "\n\
\n\
The commands d(isconnect), f(ree), and u(se) may be used to change the\n\
state of the current link. D(isconnect) aborts a session on the current\n\
link, changing it from in-use to free. The action of d(isconnect) is\n\
serious, and may involve loss of data. The f(ree) command takes a FREE link\n\
and changes it to UNUSED. This means that Hydra will no longer accept\n\
requests to use that link. The u(se) command has the opposite effect\n\
and attempts to change an UNUSED link to a FREE one.\n\
\n\
";


PRIVATE void do_help()
{ FILE *pipe = popen("more", "w");

  if (pipe eq (FILE *) NULL)
   { printf("Failed to start up more filter\n");
     return;
   }

  fputs(text1, pipe);
  fputs(text2, pipe);

  fflush(pipe);

  pclose(pipe);
}

/*------------------------------------------------------------------------
--                                                                      --
-- Configuration file support                                           --
--                                                                      --
------------------------------------------------------------------------*/


                        /* This is the list used to hold the configuration. */
PRIVATE List config_hdr;
typedef struct { Node node;
                 char name[1];         /* space for byte '\0' */
} config;
PRIVATE int config_initialised = 0;

BYTE misc_buffer1[512];

PRIVATE int read_config(config_name)
char *config_name;
{ FILE   *conf;
  register char   *buff = (char *) &(misc_buffer1[0]);
  register int    length;
  config *tempnode;
  int    error=0;

  InitList(&config_hdr);

  config_initialised++;                /* There may be memory to be freed */

                                             /* open the configuration file */
  conf = fopen(config_name, "r");
  if (conf eq NULL)
    {
      printf("Hydramon : unable to open configuration file %s\r\n",
             config_name);
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
         { printf(
       "Hydramon : insufficient memory on host machine for configuration.\r\n");
           fclose(conf);
           return(0); 
         }
       strcpy(tempnode->name, buff);
       AddTail((Node *) tempnode, &config_hdr);
       buff = &(misc_buffer1[0]);
     }

  fclose(conf);

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
#if (ST || TRIPOS || SUN3 || SUN4 || SM90 || TR5 || RS6000 | HP9000)
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

