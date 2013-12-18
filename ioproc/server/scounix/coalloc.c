/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1990, Bleistein Rohde Sytemtechnik GmbH    --
--                        All Rights Reserved.                          --
--                                                                      --
--      coalloc.c                                                       --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90\ Copyright (C) 1990 Bleistein Rohde Sytemtechnik GmbH */

/**
*** This is supplementary Code for implementation  of coroutines. 
*** It is used inside the I/O Server.
**/

#include "../helios.h"

#define	COSTACKSIZE 8*1024

extern int topofstack;
extern int bottomofstack;

typedef struct { Node  node;
                 char *stack;
		 int   size;
} NOFreelist;

struct Freelist
{
  char *stack;
  struct Freelist *next;
};
static struct Freelist *fp=NULL;

static void append();
static char *delete();
static int isempty();

static int isempty()
{
  if (fp == NULL)
    {
      return (1); 
    }

  return (0);
}

static void append(s)
char *s;
{
  struct Freelist *p;

  if (fp == NULL)
    {
      fp=(struct Freelist *)malloc(sizeof(struct Freelist));
    
      fp->stack=s;
      fp->next=NULL;
    
      return;
    }

  for (p=fp; p != NULL; p=p->next)
    {
      if (p->stack == s)	/* STRANGE! element in freelist yet */
	{
	  return;
	}

      /* appending now */
    
      if (p->next == NULL)
	{
	  p->next       =(struct Freelist *)malloc(sizeof(struct Freelist));      
	  (p->next)->stack=s;
	  (p->next)->next=NULL;

	  break;
	}

    }

  return;
}

static char *delete()
{
  struct Freelist *p;
  char *ps;
  
  if (fp == NULL)	/* STRANGE! freelist does not exist */
    {
      return ((char *)0);
    }

  ps=fp->stack;

  if (fp->next == NULL)
    {
      free(fp);
      fp=NULL;
    }
  else
    {
      free(fp->next);
      fp->next=(fp->next)->next;      
    }
  
  return (ps);
}


/***********************************************/

char *cosalloc(size)
unsigned int size;
{
  char *f;

/* ServerDebug ("cosalloc (%d (0x%lx))", size, size); */
  
  if (size >=sizeof(int) && size <= COSTACKSIZE)
    {
      if (isempty()) /* not found in freelist */
	{
	  bottomofstack-=COSTACKSIZE;

	  return((char *)bottomofstack);
	}
      else
	{
	  /* delete in freelist */

	  return(delete());
	}
    
    }

/* ServerDebug ("cosalloc () - returning 0"); */

  return ((char *)0);
}

/***********************************************/

void cosfree(p)
char *p;
{
  append(p);	/* insert in freelist */

  return;
}

/***********************************************/
