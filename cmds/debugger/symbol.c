/**
*
* Title:  Helios Debugger - Symbol Support
*
* Author: Andy England
*
* Date:   May 1989
*
*         (c) Copyright 1989 - 1993, Perihelion Software Limited.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/symbol.c,v 1.6 1993/07/06 14:09:53 nickc Exp $";
#endif

#include "tla.h"

PRIVATE void putdecl(FILE *, TYPE *, TYPE *, char *);
PRIVATE void putparam(ENTRY *, FILE *);
PRIVATE BOOL selecttype(ENTRY *);
PRIVATE BOOL selecttag(ENTRY *);
PRIVATE BOOL isenclosed(BLOCK *, BLOCK *);

PUBLIC void myputtype(TYPE *, FILE *);

/**
*
* addentry(symbol, entry);
*
* Add an entry onto a symbol.
*
**/
PRIVATE void addentry(SYMBOL *symbol, ENTRY *entry)
{
  ENTRY *prev, *next;

  switch (entry->Class)
  {
    case C_Tag:
    case C_Typedef:
    addlink(&symbol->entrylist, &entry->link);
    break;

    case C_Extern:
    addtail(&symbol->entrylist, &entry->link);
    break;

    default:
    for (prev = (ENTRY *)&symbol->entrylist;
         (next = (ENTRY *)prev->link.next) != NULL; prev = next)
    {
      if (isenclosed(entry->block, next->block)) break;
    }
    addlink((CHAIN *)prev, &entry->link);
    break;
  }
}


/* add parameters to variable list */

PUBLIC void
add_params(
	   TABLE	table,
	   BLOCK *	block,
	   TYPE *	type )
{
  PARAM *		param;

    
  for (param = (PARAM *)skiptypedef( type )->function.paramlist.head;
       param != NULL; param = (PARAM *)param->link.next)
    {
      declarevar( table, block, param->name, C_Param, reusetype( param->type ), param->offset );
    }

  return;
  
} /* add_params */


/**
*
* entry = declarevar(table, block, name, class, type, offset );
*
* Declare a variable.
*
**/
PUBLIC ENTRY *
declarevar(
	   TABLE	table,
	   BLOCK *	block,
	   char *	name,
	   CLASS	Class,
	   TYPE *	type,
	   int		offset )
{
  SYMBOL *		symbol = addsymbol(table, name);
  ENTRY *		entry  = NEW(ENTRY);

  
  /* IOdebug( "variable: name = %s, class = %d, offset = %d, block = %x",
	  name, Class, offset, block ); */
  
  entry->name   = symbol->name;
  entry->Class  = Class;
  entry->type   = type;
  entry->offset = offset;
  entry->block  = block;
  
  addentry(symbol, entry);
  return entry;
}

/**
*
* entry = declarelocal(table, block, name, type, offset);
*
* Declare a local variable.
*
**/
PUBLIC ENTRY *
declarelocal(
	     TABLE 	table,
	     BLOCK *	block,
	     char *	name,
	     TYPE *	type,
	     int	offset )
{
  SYMBOL *		symbol = addsymbol(table, name);
  LOCAL *		local  = NEW(LOCAL);

  
  /* IOdebug( "local variable: name = %s, offset = %d, block = %x", name, offset, block ); */
  
  local->name   = symbol->name;
  local->Class  = C_Auto;
  local->type   = type;
  local->offset = offset;
  local->block  = block;
  
  addentry(symbol, (ENTRY *)local);
  
  /* ACE: Local functions ! */
  
  return (ENTRY *)local;
}

/**
*
* entry = declaretype(table, block, name, type);
*
* Declare a type.
*
**/
PUBLIC ENTRY *declaretype(TABLE table, BLOCK *block, char *name, TYPE *type)
{
  SYMBOL *symbol = addsymbol(table, name);
  TYPEDEF *entry;

  debugf( "declare type, name => %s", name );
  
  if ((entry = (TYPEDEF *)searchchain(&symbol->entrylist, selecttype, 0)) == NULL)
  {
    entry        = NEW(TYPEDEF);
    entry->name  = symbol->name;
    entry->Class = C_Typedef;
    entry->type  = type;
    entry->block = block;
    
    addentry(symbol, (ENTRY *)entry);
  }
  else freetype(type);
  return (ENTRY *)entry;
}

