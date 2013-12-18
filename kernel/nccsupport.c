/*
 * File:	nccsupport.c
 * Subsystem:	Generic Helios executive
 * Authors:	P.A.Beskeen and N Clifton
 * Date:	Nov '91
 *
 * Description: This file implements the norcroft C support functions.
 *
 *		These have to be included in the kernel as the kernel cannot
 *		call any other libraries. The string functions used by the
 *		kernel are also held here (_memcpy(), memcpy() and memset()).
 *
 * RcsId: $Id: nccsupport.c,v 1.19 1993/08/11 15:13:24 nickc Exp $
 *
 * (C) Copyright 1991, 1992 Perihelion Software Ltd.
 * 
 * RcsLog: $Log: nccsupport.c,v $
 * Revision 1.19  1993/08/11  15:13:24  nickc
 * fixed compile time warning
 *
 * Revision 1.18  1993/08/11  09:50:13  nickc
 * added memcpy type routines for ARM support
 *
 * Revision 1.17  1992/12/02  14:50:36  nickc
 * moved back tracing and memory access checking functions to util libraray
 *
 * Revision 1.16  1992/11/18  14:28:57  paul
 * updated for non 0 IR0 on C40
 *
 * Revision 1.15  1992/11/12  17:00:46  paul
 * fixed for new permanent KDebug()
 *
 * Revision 1.14  1992/09/25  17:33:32  paul
 * updated for generic use
 *
 * Revision 1.13  1992/09/22  13:29:26  paul
 * renamed ExecRoot() to GetExecRoot()
 *
 * Revision 1.12  1992/07/29  09:53:51  nickc
 * changed divide and remainder functions not to use a static variable
 *
 * Revision 1.11  1992/07/29  08:55:13  paul
 * better memory access check
 *
 * Revision 1.9  1992/07/22  09:44:07  nickc
 * added more information to memory check output
 *
 * Revision 1.8  1992/07/17  15:32:36  nickc
 * changed how _backtrace works
 *
 * Revision 1.7  1992/07/13  16:13:27  nickc
 * fixed bugs in memory access check functions
 *
 * Revision 1.6  1992/06/26  14:59:58  nickc
 * added signal generation to __divtest() and check_access()
 *
 * Revision 1.5  1992/06/09  13:04:41  nickc
 * added memory access check functions
 *
 * Revision 1.4  1992/05/20  16:58:39  nickc
 * works with boolean version of _backtrace()
 *
 * Revision 1.3  1992/05/19  16:34:54  nickc
 * added code for back tracing
 *
 * Revision 1.2  1992/04/21  09:54:56  paul
 * alpha version
 *
 * Revision 1.1  1991/12/03  12:07:28  paul
 * Initial revision
 *
 */


/* Include Files: */

#include "kernel.h"
#include <signal.h>
#include <stddef.h>                /* for size_t                           */


#ifndef __ARM	/* ARM version provides assembler versions of these fn's */
/*
 * This function is called by the compiler when the
 * result of an integer division is not used, but
 * according to the ANSI spec we must still generate
 * an error if the division is illegal.
 *
 * eg:
 *
 * int func( int arg ) { int a = 1 / arg; return 2; }
 */

void
__divtest( int arg )
{
  if (arg == 0)
    CallException( _Task_, SIGFPE );

  return;
}


/*
 * Unsigned divide of a2 by a1: returns quotient
 */

