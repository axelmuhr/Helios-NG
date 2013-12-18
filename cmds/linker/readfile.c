/*{{{  Comment */

/****************************************************************/
/* Helios Linker					     	*/
/*								*/
/* File: readfile.c                                             */
/*                                                              */
/* Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994 to	*/
/*               Perihelion Software Ltd.			*/
/*                                                              */
/* Author:  NHG 17-Feb-87                                       */
/* Updates: PAB June 89						*/
/*		cleaned up from final CS code			*/
/*		latest refsym code from asm			*/
/*		recursive patches				*/
/*		fixed COMMON directive				*/
/*		implicit GLOBAL for DATAMODULE directives	*/
/*              MODSIZE directive                               */
/*          NC  December 93                                     */
/*	        Integrated Jamie's AOF -> GHOF converter        */
/*                                                              */
/****************************************************************/
/* RcsId: $Id: readfile.c,v 1.32 1994/05/04 14:46:41 nickc Exp $ */

/*}}}*/
/*{{{  #includes */

#include "link.h"

#ifdef __ARM
#include "convert.h"
#endif

/*}}}*/
/*{{{  #defines */

#define trace if (traceflags & db_gencode) _trace
#define file_trace if (traceflags & db_files) _trace
#define aof_trace if (traceflags & db_aof) _trace

/*}}}*/
/*{{{  Variables */

VMRef lastinit;
PUBLIC WORD totalcodesize;
PUBLIC UBYTE *codevec;
PUBLIC WORD curloc;
PUBLIC word bytesex = -1;

static eof = 0;

/*}}}*/
/*{{{  Prototypes */

#ifdef __STDC__
void patchpatch(VMRef pv, int ptype);
word rdint(void);
word rdch(void);
word rdword(void);
VMRef rdsymb(void);
void genpatch(word);
void geninit(void);
void genmodule(word);
void genend(void);
void genbyte(UWORD);
void copycode(void);
# if 0
void gencode(UBYTE *, word size);
VMRef deflabel(void);
# endif
#else /* !__STDC__ */
void patchpatch();
word rdint();
word rdch();
word rdword();
VMRef rdsymb();
void genpatch();
void geninit();
void genmodule();
void genend();
void genbyte();
void copycode();
# if 0
VMRef deflabel();
void gencode();
# endif
#endif /* __STDC__ */

/*}}}*/
/*{{{  Functions */

/*{{{  initcode() */

void
initcode( void )
{
  codevec  = (UBYTE *)alloc(256L);
  codepos  = 0;
  lastinit = NullVMRef;
  curloc   = 0;
}

/*}}}*/
#if defined __ARM && (defined RS6000 || defined __SUN4)
/*{{{  check_symbol() */

/* returns TRUE if pSymbolName is a code symbol */

int
check_symbol( char *	pSymbolName )
{
  char		aTmpName[ 128 ];	/* XXX */
  Symbol *	pSymbol;
  VMRef		vRef;


  if (pSymbolName == NULL)
    {
      error( "check_symbol: passed a NULL symbol name" );

      return FALSE;
    }

  if (bDeviceDriver || bSharedLib)
    aTmpName[ 0 ] = '.';
  else
    aTmpName[ 0 ] = '_';

  strcpy( aTmpName + 1, pSymbolName );
  
  /* find the named symbol */
  
  vRef = lookup( aTmpName );

  /* if the search failed the symbol does not exist */
  
  if (vRef == NullVMRef)
    {
      aof_trace( "%s does not exist", aTmpName );
      
      /* create a symbol of the given name and say that it is undefined data */

      vRef = insert( aTmpName, TRUE );
      
      pSymbol = VMAddr( Symbol, vRef );
      
      pSymbol->type           = S_UNBOUND;
      pSymbol->AOFassumedData = TRUE;
      pSymbol->file_name      = infile_duplicate;
      
      refsymbol_nondef( vRef );
      
      VMDirty( vRef );
      
      /* tell the world that the symbol is DATA */
      
      return FALSE;
    }

  /* get the symbol structure */

  pSymbol = VMAddr( Symbol, vRef );

  /* paranoia */
  
  if (pSymbol == NULL)
    {
      error( "check_symbol: symbol's structure is NULL" );
      
      return FALSE;
    }

  /* check the symbol's type */
  
  if (pSymbol->type == S_CODESYMB || /* Label in code     */
      pSymbol->type == S_FUNCSYMB || /* Unlinked function */
      pSymbol->type == S_FUNCDONE )  /* Linked function   */
    {
      aof_trace( "%s is a function", aTmpName );
      
      return TRUE;
    }

  aof_trace( "%s is data", aTmpName );
  
  return FALSE;
    
} /* check_symbol */

