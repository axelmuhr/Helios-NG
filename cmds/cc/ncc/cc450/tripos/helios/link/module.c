/************************************************************************/
/*                                                                      */
/* File: module.c                                                       */
/*                                                                      */
/* Changes:                                                             */
/*      NHG  07-July-87  : Created                                      */
/*                                                                      */
/* Description:                                                         */
/*      module manipualtion routines                                    */
/*                                                                      */
/*                                                                      */
/* Copyright (c) 1987, Perihelion Software Ltd. All Rights Reserved.    */
/************************************************************************/
static char *SccsId = "%W%	%I% Copyright (C) Perihelion Software Ltd.";

#include "link.h"
#include <stdlib.h>

#define trace if(traceflags&db_modules)_trace

#define modtabinc 64

PUBLIC VMRef module0;
PUBLIC VMRef curmod;
PUBLIC VMRef firstmodule;
PUBLIC VMRef tailmodule;

PRIVATE VMRef *modtab;
PRIVATE word modtabend = 0;

static void extendmodtab(ellipsis);

PUBLIC void initmodules()
{
	Module *m0;
	
	firstmodule = NullVMRef;
	
	tailmodule = curmod = module0 = VMNew(sizeof(Module));
	m0 = VMAddr(Module,module0);
	m0->next = NullVMRef;
	m0->id = 0;
	m0->linked = TRUE;
	m0->start = codeptr();
	
	VMlock(curmod);	/* current module page is always locked */
}

/****************************************************************/
/* Procedure: setmodules                                        */
/* Description:                                                 */
/*      assign table slots to the modules                       */
/*                                                              */
/****************************************************************/

PUBLIC void setmodules()
{
	VMRef v;
        Module *m;
        int lastm = 1;
        int i;
        int maxmodule = 0;
	
        trace("Setmodules");

	VMunlock(curmod);

        /* first assign all known modules */

        v = module0;
        while( !NullRef(v) )
        {
        	m = VMAddr(Module,v);
                if( m->id != -1 )
                {
                        while( m->id >= modtabend ) extendmodtab();
                        if( !NullRef(modtab[m->id]) )
                                error("Multiple definition of module %d",m->id);
                        modtab[m->id] = v;
                        trace("Pre-defined module %x[%x] at %d",v,m,m->id);
			if( m->id > maxmodule ) maxmodule = m->id;
                }
                v = m->next;
        }

        /* now assign the unknowns */
        v = module0;
        while( !NullRef(v) )
        {
        	m = VMAddr(Module,v);
                if( m->id == -1 ) 
                {
                        while( !NullRef(modtab[lastm]) ) 
                        {
                        	lastm++;
	                        if( lastm >= modtabend ) extendmodtab();
	                }
                        modtab[lastm] = v;
                        m->id = lastm;
                        VMDirty(v);
                        trace("Assign module %x[%x] to %d",v,m,lastm);
                        if( m->id > maxmodule ) maxmodule = m->id;
                }
                v = m->next;
        }
        
	/* now we re-build the module list with firstmodule first, but all */
	/* the others in numerical order.				  */

	curmod = module0;
	
	if( !NullRef(firstmodule) )
	{
		VMDirty(curmod);
		m = VMAddr(Module,curmod);
		curmod = m->next = firstmodule;
					
		for( i = 1; i <= maxmodule ; i++ )
			if( !NullRef(modtab[i]) && 
				!VMSame(modtab[i],firstmodule) ) 
			{
				VMDirty(curmod);
				m = VMAddr(Module,curmod);
				trace("Added module %x[%x] %d to list",curmod,m,m->id);
				curmod = m->next = modtab[i];
			}
	}
	
	/* null at the end of the list */
	m = VMAddr(Module,curmod);
	trace("Last module = %x %d",curmod,m->id);
	m->next = NullVMRef;
	VMDirty(curmod);
}

/****************************************************************/
/* refsymbol                                        		*/
/*								*/
/* The given symbol has been referenced, if it is a global	*/
/* symbol, ensure that its defining module is in the module	*/
/* list. This only makes a difference for library modules.	*/
/*                                                              */
/****************************************************************/

void refsymbol(v)
VMRef v;
{
	Symbol *s = VMAddr(Symbol,v);
	
	VMlock(v);
	if( s->global )
	{
		Module *m = VMAddr(Module,s->module);
		if( !m->linked )
		{
			m->linked = TRUE;
			m = VMAddr(Module,tailmodule);
			VMDirty(tailmodule);
			tailmodule = m->next = s->module;
			VMDirty(s->module);
		}
	}
	VMunlock(v);
}

static void extendmodtab()
{
	int i;
	VMRef *newtab;
	
	newtab = (VMRef *)malloc((modtabinc+modtabend)*sizeof(VMRef));
	
	if( newtab == NULL ) error("Cannot get space for module table");

	trace("Extending modtab from %x[%d] to %x[%d]",
			modtab,modtabend,newtab,modtabend+modtabinc);

        for( i = 0 ; i < modtabend+modtabinc ; i++ ) newtab[i] = NullVMRef;
        for( i = 0 ; i < modtabend ; i++ ) newtab[i] = modtab[i];

	free(modtab);
	
	modtab = newtab;
	modtabend += modtabinc;
}

/*  -- End of module.c -- */
