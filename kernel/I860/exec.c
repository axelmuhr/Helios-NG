#include "exec.h"
#include "idebug.h"
#include "clock.h"
#include "lowlevel.h"
#include "adapter.h"
#include "linkio.h" 
#include "mcdep.h"

#ifndef NULL
# define NULL 0
#endif

#define DEBUG1
#define NOEPSR

static void Dispatch(SaveState *s)
{	struct EXECROOT *xroot = Execroot();
	ProcessQ *q,*iq;
	int i;
#ifdef DEBUG2
	XIOdebug("Dispatch: starting \n");
#endif
	if( s != NULL )  
	{
#ifdef DEBUG2
	XIOdebug("Dispatch: saving present context on %X  \n",(word)s);
#endif
	q = &xroot->Queues[P_Pri(s)];

/* The != 0 case here is the return from RestoreCPUState */
		if( SaveCPUState(s) != 0 ) return;
		if( s->State == State_Run )   s->TrapData.PSR = Save_Psr();

	}
	Disable_Ints(); /* to cover entries to Dispatch via non-exceptions */

	while(1)
	{
#ifdef DEBUG2
	XIOdebug("Dispatch: selecting next process \n"); 
#endif

		q = &xroot->Queues[0];
		iq = &xroot->IQueues[0];
	
		for (i=0; i< MAXPRI; i++,q++,iq++)
		{	SaveState *tmp = q->tail;
			if (iq->head) 
			{
				P_RunqNext(tmp) = iq->head;
				q->tail = iq->tail;
				iq->head = NULL; iq->tail = (SaveState *)(&iq->head);
			}
		}

#ifdef DEBUG
	{ SaveState *t = q->head;
	XIOdebug("Dispatch: t %X %X \n",((word *)t)[0],((word *)t)[1]); 
	}
#endif

		for( i=0,q=&xroot->Queues[0]; i< MAXPRI; i++,q++ )
		{	SaveState *thisone;
			xroot->CurrentPri = i;
#ifdef DEBUG2
	XIOdebug("Dispatch: q %X i %d \n",(word)q,i);
	XIOdebug("Dispatch: q->head %X i %d \n",(word)q->head,i);
#endif
			if( (thisone = q->head) != NULL )
			{	xroot->CurrentP = thisone;
				if( (q->head = P_RunqNext(thisone)) == NULL)
					q->tail = (SaveState *)&q->head;
				if( i == 1)
					xroot->Lo_Pri_Counter = LO_PRI_TICKS;	
			    switch( P_State(thisone) )
			    {	
				case State_Create:
				{	/* special case for first time */
#ifdef DEBUG2
	XIOdebug("Dispatch: calling StartCPUState  pri %d\n",i);
/*	XIOdebug("thisone %X %X %X pc %X\n",(word)thisone,((word *)thisone)[0],((word *)thisone)[1],pc()); */
#endif
					P_State(thisone) = State_Run;
					StartCPUState(thisone);
#ifdef DEBUG2
	XIOdebug("Dispatch: after StartCPUState  - this shoudnt print!! \n");
#endif					/* Set_Psr(thisone->TrapData.PSR); */
				}
				case State_Except:
				{
#ifdef DEBUG2
	XIOdebug("Dispatch: callling RestoreCPUState (after exception)\n"); 
#endif					
					P_State(thisone) = State_Run;
					RestoreCPUState(thisone); /* Doesn't return */
				}
				default:
				{
#ifdef DEBUG2
	XIOdebug("Dispatch: callling RestoreCPUState \n"); 
#endif					
					P_State(thisone) = State_Run;
					Set_Psr(thisone->TrapData.PSR);
					RestoreCPUState(thisone); /* Doesn't return */
				}
			   }
			}
		}
#ifdef DEBUG2
		XIOdebug("Dispatcher: entering idler \n");
#endif
		/* Thumb twiddling time!! */
		xroot->CurrentPri = 10;
		xroot->CurrentP = (SaveState *)default_current_process_area;
		Enable_Ints();
		while(!(xroot->IntFlag));
		Disable_Ints();
		xroot->IntFlag = 0;
	}
}