/*}}}*/
#endif /* __ARM and (RS6000 or __SUN4) */
/*{{{  readfile() */

void
readfile( void )
{
  VMRef 	v;
  Symbol *	s;
  word		op = 0;
  

  /*
   * Quick Test.
   * Determine if the first byte in the input file is a GHOF
   * directive.  If not then do not bother to parse the rest
   * of the file.
   */

  op = getc( infd );

  if (op >= 0x40)
    {
#if defined __ARM && (defined RS6000 || defined __SUN4)
      
      /* only support AOF -> GHOF conversion when cross linking */
      
      if (op == 0xC3 || op == 0xC5)
	{
	  s_aof *	pAOF;
	  uword		iCodeSize;
	  char *	pTempFileName;
	  
	  
	  trace( "Assuming file is in AOF format" );

	  pTempFileName = malloc( strlen( infile_duplicate ) + 5 /* strlen( ".ghof" ) */ + 1 );

	  if (pTempFileName == NULL)
	    {
	      error( "Out of memory allocating temporary file name" );

	      return;
	    }
	  
	  strcpy( pTempFileName, infile_duplicate );
	  strcat( pTempFileName, ".ghof" );

	  file_trace( "creating GHOF copy of AOF file '%s'", infile_duplicate );
	  
	  pAOF = open_aof( infile_duplicate );

	  if (pAOF == NULL)
	    {
	      error( "Failed to reopen file in AOF mode" );
	      return;
	    }

	  (void)convert_aof( pAOF, pTempFileName, bSharedLib, bTinyModel, bDeviceDriver );

	  /* Open the temporary file again */

	  file_trace( "opening GHOF copy '%s'", pTempFileName );

	  infd = freopen( pTempFileName, "rb", infd );

	  if (infd == NULL)
	    {
	      error( "Unable to reopen temporary file %s", pTempFileName );

	      return;
	    }
	  else
	    file_trace( "copy opened" );

	  free( pTempFileName );

	  /* drop throiugh into normal readfile() code */
	}
      else
#endif /* __ARM and (RS6000 or __SUN4) */
	{
	  error( "Input file is not in GHOF format" );

	  return;
	}
    }
  else
    {
      ungetc( (char)op, infd );
    }
      
  do
    {
      op = rdint();
      
      trace("OP = %x, curloc = %x, codepos = %x",op,curloc, codepos);
      
      switch( op )
	{
	default:
	  error( "Illegal linker directive '%x'", op );
	  break;
	  
	case EOF: return;
	  
	case OBJCODE:
	    {
	      word size = rdint();
	      
	      
	      if (size < 0)
         	error( "Negative sized code directive encountered (%x)", size );
	      
	      trace("CODE %d",size);
	      
	      while( size-- ) genbyte(rdch());
	      
	      break;
	    }
	  
	case OBJBSS:
	    {
	      word size = rdint();
	      
	      
	      if (size < 0)
		error("Negative sized bss defined");
	      
	      trace("BSS %d",size);
	      
	      while( size-- ) genbyte(0L);
	      
	      break;
	    }
	  
	case OBJWORD: 
	  genpatch( OBJWORD );
	  break;
	  
	case OBJSHORT: 
	  genpatch( OBJSHORT );
	  break;
	  
	case OBJBYTE:
	  genpatch( OBJBYTE );
	  break;
	  
	case OBJINIT:
	  geninit();
	  break;
	  
	case OBJMODULE:
	  genmodule(rdint());
	  break;
	  
	case OBJBYTESEX:
	  if( bytesex != 0 ) error("bytesex already set");
	  bytesex = rdint();
	  trace("BYTESEX %d",bytesex);
	  break;
	  
	case OBJREF:
	  v = rdsymb();
	  movesym(v);		/* XXX - all REF symbols are implicitly global */	 
	  refsymbol_nondef(v);
	  break;
	  
	case OBJGLOBAL:
	  v = rdsymb();
	  s = VMAddr(Symbol,v);
	  movesym(v);
	  if (s->referenced)
	    {
	      refsymbol_def(v);
	    }
	  break;
	  
	case OBJLABEL:
	  v = rdsymb();
	  s = VMAddr(Symbol,v);
	  trace("LABEL %s",s->name);
	  if( s->type != S_UNBOUND )
	    {
	      if (!inlib)
		warn( "Duplicate definition of symbol '%s' defined in file '%s'", s->name, s->file_name );
	    }
	  else
	    {
	      if (s->AOFassumedData)
		{
		  error( "(AOF) Symbol '%s' has been assumed to be data in file '%s'",
		       s->name, s->file_name );
		  
		  s->AOFassumedData = FALSE;
		}
	      
	      copycode();
	      
	      s->type      = S_CODESYMB;
	      s->value.v   = codeptr();
	      s->module    = curmod;
	      s->file_name = infile_duplicate;
	      
	      if (s->referenced)
		refsymbol_def(v);
	      
		{
		  int len = strlen( s->name );
		  
		  /* hack to insert correct name for resident libraries */
		  VMlock( v );
		  if (len > 8 &&
		      strcmp( s->name + len - 8, ".library" ) == 0 &&
		      VMAddr( asm_Module, curmod )->id     != -1 )
		    {
		      VMAddr( asm_Module, curmod )->file_name = s->name;
		    }
		  VMunlock( v );
		}
	    }
	  break;
	  
	case OBJDATA:
	case OBJCOMMON:
	    {
	      word size = rdint();
	      
	      
	      if (size < 0)
		error("Negative sized data/common directive encountered");
	      
	      v = rdsymb();
	      
	      s = VMAddr(Symbol,v);
	      
	      trace("%s %d %s",op== OBJDATA ? "DATA" : "COMMON",size,s->name);
	      
	      if( s->type != S_UNBOUND ) 
		{
		  if( s->type != S_COMMSYMB)
		    {
		      if (!inlib)
			warn("Duplicate data definition of symbol '%s' defined in file '%s'",s->name, s->file_name);
		    }
		  else {
		      if( s->value.w < size ) 
			s->value.w = size;
		    }
		}
	      else {
		  s->type = op== OBJDATA ? S_DATASYMB : S_COMMSYMB;
		  s->value.w = size;
		  s->module = curmod;
		  s->file_name = infile_duplicate;
		  if(s->referenced)
		    refsymbol_def(v);
		}
	      (void)newcode(op,0,0,curloc,v);
	      break;
	    }
	  
	case OBJCODETABLE:
	    {
	      if (!smtopt)
		error("CODETABLE directive encountered without split module table mode set");
	      
	      v = rdsymb();
	      s = VMAddr(Symbol,v);
	      
#if 0 /* problems for .MaxCodeP */
	      movesym(v);	/* implicit global directive */
#endif
	      trace("%s %s","CODETABLE",s->name);
	      if ( s->type != S_UNBOUND ) 
		{
		  if (!inlib)
		    warn("Duplicate definition of symbol '%s' defined in file '%s'",
			 s->name, s->file_name);
		}
	      else
		{ 
		  if (s->AOFassumedData)
		    {
		      error( "(AOF) Symbol '%s' has been assumed to be data in file '%s'",
			   s->name, s->file_name );
		      
		      s->AOFassumedData = FALSE;
		    }
		  
		  s->type = S_FUNCSYMB;
		  s->value.w = 4;
		  s->module = curmod;
		  s->file_name = infile_duplicate;
		  
		  if(s->referenced)
		    refsymbol_def(v);
		}
	      (void)newcode(op,0,0,curloc,v);
	      break;
	    }
	  
	}
      
    }
  while( op != EOF );
  
  return;
}

