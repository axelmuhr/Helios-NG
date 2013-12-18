#ifndef dosstructures
#define dosstructures 1

#ifndef tripostypes
   #include "tripostypes.h"
#endif

/* Mode parameter to Open() */
#define MODE_UPDATE          1004   /* Open existing file read/write
                                     * positioned at beginning of file.
                                     * with an exclusive lock. */
#define MODE_OLDFILE         1005   /* Open existing file read/write
                                     * positioned at begining of file
                                     * with an exclusive lock */
#define MODE_NEWFILE         1006   /* Open freshly created file (delete
                                     * old file) read/write, exclusive lock */

/* Relative position to Seek() */
#define OFFSET_BEGINNING     -1     /* relative to Begining Of File */
#define OFFSET_CURRENT       0      /* relative to Current file position */
#define OFFSET_END           1      /* relative to End Of File    */

/* Passed as type to Lock() */
#define SHARED_LOCK          -2     /* File is readable by others */
#define ACCESS_READ          -2     /* Synonym */
#define EXCLUSIVE_LOCK       -1     /* No other access allowed    */
#define ACCESS_WRITE         -1     /* Synonym */

typedef struct {
   LONG  ds_Days;             /* Number of days since Jan. 1, 1978 */
   LONG  ds_Minute;           /* Number of minutes past midnight */
   LONG  ds_Tick;             /* Number of ticks past minute */
} DateVec ;

#define TICKS_PER_SECOND      50   /* Number of ticks in one second */

/* Returned by Examine() and ExInfo(), must be on a 4 byte boundary */
typedef struct {
   LONG   fib_DiskKey;
   LONG   fib_DirEntryType;  /* Type of Directory. If < 0, then a plain file.
                              * If > 0 a directory */
   BYTE   fib_FileName[108]; /* Null terminated. Max 30 chars used for now */
   LONG   fib_Protection;    /* bit mask of protection, rwxd are 3-0.      */
   LONG   fib_EntryType;
   LONG   fib_Size;          /* Number of bytes in file */
   LONG   fib_NumBlocks;     /* Number of blocks in file */
   DateVec fib_Date;         /* Date file last changed */
   BYTE   fib_Comment[116];  /* Null terminated.
                              * Comment associated with file */
} FileInfoBlock ;

/* FIB stands for FileInfoBlock */
/* FIBB are bit definitions, FIBF are field definitions */
#define FIBB_READ      3
#define FIBB_WRITE     2
#define FIBB_EXECUTE   1
#define FIBB_DELETE    0
#define FIBF_READ      (1<<FIBB_READ)
#define FIBF_WRITE     (1<<FIBB_WRITE)
#define FIBF_EXECUTE   (1<<FIBB_EXECUTE)
#define FIBF_DELETE    (1<<FIBB_DELETE)

#define t_Short        2
#define t_Long         4
#define t_Data         8
#define t_List        16

#define st_File       -3
#define st_Root        1
#define st_UserDir     2

/* returned by Info(), must be on a 4 byte boundary */
typedef struct {
   LONG   id_NumSoftErrors;   /* number of soft errors on disk */
   LONG   id_UnitNumber;      /* Which unit disk is (was) mounted on */
   LONG   id_DiskState;       /* See defines below */
   LONG   id_NumBlocks;       /* Number of blocks on disk */
   LONG   id_NumBlocksUsed;   /* Number of block in use */
   LONG   id_BytesPerBlock;
   LONG   id_DiskType;        /* Disk Type code */
   BSTR   id_VolumeNode;      /* BCPL pointer to volume name (BCPL string) */
   LONG   id_InUse;           /* Flag, zero if not in use */
} InfoData ;

/* ID stands for InfoData */
        /* Disk states */
#define ID_WRITE_PROTECTED 80    /* Disk is write protected */
#define ID_VALIDATING      81    /* Disk is currently being validated */
#define ID_VALIDATED       82    /* Disk is consistent and writeable */

        /* Disk types */
