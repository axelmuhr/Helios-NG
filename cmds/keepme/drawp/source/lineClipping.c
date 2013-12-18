/*-------------------------------------------------------------*/
/*                                             lineClipping.c  */
/*-------------------------------------------------------------*/

/* $Header: lineClipping.c,v 1.10 90/07/11 15:37:30 charles Locked $ */
/* $Source: /server/usr/users/a/charles/world/drawp/RCS/source/lineClipping.c,v $ */

/* This file is compiled eight times, each time it is compiled  */
/*   varios symbols must be defined appropriately.              */
/* Thus the file compiles eight similar functions which all     */
/*   take a graphics context, and a line-control block          */
/*   containing some of the decoded information from that       */
/*   graphics context. The routines clip the line to the        */
/*   clipping list node provided as a parameter, and            */
/*   so divide the line up into segments. For each visible      */
/*   segment the routine sets up appropriate information in the */
/*   line-control block and indirects to some assembled code    */
/*   which draws that line segment.                             */

/* On entry the compiler expects exactly one identifier from    */
/*   each of the following pairs to be #define'd so that it     */
/*   can deduce the mode of operation ...                       */

/* Xn => delta x <0      ; Xp => delta x >=0                    */
/* Yn => delta y <0      ; Yp => delta y >=0                    */
/* Mx => x is major axis ; My => y is major axis                */

/*-------------------------------------------------------------*/
/*                                             #include files  */
/*-------------------------------------------------------------*/

#include "private.h"
#include "code_interface.h"

/*-------------------------------------------------------------*/
/*                                             Special Notice  */
/*-------------------------------------------------------------*/

/* It had been intended that source/lineClipping.c would       */
/*    accept parameters in terms of sub-pixel units, and draw  */
/*    lines accurately in sub-pixel units, however, the 32-bot */
/*    limit on the accuracy of the fixed-point arithmetic in   */
/*    'C' files makes this nigh impossible, and this will have */
/*    to await coding of this file into assembly. This file    */
/*    simply rounds co-ordinates passed to the nearest pixel   */
/*    co-ordinates, and uses pixel accuracy. The old version   */
/*    of this file which attempted to use pixel accuracy has   */
/*    been moved to the file 'docs/oldLineClipping.c' for      */
/*    examination prior to moving it to assembler code.        */

/*-------------------------------------------------------------*/
/*                               Visible-reigons block format  */
/*-------------------------------------------------------------*/

/* Refer to the file 'clipping.h'                              */

/*-------------------------------------------------------------*/
/*                                          The function name  */
/*-------------------------------------------------------------*/

/* The name of the funciton to be declared is computed in the  */
/*   following tangle:                                         */

# ifdef Xp
#    ifdef Yp
#       ifdef Mx
#          define FnName dpClipLineXpYpMx
#       else
#          define FnName dpClipLineXpYpMy
#       endif
#    else
#       ifdef Mx
#          define FnName dpClipLineXpYnMx
#       else
#          define FnName dpClipLineXpYnMy
#       endif
#    endif
# else
#    ifdef Yp
#       ifdef Mx
#          define FnName dpClipLineXnYpMx
#       else
#          define FnName dpClipLineXnYpMy
#       endif
#    else
#       ifdef Mx
#          define FnName dpClipLineXnYnMx
#       else
#          define FnName dpClipLineXnYnMy
#       endif
#    endif
# endif

/*-------------------------------------------------------------*/
/*                             Printing debugging information  */
/*-------------------------------------------------------------*/

/* #define LC_Debug */ /* Enable debugging diagnostics info */

/* Use these macros for debugging: */

# ifdef Xp
#    define Xdir "dx>=0"
# else
#    define Xdir "dx<0"
# endif

# ifdef Yp
#    define Ydir "dy>=0"
# else
#    define Ydir "dy<0"
# endif

# ifdef Mx
#    define Maxis "major x"
# else
#    define Maxis "major y"
# endif

