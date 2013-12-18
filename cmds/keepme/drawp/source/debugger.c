/*------------------------------------------------------------*/
/*                                         source/debugger.c  */
/*------------------------------------------------------------*/

/* $Header: debugger.c,v 1.10 90/11/23 15:46:34 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/drawp/RCS/source/debugger.c,v $ */

/* This file contains routines to print out dsiagnostic       */
/*   information from assembled-in breakpoints in the various */
/*   assembley code parts of the drawing package.             */

/* It also contains general diagnostics-printing routines     */

/*------------------------------------------------------------*/
/*                                             Include files  */
/*------------------------------------------------------------*/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "private.h"
#include "drawp/debug.h"

/*------------------------------------------------------------*/
/*                                            Debugging flag  */
/*------------------------------------------------------------*/

/* Used to enable/disable debugging messages */
int dpDebug = 1;

/*------------------------------------------------------------*/
/*                                          dpRectDebug(...)  */
/*------------------------------------------------------------*/

extern char *dpRectanglesStart;  /* Start of rectangles code */

void dpRectDebug(BlitterControlBlock_t *bcb)
/* Called from routines in <rectangles.s>, the blitter control */
/*    block will contain the register set on breakpoint, and   */
/*    of these, PC will point to the debugging parameter and   */
/*    message.                                                 */
{  int reg,par,dig,val,cnt;
   char *ptr;
   char *data;

   if(!dpDebug) { return; }

   data = (char*)bcb->reg[15];
   data = (char*)((unsigned int)data & 0x03FFFFFC);
   par = *(int*)(data-4);

   if(par!=2)
      printf(";-------------------------------------------------------\n");
   printf("Breakpoint encountered in \"rectangles.s\" code\n");
   printf("Debugging message is : ");
   for(cnt=0,ptr=data; *ptr; ptr++)
   {  if(++cnt>100) { printf("\n(Message too long)"); break; }
      if(isprint(*ptr)) putchar(*ptr);
      else              printf("<%d>",*ptr);
   }
   printf("\n");

   if(par==2)
   {  fflush(stdout);
      return;
   }

   printf("Register set follows:\n");
   
   for(reg=0;reg<16;reg++)
   {  val = (int)bcb->reg[reg];
      if(reg<10) printf("R%d  : ",reg);
      else       printf("R%d : ",reg);
      for(dig=0;dig<8;dig++)
      {  putchar("0123456789ABCDEF"[(val>>28)&0x0F]);
         val<<=4;
      }
      printf(" , %11d  ",(int)bcb->reg[reg]);
      if(!((reg+1)%2)) putchar('\n');
   }
   
   printf("Code module start : %X\n",(int)dpRectanglesStart);
   printf("As offsets from code start, with flags masked out:\n");
   for(reg=14;reg<16;reg++)
   {  val  = (int)bcb->reg[reg] ;
      val &= 0x0FFFFFFC;
      val -= (int)dpRectanglesStart;
      if(reg<10) printf("R%d  : ",reg);
      else       printf("R%d : ",reg);
      for(dig=0;dig<8;dig++)
      {  putchar("0123456789ABCDEF"[(val>>28)&0x0F]);
         val<<=4;
      }
      printf("  ");
   }
   printf("\n");
   
   if(par==0)
   {  printf("Press RETURN:\n");
      while(getchar()!='\n');
   }

   printf("\n");

   fflush(stdout);
}

/*------------------------------------------------------------*/
/*                                          dpLineDebug(...)  */
/*------------------------------------------------------------*/

extern char *dpLinesStart;  /* Start of lines code */

void dpLineDebug(LineControlBlock_t *lcb)
/* Called from routines in <lines.s>, the line control block   */
/*    will contain the register set on breakpoint, and of      */
/*    these, PC will point to the debugging parameter and      */
/*    message.                                                 */
{  int reg,par,dig,val,cnt;
   char *ptr;
   char *data;

   if(!dpDebug) { return; }

   data = (char*)lcb->reg[15];
   data = (char*)((unsigned int)data & 0x03FFFFFC);
   par = *(int*)(data-4);

   if(par!=2)
      printf(";-------------------------------------------------------\n");
   printf("Breakpoint encountered in \"lines.s\" code\n");
   printf("Debugging message is : ");
   for(cnt=0,ptr=data; *ptr; ptr++)
   {  if(++cnt>100) { printf("\n(Message too long)"); break; }
      if(isprint(*ptr)) putchar(*ptr);
      else              printf("<%d>",*ptr);
   }
   printf("\n");

   if(par==2)
   {  fflush(stdout);
      return;
   }

   printf("Register set follows:\n");
   
   for(reg=0;reg<16;reg++)
   {  val = (int)lcb->reg[reg];
      if(reg<10) printf("R%d  : ",reg);
      else       printf("R%d : ",reg);
      for(dig=0;dig<8;dig++)
      {  putchar("0123456789ABCDEF"[(val>>28)&0x0F]);
         val<<=4;
      }
      printf(" , %11d  ",(int)lcb->reg[reg]);
      if(!((reg+1)%2)) putchar('\n');
   }
   
   printf("Code module start : %X\n",(int)dpRectanglesStart);
   printf("As offsets from code start, with flags masked out:\n");
   for(reg=14;reg<16;reg++)
   {  val  = (int)lcb->reg[reg] ;
      val &= 0x0FFFFFFC;
      val -= (int)dpLinesStart;
      if(reg<10) printf("R%d  : ",reg);
      else       printf("R%d : ",reg);
      for(dig=0;dig<8;dig++)
      {  putchar("0123456789ABCDEF"[(val>>28)&0x0F]);
         val<<=4;
      }
      printf("  ");
   }
   printf("\n");
   
   if(par==0)
   {  printf("Press RETURN:\n");
      while(getchar()!='\n');
   }

   printf("\n");

   fflush(stdout);
}

/*------------------------------------------------------------*/
/*                                    dpExamineClipList(...)  */
/*------------------------------------------------------------*/

int dpExamineClipList(DpClipNode_t *cl,FILE *fd)
/* Examine the clip-node list <cl> and validate that all it's  */
/*   internal data is correct, and send to the FILE stream     */
/*   <fd> a description of the clipping list, along with a     */
/*   description of any defects in it's structure if they are  */
/*   applicable. Flush the output stream after printing the    */
/*   diagnostics.                                              */
/* Returns a non-zero result if the clip node is in error      */
{  DpClipNode_t      *cn;
   int cnt,i,j,k,n,isErr;

   isErr = 0;
   fprintf(fd,"-------------------------------------\n");
   fprintf(fd,"Examining clip-node list at location %X\n",(int)cl);
   if(cl==DpDefaultClipping)
   {  fprintf(fd,"Clip list is <default>. Determined by context\n");
      goto EndOfDiagnostic;
   }
   for(cn=cl,cnt=0;cn;cn=cn->nextNode) cnt++;
   fprintf(fd,"There %s %d node%s on the list\n",
        (cnt==1)?"is":"are",cnt,(cnt==1)?"":"s");
   
   for(cn=cl,cnt=0;cn;cn=cn->nextNode,cnt++)
   {  fprintf(fd,"Node %d at 0x%X...\n",cnt,(int)cn);
      if(!DpNegInf(cn->node[i=0])) goto Error;
      if(!DpNegInf(cn->node[i=1])) goto Error;
      if(!DpPosInf(cn->node[i=2])) goto Error;
      if(DpVisOff(cn->node[i=1])!=2) goto Error;
      if(DpVisOff(cn->node[i=2])!=2) goto Error;
      i=3;
      if(DpNegInf(cn->node[i])||DpPosInf(cn->node[i])) goto Error;
      do
      {  if(i+1>=DpClipNodeLength)
         {  fprintf(fd,"Node overflows\n"); goto Error;  }
         if(!DpNegInf(cn->node[i+1]))
         {  fprintf(fd,"Vector -infinity not present\n"); goto Error; }
         j=i+1;
         do
         {  if(j>=DpClipNodeLength)
            {  fprintf(fd,"Vector unterminated\n"); goto Error;  }
         } while(!DpPosInf(cn->node[j++]));
         n=j-i-1;
         if(n&1) {fprintf(fd,"Odd length vector\n"); goto Error;}
         if(DpVisOff(cn->node[i+1])!=n)
         {  fprintf(fd,"Bad embedded vector length\n"); goto Error; }
         if(DpVisOff(cn->node[j-1])!=n)
         {  fprintf(fd,"Bad embedded vector length\n"); goto Error; }
         if(DpPosInf(cn->node[j]))
         {  if(n==2) break;
            fprintf(fd,"Garbage in bottom vector\n"); goto Error;
         }
         fprintf(fd,"Line record y-limits are %d .. %d\n",
                 cn->node[i],cn->node[j]);
         if(cn->node[i]>=cn->node[j])
         {  fprintf(fd,"Bad y-sequence\n"); goto Error; }
         for(k=i+2;k<j-1;k+=2)
         {  fprintf(fd,"  x region %d ... %d\n",cn->node[k],cn->node[k+1]);
            if(cn->node[k]>=cn->node[k+1])
            {  fprintf(fd,"Bad x-sequence\n"); goto Error; }
            if(k>i+1 && cn->node[k-1]>=cn->node[k])
            {  fprintf(fd,"Bad x-sequence\n"); goto Error; }
         }
         i=j; /* To next record */
      } while(1);
      if(!DpNegInf(cn->node[i+1])) { fprintf(fd,"Bad terminator\n"); goto Error; }
      if(!DpPosInf(cn->node[i+2])) { fprintf(fd,"Bad terminator\n"); goto Error; }
      if(!DpPosInf(cn->node[i+3])) { fprintf(fd,"Bad terminator\n"); goto Error; }
      if(DpVisOff(cn->node[i+1])!=2)
      {  fprintf(fd,"Bad final x-vector\n"); goto Error;  }
      if(DpVisOff(cn->node[i+2])!=2)
      {  fprintf(fd,"Bad final x-vector\n"); goto Error;  }
      if(DpVisOff(cn->node[0])!=i+4)
      {  fprintf(fd,"Bad overall length\n"); goto Error;  }
      if(DpVisOff(cn->node[i+3])!=i+4)
      {  fprintf(fd,"Bad overall length\n"); goto Error;  }
      continue; /* Next node */
      Error:
      isErr = 1;
      fprintf(fd,"Dump of clip-node. Error occured at %d = 0x%X\n",i,i);
      for(j=0;j<DpClipNodeLength;j++)
      {  if(!(j%4)) fprintf(fd,"\n0x%8X  :  ",j);
         fprintf(fd,"  %8X",cn->node[j]);
      } fprintf(fd,"\n");
   }
   EndOfDiagnostic:
   fprintf(fd,"^^^^^^^^^^^^ End of clip-node\n");
   fflush(fd);
   return isErr;
}

/*------------------------------------------------------------*/
/*                                      dpPrintRectList(...)  */
/*------------------------------------------------------------*/

void dpPrintRectList(DpRectangleList_t *rl,FILE *fd)
{  fprintf(fd,"-----------------------------\n");
   fprintf(fd,"Rectangle list reads ...\n");
   while(rl)
   {  fprintf(fd,"(%d,%d)-(%d,%d)\n",rl->lx,rl->ty,rl->rx,rl->by);
      rl=rl->next;
   }
   fprintf(fd,"^^^^^^^^^^^^ End of rect-list\n");
   fflush(fd);
}

/*---------------------------------------------------------------*/
/*                                         dpUnClippedRect(...)  */
/*---------------------------------------------------------------*/

void dpUnClippedRect(DpGraphicsContext_t *gc,
                     DpPixmap_t          *pm,
                     int                  lftX,
                     int                  topY,
                     int                  rgtX,
                     int                  botY)
{  BcbContainer_t bcbc;

   /* Decode the graphics context */
   dpDecodeGcToBcb(&bcbc.bcbProper,gc,1);

   /* Write the destination pixmap information */
   bcbc.bcbProper.desBase = (char*)pm->rawBase + 4;
   bcbc.bcbProper.desWPV  = pm->wpv;
   
   /* Validate the destination rectangle */
   if(lftX<0) lftX=0;
   if(topY<0) topY=0;
   if(rgtX>pm->sizeX) rgtX=pm->sizeX;
   if(botY>pm->sizeY) botY=pm->sizeY;

   /* Write the output rectangle co-ordinates */
   bcbc.bcbProper.desLftX = lftX;
   bcbc.bcbProper.desRgtX = rgtX;
   bcbc.bcbProper.desTopY = topY;
   bcbc.bcbProper.desBotY = botY;

   bcbc.bcbProper.ctlRtn(&bcbc.bcbProper);
}

/*---------------------------------------------------------------*/
/*                                              dpCopyArea(...)  */
/*---------------------------------------------------------------*/

void dpUnClippedCopy(  DpPixmap_t          *src,
                       DpPixmap_t          *des,
                       int srcLftX, int srcTopY,
                       int desLftX, int desTopY,
                       int blkSizX, int blkSizY,
                       DpGraphicsContext_t *gc
                    )
{  BcbContainer_t bcbc;

   dpDecodeCopyArea(&bcbc.bcbProper,src,des,desLftX-srcLftX,desTopY-srcTopY,gc);

   bcbc.bcbProper.desLftX = desLftX;
   bcbc.bcbProper.desTopY = desTopY;
   bcbc.bcbProper.desRgtX = desLftX+blkSizX;
   bcbc.bcbProper.desBotY = desTopY+blkSizY;
   
   bcbc.bcbProper.ctlRtn(&bcbc.bcbProper);
}
