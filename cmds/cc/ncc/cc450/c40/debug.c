/* $Id: debug.c,v 1.25 1993/07/27 15:04:19 nickc Exp $ */
/* Copyright (c) 1993 Perihelion Software Ltd. */

#include "target.h"

#ifdef TARGET_HAS_DEBUGGER

#define DEBUG_VARS	(1<<0)
#define DEBUG_LINES	(1<<1)
#define DEBUG_PROCS	(1<<2)

#define DEBUG_LEVEL	DEBUG_VARS

#include <stdio.h>
#include <stdarg.h>
#ifndef __C40
#define __C40
#endif
#include <module.h>
#include <string.h>
#include "globals.h"
#include "builtin.h"
#include "store.h"
#include "aeops.h"
#include "cg.h"
#include "util.h"
#include "mcdep.h"
#include "mcdpriv.h"
#include "xrefs.h"
#include "codebuf.h"	/* for codebase, codep */
#include "flowgraf.h"	/* for current_env */
#include "bind.h"	/* for globalize_int */
#include "regalloc.h"	/* for register_number() */
#include "sem.h"	/* for sizeoftype() */
#include "lex.h"	/* for curlex */
#include "errors.h"	/* for cc_warn() etc */

extern int32	hardware_register( RealRegister reg );

/*
 * class definitions.  Note that capital
 * letters are internal types, and should
 * not be printed in the debugging file
 */

#define CLASS_UNKNOWN	'Z'
#define CLASS_TYPEDEF	'T'
#define CLASS_ARGUMENT	'A'

#define CLASS_GLOBAL	'g'
#define CLASS_STATIC	's'
#define CLASS_LOCAL	'l'
#define CLASS_REGISTER	'r'

#ifndef T_SourceInfo		/* because of a bug in module.h */
#define T_SourceInfo		T_FileName
#endif

#ifndef sizeof_int
#define sizeof_int 		4
#endif

#ifndef padsize
#define padsize( n, align ) 	(-((-(n)) & (-(align))))
#endif

#ifndef streq
#define streq( a, b )		(strcmp( a, b ) == 0)
#endif

#define DbgAlloc( type )	(type *)GlobAlloc( SU_Dbg, sizeof (type) )


struct stdef
  {
    struct stdef *		next;
    int				done;
    TagBinder *			b;
  }
*stlist;

typedef struct FileName
  {
    struct FileName *		next;
    LabelNumber	*		lab;
    char *			name;	
  }
FileName;

static FileName *		filenamelist 	= NULL;

typedef struct Dbg_VarList
  {
    struct Dbg_VarList *	next;		/* chain of allocated Dbg_VarLists		*/
    Binder *			binder;		/* the binder associated with this variable	*/
    char *			type;		/* a string describing the type  of the value	*/
    char			Class;		/* a letter describing the class of the value	*/
    int32			offset;		/* the distance from wherever of the value	*/
    int32			size;		/* word padded size of the variable in bytes	*/
  }
Dbg_VarList;

#define varnext_( v )		((v)->next)
#define varbind_( v )		((v)->binder)
#define vartype_( v )		((v)->type)
#define varclass_( v )		((v)->Class)
#define varoffset_( v )		((v)->offset)
#define varsize_( v )		((v)->size)

static Dbg_VarList *		dbg_varlist 	= NULL;
static Dbg_VarList *		dbg_freevars	= NULL;

typedef struct Dbg_LineList
  {
    struct Dbg_LineList *	next;		/* chain of allocated Dbg_LineList structures	*/
    int32			first;		/* first line number in the sub-block		*/
    int32			last;		/* last  line number in the sub-block		*/
  }
Dbg_LineList;

static Dbg_LineList *		dbg_freelines	= NULL;

typedef struct Dbg_Block
  {
    struct Dbg_Block *		next;		/* chain of alloacted Dbg_Blocks 		*/
    struct Dbg_Block *		parent;		/* parent block of this block	 		*/
    struct Dbg_LineList *	linelist;	/* linked list of line numbers	 		*/
    struct Dbg_VarList *	varlist;	/* linked list of variables at this scope 	*/
    int				block_id;	/* id used for debugging purposes		*/
  }
Dbg_Block;

static Dbg_Block *		current_block 	= NULL;
static Dbg_Block *		blocklist     	= NULL;
static Dbg_Block *		dbg_freeblocks	= NULL;
static int			current_block_id = 0;


int 		usrdbgmask	= 0;
char 		dbg_name[ 4 ] 	= "tla";
Symstr *	current_proc	= NULL;
LabelNumber *	proc_label	= NULL;

static FILE *	dbfile 		= NULL;
static int32 	lastdataloc 	= 0;
static int32 	lastfuncloc 	= 0;
static int32	num_args    	= 0;
static int32 	indent 		= 0;


#ifdef __STDC__
static void
dbprintf(
	 char *	buffer,
	 char *	fmt,
	 ... 		)
