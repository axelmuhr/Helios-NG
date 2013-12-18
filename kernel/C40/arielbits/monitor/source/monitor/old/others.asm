FP	.set	ar3


**************************************************************************
*       Boot header                                                      *
**************************************************************************

START	.word	8			;Boot width in bits
	.word	3EF78000h		;Global BCR value
	.word	3EF78000h		;Local BCR value
	.word	BOTTOM - TOP		;Length of BOOT program
	.word	0002FF800h		;Where to put boot program (IRAM 0)




TOP	ldp	0002FFC00h		;RAM block 1
	ldi	@0002FFC00h,ar2
	ldi	DATA_B-DATA_T,r0
	addi3	r0,ar2,FP


;>>>> 		OUT_FIFO = IM_UP;
	LDI	*+ar2(0),AR0
	LDI	15,R0
	STI	R0,*AR0
Y1:
;>>>> 		while( !(COMM_CONT & 0xF00) );
	LDI	*+ar2(1),AR0
	LDI	*AR0,R0
	TSTB	3840,R0
	BZ	Y1
;>>>> 		sram_size = IN_FIFO;
	BD	Y2
	LDI	*+ar2(2),AR1
	LDI	*AR1,R0
	STI	R0,*+FP(3)
;>>>> 		switch( sram_size )
;>>>> 			case 1:
***	B	Y2	;BRANCH OCCURS
Y3:
;>>>> 				i = 0x10000;
	BD	Y4
	LDI	*+ar2(3),R0
	NOP
	STI	R0,*+FP(2)
;>>>> 				break;
;>>>> 			case 2:
***	B	Y4	;BRANCH OCCURS
Y5:
;>>>> 				i = 0x40000;
	BD	Y4
	LDI	*+ar2(4),R0
	NOP
	STI	R0,*+FP(2)
;>>>> 				break;
;>>>> 			case 0:
;>>>> 			default:
***	B	Y4	;BRANCH OCCURS
L7:
;>>>> 				i = 0x4000;
	BD	Y4
	LDI	16384,R0
	NOP
	STI	R0,*+FP(2)
;>>>> 				break;
***	B	Y4	;BRANCH OCCURS
Y2:
	BZ	L7
	CMPI	1,R0
	BZ	Y3
	CMPI	2,R0
	BZ	Y5
	B	L7
Y4:
;>>>> 		for( mem_ptr=0, failed=0 ; (unsigned long)mem_ptr < i ; mem_ptr++ )
	LDI	0,R1
	STI	R1,*+FP(1)
	STI	R1,*+FP(4)
	CMPI	R0,R1
	BHS	L9
L8:
;>>>> 			*mem_ptr = ZEROs;
	LDI	*+FP(1),AR0
	LDI	0,R0
	STI	R0,*AR0
;>>>> 			failed = *mem_ptr?failed++:failed;
	LDI	*AR0,R1
	BZ	YY3
	LDI	*+FP(4),R1
	BD	LL4
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
***	B	LL4	;BRANCH OCCURS
YY3:
	LDI	*+FP(4),R1
LL4:
	STI	R1,*+FP(4)
;>>>> 			*mem_ptr = Fs;
	LDI	-1,R2
	STI	R2,*AR0
;>>>> 			failed = *mem_ptr==Fs?failed:failed++;
	CMPI	R2,*AR0
	BZ	LL6
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
LL6:
	STI	R1,*+FP(4)
;>>>> 			*mem_ptr = As;
	LDI	*+ar2(5),R3
	STI	R3,*AR0
;>>>> 			failed = *mem_ptr==As?failed:failed++;
	CMPI	R3,*AR0
	BZ	LL8
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
LL8:
	STI	R1,*+FP(4)
;>>>> 			*mem_ptr = FIVEs;
	LDI	*+ar2(6),R3
	STI	R3,*AR0
;>>>> 			failed = *mem_ptr==FIVEs?failed:failed++;
	CMPI	R3,*AR0
	BZ	LL10
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
LL10:
	STI	R1,*+FP(4)
;>>>> 			if( failed )
	CMPI	0,R1
	BZ	L10
