/*
 --   ---------------------------------------------------------------------------
 --
 --      ISERVER  -  INMOS standard file server
 --
 --      iserver.h
 --
 --      Server definitions
 --
 --      Copyright (c) INMOS Ltd., 1988, 1989, 1990.
 --      All Rights Reserved.
 --
 --   ---------------------------------------------------------------------------
*/

#ifndef _INMOS_H
#include "inmos.h"
#endif

#define PROGRAM_NAME  "iserver"
#define VERSION_NAME  "1.42d of 1st Feb, 1990"
#define VERSION_ID     14




#define TRANSACTION_BUFFER_SIZE   1024
#define MAX_SLICE_LENGTH          (TRANSACTION_BUFFER_SIZE - 2 - 1)        /*  largest data item in Tbuf  */

#define BOOT_BUFFER_LENGTH        (1024 * 8)

#define DEFAULT_CORE_SIZE         (1024 * 8)    /*  peeked at analyse  */
#define TIMEOUT                   (5)                                      /*  basic transaction timeout in 10ths of a second  */
#define BOOT_TIMEOUT              (10)

#define MAX_COMMAND_LINE_LENGTH   512
#define MAX_BOOT_FILE_LENGTH      256
#define MAX_LINK_NAME_LENGTH      64


#define PEEK_TRANSPUTER           1



									   /*  primary server operation tags  */

#define SP_ZERO        0

#define SP_OPEN       10                                                   /*  filec.c  */
#define SP_CLOSE      11
#define SP_READ       12
#define SP_WRITE      13
#define SP_GETS       14
#define SP_PUTS       15
#define SP_FLUSH      16
#define SP_SEEK       17
#define SP_TELL       18
#define SP_EOF        19
#define SP_FERROR     20
#define SP_REMOVE     21
#define SP_RENAME     22
#define SP_GETBLOCK   23
#define SP_PUTBLOCK   24

#define SP_GETKEY     30                                                   /*  hostc.c  */
#define SP_POLLKEY    31
#define SP_GETENV     32
#define SP_TIME       33
#define SP_SYSTEM     34
#define SP_EXIT       35

#define SP_COMMAND    40                                                   /*  serverc.c  */
#define SP_CORE       41
#define SP_ID         42

#define SP_MSDOS      50   /*  msdos.c  */

/* NOT USED AT INMOS - MAY BE USED BY THIRD PARTY ??? 
#define SP_SUN        60
#define SP_MSC        61
#define SP_VMS        62
*/

#define SP_ALSYS      100  /* Not used by inmos iserver */
#define SP_KPAR       101  /* Not used by inmos iserver */

									   /*  INMOS reserves all numbers up to 127  */




#define SP_SUCCESS 0                                                       /*  operation results  */
#define SP_UNIMPLEMENTED 1
#define SP_ERROR 129

#define ER_LINK_BAD    (-1)                                                  /*  Failure codes for LinkIO functions */
#define ER_LINK_CANT   (-2)
#define ER_LINK_SOFT   (-3)
#define ER_LINK_NODATA (-4)
#define SUCCEEDED      (1)   


									   /*  machine specific stuff  */
#ifndef MSC
#ifndef VMS
#ifndef SUN
#define UNDEFINED_HOST
#endif
#endif
#endif


#ifdef SUN
#define SWITCH_CHAR '-'
#endif
#ifdef MSC
#define SWITCH_CHAR '/'
#endif
#ifdef VMS
#define SWITCH_CHAR '/'
#endif
#ifdef UNDEFINED_HOST
#define SWITCH_CHAR '-'
#endif


									   /*  exit status  */
#ifdef VMS
globalvalue iserv_success;
globalvalue iserv_break;
globalvalue iserv_break;
globalvalue iserv_other;
globalvalue iserv_errorflag;
globalvalue iserv_misc;
globalvalue iserv_fail;

