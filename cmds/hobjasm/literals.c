/* -> literals/c
 * Title:               Handle literal operands
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "code.h"
#include "globvars.h"
#include "literals.h"
#include "asmvars.h"
#include "store.h"
#include "tables.h"
#include "stubs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

LiteralPointer      literalPointer = NULL,
                    currentLiteralPointer;
LiteralBlockPointer literalBlockPointer = NULL,
                    currentBlockPointer = NULL;
CARDINAL            literalAddress;

/*---------------------------------------------------------------------------*/

void LiteralAsmStart(void)
{
  LiteralPointer      tempLP;
  LiteralBlockPointer tempLBP;

  while (literalPointer != NULL) {
    tempLP = literalPointer->link;
    free(literalPointer);
    literalPointer = tempLP;
  };
  while (literalBlockPointer != NULL) {
    tempLBP = literalBlockPointer->link;
    free(literalBlockPointer);
    literalBlockPointer = tempLBP;
  };
  currentBlockPointer = NULL;
  currentLiteralPointer = NULL;
} /* End LiteralAsmStart */

/*---------------------------------------------------------------------------*/

void LiteralFileStart(void)
{
  LiteralPointer tempLP;

  switch (pass) {
    case 1:
    if (literalBlockPointer == NULL) {
      /*This is the first file*/
      literalBlockPointer = mymalloc(sizeof(*literalBlockPointer));
      currentBlockPointer = literalBlockPointer;
    } else {
      /*This is a subsequent file*/
      currentBlockPointer->link = mymalloc(sizeof(*currentBlockPointer));
      currentBlockPointer = currentBlockPointer->link;
    };
    currentBlockPointer->link = NULL;
    currentBlockPointer->size = 0;
    literalAddress = 0;/*A safe temporary value*/
    break;

    case 2:
    currentBlockPointer = (currentBlockPointer->link == NULL)
      /*This is the first file in the pass*/
      ? literalBlockPointer
      /*This is a subsequent file*/
      : currentBlockPointer->link;
    literalAddress = currentBlockPointer->address;
  };
  /*Now deallocate all literals of the previous file*/
  while (literalPointer != NULL) {
    tempLP = literalPointer->link;
    free(literalPointer);
    literalPointer = tempLP;
  };
  currentLiteralPointer = NULL;
} /* End LiteralFileStart */

/*---------------------------------------------------------------------------*/

void LiteralFileEnd(void)
{
 CARDINAL       i ;
 LiteralPointer tempLP ;

 switch (pass)
  {
   case 1 : while ((programCounter % 4) != 0)
             programCounter++ ;
            if (currentBlockPointer->size != 0)
             {
              currentBlockPointer->address = programCounter ;
              programCounter += currentBlockPointer->size ;
              tempLP = literalPointer ;
              while (tempLP != NULL)
               {
                if (!tempLP->adrSet)
                 {
                  tempLP->address = currentBlockPointer->address ;
                  tempLP->adrSet = TRUE ;
                 }
                tempLP = tempLP->link ;
               }
             }
            break ;

   case 2 : /* Now output all existing literals */
            if (currentBlockPointer->size != 0)
             {
              while ((programCounter % 4) != 0)
               CodeByte(0) ;
              i = 0 ;
              tempLP = currentLiteralPointer ;
              while (tempLP != NULL)
               {
                switch (tempLP->type)
                 {
                  case ConstantLT :
                  case FPSingleLT :
                                    CodeWord(tempLP->u.constant) ;
                                    i += 4 ;
                                    break ;

                  case AddressLT  : printf("LiteralFileEnd: Helios output to be done\n") ;
                                    CodeWord(tempLP->u.offset) ;
                                    i += 4 ;
                                    break ;

                  case ExternalLT : {
                                     CARDINAL value = tempLP->u.ext.offset ;
                                     printf("LiteralFileEnd: Helios output to be done\n") ;
                                     CodeWord(value) ;
                                    }
                                    i += 4 ;
                                    break ;

                  case FPDoubleLT : CodeWord(tempLP->u.fpdouble.value1) ;
                                    CodeWord(tempLP->u.fpdouble.value2) ;
                                    i += 8 ;
                 } /* switch */
        tempLP = tempLP->link;
      }; /* while */
      while (i < currentBlockPointer->size) {
        CodeWord(0);
        i += 4;
      }; /* while */

    }; /* if */
  }; /* case */
} /* End LiteralFileEnd */

/*---------------------------------------------------------------------------*/

