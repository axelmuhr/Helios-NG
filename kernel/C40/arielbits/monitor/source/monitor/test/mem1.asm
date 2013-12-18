BITS 		.set	31
	.text


	ldp		BCR_ADDR
	ldi		@BCR_ADDR,ar0
	ldi		@BCR_VALS,r0
	sti		r0,*ar0
	ldi		@BCR_VALS+1,r0
	sti		r0,*+ar0(4)

        ldi    @VIC_A_A7,ar0     ; setup the VIC registers
        ldi    @VIC_D_A7,r0
        sti    r0,*ar0
        ldi    @VIC_A_B3,ar0
        ldi    @VIC_D_B3,r0
        sti    r0,*ar0
        ldi    @VIC_A_B7,ar0
        ldi    @VIC_D_B7,r0
        sti    r0,*ar0
        ldi    @VIC_A_C3,ar0
        ldi    @VIC_D_C3,r0
        sti    r0,*ar0
        ldi    @VIC_A_C7,ar0
        ldi    @VIC_D_C7,r0
        sti    r0,*ar0
        ldi    @VIC_A_CB,ar0
        ldi    @VIC_D_CB,r0
        sti    r0,*ar0
        ldi    @VIC_A_CF,ar0
        ldi    @VIC_D_CF,r0
        sti    r0,*ar0

        ldi    @VAC_A_00,ar0     ; setup the VAC registers
        ldi    @VAC_D_00,r0
        sti    r0,*ar0
        ldi    @VAC_A_01,ar0
        ldi    @VAC_D_01,r0
        sti    r0,*ar0
        ldi    @VAC_A_02,ar0
        ldi    @VAC_D_02,r0
        sti    r0,*ar0
        ldi    @VAC_A_03,ar0
        ldi    @VAC_D_03,r0
        sti    r0,*ar0
        ldi    @VAC_A_04,ar0
        ldi    @VAC_D_04,r0
        sti    r0,*ar0
        ldi    @VAC_A_05,ar0
        ldi    @VAC_D_05,r0
        sti    r0,*ar0
        ldi    @VAC_A_06,ar0
        ldi    @VAC_D_06,r0
        sti    r0,*ar0
        ldi    @VAC_A_07,ar0
        ldi    @VAC_D_07,r0
        sti    r0,*ar0
        ldi    @VAC_A_08,ar0
        ldi    @VAC_D_08,r0
        sti    r0,*ar0
        ldi    @VAC_A_09,ar0
        ldi    @VAC_D_09,r0
        sti    r0,*ar0
        ldi    @VAC_A_0A,ar0
        ldi    @VAC_D_0A,r0
        sti    r0,*ar0
        ldi    @VAC_A_0B,ar0
        ldi    @VAC_D_0B,r0
        sti    r0,*ar0
        ldi    @VAC_A_10,ar0
        ldi    @VAC_D_10,r0
        sti    r0,*ar0
        ldi    @VAC_A_13,ar0
        ldi    @VAC_D_13,r0
        sti    r0,*ar0
        ldi    @VAC_A_14,ar0
        ldi    @VAC_D_14,r0
        sti    r0,*ar0
        ldi    @VAC_A_1B,ar0
        ldi    @VAC_D_1B,r0
        sti    r0,*ar0
        ldi    @VAC_A_29,ar0
        ldi    *ar0,r0           ; read the VAC ID register
        sti    r0,*ar0           ; write the ID number back

;TAKE DSP 2 OUT OF RESET
	LDI	@CONTROL_REG,AR2
	LDI	@SETUP1,R0
	STI	R0,*AR2
	LDI	@SETUP2,R0
	STI	R0,*AR2

;Start Memory test


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





; VIC addresses
VIC_A_A7  .word   0BFFF0029h
VIC_A_B3  .word   0BFFF002Ch
VIC_A_B7  .word   0BFFF002Dh
VIC_A_C3  .word   0BFFF0030h
VIC_A_C7  .word   0BFFF0031h
VIC_A_CB  .word   0BFFF0032h
VIC_A_CF  .word   0BFFF0033h
; VIC data
VIC_D_A7 .word   056
VIC_D_B3 .word   070h
VIC_D_B7 .word   00Dh
VIC_D_C3 .word   010h
VIC_D_C7 .word   0FFh
VIC_D_CB .word   010h
VIC_D_CF .word   0FFh
; VAC addresses
VAC_A_00 .word   0BFFF4000h
VAC_A_01 .word   0BFFF4040h
VAC_A_02 .word   0BFFF4080h
VAC_A_03 .word   0BFFF40C0h
VAC_A_04 .word   0BFFF4100h
VAC_A_05 .word   0BFFF4140h
VAC_A_06 .word   0BFFF4180h
VAC_A_07 .word   0BFFF41C0h
VAC_A_08 .word   0BFFF4200h
VAC_A_09 .word   0BFFF4240h
VAC_A_0A .word   0BFFF4280h
VAC_A_0B .word   0BFFF42C0h
VAC_A_10 .word   0BFFF4400h
VAC_A_13 .word   0BFFF44C0h
VAC_A_14 .word   0BFFF4500h
VAC_A_1B .word   0BFFF46C0h
VAC_A_29 .word   0BFFF4A40h
; VAC data
VAC_D_00 .word   000000000h
VAC_D_01 .word   000000000h
VAC_D_02 .word   0FFC00000h
VAC_D_03 .word   034000000h
VAC_D_04 .word   0F0F10000h
VAC_D_05 .word   0FFF00000h
VAC_D_06 .word   001500000h
VAC_D_07 .word   0FEFB0000h
VAC_D_08 .word   0F0000000h
VAC_D_09 .word   008000000h
VAC_D_0A .word   00C000000h
VAC_D_0B .word   000000000h
VAC_D_10 .word   0E0140000h
VAC_D_13 .word   0F8100000h
VAC_D_14 .word   0EBEE0000h
VAC_D_1B .word   00CFF0000h

CON_REG .word   0BFFD8000h
SRAM_S  .word   07FFFFFFEh
PATTERN .word   0h

CONTROL_REG	.WORD	0BFFD8000H
SETUP1		.WORD	040E01H
SETUP2		.WORD	010040E01H



BCR_ADDR	.word	00100000h
BCR_VALS	.word	1dea4000h
		.word	19cc4000h
GSRAM	.word	08d000000h
GSIZE		.word	07FFFeh
LSRAM		.word	04000000h
LSIZE		.word	3ffffh

	.end