extern word System(WordFnPtr func, ...)
{
	word *args = ((word*)(&func)) + 1; 
	struct EXECROOT *xroot = Execroot();
	SaveState *currentP = xroot->CurrentP;
	word ret;
#ifdef DEBUG2
	XIOdebug("System: enterred with func %X arg1 %X arg2 %X args3 %X \n" ,args[-1],args[0],args[1],args[2]);
#endif		
	if (xroot->CurrentPri == 0) 
		ret = (*func)(args[0],args[1],args[2]);
	else
	{
		word oldpri = GetPhysPri();
		xroot->CurrentPri = P_Pri(currentP) = 0;
		ret = _system(func,args[0],args[1],args[2]);
		xroot->CurrentPri = P_Pri(currentP) = oldpri;
	}
#ifdef DEBUG2
	XIOdebug("System: exiting \n");
#endif		
	return ret;
}


#ifdef STATE_TABLE
/* Should NewState return a SaveState * or an ID?   */
/* bearing in mind that the caller of CreateProcess */
/* could corrupt the State pointer                  */

static struct SaveState *ExtendStateTable(void)
{	struct EXECROOT *xroot = Execroot();
	int i;
	struct StateTable *mem;

	mem = AllocMem(sizeof(struct StateTable),&xroot->FreeStatePool);
	xroot->FreeStates = &(mem->States[1]);
	for( i = 1; i < STATESPERTABLE-1; i++)
		mem->States[i].RunqNext = &mem->States[i+1];
	mem->States[STATESPERTABLE-1].RunqNext = NULL;
	return &(mem->States[0]);
}

static SaveState *NewState(void)
{	struct EXECROOT *xroot = Execroot();
	struct SaveState *s;

	if( xroot->FreeStates == 0 )
		s = ExtendStateTable();
	else
	{
		s = xroot->FreeStates;
		xroot->FreeStates = P_RunqNext(s);
	}
	P_State(s) = SS_Null;
	return s;
}	

static void FreeState(SaveState *s)
{ 	struct EXECROOT *xroot = Execroot();

	P_RunqNext(s) = xroot->FreeStates;
	P_State(s) = SS_Free;
	xroot->FreeStates = s;
}

#endif

static SystemStack *ExtendSystemStackTable(void)
{	struct EXECROOT *xroot = Execroot();
	int i;
	struct SystemStackTable *mem;
#ifdef DEBUG2
XIOdebug("ExtensSystemStackTable: enterd \n");
XIOdebug("exec: arg1 %X ar2 %X \n", sizeof(struct SystemStackTable),
			&xroot->FreeSystemStackPool);
#endif

	mem = AllocMem(sizeof(struct SystemStackTable),
				&xroot->FreeSystemStackPool);
#ifdef DEBUG2
XIOdebug("ExtensSystemStackTable: after allocmem \n");
#endif
	xroot->FreeSystemStacks = &(mem->SystemStacks[1]);
	for( i = 1; i < SYSTEMSTACKSPERTABLE-1; i++)
		mem->SystemStacks[i].cdr = &mem->SystemStacks[i+1];
	mem->SystemStacks[SYSTEMSTACKSPERTABLE-1].cdr = NULL;
#ifdef DEBUG2
XIOdebug("ExtensSystemStackTable: leaving \n");
#endif
	return &(mem->SystemStacks[0]);
}

static SystemSP NewSystemStack(void)
{	struct EXECROOT *xroot = Execroot();
	SystemStack *s;
#ifdef DEBUG2
XIOdebug("NewSystemStack: enterred \n");
#endif
	if( xroot->FreeSystemStacks == 0 )
		s = ExtendSystemStackTable();
	else
	{
		s = xroot->FreeSystemStacks;
		xroot->FreeSystemStacks = s->cdr;
	}
/* Link word at top to the base of the structure */
	s->stack[SYSSTACKSIZE-1] = (word)s;
	return &s->stack[SYSSTACKSIZE-1];
}

static void FreeSystemStack(SystemSP s)
{	struct EXECROOT *xroot = Execroot();
	struct SystemStack *ss = (SystemStack *)(s[0]);

	ss->cdr = xroot->FreeSystemStacks;
	xroot->FreeSystemStacks = ss;
}

static void major_panic(struct TrapData *td)
{
	dumpregs(td);
	while(1);
}

/* TimerQHead */
SaveState *TimerQHead(void)
{	EXECROOT *xroot = Execroot();
	return xroot->TimerQ;
}

SaveState **TimerQAddr(void)
{	EXECROOT *xroot = Execroot();
	return &xroot->TimerQ;
}

void RunqPtrs(SaveState **p, word pri)
{	struct EXECROOT *xroot = Execroot();
	p[0] = xroot->Queues[pri].head;
	p[1] = xroot->Queues[pri].tail;
}

