                                                                                /*

  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                 (c) 1988, 1991 by parsytec GmbH, Aachen                 |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  copy.c								     |
   |                                                                         |
   |    Test data transfer rate of the Parsytec File Server                  |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    2 - O. Imbusch -  1 March   1991 - Update for the PFS v2.0-handbook  |
   |    1 - A. Ishan   - 12 July    1988 - Basic version                     |
   |                                                                         |
  []-------------------------------------------------------------------------[]

                                                                                */

#include <gsp.h>  
#include <limits.h>  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pathsplt.h"

#define FG_Private   0x000000F0
#define FO_ForceSync 0x00000006

#define TimeOut(S)   ((S) * OneSec)

#define CopyUsageUnknown (0)
#define CopyOK           (0)
#define CopyError        (1)

#define ReadNetto        (SrcSize / ReadCount)

#define PrintTransfer(Kind,BytesPerSec) printf ("Data %s transfer rate is %d Bytes/s.\n", Kind, BytesPerSec);

/*****************************************************************************/

int main (int    ArgC, 
          char **ArgV)	
{
  extern INT     Sync      (STRING);
  extern INT     PathSplit (STRING, STRING, STRING);
	
         Object *SrcObject, 
                *DestObject;
         Stream *SrcStream, 
                *DestStream;
         word    SrcSize,
                 Start,
                 End,
                 WrittenSize, 
                 ReadSize,
                 I,
                 ReadCount,
                 WriteCount;
         byte   *Buffer;
         char   *PathToVol,
                *PathToSrc,
	         SrcDir     [PATH_MAX],
                 SrcName    [PATH_MAX],
	        *PathToDest,
                 DestDir    [PATH_MAX],
                 DestName   [PATH_MAX];
	
/******************************************************************************
**
**  -Check command line and print usage when necessary
**
******************************************************************************/

  if (ArgC < 6) 
  {
    printf ("Usage: %s <PathToSrc> <PathToDest> <PathToVolume> <ReadCount> <WriteCount>\n", ArgV [0]);
    return (CopyUsageUnknown);
  }

  PathToSrc  = ArgV [1];
  PathToDest = ArgV [2];
  PathToVol  = ArgV [3];
  ReadCount  = atoi (ArgV [4]);
  WriteCount = atoi (ArgV [5]);

/******************************************************************************
**
**  -Extract directory and source file name from command line
**  -open source file
**
******************************************************************************/

  if (PathSplit (PathToSrc, SrcDir, SrcName) != PathSplitOK)
  {
    printf ("Unable to split %s.\n", PathToSrc);
    return (CopyError);
  }
 
  if ((SrcObject = Locate (NULL, SrcDir)) == NULL)
  {
    printf ("Unable to locate %s.\n", SrcDir);
    return (CopyError);
  }

  if ((SrcStream = Open (SrcObject, SrcName, O_ReadOnly)) == NULL)
  {
    printf ("Unable to open %s.\n", SrcName);
    return (CopyError);
  }

/******************************************************************************
**
**  -Get buffer memory
**  -Read data into Buffer while stopping the time
**  -Type transfer rate
**
******************************************************************************/

  SrcSize = GetFileSize (SrcStream);
  if ((Buffer = Malloc (ReadNetto)) == NULL)
  {
    printf ("Not enough memory for %d bytes.\n", ReadNetto);
    Close  (SrcStream);
    return (CopyError);
  }
	
  Start = clock ();
  for (I = 0; I < ReadCount; ++I)
    ReadSize = Read (SrcStream, Buffer, ReadNetto, TimeOut (30));
  End = clock ();

  if (ReadCount > 1)
    PrintTransfer ("read", (SrcSize / (End - Start)) * 100);

  Close (SrcStream);

/******************************************************************************
**
**  -Extract directory and destination file name from command line
**  -open destination file
**
******************************************************************************/

  if (PathSplit (PathToDest, DestDir, DestName) != PathSplitOK)
  {
    printf ("Unable to split %s.\n", PathToDest);
    return (CopyError);
  }
 
  if ((DestObject = Locate (NULL, DestDir)) == NULL)
  {	
    Object *ParentDir = Locate (NULL, DestDir);
    DestObject = Create (ParentDir, DestName, Type_Directory, 0, (BYTE *) (NULL));
  }

  if (DestObject == NULL)
  {
    printf ("Unable to locate %s.\n", DestDir);
    return (CopyError);
  }

  if ((DestStream = Open (DestObject, DestName, O_Create | O_WriteOnly)) == NULL)
  {
    printf ("Unable to open %s.\n", DestName);
    return (CopyError);
  }
	
/******************************************************************************
**
**  -Write data to disc (raising the file) while stopping the time
**  -Type transfer rate
**
******************************************************************************/

  Start = clock ();
  for (I = 0; I < WriteCount; ++I)
    WrittenSize = Write (DestStream, Buffer, ReadNetto, TimeOut (30));
  End = clock ();

  if (WriteCount > 1)
  {
    PrintTransfer ("alloc", (SrcSize / (End - Start)) * (100 * WriteCount / ReadCount));
    Sync (PathToVol);
  }

/******************************************************************************
**
**  -Write data to disc (using the space on disc that was allocated before)
**   while stopping the time
**  -Type transfer rate
**
******************************************************************************/

  if (Seek (DestStream, S_Beginning, 0) != 0)
  {
    printf ("Unable to reset %s.\n", DestName);
    return (CopyError);
  }

  Start = clock ();
  for (I = 0; I < WriteCount; ++I)
    WrittenSize = Write (DestStream, Buffer, ReadNetto, TimeOut (30));
  End = clock ();

  if (WriteCount > 1)
  {
    PrintTransfer ("write", (SrcSize / (End - Start)) * (100 * WriteCount / ReadCount));
    Sync (PathToVol);
  }

  Close  (DestStream);
  Free   (Buffer);
  return (CopyOK);
}

/******************************************************************************
**
**  copy.c
**
******************************************************************************/
