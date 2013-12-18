/* $Header: /hsrc/cmds/hostutil/R140/RCS/linktest.c,v 1.3 1991/03/20 13:19:51 paul Exp $ */
/* $Source: /hsrc/cmds/hostutil/R140/RCS/linktest.c,v $ */
/************************************************************************/
/* Test program for transputer link adapter driver.			*/
/* Also used as terminal interface to Active Book prototype boards.	*/
/*									*/
/* Author: Brian Knight, April 1989					*/
/*									*/
/* (C) Copyright 1989, Active Book Company Ltd., Cambridge, England.	*/
/************************************************************************/

/*
 * $Log: linktest.c,v $
 * Revision 1.3  1991/03/20  13:19:51  paul
 * new improved version
 *
 * Revision 1.2  90/04/04  09:09:08  brian
 * Added buffering for both input and output to term_com when the input is
 * not a terminal. Now goes about 50 times faster!
 * 
 * Revision 1.1  90/04/04  08:19:38  brian
 * Initial revision
 * 
 */

#include <dev/link.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sgtty.h>
#include <signal.h>


#define MAXCOM      25 		/* Max number of commands */
#define DEVICE      "/dev/link"
#define READFILECH  '*'		/* * causes input to come from file */
#define BINREADCH   '#'		/* # causes binary input to come from file */
#define NL          10
#define NAMEBUFSIZE 256
#define CTRLD       4
#define TERMBUFSIZE 20*1024	/* Max output buffer size in term_com() */

struct command
{
  char *name;
  int  (*proc)();
} command_table[MAXCOM];


static int ncommands = 0, finished = 0;
static int link;

int (*lookup_command())();
int tidyup();

main(argc, argv)
  int  argc;
  char **argv;
{
  char  devname[256];
  build_command_table();

  /* Argument (if present) should be "0", "1", etc to select specific device */
  strcpy(devname, DEVICE);
  if (argc > 1) strcat(devname, argv[1]);
  link = open(devname, O_RDWR, 0);
  if (link < 0)
  {
    perror("Open failed");
    printf("Can't open %s\n", devname);
    exit(1); 
  }
  
  /* Install signal handler to reset terminal etc. on exit */
  signal(SIGINT, tidyup);
    
  while (!finished)
  {
    char line[256];
    char com[256], a1[256], a2[256], a3[256];
    int nwords;
    
    printf("> "); fflush(stdout);
    gets(line);
    com[0] = 0; a1[0] = 0; a2[0] = 0; a3[0] = 0;
    nwords = sscanf(line, "%s %s %s %s", com, a1, a2, a3);

    if (nwords > 0)
    {
      int (*proc)() = lookup_command(com);
      
      if ((int)proc) 
        (*proc)(com, a1, a2, a3);
      else
        printf("Unknown command '%s'\n", com);
    }
  }
}


tidyup()
{
  cooked_input();
  exit(1);
}


/* Get input characters as soon as they are typed, without echo */
raw_input()
{
  struct sgttyb sgttyb;
  
  ioctl(0, TIOCGETP, &sgttyb);
  sgttyb.sg_flags |= CBREAK; /* Get each char as soon as typed */
  sgttyb.sg_flags &= ~ECHO;
  ioctl(0, TIOCSETP, &sgttyb);
}

/* Turn on input echo and line buffering */
cooked_input()
{
  struct sgttyb sgttyb;
  
  ioctl(0, TIOCGETP, &sgttyb);
  sgttyb.sg_flags &= ~CBREAK; 
  sgttyb.sg_flags |= ECHO;
  ioctl(0, TIOCSETP, &sgttyb);
}


add_command(name, proc)
  char *name;
  int  (*proc)();
{
  struct command *next;
  
  if (ncommands >= MAXCOM)
  {
    printf("Too many commands (%d)\n", MAXCOM);
    exit(1);
  }
  
  next = &command_table[ncommands++];
  next->name = name;
  next->proc = proc;
}


int (*lookup_command(name))()
  char *name;
{
  int i;
  
  for (i=0; i<ncommands; i++)
    if (strcmp(name, command_table[i].name) == 0)
      return command_table[i].proc;
      
  return 0;
}

/*********************************************************************/
/* Command procs						     */
/*********************************************************************/

fast_com()     { do_ioctl(LIOSPEED20); }
slow_com()     { do_ioctl(LIOSPEED10); }
slreset_com()  { do_ioctl(LIOSLRESET); }
clreset_com()  { do_ioctl(LIOCLRESET); }
sanalyse_com() { do_ioctl(LIOSANALYSE); }
canalyse_com() { do_ioctl(LIOCANALYSE); }
chipreset_com(){ do_ioctl(LIOCHIPRESET); }

