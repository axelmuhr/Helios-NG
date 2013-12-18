
#include "mcdep.h"
#include <thread.h>
#include <stdlib.h>

extern void thread_create(void *stack, word pri, VoidFnPtr fn, word nargs, ... )
{
	word *args = &nargs + 1;
	word *modtab = (((word ***)&stack)[-1])[0];
	word *w = CreateProcess(stack,fn,thread_stop,modtab,nargs);
	
	move_(nargs,w,args);
	
	EnterProcess(w,pri);
}

extern void thread_stop(void)
{
	Stop();
}	

#if 0
extern bool Fork(word stacksize, VoidFnPtr fn, word nargs, ... )
{
	word *args = &nargs + 1;
	word *modtab = ((word **)&stacksize)[-1];
	extern void *malloc(int);
	void *stack = malloc(stacksize);
	word *w;
	
	if( stack == NULL ) return FALSE;
	
	w = CreateProcess(stack+stacksize,fn,thread_stop,modtab,nargs);
	
	move_(nargs,w,args);
	
	EnterProcess(w,1);
	
	return TRUE;
}
#endif

void _ProcExit(void);		/* only part in assembler */

void __ProcExit(word *stackbase)
{
	freestop(stackbase);
}

void *NewProcess(word stacksize, VoidFnPtr fn, word nargs)
{
	word *stack = (word *)malloc(stacksize);
	word *display = stack + (stacksize/sizeof(word)) - 6; /* display is 6 words */	
	word *p;
	int i;
	
	if( stack == NULL ) return NULL;
	
	display[0] = (((word **)&stacksize)[-1])[0]; 	/* transputer magic */
	display[1] = (word)stack;
	
	if( nargs < 8 ) nargs = 8;	/* leave at least 2 words */
	
	p = CreateProcess(display,fn,_ProcExit,display,nargs);

	return p;
}

void ZapProcess(void *p)
{
	/* The following is transputer magic	*/	
	word **pp = (word **)p;
	free((pp[-1])[1]);
}

void RunProcess(void *p)
{
	EnterProcess(p,0);	/* p already contains pri */
}

word Fork(word stsize, VoidFnPtr fn, word argsize, ... )
{
	byte *args = ((byte *)&argsize) + sizeof(argsize);
	byte *process = (byte *)NewProcess(stsize, fn, argsize);

	if( process == NULL ) return false;

	move_(argsize,process,args);
	
	EnterProcess(process,1);

	return true;
}
