/*-------------------------------------------------------------*/
/*                                    lochdr/code_interface.h  */
/*-------------------------------------------------------------*/

/* $Header: code_interface.h,v 1.8 90/07/11 15:38:02 charles Locked $ */
/* $Source: /server/usr/users/a/charles/world/drawp/RCS/lochdr/code_interface.h,v $ */

/* This file contains definitions of all the structures in     */
/*   module which are shared between assembley code functions  */
/*   and 'C' functions. The corresponding assembley file is    */
/*   called 'lochdr/structures.ss' and the two files must be   */
/*   updated in step                                           */

/*-------------------------------------------------------------*/
/*                                                  Interlock  */
/*-------------------------------------------------------------*/

#ifndef _DpCodeInterface_h
#define _DpCodeInterface_h

/*-------------------------------------------------------------*/
/*                                        Other include files  */
/*-------------------------------------------------------------*/

#include "drawp/pixmaps.h"
#ifdef HeliosMode
#include <stdio.h>
#include <stdlib.h>
#include <attrib.h>
#include <cpustate.h>
#endif

/*-------------------------------------------------------------*/
/*                                      BlitterControlBlock_t  */
/*-------------------------------------------------------------*/

/* The following structure is used to communicate information  */
/*   to routines in 'rectangles.s' which is used for plotting  */
/*   rectangles. Such a structure is predominantly set-up with */
/*   decoded graphics context information by routines in the   */
/*   file 'gc_decoder.c' Object of this structure are used by  */
/*   many different routines in various files as graphics      */
/*   operations are always decoded by splitting them up into   */
/*   rectangles                                                */
/* To deduce the meaning of all of the parameters in this      */
/*   structure, it is necassary to examine the files           */
/*   'gc_decoder.c' and 'rectangles.s'                         */

/* Note : DpMaxBpp is defined in <ext/pixmaps.h>, not here     */

typedef struct BlitterControlBlock_s
{  
   /* -----  Graphics plot-mode and colour information  ------ */
   void      *combMode[DpMaxBpp];    /* Array of pointers      */
   void     (*ctlRtn)(struct BlitterControlBlock_s *);
                                     /* Control routine        */

    /* ------------ Source Pix-map information --------------- */
    void     *srcBase;               /* Raw data first row     */
    void     *srcLast;               /* Raw data last row      */
    int       srcWPV;                /* Words per vector       */
    int       srcWPL;                /* Words per line         */
    int       srcSizX;               /* X-size                 */
    int       srcSizY;               /* Y-size                 */
    int       srcOffX;               /* Sub this offset fr des */
    int       srcOffY;               /*  coords to get src     */
    
    /* -------------- Mask pixmap information ---------------- */
    void     *mskBase;               /* Raw data first row     */
    int       mskWPV;                /* Words per vector       */
    int       mskOffX;               /* Sub this offset fr des */
    int       mskOffY;               /*  coords to get src     */
    
    /* ---------- Destination pixmap information ------------- */
    void     *desBase;               /* Raw data first row     */
    int       desBPP;                /* No of bit-planes       */
    int       desWPV;                /* Words per vector       */
    
    /* -------------- Rectangle to be copied ----------------- */
    int       desLftX;               /* Left   X               */
    int       desRgtX;               /* Right  X               */
    int       desTopY;               /* Top    Y               */
    int       desBotY;               /* Bottom Y               */

                /****************************/
    
    /* There now follows part of the structure which is used   */
    /*    for temporary storage of intermediates by the        */
    /*    routines in 'rectangles.s'                           */

    /* ----------------- System storage area ----------------- */
    void     *stkPtr;                /* Stack pointer on entry */

    /* ----------- Tiling control information -----------------*/
    int       blkSizX;                /* Block X size          */
    int       blkSizY;                /* Block Y size          */
    int       blkRemX;                /* Remainder to plot : X */
    int       blkRemY;                /* Remainder to plot : Y */
    int       srcFstY;                /* Cord of fst src row * */
    void     *srcFirst;               /* Addr of fst src row   */
    void     *desFirst;               /* Addr of fst des row   */
    void     *mskFirst;               /* Addr of fst msk row   */
    int       srcIncDec;              /* Src inc/dec           */
    int       desIncDec;              /* Des inc/dec           */
    void     *mskSave;                /* Save space for msk ptr*/
    void     *srcSave;                /* Save space for src ptr*/
    int       rowCnt;                 /* Row countdown         */
    void     *srcPtrRld;              /* Source reload value   */
    int       desPosX;                /* Des column counter    */

    /* ------------- Debugging storage area ----------------- */

    void     *reg[16];                /* Register set          */
    void     *diagRtn;                /* Link return for diag  */

} BlitterControlBlock_t;
    
