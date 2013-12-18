/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      linklib.c                                                       --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90\ Copyright (C) 1989, Perihelion Software Ltd.        */

/**
*** This is generic Link I/O Code for all the different unix boxes and
*** transputer systems. It is used inside the I/O Server and inside the
*** Link Daemon.
***
*** Essentially, two different types of link I/O can be supported using
*** this code. The first option is to make use of the Inmos Iserver
*** link I/O code. This is used for the Inmos B011, B014, and B016 boards
*** on a Sun3 or Sun4, for the Parsytec Sun boards, and for Bleistein's
*** Unix386 system. If you already have the Iserver up and running, this
*** is probably the simplest way to get the link I/O code up and
*** runing. The alternative approach is to interact with a standard device
*** driver. This is used for Kpar's Sun386 device driver, the Transtech
*** board, the Telmat itftp32 boards, and the Gnome board for the Acorn
*** R140 workstation.
***
*** In addition, interaction with the link daemon hydra is similar to
*** using a device driver.
***
*** In accordance with the theory that all problems in computer science
*** can be solved by another level of indirection, access to the link
*** goes through various function pointers. In particular, the module
*** linkio.c uses the pointers rdrdy_fn, wrrdy_fn, byte_to_link_fn,
*** byte_from_link_fn, send_block_fn, fetch_block_fn, reset_fn, and
*** analyse_fn to implement the higher levels of link I/O code.
*** According to the specification of the linkio module, there must be
*** a hardware-specific resetlnk() routine responsible for initialising these
*** pointers to sensible values. resetlnk() may be found further down in
*** this code. In the I/O Server resetlnk() must do an OpenLink() or
*** equivalent. In the link daemon hydra resetlnk() must open all the
*** links.
***
*** In addition to the link I/O pointers needed by linkio.c, there are three
*** other routines needed on Unix machines. It has taken a very long time
*** to figure out what these routines should be, but I think I have got it
*** right this time.
*** 
*** First, the init_link routine. This is called once only, when the
*** Server starts up. Its purpose is to initialise the link_table
*** structure. In particular, it has to work out the maximum number of
*** links that might exist and put this into the variable number_of_links.
*** Then it has to allocate enough space for number_of_links Trans_link
*** structures, and assign this buffer to link_table. Each entry in
*** link_table should be filled in with the name to be used for that name.
*** If Inmos-standard link I/O routines are used, these names are supplied
*** to the OpenLink() routine.
***
*** Example 1 :
***             the Transtech MCP1000 board comes with four link adapters,
*** and there can be up to 8 boards in one Sun. Hence number_of_links should
*** be set to 32. link_table is set to malloc(32 * sizeof(Trans_link)).
*** The links are named nap0, nap1, nap2, etc. Hence link_table[0].link_name
*** is set to nap0, etc.
***
*** Example 2 :
***             the B011 board comes with just one link adapter, and
*** although it is possible to have multiple boards in one Sun I
*** doubt that anybody would bother. The string passed to OpenLink()
*** is actually the address of the board, which gets very nasty.
*** Hence b011_init_link() does not have to change number_of_links (it
*** defaults to 1) and link_table already contains space for 1 Trans_link
*** entry. All it has to do is fill in link_name to an empty string.
***
*** Example 3 :
***             when going via the link daemon there is only one link,
*** the socket to the daemon.
***
*** There are two variables defining which link we are currently using.
*** Transputer_site defines the exact site we are using. current_link
*** defines the slot in link_table. If accessing the link hardware
*** directl current_link == transputer_site. However, if going via the
*** link daemon current_link is always 0, and transputer_site is
*** specified by the daemon.
***
*** The final routines are open_link and free_link. These correspond
*** loosely to the Inmos OpenLink and CloseLink routines, but not
*** exactly. open_link() takes one argument, a table identifier.
*** Except in the special case of a remote link, the table identifier
*** is the same as the transputer site. In the case of a remote link
*** the open_link() routine is a no-op, since the work is done by
*** rem_init_link(). free_link() also takes a single
*** argument, a table identifier corresponding to current_link.
***
*** The open_link function may want to look at two of the flags in
*** the link_table. Link_flags_firsttime is set if, for this program,
*** the open_link function is called for the firsttime.
*** Link_flags_uninitialised is set if the link may have been accessed by
*** some other program. This is important in the link daemon, less so
*** in the Server.
***
*** In addition open_link may want to set the flag
*** Link_flags_not_selectable, if select() cannot be used on the value
*** returned by open_link. This applies to the Inmos standard. The
*** corresponding flag Link_flags_waiting is used only in sun/sunlocal.c
*** The other two flags, Link_flags_free and Link_flags_unused, must
*** NEVER be used by machine-specific code.
***
*** N.B., to support enabling a link to an existing system rather than
*** booting up a system I may have to be less cavalier about resetting
*** processors whenever I feel like it. In particular, the open_link()
*** routine should no longer do a reset, and Hydra should not reset
*** a processor if it receives data from a processor believed to be
*** not-running.
**/

#define Linklib_Module

#if ARMBSD
#include "helios.h"
#else
#include "../helios.h"
#endif

/**
*** When running in the link daemon there is an external integer current_link
*** specifying which of the possible links is currently being used. The
*** macro link_fd is used to access that particular link. It is up to this
*** module to define which of the possible sites is currently being used,
*** by zapping the integer transputer_site. This is used by e.g. the file
*** name translation code to access the right etc and tmp subdirectories.
**/
#define link_fd (link_table[current_link].fildes)
extern  int transputer_site;

