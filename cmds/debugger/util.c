/**
*
* Title:  Helios Debugger - Utilities.
*
* Author: Andy England
*
* Date:   April 1989
*
*         (c) Copyright 1989 - 1993, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/util.c,v 1.7 1993/07/06 14:10:20 nickc Exp $";
#endif

#include "tla.h"

/**
*
* d = strdup(s);
*
* Duplicate a string.
*
**/
PUBLIC char *strdup(char *s)
{
  char *d = (char *)newmem(strlen(s) + 1);

  strcpy(d, s);
  return d;
}

/**
*
* tabexp(buf, s, n);
*
* Copy a string into a buffer expanding tabs.
*
**/
PUBLIC
#ifdef V1_1
void
#else
char *
#endif
tabexp(char *buf, char *s, int n)
{
  int i = 0;
  int c;

  until ((c = *s++) == '\0')
  {
    if (c == '\t')
    {
      do
      {
        unless (i < n) break;
        buf[i++] = ' ';
      } until (i % TabSize == 0);
    }
    else
    {
      unless (i < n) break;
      buf[i++] = c;
    }
  }
  buf[i] = '\0';
  
#ifdef V1_1
  return; /* JMP removed return buf */
#else
  return buf;
#endif
}

/**
*
* same = optequ(s, t)
*
* Compare a string with a string containing optional characters.
*
**/
PUBLIC BOOL optequ(char *s, char *t)
#if 0
{
  int c, d;
  until ((c = *s++) == '\0')
  {
    if ((d = *t++) == '[')
    {
      until ((d = *t++) == '\0' OR d == ']')
      {
      	if (d == '\\')
      	{
          unless (c == *t++) 
          {
            return FALSE;
          }
      	}
      	else unless ( c == d )
      	{
      	  return FALSE;
      	}
      	if ((c = *s++) == '\0')
      	{
      	  return TRUE;
      	}
      }
      /* ACE: A bit dodgy at this point */
      s--;
/*
      if (d == '\0' OR t[1] == '\0')
      { 
        return TRUE;
      }
*/
    }
    else if (d == '\\')
    {
      if (c != *t++) 
      {
        return FALSE;
      }
    } 
    else unless (c == d)
    {
      return FALSE;
    }
  }
  return TRUE;
/*  
  return (!*(t+1));
*/
/*  (d = *t++) == '\0'); */
}
#else
{
  int c, d, f = FALSE;
  
  for ( c = *s++, d = *t++; ; c = *s++, d = *t++ )
    {
      switch ( d )
	{
	case '[':
	  f = TRUE;
	  if (( d = *t++ ) == 0 )
	    {
	      return FALSE;
	    }
	  break;
	case ']':
	  if ( !f ) break;
	  /*      if ( (d = *t++) == 0 )
		  {
		  return FALSE;
		  }*/
	  d = *t++;
	  break;
	case '\\':
	  if (( d = *t++ ) == 0 )
	    {
	      return FALSE;
	    }
	  break;
	}
      if ( !c )
	{
	  return f ? TRUE : !d;
	}
      if ( !d )
	{
	  return FALSE;
	}
      if ( c != d )
	{
	  return FALSE;
	}
    }
}
#endif

/**
*
* value = getvar(env, name);
*
* Get the value of an environment variable.
*
**/
PUBLIC char *getvar(char **env, char *name)
{
  int length = strlen(name);
  char *var;

  until ((var = *env++) == NULL)
  {
    if (strncmp(name, var, length) == 0 AND var[length] == '=')
      return var + length + 1;
  }
  return NULL;
}

/*
-- crf : 17/08/91 - Don't need this anymore ...
*/
#ifdef CRFOLD
/**
* -- crf : 15/07/91 - Bug 698
*
* result = alldigits(s);
*
* TRUE if s consists only of digits
*
**/
PRIVATE int alldigits (char *s)
{
	int result = TRUE ;
	while ((*s) && (result))
	  result = isdigit (*s ++) ;
	return result ;
}
#endif

