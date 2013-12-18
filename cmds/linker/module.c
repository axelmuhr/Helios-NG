/*{{{  Comments */

/************************************************************************/
/*                                                                      */
/* File: module.c                                                       */
/*                                                                      */
/* Changes:                                                             */
/*      NHG  07-July-87   : Created                                     */
/*      NC   30-August-91 : Cosmetic changes                            */
/*                                                                      */
/* Description:                                                         */
/*      module manipualtion routines                                    */
/*                                                                      */
/*                                                                      */
/* Copyright (c) 1987 - 1994, Perihelion Software Ltd.                  */
/*   All Rights Reserved.                                               */
/************************************************************************/
/* $Id: module.c,v 1.12 1994/01/11 11:34:11 nickc Exp $ */

/*}}}*/
/*{{{  #includes */

#ifndef __TRAN
# include "link.h"
#else
# include "asm.h"
#endif

#ifdef __STDC__
# include <stdlib.h>
#endif

/*}}}*/
/*{{{  #defines */

#define trace 	if (traceflags & db_modules) _trace

#define modtabinc 128 /*64*/

/*}}}*/
/*{{{  Variables */

PUBLIC VMRef 	module0;
PUBLIC VMRef 	curmod;
PUBLIC VMRef 	firstmodule;
PUBLIC VMRef 	tailmodule;

PRIVATE word 	maxmodules    = 0;
PRIVATE word 	linkedmodules = 0;
PRIVATE word 	highmodule    = 0;
PRIVATE VMRef *	modtab        = NULL;
PRIVATE word 	modtabend     = 0;

/*}}}*/
/*{{{  forward references */

#ifdef __STDC__
void 		linkmodule( VMRef );
void 		dorefs( VMRef );
static void 	extendmodtab( void );
#else
void 		linkmodule();
void 		dorefs();
static void 	extendmodtab();
#endif

/*}}}*/
#ifdef NEW_REF
/*{{{  New Ref Variables */

VMRef		ref_start;
VMRef		ref_end;

#else

typedef struct FwdRef
  {
    struct FwdRef * 	next;
    VMRef		sym;
  }
FwdRef;

FwdRef *	fwdrefs = NULL;
FwdRef *	fwdfree = NULL;

/*}}}*/
#endif
/*{{{  Functions */

/*{{{  initmodules() */

PUBLIC void
initmodules()
{
  asm_Module *	m0;
	

  firstmodule = NullVMRef;
	
  tailmodule = curmod = module0 = VMNew( sizeof (asm_Module) );

  m0 = VMAddr( asm_Module, module0 );
  
  m0->next      = NullVMRef;
  m0->refs      = NullVMRef;
  m0->id        = 0;
  m0->linked    = TRUE;
  m0->start     = codeptr();
  m0->file_name = "Slot 0 Module";
  
#ifdef NEW_REF
  ref_end   = module0;		/* fake addr */
  ref_start = ref_end;
#endif

  VMlock( curmod );	/* current module page is always locked */

  return;
}

/*}}}*/
/*{{{  setmodules() */

/****************************************************************/
/* Procedure: setmodules                                        */
/* Description:                                                 */
/*      assign table slots to the modules                       */
/*                                                              */
/****************************************************************/

word 		lastm = 1;	/* accessed in stubs.c */

