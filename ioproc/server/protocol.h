/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--   protocol.h                                                         --
--                                                                      --
--    A header file defining the Helios protocols used by the Server.   --
--    Based on the Helios header files in /helios/include/syslib.h,     --
--    but modified to cope with 16-bit compilers.                       --
--                                                                      --
--    Author:  BLV 21/1/89                                              --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: protocol.h,v 1.11 1994/06/29 13:42:25 tony Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.        			*/

/*------------------------------------------------------------------------
--                                                                      --
-- helios.h                                                             --
--                                                                      --
------------------------------------------------------------------------*/

/* standard type definitions */

#if TR5
	/* 64 bit longs I assume */
typedef  int            WORD    ;       /* a machine word, 32 bits      */
typedef  unsigned int   UWORD   ;       /* a machine word, 32 bits      */
typedef  WORD           INT     ;       /* a synonym                    */
typedef  WORD           word    ;       /* another synonym              */
typedef  WORD           Offset  ;       /* and another                  */

#else

#if (!MSWINDOWS)
	/* MSWINDOWS already defines WORD and BYTE... */
typedef  long           WORD    ; 
#endif

typedef  unsigned long  UWORD   ;       /* a machine word, 32 bits      */
typedef  long           INT     ;       /* a synonym                    */
typedef  long           word    ;       /* another synonym              */
typedef  long           Offset  ;       /* and another                  */
#endif

typedef  short int      SHORT   ;       /* a 16 bit word                */
typedef  unsigned short USHORT  ;       /* an unsigned 16 bit value     */

#if (!MSWINDOWS)
typedef  char		BYTE;
#endif
typedef  char           byte    ;       /* a synonym                    */
typedef  unsigned char  UBYTE   ;       /* an unsigned byte             */
typedef  char           *STRING ;       /* character string             */
typedef  char           *string ;       /* synonym                      */
typedef  word           bool    ;       /* boolean value                */

typedef  void           (*VoidFnPtr)(); /* pointer to void function     */
typedef  word           (*WordFnPtr)(); /* pointer to word function     */

#define PUBLIC          extern          /* an exported symbol           */
#define PRIVATE         static          /* an unexported symbol         */
#define FORWARD         extern          /* forward proc reference       */


/* Syntactic enrichment... */

#define forever         for(;;)
#define unless(x)       if(!(x))
#define until(x)        while(!(x))
#define elif(x)         else if(x)

#ifndef TRUE
#define TRUE            1l
#endif
#define true            1l
#ifndef FALSE
#define FALSE           0l
#endif
#define false           0l
#define Variable        1
#define MinInt          0x80000000L

#define MemStart  MinInt+0x70L
#define LoadBase  (MinInt+0x1000L)

#define Null(_type) ((_type *)NULL)

#define NameMax   32
#define c_dirchar '/'                   /* Helios directory separator */

#define OneSec           1000000L       /* one second in micro-seconds */

/* end of iohelios.h */

/*------------------------------------------------------------------------
--                                                                      --
-- ioattrib.h                                                           --
--                                                                      --
------------------------------------------------------------------------*/

typedef struct Attributes {
                            word  Input;
                            word  Output;
                            word  Control;
                            word  Local;
#if swapping_needed
                            short Time;
                            short Min;
#else
                            short Min;
                            short Time;
#endif
} Attributes;

typedef word Attribute;

#define ConsoleEcho           0x00000007L
#define ConsoleIgnoreBreak    0x00000100L
#define ConsoleBreakInterrupt 0x00000200L
#define ConsolePause          0x00000400L
#define ConsoleRawInput       0x0000000bL
#define ConsoleRawOutput      0x00000101L

#define RS232_IgnPar          0x00000800L
#define RS232_ParMrk          0x00001000L
#define RS232_InPck           0x00002000L
#define RS232_IXON            0x00004000L
#define RS232_IXOFF           0x00008000L
#define RS232_Istrip          0x00010000L
#define RS232_IgnoreBreak     0x00000100L
#define RS232_BreakInterrupt  0x00000200L
#define RS232_Cstopb          0x00000102L
#define RS232_Cread           0x00000202L
#define RS232_ParEnb          0x00000402L
#define RS232_ParOdd          0x00000802L
#define RS232_HupCl           0x00001002L
#define RS232_CLocal          0x00002002L
#define RS232_Csize           0x0003C000L   /* Mask for the sizes */
#define RS232_Csize_5         0x00004002L
#define RS232_Csize_6         0x00008002L
#define RS232_Csize_7         0x00010002L
#define RS232_Csize_8         0x00020002L