unsigned long int
__udivide(
	  unsigned long int 	a1,
	  unsigned long int	a2 )
{
  unsigned long int 	a3, a4, ip;

  
  switch (a1)
    /*
     * I need a test to detect division by zero here, so I may as well
     * extend it to catch cases of division by rather small numbers,
     * especially given (a) those give worst case performance in the
     * shift-and-subtract algorithm and (b) in many programs they will
     * be especially common.  The switch statement is, furthermore,
     * about as cheap as it could be, in that it will start with in
     * effect "if (a1 <= 10u) ...", which is only one test to obey when a
     * happens to be large, and it proceeds with a table-jump.
     */
    {
    case 0:
      return 0;       /* or raise an exception, maybe */
      
    case 1:
      return a2;
      
    case 2:
      return (a2 >> 1);
      
      /*
       * All the following code is based on the idea that division by a constant
       * k can be treated as multiplication by (2^32/k) and keeping the top 32
       * bits of a double-length product.  The multiplication is coded up in
       * terms of explicit shifts and adds here.  A quotient computed this way
       * will suffer slightly in its least significant bit(s) if I do not compute
       * carries from the low order half of the product - in all except the case of
       * division by 3 the effor can be at most by 1, so I do a final check on my
       * remainder and make a correction if needed.
       */
      
    case 3:
      a1  = (a2 >> 1);
      a1 += (a1 >> 2);
      a1 += (a1 >> 4);
      a1 += (a1 >> 8);
      a1 += (a1 >> 16);
      /*
       * Division by 3 seem to give most trouble wrt rounding - I can end up
       * with an estimated quotient that is wrong by 2 after all this, even
       * though I re-use the original a2 on the next line.
       */
      a1 = (a2 >> 2) + (a1 >> 3);
      a2 = a2 - a1 - (a1 << 1);
      
      if (a2 >= 3)
        {
	  a1++;
	  a2 -= 3;

	  if (a2 >= 3)
	    a1++;
	}
      
      return a1;
      
    case 4:
      return (a2 >> 2);
      
    case 5:
      a1  = a2 - (a2 >> 2);
      a1 += (a1 >> 4);
      a1 += (a1 >> 8);
      a1 += (a1 >> 16);
      a1  = a1 >> 2;
      a2  = a2 - a1 - (a1 << 2);
      
      if (a2 >= 5)
	a1++;
      
      return a1;
      
    case 6:
      a1  =  a2 >> 1;
      a1 += (a1 >> 2);
      a1 += (a1 >> 4);
      a1 += (a1 >> 8);
      a1 += (a1 >> 16);
      /*
       * Division by 6 is nicer than division by 3 because in effect I compute
       * the quotient by dividing the 3 (which may have a significant error)
       * and then divising by 2 (which dampens the error down somewhat).
       */
      a1  = a1 >> 2;
      a3  = a1 + (a1 << 1);
      a2 -= (a3 << 1);
      
      if (a2 >= 6)
	a1++;
      
      return a1;
      
    case 7:
      a1  =  a2 >> 1;
      a1 += (a1 >> 3);
      a1 += (a1 >> 6);
      a1 += (a1 >> 12);
      a1 += (a1 >> 24);
      a1  =  a1 >> 2;
      a2  =  a2 +  a1 - (a1 << 3);
      
      if (a2 >= 7)
	a1++;
      
      return a1;
      
    case 8:
      return (a2 >> 3);
      
    case 9:
      a1  =  a2 - (a2 >> 3);
      a1 += (a1 >> 6);
      a1 += (a1 >> 12);
      a1 += (a1 >> 24);
      /*
       * from division by 9 upwards I am safe against rounding problems
       * with the multiplication, since I am able to calculate the estimated
       * quotient with 3 guard bits, and that is comfortably enough.
       */
      a1 = a1 >> 3;
      a2 = a2 - a1 - (a1 << 3);
      
      if (a2 >= 9)
	a1++;
      
      return a1;
      
    case 10:
      a1  = a2 - (a2 >> 2);
      a1 += (a1 >> 4);
      a1 += (a1 >> 8);
      a1 += (a1 >> 16);
      a1  =  a1 >> 3;
      a3  =  a1 + (a1 << 2);
      a2 -= (a3 << 1);
      
      if (a2 >= 10)
	a1++;
      
      return a1;
      
      /*
       * It is quite possible that sensible people would stop at 10 -
       * division by numbers up to 10 will indeed be especially common,
       * but I intend to go up as far as 16.
       */
      
    case 11:
      a1  =  a2 - (a2 >> 3);
      a1 += (a2 >> 5);
      a1 += (a2 >> 9);
      a1 += (a1 >> 10);
      a1 += (a1 >> 20);
      a1  = (a2 >> 1) + (a1 >> 2);
      a1  = (a1 >> 3);
      a3  =  a1 + (a1 << 1);
      a2  =  a2 + a1 - (a3 << 2);
      
      if (a2 >= 11)
	a1++;
      
      return a1;
      
    case 12:
      a1  =  a2 >> 1;
      a1 += (a1 >> 2);
      a1 += (a1 >> 4);
      a1 += (a1 >> 8);
      a1 += (a1 >> 16);
      a1  =  a1 >> 3;
      a3  =  a1 + (a1 << 1);
      a2 -= (a3 << 2);
      
      if (a2 >= 12)
	a1++;
      
      return a1;
      
    case 13:
      a1  =  a2 - (a2 >> 4);
      a1 -= (a2 >> 6);
      a1 += (a2 >> 10);
      a1 += (a1 >> 12);
      a1 += (a1 >> 24);
      a1  = (a2 >> 1) + (a1 >> 3);
      a1  =  a1 >> 3;
      a3  =  a1 + (a1 << 1);
      a2  =  a2 - a1 - (a3 << 2);
      
      if (a2 >= 13)
	a1++;
      
      return a1;
      
    case 14:
      a1  =  a2 >> 1;
      a1 += (a1 >> 3);
      a1 += (a1 >> 6);
      a1 += (a1 >> 12);
      a1 += (a1 >> 24);
      a1  =  a1 >> 3;
      a3  = (a1 << 3) - a1;
      a2  =  a2 - (a3 << 1);
      
      if (a2 >= 14)
	a1++;
      
      return a1;
      
    case 15:
      a1  =  a2 >> 1;
      a1 += (a1 >> 4);
      a1 += (a1 >> 8);
      a1 += (a1 >> 16);
      a1  =  a1 >> 3;
      a2  =  a2 + a1 - (a1 << 4);
      
      if (a2 >= 15)
	a1++;
      
      return a1;
      
    case 16:
      return (a2 >> 4);
      
    default:
      break;
    }
  
  a3 = a1;
  a4 = 0;
  ip = 0x80000000u;
  
  if (a2 < ip)
    ip = a2;
  
 u_loop:
  if      (ip <= a3)        goto u_sh0mod8;
  else if (ip <= (a3 << 1)) goto u_sh1mod8;
  else if (ip <= (a3 << 2)) goto u_sh2mod8;
  else if (ip <= (a3 << 3)) goto u_sh3mod8;
  else if (ip <= (a3 << 4)) goto u_sh4mod8;
  else if (ip <= (a3 << 5)) goto u_sh5mod8;
  else if (ip <= (a3 << 6)) goto u_sh6mod8;
  else if (ip >  (a3 << 7))
    {
      a3 = a3 << 8;

      goto u_loop;
    }
  
 u_loop2:
  if (a2 >= (a3 << 7)) a2 -= (a3 << 7), a4++;
  a4 = a4 << 1;
  
 u_sh6mod8:
  if (a2 >= (a3 << 6)) a2 -= (a3 << 6), a4++;
  a4 = a4 << 1;
  
 u_sh5mod8:
  if (a2 >= (a3 << 5)) a2 -= (a3 << 5), a4++;
  a4 = a4 << 1;
  
 u_sh4mod8:
  if (a2 >= (a3 << 4)) a2 -= (a3 << 4), a4++;
  a4 = a4 << 1;
  
 u_sh3mod8:
  if (a2 >= (a3 << 3)) a2 -= (a3 << 3), a4++;
  a4 = a4 << 1;
  
 u_sh2mod8:
  if (a2 >= (a3 << 2)) a2 -= (a3 << 2), a4++;
  a4 = a4 << 1;
  
 u_sh1mod8:
  if (a2 >= (a3 << 1)) a2 -= (a3 << 1), a4++;
  a4 = a4 << 1;
  
 u_sh0mod8:
  if (a2 >= a3) a2 -= a3, a4++;
  if (a1 <= a3 >> 1)
    {
      a3 = a3 >> 8;
      a4 = a4 << 1;
      goto u_loop2;
    }
  
  return a4;

} /* __udivide */


