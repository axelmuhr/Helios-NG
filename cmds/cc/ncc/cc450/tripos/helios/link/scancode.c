#include "link.h"

#define trace if(traceflags&db_scancode) _trace

void scancode()
{
   word datapos;
   Symbol *s;   
   Code *c;
   Module *m;
   
   VMRef curblock;
   
   curmod = module0;
   m = VMAddr(Module,curmod);
   curblock = m->start;
   VMlock(curblock);
   
   c = VMAddr(Code,curblock);
   VMDirty(curblock);
   
   datapos = 0;
   
   for(;;)
   {
      word tag = c->type;
      
      trace("scan op %x",tag);
      
      switch( tag )
      {
         
      /* go to new code page            */
      case t_newseg:
         trace("NEWSEG %x",c->value.v);
         VMunlock(curblock);
         curblock = c->value.v;
         VMlock(curblock);
         c = VMAddr(Code,curblock);
         VMDirty(curblock);
         continue;
         
      /* start new module            */
      case t_end:
         VMunlock(curblock);
         m = VMAddr(Module,curmod);
         curmod = m->next;
         if( NullRef(curmod) ) return;
         m = VMAddr(Module,curmod);
         trace("NEW MODULE %x %d",curmod,m->id);
         curblock = m->start;
         VMlock(curblock);
         c = VMAddr(Code,curblock);
         VMDirty(curblock);
         datapos = 0;
         continue;
         
      case t_data:
      {
         word size;
         s = VMAddr(Symbol,c->value.v);
         trace("DATA %d %s",s->value.w,s->name);
         if( s->type != s_datasymb ) 
            warn("Illegal symbol in DATA directive: %s",s->name);
         s->type = s_datadone;
         size = s->value.w;
         s->value.w = datapos;
         datapos += size;
         VMDirty(c->value.v);
         break;
      }

      case t_common:
      {
         word size;
         s = VMAddr(Symbol,c->value.v);
         trace("COMMON %d %s",s->value.w,s->name);
         if( s->type == s_commsymb ) 
         {
            s->type = s_commdone;
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

