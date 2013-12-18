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
/* RcsId: $Id: linklib.c,v 1.22 1994/06/29 13:46:19 tony Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.        			*/

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
*** Interaction with the link daemon Hydra is handled via a suitable
*** protocol over a TCP socket.
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
*** hydra_init_link(). free_link() also takes a single
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
*** routine should no longer do a reset.
**/

#define Linklib_Module

#if ARMBSD
#include "helios.h"
#else
#include "../helios.h"
#endif

#if !SOLARIS
typedef struct sockaddr		sockaddr;
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

/*
 * The following typedefs have been placed in structs.h, but are left here
 * for reference.
 *
 *
 * #ifdef __cplusplus
 * typedef WORD	(*WordUbyteFnPtr)(UBYTE *);
 * typedef WORD	(*WordIntFnPtr)(int);
 * typedef WORD	(*WordIntplusFnPtr)(int, BYTE *, int);
 * #else
 * typedef WordFnPtr	WordUbyteFnPtr;
 * typedef WordFnPtr	WordIntFnPtr;
 * typedef WordFnPtr	WordIntplusPtr;
 * #endif
 */

#ifdef __cplusplus
typedef int	(*IntIntFnPtr)(int);
typedef void	(*VoidIntFnPtr)(int);
/* For Inmos functions */
typedef int	(*IntIntplusFnPtr)(int, BYTE *, int, int);
typedef int	(*IntCharFnPtr)(char *);
#else
typedef int		(*IntIntFnPtr)();
typedef VoidFnPtr	VoidIntFnPtr;
/* For Inmos functions */
typedef IntIntFnPtr	IntIntplusFnPtr;
typedef IntIntFnPtr	IntCharFnPtr;
#endif

/**
*** Here are declarations for the pointers used to indirect to the link
*** I/O routines. The first two are specific to the Unix version. The
*** remainder are actually inside the linkio.c module.
**/

IntIntFnPtr	open_link_fn;
VoidIntFnPtr	free_link_fn = (VoidIntFnPtr)(NULL);

extern WordFnPtr	rdrdy_fn;
extern WordFnPtr  	wrrdy_fn;
extern WordIntFnPtr    	byte_to_link_fn;
extern WordUbyteFnPtr  	byte_from_link_fn;
extern WordIntplusFnPtr	send_block_fn;
extern WordIntplusFnPtr	fetch_block_fn;
extern VoidFnPtr	reset_fn;
extern VoidFnPtr	analyse_fn;

#define vdvd_fn_(f)	(VoidFnPtr)(f)
#define inin_fn_(f)	(IntIntFnPtr)(f)
#define vdin_fn_(f)	(VoidIntFnPtr)(f)
#define wdvd_fn_(f)	(WordFnPtr)(f)
#define wdin_fn_(f)	(WordIntFnPtr)(f)
#define wdub_fn_(f)	(WordUbyteFnPtr)(f)
#define wdinp_fn_(f)	(WordIntplusFnPtr)(f)

/* For Inmos functions */
#define ininp_fn_(f)	(IntIntplusFnPtr)(f)
#define inch_fn_(f)	(IntCharFnPtr)(f)
/*
 * Old types - left for reference
 *
 * extern int  (*rdrdy_fn)();
 * extern int  (*wrrdy_fn)();
 * extern int  (*byte_to_link_fn)();
 * extern int  (*byte_from_link_fn)();
 * extern int  (*send_block_fn)();
 * extern int  (*fetch_block_fn)();
 * extern void (*reset_fn)();
 * extern void (*analyse_fn)();
 */


/**
*** If the link I/O goes through the Inmos compatible standard, there
*** is another level of indirection to allow for different hardware.
*** Currently only applies to Sun3, Sun4 and UNIX386.
**/

#if (SUN3 || SUN4 || UNIX386)

#if (!SOLARIS)

#if ANSI_prototypes
PRIVATE	int (*inmos_read_link_fn)(int, BYTE *, int, int);
PRIVATE	int (*inmos_write_link_fn)(int, BYTE *, int, int);
PRIVATE	int (*inmos_reset_link_fn)(int);
PRIVATE	int (*inmos_analyse_link_fn)(int);
PRIVATE	int (*inmos_test_read_fn)(int);
PRIVATE	int (*inmos_test_write_fn)(int);
PRIVATE	int (*inmos_open_link_fn)(char *);
PRIVATE	int (*inmos_close_link_fn)(int);
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
#else
PRIVATE	int (*inmos_read_link_fn)();
PRIVATE	int (*inmos_write_link_fn)();
PRIVATE	int (*inmos_reset_link_fn)();
PRIVATE	int (*inmos_analyse_link_fn)();
PRIVATE	int (*inmos_test_read_fn)();
PRIVATE	int (*inmos_test_write_fn)();
PRIVATE	int (*inmos_open_link_fn)();
PRIVATE	int (*inmos_close_link_fn)();
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
#endif /* ANSI_prototypes */

#endif /* !SOLARIS */

#endif 

PRIVATE WORD fn( gen_byte_from_link, (UBYTE *));
PRIVATE WORD fn( gen_byte_to_link, (int));
PRIVATE WORD fn( gen_send_block, (int, BYTE *, int));
PRIVATE WORD fn( gen_fetch_block, (int, BYTE *, int));
PRIVATE void fn( gen_reset, (void));
PRIVATE void fn( gen_analyse, (void));
PRIVATE WORD fn( select_rdrdy, (void));
PRIVATE WORD fn( null_wrrdy, (void));

PRIVATE void fn( hydra_reset, (void));
PRIVATE void fn( hydra_analyse, (void));
PRIVATE WORD fn( hydra_byte_to_link, (int));
PRIVATE WORD fn( hydra_byte_from_link, (UBYTE *));
PRIVATE WORD fn( hydra_send_block, (int, BYTE *, int));
PRIVATE WORD fn( hydra_fetch_block, (int, BYTE *, int));
PRIVATE void fn( hydra_init_link, (void));
PRIVATE int  fn( hydra_open_link, (int));
PRIVATE void fn( hydra_free_link, (int));
PRIVATE WORD fn( hydra_rdrdy, (void));

PRIVATE void fn( Mhydra_reset, (void));
PRIVATE void fn( Mhydra_analyse, (void));
PRIVATE void fn( Mhydra_free_link, (int));
PRIVATE word fn( Mhydra_byte_to_link, (int));
PRIVATE word fn( Mhydra_byte_from_link, (UBYTE *));
PRIVATE word fn( Mhydra_send_block, (int, BYTE *, int));
PRIVATE word fn( Mhydra_fetch_block, (int, BYTE *, int));

#if (SUN3 || SUN4)
   /* Hardware supported for these machines is the Transtech MCP1000  */
   /* or Niche NTP1000, depending on your point of view, the Parsytec */
   /* boards, and the various Inmos boards B011, B014, and B016. The  */
   /* Parsytec and Inmos boards are accessed through the Inmos link   */
   /* standard. Also a Telmat board, the Volvox board, and Hunt C40   */
   /* boards for VME and SBUS.                                        */

