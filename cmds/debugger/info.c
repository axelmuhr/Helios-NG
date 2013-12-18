/**
*
* Title:  Helios Debugger - Debug Info File Loading
*
* Author: Andy England
*
* Date:   November 1988
*
*         (c) Copyright 1988 - 1993, Perihelion Software Limited.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/info.c,v 1.8 1993/07/06 14:10:42 nickc Exp $";
#endif

#include "tla.h"

PRIVATE void readentry(FILE *, TABLE, BLOCK *, TYPE *, CLASS);
PRIVATE TYPE *readtype(FILE *, TABLE, BLOCK *);
PRIVATE CLASS readclass(FILE *);
PRIVATE BLOCK *readblock( FILE *, TABLE, BLOCK *, TYPE * );
#ifdef NEWCODE
PRIVATE int readtypeid(FILE *);
#endif
PRIVATE char *readname(FILE *, char *);
PRIVATE int readnumber(FILE *);
PRIVATE int getvc(FILE *file);

/**
*
* ACE: The use of module and source needs to be tidied up.
*
**/

/**
*
* block = loadinfo(debug, module);
*
* Load the debug file.
*
**/
PUBLIC BLOCK *loadinfo(DEBUG *debug, MODULE *module)
{
  BLOCK *block;
  char name[256];
  char *p;
  FILE *file;

#ifdef OLDCODE
  strcpy(name, debug->env.Objv[0]->Name);
#endif
/*
-- crf : 12/08/91 - clean up use of Environment Objv
*/
  strcpy(name, debug->env.Objv[OV_Cdir]->Name);

  strcat(name, "/");
  strcat(name, module->name);
  if ((p = strrchr(name, '.')) == NULL OR strchr(p, '/') != NULL)
    p = name + strlen(name);
  strcpy(p, ".dbg");

#ifdef OLDCODE
  if ((file = my_fopen(name , "rb")) == NULL) return NULL;
#else
/*
-- crf : 26/09/91 - Bug 677
-- Problem : if client is run from a directory other than that where the 
-- source files are located, an empty debug window is created. 
-- Solution : if the source and .dbg files are not present in the current
-- directory, the debugger looks for the directory defined by the environment
-- variable DBGSRC_VARNAME (#defined in tla.h).
-- Notes : I have changed the following sources :
--   1. loadsource() (source.c) - loads the program sources
--   2. loadinfo() (info.c) - loads the .dbg file
*/

  if ((file = my_fopen(name , "rb")) == NULL) 
  {
    char *dbgsrc;
    char new_name [512] ;

    debugf ("testing for environment variable %s", DBGSRC_VARNAME) ;
    if ((dbgsrc = getvar(debug->env.Envv, DBGSRC_VARNAME)) == NULL) 
      return NULL;

    strcpy (new_name, dbgsrc) ;
    strcat (new_name, name + strlen (debug->env.Objv[OV_Cdir]->Name)) ;

    debugf ("trying to open .dbg file : %s", new_name) ;
    if ((file = my_fopen(new_name , "rb")) == NULL) return NULL;
  }
#endif

#ifdef NEWCODE
  bigbuf(file);
#endif
  block = newblock(NULL);
  /* ACE: A bit of a cluge */
  block->module = module;
  until (feof(file)) /* ACE: ??? */
    readentry(file, debug->table, block, NULL, C_Extern);
  fclose(file);
  return block;
}

  
/**
*
* readentry(file, table, block, host, class);
*
* Read in an debug file entry entry.
*
**/
PRIVATE void
readentry(
	  FILE *	file,
	  TABLE		table,
	  BLOCK *	block,
	  TYPE *	host,
	  CLASS		Class )
{
  TYPE *		type;
  char			name[ NAME_MAX + 1 ];
  int			offset = 0;
  int			c;

  
  (void)readname( file, name );

  switch (c = getvc( file ))
    {
      BLOCK *		func_block;

      
    case '=':
      type = readtype( file, table, block );
      
#ifdef NEWCODE
      if ((c = getvc( file )) == '=')
	{
	  (void)declarevar( table, block, name, C_Typedef, type, readtypeid( file ) );
	}
      else
	{
	  ungetc( c, file );
#endif
	  (void)declaretype( table, block, name, type );
#ifdef NEWCODE
	}
#endif
      break;

    case ':':
      type = readtype( file, table, block );

      if ((c = getvc( file )) == ':')
	offset = readnumber( file );
      else
	ungetc( c, file );
      
      if ((c = getvc( file )) == ':')
	Class = readclass( file );
      else
	ungetc( c, file );
      
      if ((c = getvc( file )) == '=')
	{
	  func_block = readblock( file, table, block, type );
	}      
      else
	{
	  ungetc( c, file );
	  
	  func_block = NULL;
	}
      
      switch (Class)
	{
	  ENTRY *	entry;

	  
	case C_Auto:
	  (void)declarelocal(table, block, name, type, offset);
	  break;

	case C_Enum:
	  (void)declareenum(table, host, name, type, offset);
	  break;

	case C_Member:
	  (void)declaremember(table, host, name, type, offset);
	  break;

	case C_Param:
	  (void)declareparam(table, host, name, type, offset);
	  break;

	case C_Register:      
	  (void)declarevar(table, block, name, Class, type, offset );
	  break;
	  
	default:
	  entry = declarevar(table, block, name, Class, type, offset );

	  if (func_block)
	    {
	      /* attach the funciton's entry in the symbol table to the outermost block it contains */
	      
	      func_block->entry = entry;
	    }
	  
	  break;
	}
      break;

    default:
      IOdebug( "TLA: error in entry %s (char '%c' (%d))", name, c, c );
      debugf( "error in entry %s", name );
      return;
    }
  
  while ((c = getvc(file)) == ';')
    ;
  
  ungetc( c, file );

  return;
  
} /* readentry */