ProcessQ *ReadyQBase(word pri)
{	struct EXECROOT *xroot = Execroot();
	return (&xroot->Queues[pri]);
}

void Suspend(SaveState **pp)
{	EXECROOT *xroot = Execroot();
	SaveState *p = xroot->CurrentP;

	*pp = p;
	Dispatch(p);
}

void Resume(SaveState *p)
{
	EXECROOT *xroot = Execroot();
	ProcessQ *pq = &xroot->Queues[P_Pri(p)];

	P_RunqNext(pq->tail) = p;
	P_RunqNext(p) = NULL;
	pq->tail = p;
}

void Restart(SaveState *p)	/* add to intermediate queue  */
{ 				/* only to be used by interrupt routines */
	EXECROOT *xroot = Execroot();
	ProcessQ *pq = &xroot->IQueues[P_Pri(p)];

	P_RunqNext(pq->tail) = p;
	P_RunqNext(p) = NULL;
	pq->tail = p;
}

void Yield(void)
{
	EXECROOT *xroot = Execroot();
	SaveState *s = xroot->CurrentP;
	word pri = P_Pri(s);
	ProcessQ *pq = &xroot->Queues[pri];

	if( pq->head != NULL )
	{	P_RunqNext(s) = NULL;
		P_RunqNext(pq->tail) = s;
		pq->tail = s;
		Dispatch(s);		
	}
}

word Timer(void)
{	EXECROOT *xroot = Execroot();
	return xroot->CurrentT;
}

word _ldtimer(void)
{
	return (Timer());
}
	
word _cputime(void)
{
	EXECROOT *xroot = Execroot();
	float time = (float)(((UWORD)xroot->CurrentT)/10000*MICROS_PER_CLK);
	
	return ( (UWORD) time);
}

void Sleep(word time)
{	EXECROOT *xroot = Execroot();
	SaveState *s = xroot->CurrentP;
	UWORD endtime = xroot->CurrentT+time;
#ifdef DEBUG2
	XIOdebug("Sleep: time %d wakeup %X \n",time,endtime);
#endif
    	Set_P_EndTime(s,endtime);
	if( xroot->TimerQ == NULL || After(xroot->WakeUp,P_EndTime(s)) )
	{	/* Process goes at head */
		P_TimerNext(s) = xroot->TimerQ;
		xroot->WakeUp = endtime;
		xroot->TimerQ = s;
	}
	else
	{	
		SaveState **nextp = &xroot->TimerQ;
		SaveState *next = *nextp;

		while( next != NULL && After(endtime,P_EndTime(next)) )
		{
			nextp = &P_TimerNext(next);
			next  = *nextp;
		}
		P_TimerNext(s) = next;
		*nextp = s;
	}
/*	Disable_Ints();
	while(1); */
	Dispatch(s);
}

void Delay(word time)
{
#ifdef DEBUG2
	XIOdebug("Delay: time %X Sleep %X \n",time,(word)Sleep);
#endif
	System((WordFnPtr)Sleep,time);
}

word *CreateProcess(word *stack, VoidFnPtr entry, VoidFnPtr exit,
			word *descript, word argsize)
{
	SaveState *newstate;
#ifdef DEBUG2
	XIOdebug("CreateProcess: stack %X entry %X exit %X \n",stack,entry,exit);  
#endif
	/* SaveState permantly allocated below display */
	stack -= sizeof(SaveState)/sizeof(word);
	newstate = (SaveState *)stack;
	stack -= argsize/sizeof(word);
	stack[-1] = argsize/sizeof(word);
	stack[-2] = (word)newstate;
	
	if (stack[-1] > MAXARGS) XIOdebug("CreateProcess: too many args \n");
/* if we need to use Parp (used by Kevin's PSS), change CreateProcess and */
/* EnterProcess as in M68020 exec68.c                                     */

	P_InstPtr(newstate) = (word)entry;
	P_Return(newstate) = (word)exit;
	P_ModTab(newstate) = descript[0];
	P_RunqNext(newstate) = NULL;
	P_State(newstate) = State_Create;
	P_Display(newstate) = descript;

	return (word *)stack;
}