/*}}}*/
/*{{{  genbyte() */

PUBLIC void
genbyte( UWORD value )
{
  trace("genbyte %2x",value);

  if ( codepos >= 255 )
    {
      copycode();
    }
  
  codevec[codepos] = (UBYTE)value;  
  codepos++;

  return;  
}

/*}}}*/
/*{{{  copycode() */

PUBLIC void
copycode( void )
{

        if( codepos == 0 ) return;
        if( codepos <= 4 ) 
      (void)newcode((WORD) OBJLITERAL, codepos,0l,curloc,*((INT *)codevec));
        else {
                VMRef vmr = VMNew((int)codepos);
                UBYTE *v = VMAddr(UBYTE,vmr);

                codesize += codepos;

	        memcpy((char *)v, (char *)codevec, (int)codepos);

                (void)newcode((WORD) OBJCODE, codepos,0l,curloc,vmr);
        }
        curloc += codepos;
        codepos = 0;
}

/*}}}*/
/*{{{  genpatch() */

void
genpatch( word op )
{
  word type = rdint();
  int size = (int)op & 0x7;

  copycode();
  trace("Read PATCH");

  switch( type )
    {
    default:
      error("genpatch: Illegal patch type: %x",type);
      return;
      
    case OBJMODSIZE:
      trace("MODSIZE");
      (void)newcode(op,size,type,curloc,0L);
      break;

    case OBJMODNUM:
      trace("MODNUM");
      (void)newcode(op,size,type,curloc,0L);
      break;

    case OBJCODESYMB:
      if (!smtopt)
      	error("CODESYMB directive encountered without split module table mode set");
    case OBJLABELREF:
    case OBJDATASYMB:
    case OBJDATAMODULE:
#ifdef NEW_STUBS 
    case OBJCODESTUB:
    case OBJADDRSTUB:
#endif
	{
	  VMRef 	v = rdsymb();
	  Symbol *	s;

	  
	  if (type == OBJDATAMODULE)
	    {
	      movesym( v );		/* implicit GLOBAL declaration */
	    }
	  s = VMAddr( Symbol, v );
	  
	  trace( "Single level %s %s %#x",
#ifdef NEW_STUBS
		type == OBJCODESTUB   ? "CODESTUB"   :
		type == OBJADDRSTUB   ? "ADDRSTUB"   :
#endif
		type == OBJDATASYMB   ? "DATASYMB"   :
		type == OBJDATAMODULE ? "DATAMODULE" :
		type == OBJCODESYMB   ? "CODESYMB"   :
		                        "LABELREF"   ,
		s->name, v );
	  
	  refsymbol_nondef( v );
	  
	  (void)newcode( op, size, type, curloc, v );
	  
	  break;
	}
      
    case OBJPATCH:      case OBJPATCH + 1:  case OBJPATCH + 2:
    case OBJPATCH + 3:  case OBJPATCH + 4:  case OBJPATCH + 5:
    case OBJPATCH + 6:  case OBJPATCH + 7:  case OBJPATCH + 8:
    case OBJPATCH + 9:  case OBJPATCH + 10: case OBJPATCH + 11:
    case OBJPATCH + 12:
	{
	  Patch *p;
	  word pword = 0;		/* patch data */
	  int ptype;		/* patch type */
	  VMRef pv = VMNew(sizeof(Patch));
	  
	  if (type != PATCHSWAP)	/* no patch data for swap patch */
	    pword = rdint();	/* patch data */
	  
	  p = VMAddr(Patch,pv);
	  p->word = pword;
	  p->type = ptype = (int)rdint();	/* patch type */
	  
	  patchpatch(pv, ptype);	/* patch in value of type, or another patch */
	  
	  (void)newcode(op,size,type,curloc,pv);
	  break;
	}
      
    }
  
  curloc += size;

  return;

} /* genpatch */