/**
*** Here are declarations for the pointers used to indirect to the link
*** I/O routines. The first two are specific to the Unix version. The
*** remainder are actually inside the linkio.c module.
**/
int  (*open_link_fn)();
void (*free_link_fn)();
extern int  (*rdrdy_fn)();
extern int  (*wrrdy_fn)();
extern int  (*byte_to_link_fn)();
extern int  (*byte_from_link_fn)();
extern int  (*send_block_fn)();
extern int  (*fetch_block_fn)();
extern void (*reset_fn)();
extern void (*analyse_fn)();

/**
*** If the link I/O goes through the Inmos compatible standard, there
*** is another level of indirection to allow for different hardware.
*** Currently only applies to Sun3, Sun4 and UNIX386.
**/
#if (SUN3 || SUN4 || UNIX386)
PRIVATE int (*inmos_read_link_fn)();
PRIVATE int (*inmos_write_link_fn)();
PRIVATE int (*inmos_reset_link_fn)();
PRIVATE int (*inmos_analyse_link_fn)();
PRIVATE int (*inmos_test_read_fn)();
PRIVATE int (*inmos_test_write_fn)();
PRIVATE int (*inmos_open_link_fn)();
PRIVATE int (*inmos_close_link_fn)();
PRIVATE void fn( inmos_reset_transputer, (void));
PRIVATE void fn( inmos_analyse_transputer, (void));
PRIVATE int  fn( inmos_rdrdy, (void));
PRIVATE int  fn( inmos_wrrdy, (void));
PRIVATE int  fn( inmos_byte_from_link, (int *));
PRIVATE int  fn( inmos_byte_to_link, (int));
PRIVATE int  fn( inmos_fetch_block, (int, BYTE *, int));
PRIVATE int  fn( inmos_send_block, (int, BYTE *, int));
PRIVATE int  fn( inmos_open_link,  (int));
PRIVATE void fn( inmos_free_link,  (int));
#endif

PRIVATE WORD fn( gen_byte_from_link, (int *));
PRIVATE WORD fn( gen_byte_to_link, (int));
PRIVATE WORD fn( gen_send_block, (int, BYTE *, int));
PRIVATE WORD fn( gen_fetch_block, (int, BYTE *, int));
PRIVATE WORD fn( select_rdrdy, (void));
PRIVATE WORD fn( null_wrrdy, (void));
PRIVATE void fn( rem_reset, (void));
PRIVATE void fn( rem_analyse, (void));
PRIVATE WORD fn( rem_byte_to_link, (void));
PRIVATE WORD fn( rem_byte_from_link, (void));
PRIVATE WORD fn( rem_send_block, (int, BYTE *, int));
PRIVATE WORD fn( rem_fetch_block, (int, BYTE *, int));
PRIVATE void fn( rem_init_link, (void));
PRIVATE void fn( rem_open_link, (int));
PRIVATE void fn( rem_free_link, (int, int));

#if (SUN3 || SUN4)
   /* Hardware supported for these machines is the Transtech MCP1000  */
   /* or Niche NTP1000, depending on your point of view, the Parsytec */
   /* boards, and the various Inmos boards B011, B014, and B016. The  */
   /* Parsytec and Inmos boards are accessed through the Inmos link   */
   /* standard.     */

/* kaha hardware added by IDGG */

extern void fn( kaha_init_link, (void));
extern void fn( kaha_reset_transputer, (void));
extern void fn( kaha_analyse_transputer, (void));
extern int  fn( kaha_open_link,   (int));
extern int  fn( kaha_free_link,   (int));
extern int  fn( kaha_byte_to_link, (int));
extern int  fn( kaha_byte_from_link, (int *));
extern int  fn (kaha_rdrdy, (void));
extern int  fn (kaha_wrrdy, (void));

/* end of kaha */

extern void fn( niche_init_link, (void));
extern void fn( niche_reset_transputer, (void));
extern void fn( niche_analyse_transputer, (void));
extern int  fn( niche_open_link,   (int));
extern int  fn( niche_free_link,   (int));

extern  void fn( b011_init_link,   (void));
extern  int  fn( b011_OpenLink,    (char *));
extern  int  fn( b011_CloseLink,   (int));
extern  int  fn( b011_ReadLink,    (int, char *, unsigned int, int));
extern  int  fn( b011_WriteLink,   (int, char *, unsigned int, int));
extern  int  fn( b011_ResetLink,   (int));
extern  int  fn( b011_AnalyseLink, (int));
extern  int  fn( b011_TestRead,    (int));
extern  int  fn( b011_TestWrite,   (int));

extern  void fn( b014_init_link,   (void));
extern  int  fn( b014_OpenLink,    (char *));
extern  int  fn( b014_CloseLink,   (int));
extern  int  fn( b014_ReadLink,    (int, char *, unsigned int, int));
extern  int  fn( b014_WriteLink,   (int, char *, unsigned int, int));
extern  int  fn( b014_ResetLink,   (int));
extern  int  fn( b014_AnalyseLink, (int));
extern  int  fn( b014_TestRead,    (int));
extern  int  fn( b014_TestWrite,   (int));

