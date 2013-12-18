                                                                                /*

  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                 (c) 1988, 1991 by parsytec GmbH, Aachen                 |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  create.c								     |
   |                                                                         |
   |    Create file to be used by copy.c                                     |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    2 - O. Imbusch -  1 March   1991 - Update for the PFS v2.0-handbook  |
   |    1 - A. Ishan   - 12 July    1988 - Basic version                     |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */ 

#include <helios.h>
#include <stdio.h>
#include <syslib.h>
#include <stdlib.h>
#include <string.h>
#include <gsp.h>

#define CreateUsageUnknown (0)
#define CreateOK           (0)
#define CreateError        (1)


int main (int    ArgC, 
          char **ArgV)	
{
        Object *ContextObject;
	Stream *FileStream;
	word    FileSize,
                WriteSize,
                I;
	byte   *Buffer;
        STRING  ContextDir,
	        FileName;

/*******************************************************************************
**
**  -Check command line and print usage when necessary
**
*******************************************************************************/

	if (ArgC < 4) 
	{
          printf ("Usage: %s <ContextDir> <FileName> <FileSize>.\n", ArgV [0]);
          return (CreateUsageUnknown);
	}

        ContextDir = ArgV [1];
	FileName   = ArgV [2];
        FileSize   = atoi (ArgV [3]);

/*******************************************************************************
**
**  -Get buffer memory
**  -Fill the buffer with byte sequences 0..255
**
*******************************************************************************/

        if ((Buffer = Malloc (FileSize)) == NULL)
	{
          printf ("Not enough memory for %d bytes.\n", FileSize);
          return (CreateError);
	}

	for (I = 0; I < FileSize; ++I)
          Buffer [I] = I % 256;
		
/*******************************************************************************
**
**  -Open destination file
**  -Write data to disc
**  -Close file and release buffer
**
*******************************************************************************/

	if ((ContextObject = Locate (NULL, ContextDir)) == NULL)
	{
          printf ("Unable to locate %s.\n", ContextDir);
          return (CreateError);
	}

	if ((FileStream = Open (ContextObject, FileName, O_Create | O_WriteOnly)) == NULL)
	{
          printf ("Unable to open %s.\n", FileName);
          return (CreateError);
	}
	
	if ((WriteSize = Write (FileStream, Buffer, FileSize, 30 * OneSec)) != FileSize)
	{
          printf ("Unable to write %d bytes.\n", FileSize);
          return (CreateError);
	}

	Close (FileStream);
	Free  (Buffer);

	return (CreateOK);
}

/*******************************************************************************
**
**  create.c
**
*******************************************************************************/
