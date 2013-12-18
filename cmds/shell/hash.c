/**
*
* Title:  Helios Shell - Hash Table Support.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/hash.c,v 1.9 1994/02/07 11:53:43 nickc Exp $
*
**/
#include "shell.h"

LIST hashtable[HASH_MAX];
int hashing = FALSE;

void hash()
{
  char **pathargv;
  char *path;

#ifdef DEBUGGING
  DEBUG("hash()");
#endif

  unhash();
  if ((pathargv = findvar("path")) == NULL) return;

#ifdef DEBUGGING
  DEBUG("hash() path set");
#endif

  until ((path = *pathargv++) == NULL)
  {
    DIRECTORY *directory;
    DIR *dir;
    DIRENT *dirent;

#ifdef DEBUGGING
  DEBUG("hash() %s", path);
#endif

    if (isabspath(path))
    {
      unless ((dir = opendir(path)) == NULL)
      {
#ifdef DEBUGGING
DEBUG("fd %d loc %x size %x pos %x dd_buf %x", dir->dd_fd,
	dir->dd_loc, dir->dd_size, dir->dd_pos, &dir->dd_buf);
#endif
        directory = adddirectory(path);
        until ((dirent = readdir(dir)) == NULL)
        {
#ifdef DEBUGGING
DEBUG("typeaddr %x type %x flags %x matrix %x name %s", &dirent->d_type, dirent->d_type,
	dirent->d_flags, dirent->d_matrix, dirent->d_name);
#endif
          if (dirent->d_name[0] != '.' AND isexec(dirent, path)) {

#ifdef DEBUGGING
	    DEBUG("hash() add %s", dirent->d_name);
#endif

            ignore addentry(directory, dirent->d_name);
	  }
        }
        closedir(dir);
      }
    }
  }
  hashing = TRUE;

#ifdef DEBUGGING
  DEBUG("hash() done");
#endif

}

void unhash()
{
  int number;

  for (number = 0; number < HASH_MAX; number++)
  {
    if (hashing) WalkList((List *)&hashtable[number], (WordFnPtr)freedirectory);
    InitList(&hashtable[number]);
  }
  hashing = FALSE;
}

void hashstat()
{
}

DIRECTORY *adddirectory(char *name)
{
  DIRECTORY *directory = new(DIRECTORY);
  int number;

  directory->name = strdup(name);
#ifdef SYSDEB
  directory->next = directory->prev = directory;
#endif
  for (number = 0; number < HASH_MAX; number++)
    InitList(&directory->hashtable[number]);
  AddTail(&hashtable[hashnumber(name)], (NODE *)directory);
  return directory;
}

DIRECTORY *finddirectory(char *name)
{
  DIRECTORY *directory;

  for (directory = (DIRECTORY *)hashtable[hashnumber(name)].Head;
       directory->next; directory = directory->next)
  {
    if (strequ(name, directory->name)) return directory;
  }
  return NULL;
}

void freedirectory(DIRECTORY *directory)
{
  int number;

  for (number = 0; number < HASH_MAX; number++)
  {
    WalkList((List *)&directory->hashtable[number], (WordFnPtr)freeentry);
  }
  freememory((int *)directory->name);
  freememory((int *)directory);
}

ENTRY *addentry(
		DIRECTORY *directory,
		char *name )
{
  ENTRY *entry = new(ENTRY);

  entry->name = strdup(name);
#ifdef SYSDEB
  entry->next = entry->prev = entry;
#endif
  AddTail(&directory->hashtable[hashnumber(name)], (NODE *)entry);
  return entry;
}

ENTRY *findentry(
		 DIRECTORY *directory,
		 char *name )
{
  ENTRY *entry;

  for (entry = (ENTRY *)directory->hashtable[hashnumber(name)].Head;
       entry->next; entry = entry->next)
  {
    if (strequ(name, entry->name)) return entry;
  }
  return NULL;
}

void freeentry(ENTRY *entry)
{
  freememory((int *)entry->name);
  freememory((int *)entry);
}