/**
*
* entry = declaretag(table, block, name, type);
*
* Declare a structure tag.
*
**/
PUBLIC ENTRY *declaretag(TABLE table, BLOCK *block, char *name, TYPE *type)
{
  SYMBOL *symbol = addsymbol(table, name);
  TYPEDEF *entry;

  
  debugf( "declare tag, name => %s", name );
  
  if ((entry = (TYPEDEF *)searchchain(&symbol->entrylist, selecttag, 0)) == NULL)
  {
    entry = NEW(TYPEDEF);
    
    entry->name  = symbol->name;
    entry->Class = C_Tag;
    entry->type  = type;
    entry->block = block;

    addentry(symbol, (ENTRY *)entry);
  }
  else freetype(type);
  return (ENTRY *)entry;
}

/**
*
* entry = declaremember(table, host, name, type, offset);
*
* Declare a structure member.
*
**/
PUBLIC ENTRY *declaremember(TABLE table, TYPE *host, char *name, TYPE *type, int offset)
{
  SYMBOL *symbol = addsymbol(table, name);
  MEMBER *member = NEW(MEMBER);

  
  debugf( "declare member, name => %s", name );
  
  member->name   = symbol->name;
  member->Class  = C_Member;
  member->type   = type;
  member->offset = offset;
  
  addtail(&host->structure.memberlist, &member->link);
  
  return (ENTRY *)member;
}

/**
*
* entry = declareparam(table, host, name, type, offset);
*
* Declare a function parameter.
*
**/
PUBLIC ENTRY *declareparam(TABLE table, TYPE *host, char *name, TYPE *type, int offset)
{
  SYMBOL * symbol = addsymbol(table, name);
  PARAM *  param  = NEW(PARAM);


  param->name   = symbol->name;
  param->Class  = C_Param;
  param->type   = type;
  param->offset = offset;
  
  addtail(&host->function.paramlist, &param->link);
  
  return (ENTRY *)param;
}

/**
*
* entry = declareenum(table, host, name, type, offset);
*
* Declare a enumeration constant.
*
**/
PUBLIC ENTRY *declareenum(TABLE table, TYPE *host, char *name, TYPE *type, int offset)
{
  SYMBOL *    symbol    = addsymbol(table, name);
  ENUMCONST * enumconst = NEW(ENUMCONST);

  
  debugf( "declare enum, name => %s", name );
  
  enumconst->name   = symbol->name;
  enumconst->Class  = C_Enum;
  enumconst->type   = type;
  enumconst->offset = offset;
  
  addlink(&host->enumeration.constlist, &enumconst->link);
  
  return (ENTRY *)enumconst;
}

/**
*
* freeentry(entry);
*
* Free a symbol table entry.
*
**/
PUBLIC void freeentry(ENTRY *entry)
{
  /*
     ACE: I am unsure about this test. It prevents recursion on things like:
          Node={$Node;Next:Node*:0;...
          I think things are more serious than this. Any type can be referenced
          more than once, for example the basetypes.
          Should really keep usage count on types ?.
          Or use duptype() all the time.
          No, the answer for type expressions is that they are freed up as far
          as any typedef.
  */
  unless (entry->Class == C_Tag) freetype(entry->type);
  freemem(entry);
}

/**
*
* freesymbol(symbol);
*
* Free a symbol table entry.
*
**/
PUBLIC void freesymbol(SYMBOL *symbol)
{
  walkchain(&symbol->entrylist, freeentry, 0);
  freemem(symbol);
}

/**
*
* entry = findentry(table, selector, block, name);
*
* Find an entry.
*
**/
#ifdef NEWCODE
PRIVATE ENTRY *findentry(TABLE table, BOOL (*selector)(), char *name, long arg)
{
  SYMBOL *symbol;

  if ((symbol = findsymbol(table, name)) == NULL) return NULL;
   return (ENTRY *)searchchain(&symbol->entrylist, selector, arg);
}
#endif

/**
*
* entry = findvar(table, block, name);
*
* Find a variable.
*
**/
PRIVATE BOOL selectvar(ENTRY *entry, BLOCK *block)
{
  /* debugf( "check entry %s, class %d, block %x", entry->name, entry->class, entry->block ); */
  
  return  entry->Class == C_Extern OR
          (entry->Class != C_Typedef AND entry->Class != C_Tag AND
          isenclosed(block, entry->block));
}

PUBLIC ENTRY *findvar(TABLE table, BLOCK *block, char *name)
{
  SYMBOL *symbol;

  
  if ((symbol = findsymbol(table, name)) == NULL) return NULL;

  return (ENTRY *)searchchain(&symbol->entrylist, selectvar, (word)block);
}