extern  void fn( b016_init_link,   (void));
extern  int  fn( b016_OpenLink,    (char *));
extern  int  fn( b016_CloseLink,   (int));
extern  int  fn( b016_ReadLink,    (int, char *, unsigned int, int));
extern  int  fn( b016_WriteLink,   (int, char *, unsigned int, int));
extern  int  fn( b016_ResetLink,   (int));
extern  int  fn( b016_AnalyseLink, (int));
extern  int  fn( b016_TestRead,    (int));
extern  int  fn( b016_TestWrite,   (int));

#ifdef INCL_PARSY
         /* I do not have the Parsytec code at the moment */
extern  void fn( par_init_link,   (void));
extern  int  fn( par_OpenLink,    (char *));
extern  int  fn( par_CloseLink,   (int));
extern  int  fn( par_ReadLink,    (int, char *, unsigned int, int));
extern  int  fn( par_WriteLink,   (int, char *, unsigned int, int));
extern  int  fn( par_ResetLink,   (int));
extern  int  fn( par_AnalyseLink, (int));
extern  int  fn( par_TestRead,    (int));
extern  int  fn( par_TestWrite,   (int));
#endif   /* INCL_PARSY */

#endif   /* SUN3 || SUN4 */

#if SUN386
   /* SUN386 means a B008 board with the K-PAR driver. */
   /* No other hardware at present. */
extern void fn( kpar_init_link,          (void));
extern void fn( kpar_reset_transputer,   (void));
extern void fn( kpar_analyse_transputer, (void));
extern int  fn( kpar_open_link,          (int));
extern void fn( kpar_free_link,          (int));
#endif  /* SUN386 */

#if SM90
     /* The Telmat SM90 box with their own link adapter, itftp32 */
     /* the relevant routines are held in smlink.c */
extern void fn( itftp32_init_link, (void));
extern void fn( itftp32_reset_transputer, (void));
extern void fn( itftp32_analyse_transputer, (void));
extern int  fn( itftp32_open_link, (int));
extern void fn( itftp32_free_link, (int));
#endif

#if ARMBSD
     /* The Gnome link adapter for the R140 unix box */
extern void fn( gnome_init_link, (void));
extern void fn( gnome_reset_transputer, (void));
extern void fn( gnome_analyse_transputer, (void));
extern WORD fn( gnome_rdrdy, (void));
extern WORD fn( gnome_wrrdy, (void));
extern void fn( gnome_open_link, (int));
extern void fn( gnome_free_link, (int));
#endif

#if UNIX386
extern  void fn( brs_init_link, (void));
extern  int  fn( OpenLink,      (char *));
extern  int  fn( CloseLink,     (int));
extern  int  fn( ReadLink,      (int, char *, unsigned int, int));
extern  int  fn( WriteLink,     (int, char *, unsigned int, int));
extern  int  fn( ResetLink,     (int));
extern  int  fn( AnalyseLink,   (int));
extern  int  fn( TestRead,      (int));
extern  int  fn( TestWrite,     (int));
#endif

/**
*** The purpose of resetlnk() is as follows. If the link I/O has already
*** been initialised, do nothing. Otherwise find out what hardware is
*** attached, call the appropriate init routine to set up number_of_links
*** and link_table as described above, and set up the indirection pointers.
*** If running as the I/O Server, open one link. Otherwise initialise the
*** link_table as per the daemon's requirements. This involves calls
*** to the open_link() and free_link() routines.
***
*** host sun3
*** host sun4  : box transtech NTP1000/MCP1000
***              Inmos B011
***              Inmos B014
***              Inmos B016
***              Parsytec who have their own version
*** host sun386 : box IMB, the K-Par device driver
*** host sm90 : box itftp32
*** host arm  : box linkpod
*** host unix386 : box /dev/la, single link only
***
*** In addition, the link may be remote, interacting with the link daemon.
*** In this case interacting with the link involves sockets.
**/

PRIVATE void fn( Server_resetlnk, (void));
PRIVATE void fn( Daemon_resetlnk, (void));

