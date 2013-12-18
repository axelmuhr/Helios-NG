#ifndef kstructures
#define kstructures 1
#ifndef tripostypes
   #include "tripostypes.h"
#endif

/* This is the structure of a packet */

typedef struct Packettag {
   struct Packettag *p_Link;        /* Link field. */
   LONG p_Dest;                  /* Task ID of destination. */
                                 /* Must be filled in each send. */
   LONG p_Type;                  /* See ACTION_... below and
                                  * 'R' means Read, 'W' means Write to the
                                  * file system */
   LONG p_Res1;                  /* For file system calls this is the result
                                  * that would have been returned by the
                                  * function, e.g. Write ('W') returns actual
                                  * length written */
   LONG p_Res2;                  /* For file system calls this is what would
                                  * have been returned by IoErr() */
/*  Device packets common equivalents */
#define p_Action  p_Type
#define p_Status  p_Res1
#define p_Status2 p_Res2
#define p_BufAddr p_Arg1
   LONG p_Arg1;
   LONG p_Arg2;
   LONG p_Arg3;
   LONG p_Arg4;
   LONG p_Arg5;
   LONG p_Arg6;
   LONG p_Arg7;
} Packet ;

/* A Task Control Block */

typedef struct TCBtag {
   struct TCBtag *tcb_Link;
   LONG tcb_TaskId;
   LONG tcb_Pri;
   Packet *tcb_WkQ;
   Packet *tcb_WkQTail;
   LONG tcb_State;
   LONG tcb_Flags;
   LONG tcb_StackSize;
   BPTR *tcb_SegVec;
   BPTR *tcb_GBase;
   BPTR *tcb_SBase;
   LONG tcb_SP;
   LONG tcb_IntHand;
   USHORT tcb_SR;
   LONG tcb_PC;
   USHORT tcb_Format;   /* used by MC68010 only */
   LONG tcb_RegDump[15];
   LONG tcb_TDisCnt;
   } TCB ;

/* An interrupt transfer block */

typedef struct ITBtag {
   USHORT  itb_Private[3];
   SHORT   itb_Offset;
   struct ITBtag  *itb_Next;
   int     (*itb_Int)();
   LONG    itb_Vector;
   struct ITBtag  *itb_RecallQ;
   LONG    itb_UserData;
   LONG    itb_Packet;
} ITB ;

/* A Device Control Block */

typedef struct {
   Packet *dcb_Head;
   Packet *dcb_Tail;
   void    (*dcb_QAct)();
   void    (*dcb_DQAct)();
   LONG    dcb_Pri;
   LONG    dcb_Id;
   void    (*dcb_Open)();
   void    (*dcb_Close)();
   void    (*dcb_StartIO)();
   void    (*dcb_Abort)();
   void    (*dcb_RecallIO)();
   LONG    dcb_Nitb;
   ITB     dcb_Itb[1];
} DCB ;

typedef struct {
    BPTR    rn_TASKTAB;
    APTR    rn_DEVTAB;
    APTR    rn_TCBLIST;
    APTR    rn_CRNTASK;
    BPTR    rn_BLKLIST;
    APTR    rn_DEBTASK;
    LONG    rn_DAYS;
    LONG    rn_MINS;
    LONG    rn_TICKS;
    LONG    rn_NOTUSED;
    LONG    rn_MEMSIZE;
    BPTR    rn_INFO;
    APTR    rn_KSTART;
} RootNode ;

#define NotInUse ((Packet *) (-1))

/* Packet types */
#define ACTION_NIL              0
#define ACTION_GET_BLOCK        2
#define ACTION_SET_MAP          4
#define ACTION_DIE              5
#define ACTION_EVENT            6
#define ACTION_CURRENT_VOLUME   7
#define ACTION_LOCATE_OBJECT    8
#define ACTION_RENAME_DISK      9
#define ACTION_WRITE          'W'
#define ACTION_READ           'R'
#define ACTION_FREE_LOCK       15
#define ACTION_DELETE_OBJECT   16
#define ACTION_RENAME_OBJECT   17

#define ACTION_COPY_DIR        19
#define ACTION_WAIT_CHAR       20
#define ACTION_SET_PROTECT     21
#define ACTION_CREATE_DIR      22
#define ACTION_EXAMINE_OBJECT  23
#define ACTION_EXAMINE_NEXT    24
#define ACTION_DISK_INFO       25
#define ACTION_INFO            26

#define ACTION_SET_COMMENT     28
#define ACTION_PARENT          29
#define ACTION_TIMER           30
#define ACTION_INHIBIT         31
#define ACTION_DISK_TYPE       32
#define ACTION_DISK_CHANGE     33
#define ACTION_SETOPTIONS     990
#define ACTION_GETOPTIONS     991
#define ACTION_VDU            992
#define ACTION_SETVDU         993
#define ACTION_SC_MODE        994
#define ACTION_FINDUPDATE     MODE_UPDATE
#define ACTION_FINDINPUT      MODE_OLDFILE
#define ACTION_FINDOUTPUT     MODE_NEWFILE
#define ACTION_END            1007
#define ACTION_SEEK           1008

/* Driver commands */

#define DEVACTION_READ   -1
#define DEVACTION_WRITE  -2
#define DEVACTION_RESET  -3
#define DEVACTION_FORMAT -4
#define DEVACTION_STATUS -5
#define DEVACTION_MOTOR  -6

/* Bit numbers that signal you that a user has issued a break */
#define SIGBREAKB_CTRL_C   0
#define SIGBREAKB_CTRL_D   1
#define SIGBREAKB_CTRL_E   2
#define SIGBREAKB_CTRL_F   3

/* Bit fields that signal you that a user has issued a break */
/* for example:  if (TestFlags( SIGBREAKF_CTRL_C )) cleanup_and_exit(); */
#define SIGBREAKF_CTRL_C   (1<<SIGBREAKB_CTRL_C)
#define SIGBREAKF_CTRL_D   (1<<SIGBREAKB_CTRL_D)
#define SIGBREAKF_CTRL_E   (1<<SIGBREAKB_CTRL_E)
#define SIGBREAKF_CTRL_F   (1<<SIGBREAKB_CTRL_F)
#endif