unsigned long int
__uremainder(
	     unsigned long int 	a1,
	     unsigned long int 	a2 )
{
  unsigned long int a3, a4, ip;

  
  switch (a1)
    /*
     * I need a test to detect division by zero here, so I may as well
     * extend it to catch cases of division by rather small numbers,
     * especially given (a) those give worst case performance in the
     * shift-and-subtract algorithm and (b) in many programs they will
     * be especially common.  The switch statement is, furthermore,
     * about as cheap as it could be, in that it will start with in
     * effect "if (a1 <= 10u) ...", which is only one test to obey when a
     * happens to be large, and it proceeds with a table-jump.
     */
    {
    case 0:
      return a2;
      
    case 1:
      return 0;
      
    case 2:
      return a2 & 1;
      
      /*
       * All the following code is based on the idea that division by a constant
       * k can be treated as multiplication by (2^32/k) and keeping the top 32
       * bits of a double-length product.  The multiplication is coded up in
       * terms of explicit shifts and adds here.  A quotient computed this way
       * will suffer slightly in its least significant bit(s) if I do not compute
       * carries from the low order half of the product - in all except the case of
       * division by 3 the effor can be at most by 1, so I do a final check on my
       * remainder and make a correction if needed.
       */
    case 3:
      a1  = (a2 >> 1);
      a1 += (a1 >> 2);
      a1 += (a1 >> 4);
      a1 += (a1 >> 8);
      a1 += (a1 >> 16);
      /*
       * Division by 3 seem to give most trouble wrt rounding - I can end up
       * with an estimated quotient that is wrong by 2 after all this, even
       * though I re-use the original a2 on the next line.
       */
      a1 = (a2 >> 2) + (a1 >> 3);
      a2 = a2 - a1 - (a1 << 1);
      
      if (a2 >= 3)
        {
	  a2 -= 3;

	  if (a2 >= 3)
	    a2 -= 3;
	}
      
      return a2;
      
    case 4:
      return a2 & 3;
      
    case 5:
      a1  =  a2 - (a2 >> 2);
      a1 += (a1 >> 4);
      a1 += (a1 >> 8);
      a1 += (a1 >> 16);
      a1  =  a1 >> 2;
      a2  =  a2 - a1 - (a1 << 2);
      
      if (a2 >= 5)
	a2 -= 5;
      
      return a2;
      
    case 6:
      a1  =  a2 >> 1;
      a1 += (a1 >> 2);
      a1 += (a1 >> 4);
      a1 += (a1 >> 8);
      a1 += (a1 >> 16);
      /*
       * Division by 6 is nicer than division by 3 because in effect I compute
       * the quotient by dividing the 3 (which may have a significant error)
       * and then divising by 2 (which dampens the error down somewhat).
       */
      a1  =  a1 >> 2;
      a3  =  a1 + (a1 << 1);
      a2 -= (a3 << 1);
      
      if (a2 >= 6)
	a2 -= 6;
      
      return a2;
      
    case 7:
      a1  =  a2 >> 1;
      a1 += (a1 >> 3);
      a1 += (a1 >> 6);
      a1 += (a1 >> 12);
      a1 += (a1 >> 24);
      a1  =  a1 >> 2;
      a2  =  a2 + a1 - (a1 << 3);
      
      if (a2 >= 7)
	a2 -= 7;
      
      return a2;
      
    case 8:
      return a2 & 7;
      
    case 9:
      a1  =  a2 - (a2 >> 3);
      a1 += (a1 >> 6);
      a1 += (a1 >> 12);
      a1 += (a1 >> 24);
      /*
       * from division by 9 upwards I am safe against rounding problems
       * with the multiplication, since I am able to calculate the estimated
       * quotient with 3 guard bits, and that is comfortably enough.
       */
      a1 = a1 >> 3;
      a2 = a2 - a1 - (a1 << 3);
      
      if (a2 >= 9)
	a2 -= 9;
      
      return a2;
      
    case 10:
      a1  = a2 - (a2 >> 2);
      a1 += (a1 >> 4);
      a1 += (a1 >> 8);
      a1 += (a1 >> 16);
      a1  = a1 >> 3;
      a3  = a1 + (a1 << 2);
      a2 -= (a3 << 1);
      
      if (a2 >= 10)
	a2 -= 10;
      
      return a2;
      
      /*
       * It is quite possible that sensible people would stop at 10 -
       * division by numbers up to 10 will indeed be especially common,
       * but I intend to go up as far as 16.
       */
      
    case 11:
      a1  = a2 - (a2 >> 3);
      a1 += (a2 >> 5);
      a1 += (a2 >> 9);
      a1 += (a1 >> 10);
      a1 += (a1 >> 20);
      a1  = (a2 >> 1) + (a1 >> 2);
      a1  = (a1 >> 3);
      a3  = a1 + (a1 << 1);
      a2  = a2 + a1 - (a3 << 2);
      
      if (a2 >= 11)
	a2 -= 11;
      
      return a2;
      
    case 12:
      a1  = a2 >> 1;
      a1 += (a1 >> 2);
      a1 += (a1 >> 4);
      a1 += (a1 >> 8);
      a1 += (a1 >> 16);
      a1  = a1 >> 3;
      a3  = a1 + (a1 << 1);
      a2 -= (a3 << 2);
      
      if (a2 >= 12)
	a2 -= 12;
      
      return a2;
      
    case 13:
      a1  =  a2 - (a2 >> 4);
      a1 -= (a2 >> 6);
      a1 += (a2 >> 10);
      a1 += (a1 >> 12);
      a1 += (a1 >> 24);
      a1  = (a2 >> 1) + (a1 >> 3);
      a1  =  a1 >> 3;
      a3  =  a1 + (a1 << 1);
      a2  =  a2 - a1 - (a3 << 2);
      
      if (a2 >= 13)
	a2 -= 13;
      
      return a2;
      
    case 14:
      a1  =  a2 >> 1;
      a1 += (a1 >> 3);
      a1 += (a1 >> 6);
      a1 += (a1 >> 12);
      a1 += (a1 >> 24);
      a1  =  a1 >> 3;
      a3  = (a1 << 3) - a1;
      a2  =  a2 - (a3 << 1);
      
      if (a2 >= 14)
	a2 -= 14;
      
      return a2;
      
    case 15:
      a1  = a2 >> 1;
      a1 += (a1 >> 4);
      a1 += (a1 >> 8);
      a1 += (a1 >> 16);
      a1  = a1 >> 3;
      a2  = a2 + a1 - (a1 << 4);
      
      if (a2 >= 15)
	a2 -= 15;
      
      return a2;
      
    case 16:
      return a2 & 15;
      
    default:
      break;
    }
  
  a3 = a1;
  a4 = 0;
  ip = 0x80000000u;
  
  if (a2 < ip)
    ip = a2;
  
 u_loop:
  if      (ip <= a3)        goto u_sh0mod8;
  else if (ip <= (a3 << 1)) goto u_sh1mod8;
  else if (ip <= (a3 << 2)) goto u_sh2mod8;
  else if (ip <= (a3 << 3)) goto u_sh3mod8;
  else if (ip <= (a3 << 4)) goto u_sh4mod8;
  else if (ip <= (a3 << 5)) goto u_sh5mod8;
  else if (ip <= (a3 << 6)) goto u_sh6mod8;
  else if (ip >  (a3 << 7))
    {
      a3 = a3 << 8;

      goto u_loop;
    }
  
 u_loop2:
  if (a2 >= (a3 << 7)) a2 -= (a3 << 7), a4++;
  a4 = a4 << 1;
  
 u_sh6mod8:
  if (a2 >= (a3 << 6)) a2 -= (a3 << 6), a4++;
  a4 = a4 << 1;
  
 u_sh5mod8:
  if (a2 >= (a3 << 5)) a2 -= (a3 << 5), a4++;
  a4 = a4 << 1;
  
 u_sh4mod8:
  if (a2 >= (a3 << 4)) a2 -= (a3 << 4), a4++;
  a4 = a4 << 1;
  
 u_sh3mod8:
  if (a2 >= (a3 << 3)) a2 -= (a3 << 3), a4++;
  a4 = a4 << 1;
  
 u_sh2mod8:
  if (a2 >= (a3 << 2)) a2 -= (a3 << 2), a4++;
  a4 = a4 << 1;
  
 u_sh1mod8:
  if (a2 >= (a3 << 1)) a2 -= (a3 << 1), a4++;
  a4 = a4 << 1;
  
 u_sh0mod8:
  if (a2 >= a3) a2 -= a3, a4++;
  if (a1 <= a3 >> 1)
    {
      a3 = a3 >> 8;
      a4 = a4 << 1;
      goto u_loop2;
    }
  
  return a2;

} /* __uremainder */