/* This should set up the process stack such that a StartCPUState
   on the SaveState in stack[-2] will enter the process at the entry
   point given in createprocess is entered.
*/
void EnterProcess( word *stack, word pri )
{	EXECROOT *xroot = Execroot();
/*	ProcessQ *pq = &(xroot->Queues[pri]); */
	SaveState *s = (SaveState *)stack[-2]; /* Check validity */
	word argno = stack[-1];
	word i;
	word *args = (word *)(&(s->TrapData.intregs[0]));
	TrapData *td = &s->TrapData;
	
/*	P_RunqNext(pq->tail) = s;
	P_RunqNext(s) = NULL; */
	
	for( i = 0 ; i < argno; i++ )
		args[FIRSTARGREG+i] = *stack++;
	P_StackP(s) = (word)stack;

	P_Pri(s) = pri;
	if (pri == 0)
		td->PSR = psr_init_hi; 
	else
		td->PSR = psr_init_lo;
	td->FSR = 0;	/* rewiew this when implementing fp */
	
	Resume(s);
}

void CallWithModTab(word arg1,word arg2,WordFnPtr fn,word *modtab)
{
	word *oldmodtab = _GetModTab();
	
	SetModTab(modtab);
	fn(arg1,arg2);
	SetModTab(oldmodtab);
}

void Stop(void)
{	EXECROOT *xroot = Execroot();

/*	FreeState(xroot->CurrentP); */
	xroot->CurrentP =  (SaveState *)default_current_process_area; 
#ifdef DEBUG2
	XIOdebug("Stop: calling Dispatch \n");
#endif
	Dispatch(NULL);
}

word _ChangePriority(word pri)
{	EXECROOT *xroot = Execroot();
	word old = xroot->CurrentPri;
	xroot->CurrentPri = pri;
	P_Pri(xroot->CurrentP) = pri;
	return old;
}

word GetPhysPri(void)
{	EXECROOT *xroot = Execroot();
	return xroot->CurrentPri;
}

word GetPhysPriRange(void)
{	EXECROOT *xroot = Execroot();
	return xroot->PriRange;
}

/* Trap Handlers start here */

#define REAL_EXEC_CALL 0x1234
static void handle_instruction_trap(struct TrapData *td)
{	union { SysRtnPtr a; word b; } bodge;

	EXECROOT *xroot = Execroot();
	UWORD ticks_left = xroot->Lo_Pri_Counter; 
	UWORD instr = *((UWORD *)td->FIR);
	if( f_src1i(instr) == REAL_EXEC_CALL )
	{	SysRtnPtr fn;
		word a1 = td->intregs[16];
		word a2 = td->intregs[17];
		word a3 = td->intregs[18];

		bodge.b = td->intregs[f_src2(instr)];
		fn = bodge.a;
		td->intregs[16] = (*fn)(a1,a2,a3);
		td->FIR += 4;
		xroot->Lo_Pri_Counter = ticks_left;
	}
	else
	{	XIOdebug("Bad instruction trap at %X\n",td->FIR);
		major_panic(td);
	}
}

static word clock_interrupt()
{	EXECROOT *xroot = Execroot();
	word currentT;
	word call_dispatch = 0;
	
	currentT = xroot->CurrentT += MICROS_PER_CLK;
#ifdef DEBUG2
	XIOdebug("clock_interrupt %X ", xroot->CurrentT);
#endif	
	
	if(xroot->CurrentPri == 1) 
	{
		if (--xroot->Lo_Pri_Counter == 0) 
		{
			call_dispatch = 1;
			Restart(xroot->CurrentP);
		}
	}		
	if( After(currentT,xroot->WakeUp))
	{
		SaveState *next = xroot->TimerQ;
		word currentPri = xroot->CurrentPri;
#ifdef DEBUG2
	XIOdebug("clock_interrupt: xroot->WakeUp %X currentT %X\n",
			xroot->WakeUp,currentT);
#endif	
		while( next != NULL && After(currentT,P_EndTime(next)) )
		{
			ProcessQ *pq = &xroot->IQueues[P_Pri(next)];
			xroot->TimerQ = P_TimerNext(next);
			if( currentPri > P_Pri(next) )
				call_dispatch = 1;
				
			P_RunqNext(pq->tail) = next;
			P_RunqNext(next) = NULL;
			pq->tail = next;
			
			next  = xroot->TimerQ;
		}
		if( xroot->TimerQ != NULL)
			xroot->WakeUp = P_EndTime(xroot->TimerQ);		
		else 
			xroot->WakeUp = currentT + LARGE_VALUE;
	}
	return call_dispatch;
}