/*-------------------------------------------------------------*/
/*                                             BcbContainer_t  */
/*-------------------------------------------------------------*/

/* This structure to contain a blitter control block, but in   */
/*    helios mode also provide space before the control block  */
/*    helios to place to processor save state in an inturrupt  */
/*    occuring in the assembley code.                          */

typedef struct BcbContainer_s
{  
#ifdef HeliosMode
   unsigned char saveStateSpace[sl_offset];
#endif
   BlitterControlBlock_t bcbProper;
} BcbContainer_t;

/*------------------------------------------------------------*/
/*                             'rectangles.s' look-up tables  */
/*------------------------------------------------------------*/

/* Here we import two look-up tables defined in the file      */
/*   'rectangles.s' which are used to look-up the control     */
/*   and combination routine addresses which are placed in    */
/*   the BlitterControlBlock_t.                               */

/* The following routine is a small assembley routine which   */
/*    looks up the address of a particular control routine ...*/
/*    it had to be written to get over some casting           */
/*    restrictions in ANSI ...                                */
extern void (*(dpLookUpControlAddress(int)))(BlitterControlBlock_t*);
/* The above declares 'dpLookUpControlAddress' as a function  */
/*    accepting an integer and returning a pointer to a       */
/*    function which accepts a pointer to a blitter control   */
/*    block and returns void.                                 */

/* The following routine is the same except for looking up    */
/*      the address of combination routines, which are        */
/*      regarded as 'pointer-to-void'                         */
void *dpLookUpCombineAddress(int);

/* The following constants are useful for computing the bit   */
/*   masks used to look-up in the above tables                */

#define CtlUnary      ( 0 << 3 )
#define CtlBinary     ( 1 << 3 )
#define CtlTertiary   ( 2 << 3 )
#define CtlUnStippled ( 0 << 2 )
#define CtlStippled   ( 1 << 2 )
#define CtlTopBottom  ( 0 << 1 )
#define CtlBottomTop  ( 1 << 1 )
#define CtlLeftRight  ( 0 << 0 )
#define CtlRightLeft  ( 1 << 0 )

#define CmbUnMasked  ( 0 << 5 )
#define CmbMasked    ( 1 << 5 )
#define CmbLeftRight ( 0 << 4 )
#define CmbRightLeft ( 1 << 4 )

/*-------------------------------------------------------------*/
/*                                         LineControlBlock_t  */
/*-------------------------------------------------------------*/

/* The following structure is used to communicate with the     */
/*    routines in lines.s from the routines in lineControl.c   */
/* Some of the information is passed implicitly (sometimes as  */
/*    repeated information) according to which of the 32       */
/*    main-entry routines is called.                           */

#define MaxDotDash 20 /* Multiple if 4 */

