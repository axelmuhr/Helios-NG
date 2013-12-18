	TTL	Display blitting code			> displaycode.s
	SUBT	(c) 1990, Active Book Company, Cambridge, United Kingdom.
	; ---------------------------------------------------------------------
	; Standalone screen device driver primitives.
	;
	; Author:		James G Smith
	; History:	900613	Created from lo-executive fragments.
	;		910225  Major updates required
	;
	; ---------------------------------------------------------------------
	; RCSId: $Id: displaycode.s,v 1.2 1991/03/20 15:41:42 paul Exp $

	GET	listopts.s	; assembly listing control directives
	GET	arm.s		; ARM processor description
	GET	basic.s		; default labels and variables
	GET	structs.s	; structure construction MACROs
	GET	module.s	; Helios object module construction MACROs
	GET	exmacros.s	; generic MACRO definitions
	GET	SWIinfo.s	; ARM Helios Executive provided SWIs
	GET	PCSregs.s	; standard ARM PCS register allocations

	OPT	(opt_on + opt_von + opt_mon + opt_mcon)

	; ---------------------------------------------------------------------

ptr_size	*	((32 * 16) / 8)	; number of bytes in the ptr definition
	; The cursor is actually 16x16 (but we pad out the X direction with
	; NULL bits).

	; ---------------------------------------------------------------------
	; character blitting control
char_bold	bit	0	; enbolden text
char_italic	bit	1	; italicise text
char_underline	bit	2	; underline text
char_inverse	bit	3	; invert text

	; ---------------------------------------------------------------------

	StartModule	displaycode,-1,100	; name,module slot,version

	; ---------------------------------------------------------------------

	static

	; EXPORTed functions
	static_extern_func	display_init		; setup the display
	static_extern_func	display_showcur		; EOR cursor to screen
	static_extern_func	display_scroll		; SOFT scroll
	static_extern_func	display_clear		; clear screen
	static_extern_func	blit_normal		; blit character
	static_extern_func	blit_reverse		; blit character

	; STATIC data requirements (exported)
	static_extern_word	screenbase		; base screen address
	static_extern_word	stride			; screen raster width
	static_extern_word	screenX			; screen pixel width
	static_extern_word	screenY			; screen raster height
	static_extern_word	attributes		; char blitting style
	static_extern_vec	ptr_size,ptr_shape	; pointer bitimage

	; STATIC data initialisation
	; "screenbase", "stride", "screenX" and "screenY" are initialised
	; at run-time.
	static_inittab		ptr_shape,ptr_size	; default pointer image

	; STATIC data requirements (internal)
	static_word		screensize		; screen size in bytes
	static_word		cursorState		; cursor ON/OFF state
	static_word		curXpos			; cursor X position
	static_word		curYpos			; cursor Y position
	static_vec		(16 * 8),dBuffer	; display buffer

	; STATIC data initialisation
	static_initword		cursorState,0		; cursor OFF initially
	static_initword		curXpos,320		; explicit position
	static_initword		curYpos,200		; explicit position

	static_end

	; ---------------------------------------------------------------------