extern void fn( niche_init_link, (void));
extern void fn( niche_reset_transputer, (void));
extern void fn( niche_analyse_transputer, (void));
extern int  fn( niche_open_link,   (int));
extern void  fn( niche_free_link,   (int));

extern void fn( telmat_init_link, (void));
extern void fn( telmat_reset_transputer, (void));
extern void fn( telmat_analyse_transputer, (void));
extern int  fn( telmat_open_link,   (int));
extern void fn( telmat_free_link,   (int));

extern void fn( volvox_init_link, (void));
extern void fn( volvox_reset_transputer, (void));
extern void fn( volvox_analyse_transputer, (void));
extern int  fn( volvox_open_link,   (int));
extern void fn( volvox_free_link,   (int));

extern void fn( hunt_init_link, (void));
extern void fn( hunt_reset_c40, (void));
extern int  fn( hunt_open_link, (int));
extern void fn( hunt_free_link, (int));
extern int  fn( hunt_rdrdy,     (void));
extern int  fn( hunt_wrrdy,     (void));

#if !SOLARIS
extern  void fn( b011_init_link,   (void));
extern  int  fn( b011_OpenLink,    (char *));
extern  int  fn( b011_CloseLink,   (int));
extern  int  fn( b011_ReadLink,    (int, char *, unsigned int, int));
extern  int  fn( b011_WriteLink,   (int, char *, unsigned int, int));
extern  int  fn( b011_ResetLink,   (int));
extern  int  fn( b011_AnalyseLink, (int));
extern  int  fn( b011_TestRead,    (int));
extern  int  fn( b011_TestWrite,   (int));
#endif

#if !SOLARIS
extern  void fn( b014_init_link,   (void));
extern  int  fn( b014_OpenLink,    (char *));
extern  int  fn( b014_CloseLink,   (int));
extern  int  fn( b014_ReadLink,    (int, char *, unsigned int, int));
extern  int  fn( b014_WriteLink,   (int, char *, unsigned int, int));
extern  int  fn( b014_ResetLink,   (int));
extern  int  fn( b014_AnalyseLink, (int));
extern  int  fn( b014_TestRead,    (int));
extern  int  fn( b014_TestWrite,   (int));
#endif

#if !SOLARIS
extern  void fn( b016_init_link,   (void));
extern  int  fn( b016_OpenLink,    (char *));
extern  int  fn( b016_CloseLink,   (int));
extern  int  fn( b016_ReadLink,    (int, char *, unsigned int, int));
extern  int  fn( b016_WriteLink,   (int, char *, unsigned int, int));
extern  int  fn( b016_ResetLink,   (int));
extern  int  fn( b016_AnalyseLink, (int));
extern  int  fn( b016_TestRead,    (int));
extern  int  fn( b016_TestWrite,   (int));
#endif

#ifdef PARSY
	/* The Parsytec code is not part of the standard	*/
	/* source release.					*/
extern  void fn( parsy_init_link,   (void));
extern  int  fn( parsy_OpenLink,    (char *));
extern  int  fn( parsy_CloseLink,   (int));
extern  int  fn( parsy_ReadLink,    (int, char *, unsigned int, int));
extern  int  fn( parsy_WriteLink,   (int, char *, unsigned int, int));
extern  int  fn( parsy_ResetLink,   (int));
extern  int  fn( parsy_AnalyseLink, (int));
extern  int  fn( parsy_TestRead,    (int));
extern  int  fn( parsy_TestWrite,   (int));
#endif   /* PARSY */

#if SOLARIS
extern void fn (matchbox_init_link, 	(void));
extern void fn (matchbox_reset,		(void));
extern void fn (matchbox_analyse,	(void));
extern int  fn (matchbox_open_link,	(int));
extern void fn (matchbox_free_link,	(int));
extern void fn (matchbox_rdrdy,		(void));
extern void fn (matchbox_wrrdy,		(void));
extern void fn (matchbox_byte_to_link,	(int));
extern int  fn (matchbox_byte_from_link,(UBYTE *));
extern int  fn (matchbox_send_block,	(int, BYTE *, int));
extern int  fn (matchbox_fetch_block,	(int, BYTE *, int));
#endif

#endif /* (SUN3 || SUN4) */

#if SUN386
   /* SUN386 means a B008 board with the K-PAR driver. */
   /* No other hardware at present. */
extern void fn( kpar_init_link,          (void));
extern void fn( kpar_reset_transputer,   (void));
extern void fn( kpar_analyse_transputer, (void));
extern int  fn( kpar_open_link,          (int));
extern void fn( kpar_free_link,          (int));
#endif  /* SUN386 */

#if (SM90 || TR5)
     /* The Telmat SM90 box with their own link adapter, itftp32 */
     /* the relevant routines are held in smlink.c */
extern void fn( itftp32_init_link, (void));
extern void fn( itftp32_reset_transputer, (void));
extern void fn( itftp32_analyse_transputer, (void));
extern int  fn( itftp32_open_link, (int));
extern void fn( itftp32_free_link, (int));

extern void fn( trp3_init_link, (void));
extern void fn( trp3_reset_transputer, (void));
extern void fn( trp3_analyse_transputer, (void));
extern int  fn( trp3_open_link, (int));
extern void fn( trp3_free_link, (int));
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

#if SCOUNIX
extern void fn ( kpar2_init_link, (void));
extern int  fn ( kpar2_open_link, (int));
extern void fn ( kpar2_free_link, (int));
extern void fn ( kpar2_analyse_transputer, (void));
extern void fn ( kpar2_reset_transputer, (void));
extern int  fn ( kpar2_rdrdy, (void));
extern int  fn ( kpar2_wrrdy, (void));

extern void fn ( bbk_init_link, (void));
extern int  fn ( bbk_open_link, (int));
extern void fn ( bbk_free_link, (int));
extern void fn ( bbk_analyse_transputer, (void));
extern void fn ( bbk_reset_transputer, (void));
extern int  fn ( bbk_rdrdy, (void));
extern int  fn ( bbk_wrrdy, (void));
#endif

#if (IUNIX386 || i486V4)
extern void fn ( kpar2_init_link, (void));
extern int  fn ( kpar2_open_link, (int));
extern void fn ( kpar2_free_link, (int));
extern void fn ( kpar2_analyse_transputer, (void));
extern void fn ( kpar2_reset_transputer, (void));
extern int  fn ( kpar2_rdrdy, (void));
extern int  fn ( kpar2_wrrdy, (void));

extern void fn ( tim40_init_link, (void));
extern int  fn ( tim40_open_link, (int));
extern void fn ( tim40_free_link, (int));
extern void fn ( tim40_reset, (void));
extern int  fn ( tim40_rdrdy, (void));
extern int  fn ( tim40_wrrdy, (void));

#if 0
extern void fn ( hepc2_init_link, (void));
extern int  fn ( hepc2_open_link, (int));
extern void fn ( hepc2_free_link, (int));
extern void fn ( hepc2_reset, (void));
extern int  fn ( hepc2_rdrdy, (void));
extern int  fn ( hepc2_wrrdy, (void));
extern int  fn ( hepc2_byte_from_link, (int *));
extern int  fn ( hepc2_byte_to_link, (int));
extern int  fn ( hepc2_send_block, (int, BYTE *, int));
extern int  fn ( hepc2_fetch_block, (int, BYTE *, int));
#endif
#endif