# ifdef LC_Debug
#    define showMode  {if(dpDebug)printf("%s %s %s\n",Xdir,Ydir,Maxis);}
#    define diag(fmt) {if(dpDebug)printf fmt;}
# else
#    define showMode  ;
#    define diag(fmt) ;
# endif

/*-------------------------------------------------------------*/
/*                                  Find quotient and residue  */
/*-------------------------------------------------------------*/

/* This macro expands to a section of code which computes the  */
/*   residue in the range (-d..-1) of the number <n> modulo    */
/*   <d>, and places the result in <n>. It also computes the   */
/*   corresponding quotient <q> such that if <n> is the old    */
/*   value of <n> and <r> the new value, we have:              */
/* n = q*d + r                                                 */
/* On entry, <n> must be non-negative                          */

#define division(n,d,q)             \
{  int i=0;                         \
   q=0;                             \
   do i++; while(n-(d<<i)>=0);      \
   i--;                             \
   do                               \
   {  q<<=1;                        \
      if(n-(d<<i)>=0) n-=d<<i,q|=1; \
   } while(--i>=0);                 \
   n-=d;q++;                        \
}

/*-------------------------------------------------------------*/
/*                       Identifying the major and minor axes  */
/*-------------------------------------------------------------*/

/* The following section defines macros which point to the     */
/*   relevent major and minor axes quantities as passed to the */
/*   rouine being compiled as parameters                       */

# ifdef Mx
#    define Dmaj Dx
#    define Lmaj Lx
#    define Amaj x
#    define Emaj ex
#    define Dmin Dy
#    define Lmin Ly
#    define Amin y
#    define Emin ey
# else
#    define Dmaj Dy
#    define Lmaj Ly
#    define Amaj y
#    define Emaj ey
#    define Dmin Dx
#    define Lmin Lx
#    define Amin x
#    define Emin ex
# endif

/*-------------------------------------------------------------*/
/*      Determining the direction on the major and minor axes  */
/*-------------------------------------------------------------*/

/* The following macros determine whether the major axis is to */
/*    be incremented or decremented, similarly for the minor   */
/*    axis ...                                                 */

# ifdef Xp
#    define SgnX ( 1)
# else
#    define SgnX (-1)
# endif

# ifdef Yp
#    define SgnY ( 1)
# else
#    define SgnY (-1)
# endif

# ifdef Mx
#    define SgnMaj SgnX
#    define SgnMin SgnY
# else
#    define SgnMaj SgnY
#    define SgnMin SgnX
# endif

/*-------------------------------------------------------------*/
/*                           The 'incMaj' and 'incMin' macros  */
/*-------------------------------------------------------------*/

/* These macros increment/decrement the major and minor axes   */
/*   co-ordinates by the given values.                         */

# if (SgnMaj>0)
#    define incMaj(d) Amaj+=d;
# else
#    define incMaj(d) Amaj-=d;
# endif

# if (SgnMin>0)
#    define incMin(d) Amin+=d;
# else
#    define incMin(d) Amin-=d;
# endif

/*-------------------------------------------------------------*/
/*                                      The 'errorTerm' macro  */
/*-------------------------------------------------------------*/

/* During plotting, the values of (x,y) and Bn determine the   */
/*   precise position of a point which is precisely on the     */
/*   line being plotted.                                       */
/* The precise position is given by adding 0.5 to the major    */
/*   axis co-ordinate, and adding the following error term to  */
/*   the minor-axis co-ordinate. The error term macro gives    */
/*   the error as the numerator of a fraction whose denominator*/
/*   is Dmaj                                                   */
/* 'adding' means adding in the posotive direction, NOT adding */
/*   along the direction in which the line moves.              */
/* The macro 'getBn' is the corresponding inverse function     */

# if (SgnMin>0)
#    define errorTerm  ( Dmaj + Bn )
#    define getBn(e)   ( (e)  - Dmaj )
# else
#    define errorTerm  ( - Bn -1 )
#    define getBn(e)   ( -(e) -1 )
# endif

