xxxfred:
// test assembler source for ARM
// P.A.Beskeen 8/1/92
// examples taken from VL86C010 RISC faimily data manual

; simple tests
	Cmp	r1, #23		; check optional '#'
	beq	Fred1
Fred1:
	CMP	r1, 23		; check different cases
	beq	Fred2
Fred2:

	cmp	R0, #23
	CMPNE	r5, #34
	beq	Fred2


UnsMult32:	// unsigned 32bit multiply r2 = r0 * r1
	mov	r2, #0
loop:	movs	r0, r0 lsr #1
	addcs	r2, r2, r1
	add	r1, r1, r1
	bne	loop

	swi	32

hoopy:	int 0xfffff >> 2 & 32 / 10 * 5 ^ (0b10101010 | 23)

	ldr	r0, hoopy
	bne	loop

// line end comment

; trad line end comment

/* block comments *//* block comments */
/*
 * 
 * bollock comment
 */

; 	branches
there:	bal	john

	b	there


SomeWhere:		// labels are case sensitive
	cmp	r1,#0
	beq	fred

	bl	fred + 8

john:	adds	r1, #1
	blcc	SomeWhere

	blnv	john		; no-op

	; return from subroutine
	; valid link reg	ret addr on stack
	movs	pc, r14

			ldmfa	sp ! , { pc } ^	; restore psr

				ldmfa	r0 , r1 ^
				ldmfa	r0, { r1 }	; restore psr
				ldmfa	r0, r1
	mov	pc, r14
				ldmfa	sp!,{pc}	; dont restore psr

	; logical ops
fred:
	addeq	r2, r4,r5
	// sdhjsd dsj
	teqs	r4, #3
	; esjsdfjdsf
	sub	r4, r5, r7 asr #4
	sub	r4, r5, r7 lsr r2
	sub	r4, r5, r7 rrx
	sub	r4, r5, r7

	teqp	r15,0
	TEQP	R15,#0	// and upper case versions of mnemonics and regs

	movnv	r0,r0

	mov	pc,r14
	MOV	PC,R14

	mov	r1, 0xff000000		; test auto shift on expr

	; multiply / multiply accumulate

	mul	r1, r2, r3
	mla	r1, r2, r3, r4

	; synthesise 64 bit result from 32 bit.

	mov	r0, r1 lsr 16		; r0 * r1 = r3:r4
	mov	r4, r2 lsr 16
	bic	r1, r1, r0 lsl 16
	bic	r2, r2, r4 lsl 16
	mul	r3, r0, r2
	mul	r2, r0, r2
	mul	r1, r4, r1
	mul	r4, r0, r4
	adds	r1, r2, r1
	addcs	r4, r4, 0x10000
	adds	r3, r3, r1 lsl 16	; r3 bottom 32 bits
	adds	r4, r4, r1 lsr 16	; r4 top 32 bits


	; load and stores

	; example load of two 16 bit words, saved in 32bit buffer
	ldr	r3, io_16
	; --> lea	r4, buf
	add	r4, pc, buffer - 8	; limited addessability

	ldr	r0, mask
	ldr	r1, [r3], 2
	and	r1, r1, r0
	ldr	r2,[r3],2
	bic	r2,r2,r0
	orr	r1,r1,r2
	str	r1,[r4],4

mask:	word	0xffff
io_16:	word	0x3100000
buffer:	word	0

	; test other combinations
	; preindexed
	str	r1,[r2,r3]!
	str	r3, [r2]
	ldr	r6, [r12,16]
	ldr	r6, [r12,#16]
	ldr	r1,[r12, r2 lsr #2]	// xxx
	ldreqb	r1,[r2,5]

	; posindexed
	str	r1,[r8],r4
	str	r1,[r8],r4
	ldr	r1,[r0],16
	ldr	r9,[r5], r0 asr 3
	ldreqb	r2,[r5],5


	ldr	r1, place
	str	r1, fred9

	swinv	4	; padding
	swinv	1
	swinv	1

place:	word	0x1
fred9:	word	0x1


	; store/load multiples

	ldmfd	sp!,{r0,r1,r2}

	stmia	r10,{r0,r15}

	stmed	sp!,{r0-r3,r14}

	bl	SomeWhere

	ldmed	sp!,{r0-r3,r15}


	ldmfd	sp!,{r0-r2,r4-r8,r3}^

	; single data swap
	swp	r0, r1, [r5]
	swpb	r9, r8, [r9]
	swpeq	r9, r3, [r2]


	; swi's
	swi	0xff | 0b0101010 & (((12 << 5) * 3) / 8 + 234)

	swi	0xff000 | 'f'

	swine	0

	; swi handler
	b	Super	; swi entry point

Super:
	stmfd	sp!,{r0,r1,r2,r14}
	bic	r1,r14, 0xfc000003
	ldr	r0,[r1, -4]
	bic	r0, r0, 0xff000000
	mov	r1, switable
	ldr	pc, [r1, r0 lsl 2]

switable:
	word	readchar
	word	writechar

readchar:
	ldmfd	sp,{r0-r2, pc}^

writechar:
	ldmfd	sp,{r0-r2, pc}^


	; coprocessor data ops
	cdo	15, 10, cr1, cr7, cr2
	cdoeq	2,5,cr1,cr2,cr3, 2

	; coproc data tran
	stc	15, cr1, [r2]
	stc	15, cr1, [r2], 8
	ldc	3, cr1, [r0, 16]
	ldceq	5,cr2,[r5, 12]!
	stc	7, cr1,[r2]
	stc	7, cr1,[r2], 8!
	ldc	9,cr1,[r0],16
	ldceql	1,cr2,[r5],4

	stc	3, cr5, this
	b across21
this:	word 0
across21:

	; coproc reg trans
	mcr	1,6,r1,cr7,cr2
	mcreq	2,5,r2,cr2,cr3,2

	; misc stuff
alabel:
	cmp	r1, 23
	beq	alabel
	cmp	r1, 21
	beq	alabel

	cmp	r1, 23
	cmpne	r1, 21
	beq	alabel

	teq	r1,0
	rsbmi	r1,r1,0

	mov	r2, r0 lsl 2
	cmp r1, 5
	addcs	r2,r2,r0
	addhi	r2,r2,r0

	teq	r2, 127
	cmpne	r2, ' ' -1
	movls	r2, '.'

	; div with rem
	mov	r4,1
div1:	cmp	r1, 0x80000000
	cmpcc	r1,r0
	movcc	r1,r1 lsl 1
	bcc	div1
	mov	r2, 0
div2:	cmp r0,r1
	subcs	r0,r0,r1
	addcs	r2,r2,r4
	movs	r2,r4 lsr 1
	movne	r1,r1 lsr 1
	bne	div2


	; rnd
	tst	r1, r1 lsr 1
	movs	r2, r0 rrx
	adc	r1, r1, r1
	eor	r2, r2, r0 lsl 12
	eor	r0, r2, r2 lsr 20

	; misc
	mov r0, r0 lsl 2

	add r0, r0, r0 lsl 2

	rsb	r0, r0, r0 lsl 3

	add	r0, r0, r0 lsl 1
	add	r0, r0 lsl 1

	add	r0, r0, r0 lsl 2
	add	r0, r3, r0 lsl 2

	
	; test pseudo ops

	int	245, 457, 0x35ff, 0b0101010

	byte	"gdshfgd fhdfg", 0

	align

	word	"dsjh", 232

	word	'1', '45', '56', 0x0
	char	'1', 'h', '5', 0x0