void resetlnk()
{ PRIVATE int first_initialisation = 1;
  char *conf;

  unless(first_initialisation) return;
  first_initialisation = 0;

  conf = get_config("BOX");   /* guaranteed to succeed */

  link_table[0].flags      = Link_flags_unused + Link_flags_uninitialised +
                                  Link_flags_firsttime;
  link_table[0].connection = -1;
  link_table[0].fildes     = -1;
  link_table[0].state      = Link_Reset;
  link_table[0].ready      = 0;

            /* for all unix boxes which use device drivers to access the */
            /* link, the following will work. They are sensible defaults */
  byte_from_link_fn    = func(rem_byte_from_link);
  byte_to_link_fn      = func(rem_byte_to_link);
  send_block_fn        = func(rem_send_block);
  fetch_block_fn       = func(rem_fetch_block);
  reset_fn             = func(rem_reset);
  analyse_fn           = func(rem_analyse);
  rdrdy_fn             = func(select_rdrdy);
  wrrdy_fn             = func(null_wrrdy);

/**
*** The first option, "remote" access via a link daemon, is always
*** compiled in. It will work on all systems. Next, depending on the
*** host machine various other boards are possible. Finally, if the link
*** is not remote nor a known device, an error message is generated. Note
*** that it is possible to have no link device at all, and always go
*** through the link daemon.
**/
  if (!mystrcmp(conf, "remote"))     /* This option is always available   */
   { if (Server_Mode eq Mode_Daemon) /* in the server, never in the daemon */
      { ServerDebug(
         "Configuration entry \"Box = remote\" illegal for the link daemon");
        longjmp(exit_jmpbuf, 1);
      }
     Server_Mode = Mode_Remote; 
     rem_init_link();
     open_link_fn      = func(rem_open_link);
     free_link_fn      = func(rem_free_link);
   }

#if (SUN3 || SUN4)
  elif (!mystrcmp(conf, "KAHA")) /* Waikato Kaha s-bus board*/
           { 
     kaha_init_link();
     open_link_fn      = func(kaha_open_link);
     free_link_fn      = func(kaha_free_link);
     reset_fn          = func(kaha_reset_transputer);
     analyse_fn        = func(kaha_analyse_transputer);
     rdrdy_fn          = func(kaha_rdrdy);
     wrrdy_fn          = func(kaha_wrrdy);
     byte_from_link_fn = func(kaha_byte_from_link);
     byte_to_link_fn   = func(kaha_byte_to_link);
     fetch_block_fn    = func(gen_fetch_block);
     send_block_fn     = func(gen_send_block);
   }
  elif ((!mystrcmp(conf, "NTP1000")) ||  /* niche board */
        (!mystrcmp(conf, "MCP1000")) )
   { 
     niche_init_link();
     open_link_fn      = func(niche_open_link);
     free_link_fn      = func(niche_free_link);
     reset_fn          = func(niche_reset_transputer);
     analyse_fn        = func(niche_analyse_transputer);
     byte_from_link_fn = func(gen_byte_from_link);
     byte_to_link_fn   = func(gen_byte_to_link);
     fetch_block_fn    = func(gen_fetch_block);
     send_block_fn     = func(gen_send_block);
   }
  else if (
#ifdef INCL_PARSY     
        !mystrcmp(conf, "PARSYTEC") ||
#endif
        !mystrcmp(conf, "B011") ||
        !mystrcmp(conf, "B014") ||
        !mystrcmp(conf, "B016") )
   { 
     reset_fn          = func(inmos_reset_transputer);
     analyse_fn        = func(inmos_analyse_transputer);
     rdrdy_fn          = func(inmos_rdrdy);
     wrrdy_fn          = func(inmos_wrrdy);
     byte_from_link_fn = func(inmos_byte_from_link);
     byte_to_link_fn   = func(inmos_byte_to_link);
     send_block_fn     = func(inmos_send_block);
     fetch_block_fn    = func(inmos_fetch_block);
     open_link_fn      = func(inmos_open_link);
     free_link_fn      = func(inmos_free_link);
     
     if (!mystrcmp(conf, "B011"))
      { b011_init_link();
        inmos_open_link_fn    = func(b011_OpenLink);
        inmos_close_link_fn   = func(b011_CloseLink);
        inmos_read_link_fn    = func(b011_ReadLink);
        inmos_write_link_fn   = func(b011_WriteLink);
        inmos_reset_link_fn   = func(b011_ResetLink);
        inmos_analyse_link_fn = func(b011_AnalyseLink);
        inmos_test_read_fn    = func(b011_TestRead);
        inmos_test_write_fn   = func(b011_TestWrite);
      }
     elif (!mystrcmp(conf, "B014"))
      { b014_init_link();
        inmos_open_link_fn    = func(b014_OpenLink);
        inmos_close_link_fn   = func(b014_CloseLink);
        inmos_read_link_fn    = func(b014_ReadLink);
        inmos_write_link_fn   = func(b014_WriteLink);
        inmos_reset_link_fn   = func(b014_ResetLink);
        inmos_analyse_link_fn = func(b014_AnalyseLink);
        inmos_test_read_fn    = func(b014_TestRead);
        inmos_test_write_fn   = func(b014_TestWrite);
      }
     elif (!mystrcmp(conf, "B016"))
      { b016_init_link();
        inmos_open_link_fn    = func(b016_OpenLink);
        inmos_close_link_fn   = func(b016_CloseLink);
        inmos_read_link_fn    = func(b016_ReadLink);
        inmos_write_link_fn   = func(b016_WriteLink);
        inmos_reset_link_fn   = func(b016_ResetLink);
        inmos_analyse_link_fn = func(b016_AnalyseLink);
        inmos_test_read_fn    = func(b016_TestRead);
        inmos_test_write_fn   = func(b016_TestWrite);
      }
#ifdef INCL_PARSY
     elif (!mystrcmp(conf, "PARSYTEC"))
      { par_init_link();
        inmos_open_link_fn    = func(par_OpenLink);
        inmos_close_link_fn   = func(par_CloseLink);
        inmos_read_link_fn    = func(par_ReadLink);
        inmos_write_link_fn   = func(par_WriteLink);
        inmos_reset_link_fn   = func(par_ResetLink);
        inmos_analyse_link_fn = func(par_AnalyseLink);
        inmos_test_read_fn    = func(par_TestRead);
        inmos_test_write_fn   = func(par_TestWrite);
      }
#endif
   }
#endif  /* SUN3 || SUN4 */

#if SUN386
  elif (!mystrcmp(conf, "IMB"))
   { kpar_init_link();
     open_link_fn      = func(kpar_open_link);
     free_link_fn      = func(kpar_free_link);
     reset_fn          = func(kpar_reset_transputer);
     analyse_fn        = func(kpar_analyse_transputer);
     byte_from_link_fn = func(gen_byte_from_link);
     byte_to_link_fn   = func(gen_byte_to_link);
     fetch_block_fn    = func(gen_fetch_block);
     send_block_fn     = func(gen_send_block);
   }
#endif

#if SM90
  elif (!mystrcmp(conf, "ITFTP32"))
   { itftp32_init_link(); 
     open_link_fn      = func(itftp32_open_link);
     free_link_fn      = func(itftp32_free_link);
     reset_fn          = func(itftp32_reset_transputer);
     analyse_fn        = func(itftp32_analyse_transputer);
     byte_from_link_fn = func(gen_byte_from_link);
     byte_to_link_fn   = func(gen_byte_to_link);
     fetch_block_fn    = func(gen_fetch_block);
     send_block_fn     = func(gen_send_block);
   }
#endif

#if ARMBSD
  elif (!mystrcmp(conf, "linkpod"))
   { gnome_init_link(); 
     open_link_fn      = func(gnome_open_link);
     free_link_fn      = func(gnome_free_link);
     reset_fn          = func(gnome_reset_transputer);
     analyse_fn        = func(gnome_analyse_transputer);
     rdrdy_fn          = func(gnome_rdrdy);
     wrrdy_fn          = func(gnome_wrrdy);
     byte_from_link_fn = func(gen_byte_from_link);
     byte_to_link_fn   = func(gen_byte_to_link);
     fetch_block_fn    = func(gen_fetch_block);
     send_block_fn     = func(gen_send_block);
   }
#endif

#if UNIX386
  elif (!mystrcmp(conf, "BRS"))
   { reset_fn          = func(inmos_reset_transputer);
     analyse_fn        = func(inmos_analyse_transputer);
     rdrdy_fn          = func(inmos_rdrdy);
     wrrdy_fn          = func(inmos_wrrdy);
     byte_from_link_fn = func(inmos_byte_from_link);
     byte_to_link_fn   = func(inmos_byte_to_link);
     send_block_fn     = func(inmos_send_block);
     fetch_block_fn    = func(inmos_fetch_block);
     open_link_fn      = func(inmos_open_link);
     free_link_fn      = func(inmos_free_link);

     brs_init_link();
     inmos_open_link_fn    = func(OpenLink);
     inmos_close_link_fn   = func(CloseLink);
     inmos_read_link_fn    = func(ReadLink);
     inmos_write_link_fn   = func(WriteLink);
     inmos_reset_link_fn   = func(ResetLink);
     inmos_analyse_link_fn = func(AnalyseLink);
     inmos_test_read_fn    = func(TestRead);
     inmos_test_write_fn   = func(TestWrite);
   }        
#endif /* UNIX386 */

/**
*** This else corresponds to the if(remote) statement above. Various
*** bits of transputer hardware may have been tested for in the meantime,
*** depending on the host machine.
**/
  else
   { ServerDebug("Unknown transputer box : %s", conf);
     longjmp(exit_jmpbuf, 1);
   } 

/**
*** This initialises new entries in the link_table. Entry 0 has been
*** initialised already at the top of the routine. This is to allow
*** rem_init_link() to fill in the file descriptor, which makes
*** rem_open_link() a no-op.
**/
  { int i;
    for (i = 1; i < number_of_links; i++)
     { link_table[i].state      = Link_Reset;
       link_table[i].fildes     = -1;
       link_table[i].ready      = 0;
       link_table[i].flags      = Link_flags_unused + Link_flags_uninitialised +
                                  Link_flags_firsttime;
       link_table[i].connection = -1;
     }
  }

  if (Server_Mode eq Mode_Normal)
    Server_resetlnk();
  else if (Server_Mode eq Mode_Daemon)
   Daemon_resetlnk();
}