/*
 * Fast unsigned divide by 10: dividend in a1
 * Returns quotient in a1, remainder in a2
 *
 * Calculate x / 10 as (x * 2^32/10) / 2^32.
 * That is, we calculate the most significant word of the double-length
 * product. In fact, we calculate an approximation which may be 1 off
 * because we've ignored a carry from the least significant word we didn't
 * calculate. We correct for this by insisting that the remainder < 10
 * and by incrementing the quotient if it isn't.
 * Note that 2^32/10 = 0x19999999
 */

unsigned int
_udiv10( unsigned int a1 )
{
    unsigned int a2 = a1, a3;
/* Watch what happens if a1 starts off as  0x80000000 */
    a1 = a1 >> 1;                       /* 0x40000000 */
    a1 += (a1 >> 1);                    /* 0x60000000 */
    a1 += (a1 >> 4);                    /* 0x66000000 */
    a1 += (a1 >> 8);                    /* 0x66660000 */
    a1 += (a1 >> 16);                   /* 0x66666666 */

    /*
     * This has computed (8*a1)/10 which can not have overflowed, and which
     * in effect gives 3 guard bits when the multiplication is done.  Right
     * here at the end I shift right to divide by the factor of 8.  Thus
     * even if there was almost a carry from each of the above 4 additions
     * the error involved gets thoroughly dampened out.
     */
    
    a1 = a1 >> 3;                       /* 0x0ccccccc */

    /*
     * Now subtract 10*a1 from a2 to get the remainder, using shift & add
     * to do the multiplication.
     */
    
    a3  = a1 + (a1 << 2);   /* 0x3fffffff : (5 * a1)       */
    a2 -= (a3 << 1);        /* 0x80000000 - 0x7fffffff = 1 */

    if (a2 >= 10) a1++;

    return a1;

} /* _udiv10 */


signed long int
__divide(
	 signed long int 	a1,
	 signed long int	a2 )
