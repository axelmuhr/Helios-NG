	.sect "BOOT"
	ldp	BOOT_ADDR
	ldi	@BOOT_ADDR,r0
	b	r0

	.globl	_c_int00
BOOT_ADDR	.word	_c_int00

	.end