#else
static void
dbprintf( buffer, fmt, va_alist )
  char *	buffer;
  char *	fmt;
  va_dcl
#endif
{
  va_list a;
	

  va_start( a, fmt );

  if (buffer == NULL)
    vfprintf( dbfile, fmt, a );
  else
    vsprintf( buffer + strlen( buffer ), fmt, a );
	
  va_end( a );

  return;

} /* dbprintf */


void
#ifdef __STDC__
db_init( char *	name )
#else
db_init( name )
  char *	name;
#endif
{
  char 		dbname[ 30 ];
  char *	p = dbname;


  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_PROCS))
    cc_msg( "debugger: initialising for file %s\n", name );

  strcpy( dbname, name );
	
  while ( *p && (*p != '.') )
    p++;

  *p = 0;
	
  strcat( dbname, ".dbg" );
	
  dbfile = fopen( dbname, "w" );

  if (dbfile == NULL)
    syserr( debug_cannot_open_output, dbname );

  dbprintf( NULL, "void=#0;\n"  );
  dbprintf( NULL, "char=#1;\n"  );
  dbprintf( NULL, "short=-2;\n" );
  dbprintf( NULL, "int=-4;\n"   );
  dbprintf( NULL, "long=-4;\n"  );

  dbprintf( NULL, "signed char=-1;\n" );	/* XXX changed from -2 by NC 26/3/92 */
	
  dbprintf( NULL, "unsigned short=#2;\n" );
  dbprintf( NULL, "unsigned int=#4;\n"   );
  dbprintf( NULL, "unsigned long=#4;\n"  );

  dbprintf( NULL, "enum=#4;\n"   );
  dbprintf( NULL, "float=.4;\n"  );
  dbprintf( NULL, "double=.8;\n" );

  return;
  
} /* db_init */


static void
#ifdef __STDC__
addstdef( TagBinder * b )
#else
addstdef( b )
  TagBinder *	b;
#endif
{
  struct stdef *	s    = stlist;
  int 			done = (tagbindmems_( b ) != 0);


  while (s)
    {
      if (s->b == b)
	{
	  if (done)
	    s->done = 1;

	  return;
	}

      s = s->next;

    }

  stlist = (struct stdef *)global_list3( SU_Dbg, stlist, done, b );

  return;
  
} /* addstdef */


static int32
#ifdef __STDC__
db_type(
	TypeExpr * 	x,
	char *		buffer )
#else
db_type( x, buffer )
  TypeExpr * 	x;
  char *	buffer;