/*
 * Divide a2 by a1 and return quotient.  Leave remainder in rem.
 * Note, by the way, that the variable "rem" is of type unsigned int,
 * even though my remainder is a signed quantity here.. but I do not
 * consider it worth having a second signed variable to hand back
 * a signed remainder, especially since I probably expect that the
 * said result will be handed back in a register rather than in memory.
 * The quotient is truncated towards zero (and I have signed values).
 * Code is similar to that for unsigned division, but has to fiddle
 * with signs, and can be very slightly quicker on the pre-normalization
 * since signed values are 1 bit shorter.
 */
{
  long int 		a4, ip;
  unsigned long int	a3, a2u;

  
  ip = (a2 & 0x80000000) | ((unsigned long int)(a1 ^ a2) >> 1);
  
  /*
   * ip bit 31  sign of dividend (= sign of remainder)
   *    bit 30  sign of dividend EOR sign of divisor (= sign of quotient)
   * these are packed into one register with a view to avoiding the
   * need for more work registers and the potential for extra register
   * save overhead.
   */
  
  if (a2 < 0)
    a2 = -a2;
  
  if (a1 < 0)
    a1 = -a1;
  
  switch (a1)
    /*
     * See commentary relating to the case of unsigned division.
     * It seems reasonable to put this after the place where I have
     * made a1 positive, since I need a high & low test on the range of a1
     * anyway.
     */
    {
    case 0:
      return 0;       /* or raise an exception, maybe */
      
    case 1:
      /*
       * The only case where signed division can give an overflow (as distinct from
       * having a division by zero) is when dividing INT_MIN by -1.  Here is
       * where I have to detect that case.  There are three plausible things to do:
       * (a) raise an exception, (b) return a quotient of 0 and a remainder
       * of INT_MIN and (c) return a quotient of -INT_MIN and a remainder of 0,
       * where in case (c) the quotient overflows and yields a bit pattern that
       * is the same as INT_MIN {I am blindly assuming twos complement 32 bit
       * arithmetic throughout this code].
       */

      if ((ip & 0x40000000) != 0)
	a2 = -a2;
      else if (a2 == 0x80000000)
        {
	  /* Overflow case here - maybe raise an exception? */
	  ;   /* I choose response (c) for now */
        }
      return a2;
      
    case 2:
      a1 = ((unsigned long int)a2 >> 1);
      if ((ip & 0x40000000) != 0) a1 = -a1;

      return a1;
      
    case 3:
      /*
       * Here I started with an unsigned dividend, so after taking its absolute
       * value the biggest value I can have is 0x80000000.  The result is that,
       * compared with the unsigned case, I have 1 bit extra available for use
       * as a guard bit.  This improves the accuracy of the estimate that I
       * generate quite usefully.
       */
      a1  = a2 + ((unsigned long int)a2 >> 2);
      a1 += ((unsigned long int)a1 >> 4);
      a1 += ((unsigned long int)a1 >> 8);
      a1 += ((unsigned long int)a1 >> 16);
      a1  = ((unsigned long int)a2 >> 2) + ((unsigned long int)a1 >> 4);
      a2  = a2 - a1 - (a1 << 1);
      
      if (a2 >= 3) a1++;
      if ((ip & 0x40000000) != 0) a1 = -a1;

      return a1;
      
    case 4:
      a1 = ((unsigned long int)a2 >> 2);
      if ((ip & 0x40000000) != 0) a1 = -a1;

      return a1;
      
    case 5:
      a1  = a2 + ((unsigned long int)a2 >> 1);
      a1 += ((unsigned long int)a1 >> 4);
      a1 += ((unsigned long int)a1 >> 8);
      a1 += ((unsigned long int)a1 >> 16);
      a1  = (unsigned long int)a1 >> 3;
      a2  = a2 - a1 - (a1 << 2);
      if (a2 >= 5) a1++;
      if ((ip & 0x40000000) != 0) a1 = -a1;

      return a1;
      
    case 6:
      a1  = a2 + ((unsigned long int)a2 >> 2);
      a1 += ((unsigned long int)a1 >> 4);
      a1 += ((unsigned long int)a1 >> 8);
      a1 += ((unsigned long int)a1 >> 16);
      a1  =  (unsigned long int)a1 >> 3;
      a3  = a1 + (a1 << 1);
      a2 -= (a3 << 1);
      if (a2 >= 6) a1++;
      if ((ip & 0x40000000) != 0) a1 = -a1;

      return a1;
      
    case 7:
      a1  = a2 + ((unsigned long int)a2 >> 3);
      a1 += ((unsigned long int)a1 >> 6);
      a1 += ((unsigned long int)a1 >> 12);
      a1 += ((unsigned long int)a1 >> 24);
      a1  =  (unsigned long int)a1 >> 3;
      a2  = a2 + a1 - (a1 << 3);
      
      if (a2 >= 7) a1++;
      if ((ip & 0x40000000) != 0) a1 = -a1;

      return a1;
      
    case 8:
      a1 = ((unsigned long int)a2 >> 3);
      
      if ((ip & 0x40000000) != 0) a1 = -a1;

      return a1;
      
    case 9:
      a1  = a2 - ((unsigned long int)a2 >> 3);
      a1 += ((unsigned long int)a1 >> 6);
      a1 += ((unsigned long int)a1 >> 12);
      a1 += ((unsigned long int)a1 >> 24);
      a1  =  (unsigned long int)a1 >> 3;
      a2  = a2 - a1 - (a1 << 3);
      
      if (a2 >= 9) a1++;
      if ((ip & 0x40000000) != 0) a1 = -a1;

      return a1;
      
    case 10:
      a1  = a2 + ((unsigned long int)a2 >> 1);
      a1 += ((unsigned long int)a1 >> 4);
      a1 += ((unsigned long int)a1 >> 8);
      a1 += ((unsigned long int)a1 >> 16);
      a1  =  (unsigned long int)a1 >> 4;
      a3  = a1 + (a1 << 2);
      a2 -= (a3 << 1);
      
      if (a2 >= 10) a1++;
      if ((ip & 0x40000000) != 0) a1 = -a1;

      return a1;
      
    case 11:
      a1  = a2 - ((unsigned long int)a2 >> 3);
      a1 += ((unsigned long int)a2 >> 5);
      a1 += ((unsigned long int)a2 >> 9);
      a1 += ((unsigned long int)a1 >> 10);
      a1 += ((unsigned long int)a1 >> 20);
      a1  = ((unsigned long int)a2 >> 1) + ((unsigned long int)a1 >> 2);
      a1  = ((unsigned long int)a1 >> 3);
      a3  = a1 + (a1 << 1);
      a2  = a2 + a1 - (a3 << 2);
      
      if (a2 >= 11) a1++;
      if ((ip & 0x40000000) != 0) a1 = -a1;

      return a1;
      
    case 12:
      a1  =  (unsigned long int)a2 >> 1;
      a1 += ((unsigned long int)a1 >> 2);
      a1 += ((unsigned long int)a1 >> 4);
      a1 += ((unsigned long int)a1 >> 8);
      a1 += ((unsigned long int)a1 >> 16);
      a1  =  (unsigned long int)a1 >> 3;
      a3  = a1 + (a1 << 1);
      a2 -= (a3 << 2);
      
      if (a2 >= 12) a1++;
      if ((ip & 0x40000000) != 0) a1 = -a1;

      return a1;
      
    case 13:
      a1  = a2 - ((unsigned long int)a2 >> 4);
      a1 -= ((unsigned long int)a2 >> 6);
      a1 += ((unsigned long int)a2 >> 10);
      a1 += ((unsigned long int)a1 >> 12);
      a1 += ((unsigned long int)a1 >> 24);
      a1  = ((unsigned long int)a2 >> 1) + ((unsigned long int)a1 >> 3);
      a1  =  (unsigned long int)a1 >> 3;
      a3  = a1 + (a1 << 1);
      a2  = a2 - a1 - (a3 << 2);
      
      if (a2 >= 13) a1++;
      if ((ip & 0x40000000) != 0) a1 = -a1;

      return a1;
      
    case 14:
      a1  =  (unsigned long int)a2 >> 1;
      a1 += ((unsigned long int)a1 >> 3);
      a1 += ((unsigned long int)a1 >> 6);
      a1 += ((unsigned long int)a1 >> 12);
      a1 += ((unsigned long int)a1 >> 24);
      a1  =  (unsigned long int)a1 >> 3;
      a3  = (a1 << 3) - a1;
      a2  = a2 - (a3 << 1);
      
      if (a2 >= 14) a1++;
      if ((ip & 0x40000000) != 0) a1 = -a1;

      return a1;
      
    case 15:
      a1  =  (unsigned long int)a2 >> 1;
      a1 += ((unsigned long int)a1 >> 4);
      a1 += ((unsigned long int)a1 >> 8);
      a1 += ((unsigned long int)a1 >> 16);
      a1  =  (unsigned long int)a1 >> 3;
      a2  = a2 + a1 - (a1 << 4);
      
      if (a2 >= 15) a1++;
      if ((ip & 0x40000000) != 0) a1 = -a1;

      return a1;
      
    case 16:
      a1 = ((unsigned long int)a2 >> 4);
      
      if ((ip & 0x40000000) != 0) a1 = -a1;

      return a1;
      
    default:
      break;
    }
  
  a2u = a2;
  a3  = a1;
  a4  = 0;
  
 s_loop:
  if      (a2u <= a3)        goto s_sh0mod8;
  else if (a2u <= (a3 << 1)) goto s_sh1mod8;
  else if (a2u <= (a3 << 2)) goto s_sh2mod8;
  else if (a2u <= (a3 << 3)) goto s_sh3mod8;
  else if (a2u <= (a3 << 4)) goto s_sh4mod8;
  else if (a2u <= (a3 << 5)) goto s_sh5mod8;
  else if (a2u <= (a3 << 6)) goto s_sh6mod8;
  else if (a2u >  (a3 << 7))
    {
      a3 = a3 << 8;
      goto s_loop;
    }
  
 s_loop2:
  if (a2u >= (a3 << 7)) a2u -= (a3 << 7), a4++;
  a4 = a4 << 1;
  
 s_sh6mod8:
  if (a2u >= (a3 << 6)) a2u -= (a3 << 6), a4++;
  a4 = a4 << 1;
  
 s_sh5mod8:
  if (a2u >= (a3 << 5)) a2u -= (a3 << 5), a4++;
  a4 = a4 << 1;
  
 s_sh4mod8:
  if (a2u >= (a3 << 4)) a2u -= (a3 << 4), a4++;
  a4 = a4 << 1;
  
 s_sh3mod8:
  if (a2u >= (a3 << 3)) a2u -= (a3 << 3), a4++;
  a4 = a4 << 1;
  
 s_sh2mod8:
  if (a2u >= (a3 << 2)) a2u -= (a3 << 2), a4++;
  a4 = a4 << 1;
  
 s_sh1mod8:
  if (a2u >= (a3 << 1)) a2u -= (a3 << 1), a4++;
  a4 = a4 << 1;
  
 s_sh0mod8:
  if (a2u >= a3) a2u -= a3, a4++;
  if ((unsigned long int)a1 <= (a3 >> 1))
    {
      a3 = a3 >> 8;
      a4 = a4 << 1;
      goto s_loop2;
    }
  
  if ((ip & 0x40000000) != 0) a4 = -a4;

  return a4;

} /* __divide */