#if (SUN3 || SUN4)

#if !SOLARIS
extern void fn ( vc40_init_link,      (void));
extern int  fn ( vc40_open_link,      (int));
extern void fn ( vc40_free_link,      (int));
extern void fn ( vc40_reset,          (void));
extern int  fn ( vc40_rdrdy,          (void));
extern int  fn ( vc40_wrrdy,          (void));
extern int  fn ( vc40_byte_from_link, (UBYTE *));
extern int  fn ( vc40_byte_to_link,   (int));
extern int  fn ( vc40_send_block,     (int, BYTE *, int));
extern int  fn ( vc40_fetch_block,    (int, BYTE *, int));

extern int 	fn ( vc40_settype,	(void));
extern void 	fn ( vc40_enint,	(void));

extern int	VC40_Type;

#endif /* !SOLARIS */

#endif

#if (SUN3 || SUN4)
extern void fn( vy86pid_init_link,		(void));
extern int  fn( vy86pid_open_link,		(int));
extern void fn( vy86pid_free_link,		(int));
extern void fn( vy86pid_reset_processor,	(void));
extern void fn( vy86pid_analyse_processor,	(void));
extern WORD fn( vy86pid_byte_from_link,		(UBYTE *));
extern WORD fn( vy86pid_byte_to_link,		(int));
extern WORD fn( vy86pid_send_block,		(int, BYTE *, int));
extern WORD fn( vy86pid_fetch_block,		(int, BYTE *, int));
extern WORD fn( vy86pid_rdrdy,			(void));
extern WORD fn( vy86pid_wrrdy,			(void));
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
*** host sun4  : box transtech NTP1000/MCP1000/TB400
***              Inmos B011
***              Inmos B014
***              Inmos B016
***              Parsytec
***              telmat sun board box TELMAT
***              Volvox board
***              Hunt C40 boards for VME and SBUS
***		 Ariel VC40 Hydra board
***		 VY86PID board, accessed via the serial lines
*** host sun386 : box IMB, the K-Par device driver
*** host tr5000 
*** host sm90 : box itftp32 and 3trp
*** host arm  : box linkpod
*** host unix386 : box /dev/la, single link only
*** host i486v4, iunix386 : box interactive, hepc2 and tim40
*** host RS6000  : no boxes
*** host HP9000  : no boxes
*** host SCOUNIX : box IMB, the K-Par device driver, or 
***		 : box BBK, the BBK-PC device driver
*** host SOLARIS : Transtech MatchBox,
***		   Hunt C40 SBUS,
***		   vy86pid	(untested)
***		   volvox	(untested)
***		   telmat	(untested)
***		   niche	(untested)
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

  byte_from_link_fn    = wdub_fn_(gen_byte_from_link);
  byte_to_link_fn      = wdin_fn_(gen_byte_to_link);
  send_block_fn        = wdinp_fn_(gen_send_block);
  fetch_block_fn       = wdinp_fn_(gen_fetch_block);
  reset_fn             = vdvd_fn_(gen_reset);
  analyse_fn           = vdvd_fn_(gen_analyse);
  rdrdy_fn             = wdvd_fn_(select_rdrdy);
  wrrdy_fn             = wdvd_fn_(null_wrrdy);

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
/*        longjmp(exit_jmpbuf, 1); */
	longjmp_exit;
      }
     Server_Mode = Mode_Remote; 
     hydra_init_link();
     open_link_fn      = inin_fn_(hydra_open_link);
     free_link_fn      = vdin_fn_(hydra_free_link);
     byte_from_link_fn = wdub_fn_(hydra_byte_from_link);
     byte_to_link_fn   = wdin_fn_(hydra_byte_to_link);
     send_block_fn     = wdinp_fn_(hydra_send_block);
     fetch_block_fn    = wdinp_fn_(hydra_fetch_block);
     reset_fn          = vdvd_fn_(hydra_reset);
     analyse_fn        = vdvd_fn_(hydra_analyse);
     rdrdy_fn	       = wdvd_fn_(hydra_rdrdy);
   }

#if (SUN3 || SUN4)

  elif ((!mystrcmp(conf, "NTP1000")) ||  /* niche board */
        (!mystrcmp(conf, "MCP1000")) ||
	(!mystrcmp(conf, "TB400"))   )
   { 
     niche_init_link();
     open_link_fn      = inin_fn_(niche_open_link);
     free_link_fn      = vdin_fn_(niche_free_link);
     reset_fn          = vdvd_fn_(niche_reset_transputer);
     analyse_fn        = vdvd_fn_(niche_analyse_transputer);
   }

  elif (!mystrcmp(conf, "TELMAT"))  /* telmat board */
   { 
     telmat_init_link();
     open_link_fn      = inin_fn_(telmat_open_link);
     free_link_fn      = vdin_fn_(telmat_free_link);
     reset_fn          = vdvd_fn_(telmat_reset_transputer);
     analyse_fn        = vdvd_fn_(telmat_analyse_transputer);
   }

  elif (!mystrcmp(conf, "VOLVOX"))  /* volvox board */
   { 
     volvox_init_link();
     open_link_fn      = inin_fn_(volvox_open_link);
     free_link_fn      = vdin_fn_(volvox_free_link);
     reset_fn          = vdvd_fn_(volvox_reset_transputer);
     analyse_fn        = vdvd_fn_(volvox_analyse_transputer);
   }

  elif (!mystrcmp(conf, "HESB40") || !mystrcmp(conf, "HEV40"))  /* Hunt boards */
   { 
     hunt_init_link();
     open_link_fn      = inin_fn_(hunt_open_link);
     free_link_fn      = vdin_fn_(hunt_free_link);
     reset_fn          = vdvd_fn_(hunt_reset_c40);
     analyse_fn        = vdvd_fn_(hunt_reset_c40);
     rdrdy_fn          = wdvd_fn_(hunt_rdrdy);
     wrrdy_fn          = wdvd_fn_(hunt_wrrdy);
   }

#if !SOLARIS

#ifdef PARSY     
  elif (
        !mystrcmp(conf, "bbk-v1")   ||
        !mystrcmp(conf, "bbk-v2")   ||
        !mystrcmp(conf, "vmtm")     ||
        !mystrcmp(conf, "bbk-v4")   ||
        !mystrcmp(conf, "mtm-sun1") ||
        !mystrcmp(conf, "mtm-sun2") ||
        !mystrcmp(conf, "bbk-s4")   ||
        !mystrcmp(conf, "B011")     ||
        !mystrcmp(conf, "B014")     ||
        !mystrcmp(conf, "B016") )
#else
  elif (
        !mystrcmp(conf, "B011")     ||
        !mystrcmp(conf, "B014")     ||
        !mystrcmp(conf, "B016") )
