/*------------------------------------------------------------------------
--                                                                      --
--                 H A S H   T A B L E   S U P P O R T                  --
--                 -----------------------------------                  --
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- hash.c                                                               --
--                                                                      --
--      Author:  ACE 8/3/90                                             --
--                                                                      --
------------------------------------------------------------------------*/
/* Modified form of HMS hash table code to allow hash tables of handles */

#include "helios.h"
#include <ddeml.h>
#include "windows\hash.h"

/* Internal storage of handles in Symbol */
typedef struct
{
  Node       node;
  BIG_HANDLE handle;
  UINT       type;
  VOID FAR   *lpData;
} Symbol;


/*
*
* Hash function. For best results HashTableSize should be prime.
*
*/
PRIVATE int HashValue(BIG_HANDLE handle)
{
  return (int)((DWORD)handle % (DWORD)HashTableSize);
}

PRIVATE void Dispose(Symbol *sym)
{
    switch (sym->type)
    {
        case HT_HCONV:
            DdeDisconnect(sym->handle);
            free(sym->lpData);
            break;

        case HT_DDE_SERVICE:
            free(sym->lpData);
            break;

        case HT_HDATA:
            DdeFreeDataHandle(sym->handle);
            break;

        case HT_HGLOBAL:
            GlobalUnlock((HGLOBAL)sym->handle);
            GlobalFree((HGLOBAL)sym->handle);
            break;

        case HT_HMENU:
            DestroyMenu((HMENU)sym->handle);
            break;

        case HT_HFONT:
        case HT_HBRUSH:
        case HT_HPEN:
        case HT_HPALETTE:
        case HT_HBITMAP:
            DeleteObject((HGDIOBJ)sym->handle);
            break;
    }
}

/*
*
* Initialise a hash table.
*
*/
PUBLIC void InitHashTable(HashTable *tab)
{
  int i;

  for (i = 0; i < HashTableSize; i++) InitList(&tab->bucket[i]);
}

PUBLIC void TidyHashTable(HashTable *tab)
{
  int i;
  Symbol *sym;

  for (i=0; i<HashTableSize; i++)
  {
    sym = (Symbol *)tab->bucket[i].head;
    while (sym->node.next != (Node *)NULL)
    {
       Debug(Graphics_Flag, ("TidyHash removing type 0x%x, handle = 0x%lx", sym->type, sym->handle));
       Dispose(sym);
       free(Remove(&sym->node));
       sym = (Symbol *)tab->bucket[i].head;
    }
  }
}

PUBLIC void AddHandle(HashTable *tab, BIG_HANDLE handle, UINT type, VOID FAR *data)
{
  int h = HashValue(handle);
  Symbol *sym = (Symbol *)malloc(sizeof(Symbol));
  sym->handle = handle;
  sym->type = type;
  sym->lpData = data;

  Debug(Graphics_Flag, ("AddHandle(,0x%lx,0x%x,0x%lx)", handle, type, data));
  AddHead(&sym->node, &tab->bucket[h]);
}

PUBLIC VOID FAR *GetData(HashTable *tab, BIG_HANDLE handle, UINT type, void FAR **posn)
{
  Symbol *sym;
  int h;

  if (*posn == NULL)
  {
    h = HashValue(handle);
    for (sym = (Symbol *)tab->bucket[h].head; sym->node.next != NULL; sym = (Symbol *)sym->node.next)
    {
      if (((handle == sym->handle) || (handle == NULL)) && (type == sym->type))
      {
        *posn = (void FAR *)sym->node.next;
        return sym->lpData;
      }
    }
  }
  else
  {
    for (sym = (Symbol *)*posn; sym->node.next != NULL; sym = (Symbol *)sym->node.next)
    {
      if (((handle == sym->handle) || (handle == NULL)) && (type == sym->type))
      {
        *posn = (void FAR *)sym->node.next;
        return sym->lpData;
      }
    }
  }
  return NULL;
}

PUBLIC BIG_HANDLE GetHandle(void FAR *pSymbol)
{
  Symbol *sym = (Symbol *)pSymbol;
  return sym->handle;
}

PUBLIC BOOL HandleInTable(HashTable *tab, BIG_HANDLE handle, UINT type, void FAR **posn)
{
  Symbol *sym;
  int h;

  if (*posn == NULL)
  {
      h = HashValue(handle);
      for (sym = (Symbol *)tab->bucket[h].head; sym->node.next != NULL; sym = (Symbol *)sym->node.next)
      {
        if ((handle == sym->handle) && (type == sym->type))
        {
          *posn = (void FAR *)sym->node.next;
          return TRUE;
        }
      }
  }
  else
  {
    for (sym = (Symbol *)*posn; sym->node.next != NULL; sym = (Symbol *)sym->node.next)
    {
      if ((handle == sym->handle) && (type == sym->type))
      {
        *posn = (void FAR *)sym->node.next;
        return TRUE;
      }
    }
  }

  *posn = NULL;
  return FALSE;
}

PUBLIC void RemoveHandle(HashTable *tab, BIG_HANDLE handle, UINT type)
{
  Symbol *sym;
  int h = HashValue(handle);

  for (sym = (Symbol *)tab->bucket[h].head; sym->node.next != NULL; sym = (Symbol *)sym->node.next)
  {
    if ((handle == sym->handle) && (type == sym->type))
    {
      free(Remove(&sym->node));
      return;
    }
  }
  ServerDebug("RemoveHandle: Failed to locate object");
  return;
}

PUBLIC void RemoveHandleApp(HashTable *tab, BIG_HANDLE handle, void FAR *app)
{
  Symbol *sym;
  int h = HashValue(handle);

  for (sym = (Symbol *)tab->bucket[h].head; sym->node.next != NULL; sym = (Symbol *)sym->node.next)
  {
    if ((handle == sym->handle) && (app == sym->lpData))
    {
      free(Remove(&sym->node));
      return;
    }
  }
  ServerDebug("RemoveHandleApp: Failed to locate object");
  return;
}

PUBLIC void RemoveAppHandles(HashTable *tab, void FAR *app)
{
  Symbol *sym, *next;
  int h;

  for (h=0; h<HashTableSize; h++)
  {
    sym = (Symbol *)tab->bucket[h].head;
    while (sym->node.next != NULL)
    {
      if (app == sym->lpData)
      {
        Debug(Graphics_Flag, ("RemoveAppHandles removing type 0x%x, handle = 0x%lx", sym->type, sym->handle));
        Dispose(sym);
        next = (Symbol *)sym->node.next;
        free(Remove(&sym->node));
        sym = next;
      }
      else
        sym = (Symbol *)sym->node.next;
    }
  }
  return;
}

/* End of hash.c */