signed long int
__remainder(
	    signed long int 	a1,
	    signed long int	a2 )
{
  signed   long int 	a4, ip;
  unsigned long int	a3, a2u;

  
  ip = (a2 & 0x80000000) | ((unsigned long int)(a1 ^ a2) >> 1);
  
  /*
   * ip bit 31  sign of dividend (= sign of remainder)
   *    bit 30  sign of dividend EOR sign of divisor (= sign of quotient)
   * these are packed into one register with a view to avoiding the
   * need for more work registers and the potential for extra register
   * save overhead.
   */
  
  if (a2 < 0)
    a2 = -a2;
  
  if (a1 < 0)
    a1 = -a1;
  
  switch (a1)
    /*
     * See commentary relating to the case of unsigned division.
     * It seems reasonable to put this after the place where I have
     * made a1 positive, since I need a high & low test on the range of a1
     * anyway.
     */
    {
    case 0:
      return ip < 0 ? -a2 : a2;
      
    case 1:
      /*
       * The only case where signed division can give an overflow (as distinct from
       * having a division by zero) is when dividing INT_MIN by -1.  Here is
       * where I have to detect that case.  There are three plausible things to do:
       * (a) raise an exception, (b) return a quotient of 0 and a remainder
       * of INT_MIN and (c) return a quotient of -INT_MIN and a remainder of 0,
       * where in case (c) the quotient overflows and yields a bit pattern that
       * is the same as INT_MIN {I am blindly assuming twos complement 32 bit
       * arithmetic throughout this code}.
       */
      return 0;
      
    case 2:
      a2 = a2 & 1;
      return ip < 0 ? -a2 : a2;
      
    case 3:
      /*
       * Here I started with an unsigned dividend, so after taking its absolute
       * value the biggest value I can have is 0x80000000.  The result is that,
       * compared with the unsigned case, I have 1 bit extra available for use
       * as a guard bit.  This improves the accuracy of the estimate that I
       * generate quite usefully.
       */
      a1  = a2 + ((unsigned long int)a2 >> 2);
      a1 += ((unsigned long int)a1 >> 4);
      a1 += ((unsigned long int)a1 >> 8);
      a1 += ((unsigned long int)a1 >> 16);
      a1  = ((unsigned long int)a2 >> 2) + ((unsigned long int)a1 >> 4);
      a2  = a2 - a1 - (a1 << 1);
      
      if (a2 >= 3) a2 -= 3;
      return ip < 0 ? -a2 : a2;
      
    case 4:
      a2 = a2 & 3;
      return ip < 0 ? -a2 : a2;

    case 5:
      a1  = a2 + ((unsigned long int)a2 >> 1);
      a1 += ((unsigned long int)a1 >> 4);
      a1 += ((unsigned long int)a1 >> 8);
      a1 += ((unsigned long int)a1 >> 16);
      a1  =  (unsigned long int)a1 >> 3;
      a2  = a2 - a1 - (a1 << 2);
      if (a2 >= 5) a2 -= 5;
      return ip < 0 ? -a2 : a2;
      
    case 6:
      a1  = a2 + ((unsigned long int)a2 >> 2);
      a1 += ((unsigned long int)a1 >> 4);
      a1 += ((unsigned long int)a1 >> 8);
      a1 += ((unsigned long int)a1 >> 16);
      a1  =  (unsigned long int)a1 >> 3;
      a3  = a1 + (a1 << 1);
      a2 -= (a3 << 1);
      if (a2 >= 6) a2 -= 6;
      return ip < 0 ? -a2 : a2;
      
    case 7:
      a1  = a2 + ((unsigned long int)a2 >> 3);
      a1 += ((unsigned long int)a1 >> 6);
      a1 += ((unsigned long int)a1 >> 12);
      a1 += ((unsigned long int)a1 >> 24);
      a1  =  (unsigned long int)a1 >> 3;
      a2  = a2 + a1 - (a1 << 3);
      
      if (a2 >= 7) a2 -= 7;
      return ip < 0 ? -a2 : a2;
      
    case 8:
      a2 = a2 & 7;
      return ip < 0 ? -a2 : a2;
      
    case 9:
      a1  = a2 - ((unsigned long int)a2 >> 3);
      a1 += ((unsigned long int)a1 >> 6);
      a1 += ((unsigned long int)a1 >> 12);
      a1 += ((unsigned long int)a1 >> 24);
      a1  =  (unsigned long int)a1 >> 3;
      a2  = a2 - a1 - (a1 << 3);
      
      if (a2 >= 9) a2 -= 9;
      return ip < 0 ? -a2 : a2;
      
    case 10:
      a1  = a2 + ((unsigned long int)a2 >> 1);
      a1 += ((unsigned long int)a1 >> 4);
      a1 += ((unsigned long int)a1 >> 8);
      a1 += ((unsigned long int)a1 >> 16);
      a1  =  (unsigned long int)a1 >> 4;
      a3  = a1 + (a1 << 2);
      a2 -= (a3 << 1);
      
      if (a2 >= 10) a2 -= 10;
      return ip < 0 ? -a2 : a2;
      
    case 11:
      a1  = a2 - ((unsigned long int)a2 >> 3);
      a1 += ((unsigned long int)a2 >> 5);
      a1 += ((unsigned long int)a2 >> 9);
      a1 += ((unsigned long int)a1 >> 10);
      a1 += ((unsigned long int)a1 >> 20);
      a1  = ((unsigned long int)a2 >> 1) + ((unsigned long int)a1 >> 2);
      a1  = ((unsigned long int)a1 >> 3);
      a3  = a1 + (a1 << 1);
      a2  = a2 + a1 - (a3 << 2);
      
      if (a2 >= 11) a2 -= 11;
      return ip < 0 ? -a2 : a2;
      
    case 12:
      a1  =  (unsigned long int)a2 >> 1;
      a1 += ((unsigned long int)a1 >> 2);
      a1 += ((unsigned long int)a1 >> 4);
      a1 += ((unsigned long int)a1 >> 8);
      a1 += ((unsigned long int)a1 >> 16);
      a1  =  (unsigned long int)a1 >> 3;
      a3  = a1 + (a1 << 1);
      a2 -= (a3 << 2);
      
      if (a2 >= 12) a2 -= 12;
      return ip < 0 ? -a2 : a2;
      
    case 13:
      a1  = a2 - ((unsigned long int)a2 >> 4);
      a1 -= ((unsigned long int)a2 >> 6);
      a1 += ((unsigned long int)a2 >> 10);
      a1 += ((unsigned long int)a1 >> 12);
      a1 += ((unsigned long int)a1 >> 24);
      a1  = ((unsigned long int)a2 >> 1) + ((unsigned long int)a1 >> 3);
      a1  =  (unsigned long int)a1 >> 3;
      a3  = a1 + (a1 << 1);
      a2  = a2 - a1 - (a3 << 2);
      
      if (a2 >= 13) a2 -= 13;
      return ip < 0 ? -a2 : a2;
      
    case 14:
      a1  =  (unsigned long int)a2 >> 1;
      a1 += ((unsigned long int)a1 >> 3);
      a1 += ((unsigned long int)a1 >> 6);
      a1 += ((unsigned long int)a1 >> 12);
      a1 += ((unsigned long int)a1 >> 24);
      a1  =  (unsigned long int)a1 >> 3;
      a3  = (a1 << 3) - a1;
      a2  = a2 - (a3 << 1);
      
      if (a2 >= 14) a2 -= 14;
      return ip < 0 ? -a2 : a2;
      
    case 15:
      a1  =  (unsigned long int)a2 >> 1;
      a1 += ((unsigned long int)a1 >> 4);
      a1 += ((unsigned long int)a1 >> 8);
      a1 += ((unsigned long int)a1 >> 16);
      a1  =  (unsigned long int)a1 >> 3;
      a2  = a2 + a1 - (a1 << 4);
      
      if (a2 >= 15) a2 -= 15;
      return ip < 0 ? -a2 : a2;
      
    case 16:
      a2 = a2 & 15;
      return ip < 0 ? -a2 : a2;
      
    default:
      break;
    }
  
  a2u = a2;
  a3  = a1;
  a4  = 0;
  
 s_loop:
  if      (a2u <= a3)        goto s_sh0mod8;
  else if (a2u <= (a3 << 1)) goto s_sh1mod8;
  else if (a2u <= (a3 << 2)) goto s_sh2mod8;
  else if (a2u <= (a3 << 3)) goto s_sh3mod8;
  else if (a2u <= (a3 << 4)) goto s_sh4mod8;
  else if (a2u <= (a3 << 5)) goto s_sh5mod8;
  else if (a2u <= (a3 << 6)) goto s_sh6mod8;
  else if (a2u >  (a3 << 7))
    {
      a3 = a3 << 8;
      goto s_loop;
    }
  
 s_loop2:
  if (a2u >= (a3 << 7)) a2u -= (a3 << 7), a4++;
  a4 = a4 << 1;
  
 s_sh6mod8:
  if (a2u >= (a3 << 6)) a2u -= (a3 << 6), a4++;
  a4 = a4 << 1;
  
 s_sh5mod8:
  if (a2u >= (a3 << 5)) a2u -= (a3 << 5), a4++;
  a4 = a4 << 1;
  
 s_sh4mod8:
  if (a2u >= (a3 << 4)) a2u -= (a3 << 4), a4++;
  a4 = a4 << 1;
  
 s_sh3mod8:
  if (a2u >= (a3 << 3)) a2u -= (a3 << 3), a4++;
  a4 = a4 << 1;
  
 s_sh2mod8:
  if (a2u >= (a3 << 2)) a2u -= (a3 << 2), a4++;
  a4 = a4 << 1;
  
 s_sh1mod8:
  if (a2u >= (a3 << 1)) a2u -= (a3 << 1), a4++;
  a4 = a4 << 1;
  
 s_sh0mod8:
  if (a2u >= a3) a2u -= a3, a4++;
  if ((unsigned long int)a1 <= (a3 >> 1))
    {
      a3 = a3 >> 8;
      a4 = a4 << 1;
      goto s_loop2;
    }
  
  return (ip < 0) ? -a2u : a2u;
  
} /* __remainder */