PUBLIC void
setmodules()
{
  VMRef 	v;
  asm_Module *	m;
  int 		i;
  word 		maxmodule = 0;

  
  trace( "Setmodules" );

  VMunlock( curmod );
  
  /* first assign all known modules */
  
  v = module0;
  
  while ( !NullRef( v ) )
    {
      m = VMAddr( asm_Module, v );
      
      if ( m->id != -1 )
	{
	  while ( m->id >= modtabend )
	    extendmodtab();
	  
	  if ( !NullRef( modtab[ m->id ] ) )
	    {
	      asm_Module *	other;

	      
	      other = VMAddr( asm_Module, modtab[ m->id ] );
	      
	      error( "Multiple definition of module %d in files '%s' and '%s'",
		    m->id, m->file_name, other->file_name );
	    }	  
	  
	  modtab[ m->id ] = v;

	  trace( "Pre-defined module %s at %d", m->file_name, m->id );

	  if ( m->id > maxmodule )
	    maxmodule = m->id;
	}
      
      v = m->next;
      
      linkedmodules++;
    }
  
  /* now assign the unknowns */
  
  v = module0;
  
  while ( !NullRef( v ) )
    {
      m = VMAddr( asm_Module, v );
      
      if ( m->id == -1 ) 
	{
	  while ( !NullRef( modtab[ lastm ] ) ) 
	    {
	      lastm++;
	      
	      if ( lastm >= modtabend )
		extendmodtab();
	    }
	  
	  modtab[ lastm ] = v;
	  
	  m->id = lastm;
	  
	  VMDirty( v );

	  trace( "Assign module %s to %d", m->file_name, lastm );

	  if ( m->id > maxmodule )
	    maxmodule = m->id;
	}
      
      v = m->next;
    }
  
  highmodule = maxmodule;
  
  /* now we re-build the module list with firstmodule first, but all 	*/
  /* the others in numerical order.				  	*/
  
  curmod = module0;
  
  if ( !NullRef( firstmodule ) )
    {
      VMDirty( curmod );
      
      m = VMAddr( asm_Module, curmod );
      
      curmod = m->next = firstmodule;
      
      for ( i = 1; i <= maxmodule ; i++ )
	if ( !NullRef( modtab[ i ] ) && 
	   !VMSame( modtab[ i ], firstmodule ) ) 
	  {
	    VMDirty( curmod );
	    
	    m = VMAddr( asm_Module, curmod );

	    trace( "Added module %s %d to list", m->file_name, m->id );

	    totalcodesize += m->length;

	    curmod = m->next = modtab[ i ];
	  }
    }
  
  /* null at the end of the list */
  
  m = VMAddr( asm_Module, curmod );
  
  trace( "Last module = %s %d", m->file_name, m->id );

  lastm = m->id;
  
  totalcodesize += m->length;

  m->next = NullVMRef;
  
  VMDirty( curmod );

  tailmodule = curmod;

  return;
}

/*}}}*/
/*{{{  refsymbol() */

/****************************************************************/
/* refsymbol                                        		*/
/*								*/
/* The given symbol has been referenced, if it is a global	*/
/* symbol, ensure that its defining module is in the module	*/
/* list. This only makes a difference for library modules.	*/
/*                                                              */
/****************************************************************/

void
refsymbol_def( VMRef v )
{
  Symbol *	s = VMAddr( Symbol, v );

  /* _trace("refsymbol_def %s (%d %d)", s->name, s->global, s->referenced );  */
  
  /* if this is a label definition, and it has been referenced,	*/
  /* then add it to the link.					*/

  if ( s->global && s->referenced )
    linkmodule( v );
}

/*}}}*/
/*{{{  refsymbol_nondef() */

void
refsymbol_nondef( VMRef v )
{
  Symbol *	s = VMAddr( Symbol, v );

  /* _trace( "refsymbol_nondef %s", s->name ); */

  /* if this is a label definition, and it has been referenced,	*/
  /* then add it to the link.					*/

  /* This is a reference to a symbol. If we already know	*/
  /* the current module is to be linked, just cause the		*/
  /* refrenced module to be linked too. Otherwise if it is	*/
  /* unbound, or for a different module, add it to the		*/
  /* forward reference list.					*/

  if ( s->type != S_UNBOUND && VMAddr( asm_Module, curmod )->linked )
    {
      linkmodule( v );
    }
  else if ( s->type == S_UNBOUND || !VMSame( curmod, s->module ) )
    {
#ifdef NEW_REF
      if (VMSame( s->fwd_ref, NullVMRef ))
	{
	  trace("adding %s to forward reflist", s->name ); 

	  s->fwd_ref = ref_start;
	  ref_start  = v;

	  VMDirty( v );
	}
#else
      FwdRef *	fwd;
      
      
      for( fwd = fwdrefs; fwd != NULL; fwd = fwd->next ) 
	if ( VMSame( fwd->sym, v ) )
	  return;
      
      fwd = fwdfree;
      
      if ( fwd == NULL )
	fwd = (FwdRef *)alloc( sizeof (FwdRef) );
      else
	fwdfree = fwd->next;
      
      fwd->next = fwdrefs;
      fwd->sym = v;
      fwdrefs = fwd;
#endif
    }

  return;
}