/**
*
* loc = getloc(debug, s);
*
* Convert a string into a location.
*
**/
PUBLIC LOCATION getloc(DEBUG *debug, char *s)
{
  LOCATION loc;

  if (s[0] == '/')
  {
    char *name = s + 1;

    if ((s = strchr(name, '/')) == NULL) loc.line = 1;
    else
    {
      *s++ = '\0';
#ifdef CRFOLD
/* 
-- crf : 15/07/91 - Bug 698
-- Before converting to int, test that s consists only of digits ...
-- If not, the location is bad (i.e. set loc.module = NULL and get out)
*/
      if (alldigits (s))
        loc.line = atoi(s);
      else
      {
        loc.module = NULL ;
        *(-- s) = '/' ; /* restore string for error message */	
        return loc ;
      }
#endif
      loc.line = atoi(s);
    }
    loc.module = findmodule(debug, name);
  }
  else
  {
    loc.module = debug->thread->loc.module;
#ifdef CRFOLD
/* 
-- crf : 15/07/91 - Bug 698
-- Before converting to int, test that s consists only of digits ...
-- If not, the location is bad (i.e. set loc.module = NULL and get out)
*/
    if (alldigits (s))
      loc.line = atoi(s);
    else
    {
      loc.module = NULL ;
      return loc ;
    }
#endif
    loc.line = atoi(s);
  }
  unless (loc.module == NULL)
  {
    SOURCE *source;

    unless ((source = getsource(loc.module)) == NULL)
    {
#ifdef OLDCODE
      if (loc.line > source->lastline) loc.module = NULL;
#endif
/* 
-- crf : 15/08/91 - Bug 698 (revisited)
-- also return NULL if loc.line < 1
*/
      if ((loc.line > source->lastline) || (loc.line < 1))
        loc.module = NULL;
    }
/* 
-- crf : 15/08/91 - Bug 698 (revisited)
-- if getsource() fails, must set loc.module to NULL
*/
    else
      loc.module = NULL ;
  }
/* 
-- crf : restore string for error message 
-- (this is a bit nasty)
*/	
  if ((loc.module == NULL) && (*(--s) == NULL))
    *s = '/' ; 

  return loc;
}

/**
*
* locstr = formloc(buf, loc);
*
* Form a location path from a location.
*
**/
PUBLIC char *formloc(char *buf, LOCATION loc)
{
  char nstr[20];

  strcpy(buf, "/");
  strcat(buf, loc.module->name);
  strcat(buf, "/");
  sprintf(nstr, "%d", loc.line);
  strcat(buf, nstr);
  return buf;
}

/**
*
* name = basename(name);
*
* Obtain the basename from a path.
*
**/
PUBLIC char *basename(char *name)
{
  char *ptr;

  if ((ptr = strrchr(name, '/')) == NULL) return name;
  return ptr + 1;
}

/**
*
* c = getkeyname(keyname);
*
* Return the key for a given key name.
*
**/
PUBLIC int getkeyname(char *keyname)
{
  int c;
  BOOL shifted    = FALSE;
  BOOL controlled = FALSE;

  
  if (strequ(keyname, "backspace")) return Backspace;
  if (strequ(keyname, "delete"))    return VDelete;
  if (strequ(keyname, "down"))	    return DownArrow;
  if (strequ(keyname, "end"))	    return End;
  if (strequ(keyname, "escape"))    return Escape;
  if (strequ(keyname, "help"))	    return Help;
  if (strequ(keyname, "home"))	    return Home;
  if (strequ(keyname, "insert"))    return VInsert;
  if (strequ(keyname, "left"))	    return LeftArrow;
  if (strequ(keyname, "pagedown"))  return PageDown;
  if (strequ(keyname, "pageup"))    return PageUp;
  if (strequ(keyname, "return"))    return Return;
  if (strequ(keyname, "right"))	    return RightArrow;
#ifdef NO_LONGER_WORKS
  if (strequ(keyname, "shift-down"))  return ShiftDownArrow;
  if (strequ(keyname, "shift-left"))  return ShiftLeftArrow;
  if (strequ(keyname, "shift-right")) return ShiftRightArrow;
  if (strequ(keyname, "shift-up"))    return ShiftUpArrow;
#endif
  if (strequ(keyname, "space")) return Space;
  if (strequ(keyname, "tab"))   return Tab;
  if (strequ(keyname, "undo"))	return Undo;
  if (strequ(keyname, "up"))	return UpArrow;

  if (keyname[0] == '$')
    return atoi(keyname + 1);

  if (strnequ(keyname, "shift-", 6))
    {
      keyname += 6;
      shifted = TRUE;
    }
  else if (strnequ(keyname, "ctrl-", 5))
    {    
      keyname += 5;
    
      controlled = TRUE;
    }
  else if (strnequ(keyname, "control-", 8))
    {
      keyname += 8;
      controlled = TRUE;
    }   
  else if (keyname[0] == '^')
    {
      keyname++;
      controlled = TRUE;
    }

  if (keyname[0] == 'f' AND keyname[1] != '\0')
    {
      if ((c = atoi(keyname + 1)) == 0 OR c > 10) return -1;
      return shifted ? c + (FunctionKeys + 9) : c + (FunctionKeys - 1);
    }
  
  unless (keyname[1] == '\0')
    return -1;
  
  c = keyname[0];
  
  return shifted ? (Shift & c) : controlled ? (VControl & c) : c;

} /* getkeyname */