#define ID_NO_DISK_PRESENT   (-1)
#define ID_UNREADABLE_DISK   (('B'<<24) | ('A'<<16) | ('D'<<8))
#define ID_DOS_DISK          (('D'<<24) | ('O'<<16) | ('S'<<8))
#define ID_NOT_REALLY_DOS    (('N'<<24) | ('D'<<16) | ('O'<<8) | ('S'))
#define ID_KICKSTART_DISK    (('K'<<24) | ('I'<<16) | ('C'<<8) | ('K'))

/* Errors from IoErr(), etc. */
#define ERROR_DEVICE_ID_ERROR             101
#define ERROR_INVALID_PRIORITY            102
#define ERROR_GETVEC_FAILURE              103
#define ERROR_DEVICE_TABLE_FULL           104
#define ERROR_TASK_TABLE_FULL             105
#define ERROR_DEVICE_INITIALISE_FAILURE   106
#define ERROR_DEVICE_NOT_DELETABLE        107
#define ERROR_TASK_NOT_DELETABLE          108
#define ERROR_CANT_FIND_PACKET            109
#define ERROR_TASK_ALREADY_HELD           110
#define ERROR_GLOBAL_INITIALISE_FAILURE   111
#define ERROR_BAD_ARGUMENT_LINE           120
#define ERROR_NOT_AN_OBJECT_MODULE        121
#define ERROR_INVALID_LIBRARY             122

#define ERROR_NO_FREE_STORE               103
#define ERROR_NO_DEFAULT_DIR              201
#define ERROR_OBJECT_IN_USE               202
#define ERROR_OBJECT_EXISTS               203
#define ERROR_DIR_NOT_FOUND               204
#define ERROR_OBJECT_NOT_FOUND            205
#define ERROR_BAD_STREAM_NAME             206
#define ERROR_OBJECT_TOO_LARGE            207
#define ERROR_ACTION_NOT_KNOWN            209
#define ERROR_INVALID_COMPONENT_NAME      210
#define ERROR_INVALID_LOCK                211
#define ERROR_OBJECT_WRONG_TYPE           212
#define ERROR_DISK_NOT_VALIDATED          213
#define ERROR_DISK_WRITE_PROTECTED        214
#define ERROR_RENAME_ACROSS_DEVICES       215
#define ERROR_DIRECTORY_NOT_EMPTY         216
#define ERROR_TOO_MANY_LEVELS             217
#define ERROR_DEVICE_NOT_MOUNTED          218
#define ERROR_SEEK_ERROR                  219
#define ERROR_COMMENT_TOO_BIG             220
#define ERROR_DISK_FULL                   221
#define ERROR_DELETE_PROTECTED            222
#define ERROR_WRITE_PROTECTED             223
#define ERROR_READ_PROTECTED              224
#define ERROR_NOT_A_DOS_DISK              225
#define ERROR_NO_DISK                     226
#define ERROR_NO_MORE_ENTRIES             232

/* Abort */
#define ABORT_STACK_OVERFLOW               84
#define ABORT_LINE1111_EXCEPTION           85
#define ABORT_LINE1010_EXCEPTION           86
#define ABORT_PRIVILEGE_EXCEPTION          87
#define ABORT_TRAPV_EXCEPTION              88
#define ABORT_CHK_EXCEPTION                89
#define ABORT_DIVIDE_BY_ZERO               90
#define ABORT_UNIMPLEMENTED_INSTRUCTION    91
#define ABORT_MEMORY_EXCEPTION             92
#define ABORT_BUS_ERROR                    93
#define ABORT_ARITHMETIC_EXCEPTION         94
#define ABORT_UNEXPECTED_TRAP              95
#define ABORT_CONSOLE_INTERRUPT            96
#define ABORT_UNEXPECTED_INTERRUPT         97
#define ABORT_UNASSIGNED_GLOBAL            98
#define ABORT_UNEXPECTED_IO_INTERRUPT      99