static word handle_interrupt(struct TrapData *td)
{	int handled = 0;
	word call_dispatch = 0;
#ifdef DEBUG2
	XIOdebug("handle_interrupt timer reg %X \n",CLOCK->clock);
#endif
		
	if( (CLOCK->clock & clk_active)==0 )
	{
#ifdef DEBUG2
	XIOdebug("calling clock \n");
#endif
		call_dispatch = clock_interrupt();
		handled = 1;
	}
	if( (ADAPTER->LA_IStat & (i_rdy|i_inte)) == (i_rdy|i_inte) )
	{
		call_dispatch = RxInterruptHandler(ADAPTER);
		handled = 1;
	}
	if( (ADAPTER->LA_OStat & (o_rdy|o_inte)) == (o_rdy|o_inte) )
	{
		call_dispatch = TxInterruptHandler(ADAPTER);
		handled = 1;
	}
	if( !handled )
		XIOdebug("Unknown interrupt at %X\n",td->FIR);

	return call_dispatch;
}

static void handle_instruction_access_trap(struct TrapData *td)
{
	XIOdebug("Instruction access trap at %X\n",td->FIR);
}


static void handle_data_access_trap(struct TrapData *td)
{	UWORD instr = *((UWORD *)td->FIR);
	UWORD topbits = (instr>>26) &0x3f;
	UWORD address;

	XIOdebug("Data access trap at %X\n",td->FIR);
	XIOdebug("Instruction = %X\n",instr);
	dumpregs(td);
	if( (topbits & 0x3a) == 0 )
	{	/* ld.x instruction */
		address = instr & bit(26) ? f_src1i(instr) & ~1l:
			td->intregs[f_src1r(instr)];
	}
	elif( (topbits & 0x3b) == 0x03 )
	{	/* st.x instruction  */
		address = instr & bit(26) ? f_src1scr(instr) & ~1l:
			td->intregs[f_src1r(instr)];
	}
	else
	{
		major_panic(td);
	}
	address += td->intregs[f_src2(instr)];
	XIOdebug("Fault address = %X\n",address);
}

static void handle_float_trap(struct TrapData *td)
{
	XIOdebug("Floating point trap at %X\n",td->FIR);
}


#ifndef NOEPSR
static handle_interlock_trap(TrapData *td)
#endif

/*
 * For this routine see section 7.2.2 of the PRM
 */
 
typedef struct insmask { UWORD mask; UWORD val; }insmask;
	
#if 0		/* trasferred to Init_Rte_Data to avoid static data */
static struct insmask autoincins[] =
{	{ 0xf0000001, 0x20000001},	/* fld.x, fst.x	*/
	{ 0xfc000001, 0x3c000001},	/* pst.d	*/
	{ 0xf8000001, 0x60000001},	/* pfld.y	*/
	{ 0xfc000001, 0x34000001}	/* flush	*/
};
#define sizeofautoincarray (sizeof(autoincins)/sizeof(struct insmask))
#endif

/* remember to change the following if any changes to Init_Rte_Data */
/* should be made. This probably can be done better, but I am going */
/* for the quick and nasty way for the moment */

#define sizeofautoincarray 4

#if 0
static struct insmask delayedins[] =
{	{ 0xfc000000, 0x40000000},	/* bri		*/
	{ 0xfc00001f, 0x4c000002},	/* calli	*/
	{ 0xfc000000, 0x68000000},	/* br		*/
	{ 0xfc000000, 0x6c000000},	/* call		*/
	{ 0xfc000000, 0x74000000},	/* bc.t		*/
	{ 0xfc000000, 0x7c000000},	/* bnc.t	*/
	{ 0xfc000000, 0xb4000000}	/* bla		*/
};
#endif

#define dlyd_bct 4
#define dlyd_bnct 5
#define dlyd_bla 6

#define sizeofdelayedarray 7

#if 0
#define sizeofdelayedarray (sizeof(delayedins)/sizeof(struct insmask))

static struct insmask pfcmpins[] =
{	{ 0xfc00007f, 0x48000034},	/* pfgt, pfle	*/
	{ 0xfc00007f, 0x48000035}	/* pst.d	*/
};
#define sizeofpfcmparray (sizeof(pfcmpins)/sizeof(struct insmask))
#endif

#define sizeofpfcmparray 2