#endif
{
  SET_BITMAP 	m;
  int32		bitoff;
  ClassMember *	pClassMember = NULL;
  TagBinder *	b;
  int32		n;
  int32		size;
  int32		padn = 0;
            

#ifdef DEBUG
  if (x == NULL)
    syserr( syserr_null_type_expr );
#endif
  
  switch (h0_( x ))
    {   
    case s_typespec:
      m = typespecmap_( x );

      switch (m & -m)    /* LSB - unsigned/long etc. are higher */
	{   
	case bitoftype_( s_char ):
	  if (m & bitoftype_( s_unsigned ))
	    {
	      dbprintf( buffer, "char" );
	    }
	  else
	    {
	      dbprintf( buffer, "signed char" );
	    }
	  
	  return sizeof_char;
	  
	case bitoftype_( s_int ):
	  if (m & BITFIELD)
	    cc_rerr( "db_type <bit field> illegal - db_type(int) assumed" );
	  
	  if (m & bitoftype_( s_short ))
	    {
	      if (m & bitoftype_( s_unsigned ))
		{
		  dbprintf( buffer, "unsigned short" );
		}
	      else
		{
		  dbprintf( buffer, "short" );
		}

	      return sizeof_short;
	    }
	  else if (m & bitoftype_( s_long ))
	    {
	      if (m & bitoftype_( s_unsigned ))
		{
		  dbprintf( buffer, "unsigned long" );
		}
	      else
		{
		  dbprintf( buffer, "long" );
		}
	      
	      return sizeof_long;
	    }
	  else 
	    {
	      if (m & bitoftype_( s_unsigned ))
		{		  
		  dbprintf( buffer, "unsigned int" );
		}
	      else
		{
		  dbprintf( buffer, "int" );
		}
	  
	      return sizeof_int;
	    }
                    
	case bitoftype_( s_enum ):
#if 0
	  b = typespectagbind_( x );
	  pClassMember = tagbindmems_( b );

	  dbprintf( buffer, "<" );

	  if (*symname_( bindsym_( b )) != '<' ) 
	    dbprintf( buffer, "$%s;", symname_( bindsym_( b ) ) );

	  for (n = 0; pClassMember != NULL; pClassMember = memcdr_( pClassMember ))
	    {
	      dbprintf( buffer, "%s:%d;", symname_( memsv_( pClassMember ) ), n );
	      
	      n++;
	    }
	  
	  dbprintf( buffer, ">int" );
#endif
	  dbprintf( buffer, "enum" );
	  
	  return sizeof_int;
                    

	case bitoftype_( s_double ):
	  if (m & bitoftype_( s_short )) 
	    {
	      dbprintf( buffer, "float" );
	      
	      return sizeof_float;
	    }
	  else
	    {
	      dbprintf( buffer, "double" );
	    }
	  
	  return sizeof_double;
	  
	  
	case bitoftype_( s_struct ):
	  b = typespectagbind_( x );
	  pClassMember = tagbindmems_( b );

	  addstdef( b );

          if (pClassMember == NULL)
	    {
	      dbprintf( buffer, "$%s", symname_( bindsym_( b ) ) );

	      return 0;
	    }

	  dbprintf( buffer, "\n {" );
	  
	  if (*symname_( bindsym_( b ) ) != '<')
	    {
	      dbprintf( buffer, "$%s;\n", symname_( bindsym_( b ) ) );
	    }

	  for (bitoff = n = 0; pClassMember != NULL; pClassMember = memcdr_( pClassMember ))
	    {
	      if (memsv_( pClassMember ) == NULL)
		{
		  dbprintf( buffer, "pad%d:", padn++ );
		}
	      else
		{
		  dbprintf( buffer, "%s:", symname_( memsv_( pClassMember ) ) );
		}
	      
	      if (pClassMember->u.membits)
		{   
		  int32		k = evaluate( pClassMember->u.membits );

		  
		  size = 0;
		  
		  n = padsize( n, alignoftype( te_int ) );
		  
		  if (k == 0)
		    k = 32 - bitoff;  /* rest of int */
		  
		  if (k + bitoff > 32)
		    {
		      size   = sizeof_int;
		      bitoff = 0;		 /* ovflow */
		    }

		  dbprintf( buffer, "%%%d,%d", bitoff, k );
		  
		  bitoff += k;
		}
	      else
		{
		  if (bitoff != 0)
		    {
		      n     += sizeof_int;
		      bitoff = 0;
		    }
		  
		  n = padsize( n, alignoftype( memtype_( pClassMember ) ) );
		  
		  size = db_type( memtype_( pClassMember ), buffer );
		}

	      dbprintf( buffer, ":%d;\n  ", n );
	      
	      n += size;
	    }

	  dbprintf( buffer, "}" );
	  
	  if (bitoff != 0)
	    {
	      n     += sizeof_int;
	      bitoff = 0;
	    }
	  
	  return padsize( n, 4 );

	  
	case bitoftype_( s_union ):

	  b = typespectagbind_( x );
	  pClassMember = tagbindmems_( b );

	  addstdef( b );

          if (pClassMember == NULL)
	    {
	      dbprintf( buffer, "$%s", symname_( bindsym_( b ) ) );
	      
	      return 0;
	    }

	  dbprintf( buffer, "{" );

          if (*symname_( bindsym_( b ) ) != '<')
	    {
	      dbprintf( buffer, "$%s;", symname_( bindsym_( b ) ) );
	    }

	  for (n = 0; pClassMember != NULL; pClassMember = memcdr_( pClassMember ))
	    {
	      dbprintf( buffer, "%s:", symname_( memsv_( pClassMember ) ) );
	      
	      n = max( n, pClassMember->u.membits ? sizeof_int : db_type( memtype_( pClassMember ), buffer ) );

	      dbprintf( buffer, ":0;" );
	    }

	  dbprintf( buffer, "}" );

	  return padsize( n, sizeof_int );
              
	case bitoftype_( s_typedefname ):
	  dbprintf( buffer, "%s", symname_( bindsym_( typespecbind_( x ) ) ) );

	  if (h0_( bindtype_( typespecbind_( x ) ) ) == t_fnap)
	    {
	      /*
	       * XXX - you cannot calculate the size of a function.
	       * Instead we asssume here that the code is actually
	       * using a typedefed function which will then be
	       * indirected to a pointer.  ie we assume that the
	       * code is like this ...
	       *
	       * typedef void FUNC ( int arg ) ;
	       *
	       * void fn( FUNC * fptr ) { return fptr( 1 ); }
	       */
	      
	      return sizeof_ptr;
	    }
	  else
	    {
	      return sizeoftype( bindtype_( typespecbind_( x ) ) );
	    }
	  
	case bitoftype_( s_void ):
	  dbprintf( buffer, "void" );
	  
	  return 0;

	default:
	  break;
	}
      /* drop through for now */

    default:
      syserr( debug_db_type, h0_( x ), typespecmap_( x ) );

    case t_subscript:
      n = sizeoftype( typearg_( x ) );
      
      if (typesubsize_( x ))
	n *= evaluate( typesubsize_( x )) ;
      else
	{
	  typesubsize_( x ) = globalize_int( 1 );

	  cc_warn( "debugger: size of a [] array required, treated as [1]" );
	}

      dbprintf( buffer, "[%d]", evaluate( typesubsize_( x ) ) );
      
      (void) db_type( typearg_( x ), buffer );
      
      return n;
            

    case t_fnap:
      dbprintf( buffer, "()" );

      (void) db_type( typearg_( x ), buffer );
      
      return sizeof_ptr;
	    

    case t_content:
        {
	  TypeExpr *	x1 = typearg_( x );


	  dbprintf( buffer, "*" );
	  
	  if ( (h0_( x1 ) == s_typespec) &&
	      (
	       (typespecmap_( x1 ) & bitoftype_( s_struct )) ||
	       (typespecmap_( x1 ) & bitoftype_( s_union  ))
	       )
	      )
	    {
	      b = typespectagbind_( x1 );
	      
	      dbprintf( buffer, "$%s", symname_( bindsym_( b ) ) );
	    }
	  else
	    {
	      (void) db_type( x1, buffer );
	    }

	  return sizeof_ptr;
	}
    }

} /* db_type */