/*-------------------------------------------------------------*/
/*                                    The 'pointsMajor' macro  */
/*-------------------------------------------------------------*/

/* The following macro determines how many points would be     */
/*   plotted of the line BEFORE it crossed the limit Lmaj on   */
/*   the major axis.                                           */

# if (SgnMaj>0)
#    define pointsMajor (Lmaj-Amaj)
# else
#    define pointsMajor (Amaj-Lmaj+1)
# endif

/*-------------------------------------------------------------*/
/*                                     The 'deltaMinor' macro  */
/*-------------------------------------------------------------*/

/* The following macro determines precisely the difference in  */
/*   the precise (with correction,) minor co-ordinate of the   */
/*   current point and the precise minor co-ordinate of the    */
/*   last point that would be plotted before crossing the      */
/*   major axis co-ordinate limit. The result is given as the  */
/*   numerator of a fraction whose denominator is Dmaj         */

# define deltaMinor ((pointsMajor-1)*Dmin)

/*-------------------------------------------------------------*/
/*                                     The 'alongMajor' macro  */
/*-------------------------------------------------------------*/

/* Suppose that points are plotted of the current line until   */
/*   just before it crosses the major axis. Then the following */
/*   macro determines whether the minor axis co-ordinate of    */
/*   that final point has already crossed the minor axis, the  */
/*   result is true if it turns out to be the case that the    */
/*   point has NOT yet crossed the minor axis limit            */

/* These macros work given that Dx does not hold the escape    */
/*   values of +/- infinity:                                   */

# if (SgnMin>0)
#    define finiteMaj ( Dmaj*(Amin-Lmin)+errorTerm+deltaMinor <  0 )
# else
#    define finiteMaj ( Dmaj*(Amin-Lmin)+errorTerm-deltaMinor >= 0 )
# endif

# ifdef Mx
#    if (SgnX>0)
#       define alongMajor (!DpPosInf(Lx)&&finiteMaj)
#    else
#       define alongMajor (!DpNegInf(Lx)&&finiteMaj)
#    endif
# else
#    if (SgnX>0)
#       define alongMajor ( DpPosInf(Lx)||finiteMaj)
#    else
#       define alongMajor ( DpNegInf(Lx)||finiteMaj)
#    endif
# endif

/*-------------------------------------------------------------*/
/*                                            The 'distMinor'  */
/*-------------------------------------------------------------*/

/* The following macro evaluates the precise distance along    */
/*   minor axis from the current (co-ordinates corrected)      */
/*   to the minor-axis boundary of the current reigon          */
/* The result is given as the numerator af a fraction whose    */
/*   denominator is Dmaj                                       */

# if (SgnMin>0)
#    define  distMinor ( Dmaj*(Lmin-Amin) - errorTerm )
# else
#    define  distMinor ( Dmaj*(Amin-Lmin) + errorTerm )
# endif

/*-------------------------------------------------------------*/
/*                                     The 'pointsMinor' axis  */
/*-------------------------------------------------------------*/

/* The following macro determines the number fof pixels that   */
/*   would be plotted of the current line BEFORE it corssed    */
/*   the minor axis co-ordinate limit. This macro should only  */
/*   be called once it has been determined that Dmin!=0,       */
/*   it is in fact called if it has been determined that the   */
/*   current line leaves the major axis along the minor axis,  */
/*   and for the point to do this, it must be true that Dmin   */
/*   is non-zero.                                              */

# if (SgnMin>0)
#    define pointsMinor (Dmin?(((distMinor+1)/(Dmin))+1):Pc)
# else
#    define pointsMinor (Dmin?((distMinor/(Dmin))+1):Pc)
# endif

/*-------------------------------------------------------------*/
/*                                          The 'NextY' macro  */
/*-------------------------------------------------------------*/

