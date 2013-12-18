/**
*
* Title:  Helios Shell - Amiga system dependent parts.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/amiga.c,v 1.1 1990/08/23 15:46:34 james Exp $
*
**/
#include "shell.h"

void putmessage(s, a, b, c, d, e)
char *s;
int a, b, c, d, e;
{
  fprintf(stderr, s, a, b, c, d, e);
}

char *getcwd(path, size)
char *path;
int size;
{
  getcd(0, path);
  return path;
}

int getpid()
{
  return 0;
}

DIR *opendir(path)
char *path;
{
  return NULL;
}

struct direct *readdir(dir)
DIR *dir;
{
  return NULL;
}

int stat(path, buf)
char *path;
struct stat *buf;
{
  return 0;
}

void closedir(dir)
DIR *dir;
{
}

void terminit()
{
}

void termbegin()
{
}

void termend()
{
}

int termgetc(inputfile)
FILE *inputfile;
{
}

void setenv(name, value)
char *name, *value;
{
}

void delenv(name)
char *name;
{
}

int execl(name, arg0)
char *name, *arg0;
{
  if (forkv(name, &arg0) == -1) return -1;
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