#endif /* PARSY */
   { 
     reset_fn          = vdvd_fn_(inmos_reset_transputer);
     analyse_fn        = vdvd_fn_(inmos_analyse_transputer);
     rdrdy_fn          = wdvd_fn_(inmos_rdrdy);
     wrrdy_fn          = wdvd_fn_(inmos_wrrdy);
     byte_from_link_fn = wdub_fn_(inmos_byte_from_link);
     byte_to_link_fn   = wdin_fn_(inmos_byte_to_link);
     send_block_fn     = wdinp_fn_(inmos_send_block);
     fetch_block_fn    = wdinp_fn_(inmos_fetch_block);
     open_link_fn      = inin_fn_(inmos_open_link);
     free_link_fn      = vdin_fn_(inmos_free_link);
     
     if (!mystrcmp(conf, "B011"))
      { b011_init_link();
        inmos_open_link_fn    = inch_fn_(b011_OpenLink);
        inmos_close_link_fn   = inin_fn_(b011_CloseLink);
        inmos_read_link_fn    = ininp_fn_(b011_ReadLink);
        inmos_write_link_fn   = ininp_fn_(b011_WriteLink);
        inmos_reset_link_fn   = inin_fn_(b011_ResetLink);
        inmos_analyse_link_fn = inin_fn_(b011_AnalyseLink);
        inmos_test_read_fn    = inin_fn_(b011_TestRead);
        inmos_test_write_fn   = inin_fn_(b011_TestWrite);
      }
     elif (!mystrcmp(conf, "B014"))
      { b014_init_link();
        inmos_open_link_fn    = inch_fn_(b014_OpenLink);
        inmos_close_link_fn   = inin_fn_(b014_CloseLink);
        inmos_read_link_fn    = ininp_fn_(b014_ReadLink);
        inmos_write_link_fn   = ininp_fn_(b014_WriteLink);
        inmos_reset_link_fn   = inin_fn_(b014_ResetLink);
        inmos_analyse_link_fn = inin_fn_(b014_AnalyseLink);
        inmos_test_read_fn    = inin_fn_(b014_TestRead);
        inmos_test_write_fn   = inin_fn_(b014_TestWrite);
      }
     elif (!mystrcmp(conf, "B016"))
      { b016_init_link();
        inmos_open_link_fn    = inch_fn_(b016_OpenLink);
        inmos_close_link_fn   = inin_fn_(b016_CloseLink);
        inmos_read_link_fn    = ininp_fn_(b016_ReadLink);
        inmos_write_link_fn   = ininp_fn_(b016_WriteLink);
        inmos_reset_link_fn   = inin_fn_(b016_ResetLink);
        inmos_analyse_link_fn = inin_fn_(b016_AnalyseLink);
        inmos_test_read_fn    = inin_fn_(b016_TestRead);
        inmos_test_write_fn   = inin_fn_(b016_TestWrite);
      }
#ifdef PARSY
     else	/* The various Parsytec boards	*/
      { parsy_init_link();
        inmos_open_link_fn    = inch_fn_(parsy_OpenLink);
        inmos_close_link_fn   = inin_fn_(parsy_CloseLink);
        inmos_read_link_fn    = ininp_fn_(parsy_ReadLink);
        inmos_write_link_fn   = ininp_fn_(parsy_WriteLink);
        inmos_reset_link_fn   = inin_fn_(parsy_ResetLink);
        inmos_analyse_link_fn = inin_fn_(parsy_AnalyseLink);
        inmos_test_read_fn    = inin_fn_(parsy_TestRead);
        inmos_test_write_fn   = inin_fn_(parsy_TestWrite);
      }
#endif
   }
#endif /* !SOLARIS */

  elif (!mystrcmp(conf, "vy86pid"))
   {
     vy86pid_init_link();
     open_link_fn	= inin_fn_(vy86pid_open_link);
     free_link_fn	= vdin_fn_(vy86pid_free_link);
     reset_fn		= vdvd_fn_(vy86pid_reset_processor);
     analyse_fn		= vdvd_fn_(vy86pid_analyse_processor);
     rdrdy_fn		= wdvd_fn_(vy86pid_rdrdy);
     wrrdy_fn		= wdvd_fn_(vy86pid_wrrdy);
     byte_to_link_fn	= wdin_fn_(vy86pid_byte_to_link);
     byte_from_link_fn	= wdub_fn_(vy86pid_byte_from_link);
     fetch_block_fn	= wdinp_fn_(vy86pid_fetch_block);
     send_block_fn	= wdinp_fn_(vy86pid_send_block);
   }

#if SOLARIS
  elif (!mystrcmp (conf, "MatchBox"))
  {
     matchbox_init_link ();
     open_link_fn	= inin_fn_(matchbox_open_link);
     free_link_fn	= vdin_fn_(matchbox_free_link);
     reset_fn		= vdvd_fn_(matchbox_reset);
     analyse_fn		= vdvd_fn_(matchbox_analyse);
     rdrdy_fn		= wdvd_fn_(matchbox_rdrdy);
     wrrdy_fn		= wdvd_fn_(matchbox_wrrdy);
     byte_to_link_fn	= wdin_fn_(matchbox_byte_to_link);
     byte_from_link_fn	= wdub_fn_(matchbox_byte_from_link);
     fetch_block_fn	= wdinp_fn_(matchbox_fetch_block);
     send_block_fn	= wdinp_fn_(matchbox_send_block);
  }
#endif

#endif (SUN3 || SUN4)

#if (SUN386)
  elif (!mystrcmp(conf, "IMB"))
   { kpar_init_link();
     open_link_fn      = inin_fn_(kpar_open_link);
     free_link_fn      = vdin_fn_(kpar_free_link);
     reset_fn          = vdvd_fn_(kpar_reset_transputer);
     analyse_fn        = vdvd_fn_(kpar_analyse_transputer);
   }
#endif

#if (SM90 || TR5)
  elif (!mystrcmp(conf, "ITFTP32"))
   { itftp32_init_link(); 
     open_link_fn      = inin_fn_(itftp32_open_link);
     free_link_fn      = vdin_fn_(itftp32_free_link);
     reset_fn          = vdvd_fn_(itftp32_reset_transputer);
     analyse_fn        = vdvd_fn_(itftp32_analyse_transputer);
   }
  elif (!mystrcmp(conf, "3TRP"))
   { trp3_init_link(); 
     open_link_fn      = inin_fn_(trp3_open_link);
     free_link_fn      = vdin_fn_(trp3_free_link);
     reset_fn          = vdvd_fn_(trp3_reset_transputer);
     analyse_fn        = vdvd_fn_(trp3_analyse_transputer);
   }
#endif

#if ARMBSD
  elif (!mystrcmp(conf, "linkpod"))
   { gnome_init_link(); 
     open_link_fn      = inin_fn_(gnome_open_link);
     free_link_fn      = vdin_fn_(gnome_free_link);
     reset_fn          = vdvd_fn_(gnome_reset_transputer);
     analyse_fn        = vdvd_fn_(gnome_analyse_transputer);
     rdrdy_fn          = wdvd_fn_(gnome_rdrdy);
     wrrdy_fn          = wdvd_fn_(gnome_wrrdy);
   }
#endif