reset_com()    
{ 
  int i;

  do_ioctl(LIOSLRESET);
  for (i=0; i<100000; ++i); /* Wait a while */
  do_ioctl(LIOCLRESET); 
}

do_ioctl(request)
{
  int res = ioctl(link, request, 0);
  
  if (res) perror("ioctl failed");
}

fork_com() { fork(); }    

help_com()
{
  int i;
  
  printf("Linktest commands are:\n");
  
  for (i=0; i<ncommands; i++)
    printf(" %s", command_table[i].name);
  printf("\n");
}

status_com()
{
  int s = 0;
  int res = ioctl(link, LIOSTATUS, &s);
  
  if (res == 0)
  {
    printf("status: speed %dMHz", ((s & LS_LINKSPEED20) ? 20 : 10));
    if (s & LS_DATAPRESENT)       printf(", data present");
    if (s & LS_OUTPUTREADY)       printf(", output ready");
    if ((s & LS_NOTERRORIN) == 0) printf(", ErrorIn");
    if (s & LS_RESETOUT)          printf(", ResetOut");
    if (s & LS_ANALYSEOUT)        printf(", AnalyseOut");
    if (s & LS_READINTEN)         printf(", read int enabled");
    if (s & LS_WRITEINTEN)        printf(", write int enabled");
    printf("\n");
  }
  else
    perror("ioctl failed");
}

selectr_com()
{
  struct timeval timeout;
  fd_set rfds;
  int nfound;
  
  timeout.tv_sec = 60;  timeout.tv_usec = 0;
  FD_ZERO(&rfds); FD_SET(link, &rfds);
  
  nfound = select(link+1, &rfds, 0, 0, &timeout);
  printf("select returned %d\n", nfound);
}


selectw_com()
{
  struct timeval timeout;
  fd_set wfds;
  int nfound;
  
  timeout.tv_sec = 60;  timeout.tv_usec = 0;
  FD_ZERO(&wfds); FD_SET(link, &wfds);
  
  nfound = select(link+1, 0, &wfds, 0, &timeout);
  printf("select returned %d\n", nfound);
}


quit_com()
{
  finished = 1;
}


read_com()
{
#define READSIZE 10
  char buf[READSIZE];
  int res = read(link, &buf[0], READSIZE);
  int i;
  
  printf("read returned %d:", res);
  for (i=0; i<res; ++i)
    safewrch(buf[i]);
  printf("\n");
}

safewrch(c)
  char c;
{
  if ((c >= ' ') && (c < 127))
    printf("%c", c);
  else
    printf("[%02x]", c);
}


write_com(com, a1)
  char *com, *a1;
{
  char *message = (a1[0] ? a1 : "hello");
  int res = write(link, message, strlen(message));
  printf("write returned %d\n", res);
}


/* Send all input to link, collect all output to screen */
term_com()
{
  int pid;
  printf("Type Control-D to get back to command mode\n") ;
  printf("*name to read a file as TTY input\n") ;
  printf("#name to read a file as pure binary\n") ;

  pid = fork() ;
  
  if (pid)
  { 
    term_input();       /* Returns when user types EXITCH or ESC */
    kill(pid, SIGKILL); /* Abolish output process */
    printf("\n");
  }
  else
    term_output(); /* Child: killed by parent, so never exits itself */
}


term_input()
{
  raw_input();
  streamtolink(stdin);
  cooked_input();
}


/* Copies the given stream to the link. Returns at EOF.              */
/* If READFILECH is encountered, then the characters between it and  */
/* end-of-line are taken as a filename, and that file is inserted in */
/* the input stream.                                                 */
/* If BINREADCH is encountered, then the characters between it and   */
/* end-of-line are taken as a filename, and that file is read as a   */
/* true binary stream.						     */

