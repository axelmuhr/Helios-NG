
#include <helios.h>
#include <module.h>
#include <memory.h>
#include <stdlib.h>
#include <salib.h>
#include <codes.h>
#include <string.h>

word AccelerateCode(VoidFnPtr p)
{
	Module *m;
	Module *module;
	word *x = (word *)p;
	void *fram;
	int i;
	
	/* first find start of module */
	until( *x == T_Module ) x--;
	m = (Module *)x;

	/* allocate space for module */
	fram = malloc_fast(m->Size);

	if( fram == NULL ) return EC_Error|SS_Kernel|EG_NoMemory;
	
	/* copy whole module into fast RAM */
	memcpy(fram,m,m->Size);
	
	module = (Module *)fram;
	
	/* now call init routines in module to install new procedure	*/
	/* pointers in module table.					*/
	
	for( i = 0; i <= 1 ; i++ )
	{
		word *curinit = &module->Init;
		
		while( *curinit != 0 )
		{
			VoidFnPtr fn;
			curinit = (word *)RTOA(*curinit);
			fn = (VoidFnPtr)(curinit+1);
			(*fn)(i,0);
		}
	}
	
	return Err_Null;
}

