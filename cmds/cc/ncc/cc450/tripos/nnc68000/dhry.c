/*
 *
 *   "DHRYSTONE" Benchmark Program
 *
 *   Version:   C/1.1, 12/01/84
 *
 *   Updated:   2-Jun-87:  Alan Mycroft, Computer Lab., Cambridge
 *         University, UK.  Add conditional compilation for ANSI
 *         C compilers (via __STDC__)  to use CLK_TCK and clock()
 *         to determine time.  This code should then work
 *         unmodified on any ANSI compiler.
 *         Also type Proc0(),... as macro VOID (q.v.)
 *         and add extern declarations to avoid the compiler
 *         needing to invent 'int Proc0()' which would conflict
 *         with a later 'void' type.
 *         It would be nice to use function prototypes instead
 *         (this could affect performance of 'char' formals
 *         in an ANSI compiler) but this would seem to require
 *         much #ifdef'ing for little gain.
 *         These changes should not affect timing results.
 *
 *   Date:      PROGRAM updated 01/06/86, COMMENTS changed 01/31/87
 *
 *   Author:      Reinhold P. Weicker,  CACM Vol 27, No 10, 10/84 pg. 1013
 *         Translated from ADA by Rick Richardson
 *         Every method to preserve ADA-likeness has been used,
 *         at the expense of C-ness.
 *
 *   Compile:   cc -O dry.c -o drynr         : No registers
 *         cc -O -DREG=register dry.c -o dryr   : Registers
 *
 *   Defines:   Defines are provided for old C compiler's
 *         which don't have enums, and can't assign structures.
 *         The time(2) function is library dependant; Most
 *         return the time in seconds, but beware of some, like
 *         Aztec C, which return other units.
 *         The LOOPS define is initially set for 50000 loops.
 *         If you have a machine with large integers and is
 *         very fast, please change this number to 500000 to
 *         get better accuracy.  Please select the way to
 *         measure the execution time using the TIME define.
 *         For single user machines, time(2) is adequate. For
 *         multi-user machines where you cannot get single-user
 *         access, use the times(2) function.  Be careful to
 *         adjust the HZ parameter below for the units which
 *         are returned by your times(2) function.  You can
 *         sometimes find this in <sys/param.h>.  If you have
 *         neither time(2) nor times(2), use a stopwatch in
 *         the dead of the night.
 *         Use a "printf" at the point marked "start timer"
 *         to begin your timings. DO NOT use the UNIX "time(1)"
 *         command, as this will measure the total time to
 *         run this program, which will (erroneously) include
 *         the time to malloc(3) storage and to compute the
 *         time it takes to do nothing.
 *
 *   Run:      drynr; dryr
 *
 *   Results:   If you get any new machine/OS results, please send to:
 *
 *            seismo!uunet!pcrat!rick   (Mail)
 *            seismo!uunet!pcrat!dry   (Submission Forms)
 *
 *         and thanks to all that do.
 *
 *   Note:      I order the list in increasing performance of the
 *         "with registers" benchmark.  If the compiler doesn't
 *         provide register variables, then the benchmark
 *         is the same for both REG and NOREG.
 *
 *   PLEASE:      Send complete information about the machine type,
 *         clock speed, OS and C manufacturer/version.  If
 *         the machine is modified, tell me what was done.
 *         On UNIX, execute uname -a and cc -V to get this info.
 *
 *   80x8x NOTE:   80x8x benchers: please try to do all memory models
 *         for a particular compiler.
 *
 *
 *   The following program contains statements of a high-level programming
 *   language (C) in a distribution considered representative:
 *
 *   assignments         53%
 *   control statements      32%
 *   procedure, function calls   15%
 *
 *   100 statements are dynamically executed.  The program is balanced with
 *   respect to the three aspects:
 *      - statement type
 *      - operand type (for simple data types)
 *      - operand access
 *         operand global, local, parameter, or constant.
 *
 *   The combination of these three aspects is balanced only approximately.
 *
 *   The program does not compute anything meaningfull, but it is
 *   syntactically and semantically correct.
 *
 */

/* Accuracy of timings and human fatigue controlled by next two lines */
#define LOOPS   50000      /* Use this for slow or 16 bit machines */
/*#define LOOPS   500000      /* Use this for faster machines */

/* Compiler dependent options */
#ifdef __STDC__
#define VOID void               /* give Proc0,...,Proc8 its proper type */
#else
#define VOID                    /* else null it away for K&R C    */
#endif
#undef   NOENUM         /* Define if compiler has no enum's */
#undef   NOSTRUCTASSIGN      /* Define if compiler can't assign structures */