void
#ifdef __STDC__
db_tidy( void )
#else
db_tidy()
#endif
{
  struct stdef *	s = stlist;


  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_PROCS))
    cc_msg( "debugger: tidy up\n" );

  dbprintf( NULL, "\n" );

  while (s)
    {
      if (!s->done)
	{
	  TagBinder *	b = s->b;
	  TypeExpr *	x = (TypeExpr *) global_list3( SU_Dbg, s_typespec, bitoftype_( s_struct ), b );


	  dbprintf( NULL, "__struct_%s=", symname_( bindsym_( b ) ) );
	  
	  (void) db_type( x, NULL );
	  
	  dbprintf( NULL, ";\n" );
	}

      s = s->next;

    }

  fclose( dbfile );

  dbfile = NULL;
  
  return;
  
} /* db_tidy */


/*
 * Place a string in the output buffer
 *
 * The following code has been stolen from mip/codebuf.c
 */
  
static void
#ifdef __STDC__
dump_name( char * name )
#else
dump_name( name )
  char * name;
#endif
{
  int32 	p;
  union
    {
      char 	c[ 4 ];
      int32 	i;
    } 		w;

    
  for (p = w.i = 0; *name;)
    {
      int32	j;

      
#ifdef REVERSE_OBJECT_BYTE_ORDER
      j = 3 - p;
#else
      j = p;
#endif
      w.c[ j ] = *name++;
	
      ++p;
	
      if (p == 4)
	{
	  outcodeword( w.i, LIT_STRING );
	  
	  p = w.i = 0;
	}
    }

  outcodeword( w.i, LIT_STRING );
  
  return;
  
} /* dump_name */


/*
 * create an entry in the list of filenames
 * associated with the current compilation unit
 *
 * Returns the label associated with the filename
 */

LabelNumber *
#ifdef __STDC__
debugger_filenamelabel( char * filename )
#else
debugger_filenamelabel( filename )
  char * filename;
#endif
{
  FileName *		f = filenamelist;
  char *		s;
  

  while (f != NULL)
    {
      if (streq( filename, f->name ))
	{
	  return f->lab;
	}      

      f = f->next;
    }

  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_LINES ))
    cc_msg( "debugger: new component file %s\n", filename );

  f = DbgAlloc( FileName );

  s = (char *)GlobAlloc( SU_Dbg, pad_to_word( (int32)strlen( filename ) + 1 ) );
  
  f->next = filenamelist;
  f->lab  = nextlabel();
  f->name = s;

  strcpy( s, filename );
  
  filenamelist = f;

  return f->lab;

} /* filenamelable */


/*
 * generate SourceInfo structures
 */

static void
#ifdef __STDC__
genfilenames( void )
#else
genfilenames( )
#endif
{
  FileName *	f = filenamelist;


  while (f != NULL)
    {
      if (!(lab_isset_( f->lab )))
	{
	  LabelNumber *	copy;

	  
	  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_LINES))
	    cc_msg( "debugger: generating SourceInfo struct : %s\n", f->name );

	  setlabel( f->lab );

	  /* force label's address to be offset by codebase as well, since this is a global label */
	     
	  f->lab->u.defn = (codep + codebase) | 0x80000000U;

	  outcodeword( T_SourceInfo, LIT_NUMBER );	/* SourceInfo header	*/
  
	  codexrefs = (CodeXref *) global_list3( SU_Other, codexrefs, X_Debug_Modnum + codebase + codep, NULL );
      
	  outcodewordaux( 0, LIT_NUMBER, NULL );	/* module number 	*/

	  dump_name( f->name );

	  /* we must copy this label, as it will be reused in the next function */
	  
	  copy = DbgAlloc( LabelNumber );

	  *copy = *(f->lab);

	  f->lab = copy;
	}
      
      f = f->next;
    }	

  return;
  
} /* genfilenames */

/*
 * create the structures needed by the Helios Source
 * Level Debugger.
 */