/*}}}*/
/*{{{  patchpatch() */

void
patchpatch(
	   VMRef 	pv,
	   int		ptype )
{
  Patch	*	p;
  
  switch( ptype )
    {
    default:
      error("patchpatch: Illegal patch subtype: %x",ptype);
      break;
      
    case OBJMODSIZE:
      trace("PATCH MODSIZE");
      break;
      
    case OBJMODNUM:
      trace("PATCH MODNUM");
      break;
      
    case OBJCODESYMB:
      if (!smtopt)
	error("CODESYMB directive encountered without split module table mode set");
    case OBJDATASYMB:
    case OBJDATAMODULE:
    case OBJLABELREF:
#ifdef NEW_STUBS 
    case OBJCODESTUB:
    case OBJADDRSTUB:
#endif
	{
	  VMRef 	v = rdsymb();
	  Symbol *	s;
	  
	  
	  s = VMAddr( Symbol, v );
	  
	  if  (ptype == OBJDATAMODULE)
	    {
	      movesym( v );		/* implicit GLOBAL declaration */
	      
	      if (s->referenced)
		refsymbol_def( v );
	    }
	  else
	    {
	      refsymbol_nondef( v );
	    }
	  
	  trace("PatchPatch %s %s %#x",
		ptype == OBJDATASYMB   ? "DATASYMB"   :
		ptype == OBJDATAMODULE ? "DATAMODULE" :
		ptype == OBJCODESYMB   ? "CODESYMB"   :
#ifdef NEW_STUBS
		ptype == OBJCODESTUB   ? "CODESTUB"   :
		ptype == OBJADDRSTUB   ? "ADDRSTUB"   :
#endif
		                         "LABELREF"   ,
		s->name, v );
	  
	  p = VMAddr( Patch, pv );
	  
	  p->value.v = v;
	  
	  VMDirty( pv );
	  
	  break;
	}
      
    case OBJPATCH:      case OBJPATCH + 1:   case OBJPATCH + 2:
    case OBJPATCH + 3:  case OBJPATCH + 4:   case OBJPATCH + 5:
    case OBJPATCH + 6:  case OBJPATCH + 7:   case OBJPATCH + 8:
    case OBJPATCH + 9:  case OBJPATCH + 10:  case OBJPATCH + 11:
    case OBJPATCH + 12:
      /* recursive patch */
	{
	  Patch *p2;
	  word pword = 0; 	/* patch data */
	  int type;		/* patch type */
	  VMRef pv2 = VMNew(sizeof(Patch));
	  
	  if (ptype != PATCHSWAP)	/* no patch data for swap patch */
	    pword = rdint();	/* patch data */
	  
	  type = (int)rdint();		/* patch type */
	  
	  p = VMAddr(Patch,pv);
	  p->value.v = pv2;			/* chain patches */
          
	  p2 = VMAddr(Patch,pv2);
	  p2->word = pword;
	  p2->type = type;
	  VMDirty(pv2);
	  VMDirty(pv);
	  patchpatch(pv2, type);	/* set value of ptype, or another patch */
	}
      
    }
  
  return;
  
} /* patchpatch */