#define RS232_B0              0
#define RS232_B50             1
#define RS232_B75             2
#define RS232_B110            3
#define RS232_B134            4
#define RS232_B150            5
#define RS232_B200            6
#define RS232_B300            7
#define RS232_B600            8
#define RS232_B1200           9
#define RS232_B1800          10
#define RS232_B2400          11
#define RS232_B4800          12
#define RS232_B9600          13
#define RS232_B19200         14
#define RS232_B38400         15
#define RS232_B56000         16

/* end of ioattrib.h */

/*------------------------------------------------------------------------
--                                                                      --
-- iocodes.h                                                            --
--                                                                      --
------------------------------------------------------------------------*/

/*----------------------------------------------------------------
-- Subsystems
----------------------------------------------------------------*/

#define SS_Mask         0x1f000000L

#define SS_Unknown      0x00000000L
#define SS_Kernel       0x01000000L
#define SS_SysLib       0x02000000L
#define SS_ProcMan      0x03000000L
#define SS_Loader       0x04000000L
#define SS_TFM          0x05000000L
#define SS_RamDisk      0x06000000L
#define SS_HardDisk     0x07000000L
#define SS_Fifo         0x08000000L
#define SS_NameTable    0x09000000L
#define SS_IOProc       0x0A000000L
#define SS_Window       0x0B000000L
#define SS_IOC          0x0C000000L
#define SS_NullDevice   0x0d000000
#define SS_Pipe         0x0e000000
#define SS_Batch        0x0f000000
#define SS_Login        0x10000000
#define SS_NetServ      0x11000000
#define SS_SM           0x12000000
#define SS_Device       0x13000000
#define SS_InterNet     0x14000000

/*----------------------------------------------------------------
-- Function Codes
----------------------------------------------------------------*/
/*----------------------------------------------------------------
-- Function Classes
----------------------------------------------------------------*/

#define FC_Mask         0x60000000L
#define FC_GSP          0x00000000L
#define FC_Private      0x60000000L

/*----------------------------------------------------------------
-- Retry Counter
----------------------------------------------------------------*/

#define FR_Mask         0x00F00000
#define FR_Inc          0x00100000

/*----------------------------------------------------------------
-- General Functions
----------------------------------------------------------------*/

#define FG_Mask         0x00FFFFF0L

/* IOC requests */
#define FG_Unknown      0x00000000L
#define FG_Open         0x00000010L
#define FG_Create       0x00000020L
#define FG_Locate       0x00000030L
#define FG_ObjectInfo   0x00000040L
#define FG_ServerInfo   0x00000050L
#define FG_Delete       0x00000060L
#define FG_Rename       0x00000070L
#define FG_Link         0x00000080L
#define FG_Protect      0x00000090L
#define FG_SetDate      0x000000a0L
#define FG_Refine       0x000000b0L
#define FG_CloseObj     0x000000c0L

/* direct server requests */
#define FG_Read                 0x00001010L
#define FG_Write                0x00001020L
#define FG_GetSize              0x00001030L
#define FG_SetSize              0x00001040L
#define FG_Close                0x00001050L
#define FG_Seek                 0x00001060L
#define FG_GetAttr              0x00001070L
#define FG_SetAttr              0x00001080L
#define FG_EnableEvents         0x00001090L
#define FG_Acknowledge          0x000010A0L
#define FG_NegAcknowledge       0x000010B0L
#define FG_Select               0x000010C0L

/* Distributed search codes */
#define FG_Search               0x00002010L
#define FG_FollowTrail          0x00002020L

/*Socket Related Requests*/
#define FG_Socket               0x00008010  /* create socket */
#define FG_Bind                 0x00008020  /* bind socket to address */
#define FG_Listen               0x00008030  /* set socket connection queue size */
#define FG_Accept               0x00008040  /* accept a connection */
#define FG_Connect              0x00008050  /* connect to a remote service */
#define FG_SendMessage          0x00008060  /* send datagram or other message */
#define FG_RecvMessage          0x00008070  /* receieve datagram or other message */

/*Device Requests*/
#define FG_Format               0x0000a010  /* format disc */
#define FG_WriteBoot            0x0000a020  /* write boot area */

/* General Server Terminate */
#define FG_Terminate            0x00001FF0
#define FG_Reboot               0x00002FF0

/*----------------------------------------------------------------
-- Error Codes
----------------------------------------------------------------*/

#define ErrBit          0x80000000L     /* set for all error codes */
#define Err_Null        0L              /* no error at all         */

/*----------------------------------------------------------------
-- Error Classes
----------------------------------------------------------------*/

#define EC_Mask         0xe0000000L

#define EC_Recover      0x80000000L      /* a retry might succeed */
#define EC_Warn         0xA0000000L      /* recover & try again   */
#define EC_Error        0xC0000000L      /* client fatal          */
#define EC_Fatal        0xE0000000L      /* system fatal          */