void
#ifdef __STDC__
debugger_end_of_function( void )
#else
debugger_end_of_function( )
#endif
{
  char *	sname = symname_( current_proc );


  if (usrdbg( DBG_PROC ))
    {
      int32	depth = greatest_stackdepth / 4;

      
      if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_LINES))
	cc_msg( "debugger: generating ProcInfo structure: %s\n", sname );

      setlabel( proc_label );
      
      /* create ProcInfo structure */
      
      outcodeword(    T_ProcInfo,	LIT_NUMBER );			/* ProcInfo header	*/
      outcodeword(    codep - 4,	LIT_NUMBER );			/* size of procedure  	*/
      outcodeword(    depth, 		LIT_NUMBER );			/* amount of stack used */
      outcodeword(    0,		LIT_NUMBER );			/* vector stack used 	*/
      
      codexrefs = (CodeXref *) global_list3( SU_Other, codexrefs, X_Debug_Modnum + codebase + codep, NULL );

      outcodewordaux( 0,		LIT_NUMBER, NULL );		/* module number 	*/

      outcodeword(    lastfuncloc - 4,	LIT_NUMBER );			/* offset of function   */
      outcodeword(    0,		LIT_NUMBER );			/* reserved 1 		*/
      outcodeword(    0,		LIT_NUMBER );			/* reserved 2 		*/

      /* no need to generate a Proc structure */
    }
  
  if (usrdbg( DBG_LINE ))
    {
      /* create any missing SourceInfo structures */
  
      genfilenames();
    }
  
  return;
  
} /* debugger_end_of_function */

  
VoidStar
#ifdef __STDC__
dbg_notefileline( FileLine fl )
#else
dbg_notefileline( fl )
  FileLine	fl;
#endif
{

  return NULL;

  use( fl );
  
} /* dbg_notefileline */


void
#ifdef __STDC__
dbg_locvar(
  Binder *	b,
  FileLine	fl,
  bool		narrow )
#else
dbg_locvar( b, fl, narrow )
  Binder *	b;
  FileLine	fl;
  bool		narrow;
#endif
{
  if (usrdbg( DBG_VAR ))
    {
      TypeExpr *	t = bindtype_( b );
      char		buffer[ 1280 ];			/* XXX */
      Dbg_VarList *	var;
      int32		size;
      char		Class;
      
      
      if (dbg_freevars == NULL)
	{
	  var = DbgAlloc( Dbg_VarList );
	}
      else
	{
	  var = dbg_freevars;
	      
	  dbg_freevars = varnext_( dbg_freevars );
	}
      
      buffer[ 0 ] = '\0';
      
      if (bindstg_( b ) & bitofstg_( s_typedef ))
	{
	  char *	name = symname_( bindsym_( b ) );

	  
	  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_VARS))
	    cc_msg( "debugger: new local type $b\n", b );
  
	  if (*name == '<')
	    {
	      /* internal symbol - do not cache */

	      varnext_( var ) = dbg_freevars;
	      
	      dbg_freevars = var;

	      return;
	    }

	  db_type( t, buffer );
	  
	  size  = 0;
	  Class = CLASS_TYPEDEF;
	}
      else
	{
	  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_VARS))
	    cc_msg( "debugger: new %s%s variable $b (type: $t)\n",
		   narrow ? "(narrow) " : "",
		   num_args > 0 ? "formal" : "local",
		   b, bindtype_( b ) );

	  size = pad_to_word( db_type( bindtype_( b ), buffer ) );

	  if (size < 4)
	    size = 4;

	  if (num_args > 0 && !narrow) /* narrow formals are really locals ... */
	    {
	      /* mark variable as being an argument variable */
	  
	      Class = CLASS_ARGUMENT;
	      
	      --num_args;
	    }
	  else
	    {
	      Class = CLASS_UNKNOWN;
	    }
	}
      
      /* prepend variable to list of un-associated vars */
	  
      varnext_(  var )  = dbg_varlist;
      varbind_(  var )  = b;
      vartype_(  var )  = strcpy( (char *)GlobAlloc( SU_Dbg, (int32) strlen( buffer ) + 1 ), buffer );
      varclass_( var )  = Class;
      varoffset_( var ) = 0;
      varsize_(  var )  = size;

      dbg_varlist = var;
    }

  return;

  use( fl );
  
} /* dbg_locvar */


/*
 * locates the information cached for a given binder
 */

static Dbg_VarList *
dbg_lookupvar( Binder * b )
{
  Dbg_VarList *	p;


  /* check local block first */

  if (current_block != NULL)
    {
      Dbg_Block *	parent;


      for (p = current_block->varlist; p; p = p->next)
	{
	  if (p->binder == b)
	    return p;
	}

      /* then check parents */

      for (parent = current_block->parent; parent != NULL; parent = parent->parent)
	{
	  for (p = parent->varlist; p; p = p->next)
	    {
	      if (p->binder == b)
		return p;
	    }
	}
    }
	   
  /* then check un-associated variables */

  for (p = dbg_varlist; p; p = p->next)
    {
      if (p->binder == b)
	return p;
    }
  
  return NULL;

} /* dbg_lookupvar */


