/*-----------------------------------------------------------*/
/*                                     source/test/test1C.c  */
/*-----------------------------------------------------------*/

/* $Header: test1C.c,v 1.2 90/02/01 20:55:31 charles Locked $ */
/* $Source: /server/usr/users/charles/world/drawp/RCS/source/test/test1C.c,v $ */

/* This file is to be used for testing the clipping-region   */
/*   processing algorithms by provideing them with random    */
/*   data, and validating the resultant regions.             */

/*-----------------------------------------------------------*/
/*                                            Include files  */
/*-----------------------------------------------------------*/

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "math.h"
#include "drawp/public.h"
#include "drawp/debug.h"

/*-----------------------------------------------------------*/
/*                                                   main()  */
/*-----------------------------------------------------------*/

#define MaxNr     50
#define MinNr      5
#define MaxMatrix 20
#define MinMatrix  5
#define MaxSq      10
#define MaxLeeW    5
#define MinLeeW    0

int matrix;

int randN(int);
int testReduce(DpClipNode_t *cl,DpRectangleList_t *rl);
int testIntersect(DpClipNode_t *cl,DpRectangleList_t *rl);
int main(int argc,char **argv)
{  DpClipNode_t *cn;
   DpRectangleList_t rct[MaxNr];
   int nr,i,ms,leeW,testN,errs,maxErrs,isolTest;
   FILE *nl;
   
   nl=fopen("/dev/null","wb");

   if(argc!=3&&argc!=4)
   {  fprintf(stderr,"Format : out/test1C <seed> <max errors> {<isolate test>\n");
      fprintf(stderr,"seed       : Random seed\n");
      fprintf(stderr,"max errors : Max # errors to do\n");
      fprintf(stderr,"skip       : Skip to n'th test and only do that\n");
      return 1;
   }
   maxErrs = atoi(argv[2]);
   if(argc==4) isolTest=atoi(argv[3]); else isolTest=0;
   if(isolTest>0) fprintf(stderr,"Isolating test number %d\n",isolTest);
   
   srand(atoi(argv[1]));
   printf("Random seed = %d\n",atoi(argv[1]));
   
   for(testN=1,errs=0;1;testN++)
   {  nr = randN(MaxNr-MinNr+1)+MinNr;
      ms = randN(MaxSq)+1;
      matrix = randN(MaxMatrix-MinMatrix+1)+MinMatrix;
      leeW   = randN(MaxLeeW-MinLeeW+1)+MinLeeW;
      for(i=0;i<nr;i++)
      {  rct[i].lx = randN(matrix+2*leeW)-leeW;
         rct[i].rx = rct[i].lx + 1 + randN(ms);
         rct[i].ty = randN(matrix+2*leeW)-leeW;
         rct[i].by = rct[i].ty + 1 + randN(ms);
         rct[i].next = rct+i+1;
      }
      rct[nr-1].next = NULL;
      /* End of random number calls here */
      if(isolTest!=0)
      {  if(isolTest!=testN) continue;
         printf("Isolated the %dth test\n",isolTest);
      }
      cn=dpSetClipRegion(0,0,matrix,matrix);
      cn=dpReduceClipList(cn,rct,1);
      if(dpExamineClipList(cn,nl))
      {  printf(
         "%3d rectangles. Max rect size %3d. Total size %3d Leeway %3d \n",
              nr,ms,matrix,leeW);
         printf("REDUCE ERROR\nBad clip-list ...\n");
         dpPrintRectList(rct,stdout);
         dpExamineClipList(cn,stdout);
      }
      else if(testReduce(cn,rct))
      {  printf(
         "%3d rectangles. Max rect size %3d. Total size %3d Leeway %3d \n",
                 nr,ms,matrix,leeW);
         dpPrintRectList(rct,stdout);
         dpExamineClipList(cn,stdout);
         errs++;
         if(errs>=maxErrs) break;
      }
      dpDestructClipList(cn);
      cn=dpSetClipRegion(0,0,matrix,matrix);
      cn=dpIntersectClipList(cn,rct,1);
      fflush(stdout);
      if(dpExamineClipList(cn,nl))
      {  printf(
         "%3d rectangles. Max rect size %3d. Total size %3d Leeway %3d \n",
              nr,ms,matrix,leeW);
         printf("INTERSECT ERROR\nBad clip-list ...\n");
         dpPrintRectList(rct,stdout);
         dpExamineClipList(cn,stdout);
         fflush(stdout);
         errs++;
         if(errs>=maxErrs) break;
      }
      else if(testIntersect(cn,rct))
      {  printf(
         "%3d rectangles. Max rect size %3d. Total size %3d Leeway %3d \n",
                 nr,ms,matrix,leeW);
         dpPrintRectList(rct,stdout);
         dpExamineClipList(cn,stdout);
         errs++;
         if(errs>=maxErrs) break;
      }
      dpDestructClipList(cn);
      if((testN%100)==0)
      {  fprintf(stderr,"%d tests complete\n",testN);
         printf("%d tests complete\n",testN);
         fflush(stdout);
      }
   }
   fprintf(stderr,"Test terminated : Too many errors\n");
   printf("Test terminated : Too many errors\n");
   return 0;
}