/*----------------------------------------------------------------
-- General Error codes
----------------------------------------------------------------*/

#define EG_Mask         0x00FF0000L      /* mask to isolate             */

#define EG_UnknownError 0x00000000L
#define EG_NoMemory     0x00010000L     /* memory allocation failure    */
#define EG_Create       0x00020000L     /* failed to create             */
#define EG_Delete       0x00030000L     /* failed to delete             */
#define EG_Protected    0x00040000L     /* object is protected          */
#define EG_Timeout      0x00050000L     /* timeout                      */
#define EG_Unknown      0x00060000L     /* object not found             */
#define EG_FnCode       0x00070000L     /* unknown function code        */
#define EG_Name         0x00080000L     /* mal-formed name              */
#define EG_Invalid      0x00090000L     /* invalid/corrupt object       */
#define EG_InUse        0x000a0000L     /* object in use/locked         */
#define EG_Congested    0x000b0000L     /* server/route overloaded      */
#define EG_WrongFn      0x000c0000L     /* fn inappropriate to object   */
#define EG_Broken       0x000d0000L     /* object broken in some way    */
#define EG_Exception    0x000e0000L     /* exception message            */
#define EG_WrongSize    0x000f0000L     /* object wrong size            */
#define EG_ReBooted     0x00100000L     /* server/processor rebooted    */
#define EG_Open         0x00110000L
#define EG_Execute      0x00120000L
#define EG_Boot         0x00130000L
#define EG_State        0x00140000L
#define EG_NoResource   0x00150000L
#define EG_Errno        0x00160000L
#define EG_Parameter    0x00ff0000L     /* bad parameter value          */

/*----------------------------------------------------------------
-- Object codes for general errors
----------------------------------------------------------------*/

#define EO_Unknown       0x00000000L
#define EO_Message       0x00008001L     /* error refers to a message    */
#define EO_Task          0x00008002L     /* error refers to a task       */
#define EO_Port          0x00008003L     /* error refers to a port       */
#define EO_Route         0x00008004L     /* error refers to a route      */
#define EO_Directory     0x00008005L     /* error refers to a directory  */
#define EO_Object        0x00008006L     /* error refers to Object struct*/
#define EO_Stream        0x00008007L     /* error refers to Stream       */
#define EO_Program       0x00008008L
#define EO_Module        0x00008009L
#define EO_Matrix        0x0000800aL     /* access matrix                */
#define EO_Fifo          0x0000800bL
#define EO_File          0x0000800cL
#define EO_Capability    0x0000800dL
#define EO_Name          0x0000800eL     /* name in name table           */
#define EO_Window        0x0000800fL
#define EO_Server        0x00008010L
#define EO_TaskForce     0x00008011L
#define EO_Link          0x00008012L
#define EO_Memory        0x00008013L
#define EO_Pipe          0x00008014L
#define EO_NetServ       0x00008015L     /* error refers to NS           */
#define EO_Subnetwork    0x00008016L     /* error refers to Subnetwork   */
#define EO_User          0x00008017L
#define EO_Session       0x00008018L
#define EO_Loader        0x00008019L
#define EO_ProcMan       0x0000801AL
#define EO_TFM           0x0000801BL
#define EO_Attribute     0x0000801CL
#define EO_NoProcessors  0x0000801DL
#define EO_ProcessorType 0x0000801EL
#define EO_Processor     0x0000801FL
#define EO_Socket        0x00008020L
#define EO_Medium        0x00008021L

/*----------------------------------------------------------------------*/
/*-- Exception codes                                                  --*/
/*----------------------------------------------------------------------*/

#define	EE_Mask		    0x0000ffff
#define	EE_Shift	    0

#define	EE_Null		    0x00000000
#define	EE_Kill		    0x00000004
#define	EE_Abort	    0x00000005
#define	EE_Suspend	    0x00000006
#define	EE_Restart	    0x00000007
#define	EE_Interrupt	    0x00000008
#define	EE_ErrorFlag	    0x00000009
#define	EE_StackError	    0x0000000a
#define	EE_Signal	    0x00007f00

/* End of iocodes.h */

/*------------------------------------------------------------------------
--                                                                      --
-- ioprot.h                                                             --
--                                                                      --
------------------------------------------------------------------------*/

typedef unsigned long   Matrix;    /* access matrix */
typedef UBYTE           AccMask;   /* access mask   */
typedef word            Key;       /* encryption key */

/*----------------------------------------------------------------------*/
/* Access capability                                                    */
/*----------------------------------------------------------------------*/

typedef struct Capability {
        AccMask         Access;   /* access mask      */
        UBYTE           Valid[7]; /* validation value */
} Capability;