signed long int
_sdiv10( long int a1 )
/*
 * Signed quotient & remainder on division by 10.
 */
{
  signed long int a2, a3, a4;

  
  if ((a4 = a1) < 0)
    a1 = -a1;

  a2 = a1;
  
  /*
   * The shift on the next line MUST be done unsigned so that dividing
   * 0x80000000 by 10 works properly.
   */

  a1 = (long int)((unsigned long int)a1 >> 1);
  
  /* See commentary for unsigned case to understand this code */
  
  a1 += (a1 >> 1);
  a1 += (a1 >> 4);
  a1 += (a1 >> 8);
  a1 += (a1 >> 16);
  a1  =  a1 >> 3;

  /* Now subtract 10*a1 from a2 to get the remainder */

  a3  = a1 + (a1 << 2);
  a2 -= (a3 << 1);
    
  if (a2 >= 10)
    {
      a1++;
      a2 -= 10;
    }

  /* Attach sign to quotient and remainder */

  if (a4 < 0)
    {
      a1 = - a1;
      a2 = - a2;
    }

  return a1;

} /* _sdiv10 */
#endif /* !ARM */


#ifdef __ARM
# ifndef size_t
#  define size_t unsigned int
# endif

/* Transputer version is in CLIB strings.h - not used by later version of */
/* the Norcroft compiler as this inlines _memcpy() automatically. */
/* Arm version now in kernel as _memcpy is used in std resident libs */

