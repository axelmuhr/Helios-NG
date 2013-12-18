// c40 assembler test file
//
// P.A.Beskeen Sept '91

/* block comment */

; c40 style line comment
* c40 style start of line comment (damn OAP FORTRAN hackers)

.main:
startofmod:

byte 1, 2, 3 ,4, 5

align

// 'C40 assembler code test:
// all of these instructions are taken from the examples in the TMS320C4x
// User Guide, page 11-21.

absf R4, r7

absf *++ar3(ir1), r4
|| stf r4, *-ar7(1)

absi	r0, r0

absi	r0

absi *-ar5(1), R5	||	sti r1,*ar2--(ir1)

addc r1,r5

addf *ar4++(ir1),r5

addc3 *+ar1(8),*+ar1(5),r4

addf3 *+ar1(2),*+ar1(8),r4

addf *+ar1(2),*+ar1(8),r4

addf3 *+ar3(ir1),r2,r5 || stf r4,*ar2

addf *+ar3(ir1),r2,r5 || stf r4,*ar2

addi r3, r7

addi3	0x24, r10, r11

addi3 *ar0--(ir0),r5,	r0

addi3 *+ar3(ir1),r2,r5 || sti r4,*ar2

and r1, r2

AND3	*AR4--(IR1), R3, R8

and3	*+ar1(ir0)	, r4,	r7    || sti r3,*ar2

andn	@0x980c,r2

ANDN3	r3,r2,r5

ash r1, r3

ash3	r1, r2, r3

ash3 r1,*ar6++(ir1),r0|| sti r5,*ar2

fred:

bz r0

bnzaf	fred

bNEat	r7

BnzD	36

br	somewhere

brd	somewhereelse

call	me

callnz	r5

somewhere:

cmpf	*+ar4,r6

cmpf3	r6,r7

cmpi	r3,r7

cmpi3	0x2,r8

dblt	ar3,r2

somewhereelse:

dbzd	ar5, 0x110

fix	r1,r2

fix	*++ar4(1),r1||sti r0,*ar2

float	*++ar2(2),r5

float *+ar2(ir0),r6 || stf r7,*ar1

frieee	*ar0, r11

frieee	*ar1, r7 || STF r4,*ar2

iack *ar5

me:

idle

laj	lajtolabel

lajz	r5

latnz	500

lb2 r1,r2

lbu2 r1,r2

lda	bk,	dp

lde	*ar6, r10

ldep ivtp,	ir0

ldf	@0x9800,r2

ldfz r3,r5

lajtolabel:

ldfi	*+ar2(1),r7

ldf *-ar1(ir0),r7 || ldf *ar7++(1), r3

ldf *ar2--(1),r1
|| stf r3,*ar4++(ir1)

ldhi	0x44, r2

ldi *-AR1(IR0),R5

LDIZ R4,r6

ldii	@0x985f,r3

ldi *-AR1(1),r7||ldi *ar7++(ir0),r1

ldi *-ar1(1),r2 
|| sti r7, *ar5++(IR0)

ldm	156,r2		// @@@ check again when float expr are supported

ldp	@809900, dp

ldp	@809900

ldpe	ar1, tvtp

LDPK	4634

LH0	R1,R2

LHU1	R1,R2

LSH	R4,R7

LSH3	R4, *AR4--(IR0), R3

LSH3 R7, *AR2--(1),R2
|| STI	R0,*+AR0(1)

LWL2 R1, R2

LWR1 AR1, R2

MB2 AR2, AR1

MH1	AR1,AR2

MPYF	R0,R2

MPYF3	R1,R2,R3

MPYF	R1,R2,R3

MPYF3	*AR5++(1),*--AR1(IR0),R0
 || ADDF3 R5,R7,R3

MPYF3 *-AR2(1),r7,r1 || stf r3,*ar0--(ir0)

mpyf3 r5, *++ar7(ir1),r0
 || subf3 r7,*ar3--(1),r2

mpyf3 *++ar7(ir1), r5, r0
 || subf3 r7,*ar3--(1),r2	// commutative mpy ops

mpyi	R1,R5

MPYI3	R2,R4,R5

MPYI3 R7,R5,R0 
|| ADDI3*-AR3,*AR5--(1),R3