/* This macro to get to the start (dx>=0) or end (dx<0) of the */
/*    next (dy>=0) or previous (dy<0) x-vector, and load Ly    */
/*    the bottom (dy>=0) or top (dy<0) y-limit of that vector, */
/*    and to set vis to 1. Branch to 'Exit' if Ly turns out to */
/*    be posotive (dy>=0) or negative (dy<0) infinity.         */

# define VectEnd   do;while(!DpPosInf(*vp++));
# define VectSrt   do;while(!DpNegInf(*vp--));

# if (SgnY>0)
#    if (SgnX>0)
#       define NextY \
        {  int o;VectEnd o=DpVisOff(*++vp);       \
           if(DpPosInf(Ly=vp[o]))goto Exit;vis=1; \
        }
#    else
#       define NextY \
        {  int o;VectEnd o=DpVisOff(*++vp);vp+=o; \
           if(DpPosInf(Ly=*vp--))goto Exit;vis=1; \
        }
#    endif
# else
#    if (SgnX>0)
#       define NextY \
        {  int o;VectSrt o=DpVisOff(*--vp);vp-=o; \
           if(DpNegInf(Ly=*vp++))goto Exit;vis=1; \
        }
#    else
#       define NextY \
        {  int o;VectSrt o=DpVisOff(*--vp);       \
           if(DpNegInf(Ly=vp[-o]))goto Exit;vis=1;\
        }
#    endif
# endif

/*-------------------------------------------------------------*/
/*                           The 'NextX' and 'SearchX' macros  */
/*-------------------------------------------------------------*/

/* NextX    : Get the next reigon to the right (dx>=0) or left */
/*              (dx<0) and invert 'vis'. Lx holds the rightmost*/
/*              (dx>=0) or leftmost (dx<0) x-limit of the      */
/*              reigon. vs points to the location from which   */
/*              Lx was loaded.                                 */
/* SearchX  : Invoke 'NextX' one or mor times until the        */
/*              x-reigon pointed contains or is to the right   */
/*              (dx>=0) or left (dx<0) of <x>.                 */

# if (SgnX>0)
#    define NextX   Lx=*++vp,vis^=1;
#    define SearchX do NextX while(!DpPosInf(Lx)&&Lx<=x);
# else
#    define NextX   Lx=*--vp,vis^=1;
#    define SearchX do NextX while(!DpNegInf(Lx)&&Lx>x);
# endif

/*-------------------------------------------------------------*/
/*                                         The 'InitVp' macro  */
/*-------------------------------------------------------------*/

/* This macro is used to initialize the visible reigon pointer */
/*    given that it initially points to the start of the       */
/*    visible reigon block.                                    */

/* SrchFwd is used if dy>=0. It searches from the start of the */
/*    clip-node until *vp=Ly is the first y-value below y.     */
/*    branch to 'Exit' if Ly turns out to be +infinity         */
/* SrchBck is used if dy<0. It searches from the end of the    */
/*    clip-node until *vp=Ly is the first y-value above y.     */
/*    branch to 'Exit' if Ly turns out to be -infinity         */

# define SrchFwd \
{  vp+=3;                              \
   do                                  \
   {  if(DpPosInf(Ly=*vp)) goto Exit;  \
      if(Ly>y) break;                  \
      {int o;o=DpVisOff(*++vp);vp+=o;} \
   } while(1);                         \
}

# define SrchBck \
{  vp+=DpVisOff(*vp)-4;                \
   do                                  \
   {  if(DpNegInf(Ly=*vp))goto Exit;   \
      if(Ly<=y)break;                  \
      {int o;o=DpVisOff(*--vp);vp-=o;} \
   } while(1);                         \
}

# if (SgnY>0)
#    if (SgnX>0)
#       define InitVp {SrchFwd{int o;o=DpVisOff(vp[-1]);vp-=o;vis=1;SearchX}}
#    else
#       define InitVp {SrchFwd vp--;vis=1;SearchX}
#    endif
# else
#    if (SgnX>0)
#       define InitVp {SrchBck vp++;vis=1;SearchX}
#    else
#       define InitVp {SrchBck{int o;o=DpVisOff(vp[1]);vp+=o;vis=1;SearchX}}
#    endif
# endif

