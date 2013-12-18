                                                                                /*
  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                    (c) 1991 by parsytec GmbH, Aachen                    |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |                          Parsytec File System                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  partchck.c							     |
   |                                                                         |
   |    Test global partition list for overlapping partitions and twice      |
   |    allocated partitions.                                                |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    3 - O.Imbusch - 30 April 1991 - Error handling centralized           |
   |    2 - O.Imbusch -  2 April 1991 - Double allocation extension          |
   |    1 - O.Imbusch - 25 March 1991 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#include <device.h>
#include <helios.h>

#define DEBUG      0
#define GEPDEBUG   0
#define FLDEBUG    0
#define IN_NUCLEUS 1

#include "error.h"
#include "fserr.h"
#include "misc.h"
#include "nfs.h"

#define OpenEnd (-1)


/** spec **********************************************************************/

PRIVATE word NOfPhysPart (IN DiscDevInfo *DDI)

/******************************************************************************
**
**  PURPOSE:
**    Calculates the number of physical partitions based on the informations
**    in <DDI>.
**
**  PARAMETERS:
**
**    In:
**      <DDI>    Devinfo's Disc Device Information
**
**  RETURN:
**    Number of physical partitions.
**
*** endspec *******************************************************************/

{
        word           Result;
        PartitionInfo *Act;

/******************************************************************************
**
**  sorry, but RTOA-macro requires
**
******************************************************************************/

#define                Parts  DDI->Partitions

  if (Parts == -1)
    Result = 0;
  else
  {
    Result = 1;
    Act = (PartitionInfo *) (RTOA (Parts));
    while (Act->Next != -1)
    {
      ++Result;
      Act = (PartitionInfo *) (RTOA (Act->Next));
    }
  }
  return (Result);

#undef Parts

}	



/** spec **********************************************************************/

PRIVATE bool PartOverlap (IN word Start1,
                          IN word End1,
                          IN word Start2,
                          IN word End2)

/******************************************************************************
**
**  PURPOSE:
**    Checks wether the partiton starting at <Start1> and ending at <End1>
**    overlaps the partition specified by <Start2> and <End2> (assuming that
**    they claim the same drive).
** 
**  PARAMETERS:
**
**    In:
**      <Start1>     First block of first partition
**      <End1>       Last block of first partition
**      <Start2>     First block of second partition
**      <End2>       Last block of second partition
**
**  RETURN:
**    TRUE:          The partitions overlap each other.
**    FALSE:         They don't.
**
**  EXAMPLES:
**
**    In:
**      <Start1> =   4711
**      <End1>   =   OpenEnd
**      <Start2> =   815
**      <End2>   =   6060842
**
**    Return:
**      TRUE
**
**-----------------------------------------------------------------------------
**
**    In:
**      <Start1> =   0
**      <End1>   =   19999
**      <Start2> =   20000
**      <End2>   =   39999
**
**    Return:
**      FALSE
**
*** endspec *******************************************************************/

{

FLdebug ("S1 (%d)   E1 (%d)\n"
         "S2 (%d)   E2 (%d)", Start1, End1, Start2, End2);	

  return (   ((Start1 >= Start2) && ((Start1 <= End2) || (End2 == OpenEnd)))
          || ((Start2 >= Start1) && ((Start2 <= End1) || (End1 == OpenEnd))));
}



/** spec **********************************************************************/

PRIVATE bool AlreadyOcc (IN DiscDevInfo   *DDI,
	                 IN PartitionInfo *Act,
	                 IN word           ActNr)

/******************************************************************************
**
**  PURPOSE:
**    Checks wether blocks of partition <Act> (with number <ActNr>) are
**    already occupied by any other partition in <DDI>.
**
**  PARAMETERS:
**
**    In:
**      <DDI>       Devinfo's Disc Device Information
**      <Act>       Partiton to be checked
**      <ActNr>     Number of the partiton (in devinfo's partition list) to be
**                  checked
**
**  RETURN:
**    TRUE:         There are blocks already claimed by other partitions
**    FLASE:        <Act> is the first partition that demands its blocks.
**
*** endspec *******************************************************************/