/**
*** For now, a server can only talk to one link. The user can specify
*** a particular site in the host.con file, or else any free site will be used.
**/

PRIVATE void Server_resetlnk()
{ WORD site = get_int_config("site");
  int i;

  if (site eq Invalid_config)  /* look for any free site */
   { for (i = 0; i < number_of_links; i++)
      if ((*open_link_fn)(i) ne 0)
       { transputer_site = i; current_link = i;
         link_table[i].flags &= ~(Link_flags_uninitialised + 
                                  Link_flags_firsttime);
         return;
       }

     ServerDebug("No free sites available at present.");
     longjmp(exit_jmpbuf, 1);
   }

  if ((site < 0L) || (site >=number_of_links)) 
   { ServerDebug("Error in host.con file : site %ld invalid", site);
     longjmp(exit_jmpbuf, 1);
   }

  if ((*open_link_fn)(site) eq 0)
   { ServerDebug("Unable to access site %d", site);
     longjmp(exit_jmpbuf, 1);
   }

  transputer_site = site;
  current_link = site;
  link_table[site].flags &= ~(Link_flags_uninitialised + 
                              Link_flags_firsttime);
}

/**
*** The daemon may use all available sites. Depending on the hydra.con
*** file it will either try to open all available sites or only those sites
*** specified.
***
*** In the daemon, a link can be in three states. If it is unused, either
*** because it is not part of the hydra.con list or because it was not
*** possible to access the link when the daemon started up, the link is
*** never accessible via the daemon. It may still be accessed directly by other
*** applications or by a Server. If the link is free, it may be accessed via
*** the daemon but the latter does not currently have an open stream to it :
*** after the init_link the daemon will free_link() everything; this allows
*** other applications access to the link; when a Server tries to connect,
*** the link is reopened by an open_link().
**/

PRIVATE void Daemon_resetlnk()
{ char *all_sites = get_config("all_sites");
  char *usable;
  int i, opened;

  opened = 0;

  if (all_sites ne NULL)
   { for (i = 0; i < number_of_links; i++)
      if ((*open_link_fn)(i) ne 0)
       { opened++;
         link_table[i].flags &= ~(Link_flags_unused +
             Link_flags_uninitialised + Link_flags_firsttime);
         link_table[i].flags |= (Link_flags_free);
         (*free_link_fn)(i);
         link_table[i].flags |= Link_flags_uninitialised;
       }
      else
       link_table[i].flags |= Link_flags_unused;

     if (opened eq 0)
      { ServerDebug("Failed to open any link devices.");
        longjmp(exit_jmpbuf, 1);
      }
     return;
   }

  for (i = 0; i < number_of_links; i++)
   { 
     usable = get_config(link_table[i].link_name);
     if (usable eq NULL)
        link_table[i].flags |= Link_flags_unused;
     else
      { if ((*open_link_fn)(i) ne 0)
         { opened++;
           link_table[i].flags &= ~(Link_flags_unused +
               Link_flags_uninitialised + Link_flags_firsttime);
           link_table[i].flags |= (Link_flags_free);
           (*free_link_fn)(i); 
           link_table[i].flags |= Link_flags_uninitialised;
         }
        else
         { ServerDebug("Warning : failed to use %s", 
                 link_table[i].link_name);
           link_table[i].flags |= Link_flags_unused;
         }
      }
   }
  if (opened eq 0)
   { ServerDebug("Error : unable to open any link devices.");
     longjmp(exit_jmpbuf, 1);
   }
}

/**
*** The tidy_link() function is called from tload.c on exit, at
*** present only on Unix systems.
**/
void tidy_link()
{ (*free_link_fn)(current_link);
  link_table[current_link].flags |= Link_flags_uninitialised;
} 

/**
*** This code contains useful routines when dealing with normal link devices
*** or sockets. read_delay() monitors a link for 1/10 second for incoming
*** data, useful for recovering from failed reads. gen_byte_from_link()
*** reads a single byte from the link. The read may fail for some reason,
*** e.g. a signal interrupted the device, so there are a number of retries.
*** gen_byte_to_link() just tries to send a byte, making no attempt to recover.
*** gen_fetch_block() reads a block of data from the link, with recovery code.
*** gen_send_block() sends a block without attempting to recover. Experience
*** so far has indicated that problems arise in reading the link rather than
*** writing. null_wrrdy() is useful if there is no simple way of testing
*** the output status of the link. select_rdrdy() tests the link for available
*** data, if no suitable ioctl is available.
**/

PRIVATE void read_delay()
{
  fd_set rdmask;	
  struct timeval timelim;  /* used for selects on link */
  FD_ZERO(&rdmask);
  FD_SET(link_fd,&rdmask);
  timelim.tv_sec  = 0;
  timelim.tv_usec = 100000;
printf("about to call select \n");
  (void)select(link_fd + 1, &rdmask, NULL, NULL, &timelim);
}

PRIVATE WORD gen_byte_from_link(where)
UBYTE *where;
{ int i;

  for (i = 0; i < 5; i++)
   { 
     if (read(link_fd, where, 1) eq 1)
        return(0);   /* 0 bytes left to read */
     if (i < 4) read_delay();
   }

  return(1);  /* Failed to read 1 byte */
}

PRIVATE WORD gen_byte_to_link(data)
int data;
{ int i;
  unsigned char fred = data & 0x00FF;

  for (i = 0; i < 5; i++)
   if (write(link_fd, &fred, 1) eq 1)
    return(0);   /* 0 bytes left to read */

  return(1);  /* Failed to write 1 byte */
}

PRIVATE WORD gen_fetch_block(count,data,timeout)
int count;
char *data;
int timeout;
{ int amount = 0, x, i;

  for (i = 0; i < 5; i++)
   { 
     x = read(link_fd, &(data[amount]), count - amount);
     if (x < 0)    /* read failed, possibly because of a signal */
      { if (errno eq EINTR)
         { read_delay(); continue; }
        else
         return(count - amount);
      }
     amount += x;               /* increment number of bytes read so far */
     if (amount >= count) return(0);   /* read all bytes, 0 left to do */
   }

  return(count - amount);       /* return amount failed to read */
  use(timeout)
} 

PRIVATE WORD gen_send_block(count,data,timeout)
int count;
char *data;
int timeout;
{ int amount = 0, x, i;

  for (i = 0; i < 5; i++)
   { 
     x = write(link_fd, &(data[amount]), count - amount);
     if (x < 0)
      { if (errno eq EINTR)
         continue;
        else
         return(count - amount);
      }
     amount += x;               /* increment number of bytes read so far */
     if (amount >= count) return(0);   /* read all bytes, 0 left to do */
   }

  return(count - amount);       /* return amount failed to read */
  use(timeout)
} 

PRIVATE WORD null_wrrdy()
{ return(TRUE);
}

PRIVATE WORD select_rdrdy()
{
  fd_set rdmask;	
  struct timeval timelim;  /* used for selects on link */
  FD_ZERO(&rdmask);
  FD_SET(link_fd,&rdmask);
  timelim.tv_sec  = 0;
  timelim.tv_usec = 1;
  if(select(link_fd + 1, &rdmask, NULL, NULL, &timelim) < 1)
     return(FALSE);
  return(TRUE);
}