/*}}}*/
/*{{{  linkmodule() */

/* linkmodule is called when we have decided that the module referenced	*/
/* by the symbol is to be added to the link. If the symbol is as-yet	*/
/* unbound the referenced flag is set to cause it to be linked when	*/
/* defined. If the module is already linked nothing else need be done.	*/
/* Otherwise the module is marked as linked and added to the link list.	*/

void
linkmodule( VMRef v )
{
  Symbol *	s = VMAddr( Symbol, v );
  asm_Module *	m;


  s->referenced = 1;

  VMDirty( v );

  if (s->type == S_UNBOUND)
    {
      trace( "linkmodule: symbol %s not yet bound", s->name );
      
      return;	/* return if not yet bound */
    }
  
  m = VMAddr( asm_Module, s->module );
  
  if (m->linked)
    {
      trace( "linkmodule: symbol %s: module %s already linked",
	    s->name, m->file_name );
      
      return;		/* return if module already linked */
    }  

  trace( "link module %x (%s) symbol %s", s->module, m->file_name, s->name );

  m->linked = TRUE;
  m = VMAddr( asm_Module, tailmodule );

  VMDirty( tailmodule );

  tailmodule = m->next = s->module;

  VMDirty( s->module );	

  dorefs( s->module );

  return;
}

/*}}}*/
/*{{{  endmodule() */

/* Endmodule is called at the end of each module. The forward reference	*/
/* list is scanned and any unbound refs, or refs to unlinked modules 	*/
/* kept. If the current module is to be linked, linkmodule is called on	*/
/* each of these. Otherwise these are placed in Virtual Memory for later*/
/* use.									*/

void
endmodule()
{
  VMRef 	v = NullVMRef;
  VMRef *	p;
  Symbol *	s;
  asm_Module *	m;
#ifdef NEW_REF
  VMRef		fwd;
  VMRef		prev;  
#else
  FwdRef *	fwd;
  FwdRef *	prev;
  FwdRef *	next;
#endif
  int 		nrefs = 0;	
  int 		i;
  bool 		linked;


  m = VMAddr( asm_Module, curmod );

  linked = m->linked;
  
  trace( "endmodule %s (linked = %d)", m->file_name, linked );
  
  /* first scan fwdrefs & throw out all local & refs to linked modules */

#ifdef NEW_REF
  prev = NullVMRef;

  for (fwd = ref_start; !VMSame( fwd, ref_end );)
    {
      s = VMAddr( Symbol, fwd );

      if ((s->type == S_UNBOUND) ||
	 (s->global && !VMSame( curmod, s->module ) && !VMAddr( asm_Module, s->module )->linked) )
	{
	  nrefs++;

	  trace( "adding %s to global reflist", s->name ); 
	  
	  prev = fwd;

	  fwd = s->fwd_ref;
	}
      else
	{
	  /* remove from forward reference list */

	  if (VMSame( prev, NullVMRef ))
	    ref_start = s->fwd_ref;
	  else
	    {
	      Symbol *	p = VMAddr( Symbol, prev );

	      
	      p->fwd_ref = s->fwd_ref;
	    }

	   trace("removing %s from global reflist", s->name ); 
	  
	  fwd = s->fwd_ref;
	  
	  s->fwd_ref = NullVMRef;

	  VMDirty( fwd );
	}
    }
#else
  for ( fwd = fwdrefs,prev = NULL; fwd != NULL; )
    {
      s = VMAddr( Symbol, fwd->sym );
      
      if ((s->type == S_UNBOUND) ||
	 (s->global && !VMSame( curmod, s->module ) && !VMAddr( asm_Module, s->module )->linked) )
	{
	  nrefs++;

	  prev = fwd;
	  fwd  = fwd->next;
	}
      else
	{
	  next = fwd->next;

	  if ( prev == NULL )
	    fwdrefs = next;
	  else
	    prev->next = next;
	  
	  fwd->next = fwdfree;
	  fwdfree   = fwd;
	  fwd       = next;
	}
    }
#endif /* NEW_REF */
  
  /* the fwdrefs list now only contains unbound refs or refs to		*/
  /* currently un-linked modules. nrefs is the number of these.		*/
  /* If we know that this module is to be linked, just mark all		*/
  /* symbols as referenced, otherwise make a list of these refs		*/
  /* in virtual memory for use when the module is linked.		*/
  
  trace( "unbound refs %d %slinked", nrefs, linked ? "" : "un" );

  if ( nrefs != 0 ) 
    {
      if ( !linked )
	{
	  v = VMNew( sizeof (VMRef) * (nrefs + 1) );
	 
	  VMDirty( v );

#ifdef NEW_REF
	  for (fwd = ref_start, i = 0; i < nrefs; i++)
	    {
	      p = VMAddr( VMRef, v );
	      
	      p[ i ] = fwd;

	      s = VMAddr( Symbol, fwd );

	      fwd = s->fwd_ref;
	      
	      s->fwd_ref = NullVMRef;

	      VMDirty( fwd );
	    }

	  p = VMAddr( VMRef, v );
#else	  
	  p = VMAddr( VMRef, v );

	  for ( i = 0; fwdrefs != NULL; i++ )
	    {
	      fwd = fwdrefs;
	  
	      
	      p[ i ] = fwd->sym;
	  
	      fwdrefs   = fwd->next;
	      fwd->next = fwdfree;
	      fwdfree   = fwd;
	    }
#endif /* NEW_REF */
	  
	  p[ nrefs ] = NullVMRef;
	}
      else
	{
#ifdef NEW_REF
	  for (fwd = ref_start; !VMSame( fwd, ref_end );)
	    {
	      linkmodule( fwd );
	      
	      s = VMAddr( Symbol, fwd );

	      fwd = s->fwd_ref;

	      s->fwd_ref = NullVMRef;

	      VMDirty( fwd );
	    }
#else
	  for ( i = 0; fwdrefs != NULL; i++ )
	    {
	      fwd = fwdrefs;
	  
	      linkmodule( fwd->sym );
	  
	      fwdrefs   = fwd->next;
	      fwd->next = fwdfree;
	      fwdfree   = fwd;
	    }
#endif
	}

#ifdef NEW_REF
      ref_start = ref_end;
#endif
    }

  m = VMAddr( asm_Module, curmod );
  
  m->refs = v;
  
  VMDirty( curmod );
  
  trace( "endmodule %s done", m->file_name );

  return;
}

