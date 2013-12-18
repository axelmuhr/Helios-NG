	.text
	.word	8		; PROM width
	.word	19CC4000H	; Local memory interface control register
	.word	1DEA4000H	; Global memory interface control register
;	.word	1939FF50H	; Local memory interface control register
:	.word	1939FF50H	; Global memory interface control register
	.word	END-START	; Length
	.word	04000000h
;	.word	002FF800h	; Address to put this stuff

	.sect	"EXEC"
START	ldp	SRAM_S
;Read from SRAM forever
	ldi	@PATTERN,r0
	ldi	@SRAM_S,ar0

LOOP	sti	r0,*ar0
	b	LOOP

SRAM_S	.word	07FFFFFFEh
PATTERN	.word	0AAAAAAAAh
END

	.word	0h		;Signal end of data stream
	.word	002FFC00H	;IVPT value
	.word   002FFC00H + 512	;TVTP value
	.word	0h		;IACK address

	.end