display_init	FnHead
	STMFD	sp!,{v1,lk}

	SWI	exec_DisplayInfo	; read display state
	SADR	a1,screenbase
	STR	a4,[a1,#&00]		; display base address
	SADR	a1,stride
	STR	v1,[a1,#&00]		; display raster width in bytes
	SADR	a1,screenX
	BIC	a2,a2,#(32 - 1)		; ensure screenX is word multiple
	STR	a2,[a1,#&00]		; screen width in pixels
	SADR	a1,screenY
	STR	a3,[a1,#&00]		; screen height in rasters
	SADR	a1,screensize
	MUL	a2,v1,a3		; size = stride * screenY
	STR	a2,[a1,#&00]		; calculated display size in bytes
	SADR	a1,attributes
	MOV	a2,#&00000000
	STR	a2,[a1,#&00]		; all character display attributes off

	LDMFD	sp!,{v1,pc}^

	; ---------------------------------------------------------------------

display_showcur	FnHead
	; in:	r0 = OFF==0, ON!=0
	;	r1 = OFF==undefined, ON==X position
	;	r2 = OFF==undefined, ON==Y position
	; out:	no conditions
	; We keep a copy of the current cursor display state. If the cursor
	; is OFF then OFF requests are ignored. If the cursor is ON an OFF
	; request will remove it from the display (assuming last position
	; information is correct).
	; If the cursor is OFF an ON request will display it, remembering the
	; display position. If the cursor is ON then an ON request will
	; restore the previous cursor position and then display it at the
	; new position, remembering the display position.
	;
	; "ptr_shape" holds the current monochrome cursor shape. This should be
	; ORed onto the display. "dBuffer" should hold the un-tainted display
	; area where the cursor has been painted.
	;
	; We cannot use r9(dp) and r13(sp) as general work registers in these
	; routines.
	;
	STMFD	sp!,{r0,r1,r2,r3,r4,r5,r6,r7,r8,r10,r11,ip,lk}

	LDR	r3,cursorState		; load the current cursor state
	CMP	r0,#&00000000		; OFF or ON
	BEQ	OFFrequest		; turn the cursor OFF
ONrequest

	CMP	r3,#&00000000
	BLNE	restoreDisplay		; cursor is ON (restore display)
cursorisOFF
	; preserve the display at (r1,r2) into "dBuffer" writing the cursor
	; onto the screen at (r1,r2)

	STMFD	sp!,{r0,r1,r2}		; so we can use as work registers

					; r1 = cursor X pos
					; r2 = cursor Y pos
	LDR	r3,screenbase		; r3 = base address of the screen
	LDR	r4,stride		; r4 = stride between rasters
	MOV	r4,#256			; r4 = stride between rasters
	MLA	r3,r2,r4,r3		; r3 = base address of starting raster
	SADR	r5,ptr_shape		; r5 = cursor definition
	MOV	r6,r1,LSR #5		; r6 = wordpos within raster
	AND	r7,r1,#&1F		; r7 = bitpos within word
	ADD	r3,r3,r6,LSL #2		; byte position of word
	SADR	r0,dBuffer		; r0 = display buffer

	; loop 16 times
	MOV	r8,#&00000000		; loop counter
displayCursor
	LDR	r10,[r5,r8,LSL #2]	; fetch the cursor raster word
	ADD	r1,r0,r8,LSL #3		; "dBuffer" raster address
	MOV	r11,r10,LSL r7		; first word mask
	LDR	r2,[r3,#&00]		; load word from screen
	STR	r2,[r1,#&00]		; store word into the "dBuffer"
	ORR	r2,r2,r11		; OR in the cursor definition
	STR	r2,[r3,#&00]		; and store the word back again
	CMP	r6,#((640 / 32) - 1)	; check maximum word position
	BCS	noDisplay
	RSB	ip,r7,#32		; second word mask
	MOV	r11,r10,LSR ip
	LDR	r2,[r3,#&04]		; load word from screen
	STR	r2,[r1,#&04]		; store word into the "dBuffer"
	ORR	r2,r2,r11		; OR in the cursor definition
	STR	r2,[r3,#&04]		; and store the word back again
noDisplay
	ADD	r3,r3,r4		; step onto the next raster
	ADD	r8,r8,#&01
	CMP	r8,#16
	BCC	displayCursor

	LDMFD	sp!,{r0,r1,r2}		; recover entry register values

	ADR	r3,cursorState
	STR	r0,[r3,#&00]		; mark the cursor as ON
	ADR	r3,curXpos
	STR	r1,[r3,#&00]		; remember new cursor X position
	ADR	r3,curYpos
	STR	r2,[r3,#&00]		; remember new cursor Y position
	; and exit cleanly
	LDMFD	sp!,{r0,r1,r2,r3,r4,r5,r6,r7,r8,r10,r11,ip,pc}^

	; ---------------------------------------------------------------------

OFFrequest
	CMP	r3,#&00000000
	BLNE	restoreDisplay		; restore display from "dBuffer"
	ADRNE	r3,cursorState
	STRNE	r0,[r3,#&00]		; mark the cursor as OFF
	LDMFD	sp!,{r0,r1,r2,r3,r4,r5,r6,r7,r8,r10,r11,ip,pc}^

	; ---------------------------------------------------------------------

restoreDisplay
	STMFD	sp!,{r0,r1,lk}

	; r0..r1,r3..r8,r10,r11,ip are available as work registers
	; r2,r9,sp,lk should NOT be used
	; (curXpos,curYpos) defines the	restore position
	; "dBuffer" contains the display contents

	LDR	r11,curXpos		; r11 = cursor X pos
	LDR	ip,curYpos		;  ip = cursor Y pos
	LDR	r3,screenbase		;  r3 = base address of the screen
	LDR	r4,stride		;  r4 = stride between rasters
	MLA	r3,ip,r4,r3		;  r3 = base address of starting raster
	MOV	r6,r11,LSR #5		;  r6 = wordpos within raster
	AND	r7,r11,#&1F		;  r7 = bitpos within word
	ADD	r3,r3,r6,LSL #2		; byte position of word
	SADR	r0,dBuffer		;  r0 = display buffer
	; loop 16 times (once for every cursor raster)
	MOV	r8,#&00000000		; loop counter
restoreLoop
	ADD	r1,r0,r8,LSL #3		; address of data in "dBuffer"
	LDMIA	r1,{r5,r10}		; two words of possible display data
	CMP	r6,#((640 / 32) - 1)	; check maximum word position
	STMCSIA	r3,{r5}			; single word only to be restored
	STMCCIA	r3,{r5,r10}		; both words to be restored
	ADD	r3,r3,r4		; step onto the next raster
	ADD	r8,r8,#&01		; and increment the raster count
	CMP	r8,#16
	BCC	restoreLoop
	
	LDMFD	sp!,{r0,r1,pc}^

	; ---------------------------------------------------------------------

ptr_shape
	; This is the standard 16x16 cursor shape we use (reversed).
	&	2_0000000111111111
	&	2_0000000011111111
	&	2_0000000001111111
	&	2_0000000000111111
	&	2_0000000001111111
	&	2_0000000011111111
	&	2_0000000111110111
	&	2_0000001111100011
	&	2_0000011111000001
	&	2_0000111110000000
	&	2_0001111100000000
	&	2_0011111000000000
	&	2_0111110000000000
	&	2_1111100000000000
	&	2_1111000000000000
	&	2_1110000000000000

	LTORG

	; ---------------------------------------------------------------------
	; int display_scroll(int direction,int amount) ;
display_scroll	FnHead
	; Perform a SOFT scroll of the display. Note: this will be slow.
	; in:	r0 = scroll direction
	;		0	vertical scroll up (new row at bottom)
	;		1	vertical scroll down (new row at top)
	;		2	horizontal scroll left (new column at right)
	;		3	horizontal scroll right (new column at left)
	;	r1 = scroll amount (in pixels or rasters)
	; out:	r0 = preserved
	;	r1 = preserved
	;
	; We need to copy large amounts of screen data around (blegh!)
	;
	; At the moment this code assumes a 1bpp display mode.
	;
	; The following gives a slow exit in the error case, but does save
	; a word.
	CMP	r0,#scroll_options
	STMCCFD	sp!,{r1,r2,r3,r4,r5,r6,r7,r8,lk}
	ADRCC	r2,scroll_jtable
	ADDCC	pc,r2,r0,LSL #2
	B	scroll_badoption		; invalid reason code
scroll_jtable
	B	vscroll_up
	B	vscroll_down
	B	hscroll_left
	B	hscroll_right
end_scroll_jtable
scroll_options	*	((end_scroll_jtable - scroll_jtable) / 4)

vscroll_up
	; r1 = scroll amount
	LDR	r6,screenbase		; screen start
	LDR	r7,stride		; stride
	MLA	r5,r1,r7,r6		; r5 = (r1 * r7) + r6
	LDR	r7,screensize		; size of the display
	ADD	r7,r6,r7		; screen end address

	; calculate the number of 80pixel units in this the screen width
	LDR	r8,screenX		; screenX in pixels
	MOV	ip,#0			; index counter
calc_up_index_loop
	SUB	r8,r8,#80		; amount we move in a single transfer
	CMP	r8,#80			; check for termination
	ADDCS	ip,ip,#1		; next index
	BCS	calc_up_index_loop
	; ip = offset address table index
	; we only provide support upto 1024 horizontal display widths
	LDR	r8,stride		; current stride
	LDR	r0,screenX		; screenX width
	SUB	r8,r8,r0,LSR #3		; unused = stride - (screenX / 8)
	; r8 = calculated unused display width
copy_up_loop
	; branch into the relevant copy start point
	ADR	r0,up_jump_into		; address the offset table
	LDR	r0,[r0,#ip]		; get offset value
	ADD	pc,r0,pc		; branch into the code
	NOP				; padding to ensure correct addresses
up_PC_position
up_jump_into
	&	(up_screen80  - up_PC_position)
	&	(up_screen160 - up_PC_position)
	&	(up_screen240 - up_PC_position)
	&	(up_screen320 - up_PC_position)
	&	(up_screen400 - up_PC_position)
	&	(up_screen480 - up_PC_position)
	&	(up_screen560 - up_PC_position)
	&	(up_screen640 - up_PC_position)
	&	(up_screen720 - up_PC_position)
	&	(up_screen800 - up_PC_position)
	&	(up_screen880 - up_PC_position)
	&	(up_screen960 - up_PC_position)

up_screen960	; 960pixel display
	LDMIA	r5!,{r0,r1,r2,r3,r4}
	STMIA	r6!,{r0,r1,r2,r3,r4}
up_screen880	; 880pixel display
	LDMIA	r5!,{r0,r1,r2,r3,r4}
	STMIA	r6!,{r0,r1,r2,r3,r4}
up_screen800	; 800pixel display
	LDMIA	r5!,{r0,r1,r2,r3,r4}
	STMIA	r6!,{r0,r1,r2,r3,r4}
up_screen720	; 720pixel display
	LDMIA	r5!,{r0,r1,r2,r3,r4}
	STMIA	r6!,{r0,r1,r2,r3,r4}
up_screen640	; 640pixel display
	LDMIA	r5!,{r0,r1,r2,r3,r4}
	STMIA	r6!,{r0,r1,r2,r3,r4}
up_screen560	; 560pixel display
	LDMIA	r5!,{r0,r1,r2,r3,r4}
	STMIA	r6!,{r0,r1,r2,r3,r4}
up_screen480	; 480pixel display
	LDMIA	r5!,{r0,r1,r2,r3,r4}
	STMIA	r6!,{r0,r1,r2,r3,r4}
up_screen400	; 400pixel display
	LDMIA	r5!,{r0,r1,r2,r3,r4}
	STMIA	r6!,{r0,r1,r2,r3,r4}
up_screen320	; 320pixel display
	LDMIA	r5!,{r0,r1,r2,r3,r4}
	STMIA	r6!,{r0,r1,r2,r3,r4}
up_screen240	; 240pixel display
	LDMIA	r5!,{r0,r1,r2,r3,r4}
	STMIA	r6!,{r0,r1,r2,r3,r4}
up_screen160	; 160pixel display
	LDMIA	r5!,{r0,r1,r2,r3,r4}
	STMIA	r6!,{r0,r1,r2,r3,r4}
up_screen80	; 80pixel display
	LDMIA	r5!,{r0,r1,r2,r3,r4}
	STMIA	r6!,{r0,r1,r2,r3,r4}

	; deal with unused display memory
	ADD	r5,r5,r8
	ADD	r6,r6,r8
	CMP	r5,r7
	BCC	copy_up_loop
copy_up_loop_end
	; clear the new screen area created
	ADRL	r5,zeroes
	LDMIA	r5,{r0,r1,r2,r3,r4}
	; zero from r6 to r7
zero_up_loop
	ADR	r5,zero_up_jump_into	; address of offset table
	LDR	r5,[r5,#ip]		; get offset value
	ADD	pc,r5,pc		; branch into the code
	NOP				; padding to ensure correct addresses
zero_up_PC_position
zero_up_jump_into
	&	(zero_up_screen80  - zero_up_PC_position)
	&	(zero_up_screen160 - zero_up_PC_position)
	&	(zero_up_screen240 - zero_up_PC_position)
	&	(zero_up_screen320 - zero_up_PC_position)
	&	(zero_up_screen400 - zero_up_PC_position)
	&	(zero_up_screen480 - zero_up_PC_position)
	&	(zero_up_screen560 - zero_up_PC_position)
	&	(zero_up_screen640 - zero_up_PC_position)
	&	(zero_up_screen720 - zero_up_PC_position)
	&	(zero_up_screen800 - zero_up_PC_position)
	&	(zero_up_screen880 - zero_up_PC_position)
	&	(zero_up_screen960 - zero_up_PC_position)

zero_up_screen960
	STMIA	r6!,{r0,r1,r2,r3,r4}
zero_up_screen880
	STMIA	r6!,{r0,r1,r2,r3,r4}
zero_up_screen800
	STMIA	r6!,{r0,r1,r2,r3,r4}
zero_up_screen720
	STMIA	r6!,{r0,r1,r2,r3,r4}
zero_up_screen640
	STMIA	r6!,{r0,r1,r2,r3,r4}
zero_up_screen560
	STMIA	r6!,{r0,r1,r2,r3,r4}
zero_up_screen480
	STMIA	r6!,{r0,r1,r2,r3,r4}
zero_up_screen400
	STMIA	r6!,{r0,r1,r2,r3,r4}
zero_up_screen320
	STMIA	r6!,{r0,r1,r2,r3,r4}
zero_up_screen240
	STMIA	r6!,{r0,r1,r2,r3,r4}
zero_up_screen160
	STMIA	r6!,{r0,r1,r2,r3,r4}
zero_up_screen80
	STMIA	r6!,{r0,r1,r2,r3,r4}
	; deal with unused display memory
	ADD	r6,r6,r8
	CMP	r6,r7
	BCC	zero_up_loop
	B	terminate_scroll

	; ---------------------------------------------------------------------

vscroll_down
	; r1 = scroll amount
	LDR	r7,screenbase		; r7 = termination point
	LDR	r6,screensize
	ADD	r6,r7,r6		; r6 = destination address
	LDR	r5,stride
	MUL	r5,r1,r5
	SUB	r5,r6,r5		; r5 = source address

	; calculate the number of 80pixel units in this the screen width
	LDR	r8,screenX		; screenX in pixels
	MOV	ip,#0			; index counter
calc_down_index_loop
	SUB	r8,r8,#80		; amount we move in a single transfer
	CMP	r8,#80			; check for termination
	ADDCS	ip,ip,#1		; next index
	BCS	calc_down_index_loop
	; ip = offset address table index
	; we only provide support upto 1024 horizontal display widths
	LDR	r8,stride		; current stride
	LDR	r0,screenX		; screenX width
	SUB	r8,r8,r0,LSR #3		; unused = stride - (screenX / 8)
	; r8 = calculated unused display width
copy_down_loop
	; deal with unused display memory on the Functional Prototype
	SUB	r5,r5,r8
	SUB	r6,r6,r8
	; branch into the relevant copy start point
	ADR	r0,down_jump_into		; address the offset table
	LDR	r0,[r0,#ip]		; get offset value
	ADD	pc,r0,pc		; branch into the code
	NOP				; padding to ensure correct addresses
down_PC_position
down_jump_into
	&	(down_screen80  - down_PC_position)
	&	(down_screen160 - down_PC_position)
	&	(down_screen240 - down_PC_position)
	&	(down_screen320 - down_PC_position)
	&	(down_screen400 - down_PC_position)
	&	(down_screen480 - down_PC_position)
	&	(down_screen560 - down_PC_position)
	&	(down_screen640 - down_PC_position)
	&	(down_screen720 - down_PC_position)
	&	(down_screen800 - down_PC_position)
	&	(down_screen880 - down_PC_position)
	&	(down_screen960 - down_PC_position)

down_screen960	; 960pixel display
	LDMDB	r5!,{r0,r1,r2,r3,r4}
	STMDB	r6!,{r0,r1,r2,r3,r4}
down_screen880	; 880pixel display
	LDMDB	r5!,{r0,r1,r2,r3,r4}
	STMDB	r6!,{r0,r1,r2,r3,r4}
down_screen800	; 800pixel display
	LDMDB	r5!,{r0,r1,r2,r3,r4}
	STMDB	r6!,{r0,r1,r2,r3,r4}
down_screen720	; 720pixel display
	LDMDB	r5!,{r0,r1,r2,r3,r4}
	STMDB	r6!,{r0,r1,r2,r3,r4}
down_screen640	; 640pixel display
	LDMDB	r5!,{r0,r1,r2,r3,r4}
	STMDB	r6!,{r0,r1,r2,r3,r4}
down_screen560	; 560pixel display
	LDMDB	r5!,{r0,r1,r2,r3,r4}
	STMDB	r6!,{r0,r1,r2,r3,r4}
down_screen480	; 480pixel display
	LDMDB	r5!,{r0,r1,r2,r3,r4}
	STMDB	r6!,{r0,r1,r2,r3,r4}
down_screen400	; 400pixel display
	LDMDB	r5!,{r0,r1,r2,r3,r4}
	STMDB	r6!,{r0,r1,r2,r3,r4}
down_screen320	; 320pixel display
	LDMDB	r5!,{r0,r1,r2,r3,r4}
	STMDB	r6!,{r0,r1,r2,r3,r4}
down_screen240	; 240pixel display
	LDMDB	r5!,{r0,r1,r2,r3,r4}
	STMDB	r6!,{r0,r1,r2,r3,r4}
down_screen160	; 160pixel display
	LDMDB	r5!,{r0,r1,r2,r3,r4}
	STMDB	r6!,{r0,r1,r2,r3,r4}
down_screen80	; 80pixel display
	LDMDB	r5!,{r0,r1,r2,r3,r4}
	STMDB	r6!,{r0,r1,r2,r3,r4}
	CMP	r5,r7
	BNE	copy_down_loop
copy_down_loop_end
	ADRL	r5,zeroes
	LDMIA	r5,{r0,r1,r2,r3,r4}
	; zero from r6 to r7
zero_down_loop
	; deal with unused display memory
	SUB	r6,r6,r8
	ADR	r5,zero_down_jump_into	; address of offset table
	LDR	r5,[r5,#ip]		; get offset value
	ADD	pc,r5,pc		; branch into the code
	NOP				; padding to ensure correct addresses
zero_down_PC_position
zero_down_jump_into
	&	(zero_down_screen80  - zero_down_PC_position)
	&	(zero_down_screen160 - zero_down_PC_position)
	&	(zero_down_screen240 - zero_down_PC_position)
	&	(zero_down_screen320 - zero_down_PC_position)
	&	(zero_down_screen400 - zero_down_PC_position)
	&	(zero_down_screen480 - zero_down_PC_position)
	&	(zero_down_screen560 - zero_down_PC_position)
	&	(zero_down_screen640 - zero_down_PC_position)
	&	(zero_down_screen720 - zero_down_PC_position)
	&	(zero_down_screen800 - zero_down_PC_position)
	&	(zero_down_screen880 - zero_down_PC_position)
	&	(zero_down_screen960 - zero_down_PC_position)

zero_down_screen960
	STMDB	r6!,{r0,r1,r2,r3,r4}
zero_down_screen880
	STMDB	r6!,{r0,r1,r2,r3,r4}
zero_down_screen800
	STMDB	r6!,{r0,r1,r2,r3,r4}
zero_down_screen720
	STMDB	r6!,{r0,r1,r2,r3,r4}
zero_down_screen640
	STMDB	r6!,{r0,r1,r2,r3,r4}
zero_down_screen560
	STMDB	r6!,{r0,r1,r2,r3,r4}
zero_down_screen480
	STMDB	r6!,{r0,r1,r2,r3,r4}
zero_down_screen400
	STMDB	r6!,{r0,r1,r2,r3,r4}
zero_down_screen320
	STMDB	r6!,{r0,r1,r2,r3,r4}
zero_down_screen240
	STMDB	r6!,{r0,r1,r2,r3,r4}
zero_down_screen160
	STMDB	r6!,{r0,r1,r2,r3,r4}
zero_down_screen80
	STMDB	r6!,{r0,r1,r2,r3,r4}
	CMP	r6,r7
	BNE	zero_down_loop
	B	terminate_scroll

	; ---------------------------------------------------------------------

hscroll_left
	!	0,"Scroll left to be coded"
	; Copy data from the right hand edge of the screen towards the left

	; example word scrolls (32pixels)
	;
	; for each raster (wordindex = (displaywidth - 4))
	;	LDR	dreg,[rasterstart,wordindex]
	;	SUB	wordindex,wordindex,#4
	;	STR	dreg,[rasterstart,wordindex]
	; until wordindex = 0
	; for each raster (wordindex = (displaywidth - 4))
	;	MOV	dreg,#&00000000
	;	STR	dreg,[rasterstart,wordindex]
	; The problem is getting "rasterstart" into the register cheaply
	; so we can do multiple load and stores. We should move as many
	; bits of data for each load/store cycle as possible.

	; ---------------------------------------------------------------------

hscroll_right
	!	0,"Scroll right to be coded"
	; Copy data from the left hand edge of the screen towards the right

	; ---------------------------------------------------------------------

terminate_scroll
	MOV	r0,#&00000000			; OK return
	LDMFD	sp!,{r1,r2,r3,r4,r5,r6,r7,r8,pc}^

scroll_badoption
	MOV	r0,#&FFFFFFFF			; Invalid option (-1)
	LDMFD	sp!,{r1,r2,r3,r4,r5,r6,r7,r8,pc}^

	; ---------------------------------------------------------------------
	; void display_clear(void) ;
display_clear	FnHead
	; Clear the display area
	STMFD	sp!,{r0,r1,r2,r3,r4,r5,r6,r7,r8,lk}

	LDR	r5,screenbase		; screen start
	LDR	r6,screensize
	ADD	r6,r5,r6		; screen end

	; calculate the number of 80pixel units in this the screen width
	LDR	r8,screenX		; screenX in pixels
	MOV	ip,#0			; index counter
calc_clear_index_loop
	SUB	r8,r8,#80		; amount we move in a single transfer
	CMP	r8,#80			; check for termination
	ADDCS	ip,ip,#1		; next index
	BCS	calc_clear_index_loop
	; ip = offset address table index
	LDR	r8,stride		; current stride
	LDR	r0,screenX		; screenX width
	SUB	r8,r8,r0,LSR #3		; unused = stride - (screenX / 8)
	; r8 = calculated unused display width

	ADRL	r0,zeroes
	LDMIA	r0,{r0,r1,r2,r3,r4}	; 5 zeroed registers
cls_loop
	ADR	r7,clear_jump_into	; address of offset table
	LDR	r7,[r7,#ip]		; get offset value
	ADD	pc,r7,pc		; branch into the code
	NOP				; padding to ensure correct addresses
clear_PC_position
clear_jump_into
	&	(clear_screen80  - clear_PC_position)
	&	(clear_screen160 - clear_PC_position)
	&	(clear_screen240 - clear_PC_position)
	&	(clear_screen320 - clear_PC_position)
	&	(clear_screen400 - clear_PC_position)
	&	(clear_screen480 - clear_PC_position)
	&	(clear_screen560 - clear_PC_position)
	&	(clear_screen640 - clear_PC_position)
	&	(clear_screen720 - clear_PC_position)
	&	(clear_screen800 - clear_PC_position)
	&	(clear_screen880 - clear_PC_position)
	&	(clear_screen960 - clear_PC_position)

clear_screen960
	STMIA	r5!,{r0,r1,r2,r3,r4}
clear_screen880
	STMIA	r5!,{r0,r1,r2,r3,r4}
clear_screen800
	STMIA	r5!,{r0,r1,r2,r3,r4}
clear_screen720
	STMIA	r5!,{r0,r1,r2,r3,r4}
clear_screen640
	STMIA	r5!,{r0,r1,r2,r3,r4}
clear_screen560
	STMIA	r5!,{r0,r1,r2,r3,r4}
clear_screen480
	STMIA	r5!,{r0,r1,r2,r3,r4}
clear_screen400
	STMIA	r5!,{r0,r1,r2,r3,r4}
clear_screen320
	STMIA	r5!,{r0,r1,r2,r3,r4}
clear_screen240
	STMIA	r5!,{r0,r1,r2,r3,r4}
clear_screen160
	STMIA	r5!,{r0,r1,r2,r3,r4}
clear_screen80
	STMIA	r5!,{r0,r1,r2,r3,r4}
	; deal with unused display memory
	ADD	r5,r5,r8
	CMP	r5,r6
	BCC	cls_loop
	; display cleared OK
	LDMFD	sp!,{r0,r1,r2,r3,r4,r5,r6,r7,r8,pc}^

	; ---------------------------------------------------------------------

zeroes	%	(16 * &04)	; 16 NULL words

	; ---------------------------------------------------------------------
	; void blit_normal(unsigned char *dptr,unsigned char cchr) ;
blit_normal	FnHead
	STMFD	sp!,{v1,v2,lk}
	; in:	a1 = screen destination pointer
	;	a2 = character ASCII code
	; out:	no conditions
	LDR	a3,attributes		; current display attributes
	ADRL	a4,glyphs		; glyph data pointer table
	LDR	v2,stride		; complete raster width in bytes
	; This code relies on the setup of the "glyphs" table. If the table
	; is extended/changed then this code should also be updated.
	; default is normal set (at glyphs)
	; if italic attribute then select italic (+2)
	; if bold attribute then select bold (+1)
	TST	a3,#char_italic		; italicise the text
	ADDNE	a4,a4,#(2 * word)	; step onto the italic section
	TST	a3,#char_bold		; enbolden the text
	ADDNE	a4,a4,#(1 * word)	; step onto the bold form
	LDR	v1,[a4,#&00]		; load the offset
	ADD	a4,a4,v1		; a4 = character set array pointer
	ADD	a4,a4,a2,LSL #3		; a4 = character glyph
	MOV	a2,#0			; data byte loop index
blit_normal_loop
	LDRB	v1,[a4,a2]		; load the character data
	CMP	a2,#7			; check for bottom line
	BNE	blit_normal_over	; NO - then continue
	TST	a3,#char_underline	; are we to underline this character
	MOVNE	v1,#&FF			; complete underline
blit_normal_over
	TST	a3,#char_inverse	; are we to invert this character
	MVNNE	v1,v1			; YES - then invert the data
	STRB	v1,[a1],v2		; store and stride onto next position
	ADD	a2,a2,#1		; next character position
	CMP	a2,#8			; check for termination
	BCC	blit_normal_loop
	; completed
	LDMFD	sp!,{v1,v2,pc}^

	; ---------------------------------------------------------------------
	; void blit_reverse(unsigned char *dptr,unsigned char cchr) ;
blit_reverse	FnHead
	STMFD	sp!,{v1,v2,v3,v4,lk}
	; in:	a1 = screen destination pointer
	;	a2 = character ASCII code
	; out:	no conditions
	LDR	a3,attributes		; current display attributes
	ADRL	a4,glyphs		; glyph data pointer table
	LDR	v2,stride		; complete raster width in bytes
	; This code relies on the setup of the "glyphs" table. If the table
	; is extended/changed then this code should also be updated.
	; default is normal set (at glyphs)
	; if italic attribute then select italic (+2)
	; if bold attribute then select bold (+1)
	TST	a3,#char_italic		; italicise the text
	ADDNE	a4,a4,#(2 * word)	; step onto the italic section
	TST	a3,#char_bold		; enbolden the text
	ADDNE	a4,a4,#(1 * word)	; step onto the bold form
	LDR	v1,[a4,#&00]		; load the offset
	ADD	a4,a4,v1		; a4 = character set array pointer
	ADD	a4,a4,a2,LSL #3		; a4 = character glyph
	MOV	a2,#8			; character position index
	MOV	v4,#&00			; dummy operand for bit-reverse
blit_reverse_loop
	SUB	a2,a2,#1		; decrement the character position
	LDRB	v1,[a4,a2]		; load the character data
	CMP	a2,#7			; check for bottom line
	BNE	blit_reverse_over	; NO - then continue
	TST	a3,#char_underline	; are we to underline this character
	MOVNE	v1,#&FF			; complete underline
blit_reverse_over
	TST	a3,#char_inverse	; are we to invert this character
	MVNNE	v1,v1			; YES - then invert the data
	; reverse the bit-ordering (we could use the hardware bit-reverse-port
	; if it becomes standard, at the moment it isn't necessarily).
	MOV	v3,#&00			; new bit mask
	; bit 0
	MOVS	v1,v1,LSR #1		; v1 = v1 >> 1 ; CS if bit set
	ADC	v3,v4,v3,LSL #1		; v3 = (v3 << 1) + Carry state
	; bit 1
	MOVS	v1,v1,LSR #1		; v1 = v1 >> 1 ; CS if bit set
	ADC	v3,v4,v3,LSL #1		; v3 = (v3 << 1) + Carry state
	; bit 2
	MOVS	v1,v1,LSR #1		; v1 = v1 >> 1 ; CS if bit set
	ADC	v3,v4,v3,LSL #1		; v3 = (v3 << 1) + Carry state
	; bit 3
	MOVS	v1,v1,LSR #1		; v1 = v1 >> 1 ; CS if bit set
	ADC	v3,v4,v3,LSL #1		; v3 = (v3 << 1) + Carry state
	; bit 4
	MOVS	v1,v1,LSR #1		; v1 = v1 >> 1 ; CS if bit set
	ADC	v3,v4,v3,LSL #1		; v3 = (v3 << 1) + Carry state
	; bit 5
	MOVS	v1,v1,LSR #1		; v1 = v1 >> 1 ; CS if bit set
	ADC	v3,v4,v3,LSL #1		; v3 = (v3 << 1) + Carry state
	; bit 6
	MOVS	v1,v1,LSR #1		; v1 = v1 >> 1 ; CS if bit set
	ADC	v3,v4,v3,LSL #1		; v3 = (v3 << 1) + Carry state
	; bit 7
	MOVS	v1,v1,LSR #1		; v1 = v1 >> 1 ; CS if bit set
	ADC	v3,v4,v3,LSL #1		; v3 = (v3 << 1) + Carry state
	STRB	v3,[a1],v2		; store and stride onto next position
	CMP	a2,#0			; check for termination
	BNE	blit_reverse_loop	; continue, if more to do
	; completed
	LDMFD	sp!,{v1,v2,v3,v4,pc}^

	; ---------------------------------------------------------------------

glyphs
glyphs0	&	(normalchars - glyphs0)	; normal
glyphs1	&	(boldchars - glyphs1)	; bold
glyphs2	&	(italicchars - glyphs2)	; italic
glyphs3	&	(italicchars - glyphs3)	; bold italic	(not yet defined)
glyphs_end
num_glyphs	*	((glyphs_end - glyphs) / 4)

	; ---------------------------------------------------------------------
	; These are the standard character set glyphs.

normalchars
	=	&00,&00,&00,&00,&00,&00,&00,&00	; ' '
	=	&18,&18,&18,&18,&18,&00,&18,&00	; '!'
	=	&36,&36,&36,&00,&00,&00,&00,&00	; '"'
	=	&6C,&6C,&FE,&6C,&FE,&6C,&6C,&00	; '#'
	=	&30,&FC,&16,&7C,&D0,&7E,&18,&00	; '$'
	=	&06,&66,&30,&18,&0C,&66,&60,&00	; '%'
	=	&1C,&36,&36,&1C,&B6,&66,&DC,&00	; '&'
	=	&18,&18,&18,&00,&00,&00,&00,&00	; '''
	=	&30,&18,&0C,&0C,&0C,&18,&30,&00	; '('
	=	&0C,&18,&30,&30,&30,&18,&0C,&00	; ')'
	=	&00,&18,&7E,&3C,&7E,&18,&00,&00	; '*'
	=	&00,&18,&18,&7E,&18,&18,&00,&00	; '+'
	=	&00,&00,&00,&00,&00,&18,&18,&0C	; ','
	=	&00,&00,&00,&7E,&00,&00,&00,&00	; '-'
	=	&00,&00,&00,&00,&00,&18,&18,&00	; '.'
	=	&00,&60,&30,&18,&0C,&06,&00,&00	; '/'
	=	&3C,&66,&76,&7E,&6E,&66,&3C,&00	; '0'
	=	&18,&1C,&18,&18,&18,&18,&7E,&00	; '1'
	=	&3C,&66,&60,&30,&18,&0C,&7E,&00	; '2'
	=	&3C,&66,&60,&38,&60,&66,&3C,&00	; '3'
	=	&30,&38,&3C,&36,&7E,&30,&30,&00	; '4'
	=	&7E,&06,&3E,&60,&60,&66,&3C,&00	; '5'
	=	&38,&0C,&06,&3E,&66,&66,&3C,&00	; '6'
	=	&7E,&60,&30,&18,&0C,&0C,&0C,&00	; '7'
	=	&3C,&66,&66,&3C,&66,&66,&3C,&00	; '8'
	=	&3C,&66,&66,&7C,&60,&30,&1C,&00	; '9'
	=	&00,&00,&18,&18,&00,&18,&18,&00	; ':'
	=	&00,&00,&18,&18,&00,&18,&18,&0C	; ';'
	=	&30,&18,&0C,&06,&0C,&18,&30,&00	; '<'
	=	&00,&00,&7E,&00,&7E,&00,&00,&00	; '='
	=	&0C,&18,&30,&60,&30,&18,&0C,&00	; '>'
	=	&3C,&66,&30,&18,&18,&00,&18,&00	; '?'
	=	&3C,&66,&76,&56,&76,&06,&3C,&00	; '@'
	=	&3C,&66,&66,&7E,&66,&66,&66,&00	; 'A'
	=	&3E,&66,&66,&3E,&66,&66,&3E,&00	; 'B'
	=	&3C,&66,&06,&06,&06,&66,&3C,&00	; 'C'
	=	&1E,&36,&66,&66,&66,&36,&1E,&00	; 'D'
	=	&7E,&06,&06,&3E,&06,&06,&7E,&00	; 'E'
	=	&7E,&06,&06,&3E,&06,&06,&06,&00	; 'F'
	=	&3C,&66,&06,&76,&66,&66,&3C,&00	; 'G'
	=	&66,&66,&66,&7E,&66,&66,&66,&00	; 'H'
	=	&7E,&18,&18,&18,&18,&18,&7E,&00	; 'I'
	=	&7C,&30,&30,&30,&30,&36,&1C,&00	; 'J'
	=	&66,&36,&1E,&0E,&1E,&36,&66,&00	; 'K'
	=	&06,&06,&06,&06,&06,&06,&7E,&00	; 'L'
	=	&C6,&EE,&FE,&D6,&D6,&C6,&C6,&00	; 'M'
	=	&66,&66,&6E,&7E,&76,&66,&66,&00	; 'N'
	=	&3C,&66,&66,&66,&66,&66,&3C,&00	; 'O'
	=	&3E,&66,&66,&3E,&06,&06,&06,&00	; 'P'
	=	&3C,&66,&66,&66,&56,&36,&6C,&00	; 'Q'
	=	&3E,&66,&66,&3E,&36,&66,&66,&00	; 'R'
	=	&3C,&66,&06,&3C,&60,&66,&3C,&00	; 'S'
	=	&7E,&18,&18,&18,&18,&18,&18,&00	; 'T'
	=	&66,&66,&66,&66,&66,&66,&3C,&00	; 'U'
	=	&66,&66,&66,&66,&66,&3C,&18,&00	; 'V'
	=	&C6,&C6,&D6,&D6,&FE,&EE,&C6,&00	; 'W'
	=	&66,&66,&3C,&18,&3C,&66,&66,&00	; 'X'
	=	&66,&66,&66,&3C,&18,&18,&18,&00	; 'Y'
	=	&7E,&60,&30,&18,&0C,&06,&7E,&00	; 'Z'
	=	&3E,&06,&06,&06,&06,&06,&3E,&00	; '['
	=	&00,&06,&0C,&18,&30,&60,&00,&00	; '\'
	=	&7C,&60,&60,&60,&60,&60,&7C,&00	; ']'
	=	&3C,&66,&00,&00,&00,&00,&00,&00	; '^'
	=	&00,&00,&00,&00,&00,&00,&00,&FF	; '_'
	=	&0C,&18,&00,&00,&00,&00,&00,&00	; '`'
	=	&00,&00,&3C,&60,&7C,&66,&7C,&00	; 'a'
	=	&06,&06,&3E,&66,&66,&66,&3E,&00	; 'b'
	=	&00,&00,&3C,&66,&06,&66,&3C,&00	; 'c'
	=	&60,&60,&7C,&66,&66,&66,&7C,&00	; 'd'
	=	&00,&00,&3C,&66,&7E,&06,&3C,&00	; 'e'
	=	&38,&0C,&0C,&3E,&0C,&0C,&0C,&00	; 'f'
	=	&00,&00,&7C,&66,&66,&7C,&60,&3C	; 'g'
	=	&06,&06,&3E,&66,&66,&66,&66,&00	; 'h'
	=	&18,&00,&1C,&18,&18,&18,&3C,&00	; 'i'
	=	&18,&00,&1C,&18,&18,&18,&18,&0E	; 'j'
	=	&06,&06,&66,&36,&1E,&36,&66,&00	; 'k'
	=	&1C,&18,&18,&18,&18,&18,&3C,&00	; 'l'
	=	&00,&00,&6C,&FE,&D6,&D6,&C6,&00	; 'm'
	=	&00,&00,&3E,&66,&66,&66,&66,&00	; 'n'
	=	&00,&00,&3C,&66,&66,&66,&3C,&00	; 'o'
	=	&00,&00,&3E,&66,&66,&3E,&06,&06	; 'p'
	=	&00,&00,&7C,&66,&66,&7C,&60,&E0	; 'q'
	=	&00,&00,&36,&6E,&06,&06,&06,&00	; 'r'
	=	&00,&00,&7C,&06,&3C,&60,&3E,&00	; 's'
	=	&0C,&0C,&3E,&0C,&0C,&0C,&38,&00	; 't'
	=	&00,&00,&66,&66,&66,&66,&7C,&00	; 'u'
	=	&00,&00,&66,&66,&66,&3C,&18,&00	; 'v'
	=	&00,&00,&C6,&D6,&D6,&FE,&6C,&00	; 'w'
	=	&00,&00,&66,&3C,&18,&3C,&66,&00	; 'x'
	=	&00,&00,&66,&66,&66,&7C,&60,&3C	; 'y'
	=	&00,&00,&7E,&30,&18,&0C,&7E,&00	; 'z'
	=	&30,&18,&18,&0E,&18,&18,&30,&00	; '{'
	=	&18,&18,&18,&18,&18,&18,&18,&00	; '|'
	=	&0C,&18,&18,&70,&18,&18,&0C,&00	; '}'
	=	&8C,&D6,&62,&00,&00,&00,&00,&00	; '~'

boldchars
	=	&00,&00,&00,&00,&00,&00,&00,&00	; 20
	=	&38,&38,&38,&38,&38,&00,&38,&00	; 21
	=	&77,&77,&77,&00,&00,&00,&00,&00	; 22
	=	&36,&7F,&7F,&36,&7F,&7F,&36,&00	; 23
	=	&18,&7E,&1B,&7E,&D8,&7E,&18,&00	; 24
	=	&67,&77,&38,&1C,&0E,&77,&73,&00	; 25
	=	&3E,&77,&77,&3E,&DF,&77,&DE,&00	; 26
	=	&70,&70,&38,&00,&00,&00,&00,&00	; 27
	=	&70,&38,&1C,&1C,&1C,&38,&70,&00	; 28
	=	&0E,&1C,&38,&38,&38,&1C,&0E,&00	; 29
	=	&18,&5A,&7E,&3C,&7E,&5A,&18,&00	; 2A
	=	&00,&1C,&1C,&7F,&1C,&1C,&00,&00	; 2B
	=	&00,&00,&00,&00,&00,&38,&38,&1C	; 2C
	=	&00,&00,&00,&7F,&7F,&00,&00,&00	; 2D
	=	&00,&00,&00,&00,&00,&38,&38,&00	; 2E
	=	&00,&70,&38,&1C,&0E,&07,&00,&00	; 2F
	=	&3E,&73,&7B,&7F,&6F,&67,&3E,&00	; 30
	=	&38,&3E,&38,&38,&38,&38,&7E,&00	; 31
	=	&3E,&73,&70,&38,&1C,&0E,&7F,&00	; 32
	=	&3E,&73,&70,&3C,&70,&73,&3E,&00	; 33
	=	&38,&3C,&3E,&3B,&7F,&38,&38,&00	; 34
	=	&7F,&07,&3F,&70,&70,&73,&3E,&00	; 35
	=	&3C,&0E,&07,&3F,&67,&67,&3E,&00	; 36
	=	&7F,&70,&38,&1C,&0E,&0E,&0E,&00	; 37
	=	&3E,&67,&67,&3E,&67,&67,&3E,&00	; 38
	=	&3E,&73,&73,&7E,&70,&38,&1E,&00	; 39
	=	&00,&00,&38,&38,&00,&38,&38,&00	; 3A
	=	&00,&00,&38,&38,&00,&38,&38,&1C	; 3B
	=	&70,&3C,&0E,&07,&0E,&3C,&70,&00	; 3C
	=	&00,&7F,&7F,&00,&7F,&7F,&00,&00	; 3D
	=	&07,&1E,&38,&70,&38,&1E,&07,&00	; 3E
	=	&3E,&77,&77,&38,&1C,&00,&1C,&00	; 3F
	=	&3E,&67,&67,&77,&77,&07,&3E,&00	; 40
	=	&3E,&67,&67,&7F,&67,&67,&67,&00	; 41
	=	&3F,&67,&67,&3F,&67,&67,&3F,&00	; 42
	=	&3E,&67,&07,&07,&07,&67,&3E,&00	; 43
	=	&1F,&37,&67,&67,&67,&37,&1F,&00	; 44
	=	&7F,&07,&07,&3F,&07,&07,&7F,&00	; 45
	=	&7F,&07,&07,&3F,&07,&07,&07,&00	; 46
	=	&3E,&67,&07,&77,&67,&67,&3E,&00	; 47
	=	&67,&67,&67,&7F,&67,&67,&67,&00	; 48
	=	&7F,&1C,&1C,&1C,&1C,&1C,&7F,&00	; 49
	=	&7E,&38,&38,&38,&38,&3B,&1E,&00	; 4A
	=	&67,&37,&1F,&0F,&1F,&37,&67,&00	; 4B
	=	&07,&07,&07,&07,&07,&07,&7F,&00	; 4C
	=	&63,&77,&7F,&6B,&6B,&63,&63,&00	; 4D
	=	&63,&67,&6F,&7F,&7B,&73,&63,&00	; 4E
	=	&3E,&67,&67,&67,&67,&67,&3E,&00	; 4F
	=	&3F,&67,&67,&3F,&07,&07,&07,&00	; 50
	=	&3E,&67,&67,&67,&57,&37,&6E,&00	; 51
	=	&3F,&67,&67,&3F,&37,&67,&67,&00	; 52
	=	&3E,&67,&07,&3E,&70,&73,&3E,&00	; 53
	=	&7F,&1C,&1C,&1C,&1C,&1C,&1C,&00	; 54
	=	&67,&67,&67,&67,&67,&67,&3E,&00	; 55
	=	&63,&63,&77,&36,&3E,&1C,&1C,&00	; 56
	=	&63,&63,&6B,&7F,&7F,&77,&63,&00	; 57
	=	&63,&77,&3E,&1C,&3E,&77,&63,&00	; 58
	=	&63,&77,&3E,&1C,&1C,&1C,&1C,&00	; 59
	=	&7F,&70,&38,&1C,&0E,&07,&7F,&00	; 5A
	=	&3E,&0E,&0E,&0E,&0E,&0E,&3E,&00	; 5B
	=	&00,&06,&0E,&1C,&38,&70,&00,&00	; 5C
	=	&7C,&70,&70,&70,&70,&70,&7C,&00	; 5D
	=	&08,&1C,&3E,&77,&63,&00,&00,&00	; 5E
	=	&00,&00,&00,&00,&00,&00,&FF,&FF	; 5F
	=	&3C,&6E,&0E,&3F,&0E,&0E,&7F,&00	; 60
	=	&00,&00,&3E,&70,&7E,&73,&7E,&00	; 61
	=	&07,&07,&3F,&67,&67,&67,&3F,&00	; 62
	=	&00,&00,&3E,&67,&07,&67,&3E,&00	; 63
	=	&70,&70,&7E,&73,&73,&73,&7E,&00	; 64
	=	&00,&00,&3E,&67,&7F,&07,&7E,&00	; 65
	=	&7C,&0E,&0E,&3F,&0E,&0E,&0E,&00	; 66
	=	&00,&00,&7E,&73,&73,&7E,&70,&3E	; 67
	=	&07,&07,&3F,&67,&67,&67,&67,&00	; 68
	=	&38,&00,&3E,&38,&38,&38,&7E,&00	; 69
	=	&38,&00,&3C,&38,&38,&38,&38,&1E	; 6A
	=	&07,&07,&67,&37,&1F,&37,&67,&00	; 6B
	=	&1E,&1C,&1C,&1C,&1C,&1C,&7E,&00	; 6C
	=	&00,&00,&63,&7F,&7F,&6B,&63,&00	; 6D
	=	&00,&00,&3F,&77,&77,&77,&77,&00	; 6E
	=	&00,&00,&3E,&77,&77,&77,&3E,&00	; 6F
	=	&00,&00,&3F,&77,&77,&3F,&07,&07	; 70
	=	&00,&00,&7E,&77,&77,&7E,&70,&70	; 71
	=	&00,&00,&3F,&6F,&07,&07,&07,&00	; 72
	=	&00,&00,&7E,&0F,&3E,&78,&3F,&00	; 73
	=	&0E,&0E,&3F,&0E,&0E,&0E,&7C,&00	; 74
	=	&00,&00,&77,&77,&77,&77,&7E,&00	; 75
	=	&00,&00,&77,&77,&77,&3E,&1C,&00	; 76
	=	&00,&00,&63,&6B,&7F,&7F,&63,&00	; 77
	=	&00,&00,&67,&3E,&1C,&3E,&73,&00	; 78
	=	&00,&00,&77,&77,&77,&7E,&70,&3E	; 79
	=	&00,&00,&7F,&38,&1C,&0E,&7F,&00	; 7A
	=	&38,&1C,&1C,&0F,&1C,&1C,&38,&00	; 7B
	=	&38,&38,&38,&00,&38,&38,&38,&00	; 7C
	=	&0E,&1C,&1C,&78,&1C,&1C,&0E,&00	; 7D
	=	&9C,&FE,&72,&00,&00,&00,&00,&00	; 7E

italicchars
	=	&00,&00,&00,&00,&00,&00,&00,&00	; 20
	=	&60,&60,&30,&30,&18,&00,&0C,&00	; 21
	=	&6C,&6C,&36,&00,&00,&00,&00,&00	; 22
	=	&D8,&D8,&FE,&6C,&FE,&1B,&1B,&00	; 23
	=	&60,&FC,&16,&7C,&D0,&7F,&0C,&00	; 24
	=	&0C,&CC,&30,&18,&0C,&33,&30,&00	; 25
	=	&38,&6C,&36,&1C,&B6,&33,&6E,&00	; 26
	=	&60,&30,&0C,&00,&00,&00,&00,&00	; 27
	=	&30,&18,&0C,&0C,&0C,&18,&30,&00	; 28
	=	&0C,&18,&30,&30,&30,&18,&0C,&00	; 29
	=	&00,&30,&7E,&3C,&7E,&0C,&00,&00	; 2A
	=	&00,&18,&18,&7E,&18,&18,&00,&00	; 2B
	=	&00,&00,&00,&00,&00,&18,&18,&0C	; 2C
	=	&00,&00,&00,&7E,&00,&00,&00,&00	; 2D
	=	&00,&00,&00,&00,&00,&18,&18,&00	; 2E
	=	&00,&C0,&30,&18,&0C,&03,&00,&00	; 2F
	=	&78,&CC,&76,&7E,&6E,&33,&1E,&00	; 30
	=	&70,&38,&30,&18,&18,&0C,&1F,&00	; 31
	=	&78,&CC,&60,&30,&18,&0C,&3F,&00	; 32
	=	&78,&CC,&60,&38,&60,&33,&1E,&00	; 33
	=	&60,&70,&3C,&36,&7E,&18,&18,&00	; 34
	=	&FC,&0C,&3E,&60,&60,&33,&1E,&00	; 35
	=	&70,&18,&06,&3E,&66,&33,&1E,&00	; 36
	=	&FC,&60,&30,&18,&0C,&06,&06,&00	; 37
	=	&78,&CC,&66,&3C,&66,&33,&1E,&00	; 38
	=	&78,&CC,&66,&7C,&60,&18,&0E,&00	; 39
	=	&00,&00,&18,&18,&00,&0C,&0C,&00	; 3A
	=	&00,&00,&18,&18,&00,&0C,&0C,&03	; 3B
	=	&60,&30,&18,&06,&0C,&0C,&18,&00	; 3C
	=	&00,&00,&7E,&00,&7E,&00,&00,&00	; 3D
	=	&18,&30,&60,&60,&18,&0C,&06,&00	; 3E
	=	&78,&CC,&30,&18,&18,&00,&0C,&00	; 3F
	=	&78,&CC,&76,&56,&76,&03,&1E,&00	; 40
	=	&78,&CC,&66,&7E,&66,&33,&33,&00	; 41
	=	&7C,&CC,&66,&3E,&66,&33,&1F,&00	; 42
	=	&78,&CC,&06,&06,&06,&66,&1E,&00	; 43
	=	&3C,&6C,&66,&66,&66,&1B,&0F,&00	; 44
	=	&FC,&0C,&06,&3E,&06,&03,&3F,&00	; 45
	=	&FC,&0C,&06,&3E,&06,&03,&03,&00	; 46
	=	&78,&CC,&06,&76,&66,&33,&1E,&00	; 47
	=	&CC,&CC,&66,&7E,&66,&33,&33,&00	; 48
	=	&FC,&30,&18,&18,&18,&0C,&3F,&00	; 49
	=	&F8,&60,&30,&30,&30,&1B,&0E,&00	; 4A
	=	&CC,&6C,&1E,&0E,&1E,&1B,&33,&00	; 4B
	=	&0C,&0C,&06,&06,&06,&03,&3F,&00	; 4C
	=	&CC,&DC,&FE,&D6,&D6,&63,&63,&00	; 4D
	=	&CC,&CC,&6E,&7E,&76,&33,&33,&00	; 4E
	=	&78,&CC,&66,&66,&66,&33,&1E,&00	; 4F
	=	&7C,&CC,&66,&3E,&06,&03,&03,&00	; 50
	=	&78,&CC,&66,&66,&56,&1B,&36,&00	; 51
	=	&7C,&CC,&66,&3E,&36,&33,&33,&00	; 52
	=	&78,&CC,&06,&3C,&60,&33,&1E,&00	; 53
	=	&FC,&30,&18,&18,&18,&0C,&0C,&00	; 54
	=	&CC,&CC,&66,&66,&66,&33,&1E,&00	; 55
	=	&CC,&CC,&66,&66,&66,&1E,&0C,&00	; 56
	=	&CC,&CC,&D6,&D6,&FE,&77,&63,&00	; 57
	=	&CC,&CC,&3C,&18,&3C,&33,&33,&00	; 58
	=	&CC,&CC,&66,&3C,&18,&0C,&0C,&00	; 59
	=	&FC,&C0,&30,&18,&0C,&03,&3F,&00	; 5A
	=	&7C,&0C,&06,&06,&06,&03,&1F,&00	; 5B
	=	&00,&0C,&0C,&18,&30,&30,&00,&00	; 5C
	=	&F8,&C0,&60,&60,&60,&30,&3E,&00	; 5D
	=	&30,&78,&66,&42,&00,&00,&00,&00	; 5E
	=	&00,&00,&00,&00,&00,&00,&00,&FF	; 5F
	=	&70,&D8,&0C,&3E,&0C,&06,&3F,&00	; 60
	=	&00,&00,&78,&60,&7C,&33,&3E,&00	; 61
	=	&18,&0C,&7C,&66,&66,&33,&1F,&00	; 62
	=	&00,&00,&78,&66,&06,&33,&1E,&00	; 63
	=	&C0,&C0,&F8,&66,&66,&33,&3E,&00	; 64
	=	&00,&00,&78,&66,&7E,&03,&1E,&00	; 65
	=	&E0,&18,&18,&3E,&0C,&06,&06,&00	; 66
	=	&00,&00,&F8,&66,&66,&3E,&30,&1E	; 67
	=	&18,&0C,&7C,&66,&66,&33,&33,&00	; 68
	=	&60,&00,&38,&18,&18,&0C,&1E,&00	; 69
	=	&30,&00,&38,&18,&18,&0C,&0C,&03	; 6A
	=	&18,&0C,&CC,&36,&1E,&1B,&33,&00	; 6B
	=	&70,&30,&30,&18,&18,&0C,&1E,&00	; 6C
	=	&00,&00,&D8,&FE,&D6,&6B,&63,&00	; 6D
	=	&00,&00,&7C,&66,&66,&33,&33,&00	; 6E
	=	&00,&00,&78,&66,&66,&33,&1E,&00	; 6F
	=	&00,&00,&7C,&66,&66,&1F,&03,&03	; 70
	=	&00,&00,&F8,&66,&66,&3E,&30,&38	; 71
	=	&00,&00,&6C,&6E,&06,&03,&03,&00	; 72
	=	&00,&00,&F8,&06,&3C,&30,&1F,&00	; 73
	=	&30,&18,&7C,&0C,&0C,&06,&1C,&00	; 74
	=	&00,&00,&CC,&66,&66,&33,&3E,&00	; 75
	=	&00,&00,&CC,&66,&66,&1E,&0C,&00	; 76
	=	&00,&00,&CC,&D6,&D6,&7F,&36,&00	; 77
	=	&00,&00,&CC,&3C,&18,&1E,&33,&00	; 78
	=	&00,&00,&CC,&66,&66,&3E,&30,&0F	; 79
	=	&00,&00,&FC,&30,&18,&06,&3F,&00	; 7A
	=	&30,&18,&18,&0E,&18,&18,&30,&00	; 7B
	=	&60,&30,&18,&00,&18,&0C,&06,&00	; 7C
	=	&0C,&18,&18,&70,&18,&18,&0C,&00	; 7D
	=	&8C,&D6,&62,&00,&00,&00,&00,&00	; 7E

	; ---------------------------------------------------------------------
	EndModule
	; ---------------------------------------------------------------------
	END
