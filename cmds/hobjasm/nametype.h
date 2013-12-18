/* -> nametype/h
 * Title:               Name handling
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */

#ifndef nametypes_h
#define nametypes_h

#include "constant.h"

typedef struct StringName {
  CARDINAL length,
           maxLength;
  char    *key;
} StringName;

typedef StringName *NamePointer;

typedef struct Name {
  CARDINAL length;
  char    *key;
} Name;

BOOLEAN NameLookup(
  Name     *nameTable,
  Name      name,
  BOOLEAN   abbreviate,
  CARDINAL *index,
  CARDINAL  length);

void CopyName(CARDINAL number, char *value, Name *table);
/*Set up a command name table element*/

#endif

/* End nametype/h */