/*-----------------------------------------------------------*/
/*                                               randN(...)  */
/*-----------------------------------------------------------*/

int randN(int r) { return ((rand()>>16)*r)>>15; }

/*-----------------------------------------------------------*/
/*                                          testReduce(...)  */
/*-----------------------------------------------------------*/

int testReduce(DpClipNode_t *cl,DpRectangleList_t *rl)
/* For each point in the matrix, determine whether it should be */
/*   visible or not. Determine if it is visible in any of the   */
/*   clip-rects or not.  Also checks if a point is visible in   */
/*   more than one clip-node.                                   */
{  DpClipNode_t      *cn;
   DpRectangleList_t *rp;
   DpClipNodeEntry_t *cp;
   int x,y;
   
   for(y=0;y<matrix;y++) for(x=0;x<matrix;x++)
   {  int vis,cvis;
      vis = 1;
      for(rp=rl;rp;rp=rp->next)
      {  if(x>=rp->lx&&x<rp->rx)
            if(y>=rp->ty&&y<rp->by)
               vis=0;
      }
      cvis = 0;
      for(cn=cl;cn;cn=cn->nextNode)
      {  cp=cn->node+3;
         while ( !DpPosInf(*cp) )
         {  int ty,by;
            ty = *cp;
            by = cp[DpVisOff(cp[1])+1];
            if(DpPosInf(by)) break;
            if(ty<0||ty>matrix||by<0||by>matrix)
            {  printf("**********************************************\n");
               printf("REDUCE ERROR\nRange error in clip-list\n");
               return 1;
            }
            {  DpClipNodeEntry_t *xp;
               for(xp=cp+2; !DpPosInf(*xp); xp+=2)
               {  if(x>=xp[0]&&x<xp[1]&&y>=ty&&y<by) cvis++; 
                  if(xp[0]<0||xp[0]>matrix||xp[1]<0||xp[1]>matrix)
                  {  printf("**********************************************\n");
                     printf("REDUCE ERROR\nRange error in clip-list\n");
                     return 1;
                  }
               }
            }
            cp = cp + DpVisOff(cp[1]) +1;
         }
      }
      if(cvis>1)
      {  printf("*******************************************************\n");
         printf("REDUCE ERROR\nPoint (%d,%d) visible in more than one place\n",x,y);
         return 1;
      }
      if( cvis != vis )
      {  printf("*******************************************************\n");
         printf("REDUCE ERROR\nPoint (%d,%d) should be %s but is %s in clip-list\n",
                  x,y,vis?"visible":"non-visible",cvis?"visible":"non-visible");
         return 1;
      }
   }
   return 0;
}

/*-----------------------------------------------------------*/
/*                                       testIntersect(...)  */
/*-----------------------------------------------------------*/

int testIntersect(DpClipNode_t *cl,DpRectangleList_t *rl)
/* For each point in the matrix, determine whether it should be */
/*   visible or not. Determine if it is visible in any of the   */
/*   clip-rects or not.  Also checks if a point is visible in   */
/*   more than one clip-node.                                   */
{  DpClipNode_t      *cn;
   DpRectangleList_t *rp;
   DpClipNodeEntry_t *cp;
   int x,y;
   
   for(y=0;y<matrix;y++) for(x=0;x<matrix;x++)
   {  int vis,cvis;
      vis = 0;
      for(rp=rl;rp;rp=rp->next)
      {  if(x>=rp->lx&&x<rp->rx)
            if(y>=rp->ty&&y<rp->by)
               vis=1;
      }
      cvis = 0;
      for(cn=cl;cn;cn=cn->nextNode)
      {  cp=cn->node+3;
         while ( !DpPosInf(*cp) )
         {  int ty,by;
            ty = *cp;
            by = cp[DpVisOff(cp[1])+1];
            if(DpPosInf(by)) break;
            if(ty<0||ty>matrix||by<0||by>matrix)
            {  printf("**********************************************\n");
               printf("INTERSECT ERROR\nRange error in clip-list\n");
               return 1;
            }
            {  DpClipNodeEntry_t *xp;
               for(xp=cp+2; !DpPosInf(*xp); xp+=2)
               {  if(x>=xp[0]&&x<xp[1]&&y>=ty&&y<by) cvis++; 
                  if(xp[0]<0||xp[0]>matrix||xp[1]<0||xp[1]>matrix)
                  {  printf("**********************************************\n");
                     printf("INTERSECT ERROR\nRange error in clip-list\n");
                     return 1;
                  }
               }
            }
            cp = cp + DpVisOff(cp[1]) +1;
         }
      }
      if(cvis>1)
      {  printf("*******************************************************\n");
         printf("INTERSECT ERROR\nPoint (%d,%d) visible in more than one place\n",x,y);
         return 1;
      }
      if( cvis != vis )
      {  printf("*******************************************************\n");
         printf("INTERSECT ERROR\nPoint (%d,%d) should be %s but is %s in clip-list\n",
                  x,y,vis?"visible":"non-visible",cvis?"visible":"non-visible");
         return 1;
      }
   }
   return 0;
}