/**
*** When the server has to use a socket to communicate with a remote
*** link of some sort, reset and analyse involves sending special protocol
*** bytes over the socket. This assumes that both sides are synchronised.
*** To shut down the socket tidily it is necessary to inform the link
*** daemon before the close. Transferring data is done via socket_read()
*** and socket_write(), which retry until successful or the socket is
*** broken and hence cannot fail. The open_link() is a no-op, since the
*** rem_init_link has already filled in the file descriptor field.
**/

PRIVATE void rem_reset()
{ char buf[1];
  buf[0] = Proto_RemoteReset;
  socket_write(link_fd, buf, 1, "hydra");
}

PRIVATE void rem_analyse()
{ char buf[1];
  buf[0] = Proto_RemoteAnalyse;
  socket_write(link_fd, buf, 1, "hydra");
}

PRIVATE void rem_free_link(i)
int i;
{ char buf[1];
  if (link_table[i].fildes eq -1) return;
  buf[0] = Proto_Close;
  socket_write(link_table[i].fildes, buf, 1, "hydra");
  close(link_fd);
}

PRIVATE void rem_open_link(i)
int i;
{ use(i)
}

PRIVATE WORD rem_byte_from_link(where)
UBYTE *where;
{
  socket_read(link_fd,where,1, "hydra");
  return(0); 
}

PRIVATE WORD rem_byte_to_link(data)
int data;
{
  unsigned char fred = data & 0x00FF;
  socket_write(link_fd,&fred,1, "hydra");
  return(0);
}

PRIVATE WORD rem_fetch_block(count,data,timeout)
int count;
char *data;
int timeout;
{ socket_read(link_fd, data, count, "hydra");
  return(0);
  use(timeout)
} 

PRIVATE WORD rem_send_block(count,data,timeout)
int count;
char *data;
int timeout;
{ socket_write(link_fd, data, count, "hydra");
  return(0);
  use(timeout)
}

/**
*** When the Server is running in remote mode, it can only access one
*** link and does so indirectly using a socket to the link daemon.
*** First it creates the socket within the appropriate family and tries
*** to connect to it, then it performs a simple packet transaction
*** with the daemon. This transaction informs the daemon who the user is,
*** which machine the Server is running on, and what site he/she wants
*** or any site. It is possible for the daemon to reject the connection
*** initially if it is very busy, so the connection may have to be
*** retried. Also, the site requested may be in use.
**/

PRIVATE void fn( rem_inet_init_link, (int));

PRIVATE void rem_init_link()
{ int my_socket;
  char *family_name = get_config("family_name");
  int family_type = AF_UNIX;
  WORD site;
  socket_msg con;
  WORD retries = get_int_config("connection_retries");
  if (retries eq Invalid_config) retries = 5L;


  if (gethostname(con.hostname) eq -1)
   { ServerDebug("Unable to obtain host name : %q");
     if (errno <= sys_nerr)
      ServerDebug("%s", sys_errlist[errno]);
     else
      ServerDebug("unknown errno %d", errno);
     longjmp(exit_jmpbuf, 1);
   }

  if (cuserid(con.userid) eq NULL)
   { ServerDebug("Unable to obtain user name : %q");
     if (errno <= sys_nerr)
      ServerDebug("%s", sys_errlist[errno]);
     else
      ServerDebug("unknown errno %d", errno);
     longjmp(exit_jmpbuf, 1);
   }

  if (family_name eq (char *) NULL)
   family_name = "AF_UNIX";

  if (!mystrcmp(family_name, "AF_UNIX"))
   family_type = AF_UNIX;
  elif(!mystrcmp(family_name, "AF_INET"))
   family_type = AF_INET;
  else
   { ServerDebug("Unknown socket family type %s in configuration file",
                 family_name);
     ServerDebug("Known families are AF_UNIX and AF_INET.");
     longjmp(exit_jmpbuf, 1);
   }

  while (retries > 0)
   {
     my_socket = socket(family_type, SOCK_STREAM, 0);
    if (my_socket < 0)
     { ServerDebug("Failed to create socket : %q");
       if (errno <= sys_nerr)
        ServerDebug("%s", sys_errlist[errno]);
       else
        ServerDebug("unknown errno %d", errno);
       longjmp(exit_jmpbuf, 1);
     }
    link_fd = my_socket;

    if (family_type eq AF_UNIX)
     { struct sockaddr_un addr;
       char *socket_name = get_config("socket_name");

       addr.sun_family = AF_UNIX;
       strcpy(addr.sun_path,
              (socket_name eq (char *) NULL) ? "hydra.skt" : socket_name);
       if (connect(my_socket, &addr, 
          strlen(addr.sun_path) + sizeof(addr.sun_family) ) eq -1)
        { ServerDebug("Failed to connect to hydra : %q");
          if (errno <= sys_nerr)
           ServerDebug("%s", sys_errlist[errno]);
          else
           ServerDebug("unknown errno %d", errno);
          longjmp(exit_jmpbuf, 1);
        }     
     }
    else
     rem_inet_init_link(my_socket);

    site = get_int_config("site");
    if (site eq Invalid_config)
     con.fnrc = swap(Any_Link);
    else
     con.fnrc = swap(site);

    if (write(my_socket, (BYTE *) &con, sizeof(socket_msg)) 
      < sizeof(socket_msg))
     { ServerDebug("Failed to write to hydra : %q");
       if (errno <= sys_nerr)
        ServerDebug("%s", sys_errlist[errno]);
       else
        ServerDebug("unknown errno %d", errno);
       longjmp(exit_jmpbuf, 1);
     }
    if (read(my_socket, &con, sizeof(socket_msg)) 
      < sizeof(socket_msg))
     { ServerDebug("Failed to read from hydra : %q");
       if (errno <= sys_nerr)
        ServerDebug("%s", sys_errlist[errno]);
       else
        ServerDebug("unknown errno %d", errno);
       longjmp(exit_jmpbuf, 1);
     }

    if (swap(con.fnrc) eq Invalid_Link)
     { ServerDebug("Site %d is invalid according to Hydra.", site);
       longjmp(exit_jmpbuf, 1);
     }

    if (swap(con.fnrc) eq Link_Unavailable)
     { if (site eq Invalid_config)
        ServerDebug("Hydra has no free sites.");
       else
        ServerDebug("Hydra has marked site %d as in use.", site);
       longjmp(exit_jmpbuf, 1);
     }

    if (swap(con.fnrc) ne Hydra_Busy)
     { transputer_site = swap(con.fnrc);
       return;
     }

    ServerDebug("Hydra is busy...");
    if (retries > 1) sleep(5);
  }

  ServerDebug("Giving up on trying to connect to hydra.");
  longjmp(exit_jmpbuf, 1);
}