/**
*
* entry = findtype(table, name);
*
* Find a type.
*
**/
PRIVATE BOOL selecttype(ENTRY *entry)
{
  return entry->Class == C_Typedef;
}

PUBLIC ENTRY *findtype(TABLE table, char *name)
{
  SYMBOL *symbol;

  if ((symbol = findsymbol(table, name)) == NULL)
    return NULL;
  
  return (ENTRY *)searchchain(&symbol->entrylist, selecttype, 0);
}

#ifdef NEWCODE
/**
*
* entry = findtypeid(table, block, typeid, name);
*
* Find a type by id.
*
**/
PRIVATE BOOL selecttypeid(ENTRY *entry, int typeid)
{
  return entry->Class == C_Typedef AND entry->offset == typeid;
}
/* CR: dont know wether *name is the right one */
PUBLIC ENTRY *findtypeid(TABLE table, BLOCK *block, int typeid, char *name)
{
  if (block == NULL)
    return NULL;
  
  return findentry( block, *selecttypeid(block->entry, typeid), name, typeid);/*table***/
}
#endif

/**
*
* entry = findtag(table, name);
*
* Find a structure tag.
*
**/
PRIVATE BOOL selecttag(ENTRY *entry)
{
  return entry->Class == C_Tag;
}

PUBLIC ENTRY *findtag(TABLE table, char *name)
{
  SYMBOL *symbol;

  if ((symbol = findsymbol(table, name)) == NULL)
    return NULL;
  return (ENTRY *)searchchain(&symbol->entrylist, selecttag, 0);
}

/**
*
* entry = findmember(type, name);
*
* Find a structure member.
*
**/
PRIVATE BOOL cmpmember(ENTRY *member, char *name)
{
  return strequ(member->name, name);
}

PUBLIC ENTRY *findmember(TYPE *type, char *name)
{
  return (ENTRY *)searchchain(&type->structure.memberlist, cmpmember, (word)name);
}

/**
*
* entry = findenumconst(type, value);
*
* Find a enumeration constant.
*
**/
PRIVATE BOOL cmpenumconst(ENTRY *enumconst, int value)
{
  return enumconst->offset == value;
}

PUBLIC ENTRY *findenumconst(TYPE *type, int value)
{
  return (ENTRY *)searchchain(&type->enumeration.constlist, cmpenumconst, value);
}

#ifdef NEWCODE
PRIVATE BOOL chkentry(ENTRY *entry, void *addr)
{
  void *entryaddr;

  if (entry->Class == C_Typedef OR entry->Class == C_Tag) return FALSE;
  lvalue(eval, entry);
  entryaddr = ppop(eval);
  return entryaddr <= addr AND entryaddr + sizeoftype(entry->type) > addr;
}

PUBLIC ENTRY *whichentry(BLOCK *block, void *addr, char *name)
{
  if (block == NULL) return NULL;
  
  return findentry( block, chkentry(block->entry, addr), name, addr);/**name ? +++ ***/
}

PRIVATE BOOL chkmember(ENTRY *member, int offset)
{
  int memberoffset;

  lvalue(member);
  memberoffset = popint();
  return memberoffset <= offset AND
         memberoffset + sizeoftype(member->type) > offset;
}

PUBLIC ENTRY *whichmember(TYPE *type, int offset)
{
  return (ENTRY *)searchchain(&type->structure.memberlist, chkmember, offset);
}
#endif

PUBLIC void addline(BLOCK *block, int line)
{
  if (block == NULL)
    return;
  
  if (block->lines % 10 == 0)
  {
    LINENO *linevec = (LINENO *)newmem((block->lines + 10) * sizeof(LINENO));

    unless (block->linevec == NULL)
    {
      memmove(linevec, block->linevec, block->lines * sizeof(LINENO));
      freemem(block->linevec);
    }
    block->linevec = linevec;
  }
  block->linevec[block->lines++] = line;
}

PRIVATE TYPE *prevtype(TYPE *type, TYPE *host)
{
  /* ACE: This test is not quite right. What if type is a 'reuse' node */
  if (skipreuse(type) == host) return NULL;
  until (hosttype(type) == host) type = hosttype(type);
  return skipreuse(type);
}

PUBLIC void walkblock(BLOCK *block, void (*applicator)(), long arg)
{
  BLOCK *child, *next;

  
  if (block == NULL)
    return;
  
  for (child = (BLOCK *)block->blocklist.head; child != NULL; child = next)
  {
    next = (BLOCK *)child->link.next;
    walkblock(child, applicator, arg);
  }
  (*applicator)(block, arg);
}