/*----------------------------------------------------------------------*/
/* Access mask bits                                                     */
/*----------------------------------------------------------------------*/

#define AccMask_Full    0xff            /* all bits set       */

/* All */
#define AccMask_R       0x01            /* Read permission   */
#define AccMask_W       0x02            /* Write permission  */
#define AccMask_D       0x40            /* Delete permission */
#define AccMask_A       0x80            /* Alter permission  */

/* Files only */
#define AccMask_E       0x04            /* Execute permission */
#define AccMask_F       0x08            /* unused - arbitrary letters */
#define AccMask_G       0x10
#define AccMask_H       0x20

/* Directories only */
#define AccMask_V       0x04            /* V access category  */
#define AccMask_X       0x08            /* X access category  */
#define AccMask_Y       0x10            /* Y access category  */
#define AccMask_Z       0x20            /* Z access category  */

/* Tasks only */
#define AccMask_K       AccMask_D       /* Kill task (== Delete) */

/*----------------------------------------------------------------------*/
/* Access Matrix category masks                                         */
/*----------------------------------------------------------------------*/

#define AccMatrix_V     0x000000ffL
#define AccMatrix_X     0x0000ff00L
#define AccMatrix_Y     0x00ff0000L
#define AccMatrix_Z     0xff000000L

/*----------------------------------------------------------------------*/
/* Printed matrix letters                                               */
/*----------------------------------------------------------------------*/

#define FileChars   "rwefghda"
#define DirChars    "rwvxyzda"
#define TaskChars   "rw????ka"
#define ModChars    "r?e???da"
#define ProgChars   "rwe???da"

/*----------------------------------------------------------------------*/
/* Default Matrices                                                     */
/*----------------------------------------------------------------------*/

#define DefDirMatrix    0x21134BC7L     /* DARWV/DRWX/RWY/RZ */
#define DefFileMatrix   0x010343C3L     /* DARW/DRW/RW/R     */
#define DefLinkMatrix   0x201088C4L     /* dav:dx:y:z        */
#define DefTFMatrix     0x21134BC7L     /* darwv:drwx:rwy:rz */
#define DefTaskMatrix   0x010343C3L     /* darw:drw:rw:r     */
#define DefModuleMatrix 0x010545C5L     /* dare:dre:re:r     */
#define DefProgMatrix   0x010545C5L     /* dare:dre:re:r     */
#define DefNameMatrix   0x21110907L     /* rwv:rx:ry:rz      */
#define DefRootMatrix   0x21130B87L     /* arwv:rwx:rwy:rz   */


/* End of ioprot.h */

/*------------------------------------------------------------------------
--                                                                      --
-- iomess.h                                                             --
--                                                                      --
------------------------------------------------------------------------*/

/* Message port */

typedef UWORD           Port;           /* true structure hidden      */
#define NullPort        ((Port)0L)      /* zero is never a valid port */

typedef struct MsgHdr {
#if swapping_needed
        byte            Flags;          /* flag byte           */
        byte            ContSize;       /* control vector size */
        USHORT          DataSize;       /* 16 bit data size    */
#else
        USHORT          DataSize;
        byte            ContSize;
        byte            Flags;
#endif
        Port            Dest;           /* destination port descriptor */
        Port            Reply;          /* reply port descriptor       */
        word            FnRc;           /* function/return code        */
} MsgHdr;

#define MsgHdr_Flags_nothdr     0x80    /* used by kernel              */
#define MsgHdr_Flags_preserve   0x40    /* preserve destination route  */
#define MsgHdr_Flags_exception  0x20    /* exception message           */
#define MsgHdr_Flags_sacrifice  0x10    /* kernel may throw message away */
#define MsgHdr_Flags_bytesex    0x08    /* 0 = even, 1 = odd           */
#define MsgHdr_Flags_bytealign  0x03    /* used by C40 for non word    */
					/* aligned data */

/* Message control block */

typedef struct MCB {
        MsgHdr          MsgHdr;         /* message header buffer       */
        word            Timeout;        /* message timeout             */
        word            *Control;       /* pointer to control buffer   */
        byte            *Data;          /* pointer to data buffer      */
} MCB;

/* -- End of iomess.h */

/*------------------------------------------------------------------------
--                                                                      --
--      iogsp.h                                                         --
--                                                                      --
------------------------------------------------------------------------*/

/* offsets within the control vector for GSP requests   */      

#define Context_off          0   /* offsets in the control vector for all */
#define Pathname_off         1   /* directory requests */
#define Nextname_off         2
#define Cap1_off             3
#define Cap2_off             4
#define cont_minsize         5

#define arg1_off             5  /* additional offsets for directory requests */
#define arg2_off             6
#define arg3_off             7
#define arg4_off             8