streamtolink(inStr)
  FILE *inStr;
{
  int	        tty = isatty(fileno(inStr));
  int           bufSize = (tty ? 1 : TERMBUFSIZE); /* Buffer output if input */
                                                   /* is not a terminal      */
  unsigned char buf[TERMBUFSIZE]; /* Buffer for output characters */
  int 		pos = 0; 	  /* Position in output buffer */

  while (1)
  {
    int ch = fgetc(inStr);

    if (ch == EOF)   break; /* end of file */
    if (ch == CTRLD) break; /* Control-D in raw mode */

    if (ch == READFILECH)
    {
      char name[NAMEBUFSIZE];
      int i = 0;
      FILE *newFile;
      if (tty) write(1, &ch, 1); /* reflect this char */
      cooked_input(); /* Enable echo and line editing */
      
      while (i < (NAMEBUFSIZE-1))
      {
        ch = fgetc(inStr);
        if (ch == EOF) break;
        if (ch == NL) break;
      	if (ch != ' ')
      	  name[i++] = ch;

      }
      name[i] = 0;
      raw_input();
      
      if (i > 0)
      {
        newFile = fopen(name, "rb");
        if (newFile != 0)
        {
      	  printf("Reading from '%s'\n", name);
      	  streamtolink(newFile);
      	  fclose(newFile);
      	  printf("\nEOF\n");
        }
        else
          printf("Can't open '%s'\n", name);
      }
    }

    if (ch == BINREADCH)
     {
      char  name[NAMEBUFSIZE] ;
      int   i = 0 ;
      FILE *newFile ;

      if (tty)
       write(1,&ch,1) ; /* reflect this char */
      cooked_input() ;  /* Enable echo and line editing */
      
      while (i < (NAMEBUFSIZE-1))
       {
        ch = fgetc(inStr) ;
        if (ch == EOF)
         break ;
        if (ch == NL)
         break ;
      	if (ch != ' ')
      	 name[i++] = ch ;
       }
      name[i] = 0 ;
      raw_input() ;
      
      if (i > 0)
       {
        newFile = fopen(name,"rb") ;
        if (newFile != 0)
         {
          unsigned char buffer[TERMBUFSIZE] ;
          int 		buffpos = 0 ;
	  char		chr ;

          printf("Reading binary from \"%s\"\n",name) ;

	  while (fread(&chr,1,1,newFile) == 1)
	   {
	    buffer[buffpos++] = chr ;
	    if (buffpos >= TERMBUFSIZE) 
             {
              write(link,buffer,buffpos) ; /* flush the buffer */
              buffpos = 0 ;
             }
           }

          /* flush any remaining output */
          if (buffpos)
           write(link,buffer,buffpos) ;

      	  fclose(newFile);
      	  printf("\nBINARY DOWNLOAD COMPLETED\n");
         }
        else
         printf("Can't open \"%s\"\n",name) ;
       }
     }

    /* Buffer the output to increase speed through the link driver 	*/
    /* For terminal input `bufSize' is 1 so there is no buffering.	*/

    buf[pos++] = ch;
    if (pos >= bufSize) 
    {
      write(link, buf, pos); /* Flush the buffer */
      pos = 0;
    }
  }

  if (pos) write(link, buf, pos);  /* Flush any remaining output */
}


term_output()
{
  unsigned char ch;
  
  while (1)
  {
    if (read(link, &ch, 1) == 1)
     {
      if (((ch < ' ') && (ch != 10) && (ch != 13)) || (ch == 0x7F))
	ch = '.' ;
      write(1, &ch, 1);
     }
  }
}


/* Set a new read/write timeout, and print the old value */
timeout_com(com, a1)
  char *com, *a1;
{
  int res, timeout = -1;
  
  if (a1[0]) timeout = strtonum(a1);
  if (timeout < 0) timeout = 2000; /* A good default value */
  
  res = ioctl(link, LIOSETTIMEOUT, &timeout);
  
  if (res == 0)
    printf("old timeout was %d ms\n", timeout);
  else
    perror("ioctl failed");
}


/* Convert a string to a number */
strtonum(s)
  char *s;
{
  int i, v = 0;
  
  for (i=0;; ++i)
  {
    int d;
    unsigned char ch = s[i];
    
    if (ch == 0) break;

    if (('0' <= ch) && (ch <= '9'))
      d = ch - '0';
    else
    {
      printf("'%s' is not a number\n", s);
      v = -1;
      break;
    }
    
    v = v*10 + d;
  }
  
  return v;
}


build_command_table()
{
  add_command("help",	  help_com);
  add_command("status",	  status_com);
  add_command("q",	  quit_com);
  add_command("fast",	  fast_com);
  add_command("slow",	  slow_com);
  add_command("fork",     fork_com);
  add_command("slreset",  slreset_com);
  add_command("clreset",  clreset_com);
  add_command("reset",	  reset_com);
  add_command("sanalyse", sanalyse_com);
  add_command("canalyse", canalyse_com);
  add_command("chipreset",chipreset_com);
  add_command("selectr",  selectr_com);
  add_command("selectw",  selectw_com);
  add_command("read",	  read_com);
  add_command("write",	  write_com);
  add_command("term",	  term_com);
  add_command("timeout",  timeout_com);
}