/*-------------------------------------------------------------*/
/*                                       The 'Outside' macros  */
/*-------------------------------------------------------------*/

/* These macros to determine if the given co-ordinate is       */
/*   inside or outside the current reigon limits               */

# if (SgnX>0)
#    define OutsideX(x) (x>=Lx)
# else
#    define OutsideX(x) (x<Lx)
# endif

# if (SgnY>0)
#    define OutsideY(y) (y>=Ly)
# else
#    define OutsideY(y) (y<Ly)
# endif

# ifdef Mx
#    define OutsideMaj OutsideX(x)
#    define OutsideMin OutsideY(y)
# else
#    define OutsideMaj OutsideY(y)
#    define OutsideMin OutsideX(x)
# endif

/*-------------------------------------------------------------*/
/*                                 The initialization process  */
/*-------------------------------------------------------------*/

/* This is the major fudge with respect to the section headed    */
/*    'special notice' at the top of this file. This defines the */
/*    initialization process: during which the sub-pixel         */
/*    accurate co-ordinates passed to this file are rounded to   */
/*    nearest pixel value, and initialization occurs with        */
/*    essentially only pixel-for-pixel accuracy.                 */
/* Note that the other major change if an improvement is to be   */
/*    made is to change dpClipLine in 'source/thinLines.c': once */
/*    this file has been changed to work satisfactorily, that    */
/*    routine should be changed so that when a line with         */
/*    coincident endpoints is drawn, instead, a line is drawn    */
/*    from (x,y) to (x+1,y) instead of from (x,y) to (x+Accuracy */
/*    ,y) as it is now.                                          */

# define I(x) ((x)>>BinaryFigures)
# define Half (Accuracy/2)

#    define InitCoords             \
Emaj = I(Emaj+Half);               \
Emin = I(Emin+Half);               \
Amaj = I(Amaj+Half);               \
Amin = I(Amin+Half);               \
Dmaj = SgnMaj * 2 * (Emaj-Amaj);   \
Dmin = SgnMin * 2 * (Emin-Amin);   \
Pc   = SgnMaj * (Emaj-Amaj);       \
if(capStyle!=CapNotLast) Pc++;     \
Bn = getBn(Dmaj/2);                \
if(Dmaj==0) Dmaj=1;                \
if(Pc==0)   goto Exit;

/*-------------------------------------------------------------*/
/*          Some sepcification and implementation information  */
/*-------------------------------------------------------------*/

/* The 'dpClipLine...' functions defined in separate compilations */
/*    of this file have the following prototype:                  */

/* void dpClipLine... ( LineControlBlock_t *lcb,                  */
/*                      DpClipNodeEntry_t   *vp,                  */
/*                      int sx, int sy, int ex, int ey,           */
/*                      int capStyle                              */
/*                    )                                           */

/* The fields of the <lcb> that pertain to the graphics context   */
/*   are correctly filled in, and <vp> points to the zero offset  */
/*   entry in some valid clip-node.                               */

/* The function shall implement the plotting of the line in       */
/*   in question, clipped to the clip-node regions supplied.      */
/* If the function were called twice with any two different clip  */
/*   nodes, then in the region given by the intersection of the   */
/*   two clip-nodes, precisely the same pixels would be plotted   */
/*   by either function. If <capStyle> is <CapNotLast> then the   */
/*   last point is not plotted.                                   */

/* The function is implemented by determining which segments of   */
/*   the line are visible, and passing (in precise form), the     */
/*   co-ordinate parameters of those line-segments down to        */
/*   assembly code routines in 'lines.s'. The function will fill  */
/*   in the following fields of the <lcb> prior to entering it:   */

/* desX, desY       : These six define the precise pixels to be   */
/* points, numer    :    plotted in the segment                   */
/* ratioX, ratioY   : These two same for each segment             */
/* strLen, strSign  : These three provide dot/dash information to */
/* strPtr           :    indicate the precise phase               */