BOOL lookforcmd(
		char *path,
		char *name )
{
  DIRECTORY *directory;

#ifdef HELIOS
  unless (hashing AND isabspath(path) AND !strequ("/loader",path) ) 
  {
    Object *cmd;
    static char pathname[PATH_MAX + 1];
    BOOL ans;

    formfilename(pathname, path, name);
    if ((cmd = Locate(CurrentDir, pathname)) == NULL) return FALSE;
    ans = (BOOL) (cmd->Type == (cmd->Type & Type_Directory) == 0);
    Close(cmd);
    return ans;
  }
#else
  unless (hashing AND isabspath(path)) return TRUE;
#endif
  return ((directory = finddirectory(path)) != NULL) ?
    (findentry(directory, name) ? TRUE : FALSE) : FALSE;
}

void listcmds(char *name)
{
  DIRECTORY *directory;
  ENTRY *entry;
  BUILTIN *builtin;
  char **newargv = nullargv();
  int n;
  int length = strlen(name);
  char **pathargv;

  for (builtin = builtins; builtin->name; builtin++)
  {
    if (strnequ(name, builtin->name, length))
    {
      newargv = addword(newargv, builtin->name);
    }
  }
  unless ((pathargv = findvar("path")) == NULL)
  {
    char *path;

    until ((path = *pathargv++) == NULL)
    {
      DIR *dir;
      DIRENT *dirent;

      if (hashing AND isabspath(path))
      {
        unless ((directory = finddirectory(path)) == NULL)
        {
          for (n = 0; n < HASH_MAX; n++)
          {
            for (entry = (ENTRY *)directory->hashtable[n].Head; entry->next;
                 entry = entry->next)
            {
              if (strnequ(name, entry->name, length))
              {
                newargv = addword(newargv, entry->name);
              }
            }
          }
        }
      }
      else
      {
        unless ((dir = opendir(path)) == NULL)
        {
          until ((dirent = readdir(dir)) == NULL)
          {
            if (isexec(dirent, path) AND strnequ(name, dirent->d_name, length))
            {
              newargv = addword(newargv, dirent->d_name);
            }
          }
          closedir(dir);
        }
      }
    }
  }
  putsortedargv(newargv);
  freeargv(newargv);
}

BOOL completecmd(char *name)
{
  DIRECTORY *directory;
  ENTRY *entry;
  BUILTIN *builtin;
  int n, length;
  int minlength = strlen(name);
  COMPSTATE state = NOTFOUND;
  char **pathargv;

  for (builtin = builtins; builtin->name; builtin++)
  {
    if ((length = strlequ(name, builtin->name)) >= minlength)
    {
      if (state == NOTFOUND)
      {
        state = COMPLETE;
        strcpy(name, builtin->name);
      }
      else
      {
        state = INCOMPLETE;
        name[length] = '\0';
      }
    }
  }
  unless ((pathargv = findvar("path")) == NULL)
  {
    char *path;

    until ((path = *pathargv++) == NULL)
    {
      DIR *dir;
      DIRENT *dirent;

      if (hashing AND isabspath(path))
      {
        unless ((directory = finddirectory(path)) == NULL)
        {
          for (n = 0; n < HASH_MAX; n++)
          {
            for (entry = (ENTRY *)directory->hashtable[n].Head; entry->next;
                 entry = entry->next)
            {
              if ((length = strlequ(name, entry->name)) >= minlength)
              {
                if (state == NOTFOUND)
                {
                  state = COMPLETE;
                  strcpy(name, entry->name);
                }
                else
                {
                  state = INCOMPLETE;
                  name[length] = '\0';
                }
              }
            }
          }
        }
      }
      else
      {
        unless ((dir = opendir(path)) == NULL)
        {
          until ((dirent = readdir(dir)) == NULL)
          {
            if (isexec(dirent, path) AND (length = strlequ(name, dirent->d_name)) >= minlength)
            {
              if (state == NOTFOUND)
              {
                state = COMPLETE;
                strcpy(name, dirent->d_name);
              }
              else
              {
                state = INCOMPLETE;
                name[length] = '\0';
              }
            }
          }
          closedir(dir);
        }
      }
    }
  }
  return (BOOL)(state == COMPLETE);
}