;>>>> 				OUT_FIFO =  0;
	LDI	*+ar2(0),AR1
	STI	R0,*AR1
;>>>> 				OUT_FIFO =  (unsigned long)mem_ptr;
	LDI	*+FP(1),R3
	STI	R3,*AR1
L11:
;>>>> 				DIE();
	B	L11
L10:
	LDI	*+FP(1),R3
	ADDI	1,R3
	STI	R3,*+FP(1)
	CMPI	*+FP(2),R3
	BLO	L8
L9:
;>>>> 		i += 0x80000000;
	LDI	*+FP(2),R0
	ADDI	*+ar2(7),R0
	STI	R0,*+FP(2)
;>>>> 		for( mem_ptr=(unsigned long *)0x80000000, failed=0 ; (unsigned long)mem_ptr < i ; mem_ptr++ )
	LDI	*+ar2(7),R2
	STI	R2,*+FP(1)
	LDI	0,R1
	STI	R1,*+FP(4)
	CMPI	R0,R2
	BHS	L13
L12:
;>>>> 			*mem_ptr = ZEROs;
	LDI	*+FP(1),AR0
	LDI	0,R0
	STI	R0,*AR0
;>>>> 			failed = *mem_ptr?failed++:failed;
	LDI	*AR0,R1
	BZ	LL11
	LDI	*+FP(4),R1
	BD	LL12
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
***	B	LL12	;BRANCH OCCURS
LL11:
	LDI	*+FP(4),R1
LL12:
	STI	R1,*+FP(4)
;>>>> 			*mem_ptr = Fs;
	LDI	-1,R2
	STI	R2,*AR0
;>>>> 			failed = *mem_ptr==Fs?failed:failed++;
	CMPI	R2,*AR0
	BZ	LL14
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
LL14:
	STI	R1,*+FP(4)
;>>>> 			*mem_ptr = As;
	LDI	*+ar2(5),R3
	STI	R3,*AR0
;>>>> 			failed = *mem_ptr==As?failed:failed++;
	CMPI	R3,*AR0
	BZ	LL16
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
LL16:
	STI	R1,*+FP(4)
;>>>> 			*mem_ptr = FIVEs;
	LDI	*+ar2(6),R3
	STI	R3,*AR0
;>>>> 			failed = *mem_ptr==FIVEs?failed:failed++;
	CMPI	R3,*AR0
	BZ	LL18
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
LL18:
	STI	R1,*+FP(4)
;>>>> 			if( failed )
	CMPI	0,R1
	BZ	L14
;>>>> 	      	OUT_FIFO =  0;
	LDI	*+ar2(0),AR1
	STI	R0,*AR1
;>>>> 				OUT_FIFO =  (unsigned long)mem_ptr;
	LDI	*+FP(1),R3
	STI	R3,*AR1
L15:
;>>>> 				DIE();
	B	L15
L14:
	LDI	*+FP(1),R3
	ADDI	1,R3
	STI	R3,*+FP(1)
	CMPI	*+FP(2),R3
	BLO	L12
L13:
;>>>> 		OUT_FIFO =  1;
	LDI	*+ar2(0),AR0
	LDI	1,R0
	STI	R0,*AR0


	idle
BOTTOM

**************************************************************************
*       Data header                                                      *
**************************************************************************

		.word	DATA_B - DATA_T		;Length of BOOT data
		.word	0002FFC00h		;Where to put data (IRAM 1)



******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
DATA_T          .word	0002FFC01h	 ;Pointer to this data
		.word 	1048690          ;0
		.word 	1048688          ;1
		.word 	1048689          ;2
		.word 	65536            ;3
		.word 	262144           ;4
		.word 	-1431655766      ;5
		.word 	1431655765       ;6
		.word 	-2147483648      ;7
VARS		.space	4
DATA_B

**************************************************************************
*       This is the trailer to the data stream                           *
**************************************************************************
		.word	0h		;Signal last block to loader
		.word	0002FF800h	;IVTP value loaded by loader
		.word	0002FF800h	;TVTP value loaded by loader
		iack	*ar0		;Signals end of boot sequence
STOP


		.end
