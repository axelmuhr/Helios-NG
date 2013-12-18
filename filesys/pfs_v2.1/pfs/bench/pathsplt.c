                                                                                /*

  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                    (c) 1991 by parsytec GmbH, Aachen                    |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  pathsplt.c                                                             |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    1 - O. Imbusch - 1 March 1991 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]

                                                                                */
#include <helios.h>
#include <stdio.h>
#include <string.h>

#include "misc.h"
#include "pathsplt.h"

/** spec **********************************************************************/

PUBLIC INT PathSplit (IN  STRING Path,
                      OUT STRING Dir,
                      OUT STRING Name)

/******************************************************************************
**
**  PURPOSE:
**    Splits a full <Path> into its components, <Dir> and <Name>.
** 
**  PARAMETERS:
**
**    In:
**      <Path>       Full path to be splitted
**    Out:
**      <Dir>        Directory part of <Path>
**      <Name>       Name part of <Path> 
**
**  RETURN:
**    PathSplitOK:                   No error occured, no illegal components
**                                   (like non-printable charcaters) in <Path>.
**    0 <= n <= strlen (<Path>) - 1: An illegal character occured in <Path> at
**                                   position n (not yet implemented).
**
**  EXAMPLES:
**
**    In:
**      <Path> = "/helios/bin/make"
** 
**    Out:
**      <Dir>  = "/helios/bin"
**      <Name> = "make"            (NOTE: We lost the separating slash ('/').)
**
**    Return:
**      PathSplitOK
**
**-----------------------------------------------------------------------------
**
**    In:
**      <Path> = "/cshrc"
** 
**    Out:
**      <Dir>  = "/"
**      <Name> = "cshrc"            (NOTE: '/' is the root directory, so the 
**                                         last (and only) slash is not obso-
**                                         lete.)
**
**    Return:
**      PathSplitOK
**
*** endspec *******************************************************************/

{

/*******************************************************************************
**
**  -Copy <Path> (the characters from the last occurence of '/' to the end of
**   the string) to <Name>
**  -Did '/' occur?
**     no:  -leave <Dir> at '' and and copy <Path> to <Name>
**     yes: -Remove the leading '/' from Name
**          -Copy all of <Path> but the <Name>-part to <Dir>
**           (NOTE: strncpy does not produce the null terminating character,
**           so we use clearing <Dir> with following appending via strncat to
**           solve the problem that occurs when <Dir> contains more character
**           at the start of SplitPath than it should have at its end.)
**  -Is <Dir> the root directory ('/')?
**     no: remove the last '/' from <Dir>
**
*******************************************************************************/

  char *LastSlash = strrchr (Path, '/');
  *Dir = '\0';

  if (LastSlash == NULL)
    strcpy (Name, Path);
  else
  {
    strcpy  (Name, LastSlash + 1); 
    strncat (Dir, Path, strlen (Path) - strlen (Name));
  }  
  
  if (strcmp (Dir, "/") != 0)
    *(Dir + strlen (Dir) - 1) = '\0';
    
  return (PathSplitOK);
}

/******************************************************************************* 
**
**  pathsplt.c
**
*******************************************************************************/
