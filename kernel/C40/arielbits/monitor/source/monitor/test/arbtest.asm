        .text
START   ldp     CON_REG

	ldi	@MICR,ar0
	ldi	@MICR_VALS,r0
	sti	r0,*ar0
	ldi	@MICR_VALS+1,r0
	sti	r0,*+ar0(4)

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

;NOW READ AND WRITE THE VAC AD NAUSEUM
LOOP	LDI	*AR0,R0
	STI	R0,*AR0
	b	LOOP


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
;
LOC_A_MICR  .word 0100004h
LOC_D_MICR  .word 019CC4000h
GLOB_A_MICR .word 0100000h
GLOB_D_MICR .word 01DEA4000h
;
CON_REG .word   0BFFD8000h
SRAM_S  .word   07FFFFFFEh
PATTERN .word   0h

MICR			.word		0100000h
MICR_VALS		.word		01dea4000h
			.word		019cc4000h
CONTROL_REG	.WORD	0BFFD8000H
SETUP1		.WORD	040E01H
SETUP2		.WORD	010040E01H
END


        .word   0h              ;Signal end of data stream
        .word   002FFC00H       ;IVPT value
        .word   002FFC00H + 512 ;TVTP value
        .word   0h              ;IACK address

        .end
