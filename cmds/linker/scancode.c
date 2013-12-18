/************************************************************************/
/* Helios Linker							*/
/*                                                                      */
/* File: scancode.c                                                     */
/*                                                                      */
/*                                                                      */
/* Copyright (c) 1987 - 1991, Perihelion Software Ltd.                  */
/*    All Rights Reserved.                                              */
/************************************************************************/
/* RcsId: $Id: scancode.c,v 1.7 1992/08/04 16:55:36 nickc Exp $ */

#include "link.h"

#define trace if(traceflags&db_scancode) _trace

void scancode()
{
  word 		datapos;
  word		funcpos;
  Symbol *	s;   
  Code *	c;
  asm_Module *	m;
   
   VMRef curblock;
   
   curmod = module0;
   m = VMAddr(asm_Module,curmod);
   curblock = m->start;
   VMlock(curblock);
   
   c = VMAddr(Code,curblock);

   VMDirty(curblock);
   
   datapos = funcpos = 0;
   
   for(;;)
   {
      word tag = c->type;
      
      trace("scan op %x",tag);
      
      switch( tag )
      {
         
      /* go to new code page            */
      case OBJNEWSEG:
         trace("NEWSEG %x",c->value.v);
         VMunlock(curblock);
         curblock = c->value.v;
         VMlock(curblock);
         c = VMAddr(Code,curblock);
         VMDirty(curblock);
         continue;
         
      /* start new module            */
      case OBJEND:
         VMunlock(curblock);
         m = VMAddr(asm_Module,curmod);
         curmod = m->next;
         if( NullRef(curmod) ) return;
         m = VMAddr(asm_Module,curmod);
         trace("NEW MODULE %x %d",curmod,m->id);
         curblock = m->start;
         VMlock(curblock);
         c = VMAddr(Code,curblock);
         VMDirty(curblock);
         funcpos = datapos = 0;
         continue;
         
      case OBJDATA:
      {
         word size;

         s = VMAddr(Symbol,c->value.v);
         trace("DATA %d %s %#x",s->value.w,s->name,c->value.v);

         if ( s->type == S_DATADONE )
	   {
	     m = VMAddr( asm_Module, curmod );
	     
	     inform( "Duplicate definition of DATA symbol '%s' in files %s and %s - using the former",
		  s->name, s->file_name, m->file_name );
	   }
         else if ( s->type != S_DATASYMB )
	   {
	     warn( "Illegal symbol '%s' in DATA directive of file %s", s->name, s->file_name );
	   }
	 else
	   {
	     s->type    = S_DATADONE;
	     size       = s->value.w;
	     s->value.w = datapos;
	     datapos   += size;
	     VMDirty(c->value.v);
	   }	 
         break;
      }

      case OBJCODETABLE:
      {
         s = VMAddr(Symbol,c->value.v);
         trace("CODETABLE %s %#x (type %x)", s->name, c->value.v, s->type);
         if( s->type == S_FUNCDONE ) 
	   warn( "Duplicate definition of function '%s' in files %s and %s",
		s->name + 1, s->file_name, m->file_name );
         else if( s->type != S_FUNCSYMB ) 
            warn("Illegal symbol '%s' in CODETABLE directive of file %s", s->name, s->file_name );
         s->type = S_FUNCDONE;
         s->value.w = funcpos;
         funcpos += 4;
         VMDirty(c->value.v);
         break;
      }

      case OBJCOMMON:
      {
         word size;
         s = VMAddr(Symbol,c->value.v);
         trace("COMMON %d %s",s->value.w,s->name);
         if( s->type == S_COMMSYMB ) 
         {
            s->type = S_COMMDONE;
            size = s->value.w;
            s->value.w = datapos;
            datapos += size;
            VMDirty(c->value.v);
         }
         break;
      }
      
      default:
         break;
      }
      
      c++;
   }
}