MPYI3 *++AR0(1),R5,R7 || STI R2,*-AR3(1)

MPYI3 R2, *++AR0(1), R0 
 || SUBI *AR5--(IR1), R4, R2

TOM:

MPYI3 *++AR0(1), R2, R0  ||
SUBI *AR5--(IR1), R4, R2	// COMMUTATIVE MPY VERSION

MPYSHI	@32,r2

mpyshi3	r1,*ar1,r4

mpyuhi	r5,r4

mpyuhi3	r4,r7,r5

negb r5,r7

negf *++ar3(2),r1

negf *ar4--(1),r7 || stf r2,*++ar5(1)

negi	174,r5

negi *-ar3,r2 || sti r2,*ar1++

nop *ar3--(1)

norm r1,r2

not @56256,r4

not	*+ar2,r3 || sti r7,*--ar4(ir1)

or *++ar1(ir1),r2

or3 r3,r2,r6

or3 *++ar2,r5,r2 || sti r6,*ar1--

pop r3

popf r4

push r6

pushf r2

rcpf r10, r4

retiz

retilod

retsge

rnd r5,r2

there:

rol r3

rolc r3

ror r6

rorc r2

rptb there

rptbd thereagain

rpts ar5

rsqrf r4, r6

thereagain:

sigi	@4545, r7

stf	r3,@0x7f87

stfi	r3,*-ar4

stf r4,*ar3-- || stf	r3,*++ar5

sti	r4, @0x9882

stii r1, @0x23e

sti	r0, *++ar2(ir0)
|| sti	r5, *ar0

stik	3, *ar5

subb *ar5++(4),r5

subb3	44,r4,r6

subc	@0b101011,r1

subf *ar0--(ir1),r5

subf3 r1,r3,r5

subf3 
r1,
 *-ar4(ir1),
r0 
||
 stf 
r7,
 *+ar5(ir0)

subi	220,r7

subi3 0x34,r4,r8

subi3 r7,*+ar2(ir0),r1 || sti r3, *++ar7

subrb r4, r6

subrf @0x990c, r5

subri *ar5++(ir0),r3

swi

toieee *ar1,r5

toieee *ar4--(ir1),r1 || stf r4, *ar1++(1)

trapv	45

tstb	*-ar4(1),r5

tstb3	r3,r4

xor	r1, r2

xor3	0b11100011,r2,r3

xor3	*ar1++,r3,r3
||
sti r6,*-ar2(ir0)


// Optional syntax compatibility test
// Taken from the examples in the TMS320C4x User Guide, page 11-15.

// #1
negf r1

// #2
addi r1,r2,r4

// #3
cmpi	r0, *ar0

// #4
ldi	*+ar0(0),r1

addi3	*+ar0(0),r1,r2		/* -> */	addi3 *ar0,r1,r2

// #5

ldi	*ar0++(1),r0		/* -> */	ldi *ar0++,r0

// #7 - @@@ NOT YET implemented
label:		/* -> */
//label


// #8 - should cause error

// @@@ rm//	ldi *+ar0(),r0

// #9

br	fred			/* -> */	br @fred

// #10	@@@ not yet implemented

ldp	@45

ldp	fred-23, r1

ldp	@fred, r1

// #11 parallel instructions in either order

addi3		*+ar3(ir1),r2,r5
|| sti		r4,*ar2

sti		r4,*ar2
|| addi3	*+ar3(ir1),r2,r1


// #12 || can be written anywhere


sti		r4,*ar2		|| addi3	*+ar3(ir1),r2,r1
sti		r4,*ar2||addi3	*+ar3(ir1),r2,r1
addi3	*+ar3(ir1),r2,r5
		||		sti	r4,*ar2

// #13 if the second register in a // instruction is the same as the destination
// register, then the destination register can be ommitted.

		addi3	*+ar3(ir1),r2,r2	||	mpyi	*ar0, r0, r0
/* -> */	addi3	*+ar3(ir1),r2		||	mpyi	*ar0, r0


// #14 All commutative operations in // instructions can be written in either
// order
// @@@ yet to implement

// @@@ rm comment
//	addi *ar0,r1,r2		/* -> */	addi r1, *ar0, r2
//

* Thats it folks
		; sure is
			// Yea
				/* no comment! */


endofmod:
