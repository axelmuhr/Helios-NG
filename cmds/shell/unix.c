/**
*
* Title:  Helios Shell - UNIX system dependent parts.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/unix.c,v 1.2 1990/11/23 18:07:59 martyn Exp $
*
**/
#include "shell.h"
#include <sgtty.h>
#include <sys/time.h>
#include <sys/resource.h>

RESOURCE resource[RESOURCE_MAX] =
{
  "cputime", RLIMIT_CPU,
  "filesize", RLIMIT_FSIZE,
  "datasize", RLIMIT_DATA,
  "stacksize", RLIMIT_STACK,
  "coredumpsize", RLIMIT_CORE,
  "memorysize", RLIMIT_RSS,
};

void putmessage(s, a, b, c, d, e)
char *s;
int a, b, c, d, e;
{
  fprintf(stderr, s, a, b, c, d, e);
}

void strcat(d, s)
char *d, *s;
{
  while (*d) d++;
  strcpy(d, s);
}

char *getcwd(path, size)
char *path;
int size;
{
  return getwd(path);
}

static int console;
static struct sgttyb normal, special;

void terminit()
{
  console = fileno(stdin);
  ioctl(console, TIOCGETP, &normal);
  special = normal;
  special.sg_flags &= ~ECHO;
  special.sg_flags |= CBREAK;
}

void termbegin()
{
  ioctl(console, TIOCSETP, &special);
}

void termend()
{
  ioctl(console, TIOCSETP, &normal);
}

void memmove(d, s, n)
char *d, *s;
int n;
{
  if (d < s) while (n--) *d++ = *s++;
  else while (--n >= 0) d[n] = s[n]; 
}

void putlimit(number)
int number;
{
  struct rlimit rlp;

  getrlimit(resource[number].value, &rlp);
  printf("%s\t", resource[number].name);
  if (rlp.rlim_max == RLIM_INFINITY) printf("unlimited\n");
  else printf("%d kbytes\n", rlp.rlim_max / 1024);
}

void setlimit(number, limit)
int number, limit;
{
  struct rlimit rlp;

  getrlimit(resource[number].value, &rlp);
  rlp.rlim_max = (limit == -1) ? RLIM_INFINITY : limit * 1024;
  setrlimit(resource[number].value, &rlp);
}
  

int findresource(name)
char *name;
{
  int i, number;
  int count = 0;
  int length = strlen(name);

  for (i = 0; i < RESOURCE_MAX; i++)
  {
    if (strnequ(name, resource[i].name, length))
    {
      count++;
      number = i;
    }
  }
  unless (count == 1) return -1;
  return number;
}

BOOL isdir(dirent, path)
DIRENT *dirent;
char *path;
{
  struct stat buf;
  char filename[MAX_PATH + 1];

  formfilename(filename, path, dirent->d_name);
  if (stat(filename, &buf) == -1) return FALSE;
  if (buf.st_mode & S_IFDIR) return TRUE;
  return FALSE;
}

BOOL isexec(dirent, path)
DIRENT *dirent;
char *path;
{
  char filename[MAX_PATH + 1];

  formfilename(filename, path, dirent->d_name);
  if (access(filename, X_OK) == -1) return FALSE;
  return TRUE;
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
