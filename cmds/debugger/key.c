/**
*
* Title:  Helios Debugger - Key mapping support.
*
* Author: Andy England
*
* Date:   September 1988
*
*         (c) Copyright 1988 - 1993, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/key.c,v 1.4 1993/03/19 16:51:58 nickc Exp $";
#endif

#include "tla.h"

/**
*
* initkeymap(keymap);
*
* Initialise a keymap.
*
**/
PUBLIC void initkeymap(KEYMAP keymap)
{
  int c;

  for (c = 0; c <= MAX_KEY; c++) keymap[c] = NULL;
}

/**
*
* freekeymap(keymap);
*
* Free keymap.
*
**/
PUBLIC void freekeymap(KEYMAP keymap)
{
  int c;

  for (c = 0; c <= MAX_KEY; c++) remkey(keymap, c);
}

/**
*
* addkey(keymap, key, cmd);
*
* Add a new key mapping.
*
**/
PUBLIC void addkey(KEYMAP keymap, int c, char *cmd)
{
  if (c > MAX_KEY) return;
  remkey(keymap, c);
  keymap[c] = strdup(cmd);
}

/**
*
* remkey(keymap, key);
*
* Remove the key mapping for the specified key.
*
**/
PUBLIC void remkey(KEYMAP keymap, int c)
{
  if (c > MAX_KEY OR keymap[c] == NULL) return;
  freemem(keymap[c]);
  keymap[c] = NULL;
}

/**
*
* getkey(keymap, c);
*
* Get key mapping for specified key.
*
**/
PUBLIC char *getkey(KEYMAP keymap, int c)
{
  return (c > MAX_KEY) ? NULL : keymap[c];
}

/**
*
-- crf : 17/07/91 - (minor) bug 678
*
* formattext(original, newtext)
*
* Format text for "list -k"
*
-- Tidy up key listings (I'm using the same technique as for aliases ... 
-- refer interp.c)
*
**/
PRIVATE void formattext(char *original, char *newtext)
{
  while (*original)
  {
    char this_char = *original ;
/*
-- crf : replace tabs and ';'s with newline + spaces
*/
    if ((this_char == '\t') || (this_char == ';'))
    {
      int count ;
      if (this_char == ';')
        *newtext ++ = this_char ;
      *newtext ++ = '\n' ;
      for (count = 0 ; count < strlen ("key ") ; count ++)
        *newtext ++ = ' ' ;
      original ++ ;
      while ((*original == this_char) || (*original == ' '))
        original ++ ;
    }
    else
      *newtext ++ = *original ++ ;
  }
/*
-- crf : get rid of spaces, newlines at end of text
*/
  newtext -- ;
  while ((*newtext == '\n') || (*newtext == ' '))
    newtext -- ;
  *(++ newtext) = NULL ;
}

/**
*
* listkeys(debug)
*
* Display a list of all key bindings.
*
**/
PUBLIC void listkeys(KEYMAP keymap, DISPLAY *display)
{
  int c;

  for (c = 0; c <= MAX_KEY; c++)
  {
    unless (keymap[c] == NULL)
    {
      char keyname[20];

      formkeyname(keyname, c);

      { 
        char newtext [1024] ;
/* 
-- crf : probably a bit big ... can't malloc 'cos I will be inserting 
-- characters into the text
*/
        formattext (keymap [c], newtext) ;

        dprintf(display, "key %s\n    [%s]\n", keyname, newtext);

#ifdef OLDCODE
      dprintf(display, "key %s [%s]\n", keyname, keymap[c]);
#endif
      }
    }
  }
}
