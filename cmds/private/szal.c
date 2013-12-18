/* ident	"@(#)cfront:szal.c	1.1" */
/*******************************************************************

	Copyright (c) 1986 AT&T, Inc. All Rights Reserved
   	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.

szal.c:

	C program to run on a machine to write a size/align file for
	the C++ translator.

	Most output line are on the form:
		typeX	sizeof(typeX)	alignment_requirement_for_typeX
	assumes

          -	that ``double d0; char c0;'' poses the worst alignment condition
          -	two's complement integer representation
          -	that a word is defined by a :0 field

*****************************************************************************/
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/private/RCS/szal.c,v 1.3 1994/03/08 12:49:42 nickc Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>

typedef int (*PF)();
struct st1 { char a; };
struct ss {
	double a0; char c0;
	char c1;
	double a00; char c00;
	short s1;
	double a2; char c2;
	int i1;
	double a3; char c3;
	long l1;
	double a4; char c4;
	float f1;
	double a6; char c5;
	double d1;
	double a7; char c6;
	char* p1;
	double a8; char c7;
	struct ss * p2;
	double a9; char c8;
	struct st1 oo;
	double a10; char c9;
	PF pf;
} oo;
struct st5 { char a; int :0; };	/* by definition: a word */
#ifdef ORIGINAL
struct st2 { char :2; };
#else
struct st2 { char x; };
#endif
struct st3 { int field :2; };
#ifdef ORIGINAL
struct st4 { char :2; char :2; };
#else
struct st4 { char y; char x; };
#endif

#ifdef ORIGINAL
struct st6 { char v[3]; char : 2; };	/* fits in 4 bytes */
#else
struct st6 { char v[3]; char x; };	/* fits in 4 bytes */
#endif
struct st7 { char v[3]; int : 2; };	/* might not */
#ifdef ORIGINAL
struct st8 { char v[3]; char : 2; char : 2; };
#else
struct st8 { char v[3]; char y; char x; };
#endif
#ifdef ORIGINAL
struct st9 { char v[7]; char : 2; };	/* fits in 8 bytes */
#else
struct st9 { char v[7]; char x; };	/* fits in 8 bytes */
#endif
struct st10 { char v[7]; int : 2; };	/* might not */
#ifdef ORIGINAL
struct st11 { char v[7]; char : 2; char : 2; };
#else
struct st11 { char v[7]; char x; char y; };
#endif

void
out( char* s, int a1, int a2, char* p )
{
	printf("%s\t%d\t%d\t%s\n",s,a1,a2,p?p:"");
}

int a123456789 = 1;	/* if this does not compile get a better C compiler */
int a123456780 = 2;

int
main()
{
	char largest[50];
	char c = 1;
	int i1 = 0;
	int i2 = 0;
	int i = 1;

	if (a123456789 == a123456780)
		fprintf(stderr,"Warning: Your C compiler is dangerous.\nIt strips trailing characters off long identifiers without warning.\nGet a new one\n");

	while (c) { c<<=1; c&=~1; i1++; }	/* i1 = #bits in byte */

	if (sizeof(struct st5) == sizeof(char))	/* i2 = #bits in word  */
		i2 = i1;
	else if (sizeof(struct st5) == sizeof(short)) {
		short i = 1;
		while (i) { i<<=1; i&=~1; i2++; }
	}
	else if (sizeof(struct st5) == sizeof(int))
		while (i) { i<<=1; i&=~1; i2++; }
	else if (sizeof(struct st5) == sizeof(long)) {
		long i = 1;
		while (i) { i<<=1; i&=~1; i2++; }
	}
	else {
		fprintf(stderr,"Warning: Your C compiler probably handles 0 lengths fields wrong\n");
		i = sizeof(int);
	}

	out("bit",i1,i2,0);
	out("word",sizeof(struct st5),sizeof(struct st5),0);
	out("char",sizeof(char),(int)&oo.c1-(int)&oo.c0,0);
	out("short",sizeof(short),(int)&oo.s1-(int)&oo.c00,0);
	i = ((unsigned)~0)>>1;
	sprintf(largest,"%d",i);	/* largest integer */
	out("int",sizeof(int),(int)&oo.i1-(int)&oo.c2,largest);
	out("long",sizeof(long),(int)&oo.l1-(int)&oo.c3,0);
	out("float",sizeof(float),(int)&oo.f1-(int)&oo.c4,0);
	out("double",sizeof(double),(int)&oo.d1-(int)&oo.c5,0);
	i = 1<<(sizeof(char*)*i1-2); 
	if (i<400*1024)
		fprintf(stderr,"Pointers to data too small to handle C++\n");
	out("bptr",sizeof(char*),(int)&oo.p1-(int)&oo.c6,0);
	out("wptr",sizeof(struct ss *),(int)&oo.p2-(int)&oo.c7,0);
	i = 1<<(sizeof(PF)*i1-2); 
	if (i<250*1024)
		fprintf(stderr,"Pointers to functions too small to handle C++\n");
/*	out("fptr",sizeof(PF),(int)&oo.pf-(int)&oo.c9,0);
*/
	if (sizeof(PF)!=sizeof(struct ss*))
		fprintf(stderr,"Cannot handle sizeof(pointer to function) != sizeof(pointer to struct)\n");
	out("struct",sizeof(struct st1),(int)&oo.oo-(int)&oo.c8,0);
	switch (sizeof(struct st1)) {
	case 1:
		i1 = sizeof(struct st2)!=sizeof(struct st3);
		i2 = sizeof(struct st2)==sizeof(struct st4);
		break;
	case 2:
		i1 = sizeof(struct st6)!=sizeof(struct st7);
		i2 = sizeof(struct st6)==sizeof(struct st8);
		break;
	case 4:
		i1 = sizeof(struct st9)!=sizeof(struct st10);
		i2 = sizeof(struct st9)==sizeof(struct st11);
		break;
	default:
		fprintf(stderr,"Cannot figure out if field sizes are sensitive to the type of fields\n");
	}
	out("struct2",i1 /* sensitive to field type */,i2 /* packs fields */,0);
}