CARDINAL AddLiteral(BOOLEAN status,CARDINAL value)
{
 LiteralPointer tempLP ;

 if (!status)
  {
   /* Must be pass 1 */
   currentBlockPointer->size += 4 ; /* Allow another slot for it */
   literalAddress += 4 ;
   return(literalAddress - 4) ;
  }

 if (literalPointer == NULL)
  {
   literalPointer = mymalloc(sizeof(*literalPointer)) ;
   currentLiteralPointer = literalPointer ;

   literalPointer->type = ConstantLT ;
   literalPointer->u.constant = value ;
   literalPointer->link = NULL ;
   literalPointer->address = literalAddress ;
   literalPointer->adrSet = pass == 2 ;
   if (pass == 1)
    currentBlockPointer->size += 4 ; /* Allow another slot for it */
   literalAddress += 4 ;

   return(literalAddress - 4) ;
  } ; /* if */

 tempLP = literalPointer ; /* Start of the chain */
 do
  {
   if ((tempLP->type == ConstantLT) && (tempLP->u.constant == value) && (!tempLP->adrSet || (abs((int)(tempLP->address - programCounter - 8)) < 0x1000)))
    {
     /* I.e. return if address known and in range, or symbol in next pool */
     return(tempLP->address) ;
    }

   if (tempLP->link == NULL)
    {
     tempLP->link = mymalloc(sizeof(*tempLP)) ;
     tempLP = tempLP->link ;
     if (currentLiteralPointer == NULL)
      currentLiteralPointer = tempLP ;
     tempLP->type = ConstantLT ;
     tempLP->u.constant = value ;
     tempLP->link = NULL ;
     tempLP->address = literalAddress ;
     tempLP->adrSet = pass == 2 ;
     if (pass == 1)
      currentBlockPointer->size += 4 ;
     /* Allow another slot for it */
     literalAddress += 4 ;
     return literalAddress-4 ;
    } ;
   tempLP = tempLP->link ;
  } while (1) ; /* loop */
} /* End AddLiteral */

/*---------------------------------------------------------------------------*/

CARDINAL AddExternalLiteral(SymbolPointer symbolPointer, CARDINAL value)
{
 LiteralPointer tempLP ;

  if (literalPointer == NULL) {
    literalPointer = mymalloc(sizeof(*literalPointer));
    currentLiteralPointer = literalPointer;
    literalPointer->type = ExternalLT;
    literalPointer->u.ext.symbolId = symbolPointer->aOFData.symbolId;
    literalPointer->u.ext.offset = value;
    literalPointer->u.ext.symbolPointer = symbolPointer;
    literalPointer->link = NULL;
    literalPointer->address = literalAddress;
    literalPointer->adrSet = pass == 2;
    if (pass == 1) currentBlockPointer->size += 4;/*Allow another slot for it*/
    literalAddress += 4;
    return literalAddress-4;
  }; /* if */
  tempLP = literalPointer;/*Start of the chain*/
  do {
    if ((tempLP->type == ExternalLT) &&
      (tempLP->u.ext.symbolId == symbolPointer->aOFData.symbolId) &&
      (tempLP->u.ext.offset == value) &&
      (!tempLP->adrSet
         || (abs((int)(tempLP->address - programCounter - 8)) < 0x1000)))
      return tempLP->address;
      /*I.e. return if address known and in range, or symbol in next pool*/
    if (tempLP->link == NULL) {
      tempLP->link = mymalloc(sizeof(*tempLP));
      tempLP = tempLP->link;
      if (currentLiteralPointer == NULL) currentLiteralPointer = tempLP;
      tempLP->type = ExternalLT;
      tempLP->u.ext.symbolId = symbolPointer->aOFData.symbolId;
      tempLP->u.ext.offset = value;
      tempLP->u.ext.symbolPointer = symbolPointer;
      tempLP->link = NULL;
      tempLP->address = literalAddress;
      tempLP->adrSet = pass == 2;
      if (pass == 1) currentBlockPointer->size += 4;
        /*Allow another slot for it*/
      literalAddress += 4;
      return literalAddress-4;
    };
    tempLP = tempLP->link;
  } while (1); /* loop */
} /* End AddExternalLiteral */

/*---------------------------------------------------------------------------*/

CARDINAL AddAddressLiteral(BOOLEAN status, CARDINAL value)
{
  LiteralPointer tempLP;

  if (!status) return AddLiteral(status, value);
  if (literalPointer == NULL) {
    literalPointer = mymalloc(sizeof(*literalPointer));
    currentLiteralPointer = literalPointer;
    literalPointer->type = AddressLT;
    literalPointer->u.offset = value;
    literalPointer->link = NULL;
    literalPointer->address = literalAddress;
    literalPointer->adrSet = pass == 2;
    if (pass == 1) currentBlockPointer->size += 4;/*Allow another slot for it*/
    literalAddress += 4;
    return literalAddress-4;
  }; /* if */
  tempLP = literalPointer;/*Start of the chain*/
  do {
    if ((tempLP->type == AddressLT) &&
      (tempLP->u.offset == value) &&
      (!tempLP->adrSet
         || (abs((int)(tempLP->address - programCounter - 8)) < 0x1000)))
      return tempLP->address;
      /*I.e. return if address known and in range, or symbol in next pool*/
    if (tempLP->link == NULL) {
      tempLP->link = mymalloc(sizeof(*tempLP));
      tempLP = tempLP->link;
      if (currentLiteralPointer == NULL) currentLiteralPointer = tempLP;
      tempLP->type = AddressLT;
      tempLP->u.offset = value;
      tempLP->link = NULL;
      tempLP->address = literalAddress;
      tempLP->adrSet = pass == 2;
      if (pass == 1) currentBlockPointer->size += 4;
        /*Allow another slot for it*/
      literalAddress += 4;
      return literalAddress-4;
    };
    tempLP = tempLP->link;
  } while (1); /* loop */
} /* End AddAddressLiteral */