/**
*
* formkeyname(keyname, c);
*
* Form the key name for a key into a buffer.
*
**/
PUBLIC void formkeyname(char *keyname, int c)
{
  switch (c)
  {
    case Backspace:
    strcpy(keyname, "backspace");
    break;

    case VDelete:
    strcpy(keyname, "delete");
    break;
    break;

    case DownArrow:
    strcpy(keyname, "down");
    break;

    case End:
    strcpy(keyname, "end");
    break;

    case Escape:
    strcpy(keyname, "escape");
    break;

    case Help:
    strcpy(keyname, "help");
    break;

    case Home:
    strcpy(keyname, "home");
    break;

    case VInsert:
    strcpy(keyname, "insert");
    break;

    case LeftArrow:
    strcpy(keyname, "left");
    break;

    case PageDown:
    strcpy(keyname, "pagedown");
    break;

    case PageUp:
    strcpy(keyname, "pageup");
    break;

    case Return:
    strcpy(keyname, "return");
    break;

    case RightArrow:
    strcpy(keyname, "right");
    break;

#ifdef NO_LONGER_WORKS
    case ShiftDownArrow:
    strcpy(keyname, "shift-down");
    break;

    case ShiftLeftArrow:
    strcpy(keyname, "shift-left");
    break;

    case ShiftRightArrow:
    strcpy(keyname, "shift-right");
    break;

    case ShiftUpArrow:
    strcpy(keyname, "shift-up");
    break;
#endif
    
    case Space:
    strcpy(keyname, "space");
    break;

    case Tab:
    strcpy(keyname, "tab");
    break;

    case Undo:
    strcpy(keyname, "undo");
    break;

    case  UpArrow:
    strcpy(keyname, "up");
    break;

    default:
    if (c >= 0 AND c <= 0x20) sprintf(keyname, "ctrl-%c", (~VControl | c));
    else if (c >= FunctionKeys AND c < FunctionKeys + 10)
      sprintf(keyname, "f%d", c - FunctionKeys + 1);
    else if (c >= FunctionKeys + 10 AND c < FunctionKeys + 20)
      sprintf(keyname, "shift-f%d", c - FunctionKeys - 9);
    else sprintf(keyname, "%c", c);
    break;
  }
}

/**
*
* formvarloc(buf, entry);
*
* Form location of symbol table entry.
*
**/
PRIVATE void formblockloc(char *buf, BLOCK *block)
{
  if (block == NULL)
    {
      buf[0] = '\0';
      return;
    }
  
  if (block->parent == NULL)
  {
    strcpy(buf, "/");
    strcat(buf, block->module->name);
  }
  else
  {
    formblockloc(buf, block->parent);
    unless (block->entry == NULL)
    {
      strcat(buf, "/");
      strcat(buf, block->entry->name);
    }
  }
}

PUBLIC void formvarloc(char *buf, ENTRY *entry)
{
  formblockloc(buf, entry->block);
  strcat(buf, "/");
  strcat(buf, entry->name);
}

