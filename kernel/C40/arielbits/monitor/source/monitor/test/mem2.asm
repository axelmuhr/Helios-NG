BITS 		.set	31
	.text


	ldp		BCR_ADDR
	ldi		@BCR_ADDR,ar0
	ldi		@BCR_VALS,r0
	sti		r0,*ar0
	ldi		@BCR_VALS+1,r0
	sti		r0,*+ar0(4)


	ldi		0,r7
TOP

;WRITE WALKING BITS TEST  DATA  TO THE GLOBAL SRAM
	ldi		@GSRAM,ar0
	ldi		@GSIZE,r1
GWRITE	ldi		BITS,rc
	ldi		1,r0
	rptb		GWWALK
	sti		r0,*ar0++
GWWALK	lsh		1,r0
	subi		BITS+1,r1
	bgt		GWRITE

;READ WALKING BITS TEST DATA FROM THE GLOBAL SRAM
	ldi		@GSIZE,r1
   ldi		@GSRAM,ar0
GREAD	ldi		BITS,rc
	ldi		1,r0
	rptb		GRWALK
	ldi		*ar0++,r2
	cmpi		r2,r0
	bne		OOPS
GRWALK	lsh		1,r0
	subi		BITS+1,r1
	bgt		GREAD


;WRITE ADDRESS STORE TEST DATA TO THE GLOBAL SRAM
	ldi		@GSRAM,ar0
	ldi		@GSIZE,rc
	rptb		GWADDR
	ldi		ar0,r0
GWADDR	sti		r0,*ar0++
;READ ADDRESS STORE TEST DATA FROM THE GLOBAL SRAM
	ldi		@GSRAM,ar0
	ldi		@GSIZE,rc
	rptb		GRADDR
	ldi		*ar0,r0
	cmpi		r0,ar0
	bne		ERROR
GRADDR	addi		1,ar0


;WRITE WALKING BITS DATA TO THE LOCAL SRAM
	ldi		@LSRAM,ar0
	ldi		@LSIZE,r1
LWRITE	ldi		BITS,rc
	ldi		1,r0
	rptb		LWWALK
	sti		r0,*ar0++
LWWALK	lsh		1,r0
	subi		BITS+1,r1
	bgt		LWRITE

;READ WALKING BITS TEST DATA FROM THE LOCAL SRAM
	ldi		@LSIZE,r1
   ldi		@LSRAM,ar0
LREAD	ldi		BITS,rc
	ldi		1,r0
	rptb		LRWALK
	ldi		*ar0++,r2
	cmpi		r2,r0
	bne		OOPS
LRWALK	lsh		1,r0
	subi		BITS+1,r1
	bgt		LREAD

;WRITE ADDRESS STORE TEST DATA TO THE LOCAL SRAM
	ldi		@LSRAM,ar0
	ldi		@LSIZE,rc
	rptb		LWADDR
	ldi		ar0,r0
LWADDR	sti		r0,*ar0++
;READ ADDRESS STORE TEST DATA FROM THE LOCAL SRAM
	ldi		@LSRAM,ar0
	ldi		@LSIZE,rc
	rptb		LRADDR
	ldi		*ar0,r0
	cmpi		r0,ar0
	bne		ERROR
LRADDR	addi	1,ar0


	b		TOP



HERE	b		HERE

ERROR	ldi		05555h,r7
	b		HERE

OOPS	ldi		0a5a5h,r7
	b		HERE


BCR_ADDR	.word	00100000h
BCR_VALS	.word	1dea4000h
		.word	19cc4000h
GSRAM	.word	08d080000h
GSIZE		.word	07FFFFh
;GSRAM	.word	0c0000000h
;GSIZE		.word	3ffffh
LSRAM		.word	04000000h
LSIZE		.word	3ffffh

	.end