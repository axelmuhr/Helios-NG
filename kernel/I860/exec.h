#ifndef  _exec_h
#define  _exec_h

#include <memory.h>
#include "i860.h"

typedef word *SystemSP;

typedef struct TrapData *(*Trap_Handler)(struct TrapData *td);
struct TrapData *(Trap_Routine)(struct TrapData *td);

/* Note that members should be added to the front of _Execptrs */
/* So that we work backwards from the top of memory */

typedef struct _ExecPtrs {
	word		 SavePSR;
	word		 SaveREG;
	VoidFnPtr	 TRAPEXIT;
	SystemSP	 SysStack;
	struct EXECROOT *Execroot;
	Trap_Handler     TRAPRTNP;
} _ExecPtrs;

#define EXECPTRS ((_ExecPtrs *)(0-sizeof(_ExecPtrs)))


#define FIRSTARGREG 16
#define MAXARGS 15	/* not sure how much this shold be - but we dont usually get more than 4 args anyway */

#define P_Return(p) (P_IntRegs(p)[1])
#define P_ModTab(p) (P_IntRegs(p)[15])
#define P_StackP(p) (P_IntRegs(p)[2])
#define P_InstPtr(p) (P_IntRegs(p)[3])	/* used only at the start */
#define P_Pri(p) ((p)->Pri)
#define P_IntRegs(p) ((p)->TrapData.intregs)
#define P_State(p) ((p)->State)
#define P_TrapData(p) ((p)->TrapData)
#define P_Display(p) ((p)->Display)

#define State_Run	1
#define State_Create    2
#define State_Except	3

#define STATESPERTABLE 20

typedef struct StateTable {
	struct SaveState States[STATESPERTABLE];
} StateTable;

#define SS_Null        0
#define SS_Interrupted 1
#define SS_WaitIO      2
#define SS_WaitTimer   3
#define SS_Dying       4
#define SS_Free        5

/* 2 K for system stack (hope its enough) */
#define SYSSTACKSIZE 511
/*
   The structure used for the system stack
*/
typedef struct SystemStack {
	struct SystemStack *cdr;
	word   stack[SYSSTACKSIZE];
} SystemStack;


#define SYSTEMSTACKSPERTABLE 20
typedef struct SystemStackTable {
	SystemStack SystemStacks[SYSTEMSTACKSPERTABLE];
} SystemStackTable;

typedef struct  TrapData {
	UWORD   intregs[32];
	UWORD   fpregs[32];
	UWORD	SystemStack;
	UWORD	PSR;
	UWORD   EPSR;			/* not sure */
	UWORD	DB;
	UWORD	DIRBASE;
	UWORD	FIR;
	UWORD	FSR;
	UWORD	KR;
	UWORD	KI;
	UWORD	T;
	UWORD 	MERGE;		/* needs to worry about fp pipline saving */
}TrapData;


typedef struct SaveState {
	struct SaveState *Next;
	word   Pri;
	word   EndTime;
	word   State;
	SystemSP   SysStack;
	word	*Display;
	struct TrapData TrapData; 
} SaveState;

typedef struct ProcessQ {
	SaveState  *head;
	SaveState  *tail;
} ProcessQ;
 
#define MAXPRI 2

typedef struct EXECROOT {
	ProcessQ   Queues[MAXPRI];
	ProcessQ   IQueues[MAXPRI];
	SaveState *CurrentP;
	SaveState *TimerQ;
	UWORD      WakeUp;
	word       CurrentT;
	UWORD      Lo_Pri_Counter;		
	word       CurrentPri;
	int        PriRange;
        int        IntFlag;
/*	SaveState *FreeStates;	*/	/* not used for the moment */
/*	Pool       FreeStatePool;  */
	SystemStack *FreeSystemStacks;
	Pool       FreeSystemStackPool;
	char	  *TxData;
	word       TxSize;
	SaveState *TxSaveState;
	char	  *RxData;
	word       RxSize;
	SaveState *RxSaveState;
} EXECROOT;

/* extern EXECROOT *Execroot(void); */
#define Execroot() (EXECPTRS->Execroot)

typedef struct NextPage { word next; }NextPage;		/* used by mmu as a static var. */
#define NEXTPGPTR ((NextPage *)(next_page_location))
#define GetNextPage() (NEXTPGPTR->next)
#define PutNextPage(p) (NEXTPGPTR->next = p)

#define MemBase 0xf0000000
#define rte_data_area 0xf0009c00
#define exec_area 0xf0009d00
#define next_page_location 0xf0009f00
#define init_stack_pointer 0xf0009000
#define default_current_process_area 0xf0009a00

#define LARGE_VALUE 0x00ffffff
#define LO_PRI_TICKS 10000  /* for debugging - otherwise about 10 */      /* about 8 ms of max run time */

/* The definition of SysRtnPtr is not

   typedef word (*SysRtnPtr)(word a1,...);

   since that would mean the the function entry code
   would have stack all arguments thus generating
   less efficient code. We know that there can be no
   more than 3 args so why not tell the compiler so?
*/
typedef word (*SysRtnPtr)(word a1,word a2, word a3);

extern word System(WordFnPtr func, ...);
extern SaveState *TimerQHead(void);
extern void RunqPtrs(SaveState **p, word pri);
extern SaveState **TimerQAddr(void);
extern ProcessQ *ReadyQBase(word pri);
extern void IqPtrs(SaveState **p, word pri);

extern void Suspend(SaveState **p);
extern void Resume(SaveState *p);
extern void Restart(SaveState *p);
extern word Timer(void);
extern void Yield(void);
extern void Sleep(word time);

extern word *CreateProcess(word *stack, VoidFnPtr entry, VoidFnPtr exit,
			word *descript, word argsize);
extern void EnterProcess(word *stack, word pri);
extern void Stop(void);
extern word _ChangePriority(word priority);
extern word GetPhysPri(void);
extern word GetPhysPriRange(void);
extern void CallWithModTab(word arg1,word arg2,WordFnPtr fn,word *modtab);

extern void ExecInit(void);
extern void MmuInit(void);

#endif