/**
*
* type = readtype(file, table, block);
*
* Read a type expression.
*
**/
PRIVATE TYPE *readtype(FILE *file, TABLE table, BLOCK *block)
{
  TYPE *type;
  ENTRY *entry;
  char name[NAME_MAX + 1];
  int size, first;
  int c;

  switch (c = getvc(file))
  {
    case '[':
    size = readnumber(file);
    if ((c = getvc(file)) == ',')
    {
      first = readnumber(file);
      c = getvc(file);
    }
    else first = 0;
    unless (c == ']')
    {
      IOdebug( "TLA: error in array type" );
      debugf( "error in array type" );
    }
    return newarray(readtype(file, table, block), size, first);

    case '{':
    type = newstruct();
    if ((c = getvc(file)) == '$')
    {
      /* ACE:
         This needs improving. We currently search for the tag using scope
         rules. Should only search in current block. This raises the issue
         of whether a tag declared after it is referenced, in a block enclosing
         the one in which it is referenced is actually the one referenced.
      */
      if ((entry = findtag(table, readname(file, name))) == NULL)
        entry = declaretag(table, block, name, type);
      else entry->type = type;
      /* ACE: Not sure if this is the best place to do this */
      type->structure.tag = entry;
      c = getvc(file); /* read ';' */
    }
    else ungetc(c, file);
    until ((c = getvc(file)) == '}')
    {
      ungetc(c, file);
      readentry(file, table, block, type, C_Member);
    }
    return type;

    case '(':
    type = newfunction(NULL);
    until ((c = getvc(file)) == ')')
    {
      ungetc(c, file);
      readentry(file, table, block, type, C_Param);
    }
    break;

    case '<':
    type = newenumeration();
    until ((c = getvc(file)) == '>')
    {
      ungetc(c, file);
      readentry(file, table, block, type, C_Enum);
    }
    return type;

    case '*':
    return newpointer(readtype(file, table, block));

    case '$':
    if ((entry = findtag(table, readname(file, name))) == NULL)
      entry = declaretag(table, block, name, NULL);
    return newtag(entry);

    case '#':
    size = readnumber(file);
    return newintegraltype(size, FALSE);

    case '-':
    size = readnumber(file);
    return newintegraltype(size, TRUE);

    case '.':
    size = readnumber(file);
    return newfloatingtype(size);

    case '%': /* ACE: bit fields, cludged at moment */
    size = readnumber(file);
    c = getvc(file);
    size = readnumber(file);
    return newintegraltype(size >> 3, FALSE);

    case ';':
    IOdebug( "TLA: error in type");
    debugf("error in type");
    return NULL;

#ifdef NEWCODE
    case '!':
    return newtypedef(findtypeid(table, block, readtypeid(file)));
#endif

    default:
    ungetc(c, file);
    if ((entry = findtype(table, readname(file, name))) == NULL)
    {
#ifdef NEWCODE
      int typeid = 0;
      char *s = name;
      int c;

      until ((c = *s++) == NULL) typeid = ((typeid << 8) | c);
      if ((entry = findtypeid(table, block, typeid, name)) == NULL)
      {
#endif
        IOdebug( "TLA: Undefined type %s", name);
        debugf("Undefined type %s", name);
        return NULL;
#ifdef NEWCODE
      }
#endif
    }
    return newtypedef(entry);
  }
  type->generic.host = readtype(file, table, block);
  return type;
}