#define OpenMode_off         5  /* additional offsets for individual requests */
#define CreateType_off       5
#define CreateSize_off       6
#define CreateInfo_off       7
#define RenameToname_off     5
#define LinkPathname_off     5
#define LinkCap1_off         6
#define LinkCap2_off         7
#define ProtectNewmatrix_off 5
#define RefineAccessMask_off 5
#define SetDateDate_off      7  /* Only interested in Modified */

#define ReadPos_off          0   /* offsets for stream requests */
#define ReadSize_off         1
#define ReadTimeout_off      2
#define WritePos_off         0
#define WriteSize_off        1
#define WriteTimeout_off     2
#define SeekPos_off          0
#define SeekMode_off         1
#define SeekNewPos_off       2
#define SetFileSizeSize_off  0
#define EnableEventsMask_off 0
#define AcknowledgeCount_off 0
#define NegAcknowledgeCount_off 0

#define Reply1_off           0     /* plus offsets for replies */
#define Reply2_off           1
#define Reply3_off           2
#define Reply4_off           3
#define Reply5_off           4
#define Reply6_off           5

#define open_reply           6L    /* size of a reply to open, locate, create */

/*
 * The reply to a read will consist of an arbitrary number of messages
 * containing the requested data, or a failure message. In addition to
 * possible error codes, the FnRc field of these messages will contain
 * a sequence number starting from 16 and incrementing in steps of 16
 * (Thus leaving the lower 4 bits clear). These lower 4 bits contain one
 * of the following values.
 */

#define ReadRc_Mask     0xfL    /* mask for lower 4 bits                */
#define ReadRc_More     0L      /* more data to come                    */
#define ReadRc_EOD      1L      /* end of data                          */
#define ReadRc_EOF      2L      /* end of data and of file              */

#define ReadRc_SeqInc   16L     /* increment for sequence numbers       */

/*
 * The first reply to a write will consist of the following structure
 * telling the sender how to format the data transfer. This is so copies
 * may be eliminated at the server end.
 * Once the data has been sent a second reply is made confirming that the
 * data were received.
 */

#define WriteRc_Done    0L
#define WriteRc_Sizes   1L

/*----------------------------------------------------------------------*/
/* some timeout constants                                               */
/*----------------------------------------------------------------------*/

                                      /* a 30-minute timeout in ticks */
#define DefaultStreamTimeout ((word) 30L * 60L * OneSec)
#define IOCDataMax    512             /* maximum size of full filenames   */
#define ControlMax    16              /* maximum size of a control vector */
#define MAXTIME       0x7FFFFFFFL     /* for timeouts of -1 (infinity)  */


/*----------------------------------------------------------------------*/
/* Object Types                                                         */
/*----------------------------------------------------------------------*/

/* bottom 4 bits are flags for major sub type                           */

#define Type_Directory  1L      /* supports GSP directory interface     */
#define Type_Stream     2L      /* supports GSP stream interface        */
#define Type_Private    4L      /* own private protocol                 */
                                /* remaining bit reserved               */

#define Type_File       0x12L   /*(0x10L+Type_Stream)*/
#define Type_Fifo       0x22L   /*(0x20L+Type_Stream)*/
#define Type_Module     0x32L   /*(0x30L+Type_Stream)*/
#define Type_Program    0x42L   /*(0x40L+Type_Stream)*/
#define Type_Task       0x52L   /*(0x50L+Type_Stream)*/
#define Type_Link       (0x60L)
#define Type_Name       (0x70L)
#define Type_TaskForce  0x81L   /*(0x80|Type_Directory)*/
#define Type_LTaskForce 0x91L   /*(0x90|Type_Directory)*/
#define Type_CacheName  (0xa0L)
#define Type_Pipe       0xb2L   /*(0xb0|Type_Stream)*/
#define Type_Pseudo     0xc2L   /*(0xc0|Type_Stream)*/
#define Type_Device     0xd4L   /*(0xd0|Type_Private)*/
#define Type_Session    0xe0L
#define Type_Socket     0xf0L
/**
*** The reply to Open, Locate and Create requests
**/
typedef struct IOCReply1 {
    word        Type;       /* object type code */
    word        Flags;      /* flag word        */
    byte        Access[8];  /* a capability for it */
    Offset      Pathname;   /* full pathname of object */
    word        Object;     /* object value if no reply port*/
} IOCReply1;


/* end of iogsp.h */

/*------------------------------------------------------------------------
--                                                                      --
--      iosyslib.h                                                      --
--                                                                      --
------------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Directory Entry structure                                            */
/*----------------------------------------------------------------------*/
typedef struct { word           Type;
                 word           Flags;
                 Matrix         Matrix;
                 char           Name[32];
} DirEntry;