void
#ifdef __STDC__
dbg_proc(
	 Symstr *	name,
	 TypeExpr *	type,
	 bool 		ext,
	 FileLine 	fl )
#else
dbg_proc( name, type, ext, fl )
  Symstr *	name;
  TypeExpr *	type;
  bool 		ext;
  FileLine 	fl;
#endif
{
  FormTypeList *	p;
  int32			offset = 4;

  
  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_LINES))
    cc_msg( "debugger: new function: $r\n", name );

  current_proc = name;
  
  dbprintf( NULL, "\n%s:(", symname_( name ) );
  
  p = typefnargs_( type );

  num_args = 0;
  
  while (p)
    {
      int32	size;

	  
      dbprintf( NULL, "%s:", symname_( p->ftname ) );

      size = db_type( p->fttype, NULL );

      size = pad_to_word( size );

      dbprintf( NULL, ":%d;", offset );

      if (size < 4)
	size = 4;

      offset += size;

      p = p->ftcdr;

      num_args++;
    }
  
  dbprintf( NULL, ")" );

  (void) db_type( type, NULL );

  dbprintf( NULL, ":%d=", lastfuncloc );

  lastfuncloc += 4;

  return;

  use( ext );
  use( fl  );
  
} /* dbg_proc */


void
#ifdef __STDC__
dbg_type(
	 Symstr *	name,
	 TypeExpr *	t )
#else
dbg_type( name, t )
  Symstr *	name;
  TypeExpr *	t;
#endif
{
  if (*symname_( name ) != '<')
    {
      dbprintf( NULL, "%s=", symname_( name ) );

      (void) db_type( t, NULL );
      
      dbprintf( NULL, ";\n" );
    }
  
  return;
  
} /* dbg_type */


void
#ifdef __STDC__
dbg_topvar(
	   Symstr *	name,
	   int32 	addr,
	   TypeExpr *	t,
	   bool		stgclass,
	   FileLine 	fl )
#else
dbg_topvar( name, addr, t, stgclass, fl )
  Symstr *	name;
  int32 	addr;
  TypeExpr *	t;
  int 		stgclass;
  FileLine 	fl )
#endif
{
  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_VARS))
    cc_msg( "debugger: top level variable $r\n", name );

  if (dataloc != lastdataloc)
    {
      dbprintf( NULL, "%s:", symname_( name ) );
      
      (void) db_type( t, NULL );
      
      dbprintf( NULL, ":%d:%c;\n", lastdataloc, stgclass ? CLASS_GLOBAL : CLASS_STATIC );
      
      lastdataloc = dataloc;
    }

  return;

  use( addr );
  use( fl   );  
  
} /* dbg_topvar */

  
void
dbg_init( void )
{
  return;

} /* dbg_init */


static void
add_var( Dbg_VarList * var )
{
  if (current_block == NULL || var == NULL)
    {
      syserr( debug_no_block );
    }

  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_VARS))
    cc_msg( "debugger: adding variable $b to block %d\n",
	   varbind_( var ), current_block->block_id );

  /* remove variable from un-associated variable list */
  
  if (dbg_varlist == var)
    {
      dbg_varlist = varnext_( var );
    }
  else
    {
      Dbg_VarList *	v;


      for (v = dbg_varlist; v; v = varnext_( v ))
	{
	  if (varnext_( v ) == var)
	    break;
	}

      if (v == NULL)
	{
	  syserr( debug_already_associated );
	}
      
      varnext_( v ) = varnext_( var );
    }

  /* add the variable to the current block's list */
  
  varnext_( var ) = current_block->varlist;

  current_block->varlist = var;
  
  return;

} /* add_var */


/*
 * adds an active line to the current block
 */

void
debugger_add_line( int32 linenumber )
{
  Dbg_LineList *	l;

  
  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_LINES))
    cc_msg( "debugger: adding line %ld to block %d\n", linenumber, current_block->block_id );
  
  if (current_block == NULL)
    {
      /*
       * This can happen when some, but not all
       * of a functions arguments are promoted to
       * register variables - it is not important
       * as the line we are trying to add is the closing
       * curly parenthesis of the function
       */
      
      if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_LINES))
	cc_msg( "debugger: no block into which to add line\n" );
      
      return;
    }

  l = current_block->linelist;

  if (l != NULL && linenumber == l->last + 1)
    {
      l->last = linenumber;
    }
  else
    {
      Dbg_LineList *	pNew;

      
      if (dbg_freelines == NULL)
	{
	  pNew = DbgAlloc( Dbg_LineList );
	}
      else
	{
	  pNew = dbg_freelines;

	  dbg_freelines = dbg_freelines->next;
	}
      
      pNew->next = l;
      pNew->last = pNew->first = linenumber;

      current_block->linelist = pNew;
    }

  return;
  
} /* debugger_add_line */

  
static void
enter_block( void )
{
  Dbg_Block *	block;


  if (dbg_freeblocks == NULL)
    {
      block = DbgAlloc( Dbg_Block );
    }
  else
    {
      block = dbg_freeblocks;

      dbg_freeblocks = dbg_freeblocks->next;
    }

  block->next     = blocklist;
  block->parent   = current_block;
  block->linelist = NULL;
  block->varlist  = NULL;
  block->block_id = ++current_block_id;
  
  current_block = block;
  blocklist     = block;

  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & (DEBUG_LINES | DEBUG_VARS)))
    cc_msg( "debugger: new block %d\n", block->block_id );

  return;
  
} /* enter_block */