typedef struct LineControlBlock_s
{  
   /* --------- Non-static co-ordiate information ------------ */
   
   int    desX;              /* First point : X co-ordinate    */
   int    desY;              /* First point : Y co-ordinate    */
   int    points;            /* Number of points to plot       */
   int    numer;             /* Bresenham numerator            */
   
   /* ---- Non-static dot/dash stroke information -------------*/
   
   unsigned int strOff;      /* Stroke offset for next segment */
   int    strLen;            /* No of pts to plt off fst strk  */
   int    strSign;           /* Sign of first stroke: even/odd */
   unsigned char *strPtr;    /* Ptr to dash list: next stroke  */
   
   /* --------- Static destination information --------------- */
   
   void  *desBase;           /* Base of raw data               */
   int    desWpv;            /* Words per vector               */
   int    desWpl;            /* Words per line                 */
   int    desDepth;          /* Number of bit-planes           */
   
   /* ---------- Static Mask pixmap information -------------- */
   
   void  *mskBase;           /* Raw data base                  */
   int    mskWpl;            /* Words per line                 */
   int    mskOffX;           /* Sub fr des X to get mask X     */
   int    mskOffY;           /* Sub fr des Y to get mask Y     */
   
   /* ------- Static Graphics context information -------------*/
   
   unsigned long foreground; /* Foreground colour mode         */
   unsigned long background; /* Background colour mode         */
   int    ratioX;            /* Line delta X (made posotive)   */
   int    ratioY;            /* Line delta Y (made posotive)   */
   int    mainGroup;         /* Group of main-entry routines   */
   unsigned char *strBase;   /* Ptr to dot/dash list           */
   unsigned int  strPeriod;  /* Total period of dot/dashes     */
   unsigned char strList[MaxDotDash]; /* Space to copy small   */
                             /*   dot/dash list.               */
   int    strAllocd;         /* Whether dot/dash list separate */
   
   /* ----------- Miscelleneous and Scratchpad --------------- */
   
   unsigned long strAlt;     /* Stroke control scratchpad      */
   unsigned char *strPtrSave;/* Stroke list pointer scratchpad */
   unsigned long fgdSave;    /* Foreground col scratchpad      */
   unsigned long bgdSave;    /* Background col scratchpad      */
   int    planeSave;         /* Bit-plane countdown scratchpad */
   void  *desPtrSave;        /* Des pointer scratchpad         */
   unsigned long desMskSave; /* Des mask    scratchpad         */
   void  *mskPtrSave;        /* Msk pointer scratchpad         */
   unsigned long mskMskSave; /* Msk mask    scratchpad         */
   
   void  *reg[16];           /* Debugging : register block     */
   void  *diagRtn;           /* Debugging : return point       */
   void  *stkPtr;            /* Save space for stack pointer   */

} LineControlBlock_t;

/*-------------------------------------------------------------*/
/*                                             LcbContainer_t  */
/*-------------------------------------------------------------*/

/* This structure to contain a line control block, but in      */
/*    helios mode also provide space before the control block  */
/*    for helios to place to processor save state in an        */
/*    inturrupt occuring in the assembley code.                */

typedef struct LcbContainer_s
{  
#ifdef HeliosMode
   unsigned char saveStateSpace[sl_offset];
#endif
   LineControlBlock_t lcbProper;
} LcbContainer_t;

/*-------------------------------------------------------------*/
/*                                     The line look-up table  */
/*-------------------------------------------------------------*/

/* The following small routine defined in assembler is used to     */
/*      look up the address of a line entry routine in the table   */
/*      and was written in assembler to avoid various casting      */
/*      problems with the ANSI C compiler ...                      */
extern void (*(dpLookUpLineRoutineAddress(int)))(LineControlBlock_t*);

#define LmnMx (0<<0)
#define LmnMy (1<<0)
#define LmnYp (0<<1)
#define LmnYn (1<<1)
#define LmnXp (0<<2)
#define LmnXn (1<<2)
#define LmnUnMasked  (0<<3)
#define LmnMasked    (1<<3)

/*-------------------------------------------------------------*/
/*                                           End of Interlock  */
/*-------------------------------------------------------------*/

#endif