/*----------------------------------------------------------------------*/
/* Generic Object Info structure                                        */
/*----------------------------------------------------------------------*/
/* data structure returned in response to an object info request. The structure
   is returned in the data vector of the message, with all the words swapped as
   necessary to get them to work on the transputer.
*/
typedef struct ObjInfo {
        DirEntry        DirEntry;       /* re-iteration of common info  */
        word            Account;        /* accounting identifier        */
        word            Size;           /* object size in bytes         */
        time_t          Creation;       /* three date stamps            */
        time_t          Access;
        time_t          Modified;
} ObjInfo;

/*----------------------------------------------------------------------*/
/* Modes for Open                                                       */
/*----------------------------------------------------------------------*/

#define O_ReadOnly      1L              /* For the Helios side */
#define O_WriteOnly     2L
#define O_ReadWrite     3L
#define O_Execute       4L
#define O_Create        0x0100L          /* create if does not exist   */
#define O_Exclusive     0x0200L          /* get exclusive access       */
#define O_Truncate      0x0400L          /* truncate if already exists */
#define O_NonBlock      0x0800L          /* do not block on read/write */
#define O_Append        0x1000L          /* append data (not to be used */
#define O_Sync          0x2000L          /* synchronous transfers */

/* The flags array of the Select call contains the standard bits in the */
/* bottom 2 bits, plus the following bits...                            */

#define O_Exception	0x04		 /* select for exception */
#define O_Selected      0x10             /* set if stream ready  */

/*----------------------------------------------------------------------*/
/* Modes for Seek                                                       */
/*----------------------------------------------------------------------*/

#define S_Beginning     0L              /* relative to start of file    */
#define S_Relative      1L              /* relative to current pos      */
#define S_End           2L              /* relative to end of file      */
#define S_Last          3L              /* relative to last operation   */

/*----------------------------------------------------------------------*/
/* Flag Bits                                                            */
/*----------------------------------------------------------------------*/

/* First column of comments indicates whose responsibility the state    */
/* of the bit is: S = Server, L = Syslib.                               */

#define Flags_Mode         0x0000000fL  /* L copy of open mode      */
#define Flags_More         0x00000010L  /* S More info available    */
#define Flags_Seekable     0x00000020L  /* S stream is seekable     */
#define Flags_Remote       0x00000040L  /* L server is non-local    */
#define Flags_StripName    0x00000080L  /* S names are stripped before pass on */
#define Flags_CacheName    0x00000100L  /* S name is cached         */
#define Flags_LinkName     0x00000200L  /* S name is for h/w link   */
#define Flags_PStream      0x00000400L  /* L set for PseudoStreams  */
#define Flags_ResetContext 0x00000800L  /* S set for remote servers */
#define Flags_Pipe         0x00001000L  /* S set for pipes          */
#define Flags_CloseOnSend  0x00002000L  /* S close in SendEnv       */
#define Flags_OpenOnGet    0x00004000L  /* S open in GetEnv         */
#define Flags_Selectable   0x00008000L  /* S can be used in Select  */

#define Flags_Interactive 0x00010000L   /* S if stream is interactive   */
#define Flags_MSdos       0x00020000L   /* S MSdos format files         */

#define Flags_CloseMask 0xe0000000L     /*   mask for following flags   */
#define Flags_Closeable 0x20000000L     /* S set if needs closing       */
#define Flags_Server    0x40000000L     /* L set if served stream       */
#define Flags_Stream    0x80000000L     /* L set if stream structure    */

#define closebits_(x)    ((((UWORD)(x))>>Flags_CloseShift)&0x7)

/* end of iosyslib.h */

/*------------------------------------------------------------------------
--                                                                      --
--  ioaddon.h                                                           --
--                                                                      --
------------------------------------------------------------------------*/

#define getfnrc(request)   (((request)->MsgHdr).FnRc & FG_Mask)

#define ReqDie             0x00FFFFF0L         /* still necessary */

#define ReplyOK            (Err_Null)

/* The reply to a ServerInfo request */

typedef struct { word type;        /* always Type_Directory */
                 word size;        /* size in bytes */
                 word used;        /* how many used */
                 word alloc;       /* unit of allocation in bytes */
} servinfo;

/**
*** These are additional error codes for floppies
**/

#define floppy_invalid   1        /* catch-all error code       */
#define floppy_protected 2        /* write-protected            */
#define floppy_removed   3        /* disk removed during access */
#define floppy_full      4        /* insufficient space on disk */

/**
*** And for printer devices.
**/
#define printer_invalid      0x11      /* catch-all */
#define printer_offline      0x12
#define printer_outofpaper   0x13
#define printer_error        0x14

