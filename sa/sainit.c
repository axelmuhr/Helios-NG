
#include "kernel.h"
#include "sysinfo.h"

static void TaskExit(void);

void SAstart(Channel *bootlink, word *loadbase, word bootaddr)
{
	word datasize = 0;
	word maxmod = 0;
	word blocksize;
	word *modtab;
	byte *dptr;
	word *sysbase = (word *)SysBase;
	Module *module;
	Program *prog = (Program *)RTOA(sysbase[2]);
	SysInfo *sysinfo = _SYSINFO;
	
	/* first pass over code is to calculate size of data segment required */
	
	module = &prog->Module;
	
	while( module->Type != 0 )
	{
		Module *mod1 = module;
		
		datasize += module->MaxData * sizeof(word);
		if( module->Id > maxmod ) maxmod = module->Id;
		
		module = (Module *)((word)mod1 + mod1->Size);
	}

	maxmod++;

	/* now datasize & maxmod contain the values we need to	*/
	/* calculate the size of the data segment.		*/
	
	blocksize =	prog->Stacksize + 
			prog->Heapsize +
			datasize +
			maxmod * sizeof(word);

	sysinfo->modtab = modtab = (word *)(sysinfo+1);
	sysinfo->freestore = (byte *)modtab + blocksize;
	sysinfo->bootlink = (bootlink&0xf)/4;
	sysinfo->TraceVec = NULL;
			
	*modtab = 0;
	move_(blocksize-4,modtab+1,modtab);
	
	*modtab = (word)modtab;

	/* second pass over code to set up module table pointers and	*/
	/* calls init routines the first time.				*/
	
	dptr = (byte *)((word)modtab + maxmod * sizeof(word));
	
	module = (Module *)RTOA(sysbase[2]);
	
	while( module->Type != 0 )
	{
		Module *mod1 = module;
		
		modtab[module->Id] = (word)dptr;
		dptr += module->MaxData * sizeof(word);
		
		{
			word *curinit = &module->Init;
			
			while( *curinit != 0 )
			{
				curinit = (word *)RTOA(*curinit);
				CallWithModTab(0,0,(VoidFnPtr)(curinit+1),modtab);
			}
		}
		
		module = (Module *)((word)mod1 + mod1->Size);
	}

	/* pass 3 calls init routines the second time */

	while( module->Type != 0 )
	{
		Module *mod1 = module;
		
		{
			word *curinit = &module->Init;
			
			while( *curinit != 0 )
			{
				curinit = (word *)RTOA(*curinit);
				CallWithModTab(1,0,(VoidFnPtr)(curinit+1),modtab);
			}
		}
		
		module = (Module *)((word)mod1 + mod1->Size);
	}

	/* the data has all been initialised, build entry frame & let it go */
	/* this code is neccesarily machine dependant			*/
	
	{
		word *s = (word *)((word)modtab+blocksize);
		void *f = RTOA(prog->Main);
		word *w = CreateProcess(s,f,TaskExit,modtab,8);
		w[0] = (bootlink&0xf)/4;
		w[1] = 0;

		EnterProcess(w,1);
	}

	Stop();
}

static void TaskExit(void)
{
	Stop();
}


/* -- End of task.c */