PUBLIC BLOCK *searchblock(BLOCK *block, BOOL (*selector)(), long arg)
{
  BLOCK *child, *next;


  if (block == NULL)
    return NULL;
  
  if ((*selector)(block, arg)) return block;
  for (child = (BLOCK *)block->blocklist.head; child != NULL; child = next)
  {
    next = (BLOCK *)child->link.next;
    unless ((child = searchblock(child, selector, arg)) == NULL) return child;
  }
  return NULL;
}

PRIVATE BOOL selectblock(BLOCK *block, int line)
{
  int index;


  if (block == NULL)
    return FALSE;
  
  for (index = 0; index < block->lines; index++)
  {
    if (block->linevec[index] == line) return TRUE;
  }
  
  return FALSE;
}

PUBLIC BLOCK *findblock(LOCATION loc)
{
  return searchblock(loc.module->outerblock, selectblock, loc.line);
}

PRIVATE BOOL cmpfunction(BLOCK *block, int offset)
{
  return block != NULL AND block->entry != NULL AND offset == block->entry->offset;
}

PUBLIC ENTRY *findfunction(MODULE *module, int offset)
{
  BLOCK *block;
  

  if ((block = searchblock(module->outerblock, cmpfunction, offset)) == NULL)
    {
      return NULL;
    }  
  
  return block->entry;
}

PUBLIC BOOL validline(LOCATION loc)
{
  return searchblock(loc.module->outerblock, selectblock, loc.line) != NULL;
}

/**
*
* block = newblock(parent);
*
* Create a new block.
*
**/
PUBLIC BLOCK *newblock(BLOCK *parent)
{
  BLOCK *block = NEW(BLOCK);


  initchain(&block->blocklist);

  block->parent = parent;
  block->entry  = NULL;

  unless (parent == NULL) block->module = parent->module;

  block->lines   = 0;
  block->linevec = NULL;

  unless (parent == NULL) addtail(&parent->blocklist, &block->link);

  return block;
}

/**
*
* inside = isenclosed(b1, b2);
*
* Determine whether block b1 is enclosed by block b2.
*
**/
PRIVATE BOOL isenclosed(BLOCK *block1, BLOCK *block2)
{
  until (block1 == NULL)
  {
    if (block1 == block2) return TRUE;
    block1 = block1->parent;
  }
  return FALSE;
}

/**
*
* freeblock(block);
*
* Free a block.
*
**/
PUBLIC void freeblock(BLOCK *block)
{
  if (block == NULL)
    return;
  
  unless (block->linevec == NULL) freemem(block->linevec);
  freemem(block);
}

/**
*
* type = newbasetype(id, size, issigned);
*
* Create a base type.
*
**/
PUBLIC TYPE *newbasetype(TYPEID id, int size, BOOL issigned)
{
  BASETYPE *basetype = NEW(BASETYPE);

  basetype->id = id;
  basetype->size = size;
  basetype->issigned = issigned;
  return (TYPE *)basetype;
}

/**
*
* type = newintegraltype(size, issigned);
*
* Create a integral type.
*
**/
PUBLIC TYPE *newintegraltype(int size, BOOL issigned)
{
  return newbasetype(TI_Integral, size, issigned);
}

/**
*
* type = newfloatingtype(size);
*
* Create a new floating type.
*
**/
PUBLIC TYPE *newfloatingtype(int size)
{
  return newbasetype(TI_Float, size, FALSE);
}

/**
*
* type = newtag(entry);
*
* Create a new tag definition.
*
**/
PUBLIC TYPE *newtag(ENTRY *entry)
{
  return (TYPE *)&entry->Class;
}

/**
*
* type = newtypedef(entry);
*
* Create a new type definition.
*
**/
PUBLIC TYPE *newtypedef(ENTRY *entry)
{
  return (TYPE *)&entry->Class;
}

/**
*
* type = newpointer(host);
*
* Create new pointer type.
*
**/
PUBLIC TYPE *newpointer(TYPE *host)
{
  POINTER *pointer = NEW(POINTER);

  pointer->id = TI_Pointer;
  pointer->host = host;
  return (TYPE *)pointer;
}

/**
*
* type = newarray(host, size, first);
*
* Create new array type.
*
**/
PUBLIC TYPE *newarray(TYPE *host, int size, int first)
{
  ARRAY *array = NEW(ARRAY);

  array->id = TI_Array;
  array->host = host;
  array->size = size;
  array->first = first;
  return (TYPE *)array;
}