/*}}}*/
/*{{{  geninit() */

void
geninit( void )
{
  VMRef c1;


  copycode();

  c1 = newcode(OBJINIT,4l,0l,curloc,NullVMRef);

  curloc += 4;

  if ( !NullRef(lastinit) )
    {
      Code *c = VMAddr(Code,lastinit);


      c->value.v = c1;

      VMDirty(lastinit);   
    }

  lastinit = c1;

  return;
   
} /* geninit */

/*}}}*/
/*{{{  genmodule() */

PUBLIC void
genmodule( WORD mod )
{
   VMRef 	v = VMNew( sizeof (asm_Module) );
   asm_Module *	m = VMAddr( asm_Module, v );
   int 		i;

   
   VMlock(v);
   
   trace("MODULE %d",mod);

   genend();   /* finish off last module */

   m->next 	= NullVMRef;
   m->start 	= codeptr();
   m->refs   	= NullVMRef;        
   m->id 	= mod;
   m->linked 	= FALSE;
   m->file_name = infile_duplicate;
   
   for ( i = 0; i < LOCAL_HASHSIZE ; i++ )
     {
       m->symtab[ i ].head    = NullVMRef,
       m->symtab[ i ].entries = 0;
     }

   /* if this is a library module, we do not link it by default but */
   /* only if it is referenced.               */
   
   if (!inlib)
   {
      asm_Module *	tm = VMAddr( asm_Module, tailmodule );

      
      tailmodule = tm->next = v;
      m->linked  = TRUE;

      if ( NullRef( firstmodule ) )
	firstmodule = v;
   }
   
   VMunlock(curmod);        

   curmod = v;

   /* leave new curmod locked */

   (void)newcode( (WORD)OBJMODULE, 0L, 0L, curloc,v );
   
   curloc   = 0;
   lastinit = NullVMRef;
   
   return;
   
} /* genmodule */