#ifndef __STDC__  /*---------------Not needed for ANSI C */
/* define only one of the next two defines */
/* #define TIMES         / * Use times(2) time function */
#define TIME         /* Use time(2) time function */

/* define the granularity of your times(2) function (when used) */
/*#define HZ   50      /* times(2) returns 1/50 second (europe?) */
#define HZ   60      /* times(2) returns 1/60 second (most) */
/*#define HZ   100      /* times(2) returns 1/100 second (WECo) */
#endif            /*---------------Not needed for ANSI C */

/* for compatibility with goofed up version */
/*#undef GOOF         /* Define if you want the goofed up version */

#ifdef GOOF
char   Version[] = "1.0";
#else
char   Version[] = "1.1";
#endif

#ifdef   NOSTRUCTASSIGN
#define   structassign(d, s)   memcpy(&(d), &(s), sizeof(d))
#else
#define   structassign(d, s)   d = s
#endif

#ifdef   NOENUM
#define   Ident1   1
#define   Ident2   2
#define   Ident3   3
#define   Ident4   4
#define   Ident5   5
typedef int   Enumeration;
#else
typedef enum   {Ident1, Ident2, Ident3, Ident4, Ident5} Enumeration;
#endif

typedef int   OneToThirty;
typedef int   OneToFifty;
typedef char   CapitalLetter;
typedef char   String30[31];
typedef int   Array1Dim[51];
typedef int   Array2Dim[51][51];

struct   Record
{
   struct Record      *PtrComp;
   Enumeration      Discr;
   Enumeration      EnumComp;
   OneToFifty      IntComp;
   String30      StringComp;
};

typedef struct Record    RecordType;
typedef RecordType *   RecordPtr;
typedef int      boolean;

#define   NULL      0
#define   TRUE      1
#define   FALSE      0

#ifndef REG
#define   REG
#endif

extern Enumeration   Func1();
extern boolean      Func2();

#ifdef __STDC__
#include <stdio.h>    /* for printf */
#include <stdlib.h>   /* for malloc/exit */
#include <string.h>   /* for strcmp/strcpy */
#include <time.h>     /* for clock */
#endif

#ifdef TIMES
#include <sys/types.h>
#include <sys/times.h>
#endif

/* stop the compiler from inventing types */
extern VOID Proc0(), Proc1(), Proc2(), Proc3(), Proc4(),
            Proc5(), Proc6(), Proc7(), Proc8();

main()
{
   Proc0();
   exit(0);
}

/*
 * Package 1
 */
int      IntGlob;
boolean      BoolGlob;
char      Char1Glob;
char      Char2Glob;
Array1Dim   Array1Glob;
Array2Dim   Array2Glob;
RecordPtr   PtrGlb;
RecordPtr   PtrGlbNext;