#if UNIX386
  elif (!mystrcmp(conf, "BRS"))
   { reset_fn          = vdvd_fn_(inmos_reset_transputer);
     analyse_fn        = vdvd_fn_(inmos_analyse_transputer);
     rdrdy_fn          = wdvd_fn_(inmos_rdrdy);
     wrrdy_fn          = wdvd_fn_(inmos_wrrdy);
     byte_from_link_fn = wdub_fn_(inmos_byte_from_link);
     byte_to_link_fn   = wdin_fn_(inmos_byte_to_link);
     send_block_fn     = wdinp_fn_(inmos_send_block);
     fetch_block_fn    = wdinp_fn_(inmos_fetch_block);
     open_link_fn      = inin_fn_(inmos_open_link);
     free_link_fn      = vdin_fn_(inmos_free_link);

     brs_init_link();
     inmos_open_link_fn    = inch_fn_(OpenLink);
     inmos_close_link_fn   = inin_fn_(CloseLink);
     inmos_read_link_fn    = ininp_fn_(ReadLink);
     inmos_write_link_fn   = ininp_fn_(WriteLink);
     inmos_reset_link_fn   = inin_fn_(ResetLink);
     inmos_analyse_link_fn = inin_fn_(AnalyseLink);
     inmos_test_read_fn    = inin_fn_(TestRead);
     inmos_test_write_fn   = inin_fn_(TestWrite);
   }        
#endif /* UNIX386 */

#if SCOUNIX
  elif (!mystrcmp(conf, "IMB"))
   {
     kpar2_init_link();
     open_link_fn      = inin_fn_(kpar2_open_link);
     free_link_fn      = vdin_fn_(kpar2_free_link);
     reset_fn          = vdvd_fn_(kpar2_reset_transputer);
     analyse_fn        = vdvd_fn_(kpar2_analyse_transputer);
     rdrdy_fn	       = wdvd_fn_(kpar2_rdrdy);
     wrrdy_fn	       = wdvd_fn_(kpar2_wrrdy);
   }        
  elif (!mystrcmp(conf, "BBK"))
   {
     bbk_init_link();
     open_link_fn      = inin_fn_(bbk_open_link);
     free_link_fn      = vdin_fn_(bbk_free_link);
     reset_fn          = vdvd_fn_(bbk_reset_transputer);
     analyse_fn        = vdvd_fn_(bbk_analyse_transputer);
     rdrdy_fn	       = wdvd_fn_(bbk_rdrdy);
     wrrdy_fn	       = wdvd_fn_(bbk_wrrdy);
   }        
#endif

#if (IUNIX386 || i486V4)
  elif (!mystrcmp(conf, "interactive"))
   { kpar2_init_link();
     open_link_fn      = inin_fn_(kpar2_open_link);
     free_link_fn      = vdvd_fn_(kpar2_free_link);
     reset_fn          = vdvd_fn_(kpar2_reset_transputer);
     analyse_fn        = vdvd_fn_(kpar2_analyse_transputer);
     rdrdy_fn	       = wdvd_fn_(kpar2_rdrdy);
     wrrdy_fn	       = wdvd_fn_(kpar2_wrrdy);
   }        
  /* TMS320C40 'box' implementations */

  elif (!mystrcmp(conf, "TIM40")) {
    /* HEPC2 defaults */
     tim40_init_link();
     open_link_fn      = inin_fn_(tim40_open_link);
     free_link_fn      = vdin_fn_(tim40_free_link);
     reset_fn          = vdvd_fn_(tim40_reset);
     analyse_fn        = vdvd_fn_(tim40_reset);
     rdrdy_fn	       = wdvd_fn_(tim40_rdrdy);
     wrrdy_fn	       = wdvd_fn_(tim40_wrrdy);
   }        
#if 0
  elif (!mystrcmp(conf, "HEPC2")) {
    /* HEPC2 defaults */
    hepc2_init_link();
    open_link_fn      = inin_fn_(hepc2_open_link);
    free_link_fn      = vdin_fn_(hepc2_free_link);
    rdrdy_fn          = wdvd_fn_(hepc2_rdrdy);
    wrrdy_fn          = wdvd_fn_(hepc2_wrrdy);
    byte_to_link_fn   = wdin_fn_(hepc2_byte_to_link);	/* not actually used */
    byte_from_link_fn = wdub_fn_(hepc2_byte_from_link);	/* not actually used */
    send_block_fn     = wdinp_fn_(hepc2_send_block);
    fetch_block_fn    = wdinp_fn_(hepc2_fetch_block);
    reset_fn          = vdvd_fn_(hepc2_reset);
    analyse_fn        = vdvd_fn_(hepc2_reset);
   }
#endif

#endif /* IUNIX386 */

#if (SUN3 || SUN4)

#if !SOLARIS
  elif (!mystrcmp(conf, "VC40")) {
    vc40_init_link();
    open_link_fn	= inin_fn_(vc40_open_link);
    free_link_fn        = vdin_fn_(vc40_free_link);
    reset_fn            = vdvd_fn_(vc40_reset);
    analyse_fn          = vdvd_fn_(vc40_reset);
    rdrdy_fn            = wdvd_fn_(vc40_rdrdy);
    wrrdy_fn            = wdvd_fn_(vc40_wrrdy);
    byte_to_link_fn     = wdin_fn_(vc40_byte_to_link);
    byte_from_link_fn   = wdub_fn_(vc40_byte_from_link);
    send_block_fn       = wdinp_fn_(vc40_send_block);
    fetch_block_fn      = wdinp_fn_(vc40_fetch_block);
  }
#endif /* !SOLARIS */

#endif

/**
*** This else corresponds to the if(remote) statement above. Various
*** bits of transputer hardware may have been tested for in the meantime,
*** depending on the host machine.
**/
  else
   { ServerDebug("Unknown processor box : %s", conf);
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
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
     {
      if ((*open_link_fn)(i) ne 0)
       { transputer_site = i; current_link = i;
         link_table[i].flags &= ~(Link_flags_uninitialised + 
                                  Link_flags_firsttime);

         return;
       }
     }
     ServerDebug("Server_resetlnk () - No free sites available at present.");
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
   }

  if ((site < 0L) || (site >=number_of_links)) 
   { ServerDebug("Error in host.con file : site %ld invalid", site);
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
   }

  if ((*open_link_fn)(site) eq 0)
   { ServerDebug("Unable to access site %d", site);
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
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
/*        longjmp(exit_jmpbuf, 1); */
	longjmp_exit;
      }
     return;
   }

  for (i = 0; i < number_of_links; i++)
   { 
     usable = get_config(link_table[i].link_name);
     if (usable eq NULL)
        link_table[i].flags |= Link_flags_unused;
     else
      {  
         if ((*open_link_fn)(i) ne 0)
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
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
   }
}

