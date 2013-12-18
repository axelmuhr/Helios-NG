
#include "link.h"

#define trace if(traceflags&db_gencode) _trace

VMRef lastinit;
PUBLIC WORD totalcodesize;
PUBLIC UBYTE *codevec;
PUBLIC WORD codepos;
PUBLIC WORD curloc;

PUBLIC word bytesex = -1;

word rdint(ellipsis);
word rdch(ellipsis);
word rdword(ellipsis);
VMRef rdsymb(ellipsis);
void genpatch(ellipsis);
void geninit(ellipsis);
void genmodule(ellipsis);
void genend(ellipsis);
void genbyte(ellipsis);
void gencode(ellipsis);
void copycode(ellipsis);
VMRef deflabel(ellipsis);

void initcode()
{
   codevec = alloc(256L);
   codepos = 0;
   lastinit = NullVMRef;
   curloc = 0;   
}

void readfile()
{
   VMRef v;
   Symbol *s;
   word op = 0;
      
   do {
      op = rdint();
      
      trace("OP = %x, curloc = %x, codepos = %x",op,curloc, codepos);
      
      switch( op )
      {
      default:
         error("Illegal op in object file: %x",op);
         break;

      case EOF: return;
      
      case t_code:
      {
         word size = rdint();
         trace("CODE %d",size);
         while( size-- ) genbyte(rdch());
         break;
      }

      case t_bss:
      {
         word size = rdint();
         trace("BSS %d",size);
         while( size-- ) genbyte(0L);
         break;
      }
      
      case t_word: 
         genpatch(t_word);
         break;
         
      case t_short: 
         genpatch(t_short);
         break;
         
      case t_byte:
         genpatch(t_byte);
         break;
         
      case t_init:
         geninit();
         break;

      case t_module:
         genmodule(rdint());
         break;
         
      case t_bytesex:
         if( bytesex != 0 ) error("bytesex already set");
         bytesex = rdint();
         trace("BYTESEX %d",bytesex);
         break;
         
      case t_global:
         v = rdsymb();
         s = VMAddr(Symbol,v);
         trace("GLOBAL %s",s->name);
         movesym(v);
         break;
      
      case t_label:
         v = rdsymb();
         s = VMAddr(Symbol,v);
         trace("LABEL %s",s->name);
         if( s->type != s_unbound ) 
            warn("Duplicate definition of '%s'",s->name);
         else {
            copycode();
            s->type = s_codesymb;
            s->value.v = codeptr();
            s->module = curmod;
         }
         break;
            
      case t_data:
      case t_common:
          {
         word size = rdint();
         v = rdsymb();
         s = VMAddr(Symbol,v);
         trace("%s %d %s",op==t_data?"DATA":"COMMON",size,s->name);
         if( s->type != s_unbound ) 
         {
            if( s->type != t_common )
               warn("Duplicate definition of '%s'",s->name);
            else {
               if( s->value.w < size ) 
                  s->value.w = size;
            }
         }
         else {
            s->type = op==t_data?s_datasymb:s_commsymb;
            s->value.w = size;
            s->module = curmod;
         }
         newcode(op,0,0,curloc,v);
         break;
          }
            
                  
      }
   
   } while( op != EOF );
}

PUBLIC void gencode(code,size)
UBYTE *code;
WORD size;
{
   int i;
   for( i = 0 ; i < size; i++ ) genbyte((WORD)code[i]);
}

PUBLIC void genbyte(value)
UWORD value;
{
        trace("genbyte %2x",value);
        if ( codepos >= 255 ) copycode();
        codevec[codepos] = value;
   codepos++;
}

PUBLIC void copycode()
{
        if( codepos == 0 ) return;
        if( codepos <= 4 ) 
      newcode((WORD)t_literal,codepos,0l,curloc,*((INT *)codevec));
        else {
                VMRef vmr = VMNew(codepos);
                UBYTE *v = VMAddr(UBYTE,vmr);

                codesize += codepos;

      memcpy(v,codevec,codepos);
      
                newcode((WORD)t_code,codepos,0l,curloc,vmr);
        }
        curloc += codepos;
        codepos = 0;
}