/**
*
* type = newstruct();
*
* Create new structure type.
*
**/
PUBLIC TYPE *newstruct(void)
{
  STRUCTURE *structure = NEW(STRUCTURE);

  structure->id = TI_Struct;
  structure->tag = NULL;
  initchain(&structure->memberlist);
  return (TYPE *)structure;
}

/**
*
* type = newfunction(host);
*
* Create new function type.
*
**/
PUBLIC TYPE *newfunction(TYPE *host)
{
  FUNCTION *function = NEW(FUNCTION);

  function->id = TI_Function;
  function->host = host;
  initchain(&function->paramlist);
  return (TYPE *)function;
}

/**
*
* type = newenumeration();
*
* Create new enumerated type.
*
**/
PUBLIC TYPE *newenumeration(void)
{
  ENUMERATION *enumeration = NEW(ENUMERATION);

  enumeration->id = TI_Enum;
  initchain(&enumeration->constlist);
  return (TYPE *)enumeration;
}

/**
*
* type = reusetype(host);
*
* Build a reuse type node.
*
**/
PUBLIC TYPE *reusetype(TYPE *host)
{
  REUSE *reuse = NEW(REUSE);

  reuse->id = TI_ReUse;
  reuse->type = host;
  return (TYPE *)reuse;
}

/**
*
* type = skipreuse(type);
*
* Skip reuse nodes of a type expression.
*
**/
PUBLIC TYPE *skipreuse(TYPE *type)
{
  if (type == NULL) IOdebug("TLA: NULL type in skipreuse");
  while (type->generic.id == TI_ReUse) type = type->reuse.type;
  return type;
}

/**
*
* type = skiptypedef(type);
*
* Skip typedef and reuse nodes of a type expression.
*
**/
PUBLIC TYPE *skiptypedef(TYPE *type)
{
  if (type == NULL)
    {
      IOdebug("TLA: NULL type in skiptypedef");

      return NULL;      
    }
  
  forever
    {
      if (type->generic.id      == TI_ReUse)
	type = type->reuse.type;
      else if (type->generic.id == TI_Typedef)
	type = type->typename.type;
      else
	return type;
    }
}

/**
*
* type = hosttype(type);
*
* Obtain the host type of a type expression.
*
**/
PUBLIC TYPE *hosttype(TYPE *type)
{
  if (type == NULL) IOdebug("TLA: NULL type in hosttype");
  type = skiptypedef(type);
  unless (type->generic.id == TI_Array OR type->generic.id == TI_Function OR
          type->generic.id == TI_Pointer)
    IOdebug("TLA: Invalid type in hosttype(%d)", type->generic.id);
  return skipreuse(type->generic.host);
}

/**
*
* freetype(type);
*
* Free a type expression.
*
**/
PUBLIC void freetype(TYPE *type)
{
  switch (type->generic.id)
  {
    case TI_Tag:
    case TI_Typedef:
    return;

    case TI_ReUse:
    case TI_Integral:
    case TI_Float:
    break;

    case TI_Struct:
    walkchain(&type->structure.memberlist, freeentry, 0);
    break;

    case TI_Enum:
    walkchain(&type->enumeration.constlist, freeentry, 0);
    break;

    case TI_Function:
    walkchain(&type->function.paramlist, freeentry, 0);
    freetype(type->function.host);
    break;

    case TI_Pointer:
    case TI_Array:
    freetype(type->pointer.host);
    break;

    default:
#ifdef NEWCODE
    bug("Unknown type %s in freetype", typenames[(int)type->generic.id]);
#endif
    return;
  }
  freemem(type);
}

PRIVATE char *classnames[11] =
{
  "auto",
  "common",
  "global",
  "enum",
  "extern",
  "member",
  "param",
  "register",
  "static",
  "tag",
  "typedef"
};