VOID Proc0()
{
   OneToFifty      IntLoc1;
   REG OneToFifty      IntLoc2;
   OneToFifty      IntLoc3;
   REG char      CharLoc;
   REG char      CharIndex;
   Enumeration       EnumLoc;
   String30      String1Loc;
   String30      String2Loc;
#ifndef __STDC__    /* illegal to do this in ANSI C */
   extern char      *malloc();
#endif
   register unsigned int   i;

#ifdef __STDC__
   clock_t         starttime;
   clock_t         benchtime;
   clock_t         nulltime;

   starttime = clock();
   for (i = 0; i < LOOPS; ++i);
   nulltime = clock() - starttime; /* Computes o'head of loop */
#endif
#ifdef TIME
   long         time();
   long         starttime;
   long         benchtime;
   long         nulltime;

   starttime = time( (long *) 0);
   for (i = 0; i < LOOPS; ++i);
   nulltime = time( (long *) 0) - starttime; /* Computes o'head of loop */
#endif
#ifdef TIMES
   time_t         starttime;
   time_t         benchtime;
   time_t         nulltime;
   struct tms      tms;

   times(&tms); starttime = tms.tms_utime;
   for (i = 0; i < LOOPS; ++i);
   times(&tms);
   nulltime = tms.tms_utime - starttime; /* Computes overhead of looping */
#endif

   PtrGlbNext = (RecordPtr) malloc(sizeof(RecordType));
   PtrGlb = (RecordPtr) malloc(sizeof(RecordType));
   PtrGlb->PtrComp = PtrGlbNext;
   PtrGlb->Discr = Ident1;
   PtrGlb->EnumComp = Ident3;
   PtrGlb->IntComp = 40;
   strcpy(PtrGlb->StringComp, "DHRYSTONE PROGRAM, SOME STRING");
#ifndef   GOOF
   strcpy(String1Loc, "DHRYSTONE PROGRAM, 1'ST STRING");   /*GOOF*/
#endif
   Array2Glob[8][7] = 10;   /* Was missing in published program */

/*****************
-- Start Timer --
*****************/
#ifdef TIME
   starttime = time( (long *) 0);
#endif
#ifdef __STDC__
   starttime = clock();
#endif
#ifdef TIMES
   times(&tms); starttime = tms.tms_utime;
#endif
   for (i = 0; i < LOOPS; ++i)
   {

      Proc5();
      Proc4();
      IntLoc1 = 2;
      IntLoc2 = 3;
      strcpy(String2Loc, "DHRYSTONE PROGRAM, 2'ND STRING");
      EnumLoc = Ident2;
      BoolGlob = ! Func2(String1Loc, String2Loc);
      while (IntLoc1 < IntLoc2)
      {
         IntLoc3 = 5 * IntLoc1 - IntLoc2;
         Proc7(IntLoc1, IntLoc2, &IntLoc3);
         ++IntLoc1;
      }
      Proc8(Array1Glob, Array2Glob, IntLoc1, IntLoc3);
      Proc1(PtrGlb);
      for (CharIndex = 'A'; CharIndex <= Char2Glob; ++CharIndex)
         if (EnumLoc == Func1(CharIndex, 'C'))
            Proc6(Ident1, &EnumLoc);
      IntLoc3 = IntLoc2 * IntLoc1;
      IntLoc2 = IntLoc3 / IntLoc1;
      IntLoc2 = 7 * (IntLoc3 - IntLoc2) - IntLoc1;
      Proc2(&IntLoc1);
   }

/*****************
-- Stop Timer --
*****************/

#ifdef __STDC__
/* All ANSI say about clock_t is that it is an arithmetic type with unit    */
/* CLK_TCK.  This code probably does not cope with a floating clock_t.      */
   benchtime = clock() - starttime - nulltime;
   printf("Dhrystone(%s) time for %ld passes = %ld CLK_TCK\n",
      Version,
      (long) LOOPS, (long)benchtime);
   {    /* avoid overflow for a fast (> 1000 ticks/second clock. */
        clock_t tck = CLK_TCK;
        while (tck > 1000) tck /= 10, benchtime /= 10;
        printf("This machine benchmarks at %ld dhrystones/second\n",
      ((long) LOOPS) * (long)tck / (long)benchtime);
   }
#endif
#ifdef TIME
   benchtime = time( (long *) 0) - starttime - nulltime;
   printf("Dhrystone(%s) time for %ld passes = %ld\n",
      Version,
      (long) LOOPS, benchtime);
   printf("This machine benchmarks at %ld dhrystones/second\n",
      ((long) LOOPS) / benchtime);
#endif
#ifdef TIMES
   times(&tms);
   benchtime = tms.tms_utime - starttime - nulltime;
   printf("Dhrystone(%s) time for %ld passes = %ld\n",
      Version,
      (long) LOOPS, benchtime/HZ);
   printf("This machine benchmarks at %ld dhrystones/second\n",
      ((long) LOOPS) * HZ / benchtime);
#endif

}

VOID Proc1(PtrParIn)
REG RecordPtr   PtrParIn;
{
#define   NextRecord   (*(PtrParIn->PtrComp))

   structassign(NextRecord, *PtrGlb);
   PtrParIn->IntComp = 5;
   NextRecord.IntComp = PtrParIn->IntComp;
   NextRecord.PtrComp = PtrParIn->PtrComp;
   Proc3(NextRecord.PtrComp);
   if (NextRecord.Discr == Ident1)
   {
      NextRecord.IntComp = 6;
      Proc6(PtrParIn->EnumComp, &NextRecord.EnumComp);
      NextRecord.PtrComp = PtrGlb->PtrComp;
      Proc7(NextRecord.IntComp, 10, &NextRecord.IntComp);
   }
   else
      structassign(*PtrParIn, NextRecord);

#undef   NextRecord
}

