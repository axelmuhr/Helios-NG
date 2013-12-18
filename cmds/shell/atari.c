/**
*
* Title:  Helios Shell - Atari system dependent parts.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/atari.c,v 1.1 1990/08/23 15:47:14 james Exp $
*
**/
#include "shell.h"
#include <osbind.h>
#include <stat.h>

void unixpath(path)
char *path;
{
  int index = 0;

  if ((strlen(path) > 1) AND (path[1] == ':'))
  {
    path[1] = path[0];
    path[0] = '/';
    index = 2;
  }
  while (path[index])
  {
    if (path[index] == '\\') path[index] = '/';
    index++;
  }
}

void syspath(path)
char *path;
{
  int index = 0;

  if (path[0] == '/')
  {
    path[0] = path[1];
    path[1] = ':';
    index = 2;
  }
  while (path[index])
  {
    if (path[index] == '/') path[index] = '\\';
    index++;
  }
}

BOOL isxdigit(c)
int c;
{
  c = toupper(c);
  return isdigit(c) OR ((c >= 'A') AND (c <= 'F'));
}

void memmove(d, s, n)
char *d, *s;
int n;
{
  if (d < s) while (n--) *d++ = *s++;
  else while (--n >= 0) d[n] = s[n];
}

void putmessage(s, a, b, c, d, e)
char *s;
int a, b, c, d, e;
{
  fprintf(stderr, s, a, b, c, d, e);
}

int chdir(path)
char *path;
{
  syspath(path);
  if ((strlen(path) > 1) AND (path[1] == ':'))
  {
    if (dsetdrv(toupper(path[0]) - 'A') < 0)
    {
      return -1;
    }
    path += 2;
  }
  if (dsetpath(path) < 0)
  {
    return -1;
  }
  return 0;
}

char *getcwd(path, size)
char *path;
int size;
{
  int drive = dgetdrv();

  path[0] = drive + 'a';
  path[1] = ':';
  dgetpath(path + 2, drive + 1);
  unixpath(path);
  return path;
}

int pipe(pfds)
int pfds[];
{
  if ((pfds[WRITE] = creat("c:\\tmp\\pipe", 0)) == -1)
  {
    return -1;
  }
  if ((pfds[READ] = open("c:\\tmp\\pipe", 0)) == -1)
  {
    close(pfds[WRITE]);
    return -1;
  }
  return 0;
}

BOOL isdir(direntry, path)
struct direct *direntry;
char *path;
{
  syspath(path);
  if (fattrib(path, 0, 0) & 0x10) return TRUE;
  else return FALSE;
}

BOOL isexec(direntry, path)
struct direct *direntry;
char *path;
{
  return FALSE;
}

int getpid()
{
  return 0;
}

DIR *opendir(path)
char *path;
{
  DMABUFFER *olddma;
  DMABUFFER dma;
  DIR *dir;
  struct direct *direntry;
  char filename[PATH_MAX + 1];

  DEBUG("opendir(%s)\n", path);
  if ((dir = (DIR *)malloc(sizeof(DIR))) == NULL) return NULL;
  dir->dd_loc = 0;
  dir->dd_size = 0;
  direntry = (struct direct *)&dir->dd_buf[0];
DEBUG("FGetdta()\n");
  olddma = (DMABUFFER *)fgetdta();
DEBUG("Fsetdta()\n");
  fsetdta(&dma);
DEBUG("FormFileName()\n");
  formfilename(filename, path, "*.*");
DEBUG("SysPath()\n");
  syspath(filename);
DEBUG("Fsfirst(%s)\n", filename);
  unless (fsfirst(filename, 0xff))
  {
    do 
    {
DEBUG("strcpy()\n");
      strcpy(direntry->d_name, dma.d_fname);
DEBUG("Name = %s\n", dma.d_fname);
      dir->dd_size++;
      if (dir->dd_size * sizeof(struct direct) > DIRBLKSIZE) break;
      direntry++;
DEBUG("Fsnext()\n");
    } until (fsnext());
  }
DEBUG("Fsetdta()\n");
/*
  Fsetdta(Olddma);
*/
DEBUG("done\n");
  return dir;
}

struct direct *readdir(dir)
DIR *dir;
{
  struct direct *direntry = ((struct direct *)&dir->dd_buf[0]) + dir->dd_loc;

  DEBUG("readdir()\n");
  if (dir->dd_loc++ < dir->dd_size) return direntry;
  return NULL;
}

void closedir(dir)
DIR *dir;
{
  free(dir);
}

void setenv(name, value)
char *name, *value;
{
}

void delenv(name)
char *name;
{
}

int execl(name, arg1)
char *name, *arg1;
{
  execve(name, &arg1, environ);
  exit(0);
}

char *strstr(s1, s2)
char *s1, *s2;
{
  int l1 = strlen(s1);
  int l2 = strlen(s2);

  while (l1 >= l2)
  {
    if (strncmp(s1, s2, l2) == 0) return s1;
    s1++;
    l1--;
  }
  return NULL;
}

int termgetc(fp)
FILE *fp;
{
  return crawcin();
}

void InitList(list)
LIST *list;
{
  list->head = (NODE *)&list->earth;
  list->earth = 0;
  list->tail = (NODE *)&list->head;
}

void AddHead(list, node)
LIST *list;
NODE *node;
{
  node->prev = (NODE *)&list->head;
  node->next = list->head;
  list->head = list->head->prev = node;
}

void AddTail(list, node)
LIST *list;
NODE *node;
{
  node->next = (NODE *)&list->earth;
  node->prev = list->tail;
  list->tail = list->tail->next = node;
}

void PreInsert(next, node)
NODE *next, *node;
{
  node->next = next;
  node->prev = next->prev;
  next->prev = next->prev->next = node;
}

void Remove(node)
NODE *node;
{
  node->prev->next = node->next;
  node->next->prev = node->prev;
}

NODE *RemHead(list)
LIST *list;
{
  NODE *node = list->head;

  unless (node->next) return NULL;
  list->head = node->next;
  node->next->prev = (NODE *)&list->head;
  unless (list->head->next) list->tail = (NODE *)&list->head;
  return node;
}