PRIVATE void rem_inet_init_link(my_socket)
int my_socket;
{ struct sockaddr_in addr;
  struct hostent *hp;
  struct servent *sp;
  char *host_name   = get_config("hydra_host");

  if (host_name eq (char *) NULL)
   { ServerDebug("Missing entry in host.con file for hydra_host.");
     longjmp(exit_jmpbuf, 1);
   }

  hp = gethostbyname(host_name);
  if (hp eq NULL)
   { ServerDebug("Unable to obtain address for hydra host %s", host_name);
     longjmp(exit_jmpbuf, 1);
   }

  sp = getservbyname("hydra", "tcp");
  if (sp eq NULL)
   { ServerDebug("Unable to obtain a port for the hydra server.");
     longjmp(exit_jmpbuf, 1);
   }

  bzero((char *) &addr, sizeof(struct sockaddr_in));
  bcopy(hp->h_addr, (char *) &addr.sin_addr, hp->h_length);
  addr.sin_family = hp->h_addrtype;
  addr.sin_port   = sp->s_port;

  if (connect(my_socket, &addr, sizeof(struct sockaddr_in)) eq -1)
   { ServerDebug("Failed to connect to hydra : %q");
     if (errno <= sys_nerr)
      ServerDebug("%s", sys_errlist[errno]);
     else
      ServerDebug("unknown errno %d", errno);
     longjmp(exit_jmpbuf, 1);
   }
}

/**
*** The code below uses the Inmos link interface scheme, if anybody wants it.
*** However, it cannot be complete because the scheme does not define the number
*** of links, nor what they are called. There are external routines
*** b011_init_link(), b014_init_link(), and so on, which deal with such
*** matters. All the routines indirect through the inmos-specific link
*** routine pointers, adding yet another level of indirection.
**/
#if (SUN3 || SUN4 || UNIX386)

PRIVATE void inmos_reset_transputer()
{ (void) (*inmos_reset_link_fn)(link_fd);
}

PRIVATE void inmos_analyse_transputer()
{ (void) (*inmos_analyse_link_fn)(link_fd);
}

PRIVATE int inmos_rdrdy()
{ return( (*inmos_test_read_fn)(link_fd) );
}

PRIVATE int inmos_wrrdy()
{ return( (*inmos_test_write_fn)(link_fd) );
}

PRIVATE int inmos_byte_from_link(where)
UBYTE *where;
{ 
  if ((*inmos_read_link_fn)(link_fd, where, 1, 5) < 1)
   return(1);  /* amount failed to read */
  return(0);
}

PRIVATE int inmos_byte_to_link(data)
int data;
{ UBYTE buf[1];
  buf[0] = (UBYTE) data;
  if ((*inmos_write_link_fn)(link_fd, buf, 1, 5) < 1)
   return(1);  /* amount failed to write */
  else
   return(0);
}

PRIVATE int inmos_fetch_block(amount, buf, timeout)
int amount;
BYTE *buf;
int timeout;
{ return(amount - ((*inmos_read_link_fn)(link_fd, buf, amount, 10)));
}

PRIVATE int inmos_send_block(amount, buf, timeout)
int amount;
BYTE *buf;
int timeout;
{ return(amount - ((*inmos_write_link_fn)(link_fd, buf, amount, 10)));
}

PRIVATE int inmos_open_link(tabno)
int tabno;
{ link_table[tabno].fildes = (*inmos_open_link_fn)(link_table[tabno].link_name);
  if (link_table[tabno].fildes < 0)
   return(0);
  link_table[tabno].flags |= Link_flags_not_selectable;
  return(1); 
}

PRIVATE void inmos_free_link(tabno)
int tabno;
{ (*inmos_close_link_fn)(link_table[tabno].fildes);
  link_table[tabno].fildes = -1;
}
 
#endif  /* INMOS */