static void
leave_block( void )
{
  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & (DEBUG_LINES | DEBUG_VARS)))
    cc_msg( "debugger: finished block %d\n", current_block->block_id );
  
  if (current_block == NULL)
    syserr( debug_no_block );

  current_block = current_block->parent;

  return;
    
} /* leave_block */
  

static void
renew_block( Binder * b )
{
  Dbg_Block *	block;


  for (block = blocklist; block; block = block->next)
    {
      Dbg_VarList *	v;


      for (v = block->varlist; v; v = v->next)
	{
	  if (v->binder == b)
	    break;
	}

      if (v)
	{
	  current_block = block;
	  return;	  
	}      
    }
  
  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_VARS))
    cc_msg( "debugger: could not renew block for binder $b\n", b );

  return;
  
} /* renew_block */


void
dbg_enterproc( void )
{
  if (usrdbg( DBG_ANY ))
    {
      enter_block();
    }
  
  return;

} /* dbg_enterproc */



/* dbg_locvar1 is called when the location for a declaration is known. By this
   time, things allocated in local storage have evaporated (in particular,
   bindtype_(b). And the Symstrs for most gensyms.
 */

void
dbg_locvar1( Binder * b )
{
#if 0
  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_VARS))
    cc_msg( "debugger: locvar1 (%s)", symname_( bindsym_( b ) ) );
#endif
  
  return;

  use( b );
  
} /* dbg_locavar1 */


/*
 * we now have more information about a variable - update its record
 */

static void
update_var(
	   Binder *		b,
	   Dbg_VarList *	var )
{
  int32		addr = bindaddr_( b );
  

#ifdef DEBUG
  if (b == NULL || var == NULL)
    syserr( syserr_null_parameter );
#endif
  
  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_VARS))
    cc_msg( "debugger: updating $b\n", b );

  switch (bindstg_( b ) & PRINCSTGBITS)
    {
    default:
      syserr( debug_unknown_storage_type );
      break;
      
    case bitofstg_( s_extern  ):
      varclass_(  var ) = CLASS_GLOBAL;
      varoffset_( var ) = lastdataloc;
      lastdataloc      += varsize_( var );
      
      break;
      
    case bitofstg_( s_static ):
      varclass_(  var ) = CLASS_STATIC;
      varoffset_( var ) = lastdataloc;
      lastdataloc      += varsize_( var );
      
      break;
      
    case bitofstg_( s_typedef ):
      break;                   /* typedefs, statics and externs already done in locvar */

    case bitofstg_( s_auto ):
      switch (addr & BINDADDR_MASK)
	{
	case BINDADDR_ARG:
	case BINDADDR_LOC:
	  if (bindxx_( b ) != GAP)
	    {
	      varclass_(  var ) = CLASS_REGISTER;
	      varoffset_( var ) = saved_regs_offsets[ register_number( bindxx_( b ) ) ];
	    }
	  else
	    {
	      varclass_(  var ) = CLASS_LOCAL;
	      varoffset_( var ) = (bindaddr_( b ) & ~BINDADDR_MASK);
	    }
	  
	  break;

	default:
	  syserr( debug_unknown_auto_storage );
	  break;
        }
      break;      
    }
  
  return;
  
} /* update_var */

  
bool
dbg_scope(
	  BindListList * 	pNew,
	  BindListList * 	pOld  )
{
  int32 oldlevel = length( (List *)pOld );
  int32 newlevel = length( (List *)pNew );
  int32 entering = newlevel - oldlevel;
  int32	nvars;
  
  
  if (!usrdbg( DBG_VAR | DBG_LINE ))
    return NO;

  if (pNew == pOld)
    return NO;

  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_LINES))
    cc_msg( "debugger: %s scope\n", entering > 0 ? "entering new" : "leaving current" );

  if (entering < 0)
    {
      BindListList *	t = pNew;

      pNew = pOld;
      pOld = t;
    }

  nvars = 0;
  
  while (pNew != pOld)
    {
      SynBindList *	bl;

      
      if (pNew == NULL)
	syserr( debug_bad_scope );

      for (bl = pNew->bllcar; bl; bl = bl->bindlistcdr)
        {
	  Binder *	b = bl->bindlistcar;
	  Dbg_VarList *	var;
	  

	  if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_VARS))
	    cc_msg( "debugger: %sbinding $b\n", entering < 0 ? "un" : "", b ); 

	  if ((bindstg_( b ) & b_dbgbit))
	    {
	      if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_VARS))
		cc_msg( "debugger: renewing block\n" ); 

	      renew_block( b );
	    }
	  else
	    {
	      var = dbg_lookupvar( b );

	      if (var)
		{
		  if (entering < 0)
		    {
		      bindstg_( b ) |= b_dbgbit;

		      update_var( b, var );
		    }
		  else
		    {
		      if (nvars == 0 && varclass_( var ) != CLASS_ARGUMENT)
			{
			  enter_block();
			}
		      
		      add_var( var );
		    }

		  if (varclass_( var ) != CLASS_ARGUMENT)
		    {
		      nvars++;
		    }
		}
	      else
		{
		  cc_err( "debugger: UNKNOWN variable $b at line %d\n", b, curlex.fl.l ); 
		}	      
	    }
        }

      pNew = pNew->bllcdr;
    }

  if (entering < 0 && nvars > 0)
    {
      leave_block();
    }

  return NO;

} /* dbg_scope */


