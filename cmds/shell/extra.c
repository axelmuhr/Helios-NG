/**
*
* Title:  Helios Shell - Extra Support.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/extra.c,v 1.11 1994/03/01 14:14:03 nickc Exp $
*
**/
#include "shell.h"

LIST filelist;
LIST looplist;

BOOL pushdir(char *path)
{
  char name[PATH_MAX + 1];

  if (getcwd(name, PATH_MAX) == NULL) return FALSE;
  unless (changedir(path)) return FALSE;
  adddir(name);
  dirs();
  return TRUE;
}

BOOL changedir(char *path)
{
  char name[PATH_MAX + 1];
  char **cdpathargv;
  char *cdpath;
  void stripslash(char *);

  stripslash(path);
  unless (chdir(path) == -1)
  {
    if (getcwd(name, PATH_MAX) == NULL) return FALSE;
    set("cwd", makeargv(name));
    return TRUE;
  }
  unless ((cdpathargv = findvar("cdpath")) == NULL)
  {
    until ((cdpath = *cdpathargv++) == NULL)
    {
      formfilename(name, cdpath, path);
      stripslash(name);
      unless (chdir(name) == -1)
      {
        if (getcwd(name, PATH_MAX) == NULL) return FALSE;
        set("cwd", makeargv(name));
        return TRUE;
      }
    }
  }
  return FALSE;
}

BOOL dirs()
{
  char path[PATH_MAX + 1];

  if (getcwd(path, PATH_MAX) == NULL) return FALSE;
  printf("%s ", path);
  WalkList((List *)&dirlist, (WordFnPtr)putdirnode);
  putchar('\n');
  return TRUE;
}

void newfile(FILE *file)
{
  FILEINFO *fileinfo = new(FILEINFO);
  int fd = fileno(file);

#ifdef DEBUGGING
  DEBUG("newfile()\n");
#endif  
fileinfo->inputfile = inputfile;
  fileinfo->interactive = interactive;
  fileinfo->cmd = globcmd;
  fileinfo->argv = globargv;
  inputfile = file;
  interactive = (BOOL) isatty(fd);
  globcmd = NULL;
  innewfile++ ;
#ifdef SYSDEB
  fileinfo->next = fileinfo->prev = fileinfo;
#endif
  AddHead((List *)&filelist, (Node *)fileinfo);
}

void oldfile()
{
  FILEINFO *fileinfo = (FILEINFO *)RemHead(&filelist);

#ifdef DEBUGGING
  DEBUG("oldfile()\n");
#endif
  fclose(inputfile);
  freecmd(globcmd);
  freeargv(globargv);
  inputfile = fileinfo->inputfile;
  interactive = fileinfo->interactive;
  innewfile-- ;
  globcmd = fileinfo->cmd;
  globargv = fileinfo->argv;
  freememory((int *)fileinfo);
}

void tidyupfiles()
{
#ifdef DEBUGGING
  DEBUG("tidyupfiles()\n");
#endif
  until (filelist.Head->Next == NULL) oldfile();
}

void newloop()
{
  LOOPINFO *loopinfo = new(LOOPINFO);

#ifdef DEBUGGING
  DEBUG("newloop()\n");
#endif
  loopinfo->cmd = globcmd;
  globcmd = NULL;
#ifdef SYSDEB
  loopinfo->next = loopinfo->prev = loopinfo;
#endif
  AddHead(&looplist, (NODE *)loopinfo);
}

void oldloop()
{
  LOOPINFO *loopinfo = (LOOPINFO *)RemHead(&looplist);

#ifdef DEBUGGING
  DEBUG("oldloop()\n");
#endif
  freecmd(globcmd);
  globcmd = loopinfo->cmd;
  freememory((int *)loopinfo);
}

void tidyuploops()
{
#ifdef DEBUGGING
  DEBUG("tidyuploops()\n");
#endif
  until (looplist.Head->Next == NULL) oldloop();
}

BOOL inloop()
{
  return (BOOL) (looplist.Head->Next != NULL);
}

char *strdup(char *s)
{
  char *d = (char *)newmemory(strlen(s) + 1);

  strcpy(d, s);
  return d;
}


char *newmemory(int size)
{
  if (memorychecking)
  {
    int *mem = (int *)malloc(size + sizeof(int) + sizeof (int));
    char *bmem = (char *)mem;

    if (mem == NULL) exit(error(ERR_NOMEMORY, NULL));
    mem[0] = size;
    totalmemory += size;
    					/* Put adress of size behind	*/
    					/* allocated data area		*/
    memcpy (&bmem[size+sizeof(int)], &mem, sizeof(int));
    return (char *)(mem + 1);
  }
  else return (char *)malloc(size);
}

void freememory(int *mem)
{
  if (memorychecking)
  {
    int size;
    char *bmem;

    mem = (int *)mem - 1;
    bmem = (char *)mem;
    size = ((int *)mem)[0];
    					/* check size address...	*/
    if ( ((word)mem & 3) || memcmp(&mem, &bmem[size+sizeof(int)], sizeof (int)))
    {
      IOdebug ("shell: fatal memory error at %x.", (int *)mem);
      memcpy (&mem, &bmem[size+sizeof(int)], sizeof(int));
      IOdebug( "size = %x, ptr to mem = %x ", size, mem );
      exit (1);
    }
    totalmemory -= size;
  }
  free(mem);
}



void putmem()
{
  printf("Memory used = %d bytes\n", totalmemory);
}

void stripslash(char *name)
{
	char *freeloc, *p = (char *) Malloc((word)strlen(name)+1);
	char *start = name;

	if(p == (char *)NULL)
		return;

	freeloc = p;

	memcpy(p, name, strlen(name)+1);

	while(*p)
		{
		*name++ = *p;
		if(*p++ == '/')
			while(*p == '/')
				p++;
		}

	*name = 0;

	if(name[-1] == '/' && (name > start+1))
		name[-1] = 0;

	Free(freeloc);

	return;
}

int strnequ(char *p, char *q, int n)
{
	if(p == (char *)NULL || q == (char *)NULL)
		return 0;

	return(strncmp(p, q, n) == 0);
}

int strequ(char *p, char *q)
{
	if(p == (char *)NULL || q == (char *)NULL)
		return 0;

	return(strcmp(p, q) == 0);
}