#if 0
#ifdef BUG17
static struct insmask anybranchins[] =
{	{ 0xfc000000, 0x40000000},	/* bri		*/
	{ 0xfc00001f, 0x4c000002},	/* calli	*/
	{ 0xfc000000, 0x68000000},	/* br		*/
	{ 0xfc000000, 0x6c000000},	/* call		*/
	{ 0xfc000000, 0x70000000},	/* bc.t		*/
	{ 0xfc000000, 0x78000000},	/* bnc.t	*/
	{ 0xfc000000, 0xb4000000}	/* bla		*/
};
#define sizeofanybrancharray (sizeof(anybranchins)/sizeof(struct insmask))
#endif
#endif

/* Private to rte */
static int findins(insmask *v, int n, UWORD w)
{	int i;
	for( i = 0; i < n; i++,v++)
	{
		if( (w & v->mask) == v->val )
			return i;
	}
	return -1;
}

static struct TrapData *rte(struct TrapData *td)
{	UWORD fir = td->FIR;
	insmask *autoincins = (insmask *)rte_data_area;
	insmask *delayedins = autoincins + sizeofautoincarray;
	insmask *pfcmpins   = delayedins + sizeofdelayedarray;
	
/* Part 2. */
	if( (td->PSR & (psr_dat|psr_ft)) == psr_dat )
	{	int i;
		UWORD instr = *( (UWORD *)fir );
#ifdef DEBUG
		XIOdebug("Inspecting instruction %X for autoinc\n",instr);
#endif
		i = findins( autoincins, sizeofautoincarray, instr);
		if( i != -1 )
		{
			UWORD src1 = instr & bit(26) ?
					f_src1i(instr):
					td->intregs[f_src1r(instr)];
			UWORD *src2p = &td->intregs[f_src2(instr)];
			XIOdebug("found autoinc instruction %d, instr = %X at %X\n",i, instr, fir);
			*src2p -= src1;
		}
	}
/* Part 3. */
/* Section 7.2.2.1 */
	{	UWORD instr = *( (UWORD *)(fir-4) );
		int i;
#ifdef DEBUG
	XIOdebug("Inspecting instruction %X for delayed ctrl\n",instr);
#endif
		i = findins( delayedins, sizeofdelayedarray, instr);
		if( i != -1 )
		{
#ifdef DEBUG
			XIOdebug("found delayed instruction %d, instr = %X at %X\n",i,instr,fir-4);
#endif
			if( i == dlyd_bla )
			{	UWORD *src2p = &td->intregs[f_src2(instr)];
			XIOdebug("its's a bla\n");
				*src2p -= td->intregs[f_src1r(instr)];
			}
			else if (i >= dlyd_bct )
			{	UWORD instr1 = *( (UWORD *)(fir-4) );
				int j;
				XIOdebug("its's a bc.t or bnc.t\n");
				j = findins( pfcmpins, sizeofpfcmparray,
							instr1);
				if( j != -1 )
					major_panic(td);
			}
			fir -= 4;
/* floating point traps and dual instruction problems are not handled here */
#ifdef BUG17
			if( *(UWORD *)(fir-8) == NOP )
				fir -= 12;
			else
				fir -= 8;
#endif
#ifdef DEBUG
			XIOdebug("fir changed to %X\n",fir);
#endif
			td->FIR = fir;
		}
#ifdef BUG17
		else
		{	UWORD instr = *( (UWORD *)fir );
			i = findins(anybranchins, sizeofanybrancharray, instr);
			if( i != -1 )
			{
#ifdef DEBUG
				XIOdebug("Found some kind of branch %X at %X\n",instr,fir);
#endif
				if( *(UWORD *)(fir-8) == NOP )
					fir -= 12;
				else
					fir -= 8;
#ifdef DEBUG
				XIOdebug("fir corrected to %X\n",fir);
#endif
				td->FIR = fir;
			}
		}
#endif
	}
	return td;
}

#define temp_stack  0xf000e000

void ToProcessStack(void)
{	struct EXECROOT *xroot = Execroot();
	struct SaveState *p = xroot->CurrentP;
	struct _ExecPtrs *xp = EXECPTRS;
	SystemSP newstack;
#if 1
	newstack = NewSystemStack();
	p->SysStack = xp->SysStack;
#else
	newstack = (SystemSP)temp_stack;
	p->SysStack = (SystemSP)0xf000f000;
#endif
	xp->SysStack = newstack;
}