/**
*** The tidy_link() function is called from tload.c on exit, at
*** present only on Unix systems.
**/
void tidy_link()
{
  if(free_link_fn)
 	(*free_link_fn)(current_link);
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

  if (link_table[current_link].flags & Link_flags_not_selectable)
   return;
   
  FD_ZERO(&rdmask);
  FD_SET(link_fd,&rdmask);
#if SM90
  timelim.tv_sec  = 1;
#else
  timelim.tv_sec  = 0;
#endif
  timelim.tv_usec = 100000;
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
	{
         return(count - amount);
	}
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

#if (SM90)
  timelim.tv_sec  = 1;
#else
  timelim.tv_sec  = 0;
#endif
  timelim.tv_usec = 1;
  if(select(link_fd + 1, &rdmask, NULL, NULL, &timelim) < 1)
    {
      return(FALSE);
    }
  return(TRUE);
}

PRIVATE void gen_reset()
{
  ServerDebug("internal error, gen_reset() should not have been called");
/*  longjmp(exit_jmpbuf, 1); */
  longjmp_exit;
}

PRIVATE void gen_analyse()
{
  ServerDebug("internal error, gen_analyse() should not have been called");
/*  longjmp(exit_jmpbuf, 1); */
  longjmp_exit;
}

/**----------------------------------------------------------------------------
*** BLV - March 1993. The code which handles the communication between Hydra
*** and the I/O Server has been rewritten to allow it to support non-transputer
*** systems. In particular Hydra should not have any knowledge about the detailed
*** bootstrap mechanism, as the intelligence for this should be left to the
*** I/O Server. Therefore I have defined a higher level protocol which is used
*** during the bootstrap.
***
*** However, once the bootstrap has been completed and the root processor starts
*** sending messages I do not want the overheads associated with this higher level
*** protocol. Therefore there is a call to switch back to the old protocol
*** where actual Helios messages were sent across the socket.
**/

PRIVATE bool	Hydra_ByteAvailable	= FALSE;
PRIVATE	int	Hydra_ByteValue		= 0;
PRIVATE bool	Hydra_WordAvailable	= FALSE;
PRIVATE BYTE	Hydra_WordValue[4];

PRIVATE void hydra_get_reply(message, byte_only, word_only)
Hydra_Message *message;
bool byte_only, word_only;
{
	message->FnRc	= swap(Hydra_Broken);
	forever
	{
		socket_read(link_fd, (BYTE *) message, sizeof(Hydra_Message), "hydra");
		message->FnRc	= swap(message->FnRc);
#if 0
		ServerDebug("got fnrc %x, byte avail %d, word avail %d", message->FnRc,
			Hydra_ByteAvailable, Hydra_WordAvailable);
#endif

		switch(message->FnRc)
		{
		case	Hydra_Nop	:
			/* Used to wake up select() under certain circumstances.		*/
			continue;

		case	Hydra_DataReadyByte :
			/* Root processor has data available and Hydra has read one byte	*/
			Hydra_ByteAvailable	= TRUE;
			Hydra_ByteValue		= message->Extra.Buf[0];
			if (byte_only)
			{
				message->FnRc = Hydra_ReadAck;
				return;
			}
			continue;

		case	Hydra_DataReadyWord :
			/* Ditto, but Hydra has read a word					*/
			Hydra_WordAvailable	= TRUE;
			memcpy(Hydra_WordValue, message->Extra.Buf, 4);
			if (word_only)
			{
				message->FnRc = Hydra_ReadAck;
				return;
			}
			continue;

		default:
			return;
		}
	}
}

PRIVATE word hydra_rdrdy()
{
	if (Hydra_ByteAvailable || Hydra_WordAvailable)
		return(TRUE);
	else
		return(select_rdrdy());
}

PRIVATE void hydra_reset()
{
	Hydra_Message message;

	message.FnRc	= swap(Hydra_ResetRequest);
	socket_write(link_fd, (BYTE *) &message, sizeof(Hydra_Message), "hydra");

	hydra_get_reply(&message, FALSE, FALSE);
	if (message.FnRc ne Hydra_ResetAck)
	{
  		ServerDebug("hydra_reset, invalid fnrc %x from hydra", message.FnRc);
/*	  	longjmp(exit_jmpbuf, 1); */
		longjmp_exit;
	}
	Hydra_ByteAvailable = Hydra_WordAvailable = FALSE;
}

PRIVATE void hydra_analyse()
{
	Hydra_Message message;

	message.FnRc	= swap(Hydra_AnalyseRequest);
	socket_write(link_fd, (BYTE *) &message, sizeof(Hydra_Message), "hydra");

	hydra_get_reply(&message, FALSE, FALSE);
	if (message.FnRc ne Hydra_AnalyseAck)
	{
  		ServerDebug("hydra_reset, invalid fnrc %x from hydra", message.FnRc);
/*	  	longjmp(exit_jmpbuf, 1); */
		longjmp_exit;
	}
	Hydra_ByteAvailable = Hydra_WordAvailable = FALSE;
}

PRIVATE void hydra_free_link(i)
int i;
{
	Hydra_Message message;

	if (link_fd eq -1) return;

	message.FnRc = swap(Hydra_Done);
	socket_write(link_table[current_link].fildes, (BYTE *) &message, sizeof(Hydra_Message), "hydra");
	Hydra_ByteAvailable = Hydra_WordAvailable = FALSE;
	shutdown(link_fd, 2);
	close(link_fd);
}

	/* hydra_open_link() is a no-op since the work is done in init_link()	*/
PRIVATE int hydra_open_link(i)
int i;
{
	return(i);
}

PRIVATE WORD	hydra_byte_to_link(data)
int	data;
{
	Hydra_Message	message;

	message.FnRc		= swap(Hydra_WriteByte);
	message.Extra.Buf[0]	= data;
	socket_write(link_fd, (BYTE *) &message, sizeof(Hydra_Message), "hydra");

	hydra_get_reply(&message, FALSE, FALSE);

	if (message.FnRc eq Hydra_Broken)
		return(1);

	if (message.FnRc ne Hydra_WriteAck)
	{
  		ServerDebug("hydra_byte_to_link, invalid fnrc %x from hydra", message.FnRc);
/*	  	longjmp(exit_jmpbuf, 1); */
		longjmp_exit;
	}

	return(0);
}

PRIVATE WORD	hydra_word_to_link(data)
BYTE	*data;
{
	Hydra_Message	message;

	message.FnRc	= swap(Hydra_WriteWord);
	memcpy(message.Extra.Buf, data, 4);
	socket_write(link_fd, (BYTE *) &message, sizeof(Hydra_Message), "hydra");

	hydra_get_reply(&message, FALSE, FALSE);

	if (message.FnRc eq Hydra_Broken)
		return(4);

	if (message.FnRc ne Hydra_WriteAck)
	{
  		ServerDebug("hydra_word_to_link, invalid fnrc %x from hydra", message.FnRc);
/*	  	longjmp(exit_jmpbuf, 1); */
		longjmp_exit;
	}

	return(0);
}

PRIVATE WORD	hydra_send_block(count, data, timeout)
int	 count;
char	*data;
int	 timeout;
{
	Hydra_Message	message;

	if (count eq 4)
		return(hydra_word_to_link(data));
		
	message.FnRc		= swap(Hydra_WriteRequest);
	message.Extra.Size	= swap(count);
	socket_write(link_fd, (BYTE *) &message, sizeof(Hydra_Message), "hydra");
	socket_write(link_fd, data, count, "hydra");
	
	hydra_get_reply(&message, FALSE, FALSE);

	if (message.FnRc eq Hydra_Broken)
		return(count);

	if (message.FnRc ne Hydra_WriteAck)
	{
  		ServerDebug("hydra_send_block, invalid fnrc %x from hydra", message.FnRc);
/*	  	longjmp(exit_jmpbuf, 1); */
		longjmp_exit;
	}

	return(0);
}