/*---------------------------------------------------------------------------*/

CARDINAL AddFPLiteralSingle(CARDINAL value)
{
  LiteralPointer tempLP;

  if (literalPointer == NULL) {
    literalPointer = mymalloc(sizeof(*literalPointer));
    currentLiteralPointer = literalPointer;
    literalPointer->type = FPSingleLT;
    literalPointer->u.constant = value;
    literalPointer->link = NULL;
    literalPointer->address = literalAddress;
    literalPointer->adrSet = pass == 2;
    if (pass == 1) currentBlockPointer->size += 4;/*Allow another slot for it*/
    literalAddress += 4;
    return literalAddress-4;
  }; /* if */
  tempLP = literalPointer;/*Start of the chain*/
  do {
    if ((tempLP->type == FPSingleLT) && (tempLP->u.constant == value) &&
      (!tempLP->adrSet
         || (abs((int)(tempLP->address - programCounter - 8)) < 0x400)))
      return tempLP->address;
      /*I.e. return if address known and in range, or symbol in next pool*/
    if (tempLP->link == NULL) {
      tempLP->link = mymalloc(sizeof(*tempLP));
      tempLP = tempLP->link;
      if (currentLiteralPointer == NULL) currentLiteralPointer = tempLP;
      tempLP->type = FPSingleLT;
      tempLP->u.constant = value;
      tempLP->link = NULL;
      tempLP->address = literalAddress;
      tempLP->adrSet = pass == 2;
      if (pass == 1) currentBlockPointer->size += 4;
        /*Allow another slot for it*/
      literalAddress += 4;
      return literalAddress-4;
    };
    tempLP = tempLP->link;
  } while (1); /* loop */
} /* End AddFPLiteralSingle */

/*---------------------------------------------------------------------------*/

CARDINAL AddFPLiteralDouble(CARDINAL value1, CARDINAL 
         value2)
{
  LiteralPointer tempLP;

  if (literalPointer == NULL) {
    literalPointer = mymalloc(sizeof(*literalPointer));
    currentLiteralPointer = literalPointer;
    literalPointer->type = FPDoubleLT;
    literalPointer->u.fpdouble.value1 = value1;
    literalPointer->u.fpdouble.value2 = value2;
    literalPointer->link = NULL;
    literalPointer->address = literalAddress;
    literalPointer->adrSet = pass == 2;
    if (pass == 1) currentBlockPointer->size += 8;/*Allow another slot for it*/
    literalAddress += 8;
    return literalAddress-8;
  }; /* if */
  tempLP = literalPointer;/*Start of the chain*/
  do {
    if ((tempLP->type == FPDoubleLT) &&
      (tempLP->u.fpdouble.value1 == value1) &&
      (tempLP->u.fpdouble.value2 == value2) &&
      (!tempLP->adrSet
         || (abs((int)(tempLP->address - programCounter - 8)) < 0x400)))
      return tempLP->address;
      /*I.e. return if address known and in range, or symbol in next pool*/
    if (tempLP->link == NULL) {
      tempLP->link = mymalloc(sizeof(*tempLP));
      tempLP = tempLP->link;
      if (currentLiteralPointer == NULL) currentLiteralPointer = tempLP;
      tempLP->type = FPDoubleLT;
      tempLP->u.fpdouble.value1 = value1;
      tempLP->u.fpdouble.value2 = value2;
      tempLP->link = NULL;
      tempLP->address = literalAddress;
      tempLP->adrSet = pass == 2;
      if (pass == 1) currentBlockPointer->size += 8;
        /*Allow another slot for it*/
      literalAddress += 8;
      return literalAddress-8;
    };
    tempLP = tempLP->link;
  } while (1); /* loop */
} /* End AddFPLiteralDouble */

/*---------------------------------------------------------------------------*/
/* Define a literal origin at the current PC */
void LiteralOrg(void)
{
 LiteralFileEnd() ;
 StubFileEnd() ;
 currentLiteralPointer = NULL ;
 switch (pass)
  {
   case 1 : currentBlockPointer->link = mymalloc(sizeof(*currentBlockPointer)) ;
            currentBlockPointer = currentBlockPointer->link ;
            currentBlockPointer->size = 0 ;
            currentBlockPointer->link = NULL ;
            literalAddress = 0 ; /* A safe temporary value */
            break ;

   case 2 : currentBlockPointer = currentBlockPointer->link ;
            literalAddress = currentBlockPointer->address ;
  }
} /* End LiteralOrg */

/*---------------------------------------------------------------------------*/
/* EOF literals/c */