/**
*
* class = readclass(file);
*
* Read a class.
*
**/
PRIVATE CLASS readclass(FILE *file)
{
  switch (getvc(file))
  {
    case 'c':
    return C_Common;

    case 'd':
    return C_Display;

    case 'e':
    return C_Enum;

    case 'g':
    return C_Extern;

    case 'l':
    return C_Auto;

    case 'm':
    return C_Member;

    case 'p':
    return C_Param;

    case 'r':
    return C_Register;

    case 's':
    return C_Static;

    case 't':
    return C_Typedef;

    default:
    /* ACE: error */
    return C_Extern;
  }
}

/**
*
* block = readblock(file, table, parent, type );
*
* Read a block.
*
**/
PRIVATE BLOCK *
readblock(
	  FILE *	file,
	  TABLE		table,
	  BLOCK *	parent,
	  TYPE *	type )
{
  BLOCK *		block = newblock( parent );
  int			c     = getvc( file );

  
  if (type != NULL)
    {
      add_params( table, block, type );
    }

  c = getvc( file );
  
  if (isdigit(c))
    {
      ungetc( c, file );
      
      do
	addline( block, readnumber( file ) );
      while ((c = getvc( file )) == ',');
    }
  else
    ungetc( c, file );
  
  until ((c = getvc( file )) == '}')
    {
      ungetc( c, file );

      if (c == '{')
	readblock( file, table, block, NULL );
      else
	readentry( file, table, block, NULL, C_Auto );
    }

  while ((c = getvc( file )) == ';')
    ; /* ACE: I would like to take this out */
  
  ungetc( c, file );

  return block;
}

#ifdef NEWCODE
/**
*
* typeid = readtypeid(file);
*
* Read a type id.
*
**/
PRIVATE int readtypeid(FILE *file)
{
  int typeid = 0;
  int c;

  until ((c = getc(file)) == ';' OR c == ':' OR c == '=')
    typeid = ((typeid << 8) | c);
  ungetc(c, file);
  return typeid;
}
#endif

/**
*
* name = readname(file, buffer);
*
* Read a name into a buffer.
*
**/
PRIVATE char *readname(FILE *file, char *buffer)
{
  int index = 0;
  int c;

  until ((c = getc(file)) == ';' OR c == ':' OR c == '=') buffer[index++] = c;
  ungetc(c, file);
  buffer[index] = '\0';
  return buffer;
}

/**
*
* number = readnumber(file);
*
* Read a number.
*
**/
PRIVATE int readnumber(FILE *file)
{
  int number = 0;
  int negative = FALSE;
  int c;

  if ((c = getvc(file)) == '-')
  {
    negative = TRUE;
    c = getvc(file);
  }
  while (isdigit(c))
  {
    number = (number * 10) + (c - '0');
    c = getvc(file);
  }
  ungetc(c, file);
  return negative ? -number : number;
}

/**
*
* c = getvc(file);
*
* Get a visible (non-space) character.
*
**/
PRIVATE int getvc(FILE *file)
{
  int c;

  while (isspace(c = getc(file)));
  return c;
}