PRIVATE	WORD	hydra_byte_from_link(where)
UBYTE	*where;
{
	Hydra_Message	message;

	message.FnRc	= swap(Hydra_ReadByte);
	socket_write(link_fd, (BYTE *) &message, sizeof(Hydra_Message), "hydra");

	unless(Hydra_ByteAvailable)
	{
		hydra_get_reply(&message, TRUE, FALSE);

		if (message.FnRc eq Hydra_Broken)
			return(1);

		if (message.FnRc ne Hydra_ReadAck)
		{
			ServerDebug("hydra_byte_from_link, invalid fnrc %x from hydra", message.FnRc);
/*			longjmp(exit_jmpbuf, 1); */
			longjmp_exit;
		}
	}

		/* The data may have been sent before the reply, or it may be part	*/
		/* of the reply.							*/
	if (Hydra_ByteAvailable)
		*where	= (UBYTE) Hydra_ByteValue;
	else
		*where	= (UBYTE) message.Extra.Buf[0];

	Hydra_ByteAvailable	= FALSE;
	return(0);

}

PRIVATE WORD hydra_word_from_link(where)
BYTE *where;
{
	Hydra_Message	message;

	message.FnRc	= swap(Hydra_ReadWord);
	socket_write(link_fd, (BYTE *) &message, sizeof(Hydra_Message), "hydra");

	unless(Hydra_WordAvailable)
	{
		hydra_get_reply(&message, FALSE, TRUE);

		if (message.FnRc eq Hydra_Broken)
			return(1);

		if (message.FnRc ne Hydra_ReadAck)
		{
			ServerDebug("hydra_word_from_link, invalid fnrc %x from hydra", message.FnRc);
/*			longjmp(exit_jmpbuf, 1); */
			longjmp_exit;
		}
	}

		/* The data may have been sent before the reply, or it may be part	*/
		/* of the reply, or part of the data may be part of the reply.		*/
	if (Hydra_WordAvailable)
		memcpy(where, Hydra_WordValue, 4);
	elif (Hydra_ByteAvailable)
	{
		message.Extra.Buf[0] = Hydra_ByteValue;
		memcpy(where, message.Extra.Buf, 4);
	}
	else
		memcpy(where, message.Extra.Buf, 4);

	Hydra_ByteAvailable = Hydra_WordAvailable = FALSE;
	return(0);
}


PRIVATE WORD hydra_fetch_block(count,data,timeout)
int count;
char *data;
int timeout;
{
	Hydra_Message	message;

	if (count eq 4)
		return(hydra_word_from_link(data));

	message.FnRc		= swap(Hydra_ReadRequest);
	message.Extra.Size	= swap(count);
	socket_write(link_fd, (BYTE *) &message, sizeof(Hydra_Message), "hydra");

	hydra_get_reply(&message, FALSE, FALSE);

	if (message.FnRc eq Hydra_Broken)
		return(count);

	if (message.FnRc ne Hydra_ReadAck)
	{
  		ServerDebug("hydra_fetch_block, invalid fnrc %x from hydra", message.FnRc);
/*	  	longjmp(exit_jmpbuf, 1); */
		longjmp_exit;
	}

	if (Hydra_ByteAvailable)
	{
		data[0]	= Hydra_ByteValue;
		if (count > 1)
			socket_read(link_fd, &(data[1]), count - 1, "hydra");
	}
	elif (Hydra_WordAvailable)
	{
		memcpy(data, Hydra_WordValue, 4);
		if (count > 4)
			socket_read(link_fd, &(data[4]), count - 4, "hydra");
	}
	else
		socket_read(link_fd, data, count, "hydra");

	Hydra_ByteAvailable = Hydra_WordAvailable = FALSE;
	return(0);
} 

/**
*** Once the root processor has been booted I want to switch back to the
*** old protocol, whereby actual Helios messages are sent across the link.
*** However, certain requests such as reset implicitly switch back to the
*** new protocol.
**/
/**
*** This routine performs the actual switch. First it uses the new protocol
*** to update Hydra. Then it updates the function pointers.
**/
void Hydra_SwitchMode()
{
	Hydra_Message	message;
	message.FnRc		= swap(Hydra_MessageMode);
	message.Extra.Size	= swap(C40HalfDuplex);
	socket_write(link_fd, (BYTE *) &message, sizeof(Hydra_Message), "hydra");

	hydra_get_reply(&message, FALSE, FALSE);

	if (message.FnRc ne Hydra_MessageAck)
	{
		ServerDebug("Hydra_SwitchMode, invalid fnrc %x from hydra", message.FnRc);
/*		longjmp(exit_jmpbuf, 1); */
		longjmp_exit;
	}

		/* If Hydra has already sent a byte or a word it will process it	*/
		/* again.								*/
	Hydra_ByteAvailable = Hydra_WordAvailable = FALSE;

		/* Now update the function pointers.					*/
	free_link_fn		= vdin_fn_(Mhydra_free_link);
	byte_from_link_fn	= wdub_fn_(Mhydra_byte_from_link);
	byte_to_link_fn		= wdin_fn_(Mhydra_byte_to_link);
	send_block_fn		= wdinp_fn_(Mhydra_send_block);
	fetch_block_fn		= wdinp_fn_(Mhydra_fetch_block);
	reset_fn		= vdvd_fn_(Mhydra_reset);
	analyse_fn		= vdvd_fn_(Mhydra_analyse);
	rdrdy_fn		= wdvd_fn_(select_rdrdy);
}

PRIVATE void Hydra_SwitchBack()
{
	free_link_fn		= vdin_fn_(hydra_free_link);
	byte_from_link_fn	= wdub_fn_(hydra_byte_from_link);
	byte_to_link_fn		= wdin_fn_(hydra_byte_to_link);
	send_block_fn		= wdinp_fn_(hydra_send_block);
	fetch_block_fn		= wdinp_fn_(hydra_fetch_block);
	reset_fn		= vdvd_fn_(hydra_reset);
	analyse_fn		= vdvd_fn_(hydra_analyse);
	rdrdy_fn		= wdvd_fn_(hydra_rdrdy);
}

/**
*** These routines implement the old protocol where the traffic across the
*** socket consists of the same messages that are sent across the
*** link. There are special protocol bytes for reset, analyse and freelink.
*** For certain operations such as reset Hydra will automatically switch
*** back to the new protocol, because there is likely to be a bootstrap.
**/
PRIVATE void Mhydra_reset()
{
	if (target_processor eq Processor_C40)
	{
		int	x = swap(Proto_RemoteReset);
		socket_write(link_fd, (char *)(&x), 4, "hydra");
	}
	else
	{
		char	buf[1];
		buf[0]	= Proto_RemoteReset;
		socket_write(link_fd, buf, 1, "hydra");
	}
	Hydra_SwitchBack();
}

