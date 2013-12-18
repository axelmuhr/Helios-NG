#include "link.h"

#define trace if(traceflags&db_genimage) _trace

void putpatch(Code *c);
word mcpatch(word type, VMRef v);
word stdpatch(word type,Value value);

static void outword(ellipsis);
static void outbyte(ellipsis);
static void outbyte1(ellipsis);
static void imageheader(ellipsis);
static word nchars;
static word codepos;

void genimage()
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
   codepos = 0;

   imageheader(totalcodesize+4);
      
   for(;;)
   {
      word tag = c->type;
      
      trace("image op %x",tag);
      
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
         if( NullRef(curmod) )
         { outword(0);
           return;
         }
         m = VMAddr(Module,curmod);
         trace("NEW MODULE %x %d",curmod,m->id);
         curblock = m->start;
         VMlock(curblock);
         c = VMAddr(Code,curblock);
         VMDirty(curblock);
         datapos = 0;
         codepos = 0;
         continue;

      case t_module:
      case t_data:
      case t_common:
         break;
      
      case t_code:
      {
         int i, size = c->size;
         UBYTE *v = VMAddr(UBYTE,c->value.v);
         for( i = 0; i < size ; outbyte(v[i++]) );
         codepos += size;
         break;
      }
            
      case t_bss:
      {
         int i, size = c->size;
         for( i = 0; i < size ; outbyte(0) );
         codepos += size;
         break;
      }
      
      case t_literal:
      {
         int i, size = c->size;
         UBYTE *v = (UBYTE *)&c->value.w;
         for( i = 0; i < size ; outbyte(v[i++]) );
         codepos += size;
         break;
      }

      case t_word:
      case t_short:
      case t_byte:
         putpatch(c);
         codepos += tag & 0x07;
         break;
         
      case t_init:
         if( NullRef(c->value.v) ) outword(0L);
         else 
         {
            word next = VMAddr(Code,c->value.v)->loc;
            outword((WORD)(next-c->loc));
         }
         codepos += 4;
         break;
                  
      default:
         error("Unknown tag %x",tag);
         break;
      }
      
      c++;
   }
}

void putpatch(c)
Code *c;
{
   word value = 0;
   
   if( t_patch <= c->vtype && c->vtype <= t_maxpatch )
   {
      value = mcpatch(c->vtype,c->value.v);
   }
   else 
   {
      value = stdpatch(c->vtype,c->value);
   }
   
   trace( "Value = %d", value);
   switch( c->type )
   {
   case t_word:
      outword(value);
      break;
      
   case t_short:
      if( value < 0xffff8000 || value > 0xffff )
         warn("Value %x too large for short",value);
#ifdef BYTESEX_EVEN
      outbyte1(value);
      outbyte1(value>>8);
#else
      outbyte1(value>>8);
      outbyte1(value);
#endif
      break;
      
   case t_byte:
      if( value < -128 || value > 255 )
         warn("Value %x too large for byte",value);
      outbyte1(value);
      break;
   }
}

word stdpatch(vtype,value)
word vtype;
Value value;
{
   Module *m;
   Symbol *s;

   switch( vtype )
   {
   default:
      error("Unknown type in stdpatch: %x",vtype);
      return 0;
      
   case t_modnum:
      m = VMAddr(Module,curmod);
      trace("Stdpatch: returning module %08x, number %d",curmod, m->id);
      return m->id;

   case t_labelref:
      {  word r;
         s = VMAddr(Symbol, value.v);
         if( s->type != s_codesymb )
         {  warn("Label \"%s\" not defined: assuming offset 0",s->name);
            return 0;
         }
         r = VMAddr(Code,s->value.v)->loc;
         trace("Stdpatch: returning label offset of %s = %d",s->name, r);
         return r - codepos;
      }
   case t_dataref:
      s = VMAddr(Symbol,value.v);
      if( s->type != s_datadone )
      {  warn("Symbol \"%s\" not defined: assuming offset 0",s->name);
         return 0;
      }
      return s->value.w;
      
   case t_datamod:
      s = VMAddr(Symbol,value.v);
      if( s->type != s_datadone )
      {  warn("Symbol \"%s\" not defined: assuming module 0",s->name);
         return 0;
      }
      m = VMAddr(Module,s->module);
      return m->id;
   }
}

/****************************************************************/
/* imageheader                                                  */
/*                                                              */
/* Generate image file header                                   */
/*                                                              */
/****************************************************************/

PRIVATE void imageheader(imagesize)
WORD imagesize;
{
        outword(0x12345678L);
        outword(0L);
        outword(imagesize);
        nchars = 0;
}

/****************************************************************/
/* outword                                                      */
/*                                                              */
/* output a word to image file                                  */
/*                                                              */
/****************************************************************/

PRIVATE void outword(val)
WORD val;
{
        int i;
#ifdef BYTESEX_EVEN
        for( i = 0 ; i < 32 ; i+=8 ) outbyte((UBYTE)(val>>i));
#else
   for( i = 24 ; i >= 0 ; i-=8 ) outbyte((UBYTE)(val>>i));
#endif
}

/****************************************************************/
/* Procedure: outbyte                                           */
/*                                                              */
/*      output a byte to image file                             */
/*                                                              */
/****************************************************************/

PRIVATE void outbyte1(b)
WORD b;
{
   outbyte((UBYTE)b);
}

PRIVATE void outbyte(b)
UBYTE b;
{
        if( hexopt )
        {
                if( (nchars % 16) == 0 ) putc('\n',outfd);
                fprintf(outfd,"%02x ",(UBYTE)b);
        }
        else {
                putc(b,outfd);
        }
        nchars++;
}

#ifdef ARM
word mcpatch(type,v)
word type;
VMRef v;
{
   Patch *p = VMAddr(Patch,v);
   type = type;
   return p->word | stdpatch(p->type,p->value);
}
#endif

#ifdef PGC1
word mcpatch(type,v)
word type;
VMRef v;
{
   Patch *p = VMAddr(Patch,v);
   type = type;
   return p->word | stdpatch(p->type,p->value);
}
#endif

#ifdef M68K
word mcpatch(type,v)
word type;
VMRef v;
{
   Patch *p = VMAddr(Patch,v);
   type = type;
   return p->word + stdpatch(p->type,p->value);
}
#endif