void ToCommonStack(void)
{	struct EXECROOT *xroot = Execroot();
	struct SaveState *p = xroot->CurrentP;
	struct _ExecPtrs *xp = EXECPTRS;
#if 1
	FreeSystemStack(xp->SysStack);
#endif
	xp->SysStack = p->SysStack;
	p->SysStack = NULL;
}

/*
   We enter this routine on a common system stack.
   If (at any time during this exception) we need to 
   enable interrupts then they should only be enabled
   after a ToProcessStack() call.
*/
extern struct TrapData *Trap_Routine(struct TrapData *td)
{	struct EXECROOT *xroot = Execroot();
	UWORD psr = td->PSR;
	SaveState *cp = xroot->CurrentP;
	word call_dispatch = 0;
	ToProcessStack(); 	 /* Make this stack the system stack for this
				   process */
	cp->State = State_Except;
#ifdef DEBUG2
XIOdebug("****Trap_Routine no system stack!!****\n  psr = %X",psr);
#endif
/* Handle interrupts first! */

	if( psr & psr_in )	/* Interrupt */
	{	
#ifdef DEBUG2
		XIOdebug("***calling handle_interrupt \n");
#endif
		call_dispatch = handle_interrupt(td);
#ifdef DEBUG2
		XIOdebug("***returning from int routine\n");
#endif
	}

	if( psr & psr_it )	/* Instruction trap */
	{
#ifdef DEBUG2
		XIOdebug("***instruction trap \n");
#endif
		handle_instruction_trap(td);
	}
	
	if( psr & psr_iat )	/* Instruction access trap */
	{
		XIOdebug("***instruction access trap \n");
		handle_instruction_access_trap(td);
	}
	
	if( psr & psr_dat )	/* Data access trap */
	{
		XIOdebug("***data access trap \n");
		handle_data_access_trap(td);
	}
	
	if( psr & psr_ft )	/* Float trap */
	{
		XIOdebug("***float trap \n");
		handle_float_trap(td);
	}
#ifndef NOEPSR
	if( TrapData.EPSR & epsr_il )	/* Interlock trap */
	{
		handle_interlock_trap(td);
 	}
#endif
/*
   Interrupt routines do not call the dispatcher if interrupted
     supervisor mode!
*/
	if( call_dispatch)	/* check if we really need process swapping */
	{
#ifdef DEBUG2
	XIOdebug("clk_int: call_dispatch psr %X \n",psr);
#endif
		if( (psr & psr_pu) != 0 )
			Dispatch(cp);
		else
			xroot->IntFlag = 1;    
	}
	ToCommonStack();  	/* And release it back to common stack */
#ifdef DEBUG
	XIOdebug("**** current time %X\n",xroot->CurrentT);
#endif
	cp->State = State_Run;
	return rte(td);
}

static void Init_Rte_Data(void);