/*}}}*/
/*{{{  genend() */

PUBLIC void
genend()
{
   asm_Module *m;


   copycode();

   m = VMAddr( asm_Module, curmod );
   
   m->length = curloc;
   
   m->end = newcode((WORD)OBJEND,0l,0l,curloc,0l);
   
   endmodule();

   return;
   
} /* genend */

/*}}}*/
/*{{{  digit() */

word
digit( void )
{
  int c;

  
  if ( eof ) return 0;
   
  c = getc(infd);
   
  while ( c == ' ' || c == '\n' || c == '\r' ) c = getc(infd);

  if ( c == EOF )
    {
      eof  = 1;
      return 0;
    }

  if( '0' <= c && c <= '9' ) return (word)c - '0';
  elif ( 'a' <= c && c <= 'z' ) return 10 + (word)c - 'a';
  else return 10 + (word)c - 'A';

} /* digit */
     

/*}}}*/
/*{{{  rdch() */

word
rdch( void )
{
#if 1
/*  
  int a;

  a = getc(infd);
  if (infd == stdin)
    IOdebug( "%x %", a );
  return a;
*/
  return getc(infd);
  
#else
   int v = digit()<<4;
   v |= digit();
   return eof?EOF:v;
#endif
}

/*}}}*/
/*{{{  rdint() */

word
rdint( void )
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
   
   c = (int)rdch();
   if ( c == EOF )
     return EOF;
   v    = c & 0x3f;
   sign = c & 0x40;
   while( (c & 0x80) != 0 )
   {
      c = (int)rdch();
      if( c == EOF ) return EOF;
      v = (v << 7) | (c & 0x7f);
   }
   return sign ? -(word)v : (word)v;
#endif   
}

/*}}}*/
#if 0
/*{{{  rdword() */

word
rdword()
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

/*}}}*/
#endif
/*{{{  rdsymb() */

VMRef
rdsymb( void )
{
  VMRef v;
  char buf[128];
  word pos = 0;
  word c;
   
  do
    {
      c = rdch();
      buf[pos++] = (char)c;
    }
  while ( c != 0 );
   
  v = lookup(buf);
   
  if ( NullRef(v) ) 
    {
      Symbol *	s;

      
      v = insert(buf,TRUE);
      s = VMAddr(Symbol,v);
      s->type = S_UNBOUND;
      s->AOFassumedData = FALSE;
    }
   
  return v;
}

/*}}}*/

/*}}}*/

/* readfile.c */