/* End of ioaddon.h */

/*------------------------------------------------------------------------
--                                                                      --
-- ioevents.h                                                           --
--                                                                      --
------------------------------------------------------------------------*/

/* These are valid types for the Type field of an Event */
#define Event_Mouse            0x1L
#define Event_Keyboard         0x2L
#define Event_Break            0x4L
#define Event_SerialBreak      0x8L
#define Event_ModemRing        0x10L

#define Flag_Buffer            0x80000000L

/* additional reply codes */
#define EventRc_Acknowledge     0x1L   /* the other side should acknowledge   */
#define EventRc_IgnoreLost      0x2L   /* unimportant messages have been lost */

#if (mouse_supported || gem_supported || MSWINDOWS)
/**
*** Potential problem here, with the way shorts are packed into words in
*** data structures.
**/
typedef struct {
#if (ST || AMIGA || SUN3)    /* || TRIPOS  I imagine */
                 SHORT  Y;
                 SHORT  X;
#else
                 SHORT  X;
                 SHORT  Y;
#endif
                 word   Buttons;
} Mouse_Event;

#define Buttons_Unchanged       0x00000000L
#define Buttons_Button0_Down    0x00000001L
#define Buttons_Button0_Up      0x00008001L
#define Buttons_Button1_Down    0x00000002L
#define Buttons_Button1_Up      0x00008002L
#define Buttons_Button2_Down    0x00000004L
#define Buttons_Button2_Up      0x00008004L
#define Buttons_Button3_Down    0x00000008L
#define Buttons_Button3_Up      0x00008008L
#endif  /* mouse_supported */

#if keyboard_supported
typedef struct { word   Key;
                 word   What;
} Keyboard_Event;

#define Keys_KeyUp      1L
#define Keys_KeyDown    2L
#endif /* keyboard_supported */

typedef struct { word   junk1;
                 word   junk2;
} Break_Event;   /* this is for ctrl-C */

#if RS232_supported
typedef struct { word    junk1;
                 word    junk2;
} SerialBreak_Event;

typedef struct { word    junk1;
                 word    junk2;
} ModemRing_Event;
#endif

typedef struct IOevent { word Type;
                         word Counter;
                         word Stamp;
                         union {
#if (mouse_supported || gem_supported)
                         Mouse_Event        Mouse;
#endif
#if keyboard_supported
                         Keyboard_Event     Keyboard;
#endif
                         Break_Event        Break;
#if RS232_supported
                         SerialBreak_Event  RS232_Break;
                         ModemRing_Event    ModemRing;
#endif
                    } Device;
} IOEvent;


                /* this structure is used to keep track of event handlers */
typedef struct { word port;
                 word *ownedby; /* to keep track of streams */
} event_handler;

/* end of ioevents.h */

/*------------------------------------------------------------------------
--                                                                      --
-- ioconfig.h                                                           --
--                                                                      --
------------------------------------------------------------------------*/
#define CONFIGSPACE	128

typedef struct Config {
        word    PortTabSize;    /* # slots in port table        */
        word    Incarnation;    /* what booter believes our incarnation is */
        word    loadbase;       /* address at which system was loaded */
        word    ImageSize;      /* size of system image         */
        word    Date;           /* current system date          */
        word    FirstProg;      /* offset of initial program    */
        word    Memory;         /* Size of transputer memory, or 0 */
        word    Flags;          /* Various flags */
        word    Spare;          /* a spare slot                 */
        word    MyName;         /* full path name               */
        word    ParentName;     /* ditto                        */
        word    NLinks;         /* number of links              */
        word    LinkConf[1];    /* NLinks LinkConf structs      */
        char    namespace[CONFIGSPACE];	/* space for extra links and */
					/* MyName / ParentName */
} Config;

/* Config.Flags end up as Root.Flags */
#define Config_Flags_rootnode	   0x00000001	/* This is rootnode */
#define Config_Flags_special	   0x00000002	/* This is special nuc. */
#define Config_Flags_ROM	   0x00000004	/* This is ROMm'ed nuc. */
#define Config_Flags_xoffed	   0x00000100	/* Set if links xoffed	*/
#define Config_Flags_CacheOff	   0x00000200	/* Dont enable cache */

#define Link_Flags_parent	0x40	/* indicates the link which booted us */
#define Link_Flags_ioproc	0x20	/* indicates an io processor	*/
#define Link_Flags_debug	0x10	/* debugging link for IOdebug	*/
#define Link_Flags_report	0x08	/* report state changes		*/
#define Link_Flags_stopped	0x04	/* link traffic has been stopped*/
#define Link_Flags_HalfDuplex	0x80	/* use half duplex protocol     */

#define Link_Mode_Null		0	/* not connected to anything	*/
#define Link_Mode_Dumb		1	/* link is a dumb device	*/
#define Link_Mode_Intelligent	2	/* part of Helios network	*/

#define Link_State_Null		0	/* not connected to anything	*/
#define Link_State_Booting	1	/* booting remote processor	*/
#define Link_State_Dumb		2	/* dumb device			*/
#define Link_State_Running	3	/* live network link		*/
#define Link_State_Timedout	4	/* doing idle exchange		*/
#define Link_State_Crashed	5	/* remote processor has crashed	*/
#define Link_State_Dead		6	/* remote processor not running	*/

	/* These support "special" links, e.g. the seventh link on a	*/
	/* Hema DSP1. The values should be kept in step with kernel.h	*/
#define Link_Mode_Special	3
#define Link_State_DSP1		0x10
#define Link_State_Hydra	0x11

/* -- End of ioconfig.h */

/**
*** Link protocol bytes (words on C40)
**/
#define Proto_Write               0
#define Proto_Read                1
#define Proto_Msg                 2
#define Proto_Null                3
#define Proto_Term                4
#define Proto_Reconfigure         5
#define Proto_SecurityCheck       6
#define Proto_Reset               7
#define Proto_Go		  0x0A
#define Proto_ReSync		  0x7f
#define Proto_Info                0x0F0

#define Proto_Debug               0x064
#define Proto_RemoteReset         0x0F2
#define Proto_RemoteAnalyse       0x0F3
#define Proto_Close               0x0F4
#define Proto_Boot                0x0F5

/**
*** Network control function codes
**/
#define NC_Reset                 0x2010L
#define NC_Analyse               0x2020L
#define NC_Connect               0x2030L
#define NC_Disconnect            0x2040L
#define NC_Enquire               0x2050L

#define ND_INVALID               0x0000L
#define ND_HARDWIRED             0x0001L
#define ND_SOFTWIRED             0x0002L
#define ND_NOCONNECT             0x0003L

/**
*** Bootstrap stuff, used by tload.c and hydra
**/
#define Processor_Trannie   1
#define Processor_Arm       2
#define Processor_i860      3
#define Processor_68000     4
#define Processor_C40	    5
/* and lots of others... */

#define B_Reset_Processor    0x0001
#define B_Send_Bootstrap     0x0002
#define B_Send_Image         0x0004
#define B_Send_Config        0x0008
#define B_Wait_Sync          0x0010
#define B_Check_Processor    0x0020
#define B_Send_Sync          0x0040
#define B_Send_IdRom         0x0080

#ifdef internet_supported

#ifndef SOL_SOCKET
/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define	SOL_SOCKET	0xffff		/* options for socket level */
#endif

/*
 * Level number for (get/set)sockopt() to apply to system.
 */
#define SOL_SYSTEM	0xfff0		/* options for system level */

/*
 * Level number for (get/set)sockopt() to apply an ioctl (yuk).
 */
#define SOL_IOCTL	0xfff1		/* options for ioctl level */


#ifndef SO_DEBUG
/*
 * Option flags per-socket.
 */
#define	SO_DEBUG	0x0001		/* turn on debugging info recording */
#define	SO_ACCEPTCONN	0x0002		/* socket has had listen() */
#define	SO_REUSEADDR	0x0004		/* allow local address reuse */
#define	SO_KEEPALIVE	0x0008		/* keep connections alive */
#define	SO_DONTROUTE	0x0010		/* just use interface addresses */
#define	SO_BROADCAST	0x0020		/* permit sending of broadcast msgs */
#define	SO_USELOOPBACK	0x0040		/* bypass hardware when possible */
#define	SO_LINGER	0x0080		/* linger on close if data present */
#define	SO_OOBINLINE	0x0100		/* leave received OOB data in line */

#endif
/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF	0x1001		/* send buffer size */
#define SO_RCVBUF	0x1002		/* receive buffer size */
#define SO_SNDLOWAT	0x1003		/* send low-water mark */
#define SO_RCVLOWAT	0x1004		/* receive low-water mark */
#define SO_SNDTIMEO	0x1005		/* send timeout */
#define SO_RCVTIMEO	0x1006		/* receive timeout */
#define	SO_ERROR	0x1007		/* get error status and clear */
#define	SO_TYPE		0x1008		/* get socket type */

/* Extra options for Helios */
#define	SO_HOSTID	0x8001		/* Host Id (system level)	*/
#define	SO_HOSTNAME	0x8002		/* Host Name (system level)	*/
#define SO_PEERNAME	0x8003		/* Peer name (Get only)		*/
#define SO_SOCKNAME	0x8004		/* socket name (Get only)	*/

#endif