PRIVATE TYPE *puttypespec(FILE *file, TYPE *type)
{
  TYPE *host = type;

  until (host->generic.id == TI_Tag OR host->generic.id == TI_Typedef OR
         host->generic.id == TI_Integral OR host->generic.id == TI_Float OR
         host->generic.id == TI_Struct OR host->generic.id == TI_Enum)
    host = host->generic.host;
  switch (host->generic.id)
  {
    case TI_Tag:
    fprintf(file, "struct %s ", GetTypeEntry(host)->name);
    break;

    case TI_Typedef:
    fprintf(file, "%s ", GetTypeEntry(host)->name);
    break;

    case TI_Integral:
    if (host->basetype.issigned)
      fprintf(file, "<%d byte signed integral> ", host->basetype.size);
    else
      fprintf(file, "<%d byte unsigned integral> ", host->basetype.size);
    break;

    case TI_Float:
    fprintf(file, "<%d byte floating point> ", host->basetype.size);
    break;

    case TI_Struct:
    fprintf(file, "struct ");
    unless (host->structure.tag == NULL)
      fprintf(file, "%s ", host->structure.tag->name);
    fprintf(file, "{\n");
    walkchain(&host->structure.memberlist, putentry, (word)file);
    fprintf(file, "} ");
    break;

    case TI_Enum:
    fprintf(file, "enum {\n");
    walkchain(&host->enumeration.constlist, putparam, (word)file);
    fprintf(file, "} ");
    break;

    default:
#ifdef NEWCODE
    bug("Unknown type %s in putbasetype", typenames[(int)host->generic.id]);
#endif
    return NULL;
  }
  return prevtype(type, host);
}

PRIVATE void putdirectdecl(FILE *file, TYPE *type, TYPE *host, char *name)
{
  if (host == NULL)
  {
    unless (name == NULL) fprintf(file, "%s", name);
    return;
  }
  switch (host->generic.id)
  {
    case TI_Pointer:
    fprintf(file, "(");
    putdecl(file, type, host, name);
    fprintf(file, ")");
    return;

    case TI_Array:
    putdirectdecl(file, type, prevtype(type, host), name);
    fprintf(file, "[%d]", host->array.size);
    return;

    case TI_Function:
    putdirectdecl(file, type, prevtype(type, host), name);
    fprintf(file, "(");
    walkchain(&host->function.paramlist, putparam, (word)file);
    fprintf(file, ")");
    return;

    default:
#ifdef NEWCODE
    bug("Unknown type %s in putdirectdecl", typenames[(int)host->generic.id]);
#endif
    return;
  }
}

PRIVATE void putdecl(FILE *file, TYPE *type, TYPE *host, char *name)
{
  unless (host == NULL)
    {
      while (host != NULL && host->generic.id == TI_Pointer)
	{
	  fprintf(file, "*");
	  
	  host = prevtype(type, host);
	}
    }
  
  putdirectdecl(file, type, host, name);
  
  return;
}

#ifndef OLDCODE
PUBLIC void myputtype(TYPE *type, FILE *file)
{
  if (type == NULL) IOdebug("TLA: NULL type in puttype()");

  switch (type->generic.id)
  {
    case TI_Array:
    fprintf(file, "[%d]", type->array.size);
    break;

    case TI_Enum:
    fprintf(file, "<>");
    return;

    case TI_Float:
    fprintf(file, ".%d", type->basetype.size);
    return;

    case TI_Function:
    fprintf(file, "()");
    break;

    case TI_Integral:
    fprintf(file, "#%d", type->basetype.size);
    return;

    case TI_Pointer:
    fprintf(file, "*");
    break;

    case TI_ReUse:
    fprintf(file, "!");
    break;

    case TI_Struct:
    fprintf(file, "{}");
    return;

    case TI_Tag:
    fprintf(file, "$%s", GetTypeEntry(type)->name);
    return;

    case TI_Typedef:
    fprintf(file, "%s", GetTypeEntry(type)->name);
    return;
  }
  myputtype(type->generic.host, file);
}
#endif

PUBLIC void puttype(TYPE *type, FILE *file)
{
  putdecl(file, type, puttypespec(file, type), NULL);
#ifdef TESTING
  fprintf(file, " / ");
  myputtype(type, file);
#endif
}

PRIVATE void putparam(ENTRY *param, FILE *file)
{
  putdecl(file, param->type, puttypespec(file, param->type), param->name);
  unless (param->link.next == NULL) fprintf(file, ", ");
}

/**
*
* putentry(entry, file);
*
* Display entry.
*
**/
PUBLIC void putentry(ENTRY *entry, FILE *file)
{
  CLASS Class = entry->Class;

  if (Class == C_Tag) return;
  if (Class == C_Extern OR Class == C_Auto OR Class == C_Typedef)
    fprintf(file, "%s ", classnames[(int)Class]);
  putdecl(file, entry->type, puttypespec(file, entry->type), entry->name);
  fprintf(file, ";\n");
}

/**
*
* putsymbol(symbol, file);
*
* Display symbol entries.
*
**/
PUBLIC void putsymbol(SYMBOL *symbol, FILE *file)
{
  walkchain(&symbol->entrylist, putentry, (word)file);
}