void ExecInit(void)
{	EXECROOT *xroot;
	struct _ExecPtrs *xp = EXECPTRS;
	SystemSP newstack;
	SaveState *sa;

#ifdef DEBUG2
	TrapData td;
	XIOdebug("ExecInit: starting\n");
	cpu_state(&td);
	dumpregs(&td);
#endif

	xp->Execroot = (EXECROOT *)exec_area; /* init execroot pointer */

 	xroot = Execroot();
#ifdef DEBUG2
	XIOdebug("ExecInit pc = %X \n",pc());
	
	XIOdebug("ExecInit: starting\n");
	XIOdebug("&xroot->Queues[0] = %X \n",(word)&xroot->Queues[0]);
	XIOdebug("&xroot->CurrentT = %X \n",(word)&xroot->CurrentT);
	XIOdebug("&xroot->IntFlag = %X \n",(word)&xroot->IntFlag);
	XIOdebug("&xroot->FreeStates = %X \n",(word)&xroot->FreeStates);
	XIOdebug("&xroot->FreeStatePool = %X \n",(word)&xroot->FreeStatePool);
	XIOdebug("&xroot->FreeSystemStacks = %X \n",(word)&xroot->FreeSystemStacks);
	XIOdebug("&xroot->FreeSystemStackPool = %X \n",(word)&xroot->FreeSystemStackPool);
	XIOdebug("&xroot->TxData = %X \n",(word)&xroot->TxData);
	XIOdebug("&xroot->TxSaveState = %X \n",(word)&xroot->TxSaveState);
	XIOdebug("&xroot->RxSaveState = %X \n",(word)&xroot->RxSaveState);
#endif
	{	int i;
		ProcessQ *pq = &xroot->Queues[0];
		for( i=0; i<MAXPRI*2; i++,pq++)
		{
			pq->head = NULL;
			pq->tail = (SaveState *)(&pq->head);
 		}
	}
	xroot->TimerQ = NULL;
	sa = xroot->CurrentP = (SaveState *)default_current_process_area;  /* (SaveState *)0xf000b000; */
	xroot->CurrentT = 0;
	xroot->WakeUp = LARGE_VALUE;
	xroot->IntFlag = NULL;
	xroot->CurrentPri = 0;
	xroot->PriRange = 1;
	xroot->FreeSystemStacks = 0;
	
	P_RunqNext(sa) = NULL;
	P_Pri(sa) = 0;
	
#ifdef DEBUG2
	XIOdebug("Initialising rte data \n");
#endif
	Init_Rte_Data();		
#ifdef DEBUG2
	XIOdebug("Installing trap handler \n");
	{ word *x = (word *)&xroot->FreeSystemStacks;
	XIOdebug("xroot from FreeSys.. \n %X %X %X %X %X %X %X %X %X %X\n",
	*x++,*x++,*x++,*x++,*x++,*x++,*x++,*x++,*x++,*x++);
	} 
#endif
	
/*	InitPool(&xroot->FreeStatePool); */
	InitPool(&xroot->FreeSystemStackPool);
#ifdef DEBUG2
	XIOdebug("pool initialised \n");
	{ word *x = (word *)&xroot->FreeSystemStackPool;
	XIOdebug("xroot from FreeSys.. \n %X %X %X %X %X %X %X %X %X %X\n",
	*x++,*x++,*x++,*x++,*x++,*x++,*x++,*x++,*x++,*x++);
	} 
#endif
	newstack = NewSystemStack(); 
#ifdef DEBUG2
	XIOdebug("before Install_Trap_Handler\n");
	cpu_state(&td);
	dumpregs(&td); 
	{
		word *a = (word *)0xfffff000; int i;
		for (i=0; i<500; i++) a[i] = 0;
	}
#endif
	xp->SysStack = newstack;
	Install_Trap_Handler(Trap_Routine); 
#ifdef DEBUG2
	XIOdebug("after Install_Trap_Handler\n");
	cpu_state(&td);
	dumpregs(&td); 
#endif
	
	xroot->RxSaveState = (SaveState *)MinInt;
	xroot->TxSaveState = (SaveState *)MinInt;

/*	Link_Int_Disable();	*/
#ifdef DEBUG2
	XIOdebug("Enabling ints \n");
#endif
	Enable_Ints();  
	
#if 0
	while(1) 
	{ word t;
	t = Timer(); /* Sleep(1000000); */ 
	XIOdebug(" time %D  timer %X\n",t/1000000,t); 
	} 	
#endif

#ifdef DEBUG2
	XIOdebug("return address %X \n",pc());
	XIOdebug("ExecInit: leaving \n");
	cpu_state(&td);
	dumpregs(&td); 
#endif
}

#if 0
void *AllocMem(word size, Pool *pool)
{ 
	size = size; 
}

void InitPool(Pool *pool)
{
	pool = pool;
}

#endif

static void Init_Rte_Data(void)
{
	word *r = (word *)rte_data_area;
	
 
	*r++ = 0xf0000001;	/* autoincins */
	*r++ = 0x20000001;
	*r++ = 0xfc000001;
	*r++ = 0x3c000001;
	*r++ = 0xf8000001;
	*r++ = 0x60000001;
	*r++ = 0xfc000001;
	*r++ = 0x34000001;

	*r++ = 0xfc000000;	/* delayedins */
	*r++ = 0x40000000;
	*r++ = 0xfc00001f;
	*r++ = 0x4c000002;
	*r++ = 0xfc000000;
	*r++ = 0x68000000;
	*r++ = 0xfc000000;
	*r++ = 0x6c000000;
	*r++ = 0xfc000000;
	*r++ = 0x74000000;
	*r++ = 0xfc000000;
	*r++ = 0x7c000000;
	*r++ = 0xfc000000;
	*r++ = 0xb4000000;

	*r++ = 0xfc00007f;	/* pfcmpins */
	*r++ = 0x48000034;
	*r++ = 0xfc00007f;
	*r++ = 0x48000035;

}


word *GetDisplay(void)
{
	EXECROOT *xroot = Execroot();
	SaveState *cp = xroot->CurrentP;
	
	return (cp->Display);
}
