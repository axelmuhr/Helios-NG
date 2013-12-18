                                                                                /*

  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                    (c) 1991 by parsytec GmbH, Aachen                    |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  file.c                                                                 |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    2 - O. Imbusch - 14 February 1991 - 1st extension (FOpenIfNotEx)     |
   |    1 - O. Imbusch - 24 January  1991 - Basic version (PathSplit)        |
   |                                                                         |
  []-------------------------------------------------------------------------[]

                                                                                */

#include <stdio.h>
#include <string.h>

#include "misc.h"
#include "file.h"
#include "fstring.h"


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
**                                   position n.
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
  strcpy  (Dir, "");

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

/** spec **********************************************************************/

PUBLIC STRING ConvertPath (INOUT STRING Path,
                           IN    INT    Kind)

/******************************************************************************
**
**  PURPOSE:
**    Converts a full <Path> either from DOS-Format to Helios-Format or vice
**    versa.
**
**  PARAMETERS:
**
**    In:
**      <Kind>          Kind of Conversion
**
**    InOut:
**      <Path>          Full path to be converted
**
**  RETURN:
**    <Path> is correctly converted: Address of converted <Path>
**    An error occured:              NULL
**
**  EXAMPLES:
**
**    In:
**      <Path> = "J:\INC\FILE.C"
**      <Kind> = DOStoHelios
**
**    Out:
**      <Path> = "/j/inc/file.c"
**
**    Return:
**      Address of <Path>
**
**------------------------------------------------------------------------------
**
**    In:
**      <Path> = "cc"
**      <Kind> = HeliosToDOS
**
**    Out:
**      <Path> = "cc"
**
**    Return:
**      NULL ("cc" is not a full path)
**
*** endspec *******************************************************************/

{
  if (Kind == DOStoHelios)
    if (strlen (Path) >= 3)
    {
      Replace (Path, '\\', '/');
      return (Path);
    }
    else
      return (NULL);
  else
    if (strlen (Path) >= 2)
    {
      Replace (Path, '/', '\\');
      return (Path);
    }
    else
      return (NULL);
}

/** spec **********************************************************************/

PUBLIC FILE *FOpenIfNotEx (IN STRING FileName,
			   IN STRING Mode)

/******************************************************************************
**
**  PURPOSE:
**    Opens the file <FileName> in <Mode> if it doesn't already exsist.
**
**  PARAMETERS:
**
**    In:
**      <FileName>   Name of the file to be opened
**      <Mode>       Mode of the file to be opened
**
**  RETURN:
**    Pointer to the new FILE:   File didn't exsits, it's now opened.
**    NULL:                      File did already exsists.
**
*** endspec *******************************************************************/

{
  FILE *TestFile;

  if ((TestFile = fopen (FileName, "r")) != NULL)
  {
    fclose (TestFile);
    return (NULL);
  }
  else
    return (fopen (FileName, Mode));
}