void listfiles(char *filename)
{
  char **newargv = nullargv();
  char path[PATH_MAX + 1], name[PATH_MAX + 1];
  DIR *dir;
  DIRENT *dirent;
  int length;

  ignore splitfilename(filename, path, name);
  unless ((dir = opendir(path)) == NULL)
  {
    length = strlen(name);
    until ((dirent = readdir(dir)) == NULL)
    {
      unless (dirent->d_name[0] == '.' AND name[0] != '.')
      {
        if (strnequ(name, dirent->d_name, length))
        {
          char buffer[PATH_MAX + 1];

          strcpy(buffer, dirent->d_name);
          if (isdir(dirent, path)) strcat(buffer, "/");
#ifdef HELIOS
          else if (isexec(dirent, path)) strcat(buffer, "*");
#endif
          newargv = addword(newargv, buffer);
        }
      }
    }
    closedir(dir);
  }
  putsortedargv(newargv);
  freeargv(newargv);
}

BOOL completefile(char *filename)
{
  char *split;
  char path[PATH_MAX + 1], name[PATH_MAX + 1];
  DIR *dir;
  DIRENT *dirent;
  int length, minlength;
  COMPSTATE state = NOTFOUND;

  split = splitfilename(filename, path, name);
  unless ((dir = opendir(path)) == NULL)
  {
    minlength = strlen(name);
    until ((dirent = readdir(dir)) == NULL)
    {
      unless (dirent->d_name[0] == '.' AND name[0] != '.')
      {
        if ((length = strlequ(name, dirent->d_name)) >= minlength)
        {
          if (state == NOTFOUND)
          {
            state = COMPLETE;
            strcpy(name, dirent->d_name);
          }
          else
          {
            state = INCOMPLETE;
            name[length] = '\0';
          }
        }
      }
    }
    closedir(dir);
    strcpy(split, name);
  }
  return (BOOL)(state == COMPLETE);
}

char *splitfilename(
		    char *filename,
		    char *path,
		    char *name )
{
  char *split;

  char *p = filename;

  path[0] = '\0';
  name[0] = '\0';

  if (*p == '~')			/* filename starts with ~	*/
  {
    char username[WORD_MAX + 1];
    int i = 0;

    p++;				/* copy username to local var	*/
    until (*p == '\0' OR *p == '/') 
      username[i++] = *p++;
    if (i > 0)				/* named user, get pwd entry	*/
    {
      struct passwd *pwent;

      username[i] = '\0';
      pwent = getpwnam(username);
      endpwent();
      if (pwent != NULL)
        strcpy(path, pwent->pw_dir);	/* user found, get its home dir	*/
      else
        p = filename;			/* uset not found, reset..	*/
    }
    else				/* no named user, get $home dir	*/
    {
      char *home;

      if ((home = getvar("home")) != NULL)
        strcpy(path, home);
      else
        p = filename;			/* no $home available, reset..	*/
    }
  }
  if ((split = strrchr(p, '/')) == NULL)
    split = p;
  else
  {
    split++;
    strncat(path, p, split - p);
  }
  strcpy(name, split);
  return split;
}

void formfilename(
		  char *filename,
		  char *path,
		  char *name )
{
  if (path[0] == '\0' OR name[0] == '/' OR strequ(path, "."))
    strcpy(filename, name);
  else
  {
    strcpy(filename, path);
    strcat(filename, "/");
    strcat(filename, name);
  }
}

int hashnumber(char *name)
{
  int c;
  unsigned h = 0;
  unsigned g;

  until ((c = *name++) == '\0')
  {
    h = (h << 4) + c;
    unless ((g = h & 0xF0000000) == 0)
    {
      h ^= (g >> 24);
      h ^= g;
    }
  }
  return h % HASH_MAX;
}
 
int strlequ(
	    char *s,
	    char *t  )
{
  int length = 0;

  while (*s AND (*s++ == *t++)) length++;
  return length;
}