void *_memcpy(void *a, const void *b, size_t n)
/* copy memory assuming no overlap, word aligned etc */
/* Relies on sizeof(int)=sizeof(void *) and byte addressing.
   Used by compiler for structure assignments */
{   
    int *wa,*wb;
    n >>= 2;
    for (wa = (int *)a, wb = (int *)b; n-- > 0;) *wa++ = *wb++;
    return a;
}

/* Used by kernel for task static data initialisation. */
/* Old commented out version is in Util/ARM/string.c. */

void *memset(void *s, int c, size_t n)
{
    unsigned char *p = (unsigned char *)s;
    while (n > 0)
    {
        if (n >= 4 && ((int)p & 3) == 0)
        {   int w = 0x01010101 * (unsigned char)c;     /* duplicate 4 times */
            do *(int *)p = w, p += 4, n -= 4; while (n >= 4);
        }
        else
            *p++ = (unsigned char)c, n--;
    }
    return s;
}

#if 1 /* also assemblerversion */
#define _chararg int               /* arg spec for char when ANSI says int */
#define _copywords 1               /* do fast cpy/cmp if word aligned      */

#  define BYTESEX_EVEN
/* BYTESEX_EVEN or BYTESEX_ODD should be defined for ARM/370 respectively  */

/* The following magic check was designed by A. Mycroft. It yields a     */
/* nonzero value if the argument w has a zero byte in it somewhere. The  */
/* messy constants have been unfolded a bit in this code so that they    */
/* get preloaded into registers before relevant loops.                   */

#  define ONES_WORD   0x01010101
#  define EIGHTS_WORD 0x80808080
#  define nullbyte_prologue_() \
      int ones_word = ONES_WORD; int eights_word = ones_word << 7
#  define word_has_nullbyte(w) (((w) - ones_word) & ~(w) & eights_word)

void *memcpy(void *a, const void *b, size_t n)
/* copy memory (upwards) - it is an errof for args to overlap. */
/* Relies on sizeof(int)=sizeof(void *) and byte addressing.   */
{
#ifdef _copywords
    /* do it fast if word aligned ... */
    if ((((int)a | (int)b | (int)n) & 3) == 0)
    { int *wa,*wb;
      n >>= 2;
      for (wa = (int *)a, wb = (int *)b; n-- > 0;) *wa++ = *wb++;
    }
    else
#endif
    { char *ca,*cb;
      for (ca = (char *)a, cb = (char *)b; n-- > 0;) *ca++ = *cb++;
    }
    return a;
}
#endif

#endif /* __ARM */



/* end of nccsupport.c */