static void
print_lines( Dbg_LineList * l )
{
  int32		i;


#ifdef DEBUG
  if (l == NULL)
    syserr( debug_null_parameter );
#endif
  
  if (l->next != NULL)
    {
      print_lines( l->next );
  
      for (i = l->first; i <= l->last; i++)
	{
	  dbprintf( NULL, ",%d", i );
	}
    }
  else
    {
      for (i = l->first; i < l->last; i++)
	{
	  dbprintf( NULL, "%d,", i );
	}
      
      dbprintf( NULL, "%d", i );
    }

  /* prepend the line to the free lines list */
  
  l->next = dbg_freelines;
  
  dbg_freelines = l;
  
  return;

} /* print_lines */


static void
print_variables( Dbg_VarList * var )
{
  char *	name;

  
  if (var == NULL)
    return;

  /* recurse down list */
  
  print_variables( varnext_( var ) );

  /* display this variable */
  
  name = symname_( bindsym_( varbind_( var ) ) );

  switch (varclass_( var ))
    {
    case CLASS_TYPEDEF:
      dbprintf( NULL, "\n%*c %s=%s;", indent, ' ', name, vartype_( var ) );
      break;

    case CLASS_UNKNOWN:
      syserr( debug_unresolved_var_class, name );
      break;

    case CLASS_ARGUMENT:
      dbprintf( NULL, "\n%*c %s:%s:%d:r;", indent, ' ', name, vartype_( var ), varoffset_( var ) );
      break;
      
    default:
      dbprintf( NULL, "\n%*c %s:%s:%d:%c;", indent, ' ', name,
	       vartype_( var ), varoffset_( var ), varclass_( var ) );
      break;      
    }
	      
  /* XXX free 'var->type' */

  /* return the variable to the free list */

  varnext_( var ) = dbg_freevars;

  dbg_freevars = var;
  
  return;
  
} /* print_variables */


static void
dump_blocks( Dbg_Block * parent )
{ 
  Dbg_Block *	p;


  for (p = blocklist; p != NULL; p = p->next)
    {    
      /* display a block with the given parent */
      
      if (p->parent == parent)
	{
	  indent++;
	  
	  dbprintf( NULL, "\n%*c{", indent, ' ' );

	  /* display the lines occupied by this block */
	  
	  if (p->linelist)
	    {
	      print_lines( p->linelist );

	      dbprintf( NULL, ";" );
	    }
	  
	  /* display the variables created in this block */

	  print_variables( p->varlist );
	  
	  /* display children of this block */
	  
	  dump_blocks( p );

	  /* close block */
	  
	  dbprintf( NULL, "\n%*c}", indent, ' ' );
	  
	  indent--;
	}
    }

  return;
  
} /* dump_blocks */


void
dbg_xendproc( FileLine fl )
{
  if (usrdbg( DBG_ANY ))
    {
      Dbg_Block *	p;

  
      if (debugging( DEBUG_Q ) && (DEBUG_LEVEL & DEBUG_PROCS))
	cc_msg( "debugger: end of function\n" );
  
      if (blocklist == NULL)
	syserr( debug_no_blocks );
      
      dump_blocks( NULL );

      /* prepend blocks to free block list */
  
      for (p = blocklist; p->next != NULL; p = p->next)
	;
      
      p->next = dbg_freeblocks;
      
      dbg_freeblocks = blocklist;
      
      /* empty list pointers */
      
      current_block = blocklist = NULL;
      
      dbprintf( NULL, "\n" );
    }

  return;

  use( fl );
  
} /* dbg_xendproc */

#endif /* TARGET_HAS_DEBUGGER */