/*}}}*/
/*{{{  dorefs() */

/* dorefs is called when a module is to be linked. It applies	*/
/* linkmodule to each entry in the pending ref list.		*/

void
dorefs( VMRef mod )
{
  asm_Module *	m = VMAddr( asm_Module, mod );
  VMRef 	r = m->refs;
  VMRef *	p;
  int 		i;

  
  trace( "scanning refs for %s", m->file_name );

  if ( NullRef( r ) )
    {
      trace( "no refs to scan" );
      
      return;
    }

  VMlock( r );
  
  p = VMAddr( VMRef, r );
  
  for (i = 0; !NullRef( p[ i ] ); i++ ) 
    {
      trace( "linking in module containing '%s'", VMAddr( Symbol, p[i] )->name );
      
      linkmodule( p[ i ] );
    }
  
  VMunlock( r );

  trace( "refs scanned" );
  
  return;
}

/*}}}*/
/*{{{  extendmodtab() */

static void
extendmodtab()
{
  word 		i;
  VMRef *	newtab;

  
  newtab = (VMRef *)malloc( (int)(modtabinc + modtabend) * sizeof (VMRef) );
  
  if ( newtab == NULL )
    error( "Cannot get space for module table" );
  
  trace( "Extending modtab from %x[%d] to %x[%d]", modtab, modtabend, newtab, modtabend + modtabinc );
  
  for( i = modtabend ; i < modtabend + modtabinc ; i++ ) newtab[ i ] = NullVMRef;
  for( i = 0 ;         i < modtabend ;             i++ ) newtab[ i ] = modtab[ i ];

  if (modtab != NULL)
    free( modtab );
  
  modtab     = newtab;
  modtabend += modtabinc;

  return;
}

/*}}}*/
/*{{{  modstats() */

void
modstats()
{
  report( "Link Statistics: Total Modules  %8ld Linked Modules %8ld", maxmodules, linkedmodules - 1 );	
  report( "                 Highest Module %8ld", highmodule );

  return;
}

/*}}}*/

/*}}}*/