/* Thus the 'dpClipLine...' routines should be entered with the   */
/*   other information in the lcb set-up correctly. In particular,*/
/*   the entry 'mainGroup' should be set to show the offset into  */
/*   the 'lines.s' look-up table called 'dpLookUpLineEntry' of    */
/*   where the correct set of main-entry routine addresses are    */
/*   stored. The value of 'mainGroup' is essentially one of       */
/*   'LmnMasked' or 'LmnUnmaked'. Also, on entry to this routine  */
/*   'strOff' is the dot-dash offset in the case of a dot-dash    */
/*   list and 'strBase' is a pointer to a 0-terminated list of    */
/*   unsigned characters defining the dot-dash list, or NULL if   */
/*   the line style is 'LineSolid'. If the line style is one of   */
/*   'LineDoubleDash' or 'LineOnOffDash' then which of these it   */
/*   is is embodied in the graphical information also supplid, ie */
/*   the graphics mode for the odd-dashes is always set to 'noop' */
/*   in 'LineOnOffDash'. On exit from a 'dpClipLine...' routine,  */
/*   the parameter 'strOff' is changed to give a correct new      */
/*   dot/dash offset for the next point that would be plotted, in */
/*   the case of 'CapNotLast' this referes to the last point on   */
/*   the line provided - ie. the point that isn't plotted.        */

/* In this 'C' code 'Accuracy' and 'BinaryFigures' are two        */
/*    manifest integer constants, with Accuracy=2^BinaryFigures   */
/* The start and end co-ordinates supplied are in (1/Accuracy)    */
/*    pixel-width units, taken relative to the pixel centre of    */
/*    the pixel at location (0,0). However, during the            */
/*    initialization process these parameters are soon reduced to */
/*    being in units of 1 pixel-width, and relative to the upper  */
/*    left corner of the bounding box of the upper left pixel,    */
/*    all fractional information about the precise co-ordinate    */
/*    location of the pixels being embodied in the variable Bn,   */
/*    as described in the section of this file where the symbol   */
/*    'errorTerm' is defined                                      */
/* Dx and Dy are loaded with the line deltas in the high-         */
/*   precision form of units of (1/Accuracy) pixel-widths.        */

/*-------------------------------------------------------------*/
/*                                 Identifying the line-class  */
/*-------------------------------------------------------------*/

/* These symbols are used to look-up the function to call in   */
/*   assembler code to plot the line.                          */

# if (SgnX>0)
#    define LmnX LmnXp
# else
#    define LmnX LmnXn
# endif

# if (SgnY>0)
#    define LmnY LmnYp
# else
#    define LmnY LmnYn
# endif

# ifdef Mx
#    define LmnM LmnMx
# else
#    define LmnM LmnMy
# endif

/*-------------------------------------------------------------*/
/*                                        The function itself  */
/*-------------------------------------------------------------*/

void FnName ( LineControlBlock_t *lcb,
              DpClipNodeEntry_t  *vp,
              int x,int y,int ex,int ey,
              int capStyle
            )