PRIVATE void Mhydra_analyse()
{
	if (target_processor eq Processor_C40)
	{
		int	x = swap(Proto_RemoteAnalyse);
		socket_write(link_fd, (char *)(&x), 4, "hydra");
	}
	else
	{
		char	buf[1];
		buf[0]	= Proto_RemoteAnalyse;
		socket_write(link_fd, buf, 1, "hydra");
	}
	Hydra_SwitchBack();
}

PRIVATE void Mhydra_free_link(i)
int i;
{
	if (link_table[i].fildes eq -1)
		return;

	if (target_processor eq Processor_C40)
	{
		int	x = swap(Proto_Close);
		socket_write(link_fd, (char *)(&x), 4, "hydra");
	}
	else
	{
		char	buf[1];
		buf[0]	= Proto_Close;
		socket_write(link_fd, buf, 1, "hydra");
	}
	Hydra_SwitchBack();
}

PRIVATE word Mhydra_byte_from_link(where)
UBYTE *where;
{
  socket_read(link_fd, (char *)where, 1, "hydra");
  return(0);
}

PRIVATE word Mhydra_byte_to_link(data)
int	data;
{
	char	buf[1];
	buf[0]	= (char) data;
	socket_write(link_fd, buf, 1, "hydra");
	return(0);
}

PRIVATE word Mhydra_fetch_block(count, data, timeout)
int	 count;
char	*data;
int	 timeout;
{
	socket_read(link_fd, data, count, "hydra");
	return(0);
	use(timeout);
}

PRIVATE word Mhydra_send_block(count, data, timeout)
int	 count;
char	*data;
int	 timeout;
{
	socket_write(link_fd, data, count, "hydra");
	return(0);
	use(timeout);
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

PRIVATE void fn( hydra_inet_init_link, (int));

#ifdef __cplusplus
extern "C"
{
#endif
int fn (gethostname, (char *, int));
#ifdef __cplusplus
}
#endif

PRIVATE void hydra_init_link()
{ int my_socket;
  char *family_name = get_config("family_name");
  int family_type = AF_UNIX;
  WORD site;
  socket_msg con;
  WORD retries = get_int_config("connection_retries");
  if (retries eq Invalid_config) retries = 5L;

#if (SM90 || TR5 || i486V4 || HP9000 || SOLARIS)
  if (gethostname(con.hostname,sizeof(con.hostname)) eq -1)
#else
  if (gethostname(con.hostname) eq -1)
#endif
   { ServerDebug("Unable to obtain host name : %q");
     if (errno <= sys_nerr)
      ServerDebug("%s", sys_errlist[errno]);
     else
      ServerDebug("unknown errno %d", errno);
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
   }

  if (cuserid(con.userid) eq NULL)
   { ServerDebug("Unable to obtain user name : %q");
     if (errno <= sys_nerr)
      ServerDebug("%s", sys_errlist[errno]);
     else
      ServerDebug("unknown errno %d", errno);
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
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
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
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
/*       longjmp(exit_jmpbuf, 1); */
       longjmp_exit;
     }
    link_fd = my_socket;

    if (family_type eq AF_UNIX)
     { struct sockaddr_un addr;
       char *socket_name = get_config("socket_name");

       addr.sun_family = AF_UNIX;
       strcpy(addr.sun_path,
              (socket_name eq (char *) NULL) ? "hydra.skt" : socket_name);
       if (connect(my_socket, (sockaddr *)(&addr),
          strlen(addr.sun_path) + sizeof(addr.sun_family) ) eq -1)
        { ServerDebug("Failed to connect to hydra : %q");
          if (errno <= sys_nerr)
           ServerDebug("%s", sys_errlist[errno]);
          else
           ServerDebug("unknown errno %d", errno);
/*          longjmp(exit_jmpbuf, 1); */
	  longjmp_exit;
        }     
     }
    else
     hydra_inet_init_link(my_socket);

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
/*       longjmp(exit_jmpbuf, 1); */
       longjmp_exit;
     }
    if (read(my_socket, &con, sizeof(socket_msg)) 
      < sizeof(socket_msg))
     { ServerDebug("Failed to read from hydra : %q");
       if (errno <= sys_nerr)
        ServerDebug("%s", sys_errlist[errno]);
       else
        ServerDebug("unknown errno %d", errno);
/*       longjmp(exit_jmpbuf, 1); */
       longjmp_exit;
     }

    if (swap(con.fnrc) eq Invalid_Link)
     { ServerDebug("Site %d is invalid according to Hydra.", site);
/*       longjmp(exit_jmpbuf, 1); */
       longjmp_exit;
     }

    if (swap(con.fnrc) eq Link_Unavailable)
     { if (site eq Invalid_config)
        ServerDebug("Hydra has no free sites.");
       else
        ServerDebug("Hydra has marked site %d as in use.", site);
/*       longjmp(exit_jmpbuf, 1); */
       longjmp_exit;
     }

    if (swap(con.fnrc) ne Hydra_Busy)
     { transputer_site = swap(con.fnrc);
       return;
     }

    ServerDebug("Hydra is busy...");
    if (retries > 1) sleep(5);
  }

  ServerDebug("Giving up on trying to connect to hydra.");
/*  longjmp(exit_jmpbuf, 1); */
  longjmp_exit;
}

PRIVATE void hydra_inet_init_link(my_socket)
int my_socket;
{ struct sockaddr_in addr;
  struct hostent *hp;
  struct servent *sp;
  char *host_name   = get_config("hydra_host");

  if (host_name eq (char *) NULL)
   { ServerDebug("Missing entry in host.con file for hydra_host.");
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
   }

  hp = gethostbyname(host_name);
  if (hp eq NULL)
   { ServerDebug("Unable to obtain address for hydra host %s", host_name);
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
   }

  sp = getservbyname("hydra", "tcp");
  if (sp eq NULL)
   { ServerDebug("Unable to obtain a port for the hydra server.");
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
   }

#if SOLARIS
  memset ((void *)(&addr), 0, sizeof (struct sockaddr_in));
  memcpy ((void *)(&addr.sin_addr), hp -> h_addr, hp -> h_length);
#else
  bzero((char *) &addr, sizeof(struct sockaddr_in));
  bcopy(hp->h_addr, (char *) &addr.sin_addr, hp->h_length);
#endif

  addr.sin_family = hp->h_addrtype;
  addr.sin_port   = sp->s_port;

  if (connect(my_socket, (sockaddr *)(&addr), sizeof(struct sockaddr_in)) eq -1)
   { ServerDebug("Failed to connect to hydra : %q");
     if (errno <= sys_nerr)
      ServerDebug("%s", sys_errlist[errno]);
     else
      ServerDebug("unknown errno %d", errno);
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
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
#if (!SOLARIS)

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
BYTE *where;
{ 
  if ((*inmos_read_link_fn)(link_fd, where, 1, 5) < 1)
   return(1);  /* amount failed to read */
  return(0);
}

PRIVATE int inmos_byte_to_link(data)
int data;
{ BYTE buf[1];
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

#endif /* (!SOLARIS) */