#define TERMINATE_OK_EXIT     (iserv_success)
#define TERMINATE_FAIL_EXIT   (iserv_fail)
#define USER_EXIT             (iserv_break)
#define ERROR_FLAG_EXIT       (iserv_errorflag)
#define MISC_EXIT             (iserv_misc)
#define TERMINATE_OTHER_STATUS   (iserv_other)
#endif
#ifdef MSC
#define TERMINATE_OK_EXIT     (0)                 
#define TERMINATE_FAIL_EXIT   (1)
#define USER_EXIT             (2)
#define ERROR_FLAG_EXIT       (3)
#define MISC_EXIT             (4)
#define TERMINATE_OTHER_STATUS   (5)
#endif
#ifdef SUN
#define TERMINATE_OK_EXIT     (0)
#define TERMINATE_FAIL_EXIT   (1)
#define USER_EXIT             (2)
#define ERROR_FLAG_EXIT       (3)
#define MISC_EXIT             (4)
#define TERMINATE_OTHER_STATUS   (5)
#endif
#ifdef UNDEFINED_HOST            
#define TERMINATE_OK_EXIT     (0)
#define TERMINATE_FAIL_EXIT   (1)
#define USER_EXIT             (2)
#define ERROR_FLAG_EXIT       (3)
#define MISC_EXIT             (4)
#define TERMINATE_OTHER_STATUS   (5)
#endif


#ifdef MSC                                                                 /*  DOS cant redirect stderr  */
#define STANDARD_ERROR stdout
#else
#define STANDARD_ERROR stderr
#endif




									   /*  all this is for SpId  */
#define BOX_X      0
#define BOX_PC     1
#define BOX_NEC    2
#define BOX_VAX    3
#define BOX_SUN3   4
#define BOX_S370   5
#define BOX_SUN4   6
#define BOX_SUN386 7
#define BOX_APOLLO 8

#define OS_X       0
#define OS_DOS     1
#define OS_HELIOS  2
#define OS_VMS     3
#define OS_SUN40   4
#define OS_CMS     5

#ifdef sun3
#define HOST         "Sun3/SunOS4.0"
#define HOST_ID      BOX_SUN3
#define OS_ID        OS_SUN40
#endif

#ifdef sun4
#define HOST         "Sun4/SunOS4.0"
#define HOST_ID      BOX_SUN3
#define OS_ID        OS_SUN40
#endif

#ifdef sun386
#define HOST         "Sun386/SunOS4.0"
#define HOST_ID      BOX_SUN3
#define OS_ID        OS_SUN40
#endif

#ifdef MSC
#define HOST         "IBM-PC/MS-DOS"
#ifdef NEC
#undef HOST
#define HOST         "NEC-PC/MS-DOS"
#endif
#define HOST_ID      BOX_PC
#define OS_ID        OS_DOS
#endif

#ifdef VMS
#define HOST         "VAX/VMS"
#define HOST_ID      BOX_VAX
#define OS_ID        OS_VMS
#endif

#ifdef HELIOS
#define HOST         "HELIOS 1.0"
#define HOST_ID      BOX_X
#define OS_ID        OS_HELIOS
#endif

#ifndef HOST_ID
#define HOST         "???"
#define HOST_ID BOX_X
#endif

#ifndef OS_ID
#define OS_ID OS_X
#endif

#define HW_X      0
#define B004      1
#define B008      2 
#define B010      3
#define B011      4
#define B014      5
#define DRX11     6
#define QT0       7
#define B015      8
#define IBM_CAT   9
#define B016      10
#define UDP       11

#ifndef BOARD_ID
#define BOARD_ID HW_X
#endif



									   /*  some global inlines  */
#define DEBUG(x) { if (CocoPops) { fputs("(", stdout); printf x; fputs(")", stdout); fputc((VerboseSwitch ? '\n' : ' '), stdout); fflush(stdout); } }
#define INFO(x) { if (VerboseSwitch) printf x ; fflush(stdout); }
#define SE STANDARD_ERROR
#define ABORT(xit_code, x) { fprintf(STANDARD_ERROR, "\nError - %s - ", PROGRAM_NAME); fprintf x; fputs(".\n",STANDARD_ERROR); if (TheLink != -1 ) CloseLink(TheLink); HostEnd(); exit(xit_code); } 



/*
 *   Eof
 */

