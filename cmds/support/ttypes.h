/* Sccs Id: "@(#)ttypes.h	1.3\t18/9/87 Copyright (C) Perihelion Software Ltd.	*/

#ifndef __helios_h
#define __helios_h

/* standard type definitions */

typedef  long           WORD    ;       /* a machine word, 32 bits      */
typedef  unsigned long  UWORD   ;       /* a machine word, 32 bits      */
typedef  WORD           INT     ;       /* a synonym                    */
typedef  WORD           word    ;       /* a synonym                    */
typedef  char           BYTE    ;       /* a byte, used in place of char*/
typedef  char           byte    ;       /* a byte, used in place of char*/
typedef  unsigned char  UBYTE   ;       /* an unsigned byte             */
typedef  char *         STRING  ;       /* character string             */
typedef  void	(*VoidFnPtr)()	;	/* pointer to function		*/
typedef  word	(*WordFnPtr)()	;	/* pointer to function		*/
typedef  char *         APTR    ;       /* a machine pointer            */
typedef  WORD           RPTR    ;       /* a self relative pointer      */
typedef	 int		bool	;	/* boolean value		*/

#define Variable	1

#define PUBLIC
#define PRIVATE		static

#define  TRUE           1l
#define  FALSE          0l

#define RTOA(x) ((APTR)(&(x)+(x)))      /* convert RPTR to APTR         */
#define ATOR(x) ((RPTR)((x)-&(x)))      /* convert APTR to RPTR         */

#define MinInt          0x80000000l

#define TargetBytesPerWord 4

#endif