/* Note : The values Dx and Dy are made posotive, ie they are  */
/*    absolute value of the relevant offsets. The sign of the  */
/*    two quantities is determined by the context              */
/* Note : We are guaranteed that we will not get Dx==Dy==0,    */
/*    and also that Dmaj>=Dmin, so it follows that Dmaj!=0     */
/* Note : The dot/dash period on entry is guaranteed to be     */
/*    even.                                                    */
{  int Lx,Ly;   /* Limit values of current reigon              */
   int Dx,Dy;   /* Slope of the line (both made posotive)      */
   int q,Bn;    /* Bresenham number                            */
   int vis;     /* Visibility flags for reigons                */
   int Pc,Pp;   /* Number of points to plot this segment       */
   void (*plotFn)(LineControlBlock_t*);

   diag(("----------------------- Entering a clipLine routine\n"))
   diag(("Given co-ordinates are (%d,%d) to (%d,%d)\n",x,y,ex,ey))
   
   InitCoords   /* Initialize co-ordinate information             */
   InitVp       /* Get to right position in visible region str    */
   lcb->ratioX = Dx; lcb->ratioY = Dy;
   plotFn = dpLookUpLineRoutineAddress(LmnX|LmnY|LmnM|lcb->mainGroup);
   
   showMode
   diag(("Line from (%d,%d) to (%d,%d)\n",x,y,ex,ey))
   diag(("Limits are (%d,%d)\n",Lx,Ly))

   /* Trap all-of-line-inside-single-region here: */
   if(!OutsideX(ex)&&!OutsideY(ey)) { Pp=Pc; Pc=0; goto LineLengthFound; }

   do
   {  diag( ("Examining section from (%d,%d) against limits (%d,%d)\n",
             x,y,Lx,Ly))
      if(alongMajor)              /* Find the number of points */
      {  diag(("Exiting on major axis\n"))
         Pp=pointsMajor;          /*   plottable within the    */
      }
      else                        /*   current region ...      */
      {  diag(("Exiting on minor axis\n"))
         Pp=pointsMinor;          /* ... continued ...         */
      }
      if((Pc-=Pp)<0) Pp+=Pc;      /* Adjust Pp and Pc.         */
      /* Now Pp contains the number of pixels to plot in the      */
      /*   current segment, and Pc contains the number of         */
      /*   remaining pixels, or otherwise a number <=0.           */
      LineLengthFound:          /* Here when current seg computed */
      diag(("%d points starting from (%d,%d) are %s\n",
            Pp,x,y,vis?"visible":"invisible"))
      if(lcb->strBase)               /* If dot-dash ...           */
      {  unsigned char *ddp,*dds;    /* Dot/dash pointers         */
         unsigned int o;             /* To compute stroke offset  */
         int r;                      /* To compute first str len  */
         o=lcb->strOff;              /* Get current offset        */
         r=(int)(o%lcb->strPeriod);  /* Reduce it mod <period>    */
         lcb->strOff = r+Pp;         /* Write back next offset    */
         ddp=dds=lcb->strBase;       /* Get dot/dash list base    */
         do r-=*ddp++;while(r>=0);   /* Find correct posn. in list*/
         lcb->strLen=-r;             /* Set length of first stroke*/
         lcb->strSign                /* Set sign of first         */
          = ((ddp-dds)&1)^1;         /*      stroke               */
         lcb->strPtr=ddp;            /* Set ptr to next str len   */
      } else                         /* If not dot-dash:          */
      {  lcb->strLen=Pp+1;           /* Set ample length          */
         lcb->strSign=0;             /* Always an 'even' dash     */
      }
      if(vis)                        /* If visible line segment:  */
      {  lcb->desX = x;              /* Set coordinate params ... */
         lcb->desY = y;              /* ... continued ...         */
         lcb->points = Pp;           /* ... continued ...         */
         lcb->numer  = Bn;           /* ... continued             */
         (*plotFn)(lcb);             /* Call main routine         */
      }
      if(Pc<=0) goto Exit;           /* Exit if no more points    */
      incMaj(Pp)                     /* Compute loc of new point: */
      q=0;                           /* Default: no maj axis mvmnt*/
      Bn+=Pp*Dmin;                   /* Compute Total bres. offset*/
      if(Bn>=0)                      /* If Bn overflows ...       */
      {  division(Bn,Dmaj,q);        /* Compute new bres. offset  */
         incMin(q)                   /* Compute new minor axis    */
      }                              /* Resume here               */
      if(OutsideY(y)){NextY SearchX} /* To new line record?       */
      else {NextX}                   /*  or still on existing?    */
   } while(1);                       /* Look at next segment      */
   Exit:
   diag (("^^^^^^^^^^^^^^^^^^^^ Exiting a clipLine Routine\n"))
   return;
}

	