void genpatch(op)
word op;
{
   word type = rdint();
   VMRef v;
   int size = op & 0x7;
   Symbol *s;

   copycode();
   trace("PATCH");
   switch( type )
   {
   default:
      error("Illegal patch type: %x",type);
      return;
      
   case t_modnum:
      trace("MODNUM");
      newcode(op,size,type,curloc,0L);
      break;

   case t_labelref:
   case t_dataref:
   case t_datamod:
      v = rdsymb();
      s = VMAddr(Symbol,v);
      trace("%s %s",type==t_dataref?"DATAREF":
                    type==t_datamod?"DATAMOD": "LABELREF",s->name);
      refsymbol(v);
      newcode(op,size,type,curloc,v);
      break;
      
   case t_patch:   case t_patch+1:   case t_patch+2:
   case t_patch+3:   case t_patch+4:   case t_patch+5:
   case t_patch+6:   case t_patch+7:   case t_patch+8:
   case t_patch+9:   case t_patch+10:case t_patch+11:
   case t_patch+12:
   {
      Patch *p;
      word pword = rdint();
      int ptype = rdint();
      VMRef pv = VMNew(sizeof(Patch));
      
      p = VMAddr(Patch,pv);
      p->word = pword;
      
      switch( p->type = ptype )
      {
      default:
         error("Illegal patch subtype: %x",ptype);
         break;

      case t_modnum:
         trace("PATCH MODNUM");
         break;

      case t_dataref:
      case t_datamod:
         v = rdsymb();
         refsymbol(v);
         s = VMAddr(Symbol,v);
         trace("PATCH %s %s",type==t_dataref?"DATAREF":"DATAMOD",s->name);
         p = VMAddr(Patch,pv);
         p->value.v = v;
         break;
      }
      newcode(op,size,type,curloc,pv);
      break;
   }
   
   }
   curloc += size;
}

void geninit()
{
   VMRef c1;
   copycode();
   c1 = newcode(t_init,4l,0l,curloc,NullVMRef);
   curloc += 4;
   if( !NullRef(lastinit) )
   {
      Code *c = VMAddr(Code,lastinit);
      c->value.v = c1;
      VMDirty(lastinit);   
   }
   lastinit = c1;
}

PUBLIC void genmodule(mod)
WORD mod;
{
        int i;
        VMRef v = VMNew(sizeof(Module));
        Module *m;

   VMlock(v);
   
   trace("MODULE %d",mod);
   
   m = VMAddr(Module,v);
   
   genend();   /* finish off last module */

   curloc = 0;

   m->next = NullVMRef;
        m->start = codeptr();
        m->id = mod;
        m->linked = FALSE;
        for( i = 0; i < HASHSIZE ; i++ ) m->symtab[i].head = NullVMRef,
                     m->symtab[i].entries = 0;

   newcode((WORD)t_module,0l,0l,curloc,v);

   /* if this is a library module, we do not link it by default but*/
   /* only if it is referenced.               */
   if( !inlib )
   {
      Module *tm = VMAddr(Module,tailmodule);
      tailmodule = tm->next = v;
             m->linked = TRUE;
             if( NullRef(firstmodule) ) firstmodule = v;
   }
   
          VMunlock(curmod);        
        curmod = v;
        lastinit = NullVMRef;
        trace("curmod = %x",curmod);
        
   /* leave new curmod locked */
}

PUBLIC void genend()
{
   Module *m;
   copycode();
   totalcodesize += curloc;
   trace("Last module size = %d, total code size = %d",curloc,totalcodesize);
   m = VMAddr(Module,curmod);
        m->end = newcode((WORD)t_end,0l,0l,curloc,0l);
}

static eof = 0;

word digit()
{
   int c;
   
   if( eof ) return 0;
   
   c = getc(infd);
   
   while( c == ' ' || c == '\n' || c == '\r' ) c = getc(infd);

   if( c == EOF ) { eof = 1 ; return 0; }

   if( '0' <= c && c <= '9' ) return c - '0';
   elif ( 'a' <= c && c <= 'z' ) return 10 + c - 'a';
   else return 10 + c - 'A';
}

word rdch()
{
#if 1
   return getc(infd);
#else
   int v = digit()<<4;
   v |= digit();
   return eof?EOF:v;
#endif
}

word rdint()
{
#if 0   
   int v = 0;
   int c;

   do {
      c = rdch();
      if( c == EOF ) return EOF;
      v = (v<<7) | (c&0x7f);
   } while( (c & 0x80) != 0 );
   return v;
#else
   int v;
   int c;
   int sign;
   
   c = rdch();
   if( c == EOF ) return EOF;
   v = c&0x3f;
   sign = c & 0x40;
   while( (c & 0x80) != 0 )
   {
      c = rdch();
      if( c == EOF ) return EOF;
      v = (v<<7) | (c&0x7f);
   }
   return sign ? -v : v;
#endif   
}

#if 0
word rdword()
{
   int v = 0;

   if( bytesex <= 0 )
   {
      v |= rdch();
      v |= rdch()<<8;
      v |= rdch()<<16;
      v |= rdch()<<24;
   }
   else {
      v |= rdch()<<24;
      v |= rdch()<<16;
      v |= rdch()<<8;
      v |= rdch();
   }
   return v;
}
#endif

VMRef rdsymb()
{
   VMRef v;
   char buf[128];
   word pos = 0;
   word c;
   
   do {
      c = rdch();
      buf[pos++] = c;
   } while( c != 0 );
   
   v = lookup(buf);
   
   if( NullRef(v) ) 
   {
      Symbol *s;
      v = insert(buf,TRUE);
      s = VMAddr(Symbol,v);
      s->type = s_unbound;
   }
   
   return v;
      
}
