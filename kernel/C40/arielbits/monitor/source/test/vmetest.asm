        .text
        .word   8               ; PROM width
        .word   19CC4000H       ; Local memory interface control register
        .word   1DEA4000H       ; Global memory interface control register
;       .word   1939FF50H       ; Local memory interface control register
;       .word   1939FF50H       ; Global memory interface control register
        .word   END-START       ; Length
;       .word   04000000h
        .word   002FF800h       ; Address to put this stuff

        .sect   "EXEC"
START   ldi	@EPROM,ar0	; load EPROM address into ar0
	ldi 	*ar0,r0		; read from the EPROM space
	ldi	@PATTERN,r0	; load zero into r0
	sti 	r0,*ar0		; write a zero to 0FF000000h
	ldp     CON_REG
        ldi     @PATTERN,r0
        ldi     @CON_REG,ar0
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
	ldi    @VAC_A_0C,ar0
	ldi    @VAC_D_0C,r0
	sti    r0,*ar0
	ldi    @VAC_A_0D,ar0
	ldi    @VAC_D_0D,r0
	sti    r0,*ar0
	ldi    @VAC_A_0E,ar0
	ldi    @VAC_D_0E,r0
	sti    r0,*ar0
	ldi    @VAC_A_0F,ar0
	ldi    @VAC_D_0F,r0
	sti    r0,*ar0
        ldi    @VAC_A_10,ar0
        ldi    @VAC_D_10,r0
        sti    r0,*ar0
	ldi    @VAC_A_11,ar0
	ldi    @VAC_D_11,r0
	sti    r0,*ar0
	ldi    @VAC_A_12,ar0
	ldi    @VAC_D_12,r0
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

LOOP    b      LOOP
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
VAC_A_0C .word   0BFFF4300h
VAC_A_0D .word   0BFFF4340h
VAC_A_0E .word   0BFFF4380h 
VAC_A_0F .word   0BFFF43C0h
VAC_A_10 .word   0BFFF4400h
VAC_A_11 .word   0BFFF4440h
VAC_A_12 .word   0BFFF4480h
VAC_A_13 .word   0BFFF44C0h
VAC_A_14 .word   0BFFF4500h
VAC_A_1B .word   0BFFF46C0h
VAC_A_29 .word   0BFFF4A40h
; VAC data
VAC_D_00 .word   000F80000h
VAC_D_01 .word   000400000h 
VAC_D_02 .word   0FF000000h
VAC_D_03 .word   034000000h
VAC_D_04 .word   0F0F10000h
VAC_D_05 .word   000010000h
VAC_D_06 .word   000020000h
VAC_D_07 .word   0FDFF0000h
VAC_D_08 .word   07F000000h
VAC_D_09 .word   02C000000h
VAC_D_0A .word   04C000000h
VAC_D_0B .word   004000000h
VAC_D_0C .word   0F8100000h
VAC_D_0D .word   0F8100000h
VAC_D_0E .word   0E0100000h
VAC_D_0F .word   0F8100000h
VAC_D_10 .word   0E0140000h
VAC_D_11 .word   0F8100000h
VAC_D_12 .word   0F8100000h
VAC_D_13 .word   0F8100000h
VAC_D_14 .word   0EBFE0000h
VAC_D_1B .word   01FFF0000h
;
LOC_A_MICR  .word 0100004h
LOC_D_MICR  .word 019CC4000h
GLOB_A_MICR .word 0100000h
GLOB_D_MICR .word 01DEA4000h
;
CON_REG .word   0BF7FC008h
SRAM_S  .word   07FFFFFFEh
PATTERN .word   0h
EPROM	.word   0BFC00000h
END


        .word   0h              ;Signal end of data stream
        .word   002FFC00H       ;IVPT value
        .word   002FFC00H + 512 ;TVTP value
        .word   0h              ;IACK address

        .end