#define ABORT_DEACTIVATE_ERROR            180
#define ABORT_QPKT_FAILURE                181
#define ABORT_UNEXPECTED_PACKET           182
#define ABORT_ACTIVATE_ERROR              196
#define ABORT_ERROR_IN_STORE              197
#define ABORT_PACKET_IN_USE               198
#define ABORT_ILLEGAL_FREEVEC             199
#define ABORT_BITMAP_CORRUPT              287
#define ABORT_BUSY                        288
#define ABORT_KEY_ALREADY_FREE            289
#define ABORT_KEY_ALREADY_ALLOCATED       290
#define ABORT_LOADSEG_FAILURE             292
#define ABORT_INVALID_CHECKSUM            293
#define ABORT_DISC_ERROR                  296
#define ABORT_KEY_OUT_OF_RANGE            297
#define ABORT_ACTION_NOT_KNOWN            298
#define ABORT_IMPOSSIBLE                  299

/* These are the return codes used by convention by system commands */
/* See FAILAT and IF for relvance to EXECUTE files                    */
#define RETURN_OK                           0  /* No problems, success */
#define RETURN_WARN                         5  /* A warning only */
#define RETURN_ERROR                       10  /* Something wrong */
#define RETURN_FAIL                        20  /* Complete or severe failure*/

typedef struct {
   LONG fh_Link;
   LONG fh_Interact;
   LONG fh_ProcessID;
   BPTR fh_Buf;
   LONG fh_Pos;
   LONG fh_End;
   APTR fh_Funcs;
#define fh_Func1 fh_Funcs
   APTR fh_Func2;
   APTR fh_Func3;
   LONG fh_Args;
#define fh_Arg1 fh_Args
   LONG fh_Arg2;
} FileHandle ;

typedef struct DosInfo {
    BPTR    di_McName;         /* Network name of this machine; currently 0 */
    BPTR    di_DevInfo;        /* Device List                               */
    BPTR    di_Segments;       /* Segment List                              */
    BPTR    di_Devices;        /* Currently zero                            */
    APTR    di_NetHand;        /* Network handler processid; currently zero */
} DosInfo ;

typedef struct {
    BPTR      di_Next;
    LONG      di_Type;
    LONG      di_Task;
    BPTR      di_Lock;
    BSTR      di_Handler;
    LONG      di_StackSize;
    LONG      di_Priority;
    BPTR      di_StartUp;
    BPTR      di_SegList;
    BPTR      di_GlobVec;
    BSTR      di_Name;
} DevInfo;

typedef struct {
    BPTR      vi_Next;
    LONG      vi_Type;
    LONG      vi_Task;
    BPTR      vi_Lock;
    DateVec   vi_VolTime;
    BPTR      vi_LockList;
    LONG      vi_DiskType;
    LONG      vi_Spare;
    BSTR      vi_VolName;
} VolInfo ;

typedef struct {
    BPTR      sl_Next;     /* bptr to next device */
    BSTR      sl_Name;     /* bptr to bcpl name   */
    LONG      sl_Use;      /* Use count           */
    BPTR      sl_SegList;  /* Pointer to seg list */
} SegmentList ;

typedef struct {
    BPTR      dl_Next;     /* bptr to next device list   */
    BSTR      dl_Name;     /* bptr to bcpl name          */
    LONG      dl_Use;      /* Use count                  */
    LONG      dl_DeviceID; /* Identifier for this device */
} DeviceList ;

/* a lock structure, as returned by Lock() or DupLock() */
typedef struct {
    BPTR      fl_NextLock;    /* bcpl pointer to next lock */
    LONG      fl_DiskBlock;   /* disk block number */
    LONG      fl_AccessType;  /* exclusive or shared */
    LONG      fl_ProcessID;   /* handler task's port */
    BPTR      fl_VolNode;     /* bptr to a DeviceList */
} FileLock;
#endif
