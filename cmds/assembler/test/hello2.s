        align
        module  -1
.ModStart:
        word    0x60f160f1
		word modsize
	        blkb    31, "Hello"
		byte 0
	word	modnum
	word	1
		word datasymb(.MaxData)
        init
		word	codesymb(.MaxCodeP)
	export .main				
	import .printf
		align
	__procname_main:
		byte	"main", 0
		align
		word 	0xff000000 | - __procname_main * 4
	.main:
	main:
	sti	R11, *--AR6			
	nop
	laj	4
		nop				
		patchinstr(PATCHC40MASK16ADD,
			shift(-2, labelref(hello_msg)),
			addi	-2, R11)	
		ldi	R11, R0
	lsh	2, R0
	laj	.printf				
		nop
		nop
		nop
	ldi	*AR6++, R11			
	b	R11				
hello_msg:
	byte	"Hello World via assembly language\n", 0
	data .MaxData, 0
		codetable .MaxCodeP
	align		
.ModEnd:
		end
