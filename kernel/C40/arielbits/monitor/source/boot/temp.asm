	ldi	@CON_REG,ar0
	ldi	@LED_ON,r6
	ldi	@LED_OFF,r7
	ldi	@WAIT_COUNT,r5
LOOP:	
	sti	r6,*ar0
	ldi	r5,r4
WAIT_ON	subi	1,r4
	bnz	WAIT_ON
	sti	r7,*ar0
	ldi	r5,r4
WAIT_OFF	subi	1,r4
	bnz	WAIT_OFF
	b      LOOP


CON_REG .word   0BFFD8000h
SRAM_S  .word   07FFFFFFEh
PATTERN .word   0h

LED_ON	.word	080000000h
LED_OFF	.word	000000000h
WAIT_COUNT	.word	0100000h

	.end