/**
*
* arg = formword(argv);
*
* Form an argv into a single word;
*
**/
PUBLIC char *formword(char **argv)
{
  int argc = 0;
  int len = 0;
  char *arg, *text;

  until ((arg = argv[argc++]) == NULL) len += strlen(arg) + 1;
  text = (char *)newmem(len + 1);
  text[0] = '\0';
  argc = 0;
  until ((arg = argv[argc++]) == NULL)
  {
    strcat(text, arg);
    strcat(text, " ");
    freemem(arg);
  }
  return text;
}

/**
*
* bigbuf(file);
*
* Attempts to give the file a buffer the size of the file.
*
**/
PUBLIC void bigbuf(FILE *file)
{
  setvbuf(file, NULL, _IOFBF, (int)GetFileSize(Heliosno(file)));
}


#ifdef V1_1

/* JMP 08/03/89
   These calls have been added to lift the 16 filedescriptor limit
   on the debugger. Bad eh ?
*/

#include <posix.h>
#include <fcntl.h>

typedef struct MFILE
{ unsigned char *_ptr;
  int _icnt;      /* two separate _cnt fields so we can police ...        */
  int _ocnt;      /* ... restrictions that read/write are fseek separated */
  int _flag;
  unsigned char *_base;
  int *_sysbase;		/* BLV - used for MSdos files */
  int _file;
  int _pos;
  int _bufsiz;
  int _signature;
  unsigned char _lilbuf[1];
} MFILE;

#define _IOAPPEND 0x8000
#define _IOFUNNY  0x00010000

# define _IOREAD   0x01
# define _IOWRITE  0x02
# define _IOBIN    0x04
# define _IOSTRG   0x08
# define _IOSEEK   0x10
# define _IOSBF   0x800


PUBLIC FILE *my_fopen(const char *filename, const char *access_mode)
{

  FILE  *fopen_result;
  int   i;
  Wait(&loadlock);
  for (i=3; i<_MYSYS_OPEN; i++)
    {   FILE *stream = (FILE*)&my_iob[i];
        if (!(stream->_flag & _IOREAD + _IOWRITE))  /* if not open then try it */
          {
            fopen_result = freopen(filename, access_mode, stream);
            Signal(&loadlock);
            return fopen_result;
	  }
    }
    Signal(&loadlock);
    return Null(FILE);   /* no more i/o channels allowed for */
}
  
PUBLIC FILE *my_fdopen(int fh, char *mode)
{
    int i;
    
    Wait(&loadlock);    
    /* first check that the fd is valid */
    if( fcntl(fh,F_GETFD) == -1 ){Signal(&loadlock);  return Null(FILE);}

    for (i=0; i<_MYSYS_OPEN; i++)
    {   MFILE *iob = (MFILE*)&my_iob[i];
        if (!(iob->_flag & _IOREAD+_IOWRITE))  /* if not open then try it */
        {
                int flag, openmode;
                    switch (*mode++)
                    {   default:  return(NULL);               /* mode is incorrect */
                        case 'r': flag = _IOREAD;  openmode = 0; break;
                        case 'w': flag = _IOWRITE; openmode = 4; break;
                        case 'a': flag = _IOWRITE | _IOAPPEND;
                                                   openmode = 8; break;
                    }
                for (;;)
                    {   switch (*mode++)
                        {
                    case '+':   flag |= _IOREAD+_IOWRITE, openmode |= 2;
                                continue;
                    case 'b':   flag |= _IOBIN, openmode |= 1;
                                continue;
                        }
                        break;
                    }
        
                    if( ( ( fdstream(fh)->Flags & Flags_MSdos ) != 0 ) &&
                          ((openmode & 0x1) == 0)) flag |= _IOFUNNY;
     
                    iob->_flag = flag;
                    iob->_file = fh;
		    iob->_sysbase = NULL;

		    if (openmode & 8) fseek((FILE*)iob, 0L, SEEK_END);  /* a or a+ */
                    Signal(&loadlock);
                    return (FILE*)iob;
	}
     }
     Signal(&loadlock);
     return Null(FILE);   /* no more i/o channels allowed for */
}
 
#endif /* V1_1 */
