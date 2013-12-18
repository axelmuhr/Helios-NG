/*------------------------------------------------------------------------
--                                                                      --
--                 H A S H   T A B L E   S U P P O R T                  --
--                 -----------------------------------                  --
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- hash.h                                                               --
--                                                                      --
--      Author:  ACE 8/3/90                                             --
--                                                                      --
------------------------------------------------------------------------*/
/* Modified form of HMS hash table code for storing handle hash tables  */

#ifndef __hash_h
#define __hash_h

#define HashTableSize   19

#define HT_HWND         0x0001
#define HT_HCONV        0x0002
#define HT_DDE_SERVER   0x0003
#define HT_DDE_SERVICE  0x0004
#define HT_HGLOBAL      0x0005
#define HT_HFONT        0x0006
#define HT_HBRUSH       0x0007
#define HT_HPEN         0x0008
#define HT_HPALETTE     0x0009
#define HT_HBITMAP      0x000a
#define HT_HMENU        0x000b
#define HT_HDATA        0x000c

DECLARE_HANDLE32(BIG_HANDLE);

typedef struct
{
  List bucket[HashTableSize];
} HashTable;

PUBLIC void InitHashTable(HashTable *);
PUBLIC void TidyHashTable(HashTable *);
PUBLIC void AddHandle(HashTable *, BIG_HANDLE, UINT, VOID FAR *);
PUBLIC VOID FAR *GetData(HashTable *, BIG_HANDLE, UINT, void FAR **);
PUBLIC BIG_HANDLE GetHandle(void FAR *);
PUBLIC BOOL HandleInTable(HashTable *, BIG_HANDLE, UINT, void FAR **);
PUBLIC void RemoveHandle(HashTable *, BIG_HANDLE, UINT);
PUBLIC void RemoveHandleApp(HashTable *, BIG_HANDLE, void FAR *);
PUBLIC void RemoveAppHandles(HashTable *, void FAR *);

#endif

/* End of hash.h */