{
        PartitionInfo *Test;
        word           NOfPart = NOfPhysPart (DDI),
                       TestNr;

/******************************************************************************
**
**  sorry, but RTOA-macro requires
**
******************************************************************************/

#define Parts          DDI->Partitions 

  for (TestNr = 0, Test = (PartitionInfo *) (RTOA (Parts)); 
       TestNr < NOfPart; 
       ++TestNr,   Test = (PartitionInfo *) (RTOA (Test->Next)))
  {

FLdebug ("  with partition #%d", TestNr);

    if (   (Test        != Act)
        && (Test->Drive == Act->Drive) 
        && PartOverlap (Test->StartCyl, Test->EndCyl, Act->StartCyl, Act->EndCyl))
    {    
      Error (FSErr [Overlap], ActNr, TestNr);
      return (TRUE);
    }  
  }

  return (FALSE);

#undef Parts
  
}			   



/** spec **********************************************************************/

PUBLIC bool IllegalPartitioning (IN DiscDevInfo *DDI)

/******************************************************************************
**
**  PURPOSE:
**    Checks wether there are blocks requested by more than one partition.
**
**  PARAMETERS:
**
**    In:
**      <DDI>   Devinfo's Disc Device Information
**
**  RETURN:
**    TRUE:     There's an error in devinfo, blocks are double allocated.
**    FALSE:    OK, no block is allocated twice.
**
*** endspec *******************************************************************/

{
  PartitionInfo *Act;
  word           ActNr,
                 NOfParts = NOfPhysPart (DDI);
  
  if (NOfParts > 0)
  {
    for (ActNr = 0, Act = (PartitionInfo *) (RTOA (DDI->Partitions)); 
         ActNr < NOfParts; 
         ++ActNr,   Act = (PartitionInfo *) (RTOA (Act->Next)))
    {     

FLdebug ("Checking partition #%d", ActNr);

      if (AlreadyOcc (DDI, Act, ActNr))
        return (TRUE);
    }      
    return (FALSE);
  }
  else
  {
    Error (FSErr [MinOne]);
    return (TRUE);
  }    
}



/** spec **********************************************************************/

PUBLIC bool IllegalAllocParts (IN DiscDevInfo *DDI)

/******************************************************************************
**
**  PURPOSE:
**    Checks wether partitions are claimed by two volumes.
** 
**  PARAMETERS:
**
**    In:
**      <DDI>   Devinfo's Disc Device Information
**
**  RETURN:
**    TRUE:     There's an error in devinfo, a partition is double allocated.
**    FALSE:    OK, no partiton is allocated twice.
**
*** endspec *******************************************************************/

{
  word           ActPartNr,
                 ActVolNr,
  		 PartNrInVol,
                 NOfParts = NOfPhysPart (DDI);
  bool          *PartIsAlloc,
                 Result;
  
  if (NOfParts > 0)
  {
    PartIsAlloc = PolyNew (bool, NOfParts);
    for (ActPartNr = 0; ActPartNr < NOfParts; ++ActPartNr)
      PartIsAlloc [ActPartNr] = FALSE;
    Result = FALSE;  
      
    for (ActVolNr = 0; ActVolNr < volume [0].num_of_vols; ++ActVolNr)
      for (PartNrInVol = 0; PartNrInVol < volume [ActVolNr].num_of_parts; ++ PartNrInVol)
        if (   (volume [ActVolNr].logic_partition [PartNrInVol].partnum >= NOfParts)
            || (volume [ActVolNr].logic_partition [PartNrInVol].partnum < 0))
        {
          Error (FSErr [IllegalNr], volume [ActVolNr].logic_partition [PartNrInVol].partnum);
          Result = TRUE;
        }
        elif (PartIsAlloc [volume [ActVolNr].logic_partition [PartNrInVol].partnum])
        {
          Error (FSErr [AllocTwice], volume [ActVolNr].logic_partition [PartNrInVol].partnum);
          Result = TRUE;
        }
        else
          PartIsAlloc [volume [ActVolNr].logic_partition [PartNrInVol].partnum] = TRUE;

    Free (PartIsAlloc);
    return (Result);
  }
  else
  {
    Error (FSErr [MinOne]);
    return (TRUE);
  }    
}

/*******************************************************************************
**
**  partchck.c
**
*******************************************************************************/