VOID Proc2(IntParIO)
OneToFifty   *IntParIO;
{
   REG OneToFifty      IntLoc;
   REG Enumeration      EnumLoc;

   IntLoc = *IntParIO + 10;
   for(;;)
   {
      if (Char1Glob == 'A')
      {
         --IntLoc;
         *IntParIO = IntLoc - IntGlob;
         EnumLoc = Ident1;
      }
      if (EnumLoc == Ident1)
         break;
   }
}

VOID Proc3(PtrParOut)
RecordPtr   *PtrParOut;
{
   if (PtrGlb != NULL)
      *PtrParOut = PtrGlb->PtrComp;
   else
      IntGlob = 100;
   Proc7(10, IntGlob, &PtrGlb->IntComp);
}

VOID Proc4()
{
   REG boolean   BoolLoc;

   BoolLoc = Char1Glob == 'A';
   BoolLoc |= BoolGlob;
   Char2Glob = 'B';
}

VOID Proc5()
{
   Char1Glob = 'A';
   BoolGlob = FALSE;
}

extern boolean Func3();

VOID Proc6(EnumParIn, EnumParOut)
REG Enumeration   EnumParIn;
REG Enumeration   *EnumParOut;
{
   *EnumParOut = EnumParIn;
   if (! Func3(EnumParIn) )
      *EnumParOut = Ident4;
   switch (EnumParIn)
   {
   case Ident1:   *EnumParOut = Ident1; break;
   case Ident2:   if (IntGlob > 100) *EnumParOut = Ident1;
         else *EnumParOut = Ident4;
         break;
   case Ident3:   *EnumParOut = Ident2; break;
   case Ident4:   break;
   case Ident5:   *EnumParOut = Ident3;
   }
}

VOID Proc7(IntParI1, IntParI2, IntParOut)
OneToFifty   IntParI1;
OneToFifty   IntParI2;
OneToFifty   *IntParOut;
{
   REG OneToFifty   IntLoc;

   IntLoc = IntParI1 + 2;
   *IntParOut = IntParI2 + IntLoc;
}

VOID Proc8(Array1Par, Array2Par, IntParI1, IntParI2)
Array1Dim   Array1Par;
Array2Dim   Array2Par;
OneToFifty   IntParI1;
OneToFifty   IntParI2;
{
   REG OneToFifty   IntLoc;
   REG OneToFifty   IntIndex;

   IntLoc = IntParI1 + 5;
   Array1Par[IntLoc] = IntParI2;
   Array1Par[IntLoc+1] = Array1Par[IntLoc];
   Array1Par[IntLoc+30] = IntLoc;
   for (IntIndex = IntLoc; IntIndex <= (IntLoc+1); ++IntIndex)
      Array2Par[IntLoc][IntIndex] = IntLoc;
   ++Array2Par[IntLoc][IntLoc-1];
   Array2Par[IntLoc+20][IntLoc] = Array1Par[IntLoc];
   IntGlob = 5;
}

Enumeration Func1(CharPar1, CharPar2)
CapitalLetter   CharPar1;
CapitalLetter   CharPar2;
{
   REG CapitalLetter   CharLoc1;
   REG CapitalLetter   CharLoc2;

   CharLoc1 = CharPar1;
   CharLoc2 = CharLoc1;
   if (CharLoc2 != CharPar2)
      return (Ident1);
   else
      return (Ident2);
}

boolean Func2(StrParI1, StrParI2)
String30   StrParI1;
String30   StrParI2;
{
   REG OneToThirty      IntLoc;
   REG CapitalLetter   CharLoc;

   IntLoc = 1;
   while (IntLoc <= 1)
      if (Func1(StrParI1[IntLoc], StrParI2[IntLoc+1]) == Ident1)
      {
         CharLoc = 'A';
         ++IntLoc;
      }
   if (CharLoc >= 'W' && CharLoc <= 'Z')
      IntLoc = 7;
   if (CharLoc == 'X')
      return(TRUE);
   else
   {
      if (strcmp(StrParI1, StrParI2) > 0)
      {
         IntLoc += 7;
         return (TRUE);
      }
      else
         return (FALSE);
   }
}

boolean Func3(EnumParIn)
REG Enumeration   EnumParIn;
{
   REG Enumeration   EnumLoc;

   EnumLoc = EnumParIn;
   if (EnumLoc == Ident3) return (TRUE);
   return (FALSE);
}

#ifdef   NOSTRUCTASSIGN
memcpy(d, s, l)
register char   *d;
register char   *s;
register int   l;
{
   while (l--) *d++ = *s++;
}
#endif

