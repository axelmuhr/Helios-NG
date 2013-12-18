
********************************************************************************
*        ASMHDR                                                                *
********************************************************************************
* This is the standard header file for assembler routines                      *
* written under TRIPOS. It contains useful definitions for                     *
* register names and rootnode offsets, as well as standard                     *
* DCB and packet layouts                                                       *
********************************************************************************

********************************************************************************
* Register definitions
********************************************************************************

Z        EQUR     A0                The constant zero
P        EQUR     A1                BCPL  P ptr
G        EQUR     A2                BCPL  G ptr
L        EQUR     A3                Work reg
B        EQUR     A4                Work reg
S        EQUR     A5                Save routine address
R        EQUR     A6                Return routine address

********************************************************************************
* TCB definition
********************************************************************************

T_LINK    EQU        0               Forward link
T_ID      EQU        4               Task id
T_PRI     EQU        8               Priority
T_WKQ     EQU       12               Work queue
T_WKQTAIL EQU       16               Work queue tail
T_STATE   EQU       20               State (address of long word)
T_STATEW  EQU       22               State (address of word)
T_STATEB  EQU       23               State (address of byte)
T_FLAGS   EQU       24               Flags for break etc
T_STSIZE  EQU       28               Stack size
T_SEGL    EQU       32               Segment list
T_GBASE   EQU       36               Global vector base
T_SBASE   EQU       40               Root stack base
T_SP      EQU       44
T_INTHAND EQU       48
T_SR      EQU       52
T_PC      EQU       54
T_FORM    EQU       58
T_REGDUMP EQU       60
T_UPB     EQU       120              29 words

********************************************************************************
*           Offsets within any DCB                                             *
********************************************************************************

D_Head  EQU     0                   Work Q head pointer
D_Tail  EQU     4                   Work Q tail pointer
D_QAct  EQU     8                   Queuing action routine
D_DQAct EQU    12                   Dequeuing action routine
D_Pri   EQU    16                   Recall priority
D_Id    EQU    20                   Device Id
D_Open  EQU     24
D_Close EQU     28
D_Start EQU     32
D_Abort EQU     36
D_Recall EQU     40
D_Nitb  EQU     44
D_Itb   EQU     48
*
ITB_OFF EQU    6
ITB_NXT EQU    8
ITB_COD EQU    12
ITB_VEC EQU    16
ITB_RCQ EQU    20
ITB_USR EQU    24
ITB_PKT EQU    28

********************************************************************************
*                       Locations in the Root Node                             *
********************************************************************************

TASKTAB         EQU     $00                   Pointer to TRIPOS Task table
DEVTAB          EQU     $04                   Pointer to TRIPOS device table
TCBLIST         EQU     $08                   Start of TCB chain
CRNTSK          EQU     $0C                   Pointer to TCB of current task
BLKLIST         EQU     $10                   Start of free store list
DEBTASK         EQU     $14                   Pointer to DEBUG TCB
DAYS            EQU     $18                   Count of Day
MINS            EQU     $1C                   Count of minute since midnight
TICKS           EQU     $20                   Count of clock ticks
CLKWQ           EQU     $24                   Pointer to first packet for clock
MEMSIZE         EQU     $28                   Memory size in Kwords
INFO            EQU     $2C                   Pointer to info vector
KSTART          EQU     $30                   Kernel start address
DEVUNSET        EQU     $34
GVTSK           EQU     $38                   Calling task in GETVEC
HTCB            EQU     $3C                   Highest TCB
MEMERR          EQU     $40                   Memory error count
ABORTHAND       EQU     $44                   Address of abort handler
SAVERTN         EQU     $48                   Address of SAVE routine
RETRTN          EQU     $4C                   Address of RET routine
IDLET           EQU     $50                   Mc address of IDLE tcb
* Machine dependent routines in DLIB
CLKINIT         EQU     $54                   Initialise clock & anything else
BOOT            EQU     $58                   Address of routine to reboot
PANIC           EQU     $5C                   Rtn to write panic message

*******************************************************************************
*                       Useful constants                                      *
*******************************************************************************

INTLV1          EQU     $2100
INTSON          EQU     $2000
LIBWORD         EQU     23456
SECWORD         EQU     12345

********************************************************************************
*               Offsets within a standard Packet                               *
********************************************************************************

P_LINK          EQU     0
P_ID            EQU     4
P_TYPE          EQU     8
P_RES1          EQU     12
P_RES2          EQU     16
P_ARG           EQU     20
P_ARG1          EQU     P_ARG
P_ARG2          EQU     24
P_ARG3          EQU     28
P_ARG4          EQU     32
P_ARG5          EQU     36
P_ARG6          EQU     40

********************************************************************************
* Kernel request function codes                                                *
********************************************************************************

K_GetMem     EQU      1
K_FreeMem    EQU      2
K_AddTask    EQU      3
K_RemTask    EQU      4
K_Permit     EQU      5
K_Forbid     EQU      6
K_SuperMode  EQU      7
K_UserMode   EQU      8
K_Hold       EQU      9
K_Release    EQU     10
K_ChangePri  EQU     11
K_QPkt       EQU     12
K_TaskWait   EQU     13
K_TestWkQ    EQU     14
K_DQPkt      EQU     15
K_SetFlags   EQU     16
K_TestFlags  EQU     17
K_FindDOS    EQU     18
K_AddDevice  EQU     19
K_RemDevice  EQU     20
K_FindTask   EQU     21
K_RootStruct EQU     22

********************************************************************************
* DOS request vector offsets                                                   *
********************************************************************************

_LVOExit           EQU   0*8
_LVODelay          EQU   1*8
_LVOInput          EQU   2*8
_LVOOutput         EQU   3*8
_LVOUnLoadSeg      EQU   4*8
_LVOWaitForChar    EQU   5*8
_LVOClose          EQU   6*8
_LVOUnLock         EQU   7*8
_LVODupLock        EQU   8*8
_LVOSeek           EQU   9*8
_LVORead           EQU   10*8
_LVOWrite          EQU   11*8
_LVOParentDir      EQU   12*8
_LVOExecute        EQU   13*8
_LVOIsInteractive  EQU   14*8
_LVODateStamp      EQU   15*8
_LVOSetProtection  EQU   16*8
_LVOSetComment     EQU   17*8
_LVODeviceProc     EQU   18*8
_LVOLoadSeg        EQU   19*8
_LVOCreateProc     EQU   20*8
_LVOIoErr          EQU   21*8
_LVOCurrentDir     EQU   22*8
_LVOCreateDir      EQU   23*8
_LVOInfo           EQU   24*8
_LVOExNext         EQU   25*8
_LVOExamine        EQU   26*8
_LVOLock           EQU   27*8
_LVORename         EQU   28*8
_LVODeleteFile     EQU   29*8
_LVOOpen           EQU   30*8
_LVOPktWait        EQU   31*8
_LVOVDU            EQU   32*8

********************************************************************************
* Standard commands to devices                                                 *
********************************************************************************

C_READ  EQU     -1
C_WRITE EQU     -2
C_RESET EQU     -3

********************************************************************************
* Useful values for VDU                                                        *
********************************************************************************

VDU_INIT    EQU   1
VDU_UNINIT  EQU   2
VDU_SETCUR  EQU   8

ACT_VDU     EQU   992
ACT_SC_MODE EQU   994
