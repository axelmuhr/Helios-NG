        SUBT Lo-Level ROM Monitor				      > lomon/s
	;    Copyright (c) 1991, Active Book Company, Cambridge, United Kingdom
	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; ARM HELIOS Monitor ROM
	; ---------------------------------------------------------------------
	; Simple monitor for Active-Books and prototypes (interface through
	; transputer link adaptor).
	; -----**** At the moment this is an Active Book only monitor ****-----
	;
	; Memory test based on one written by Roger Wilson of Acorn Computers.
	;
	; ---------------------------------------------------------------------

		GBLL	fontcode
fontcode	SETL	{FALSE}

	; ---------------------------------------------------------------------
	; onboard IO peripherals/registers

	[	(hercules)
	[	(dynlcd)
	; define a fixed screen location (just beneath 34MB)
LCD_base	*	(RAM_base + twomeg - LCD_size)
LCD_base_tier1	*	(LCD_base)
LCD_base_tier2	*	(LCD_base + (LCD_size / 2))
	]	; EOF (dynlcd)
	]	; EOF (hercules)

	; ---------------------------------------------------------------------
	; Other constants

NewLine		*	&0A
LineFeed	*       &0A
CarriageReturn	*       &0D

	; ---------------------------------------------------------------------
	; Codes for the Brazil SWIs (not all of which are implemented here yet)

mon_WriteC	*	0	; write single character (in r0) to output
mon_WriteS	*	1	; write in-line string (NULL terminated)
mon_Write0	*	2	; write NULL terminated string referenced by r0
mon_NewLine	*	3	; write CR/LF to output
mon_ReadC	*	4	; read single character into r0 from input
;mon_CLI	*	5	; operating system Command Line Interpreter
;mon_Byte	*	6	; OS_Byte call
;mon_Word	*	7	; OS_Word call
;mon_File	*	8	; OS_File call
;mon_Args	*	9	; OS_Args call
;mon_BGet	*	10	; OS_BGet call
;mon_BPut	*	11	; OS_BPut call
;mon_GBPB	*	12	; OS_GBPB call
;mon_Find	*	13	; OS_Find call
;mon_ReadLine	*       14	; OS_ReadLine call
;mon_Control	*	15
;mon_GetEnv	*       16	; OS_GetEnv call
mon_Exit	*	17	; exit the monitor (return to the Executive)
;mon_SetEnv	*	18	; OS_SetEnv call
mon_IntOn	*       19	; enable processor IRQs
mon_IntOff	*       20	; disable processor IRQs
;mon_CallBack	*	21
mon_EnterSVC	*	22	; enter SVC mode
;mon_BreakPT	*	23
;mon_BreakCTRL	*	24
;mon_UnusedSWI	*	25
;mon_UpdateMEMC	*	26

mon_WriteI	*       256	; base of single character output SWI set

	; ---------------------------------------------------------------------
	; Some more (non-Brazil) SWIs, from 32 onwards...

mon_ReadX	*	32	; read a hex number from the link
mon_WriteX	*	33      ; write a word in hex to the link
mon_WriteXB	*       34      ; write a byte in hex to the link
mon_HWPeek	*	35      ; peek HW word
mon_HWPoke	*	36      ; poke HW word                   
mon_HWPokeB	*       37      ; poke HW byte
mon_MaxSWI	*	37      ; highest valid SWI number

	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; Fixed addresses used by the monitor.
	; Space is reserved at the top of the static RAM for the register dump
	; produced by unhandled traps. Its address should be an immediate
	; constant.

	; leave 16 words for register save area
mon_register_dump	*	((SRAM_base + SRAM_size) - (16 * word))
mon_dumped_r8		*	(mon_register_dump + (8 * word))
mon_dumped_r14		*	(mon_register_dump + (14 * word))
mon_dumped_r15		*	(mon_register_dump + (15 * word))

	; The SVC mode stack is also in the static RAM, running down below the
	; register dump area. This too should be an immediate constant.

mon_SVC_stack	*	mon_register_dump

		^	&00		
vectors		#	(8 * word)	; 8 vectors
vector_tab	#	(8 * word)	; 8 indirection pointers
locals		#	(null)		; local storage for monitor

	[	(fontcode)
	; TTY variables
		^	locals
x		#	word		; TTY x position
y               #	word		; TTY y pos     
escflag         #	word		; ESC mode processing
print		#	word		; charblt style 0-7, -1 for LINK
	]	; EOF (fontcode)

	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; Default vector table
mon_vectortable
        LDR     pc,mon_VECTOR0	; branch through zero
        LDR     pc,mon_VECTOR1	; illegal instruction
        LDR     pc,mon_VECTOR2	; SWI
        LDR     pc,mon_VECTOR3	; prefetch abort
        LDR     pc,mon_VECTOR4	; data abort
        LDR     pc,mon_VECTOR5	; address exception
        LDR     pc,mon_VECTOR6	; IRQ
        LDR	pc,mon_VECTOR7	; FIQ

	; and the default vector contents
	; These values are just the offsets from the start of the Executive.
	; They need to be patched during the copying down to reference the
	; the TRUE addresses.
mon_VECTOR0	&       (branch0_trap	- codeBase)	; reset
mon_VECTOR1	&       (undef_handler	- codeBase)	; undefined
mon_VECTOR2	&       (swi_handler	- codeBase)	; SWI
mon_VECTOR3	&       (pabort_handler - codeBase)	; prefetch
mon_VECTOR4	&       (dabort_handler - codeBase)	; data
mon_VECTOR5	&       (addrex_handler - codeBase)	; address
mon_VECTOR6	&       (irq_handler	- codeBase)	; IRQ
mon_VECTOR7	&       (fiq_handler	- codeBase)	; FIQ

	; ---------------------------------------------------------------------

	[	(hercules)
	[	(dynlcd)
DMA_table
	; Used if we have to provide our own LCD screen and DMAs
	[	{TRUE}
	&	(LCD_base_tier1 :SHL: DMA_src_shift) :OR: DMALCD_xfer
	&	(LCD_base_tier1 :SHL: DMA_src_shift) :OR: DMALCD_xfer
	&	(LCD_base_tier2 :SHL: DMA_src_shift) :OR: DMALCD_xfer
	&	(LCD_base_tier2 :SHL: DMA_src_shift) :OR: DMALCD_xfer
	|
	&	(LCD_base_tier1 :SHL: DMA_src_shift) :OR: DMALCD_xfer
	&	(RAM_base       :SHL: DMA_src_shift) :OR: DMALCD_xfer
	&	(LCD_base_tier2 :SHL: DMA_src_shift) :OR: DMALCD_xfer
	&	(RAM_base       :SHL: DMA_src_shift) :OR: DMALCD_xfer
	]	; EOF {boolean}
DMA_table_end
DMA_table_size	*	(DMA_table_end - DMA_table)
	]	; EOF (dynlcd)
	]	; EOF (hercules)

	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; We are entered here from the Executive main startup thread.
	; This is also a restart point internally, so the processor mode is
	; undefined.
	; Switch into SVC mode and disable processor interrupts (which will not
	; work if we are in USR mode).
	;
monitor_reset
	MOV	r0,#(INTflags :OR: SVCmode)
	TEQP	r0,#&00				; SVC mode; IRQs/FIQs disabled
	NOP					; wait for mapping
        MOV     sp,#mon_SVC_stack		; default monitor SVC stack

	; Initialise FastRAM (vectors) and registers as we require them...
        MOV     r0,#SRAM_base
        ADRL    r9,mon_vectortable
        LDMIA   r9!,{r1-r8}		; copy 8 words of code
        STMIA   r0!,{r1-r8}
	LDMIA	r9,{r1-r8}		; load 8 words of vectors
	ADRL	r9,codeBase		; reference the start of the Executive
	ADD	r1,r1,r9		; and update the vector addresses
	ADD	r2,r2,r9
	ADD	r3,r3,r9
	ADD	r4,r4,r9
	ADD	r5,r5,r9
	ADD	r6,r6,r9
	ADD	r7,r7,r9
	ADD	r8,r8,r9
	STMIA	r0,{r1-r8}		; store the modified vectors in RAM

	[	(fontcode)
        MOV	r0,#&FFFFFFFF		; print mode flag to LINK tty
        MOV     r1,#SRAM_base
        STR     r0,[r1,#print]
	]	; EOF (fontcode)

	[	(hercules)
	[	(dynlcd)
	; LCD initialisation (fixed place LCD DMAs)
	; Initialise the DMA registers (All off except LCD DMA (for DRAM
	; refresh)). NOTE: The default RESET state is that the DMA channels
	; are disabled. The LCD DMA transfer is controlled by the LCD ON bits
	; in the register "(LCD_regs + LCD_control)".
	; 
	ADRL	r0,DMA_table			; default values (in table)
	MOV	r1,#DMA_base			; hardware registers
	ASSERT	(DMA_table_size = (4 * 4))	; only program channel 0
	LDMIA	r0,{r3,r4,r5,r6}
	STMIA	r1,{r3,r4,r5,r6}
	; and enable the LCD DMAs
	;	       delay          latch pulse type     clock rate
	MOV	r1,#(LCD_DLY_80  :OR:   (&01 :SHL: 4)  :OR:   &09)
	ORR	r1,r1,#(LCD_WAI :OR: LCD_normal :OR: LCD_ICP)
	MOV	r0,#LCD_regs			; LCD control registers
	STR	r1,[r0,#LCD_control]		; force write to resync world
	]	; EOF (dynlcd)
	]	; EOF (hercules)

	BL      screeninit			; setup default display

        MOV     r9,#&00     			; suppress echo mode on startup
	; and fall through to ... next_command

	; ---------------------------------------------------------------------
	; Main body of monitor.
	; The code of all the commands avoids using any stack and does IO
	; using SWIs so that it can run in USR mode with all the memory 
	; mapped out. This means that there is a maximum of one level of 
	; subroutine call ONLY.
	;
next_command	; main command loop (may be in any processor mode here)
	; prompt if local echo allows
        CMP     r9,#&00
        BEQ     next_command_get
	; display the prompt
        SWI     mon_WriteS
	[	(activebook)
        =       "Executive AB1> ",null	; the default prompt
	|
	[	(heval)
	=	"Executive Heval> ",null
	|
	=	"Executive ABFP> ",null
	]	; EOF (heval)
	]	; EOF (activebook)
        ALIGN
next_command_get
	; r9  = command echo state
	; r10 = address of next write (for incremental commands)
        SWI     mon_ReadC			; get a command character
	; r0 = character read
	ADRL	r1,mon_command_table		; address the commands
	MOV	r2,#&00				; current table index
	LDR	r4,=(mon_command_table_end - mon_command_table)
mon_find_cmd_loop
	ADD	r2,r2,#word			; step over the function offset
	LDRB	r3,[r1,r2]			; and load the command byte
	CMP	r0,r3				; check for command
	BEQ	mon_found_cmd

	ADD	r2,r2,#&01			; reference the length byte
	LDRB	r3,[r1,r2]			; and load it
	ADD	r2,r2,#&01			; reference the first text byte

	ADD	r2,r2,r3			; step to the end of the text
	ADD	r2,r2,#(word - 1)		; and word align the index
	BIC	r2,r2,#(word - 1)		; value upwards
	CMP	r2,r4				; check for table end
	BLT	mon_find_cmd_loop		; if not, then continue

	; command not found in table
	MOV	r9,#&01				; ensure prompt is echoed

	TEQ	r0,#CarriageReturn		; check for simple line
	TEQNE	r0,#NewLine			; end commands
	SWIEQ	mon_NewLine			; perform the NewLine
	BEQ	next_command			; and go around again

	; unrecognised command
        SWI     mon_WriteS
        =       "\nUnknown command: use \"h\" for help\n",null
	ALIGN
	B	next_command			; and go around again

	; ---------------------------------------------------------------------

mon_found_cmd
	; r2 references the function information
	ADD	r2,r1,r2			; reference the command entry
	LDR	r2,[r2,#-word]			; and load the function offset
	; r0 = command character
	; r1 = command table
	; r2 = function offset from command table
	CMP	r9,#&00				; check for echo disabled
	TEQNE	r0,#"+"				; only explicit not echoing cmd
	SWINE	mon_WriteC			; otherwise echo the character
	ADD	pc,r1,r2			; enter the command handler

	; ---------------------------------------------------------------------

	MACRO
$label	COMMAND	$letter,$text,$function
$label	&	($function - mon_command_table)
	=	"$letter",((:LEN: "$text") + &01),"$text",null
	ALIGN	; to next word boundary
	MEND

mon_command_table
	COMMAND "b","<from> <to> <n> : Block copy bytes",cmd_blockcopy
	COMMAND	"c","                : Clear screen",cmd_clear
	COMMAND "d","<start> <end>   : Dump words to display",cmd_dumpwords
	COMMAND "D","<start> <end>   : Dump bytes to display",cmd_dumpbytes
	[	(fontcode)
	COMMAND	"f","<A-Z>           : Font select (default = A)",cmd_font
	]	; EOF (fontcode)
	COMMAND "g","<addr>          : Goto (call) code at <addr>",cmd_goto
        COMMAND "h","                : Print help and current state",cmd_help
        COMMAND "H","                : Print help and current state",cmd_help
	COMMAND "i","                : IRQ off",cmd_intoff
	COMMAND "I","                : IRQ on",cmd_inton
 	COMMAND "m","                : Memory test (main RAM)",cmd_memtestmain
        COMMAND "M","<start> <end>   : Memory test",cmd_memtest
	[	(fontcode)
	COMMAND "o","                : Send TTY output to LCD",cmd_ttyon
	COMMAND "O","                : Send TTY output to link",cmd_ttyoff
	]	; EOF (fontcode)
        COMMAND "p","<addr>          : Poll word at <addr>",cmd_pollword
        COMMAND "P","<addr>          : Poll byte at <addr>",cmd_pollbyte
 	COMMAND "q","                : Quit monitor (restart Helios)",cmd_quit
        COMMAND "r","<addr>          : Read word at <addr>",cmd_readword
	COMMAND "R","<addr>          : Read byte at <addr>",cmd_readbyte
	COMMAND "s","                : Size main RAM",cmd_sizemain
	COMMAND "S","<addr>          : Size memory from <addr>",cmd_size
	COMMAND "t","<addr> <size>   : Transfer binary data to memory",cmd_bin
	COMMAND "u","                : Enter USR mode",cmd_usr
	COMMAND "v","                : Enter SVC mode",cmd_svc
	COMMAND "w","<addr> <value>  : Write word <value> to <addr>",cmd_word
	COMMAND "W","<addr> <value>  : Write byte <value> to <addr>",cmd_byte
	COMMAND "+","<value>         : Write <value> to next word",cmd_noecho
        COMMAND "?","                : Print state information",cmd_info
mon_command_table_end

	LTORG

	; ---------------------------------------------------------------------

cmd_help
        SWI     mon_WriteS
	[	(activebook)
        =       "\nAB1"
	|	; middle (activebook)
	[	(heval)
	=	"\nHEVAL"
	|	; middle (heval)
	=	"\nABFP"
	]	; EOF (heval)
	]	; EOF (activebook)
	=	" Executive Monitor v0.02\n"
	=	"Commands are:\n",null
	ALIGN
	ADRL	r1,mon_command_table		; address the commands
	ADRL	r2,mon_command_table_end	; end of the table
cmd_help_loop
	ADD	r1,r1,#word			; step over the function offset
	LDRB	r0,[r1],#&01			; command character
	SWI	mon_WriteC			; and display the character
	MOV	r0,#" "				; padding character
	SWI	mon_WriteC			; and display padding character
	LDRB	r3,[r1],#&01			; load the help text length
	MOV	r0,r1				; the help text address
	SWI	mon_Write0			; display the help text
	SWI	mon_NewLine			; and newline
	ADD	r1,r1,r3			; step over the help text
	ADD	r1,r1,#(word - 1)		; and word-align the address
	BIC	r1,r1,#(word - 1)		; upwards
	CMP	r1,r2				; check for the table end
	BLT	cmd_help_loop			; if not, then around again
        SWI     mon_WriteS
        =       "\nAll arguments are hexadecimal numbers\n",null
	ALIGN
	; and fall through to... cmd_info

	; ---------------------------------------------------------------------
	; Print some information on the current state of the monitor

cmd_info
        SWI     mon_NewLine
        BL      printmode			; print mode name
        SWI     mon_WriteS
        =       " mode, flags: ",null
	ALIGN
	; print PSR bits
	ADRL	r1,mon_psr_flags		; address the flag names
	MOV	r2,pc,LSR #26			; move PSR bits into lo6bits
	MOV	r3,#(1 :SHL: 5)			; flag display index
mon_psr_flags_loop
	LDRB	r0,[r1],#&01			; load the flag character
	TST	r2,r3				; check flag condition
	ANDNE	r0,r0,#&DF			; upper-case if bit set
	SWI	mon_WriteC			; and display the character
	MOVS	r3,r3,LSR #1			; shift down the index
	BNE	mon_psr_flags_loop		; and onto the next flag
	; display other useful information
        SWI     mon_WriteS
        =       ", pc = &",null
        MOV     r0,pc
        SWI     mon_WriteX
        SWI     mon_WriteS
        =       "\nCommand loop starts at &",null
        ADRL    r0,next_command
        SWI     mon_WriteX
	SWI	mon_NewLine
	MOV	r9,#&01				; ensure the prompt is echoed
        B       next_command			; and around again

	; ---------------------------------------------------------------------

mon_psr_modes
        	=       "USR",null
	        =       "FIQ",null
       		=       "IRQ",null
	        =       "SVC",null
mon_psr_flags	=	"nzcvif",null
        ALIGN

	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------

cmd_quit
	; This should re-start the Executive (and start up Helios from ROM)
	!	0,"TODO: *** ensure we are in SVC mode, IRQs/FIQs disabled ***"
	; An even better way would be to return to the "lk" we have on entry
	; to "monitor_reset". This would allow the monitor to be called from
	; other points in the system.
	SWI	mon_WriteS
	=	"\nStarting Helios\n",null
	ALIGN
	B	Helios_startup

	; ---------------------------------------------------------------------

cmd_readword
        SWI     mon_ReadX	; get the address
        LDR     r0,[r0,#&00]	; read the word
        SWI     mon_WriteX	; and then print it
	SWI	mon_NewLine
        B       next_command

	; ---------------------------------------------------------------------

cmd_readbyte
        SWI 	mon_ReadX
        LDRB    r0,[r0,#&00]
        SWI     mon_WriteXB
	SWI	mon_NewLine
        B       next_command

	; ---------------------------------------------------------------------

cmd_word
        SWI     mon_ReadX	; get address
        MOV     r10,r0          ; r10 used globally for last write address
        SWI     mon_ReadX       ; get new value
        STR     r0,[r10],#&04   ; leave next address in r10 for cmd_noecho
        B       next_command

	; ---------------------------------------------------------------------
	; Incrementing write command: uses address after that in last write
	; or incwrite command (in r10), and does not reflect any characters 
	; (so it goes faster).

cmd_noecho
        SWI     mon_ReadX	; get new value
        STR     r0,[r10],#&04	; leave next address in r10
        B       next_command

	; ---------------------------------------------------------------------

cmd_byte
        SWI     mon_ReadX	; get address
        MOV     r1,r0
        SWI     mon_ReadX       ; get new value
        STRB    r0,[r1,#&00]
        B       next_command

	; ---------------------------------------------------------------------

cmd_bin				; perform simple binary download
        SWI     mon_ReadX       ; get start address
        MOV     r1,r0		; into r1
        SWI     mon_ReadX       ; get size value
        MOV     r2,r0           ; into r2
cmd_bin_loop			; read "r2" bytes from the link to "r1"
	BL	get		; get byte into r0
	STRB	r0,[r1],#&01	; store byte and increment address
	SUBS	r2,r2,#&01	; decrement counter

	BNE	cmd_bin_loop	; around again if not terminated
	MOV	r9,#&00		; disable echo after a binary download
	B	next_command	; and finish this command

	; ---------------------------------------------------------------------

cmd_dumpbytes
	SWI	mon_ReadX	; get start address
	MOV	r1,r0		; into r1
	SWI	mon_ReadX	; get end address
	MOV	r2,r0		; into r2
	SWI	mon_NewLine	; ensure display starts on newline
cmd_dumpbytes_outer_loop
	MOV	r0,r1		; current address
	SWI	mon_WriteX	; display it
	SWI	mon_WriteS
	=	" : ",null	; display padding string
	ALIGN
	MOV	r3,#0		; byte index into line
cmd_dumpbytes_inner_loop_hex
	LDRB	r0,[r1,r3]	; load data byte
	SWI	mon_WriteXB	; and display byte
	SWI	mon_WriteI + " "
	ADD	r3,r3,#1	; step onto next byte
	CMP	r3,#16		; 16bytes per line
	BNE	cmd_dumpbytes_inner_loop_hex
	MOV	r3,#0		; byte index into line
cmd_dumpbytes_inner_loop_char
	LDRB	r0,[r1,r3]	; load data byte
	CMP	r0,#" "		; check against space (for control codes)
	MOVCC	r0,#"."
	CMP	r0,#127		; check against delete (for top-bit set)
	MOVCS	r0,#"."
	SWI	mon_WriteC	; and display character
	ADD	r3,r3,#1	; step onto next byte
	CMP	r3,#16		; 16bytes per line
	BNE	cmd_dumpbytes_inner_loop_char
	SWI	mon_NewLine
	ADD	r1,r1,#16	; number of bytes printed
	CMP	r1,r2
	BLT	cmd_dumpbytes_outer_loop
	B	next_command

	; ---------------------------------------------------------------------

cmd_dumpwords
	SWI	mon_ReadX		; get start address
	BIC	r1,r0,#(word - 1)	; into r1 (word-aligned downwards)
	SWI	mon_ReadX		; get end address
	ADD	r2,r0,#(word - 1)	; into r2 (word-aligned upwards)
	BIC	r2,r2,#(word - 1)
	SWI	mon_NewLine		; ensure display starts on new line

cmd_dumpwords_outer_loop
	MOV	r0,r1		; current address
	SWI	mon_WriteX	; display it
	SWI	mon_WriteS
	=	" : ",null	; display padding string
	ALIGN
	MOV	r3,#0		; word index into line
cmd_dumpwords_inner_loop_hex
	LDR	r0,[r1,r3]	; load data word
	SWI	mon_WriteX	; and display word
	SWI	mon_WriteI + " "
	ADD	r3,r3,#&04	; step onto next word
	CMP	r3,#(4 * word)	; 4words per line
	BNE	cmd_dumpwords_inner_loop_hex

	MOV	r3,#0		; byte index into line
cmd_dumpwords_inner_loop_char
	LDRB	r0,[r1,r3]	; load data byte
	CMP	r0,#" "		; check against space (for control codes)
	MOVCC	r0,#"."
	CMP	r0,#127		; check against delete (for top-bit set codes)
	MOVCS	r0,#"."
	SWI	mon_WriteC	; and display character
	ADD	r3,r3,#1	; step onto next byte
	CMP	r3,#(4 * word)	; 4words per line
	BNE	cmd_dumpwords_inner_loop_char

	SWI	mon_NewLine
	ADD	r1,r1,#(4 * word)
	CMP	r1,r2
	BLT	cmd_dumpwords_outer_loop
	B	next_command

	; ---------------------------------------------------------------------

cmd_goto
        SWI     mon_ReadX
        MOV     lr,pc           ; performs subroutine call rather than goto
        MOV     pc,r0
        B       next_command

	; ---------------------------------------------------------------------

cmd_blockcopy
        SWI     mon_ReadX	; get source address
        MOV     r3,r0
        SWI     mon_ReadX       ; get destination address
        MOV     r1,r0
        SWI     mon_ReadX       ; get number of bytes
        MOV     r2,r0
        MOV     r0,r3
        BL      bcopy
        B       next_command

	; ---------------------------------------------------------------------
	; Block copy routine.
	; Source in r0, destination in r1, number of bytes in r2.
	; Corrupts r0 - r3 (but avoids using any stack).
	; This is a very slow implementation! but who cares!

bcopy
	ADD     r2,r2,r0        ; end address of source in r2
bcopy1
	CMP     r0,r2
        LDRNEB  r3,[r0],#1
        STRNEB  r3,[r1],#1
        BNE     bcopy1
        MOVS    pc,lr

	; ---------------------------------------------------------------------

cmd_clear
	!	0,"TODO: interrogate LCD DMA base register"
	;		 on Hercules systems only
        MOV     r0,#&00000000	; clear LCD screen memory
        LDR     r1,=LCD_base
        ADD     r2,r1,#LCD_size ; address of end of screen memory
clear1
	STR     r0,[r1],#&04
        CMP     r1,r2
        BNE     clear1

	SWI	mon_NewLine
        B       next_command

	LTORG

	; ---------------------------------------------------------------------
	; Memory test for main RAM

cmd_memtestmain
        BL      ramsize         ; Get size of RAM in r0
        ADD     r1,r0,#RAM_base ; Get end of RAM address in r1
        MOV     r0,#RAM_base	; Get start of RAM address in r0
        B       memtest         ; Do the memory test

	; ---------------------------------------------------------------------
	; Memory test between given addresses

cmd_memtest
        SWI     mon_ReadX       ; Get start address in r0
        MOV     r4,r0
        SWI     mon_ReadX       ; Get end address in r0
        MOV     r1,r0           ; Juggle registers
        MOV     r0,r4
        B       memtest         ; Do the memory test

	; ---------------------------------------------------------------------
	; Memory test code.
	; First address to test is in r0
	; First address not to test is in r1
	; Exits to `next_command' on error, otherwise carries on for ever.

memtest
        SWI     mon_WriteS
        =       "\nTesting memory from &",null
        SWI     mon_WriteX
        SWI     mon_WriteS
        =       " up to (but not including) &",null
	ALIGN
        MOV     r11,r0          ; Start address lives in r11 for whole test
        MOV     r0,r1
        SWI     mon_WriteX
        SWI     mon_NewLine
        MOV     r12,r1          ; Save end of memory address
        MOV     r10,r12

; Start address in r11, end address in r12

memtest_restart

; Incrementing pattern test
        SWI     mon_WriteS
        =       "\nPhase one: incrementing pattern: ",null
	ALIGN
        MOV     r7,#4           ; R7 holds loop count for repeats of whole test
        MOV     r8,#0           ; R8 holds number of errors in this phase
        LDR     r3,p1ptn        ; R3 holds initial pattern for this iteration

phase1  MOV     r0,r3           ; Initial pattern in r3
        LDR     r1,p1inc
        MOV     r4,r11          ; Get base address in R4
ph1fill STR     r0,[r4],#4      ; Loop filling memory with incrementing pattern
        ADD     r0,r0,r1
        TEQ     r4,r12
        BNE     ph1fill

        MOV     r0,#"."
        SWI     mon_WriteI + "."	; Finished writing
; Now check the memory contents
        MOV     r4,r11          ; r4 holds memory address
        MOV     r0,r3           ; Initial pat in r3; r0 holds current pattern
ph1chk  LDR     r2,[r4],#4
        TEQ     r2,r0           ; Check one word of memory
        BNE     ph1fail

ph1cont ADD     r0,r0,r1
        TEQ     r4,r12
        BNE     ph1chk

        MOV     r3,r0           ; New initial pattern for next iteration
        SUBS    r7,r7,#1
        BNE     phase1          ; Repeat whole test with different patterns

        TEQ     r8,#0           ; Were there any errors?
        BNE     phasedead

; TRUE hierarchy test
; Set all of memory to 0 except with bit 2**n set in their address
; (where n is incremented on each iteration)

        SWI     mon_WriteS
        =       "\nPhase two: TRUE hierarchy: ",null
	ALIGN
        MOV     r0,#0
        MVN     r1,#0
        MOV     r2,#2           ; r2 holds address bit being tested
; Shared code for phases 2 and 3
phase2  MOV     r4,r11          ; Get base address in r4
phase2a TST     r4,r2           ; Fill memory with r0 except locations
        STREQ   r0,[r4],#4      ; with the bit in r2 set in their address
        STRNE   r1,[r4],#4
        CMP     r4,r12
        BCC     phase2a

        SWI     mon_WriteI +"."      ; Memory filled
        MOV     r4,r11          ; Get base address in r4
phase2b TST     r4,r2
        LDR     r3,[r4],#4
        BNE     phase2b1
phase2b2
        CMP     r0,r3           ; Location should hold R0
        BNE     ph2fail
        B       ph2cont
phase2b1
        CMP     r1,r3           ; Location should hold r1
        BNE     ph2fail

ph2cont CMP     r4,r12          ; Reached end of memory?
        BCC     phase2b         ; No - test next word

        CMP     r2,r12          ; Special location reached end of memory?
        ADDCC   r2,r2,r2        ; No - shift address bit being tested
        BCC     phase2

        TEQ     r8,#0           ; Any errors?
        BNE     phasedead
        TEQ     r1,#0           ; Different exits from phase 2/3 shared code
        BEQ     phase4          ; Skip phase 3 code if just done phase 3

; FALSE hierarchy

        SWI     mon_WriteS
        =       "\nPhase three: FALSE hierarchy: ",null
	ALIGN
        MVN     r0,#0
        MOV     r1,#0
        MOV     r2,#2
        B       phase2          ; Share code with phase 2
phase4  TEQ     r8,#0
        BNE     phasedead

; Cycling bits

        SWI     mon_WriteS
        =       "\nPhase four: Cycling bits: ",null
	ALIGN
        MOV     r7,#1
phase4a MOV     r4,r11          ; Get base address in r4
        MOV     r2,r7
phase4a1
        STR     r2,[r4],#4      ; Fill memory with cycling bit pattern
        MOV     r2,r2,ROR #31
        CMP     r4,r12
        BCC     phase4a1

        SWI     mon_WriteI +"."
        MOV     r4,r11          ; Get base address in r4
        MOV     r2,r7
phase4b1
        LDR     r1,[r4],#4
        CMP     r1,r2
        BNE     ph4fail
ph4cont MOV     r2,r2,ROR #31
        CMP     r4,r12
        BCC     phase4b1

        ADDS    r7,r7,r7        ; Shift starting bit posn and repeat
        BCC     phase4a
        TEQ     r8,#0
        BNE     phasedead

        SWI     mon_WriteS
        =       "\n\n  PASSED......",null
	ALIGN
        B       memtest_restart ; All OK, so do it again

phasedead
        SWI     mon_WriteS
        =       "\n  There were &",null
	ALIGN
        MOV     r10,r8
        BL      wordhx
        SWI     mon_WriteS
        =       " failures in total\n",null
	ALIGN
        SWI     mon_WriteS
        =       "\n   TESTING ABORTED\n",null
	ALIGN
        B       memtest_exit    ; Give up

; ph1fail must not use r3 (among others!)
ph1fail CMP     r8,#10          ; Limit number of messages per phase
        ADD     r8,r8,#1
        BCS     ph1cont

        SWI     mon_WriteS
        =       "\n  Phase 1 fail at &",null
	ALIGN
        MOV     r5,r0
        SUB     r10,r4,#4
        BL      wordhx
        SWI     mon_WriteS
        =       " with &",null
	ALIGN
        MOV     r10,r2
        BL      wordhx
        SWI     mon_WriteS
        =       " instead of &",null
	ALIGN
        MOV     r10,r5
        BL      wordhx
        MOV     r0,r5
        B       ph1cont

ph2fail CMP     r8,#10          ; Limit no. of messages per phase
        ADD     r8,r8,#1
        BCS     ph2cont
        SWI     mon_WriteS
        =       "\n  Phase 2 fail at &",null
	ALIGN
        SUB     r10,r4,#4
        MOV     r5,r0
        BL      wordhx
        SWI     mon_WriteS
        =       " with &",null
	ALIGN
        MOV     r10,r3
        BL      wordhx
        SWI     mon_WriteS
        =       " instead of &",null
	ALIGN
        TST     r4,r2
        MOVNE   r10,r1
        MOVEQ   r10,r5
        BL      wordhx
        MOV     r0,r5
        B       ph2cont

ph4fail CMP     r8,#10
        ADD     r8,r8,#1
        BCS     ph4cont
        SWI     mon_WriteS
        =       "\n  Phase 4 fail at &",null
	ALIGN
        SUB     r10,r4,#4
        BL      wordhx
        SWI     mon_WriteS
        =       " with &",null
	ALIGN
        MOV     r10,r1
        BL      wordhx
        SWI     mon_WriteS
        =       " instead of &",null
	ALIGN
        MOV     r10,r2
        BL      wordhx
        B       ph4cont

memtest_exit
	MOV	r0,#&01			; ensure prompt is echoed
        B       next_command

p1ptn   &       &86427531
p1inc   &       &0F020501

wordsp  SWI     mon_WriteI +" "      ; Print r10 in hex with leading space
;print r10 using R0,R6,r9
wordhx  MOV     r6,#32-4
wordlp  MOV     r0,r10,LSR r6
        AND     r0,r0,#15
        CMP     r0,#9
        ORRLS   r0,r0,#"0"
        ADDHI   r0,r0,#"A"-10
        SWI     mon_WriteC
        SUBS    r6,r6,#4
        BPL     wordlp
        MOV     pc,lr

	; ---------------------------------------------------------------------
	; Print size of main RAM

cmd_sizemain
        SWI     mon_WriteS
        =       "\nRAM size is &",null
	ALIGN
        BL      ramsize			; get RAM size in R0
        SWI     mon_WriteX		; print it
        SWI     mon_WriteS
        =       " bytes\n",null
	ALIGN
        B       next_command

	; ---------------------------------------------------------------------
	; Find size of memory starting at given address

cmd_size
        SWI     mon_ReadX		; get start address in r0
        MOV     r8,r0			; keep safe copy
        MOV     r1,#1			; check every byte (step 1)
        BL      findsize        	; get size in r0 (corrupts r1-r6)
        SWI     mon_WriteS
        =       "Memory extends to &",null
	ALIGN
        MOV     r1,r0
        ADD     r0,r8,r0        	; address of byte past end
        SUB     r0,r0,#1        	; address of last byte
        SWI     mon_WriteX
        SWI     mon_WriteS
        =       " (size &",null
	ALIGN
        MOV     r0,r1           	; retrieve size
        SWI     mon_WriteX
        SWI     mon_WriteS
        =       " bytes)\n",null
	ALIGN
        B       next_command
        
	; ---------------------------------------------------------------------
	; Find size of main RAM and return in r0. r1-r6 corrupted.

ramsize
	MOV     r0,#RAM_base	; start at base of physical RAM
        MOV     r1,#1024        ; 1K grain is fine enough
	; Drop into findsize

	; Find size of memory from address in r0 upwards and return in r0.
	; r1 specifies the address step to be used.
	; Memory is left intact after the size test (unless there is an abort).
	; r1-r6 corrupted.

findsize
        MOV     r5,r1           ; keep step size in r5  
        MOV     r1,r0           ; keep start address in r1
        MOV     r2,r1           ; r2 holds current probe address
        LDRB    r4,[r2,#0]      ; use original base contents as 2nd pattern
        EOR     r3,r4,#&FF      ; use inverse byte as first pattern

	; Move up memory in steps, probing each location with two different
	; patterns to see if memory is present, and checking the base of
	; memory too (to detect aliasing).
	; Handling memory abort TBD.

        MOV     r6,r4           ; save contents of current location in r6
        STRB    r3,[r2,#0]      ; store first pattern at base
        B       ramsize2        ; enter loop avoiding wraparound test
ramsize1
        LDRB    r6,[r2,#0]      ; save original contents in r6
        STRB    r3,[r2,#0]      ; store first pattern (inverse of base)
        LDRB    r0,[r1,#0]      ; see if it appears at base of memory
        CMP     r0,r3
        BEQ     ramsize3        ; yes - memory has wrapped round
ramsize2
        LDRB    r0,[r2,#0]      ; read back written location
        CMP     r0,r3           ; is it correct?
        BNE     ramsize3        ; no working memory here (and no abort!)
        STRB    r4,[r2,#0]      ; store second pattern
        LDRB    r0,[r2,#0]      ; read back written location
        CMP     r0,r4           ; ss it correct?
        STREQB  r6,[r2,#0]      ; yes - restore original contents
        ADDEQ   r2,r2,r5        ; yes - step pointer
        BEQ     ramsize1        ; and carry on

	; Address of first absent location in r2, original contents in r6
	; Original contents of base in r6
ramsize3
        STRB    r6,[r2,#0]      ; restore original contents of current location
        STRB    r4,[r1,#0]      ; restore original contents of base (2nd pat)
        SUB     r0,r2,r1        ; get size of memory in r0
        MOV     pc,lr

	; ---------------------------------------------------------------------
	; Enter USR mode

cmd_usr
        SWI     mon_WriteS
        =       "\nEntering USR mode\n",null
	ALIGN
        TEQP    pc,#0           ; Enter USR mode and clear all flags
                                ; `Fails' if already in USR mode!
        B       next_command

	; ---------------------------------------------------------------------
	; Enter SVC mode

cmd_svc
        SWI     mon_WriteS
        =       "\nEntering SVC mode\n",null
	ALIGN
        SWI     mon_EnterSVC	; Enter SVC mode via a SWI
        B       next_command

	; ---------------------------------------------------------------------
	; IRQ enable

cmd_inton
        SWI     mon_WriteS
        =       "\nEnabling IRQs\n",null
	ALIGN
        SWI     mon_IntOn
        B       next_command

	; ---------------------------------------------------------------------
	; IRQ disable

cmd_intoff
        SWI     mon_WriteS
        =       "\nDisabling IRQs\n",null
	ALIGN
        SWI     mon_IntOff
        B       next_command

	; ---------------------------------------------------------------------
	; Poll a word location 

cmd_pollword
        SWI     mon_ReadX	; get the address into r0
        MOV     r1,r0           ; pointer
poll1
        MOV     r2,#1024        ; loop count
poll2
	LDR     r0,[r1,#0]
        SUBS    r2,r2,#1        ; decrement
        BNE     poll2

	LDR	r0,=LINK0_base
	LDRB	r0,[r0,#LINK_rstatus]
        ANDS    r0,r0,#LINK_data
        BEQ     poll1

        B       next_command

	; ---------------------------------------------------------------------
	; Poll a byte location

cmd_pollbyte
        SWI     mon_ReadX	; get the address into r0
        MOV     r1,r0           ; pointer
pollb1
        MOV     r2,#1024        ; loop count
pollb2
        LDRB    r0,[r1,#0]
        SUBS    r2,r2,#1        ; decrement
        BNE     pollb2

	LDR	r0,=LINK0_base
	LDRB	r0,[r0,#LINK_rstatus]
        ANDS    r0,r0,#LINK_data
        BEQ     pollb1

        B       next_command
        
	; ---------------------------------------------------------------------

	[	(fontcode)
cmd_font
        BL      get             	; get the identifier
        AND	r0,r0,#&DF		; case insensitive
        SUBS    r0,r0,#"A"		; base identifier
        MOV     r1,#SRAM_base
        STR     r0,[r1,#print]		; and mark this new font
        B       next_command

	; ---------------------------------------------------------------------

cmd_ttyon
        MOV     r0,#&00   		; initialise varibles
        MOV     r2,#SRAM_base
        STR     r0,[r2,#x]
        MOV     r1,#maxY
        STR     r1,[r2,#y]
        STR     r0,[r2,#escflag]
        STR     r0,[r2,#print]
        BL      ImageClear
        B       next_command

	; ---------------------------------------------------------------------

cmd_ttyoff
        MOV	r0,#-1		; flag for print mode
        MOV     r2,#SRAM_base
        STR     r0,[r2,#print]
        B       next_command
	]	; EOF (fontcode)

	; ---------------------------------------------------------------------
	; Print name of current processor mode. Corrupts r0 and r1

printmode
	STMFD	sp!,{r0,r1,lk}
        AND     r1,lk,#MODEmask		; since pc would not give PSR bits
        ADRL    r0,mon_psr_modes
        ADD     r0,r0,r1,LSL #2		; r0 = r0 + (r1 * 4)
	SWI	mon_Write0
	LDMFD	sp!,{r0,r1,pc}^

	; ---------------------------------------------------------------------
	; *********************************************************************
	; The following IO routines are called only from SWI routines, so that
	; the monitor will still work in USR mode.
	; *********************************************************************
	; ---------------------------------------------------------------------
	; Get a character from the link, reflect it to the link, and return
	; it in r0. Reflection is disabled if r9 is zero.

getput 
        STMFD   sp!,{lr}	; save the link
        BL      get
        CMP     r9,#&00		; reflection enabled?
        BLNE    put
        LDMFD   sp!,{pc}^	; return

	; ---------------------------------------------------------------------
	; Get a character from the link and return it in r0

get
	STMFD	sp!,{r1,lk}
	LDR	r0,=LINK0_base
get_poll
        LDRB    r1,[r0,#LINK_rstatus]
	TST	r1,#LINK_data
	BEQ	get_poll

        LDRB    r0,[r0,#LINK_read]

	LDMFD	sp!,{r1,pc}^

	; ---------------------------------------------------------------------
	; Put the character in r0 out to the link, preserving all regs
	; OR TO THE LCD!!!

put
	STMFD   sp!,{r0,r1,r2,lr}
	[	(fontcode)
        MOV     r2,#SRAM_base
        LDR     r1,[r2,#print]
        CMP     r1,#0
        BMI     put1

        AND     r0,r0,#&FF
        BL      PrintChar
        B       put99
	]	; EOF (fontcode)
put1
	LDR	r2,=LINK0_base
put1_poll
        LDRB    r1,[r2,#LINK_wstatus]
	TST	r1,#LINK_data
	BEQ	put1_poll

        STRB    r0,[r2,#LINK_write]
put99
        LDMFD   sp!,{r0,r1,r2,pc}^

	; ---------------------------------------------------------------------
	; Read and reflect a hex number. Preserves all registers.
	;
	; set carry flag on error

getvalue 
        STMFD   sp!,{r1-r2,lr}
getvaluestart
        MOV     r1,#0           ; clear result register
        MOV     r2,#0           ; char count
        BL      getput          ; get first char
        BL      hexchar         ; convert it
        BPL     getvaluenext    ; ok
        CMP     r0,#" "         ; leading spaces
        BEQ     getvaluestart   ; .. are ignored
        B       getvaluefail    ; any other char is fatal
getvaluenext
        ADD     r2,r2,#1        ; inc count
        ORR     r1,r0,r1,LSL #4 ; merge result
getvaluecont
        BL      getput          ; get first char
        BL      hexchar         ; convert it
        BPL     getvaluenext    ; ok
        CMP     r0,#CarriageReturn	; terminate with CR
        BEQ     getvaluedone    ;
        CMP     r0,#" "         ; terminate with SPACE
        BEQ     getvaluedone    ;
        CMP     r0,#NewLine     ; terminate or LF
        BEQ     getvaluedone    ;
        CMP     r0,#127         ; delete?
        BEQ     getvalueundo    ; try undo
        CMP     r0,#8           ; backspace?
        BEQ     getvalueundo    ; try undo 
        B       getvaluefail    ; else fail
getvalueundo
        MOV     r1,r1,LSR #4    ; remove last nibble
        SUBS    r2,r2,#1        ; dec count
        BLE     getvaluestart   ; start again if last digit
        MOV     r0,#NewLine
        BL      put
        MOV     r0,r1           ; reecho the number
        BL      putvalue
        B       getvaluecont
getvaluedone
        MOV     r0,r1           ; the result
        LDMFD   r13!,{r1-r2,lr}
        BICS    pc,lr,#Cbit
getvaluefail
        MVN     r0,#0
        LDMFD   sp!,{r1-r2,lr}
        ORRS    pc,lr,#Cbit

	; ---------------------------------------------------------------------
	; Write the byte in r0 out in hex. Preserves all registers.

putbyteval
        STMFD   sp!,{r0-r2,lr}
        MOV     r1,r0           ; r1 holds original value
        MOV     r2,#32-28       ; r2 holds shift for next nybble
        B       putvalue1       ; Share code with putvalue

	; Write the word in r0 out in hex. Preserves all registers.

putvalue
        STMFD   sp!,{r0-r2,lr}
        MOV     r1,r0           ; r1 holds original value
        MOV     r2,#32-4        ; r2 holds shift for next nybble
putvalue1
        MOV     r0,r1,LSR r2
        AND     r0,r0,#15       ; Extract next nybble
        CMP     r0,#9
        ORRLS   r0,r0,#"0"
        ADDHI   r0,r0,#"A"-10
        BL      put
        SUBS    r2,r2,#4
        BPL     putvalue1

        LDMFD   sp!,{r0-r2,pc}^

	; ---------------------------------------------------------------------
	; Convert one hex digit to binary
	; entry: char in r0 (low byte masked)
	; +ve return low nibble of R0 = hex
	; -ve return indicates R0 = invalid hex char low byte...

hexchar
        AND     r0,r0,#&FF
        CMP     r0,#"0"
        BLT     hexcharerror
        CMP     r0,#"9"
        BGT     hexcharuc
        SUB     r0,r0,#"0"      ; leave (0 or) +ve result
        BICS    pc,r14,#Nbit    ; return with flags
hexcharuc
        CMP     r0,#"A"
        BLT     hexcharerror
        CMP     r0,#"F"
        BGT     hexcharlc
        SUB     r0,r0,#"A"-10   ; leave (0 or) +ve result
        BICS    pc,r14,#Nbit    ; return with flags
hexcharlc
        CMP     r0,#"a"
        BLT     hexcharerror
        CMP     r0,#"f"
        BGT     hexcharerror
        SUB     r0,r0,#"a"-10   ; leave (0 or) +ve result
        BICS    pc,r14,#Nbit    ; return with flags
hexcharerror
        AND     r0,r0,#&FF      ; but mask result to 8 bit
        ORRS    pc,lr,#Nbit

	; ---------------------------------------------------------------------
	; SWI handler

swi_handler
	NOP
        STMFD   sp!,{r1-r12,lr} ; r0 left alone unless result returned
        AND     r1,lr,#INTflags
        TEQP    r1,#SVCmode
        BIC     r1,lr,#&FC000003; Clear PSR 
        SUB     r3,r1,#4        ; Get address of SWI instruction in r3
	; Get SWI instruction word using address translation if user mode SWI
        TST     lr,#3           ; Test which mode we came from
        LDREQT  r2,[r3],#0      ; User: get instruction using addr translation
        LDRNE   r2,[r3],#0      ; SVC, IRQ or FIQ: no translation
        BIC     r2,r2,#&FF000000; Extract SWI number
        CMP     r2,#mon_MaxSWI  ; Is it one of the low-numbered SWIs?
        BHI     big_swi         ; No - outside range of table
        ADR     r3,swi_table
        ADD     pc,r3,r2,LSL #2 ; Jump into table

	; Jump through table with SWI number in r2, return address in r1.
	; Other registers still intact.

swi_table
	; BRAZIL emulations, where implemented...
        B       swi_writec      ;  0 mon_WriteC
        B       swi_writes      ;  1 mon_WriteS
        B       swi_write0      ;  2 mon_Write0
        B       swi_newline     ;  3 mon_NewLine
        B       swi_readc       ;  4 mon_ReadC
        B       bad_swi         ;  5 mon_CLI
	B	bad_swi		;  6 mon_Byte
        B       bad_swi         ;  7 mon_Word
        B       bad_swi         ;  8 mon_File
        B       bad_swi         ;  9 mon_Args
        B       bad_swi         ; 10 mon_BGet
        B       bad_swi         ; 11 mon_BPut
        B       bad_swi         ; 12 mon_GBPB
        B       bad_swi         ; 13 mon_Find
        B       bad_swi		; 14 mon_ReadLine
        B       bad_swi		; 15 mon_Control
        B       bad_swi		; 16 mon_GetEnv
        B       swi_exit        ; 17 mon_Exit
        B       bad_swi		; 18 mon_SetEnv
        B       swi_inton       ; 19 mon_IntOn
        B       swi_intoff      ; 20 mon_IntOff
        B       bad_swi         ; 21 mon_CallBack
        B       swi_svcmode     ; 22 mon_EnterOS
        B       bad_swi         ; 23 mon_BreakPt
        B       bad_swi         ; 24 mon_BreakCtrl
        B       bad_swi         ; 25 mon_Unused
        B       bad_swi         ; 26 mon_MEMC
        B       bad_swi         ; 27 mon_SetCallBack
        B       bad_swi         ; 28
        B       bad_swi         ; 29
        B       bad_swi         ; 30
        B       bad_swi         ; 31
	; extensions
        B       swi_readx       ; 32 READX
        B       swi_writex      ; 33 WRITEX
        B       swi_writexb     ; 34 WRITEXB
        B       swi_hwpeek      ; 35 HWPEEK
        B       swi_hwpoke      ; 36 HWPOKE
        B       swi_hwpokeb     ; 37 HWPOKEB

	; ---------------------------------------------------------------------
        ; Come here for SWIs outside the range of the table

big_swi
	BIC     r4,r2,#&FF      ; Clear bottom byte of SWI number
        CMP     r4,#mon_WriteI  ; in order to test for SWI mon_WriteI  range
        BEQ     swi_writei
	; and fall through to...
	; ---------------------------------------------------------------------
	; Unsupported SWI: number in r2, return address in r1
	; r0 still intact, r1-r12, r14 on stack
	; Set up for entry to generic trap code.

bad_swi
	LDMFD   sp!,{r1-r12}    ; Retrieve some regs
        MOV     r14,#mon_register_dump
        STMIA   r14,{r0-r7}     ; Dump unbanked regs
        MOV     r0,r14          ; Put dump area address in free reg
        LDMFD   sp!,{r14}       ; restore sp and aborted pc
        STR     r14,[r0,#(mon_dumped_r15 - mon_register_dump)] ; Dump aborted pc
        SWI     mon_WriteS          ; Recursive SWI loses r14_svc
        =       "\n*** Unimplemented SWI\n",null
        B       trap            ; Enter generic trap handler

	; ---------------------------------------------------------------------
	; SWI_EXIT - return to monitor
	; dumps registers
	; Set up for entry to generic trap code.

swi_exit
        LDMFD   sp!,{r1-r12}    ; Retrieve some regs
        MOV     r14,#mon_register_dump
        STMIA   r14,{r0-r7}     ; Dump unbanked regs
        MOV     r0,r14          ; Put dump area address in free reg
        LDMFD   sp!,{r14}       ; restore sp and aborted pc
        STR     r14,[r0,#(mon_dumped_r15 - mon_register_dump)] ; Dump aborted pc
        SWI     mon_WriteS          ; Recursive SWI loses r14_svc
        =       "\n*** EXIT TO MONITOR\n",null
        MOV     r9,#&01 	; ensure echo mode
        B       next_command

	; ---------------------------------------------------------------------
	; Common return from SWI. r0 is either undisturbed or contains result.

swi_ret
	LDMFD   sp!,{r1-r12,pc}^

	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; Print string immediately following SWI instruction and jump round it.
	; String address is in r1.

swi_writes
        MOV     r2,r0           ; r0 is not stacked so save it
        LDR     r3,[sp,#(12*4)] ; Get stacked lr for mode of caller
writes1
	; Get character and point at next.
        TST     r3,#MODEmask    ; Test mode of caller
        LDREQBT r0,[r1],#1      ; USR mode so force translation
        LDRNEB  r0,[r1],#1      ; non-USR mode
        CMP     r0,#0           ; Test for terminator
        BLNE    put             ; Print the character if not null
        BNE     writes1         ; Loop if not null

        MOV     r0,r2           ; Restore r0

	; Return to instruction following end of string
        ADD     r1,r1,#3        ; Point to 4th byte after string
        BIC     r1,r1,#3        ; Align to word after string
        LDR     lr,[sp,#(12*4)] ; Retrieve stacked lr
        AND     lr,lr,#&FC000003; Keep PSR bits
        ORR     lr,lr,r1        ; Replace address
        LDMFD   sp!,{r1-r12}    ; Restore other regs
        ADD     sp,sp,#4        ; Discard stacked lr
        MOVS    pc,lr           ; Return to instruction after string

	; ---------------------------------------------------------------------

swi_write0
	; print the string referenced by r0
	STMFD	sp!,{r1,lk}
	MOV	r1,r0
swi_write0_loop
	LDRB	r0,[r1],#&01
	TEQ	r0,#null
	BLNE	put
	BNE	swi_write0_loop
	LDMFD	sp!,{r1,lk}
	B	swi_ret		; and return

	; ---------------------------------------------------------------------

swi_newline
	; print CR/LF
	MOV	r0,#CarriageReturn
	BL	put
	MOV	r0,#NewLine
	BL	put
	B	swi_ret		; and return

	; ---------------------------------------------------------------------
	; mon_WriteC: print character in r0

swi_writec
        BL      put             ; Print the character
        B       swi_ret         ; and return

	; ---------------------------------------------------------------------
	; READC: read a character from the link and return in r0

swi_readc
        BL      get             ; Get char from link into r0
        B       swi_ret         ; and return

	; ---------------------------------------------------------------------
	; READX: read a hex number from the link and return value in r0

swi_readx
        BL      getvalue        ; Read number into r0
        B       swi_ret         ; and return

	; ---------------------------------------------------------------------
	; WRITEX: write out the word in r0 in hex

swi_writex
        BL      putvalue        ; Print word in r0
        B       swi_ret

	; ---------------------------------------------------------------------
	; WRITEXB: write out the byte in r0 in hex

swi_writexb
        BL      putbyteval      ; Print byte in r0
        B       swi_ret

	; ---------------------------------------------------------------------
	; SVCMODE: return in SVC mode

swi_svcmode
        ORR     lr,lr,#3        ; Set SVC mode in return PSR
        STR     lr,[sp,#(12*4)] ; Insert in stacked register set
        B       swi_ret

	; ---------------------------------------------------------------------
	; INTON: enable IRQ

swi_inton
        BIC     lr,lr,#Ibit   ; clear IRQ disable in return PSR
        STR     lr,[sp,#(12*4)] ; Insert in stacked register set
        B       swi_ret

	; ---------------------------------------------------------------------
	; INTOFF: disable IRQ

swi_intoff
        ORR     lr,lr,#Ibit   ; set IRQ disable in return PSR
        STR     lr,[sp,#(12*4)] ; Insert in stacked register set
        B       swi_ret

	; ---------------------------------------------------------------------
	; HWPEEK: stacked R1-> address, R0 returns WORD READ

swi_hwpeek
        LDR     r2,[sp,#(0*4)] ; get stacked register ptr
        BIC     r2, r2, #&FC000003 ; safe address
        LDR     r0, [r2]
        B       swi_ret         ; and return

	;----------------------------------------------------------------------
	; HWPOKE: R1-> address, R0 word data and returns WORD READ

swi_hwpoke
        LDR     r2,[sp,#(0*4)] ; get stacked register ptr
        BIC     r2, r2, #&FC000003 ; safe address
        MOV     r3, r0          ; save entry R0
        LDR     r0, [r2]        ; get original value
        STR     r3, [r2]        ; store new value
        B       swi_ret         ; and return

	; ---------------------------------------------------------------------
	; HWPOKE: R1-> address, R0 byte data and returns BYTE READ

swi_hwpokeb
        LDR     r2,[sp,#(0*4)] ; get stacked register ptr
        BIC     r2, r2, #&FC000000 ; safe address
        MOV     r3, r0          ; save entry R0
        LDRB    r0, [r2]        ; get original value
        STRB    r3, [r2]        ; store new value
        B       swi_ret         ; and return

	; ---------------------------------------------------------------------
	; mon_WriteI : print character in low byte of SWI number (in r2)

swi_writei
        MOV     r3,r0           ; Save original r0
        AND     r0,r2,#&FF      ; Get character into r0
        BL      put             ; Print the character
        MOV     r0,r3           ; Restore original r0
        B       swi_ret         ; and return

	; ---------------------------------------------------------------------
	; Undefined instruction handler

undef_handler
        MOVNV   r0, r0  ;;; NOP
        TEQP    pc,#(INTflags :OR: SVCmode) ; IRQ and FIQ disabled, SVC mode
        STMFD   sp!,{r0}        ; Save one register on stack
        MOV     r0,#mon_register_dump ; Get address of fixed dump area
        STMIA   r0,{r0-r7}      ; Save unbanked regs (with bogus r0)
        LDMFD   sp!,{r1}        ; Retrieve original r0 into r1
        STR     r1,[r0,#0]      ; Save correct r0 in register dump
        STR     r14,[r0,#(mon_dumped_r15 - mon_register_dump)] ; Save aborted pc

        SWI     mon_WriteS
        =       "\n*** Undefined instruction\n",null
        B       trap

	; ---------------------------------------------------------------------
	; Prefetch abort handler

pabort_handler
        MOVNV   r0, r0  ;;; NOP
        TEQP    pc,#(INTflags :OR: SVCmode) ; IRQ and FIQ disabled, SVC mode
        STMFD   sp!,{r0}        ; Save one register on stack
        MOV     r0,#mon_register_dump ; Get address of fixed dump area
        STMIA   r0,{r0-r7}      ; Save unbanked regs (with bogus r0)
        LDMFD   sp!,{r1}        ; Retrieve original r0 into r1
        STR     r1,[r0,#0]      ; Save correct r0 in register dump
        STR     r14,[r0,#(mon_dumped_r15 - mon_register_dump)] ; Save aborted pc

        SWI     mon_WriteS
        =       "\n*** Prefetch abort\n",null
        B       trap

	; ---------------------------------------------------------------------
	; Data abort handler

dabort_handler
        MOVNV   r0, r0  ;;; NOP
        TEQP    pc,#(INTflags :OR: SVCmode) ; IRQ and FIQ disabled, SVC mode
        STMFD   sp!,{r0}        ; Save one register on stack
        MOV     r0,#mon_register_dump ; Get address of fixed dump area
        STMIA   r0,{r0-r7}      ; Save unbanked regs (with bogus r0)
        LDMFD   sp!,{r1}        ; Retrieve original r0 into r1
        STR     r1,[r0,#0]      ; Save correct r0 in register dump
        STR     r14,[r0,#(mon_dumped_r15 - mon_register_dump)] ; Save aborted pc

        SWI     mon_WriteS
        =       "\n*** Data abort\n",null
        B       trap

	; ---------------------------------------------------------------------
	; Address exception handler

addrex_handler
        MOVNV   r0, r0  ;;; NOP
        TEQP    pc,#(INTflags :OR: SVCmode) ; IRQ and FIQ disabled, SVC mode
        STMFD   sp!,{r0}        ; Save one register on stack
        MOV     r0,#mon_register_dump ; Get address of fixed dump area
        STMIA   r0,{r0-r7}      ; Save unbanked regs (with bogus r0)
        LDMFD   sp!,{r1}        ; Retrieve original r0 into r1
        STR     r1,[r0,#0]      ; Save correct r0 in register dump
        STR     r14,[r0,#(mon_dumped_r15 - mon_register_dump)] ; Save aborted pc

        SWI     mon_WriteS
        =       "\n*** Address exception\n",null
        B       trap

	; ---------------------------------------------------------------------
	; branch through 0 (RAM reset vector) trap

branch0_trap
        MOVNV   r0, r0  ;;; NOP
        TEQP    pc,#(INTflags :OR: SVCmode) ; IRQ and FIQ disabled, SVC mode
        STMFD   sp!,{r0}        ; Save one register on stack
        MOV     r0,#mon_register_dump ; Get address of fixed dump area
        STMIA   r0,{r0-r7}      ; Save unbanked regs (with bogus r0)
        LDMFD   sp!,{r1}        ; Retrieve original r0 into r1
        STR     r1,[r0,#0]      ; Save correct r0 in register dump
        STR     r14,[r0,#(mon_dumped_r15 - mon_register_dump)] ; Save aborted pc

        SWI     mon_WriteS
        =       "\n*** Branch through 0 trap\n",null
        B       trap

	; ---------------------------------------------------------------------
	; IRQ handler

irq_handler
        MOVNV   r0, r0  ;;; NOP
        TEQP    pc,#(INTflags :OR: SVCmode) ; IRQ and FIQ disabled, SVC mode
        STMFD   sp!,{r0}        ; Save one register on stack
        MOV     r0,#mon_register_dump ; Get address of fixed dump area
        STMIA   r0,{r0-r7}      ; Save unbanked regs (with bogus r0)
        LDMFD   sp!,{r1}        ; Retrieve original r0 into r1
        STR     r1,[r0,#0]      ; Save correct r0 in register dump
        STR     r14,[r0,#(mon_dumped_r15 - mon_register_dump)] ; Save aborted pc

        SWI     mon_WriteS
        =       "\n*** IRQ\n",null
        B       trap

	; ---------------------------------------------------------------------
	; FIQ handler

fiq_handler
        MOVNV   r0, r0  ;;; NOP
        TEQP    pc,#(INTflags :OR: SVCmode) ; IRQ and FIQ disabled, SVC mode
        STMFD   sp!,{r0}        ; Save one register on stack
        MOV     r0,#mon_register_dump ; Get address of fixed dump area
        STMIA   r0,{r0-r7}      ; Save unbanked regs (with bogus r0)
        LDMFD   sp!,{r1}        ; Retrieve original r0 into r1
        STR     r1,[r0,#0]      ; Save correct r0 in register dump
        STR     r14,[r0,#(mon_dumped_r15 - mon_register_dump)] ; Save aborted pc

        SWI     mon_WriteS
        =       "\n*** FIQ\n",null
        B       trap

	; ---------------------------------------------------------------------
	; Common exit for all unhandled traps. Entered in SVC mode with ints
	; off. Expects r0-r7 and aborted pc to be stored at `mon_register_dump'
	; Reenters the monitor on exit.

trap
	MOV     r3,#mon_register_dump       ; Address for the (possibly)
        ADD     r3,r3,#(mon_dumped_r8 - mon_register_dump) ; mapped regs

        LDR     r2,[r3,#(mon_dumped_r15 - mon_dumped_r8)] ; Retrieve aborted pc
        TST     r2,#MODEmask
        STMEQIA r3,{r8-r14}^            ; USR mode aborted
        BEQ     printregs

        TST     r2,#IRQmode
        TSTNE   r2,#FIQmode
        BNE     mon_wasSVCmode

	; IRQ or FIQ mode abort
        ORR     r2,r2,#INTflags         ; ensure IRQ/FIQ disabled
        TEQP    r2,#&00                 ; and enter the aborted mode
        MOVNV   r0,r0                   ; allow time for register remapping
        STMIA   r3,{r8-r14}             ; and stack the correct registers
        TEQP    pc,#(INTflags :OR: SVCmode) ; and re-enter SVC mode (IRQ/FIQ off)
        B       printregs

deadval &       &DEADDEAD

mon_wasSVCmode      ; we were in SVC mode when the abort occured
        STMIA   r3,{r8-r13}             ; and stack the easy registers
        LDR     r2,deadval           ; the abort corrupted svc_r14
        STR     r2,[r3,#(mon_dumped_r14 - mon_dumped_r8)]

	; All registers now dumped into "mon_register_dump"
	; SVC mode; IRQs and FIQs disabled
printregs
        SWI     mon_NewLine
        MOV     r1,#mon_register_dump   ; r1 holds pointer into dump
        MOV     r2,#0                   ; r2 holds register number
printregs1
        SWI     mon_WriteI + "r"
        MOV     r3,r2                   ; Copy register number
        CMP     r3,#9                   ; Big register number?
        SUBHI   r3,r3,#10               ; Get second digit
        SWIHI   mon_WriteI + "1"        ; Write first digit
        ADD     r0,r3,#"0"              ; Char for second (or only) digit
        SWI     mon_WriteC
        CMP     r2,#9                   ; Leave space after single digit
        SWILS   mon_WriteI + " "
        SWI     mon_WriteI + " "
        LDR     r0,[r1],#4              ; Get register contents
        SWI     mon_WriteX              ; and print
        ADD     r2,r2,#1                ; Next register number
        TST     r2,#3                   ; Multiple of 4?
        SWIEQ   mon_NewLine             ; Yes - start new line
        SWINE   mon_WriteI + " "        ; No - print spaces
        SWINE   mon_WriteI + " "      
        CMP     r2,#16                  ; Printed all regs?
        BNE     printregs1              ; Not yet

	; ---------------------------------------------------------------------
	; Reenter monitor
reenter
        SWI     mon_WriteS
        =       "\n*** Re-entering monitor in ",null
        BL      printmode
        SWI     mon_WriteS
        =       " mode\n",null
        B       monitor_reset

	; ---------------------------------------------------------------------

	[	(fontcode)

;*********************************************************
;**     HERCULES huge font (Courier) TTY emulator       **
;**                                                     **
;**     Author: David Flynn                             **
;**                                                     **
;**     lastEdit 17/9/90                                **
;**                                                     **
;**     (Based on an early laser printer test system)   **
;**                                                     **
;**     simple wrapping/scrolling text display          **
;**     for 640x400 LCD screen                          **
;*********************************************************

;* SPECIFICATION
;*
;* ASCII characters 32 to 126 are printed directly
;*  (with simple auto wrap)
;*
;* ASCII charcters greater than 127 are ignored
;*
;* ASCII control codes are ignored except for:
;*
;*  HT   (9) move to next 8 character boundary
;*  LF  (10) move down one line (and force new page at bottom)
;*  FF  (12) print the page and force new page
;*  CR  (13) move to left hand margin
;*  ESC (27) escape code prefix : one character code follows
;*         "B" or "b" for Bold style
;*         "I" or "i" for Italic style
;*         "N" or "n" for Normal text style
;*         "R" or "r" for Reverse video style
;*         any other escape charcter is ignored (no effect)
;*  DEL (127) backspace (but non-rubout)
;


CharOffset   * 32 ; start of printable chars

maxX         * 70 ; 95  ; 96 characters horizontally
maxY         * 24 ; 91  ; 92 lines vertically
Spacing      * 9  ; for 300 d.p.i at 10 point
Height       * 16 ; 37  ; for 300 d.p.i 10 point


; simple print style emulations FLAGS
NormalPrint  * 0
BoldPrint    * 1
ItalicPrint  * 2
ReversePrint * 4


TTYinit
        MOV     R0, #0          ; init varibles
        MOV     R2, #0
        STR     R0, [R2, #x]
        MOV     R1, #maxY
        STR     R1, [R2, #y]
        STR     R0, [R2, #escflag]
        STR     R0, [R2, #print]
        B       ImageClear      ; and return

	; ---------------------------------------------------------------------
; ** char in R0
;  may use R1, R2
PrintChar
  STMFD R13!, {R3,R4,R5,R12,R14}                ; save char
  MOV   R12,#0          ; R12 -> vars in page 0
  LDR   R1, [R12, #escflag]
;
  CMP R1,#0
  BNE ESCsequence
  CMP R0,#127
  BGT PrintDone
  BEQ print_Delete
  CMP R0,#32
  ADDLO PC,PC,R0,LSL #2
  B PutChar
JumpTable
  B print_Ignore ; 0
  B print_Ignore ; 1
  B print_Ignore ; 2
  B print_Ignore ; 3
  B print_Ignore ; 4
  B print_Ignore ; 5
  B print_Ignore ; 6
  B print_Ignore ; 7
  B print_BackSpace ; 8
  B print_HTab ; 9
  B print_NewLine ; 10
  B print_Ignore ; 11
  B print_NewPage ; 12
  B print_Return ; 13
  B print_Ignore ; 14
  B print_Ignore ; 15
  B print_Ignore ; 16
  B print_Ignore ; 17
  B print_Ignore ; 18
  B print_Ignore ; 19
  B print_Ignore ; 20
  B print_Ignore ; 21
  B print_Ignore ; 22
  B print_Ignore ; 23
  B print_Ignore ; 24
  B print_Ignore ; 25
  B print_Ignore ; 26
  B print_Escape ; 27 &1B ESC
  B print_Ignore ; 28
  B print_Ignore ; 29
  B print_Ignore ; 30
  B print_Ignore ; 31
print_Delete
print_BackSpace
  LDR   R1, [R12, #x]
  CMP   R1,#0
  SUBGT R1,R1,#1
  B     XUpdate
print_HTab
  LDR   R1, [R12, #x]
  ADD   R1,R1,#8
  BIC   R1,R1,#7
  B     XUpdate
print_NewLine
  MOV   R1,#0
  STR   R1, [R12, #x]
print_LineFeed
  LDR   R1, [R12, #y]
  SUBS  R1,R1,#1
  STRPL R1, [R2, #y]
  BPL   PrintDone
; new scroll code
  MOV   R0,#Height
  BL    ImageScroll
  B     PrintDone

print_NewPage
  BL    ImageClear
  MOV   R0,#maxY
  STR   R0, [R12, #y]
;
print_Return
  MOV   R1,#0
XUpdate
  STR   R1, [R12, #x]
;
;;Ignore
  
PrintDone
  LDMFD R13!, {R3,R4,R5,R12,PC}         ; and exit

print_Escape
  MOV   R1,#&FF
  STR   R1, [R12, #escflag]
  B     PrintDone

print_Ignore
PutChar
  MOV   R2,R0      ; save char in R0
  LDR   R4, [R12, #x]
  CMP   R4,#maxX
  BLE   PutChar1
; auto line feed
  MOV   R0,#0
  STR   R0, [R12, #x]
  LDR   R0, [R12, #y]
  SUBS  R0, R0, #1
  STRPL R0, [R12, #y]
  BPL   PutChar0
; new scroll code
  MOV   R0,#Height
  BL    ImageScroll
PutChar0
  MOV   R4,#0
PutChar1
  MOV   R3,#Spacing
  MUL   R0,R4,R3
  ADD   R4,R4,#1
  STR   R4,[R12, #x]
  LDR   R5,[R12, #y]
  MOV   R3,#Height
  MUL   R1,R5,R3
  ADD   R1,R1,R3 ; offset!
  LDR   R3,[R12, #print]
  MOV   R4, R0          ; preserve R0
  BL    Char10
; shadow mode
  ADD   R0, R4, #1
  ADD   R0, R0, #1024
  BL    Char10
  B     PrintDone

ESCsequence
  CMP R0, #&1B  ;<ESC><ESC> for exit
;;  SWIEQ OS_Exit
; else parse for valid print attribute
  BIC R0,R0,#&20 ; case insensitive
  SUBS  R0, R0, #"A"  ; -ve will turn off LCD tty!
  STR   R0, [R12, #print]
  MOV   R0, #0
  STR   R0, [R12, #escflag]
  B PrintDone

;****************************
; RAM variables... moved to RAM0
;;TTYdata
;;x       & 0
;;y       & 0 ;;maxY
;;escflag & 0
;;print   & NormalPrint

;****************************
ImageClear
  STMFD R13!,{R0-R9,R14}
  ADR   R3,CharBltData
  LDMIA R3,{R7,R8,R9}
  MLA   R5,R9,R7,R8
  MOV   R0,#0
  MOV   R1,#0
  MOV   R2,#0
  MOV   R3,#0
ImageClear1
  STMIA R8!,{R0,R1,R2,R3}
  STMIA R8!,{R0,R1,R2,R3}
  STMIA R8!,{R0,R1,R2,R3}
  STMIA R8!,{R0,R1,R2,R3}
  CMP   R8,R5
  BLO   ImageClear1
  LDMFD R13!,{R0-R9,PC}


;****************************
;* SCROLL image up N rasters
;* R0 is number of lines to scroll UP
;****************************
ImageScroll
  STMFD R13!,{R0-R9,R14}
  ADR   R3,CharBltData
  LDMIA R3,{R7,R8,R9}           ; R7 = raster offset, R8 = screen base
  MLA   R5,R9,R7,R8             ; R9 = max raster, R5 = end address
  MLA   R4,R0,R7,R8             ; R4 is copy source pointer
  SUB   R3,R9,R0                ; R3 is blank start raster
  MLA   R9,R3,R7,R8             ; R9 is now start of blank region  

; copy region
ImageScroll1
  LDMIA R4!,{R0,R1,R2,R3}
  STMIA R8!,{R0,R1,R2,R3}
  LDMIA R4!,{R0,R1,R2,R3}
  STMIA R8!,{R0,R1,R2,R3}

  CMP   R4,R9
  BLO   ImageScroll1

; copy and blank region
  MOV   R0,#0
  MOV   R1,#0
ImageScroll2
  LDMIA R4,{R2,R3}
  STMIA R4!,{R0,R1}
  STMIA R8!,{R2,R3}
  LDMIA R4,{R2,R3}
  STMIA R4!,{R0,R1}
  STMIA R8!,{R2,R3}
  LDMIA R4,{R2,R3}
  STMIA R4!,{R0,R1}
  STMIA R8!,{R2,R3}
  LDMIA R4,{R2,R3}
  STMIA R4!,{R0,R1}
  STMIA R8!,{R2,R3}

  CMP   R4,R5
  BLO   ImageScroll2

  LDMFD R13!,{R0-R9,PC}

;****************************
;** charblt
;** R0 is X
;** R1 is Y
;** R2 is char number
;** R3 is font number
;****************************
Char10
  STMFD R13!,{R2,R3,R4,R5,R6,R7,R8,R9,R10,R14}
; check top bit
  MOV   R10, #0         ; EOR mask
  TST   R2, #&80        ; check top bit
  MOVNE R10, #&FF       ; reverse video if > 128
  ORRNE R10, R10, #&100
; make valid char
  AND   R2, R2, #&7F    ; mask to 7-bit
; font checking
  CMP   R3, #20         ; fonts 0-20 valid...
  MOVGT R3, #0          ; make pointer safe
; control chars use font 0
  CMP   R2, #CharOffset
  MOVLT R3, #0          ; use font 0 for control chars
  CMP   R2, #127
  MOVEQ R3, #0
; merge char and font pointers
  ORR   R2, R2, R3, LSL #7 ; font no * 128 + char no
; start the BLT code
  ADR   R3,CharBltData
CharBlt
  LDMIA R3,{R7,R8,R9}
  ADRL  R6,FontBitMap
;;;  LDMIA R6!,{R4,R5}
  MOV   R4,#Spacing
  MOV   R5,#Height
;;;
  AND   R3,R0,#&1F ; extract start offset in bits
  ADD   R0,R0,R4   ; advance X
  STMFD R13!,{R0,R1} ; as return param
  SUB   R0,R0,R4   ; restore X
  BIC   R0,R0,#&1F ; mask to word boundary
  SUBS  R1,R9,R1
  CMP   R1,#0
  MOVLT R1,#0
  MLA   R8,R7,R1,R8 ; compute line byte address
  ADD   R8,R8,R0,LSR #3 ; add in X byte offset
  RSB   R4,R3,#32
;;;  SUB   R2,R2,#CharOffset
;;;  MOV   R2,R2,LSL #2 ; scale char no. to word
  MLA   R6,R2,R5,R6 ; point to bit map
CharBltLine
  LDMIA R8,{R0,R1} ; get 64 bits
;;;  LDR   R2,[R6],#4 ; and line of char
  LDRB  R2,[R6],#1 ; and line of char
;;; and EOR code
  EOR   R2, R2, R10
;;;
  ORR   R0,R0,R2,LSL R3 ; merge low
  ORR   R1,R1,R2,LSR R4 ; and high
  STMIA R8,{R0,R1} ; and replace
  ADD   R8,R8,R7   ; scan advance
  SUBS  R5,R5,#1   ; line count
  BGT   CharBltLine
  LDMFD R13!,{R0,R1,R2,R3,R4,R5,R6,R7,R8,R9,R10,PC}

	;****************************
	; ROMmable constants...

CharBltData
BytesPerLine  & 256
ImagePtr      & LCD_base
LinesPerPage  & 400
	]	; EOF (fontcode)

	; ---------------------------------------------------------------------

screeninit
	!	0,"TODO: *** read screen base address from LCD DMA register"
	;		     on Hercules systems only
        LDR     r0,=LCD_base
        ADRL    r1,ScreenImage	; image is 16bytes wide
	MOV	r2,#130		; number of rasters
screeninit_loop
	LDMIA	r1!,{r3,r4,r5,r6}	; data fetch (16bytes)
	STMIA	r0,{r3,r4,r5,r6}	; and copy
	; step onto the next raster
	ADD	r0,r0,#LCD_stride
	SUBS	r2,r2,#1
	BNE	screeninit_loop

        MOVS	pc,lk

	; ---------------------------------------------------------------------

	LTORG

	; ---------------------------------------------------------------------

	[	(fontcode)
FontBitMap	; in reality a collection of font bitmaps
; FONT: <BOLD>
 = &04,&0A,&11,&00,&00,&70,&88,&A8,&E8,&08,&08,&F0,&00,&00,&00,&00 ;  ; 0 
 = &04,&0A,&11,&00,&00,&70,&88,&88,&F8,&88,&88,&88,&00,&00,&00,&00 ;  ; 1 
 = &04,&0A,&11,&00,&00,&78,&88,&88,&78,&88,&88,&78,&00,&00,&00,&00 ;  ; 2 
 = &04,&0A,&11,&00,&00,&70,&88,&08,&08,&08,&88,&70,&00,&00,&00,&00 ;  ; 3 
 = &04,&0A,&11,&00,&00,&78,&88,&88,&88,&88,&88,&78,&00,&00,&00,&00 ;  ; 4 
 = &04,&0A,&11,&00,&00,&F8,&08,&08,&78,&08,&08,&F8,&00,&00,&00,&00 ;  ; 5 
 = &04,&0A,&11,&00,&00,&F8,&08,&08,&78,&08,&08,&08,&00,&00,&00,&00 ;  ; 6 
 = &04,&0A,&11,&00,&00,&70,&88,&08,&E8,&88,&88,&70,&00,&00,&00,&00 ;  ; 7 
 = &04,&0A,&11,&00,&00,&88,&88,&88,&F8,&88,&88,&88,&00,&00,&00,&00 ;  ; 8 
 = &04,&0A,&11,&00,&00,&70,&20,&20,&20,&20,&20,&70,&00,&00,&00,&00 ;  ; 9 
 = &04,&0A,&11,&00,&00,&E0,&40,&40,&40,&40,&48,&30,&00,&00,&00,&00 ;  ; 10 
 = &04,&0A,&11,&00,&00,&88,&48,&28,&18,&28,&48,&88,&00,&00,&00,&00 ;  ; 11 
 = &04,&0A,&11,&00,&00,&08,&08,&08,&08,&08,&08,&F8,&00,&00,&00,&00 ;  ; 12 
 = &04,&0A,&11,&00,&00,&88,&D8,&A8,&88,&88,&88,&88,&00,&00,&00,&00 ;  ; 13 
 = &04,&0A,&11,&00,&00,&88,&98,&A8,&C8,&88,&88,&88,&00,&00,&00,&00 ;  ; 14 
 = &04,&0A,&11,&00,&00,&70,&88,&88,&88,&88,&88,&70,&00,&00,&00,&00 ;  ; 15 
 = &04,&0A,&11,&00,&00,&78,&88,&88,&78,&08,&08,&08,&00,&00,&00,&00 ;  ; 16 
 = &04,&0A,&11,&00,&00,&70,&88,&88,&88,&A8,&48,&B0,&00,&00,&00,&00 ;  ; 17 
 = &04,&0A,&11,&00,&00,&78,&88,&88,&78,&28,&48,&88,&00,&00,&00,&00 ;  ; 18 
 = &04,&0A,&11,&00,&00,&70,&88,&08,&70,&80,&88,&70,&00,&00,&00,&00 ;  ; 19 
 = &04,&0A,&11,&00,&00,&F8,&20,&20,&20,&20,&20,&20,&00,&00,&00,&00 ;  ; 20 
 = &04,&0A,&11,&00,&00,&88,&88,&88,&88,&88,&88,&70,&00,&00,&00,&00 ;  ; 21 
 = &04,&0A,&11,&00,&00,&88,&88,&88,&88,&50,&50,&20,&00,&00,&00,&00 ;  ; 22 
 = &04,&0A,&11,&00,&00,&88,&88,&88,&88,&A8,&D8,&88,&00,&00,&00,&00 ;  ; 23 
 = &04,&0A,&11,&00,&00,&88,&88,&50,&20,&50,&88,&88,&00,&00,&00,&00 ;  ; 24 
 = &04,&0A,&11,&00,&00,&88,&88,&88,&50,&20,&20,&20,&00,&00,&00,&00 ;  ; 25 
 = &04,&0A,&11,&00,&00,&F8,&80,&40,&20,&10,&08,&F8,&00,&00,&00,&00 ;  ; 26 
 = &04,&0A,&11,&00,&00,&70,&10,&10,&10,&10,&10,&70,&00,&00,&00,&00 ;  ; 27 
 = &04,&0A,&11,&00,&00,&08,&08,&10,&20,&40,&80,&80,&00,&00,&00,&00 ;  ; 28 
 = &04,&0A,&11,&00,&00,&70,&40,&40,&40,&40,&40,&70,&00,&00,&00,&00 ;  ; 29 
 = &04,&0A,&11,&00,&00,&20,&70,&A8,&20,&20,&20,&20,&00,&00,&00,&00 ;  ; 30 
 = &04,&0A,&11,&00,&00,&40,&20,&10,&F8,&10,&20,&40,&00,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &38,&38,&38,&38,&38,&38,&38,&38,&00,&00,&38,&38,&00,&00,&00,&00 ;  ; 33 !
 = &E7,&E7,&66,&24,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &E7,&E7,&E7,&FF,&FF,&66,&66,&FF,&FF,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 35 #
 = &00,&30,&FE,&33,&33,&33,&7E,&CC,&CC,&CC,&7F,&0C,&00,&00,&00,&00 ;  ; 36 $
 = &06,&8F,&CF,&E6,&70,&38,&1C,&0E,&67,&F3,&F1,&60,&00,&00,&00,&00 ;  ; 37 %
 = &00,&0E,&1B,&1B,&1B,&0E,&0E,&9B,&F3,&63,&F6,&9C,&00,&00,&00,&00 ;  ; 38 &
 = &38,&38,&38,&1C,&0C,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &78,&3C,&1E,&0F,&07,&07,&07,&07,&0F,&1E,&3C,&78,&00,&00,&00,&00 ;  ; 40 (
 = &1E,&3C,&78,&F0,&E0,&E0,&E0,&E0,&F0,&78,&3C,&1E,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&5D,&7F,&3E,&7F,&7F,&3E,&7F,&5D,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&38,&38,&38,&FE,&FE,&38,&38,&38,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&38,&38,&38,&1C,&0C,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&FE,&FE,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&38,&38,&38,&00,&00,&00,&00 ;  ; 46 .
 = &00,&80,&C0,&E0,&70,&38,&1C,&0E,&07,&03,&01,&00,&00,&00,&00,&00 ;  ; 47 /
 = &3C,&7E,&E7,&E7,&E7,&F7,&FF,&EF,&E7,&E7,&7E,&3C,&00,&00,&00,&00 ;  ; 48 0
 = &38,&3C,&3E,&38,&38,&38,&38,&38,&38,&FE,&FE,&FE,&00,&00,&00,&00 ;  ; 49 1
 = &7F,&FF,&E0,&E0,&E0,&FE,&7F,&07,&07,&07,&FF,&FE,&00,&00,&00,&00 ;  ; 50 2
 = &7F,&FF,&E0,&E0,&E0,&FE,&FE,&E0,&E0,&E0,&FF,&7F,&00,&00,&00,&00 ;  ; 51 3
 = &E0,&70,&38,&1C,&0E,&E7,&E7,&FF,&FF,&E0,&E0,&E0,&00,&00,&00,&00 ;  ; 52 4
 = &FE,&FF,&07,&07,&07,&7F,&FE,&E0,&E0,&E0,&FF,&7F,&00,&00,&00,&00 ;  ; 53 5
 = &7E,&7F,&07,&07,&7F,&FF,&E7,&E7,&E7,&E7,&FF,&7E,&00,&00,&00,&00 ;  ; 54 6
 = &7F,&FF,&E0,&E0,&E0,&E0,&E0,&E0,&E0,&E0,&E0,&E0,&00,&00,&00,&00 ;  ; 55 7
 = &7E,&FF,&E7,&E7,&E7,&FF,&FF,&E7,&E7,&E7,&FF,&7E,&00,&00,&00,&00 ;  ; 56 8
 = &7E,&FF,&E7,&E7,&E7,&FF,&FE,&E0,&E0,&E0,&E0,&E0,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&00,&38,&38,&38,&00,&00,&38,&38,&38,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&00,&38,&38,&38,&00,&00,&38,&38,&38,&1C,&0C,&00,&00 ;  ; 59 ;
 = &E0,&70,&38,&1C,&0E,&07,&07,&0E,&1C,&38,&70,&E0,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&7F,&7F,&00,&00,&7F,&7F,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &07,&0E,&1C,&38,&70,&E0,&E0,&70,&38,&1C,&0E,&07,&00,&00,&00,&00 ;  ; 62 >
 = &7E,&FF,&E7,&E0,&E0,&70,&38,&38,&00,&00,&38,&38,&00,&00,&00,&00 ;  ; 63 ?
 = &7C,&7E,&E7,&E7,&E7,&F7,&F7,&F7,&07,&07,&FE,&FC,&00,&00,&00,&00 ;  ; 64 @
 = &3C,&7E,&E7,&E7,&E7,&FF,&FF,&E7,&E7,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 65 A
 = &7F,&FF,&E7,&E7,&F7,&7F,&FF,&E7,&E7,&E7,&FF,&7F,&00,&00,&00,&00 ;  ; 66 B
 = &3C,&7E,&E7,&E7,&07,&07,&07,&07,&E7,&E7,&7E,&3C,&00,&00,&00,&00 ;  ; 67 C
 = &3F,&7F,&E7,&E7,&E7,&E7,&E7,&E7,&E7,&E7,&7F,&3F,&00,&00,&00,&00 ;  ; 68 D
 = &FF,&FF,&07,&07,&07,&3F,&3F,&07,&07,&07,&FF,&FF,&00,&00,&00,&00 ;  ; 69 E
 = &FF,&FF,&07,&07,&07,&3F,&3F,&07,&07,&07,&07,&07,&00,&00,&00,&00 ;  ; 70 F
 = &3C,&7E,&E7,&E7,&07,&F7,&F7,&E7,&E7,&E7,&7E,&3C,&00,&00,&00,&00 ;  ; 71 G
 = &E7,&E7,&E7,&E7,&E7,&FF,&FF,&E7,&E7,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 72 H
 = &7C,&7C,&38,&38,&38,&38,&38,&38,&38,&38,&7C,&7C,&00,&00,&00,&00 ;  ; 73 I
 = &F0,&F0,&E0,&E0,&E0,&E0,&E0,&E0,&E7,&E7,&7E,&3C,&00,&00,&00,&00 ;  ; 74 J
 = &E7,&E7,&77,&3F,&1F,&0F,&1F,&3F,&7F,&F7,&E7,&E7,&00,&00,&00,&00 ;  ; 75 K
 = &07,&07,&07,&07,&07,&07,&07,&07,&07,&07,&FF,&FF,&00,&00,&00,&00 ;  ; 76 L
 = &C3,&E7,&FF,&FF,&FF,&FF,&E7,&E7,&E7,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 77 M
 = &E3,&E7,&EF,&FF,&FF,&F7,&E7,&E7,&E7,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 78 N
 = &3C,&7E,&E7,&E7,&E7,&E7,&E7,&E7,&E7,&E7,&7E,&3C,&00,&00,&00,&00 ;  ; 79 O
 = &7F,&FF,&E7,&E7,&E7,&FF,&7F,&07,&07,&07,&07,&07,&00,&00,&00,&00 ;  ; 80 P
 = &3C,&7E,&E7,&E7,&E7,&E7,&E7,&E7,&E7,&E7,&7E,&FC,&E0,&C0,&00,&00 ;  ; 81 Q
 = &7F,&FF,&E7,&E7,&E7,&7F,&7F,&F7,&E7,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 82 R
 = &7E,&FF,&E7,&E7,&07,&7F,&FE,&E0,&E7,&E7,&FF,&7E,&00,&00,&00,&00 ;  ; 83 S
 = &7F,&7F,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&00,&00,&00,&00 ;  ; 84 T
 = &E7,&E7,&E7,&E7,&E7,&E7,&E7,&E7,&E7,&E7,&FF,&7E,&00,&00,&00,&00 ;  ; 85 U
 = &E7,&E7,&E7,&E7,&E7,&E7,&E7,&E7,&E7,&7E,&3C,&18,&00,&00,&00,&00 ;  ; 86 V
 = &E7,&E7,&E7,&E7,&E7,&E7,&FF,&FF,&FF,&FF,&66,&66,&00,&00,&00,&00 ;  ; 87 W
 = &E7,&E7,&E7,&E7,&7E,&3C,&3C,&7E,&E7,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 88 X
 = &77,&77,&77,&77,&7F,&7F,&3E,&1C,&1C,&1C,&1C,&1C,&00,&00,&00,&00 ;  ; 89 Y
 = &FF,&FF,&E0,&E0,&70,&38,&1C,&0E,&07,&07,&FF,&FF,&00,&00,&00,&00 ;  ; 90 Z
 = &7E,&7E,&0E,&0E,&0E,&0E,&0E,&0E,&0E,&0E,&7E,&7E,&00,&00,&00,&00 ;  ; 91 [
 = &00,&01,&03,&07,&0E,&1C,&38,&70,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 92 \
 = &7E,&7E,&70,&70,&70,&70,&70,&70,&70,&70,&7E,&7E,&00,&00,&00,&00 ;  ; 93 ]
 = &18,&3C,&7E,&E7,&C3,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&FF,&00,&00,&00,&00 ;  ; 95 _
 = &1C,&1C,&1C,&38,&30,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&3E,&7E,&70,&7E,&7F,&77,&FF,&FE,&00,&00,&00,&00 ;  ; 97 a
 = &07,&07,&07,&07,&7F,&FF,&E7,&E7,&E7,&E7,&FF,&7F,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&7C,&FE,&E7,&07,&07,&E7,&FE,&7C,&00,&00,&00,&00 ;  ; 99 c
 = &E0,&E0,&E0,&E0,&FE,&FF,&E7,&E7,&E7,&E7,&FF,&FE,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&3C,&7E,&E7,&FF,&FF,&07,&7E,&7C,&00,&00,&00,&00 ;  ; 101 e
 = &7C,&FE,&EE,&0E,&7F,&7F,&0E,&0E,&0E,&0E,&0E,&0E,&00,&00,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&3C,&7E,&E7,&E7,&E7,&FE,&FC,&E0,&7E,&3E,&00,&00 ;  ; 103 g
 = &07,&07,&07,&07,&7F,&FF,&E7,&E7,&E7,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 104 h
 = &00,&1C,&1C,&00,&1E,&1E,&1C,&1C,&1C,&1C,&3E,&3E,&00,&00,&00,&00 ;  ; 105 i
 = &00,&E0,&E0,&00,&F0,&F0,&E0,&E0,&E0,&E0,&E0,&E7,&FF,&7E,&00,&00 ;  ; 106 j
 = &07,&07,&07,&C7,&E7,&F7,&7F,&3F,&77,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 107 k
 = &1E,&1E,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&3E,&3E,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&77,&FF,&FF,&DB,&DB,&DB,&DB,&DB,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&3F,&7F,&E7,&E7,&E7,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&3C,&7E,&E7,&E7,&E7,&E7,&7E,&3C,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&3F,&7F,&E7,&E7,&E7,&7F,&3F,&07,&07,&07,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&FC,&FE,&E7,&E7,&E7,&FE,&FC,&E0,&E0,&E0,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&7F,&FF,&E7,&07,&07,&07,&07,&07,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&FE,&FF,&07,&7F,&FE,&E0,&FF,&7F,&00,&00,&00,&00 ;  ; 115 s
 = &00,&1C,&1C,&1C,&7F,&7F,&1C,&1C,&1C,&1C,&FC,&F8,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&E7,&E7,&E7,&E7,&E7,&E7,&FE,&FC,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&E7,&E7,&E7,&E7,&E7,&7E,&3C,&18,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&C3,&C3,&DB,&DB,&FF,&FF,&E7,&C3,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&C3,&E7,&7E,&3C,&3C,&7E,&E7,&C3,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&E7,&E7,&E7,&E7,&E7,&FF,&FE,&E0,&FE,&7E,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&FF,&FF,&70,&38,&1C,&0E,&FF,&FF,&00,&00,&00,&00 ;  ; 122 z
 = &38,&3C,&0C,&0E,&0E,&07,&07,&0E,&0E,&0C,&3C,&38,&00,&00,&00,&00 ;  ; 123 {
 = &00,&38,&38,&38,&38,&00,&00,&38,&38,&38,&38,&00,&00,&00,&00,&00 ;  ; 124 |
 = &1C,&3C,&30,&70,&70,&E0,&E0,&70,&70,&30,&3C,&1C,&00,&00,&00,&00 ;  ; 125 }
 = &0E,&9F,&FF,&F9,&70,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 127 
; FONT: <BROADWAY>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7E,&81,&A5,&81,&81,&BD,&99,&81,&7E,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&6C,&FE,&FE,&FE,&FE,&7C,&38,&10,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&7C,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&18,&3C,&3C,&E7,&E7,&E7,&3C,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&E7,&C3,&81,&E7,&FF,&FF,&FF,&FF,&FF,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&3C,&66,&42,&42,&66,&3C,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&C3,&99,&BD,&BD,&99,&C3,&FF,&FF,&FF,&FF,&00,&00 ;  ; 10 
 = &00,&00,&F0,&E0,&B0,&98,&3C,&66,&66,&66,&3C,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&3C,&66,&66,&66,&3C,&18,&7E,&18,&18,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&FC,&CC,&FC,&0C,&0C,&0C,&0E,&0F,&07,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FE,&C6,&FE,&C6,&C6,&C6,&E6,&E7,&67,&06,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&E7,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0E,&3E,&FE,&3E,&0E,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&80,&C0,&E0,&F8,&FE,&F8,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&CC,&CC,&CC,&CC,&CC,&CC,&00,&CC,&CC,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&FE,&DB,&DB,&DB,&DE,&D8,&D8,&D8,&D8,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&7C,&C6,&0C,&38,&6C,&C6,&C6,&6C,&38,&60,&C6,&7C,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&00,&00,&FE,&FE,&FE,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&18,&3C,&7E,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&18,&18,&18,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&30,&60,&FF,&60,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&18,&0C,&FE,&0C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&06,&06,&06,&FE,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&00,&00,&10,&38,&38,&7C,&7C,&FE,&FE,&00,&00,&00,&00,&00,&00 ;  ; 30 
 = &00,&00,&00,&FE,&FE,&7C,&7C,&38,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&1C,&1C,&1C,&1C,&1C,&1C,&00,&1C,&1C,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&C6,&C6,&C6,&44,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&6C,&6C,&FE,&6C,&6C,&6C,&FE,&6C,&6C,&00,&00,&00,&00,&00 ;  ; 35 #
 = &10,&10,&7C,&86,&0E,&3C,&78,&E0,&C0,&C2,&7C,&10,&10,&00,&00,&00 ;  ; 36 $
 = &00,&00,&86,&C9,&E6,&70,&38,&1C,&0E,&67,&93,&61,&00,&00,&00,&00 ;  ; 37 %
 = &00,&00,&3C,&46,&4F,&3E,&9E,&59,&31,&71,&EE,&00,&00,&00,&00,&00 ;  ; 38 &
 = &00,&0C,&0C,&0C,&06,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&30,&18,&0C,&0C,&0C,&0C,&0C,&18,&30,&00,&00,&00,&00,&00 ;  ; 40 (
 = &00,&00,&18,&30,&60,&60,&60,&60,&60,&30,&18,&00,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&00,&00,&66,&3C,&FF,&3C,&66,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&18,&18,&18,&FF,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&18,&0C,&00,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&80,&C0,&60,&30,&18,&0C,&06,&03,&01,&00,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&3C,&46,&87,&87,&87,&87,&87,&46,&3C,&00,&00,&00,&00,&00 ;  ; 48 0
 = &00,&00,&30,&38,&3C,&38,&38,&38,&38,&38,&38,&00,&00,&00,&00,&00 ;  ; 49 1
 = &00,&00,&7E,&C1,&E0,&70,&38,&1C,&0E,&07,&FF,&00,&00,&00,&00,&00 ;  ; 50 2
 = &00,&00,&3E,&61,&E0,&60,&1C,&60,&E0,&61,&3E,&00,&00,&00,&00,&00 ;  ; 51 3
 = &00,&00,&60,&70,&78,&74,&72,&71,&FF,&70,&70,&00,&00,&00,&00,&00 ;  ; 52 4
 = &00,&00,&7E,&02,&02,&3E,&60,&E1,&E1,&62,&3C,&00,&00,&00,&00,&00 ;  ; 53 5
 = &00,&00,&7C,&86,&07,&3F,&47,&87,&87,&46,&3C,&00,&00,&00,&00,&00 ;  ; 54 6
 = &00,&00,&FF,&80,&C0,&E0,&70,&38,&1C,&0E,&0E,&00,&00,&00,&00,&00 ;  ; 55 7
 = &00,&00,&7C,&86,&8F,&5F,&7E,&FA,&F1,&E1,&7E,&00,&00,&00,&00,&00 ;  ; 56 8
 = &00,&00,&3C,&62,&E1,&E1,&E2,&FC,&E0,&61,&3E,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&18,&18,&00,&00,&18,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&18,&18,&00,&00,&00,&18,&18,&0C,&00,&00,&00,&00,&00 ;  ; 59 ;
 = &00,&00,&60,&30,&18,&0C,&06,&0C,&18,&30,&60,&00,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&00,&7E,&00,&00,&7E,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&00,&06,&0C,&18,&30,&60,&30,&18,&0C,&06,&00,&00,&00,&00,&00 ;  ; 62 >
 = &00,&00,&3E,&61,&E0,&E0,&60,&30,&38,&00,&38,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&00,&7C,&C6,&C6,&F6,&F6,&F6,&76,&06,&7C,&00,&00,&00,&00,&00 ;  ; 64 @
 = &00,&00,&18,&3C,&76,&E3,&E1,&FF,&E1,&E1,&E1,&00,&00,&00,&00,&00 ;  ; 65 A
 = &00,&00,&7E,&8E,&8E,&8E,&7E,&8E,&8E,&8E,&7E,&00,&00,&00,&00,&00 ;  ; 66 B
 = &00,&00,&7C,&86,&07,&07,&07,&07,&07,&86,&7C,&00,&00,&00,&00,&00 ;  ; 67 C
 = &00,&00,&3E,&4E,&8E,&8E,&8E,&8E,&8E,&4E,&3E,&00,&00,&00,&00,&00 ;  ; 68 D
 = &00,&00,&FE,&0E,&0E,&FE,&0E,&0E,&0E,&0E,&FE,&00,&00,&00,&00,&00 ;  ; 69 E
 = &00,&00,&FE,&0E,&0E,&FE,&0E,&0E,&0E,&0E,&0E,&00,&00,&00,&00,&00 ;  ; 70 F
 = &00,&00,&7C,&C6,&87,&07,&07,&F7,&87,&86,&7C,&00,&00,&00,&00,&00 ;  ; 71 G
 = &00,&00,&8E,&8E,&8E,&8E,&FE,&8E,&8E,&8E,&8E,&00,&00,&00,&00,&00 ;  ; 72 H
 = &00,&00,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&00,&00,&00,&00,&00 ;  ; 73 I
 = &00,&00,&E0,&E0,&E0,&E0,&E0,&E0,&E2,&E2,&7C,&00,&00,&00,&00,&00 ;  ; 74 J
 = &00,&00,&8E,&4E,&4E,&2E,&1E,&2E,&4E,&4E,&8E,&00,&00,&00,&00,&00 ;  ; 75 K
 = &00,&00,&0E,&0E,&0E,&0E,&0E,&0E,&0E,&0E,&FE,&00,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&C1,&E3,&F7,&E9,&E1,&E1,&E1,&E1,&E1,&00,&00,&00,&00,&00 ;  ; 77 M
 = &00,&00,&86,&8E,&9E,&BA,&F2,&E2,&C2,&82,&82,&00,&00,&00,&00,&00 ;  ; 78 N
 = &00,&00,&3C,&46,&87,&87,&87,&87,&87,&46,&3C,&00,&00,&00,&00,&00 ;  ; 79 O
 = &00,&00,&3E,&4E,&8E,&8E,&4E,&3E,&0E,&0E,&0E,&00,&00,&00,&00,&00 ;  ; 80 P
 = &00,&00,&3C,&46,&87,&87,&87,&87,&87,&56,&3C,&40,&80,&00,&00,&00 ;  ; 81 Q
 = &00,&00,&3E,&4E,&8E,&8E,&4E,&3E,&4E,&8E,&8E,&00,&00,&00,&00,&00 ;  ; 82 R
 = &00,&00,&7C,&86,&0E,&3C,&78,&E0,&C0,&C2,&7C,&00,&00,&00,&00,&00 ;  ; 83 S
 = &00,&00,&FE,&38,&38,&38,&38,&38,&38,&38,&38,&00,&00,&00,&00,&00 ;  ; 84 T
 = &00,&00,&8E,&8E,&8E,&8E,&8E,&8E,&8E,&8E,&7C,&00,&00,&00,&00,&00 ;  ; 85 U
 = &00,&00,&87,&87,&87,&87,&87,&87,&46,&24,&18,&00,&00,&00,&00,&00 ;  ; 86 V
 = &00,&00,&87,&87,&87,&87,&87,&97,&AF,&C7,&83,&00,&00,&00,&00,&00 ;  ; 87 W
 = &00,&00,&81,&43,&27,&1E,&1C,&38,&74,&E2,&C1,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&00,&81,&83,&47,&2E,&1C,&08,&04,&02,&01,&00,&00,&00,&00,&00 ;  ; 89 Y
 = &00,&00,&FF,&E0,&70,&38,&1C,&0E,&07,&03,&FF,&00,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&00,&3C,&0C,&0C,&0C,&0C,&0C,&0C,&0C,&3C,&00,&00,&00,&00,&00 ;  ; 91 [
 = &00,&00,&02,&06,&0E,&1C,&38,&70,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&3C,&30,&30,&30,&30,&30,&30,&30,&3C,&00,&00,&00,&00,&00 ;  ; 93 ]
 = &10,&38,&6C,&C6,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00 ;  ; 95 _
 = &18,&18,&30,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&3E,&61,&FE,&E7,&E7,&E7,&DE,&00,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&07,&07,&3F,&67,&E7,&E7,&E7,&67,&3B,&00,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&7C,&86,&07,&07,&07,&86,&7C,&00,&00,&00,&00,&00 ;  ; 99 c
 = &00,&00,&E0,&E0,&FC,&E6,&E7,&E7,&E7,&E6,&DC,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&3C,&66,&E7,&FF,&07,&86,&7C,&00,&00,&00,&00,&00 ;  ; 101 e
 = &00,&00,&78,&9C,&1C,&7E,&1C,&1C,&1C,&1C,&1C,&00,&00,&00,&00,&00 ;  ; 102 f
 = &00,&00,&40,&20,&7E,&C3,&C3,&C3,&7E,&01,&7F,&FE,&80,&00,&00,&00 ;  ; 103 g
 = &00,&00,&07,&07,&37,&EF,&E7,&E7,&E7,&E7,&E7,&00,&00,&00,&00,&00 ;  ; 104 h
 = &00,&00,&38,&00,&38,&38,&38,&38,&38,&38,&38,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&70,&00,&70,&70,&70,&70,&70,&70,&72,&3C,&00,&00,&00,&00 ;  ; 106 j
 = &00,&00,&07,&47,&27,&17,&1F,&3F,&77,&E7,&C7,&00,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&38,&38,&38,&38,&38,&38,&38,&38,&38,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&4E,&B7,&B7,&B7,&B7,&B7,&B7,&00,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&77,&EF,&E7,&E7,&E7,&E7,&E7,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&3C,&46,&87,&87,&87,&46,&3C,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&3B,&67,&E7,&E7,&E7,&67,&3F,&07,&07,&00,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&DC,&E6,&E7,&E7,&E7,&E6,&FC,&E0,&E0,&00,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&77,&8F,&07,&07,&07,&07,&07,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&7C,&8E,&3C,&78,&E0,&C2,&7C,&00,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&1C,&1C,&7F,&1C,&1C,&1C,&1C,&9C,&78,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&E7,&E7,&E7,&E7,&E7,&E7,&DE,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&87,&87,&87,&87,&46,&24,&18,&00,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&87,&87,&87,&97,&AF,&C7,&83,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&87,&4E,&3C,&38,&74,&E2,&C1,&00,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&87,&87,&87,&87,&86,&4C,&38,&10,&0C,&03,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&FE,&E0,&70,&38,&1C,&0E,&FE,&00,&00,&00,&00,&00 ;  ; 122 z
 = &00,&00,&70,&18,&18,&18,&0E,&18,&18,&18,&70,&00,&00,&00,&00,&00 ;  ; 123 {
 = &00,&00,&18,&18,&18,&18,&00,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&00,&0E,&18,&18,&18,&38,&18,&18,&18,&0E,&00,&00,&00,&00,&00 ;  ; 125 }
 = &00,&00,&DC,&76,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&00,&00,&10,&38,&6C,&C6,&C6,&C6,&FE,&00,&00,&00,&00 ;  ; 127 
; FONT: <COMPUTER>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7E,&81,&A5,&81,&81,&BD,&99,&81,&7E,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&6C,&FE,&FE,&FE,&FE,&7C,&38,&10,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&7C,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&18,&3C,&3C,&E7,&E7,&E7,&3C,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&E7,&C3,&81,&E7,&FF,&FF,&FF,&FF,&FF,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&3C,&66,&42,&42,&66,&3C,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&C3,&99,&BD,&BD,&99,&C3,&FF,&FF,&FF,&FF,&00,&00 ;  ; 10 
 = &00,&00,&F0,&E0,&B0,&98,&3C,&66,&66,&66,&3C,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&3C,&66,&66,&66,&3C,&18,&7E,&18,&18,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&FC,&CC,&FC,&0C,&0C,&0C,&0E,&0F,&07,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FE,&C6,&FE,&C6,&C6,&C6,&E6,&E7,&67,&06,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&E7,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0E,&3E,&FE,&3E,&0E,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&80,&C0,&E0,&F8,&FE,&F8,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&CC,&CC,&CC,&CC,&CC,&CC,&00,&CC,&CC,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&FE,&DB,&DB,&DB,&DE,&D8,&D8,&D8,&D8,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&7C,&C6,&0C,&38,&6C,&C6,&C6,&6C,&38,&60,&C6,&7C,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&00,&00,&FE,&FE,&FE,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&18,&3C,&7E,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&18,&18,&18,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&30,&60,&FF,&60,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&18,&0C,&FE,&0C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&06,&06,&06,&FE,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&00,&00,&10,&38,&38,&7C,&7C,&FE,&FE,&00,&00,&00,&00,&00,&00 ;  ; 30 
 = &00,&00,&00,&FE,&FE,&7C,&7C,&38,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&1C,&1C,&1C,&18,&10,&10,&10,&00,&00,&1C,&1C,&00,&00,&00,&00 ;  ; 33 !
 = &00,&E7,&E7,&66,&24,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&21,&21,&21,&7F,&21,&21,&63,&E7,&FF,&E7,&E7,&00,&00,&00,&00 ;  ; 35 #
 = &00,&08,&7F,&69,&69,&09,&7F,&6C,&6D,&6D,&7F,&0C,&00,&00,&00,&00 ;  ; 36 $
 = &00,&86,&8D,&CD,&66,&30,&18,&0C,&66,&D3,&D1,&61,&00,&00,&00,&00 ;  ; 37 %
 = &00,&0E,&11,&11,&11,&0A,&06,&89,&51,&21,&52,&8C,&00,&00,&00,&00 ;  ; 38 &
 = &00,&18,&18,&08,&04,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&30,&18,&0C,&06,&06,&06,&06,&06,&0C,&18,&30,&00,&00,&00,&00 ;  ; 40 (
 = &00,&0C,&18,&30,&60,&60,&60,&60,&60,&30,&18,&0C,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&00,&08,&2A,&1C,&7F,&1C,&2A,&08,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&08,&08,&08,&7F,&08,&08,&08,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&08,&04,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&7F,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&80,&80,&C0,&60,&30,&18,&0C,&06,&03,&01,&01,&00,&00,&00,&00 ;  ; 47 /
 = &00,&7E,&C3,&E1,&F1,&99,&9D,&9D,&81,&81,&C3,&7E,&00,&00,&00,&00 ;  ; 48 0
 = &00,&0E,&0E,&08,&08,&08,&08,&08,&7E,&7E,&7E,&7E,&00,&00,&00,&00 ;  ; 49 1
 = &00,&FF,&E0,&E0,&E0,&E0,&FF,&01,&01,&01,&01,&FF,&00,&00,&00,&00 ;  ; 50 2
 = &00,&3F,&20,&20,&20,&60,&FF,&E0,&E0,&E0,&E0,&FF,&00,&00,&00,&00 ;  ; 51 3
 = &00,&30,&08,&04,&02,&71,&71,&FF,&70,&70,&70,&70,&00,&00,&00,&00 ;  ; 52 4
 = &00,&FF,&01,&01,&01,&01,&FF,&E0,&E0,&E0,&E0,&FF,&00,&00,&00,&00 ;  ; 53 5
 = &00,&1F,&11,&11,&01,&01,&FF,&87,&87,&87,&87,&FF,&00,&00,&00,&00 ;  ; 54 6
 = &00,&FF,&83,&83,&80,&C0,&E0,&E0,&E0,&E0,&E0,&E0,&00,&00,&00,&00 ;  ; 55 7
 = &00,&3E,&22,&22,&22,&63,&FF,&E3,&E3,&E3,&E3,&FF,&00,&00,&00,&00 ;  ; 56 8
 = &00,&FE,&83,&81,&81,&C3,&FE,&E0,&E0,&E0,&E0,&E0,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&00,&38,&38,&38,&00,&00,&38,&38,&38,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&00,&38,&38,&38,&00,&00,&38,&38,&38,&10,&08,&00,&00 ;  ; 59 ;
 = &00,&60,&30,&18,&0C,&06,&03,&06,&0C,&18,&30,&60,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&7F,&00,&00,&00,&7F,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&06,&0C,&18,&30,&60,&C0,&60,&30,&18,&0C,&06,&00,&00,&00,&00 ;  ; 62 >
 = &00,&FF,&E3,&E3,&60,&10,&08,&08,&00,&00,&1C,&1C,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&7E,&C3,&81,&81,&E3,&97,&97,&F7,&07,&FF,&FF,&00,&00,&00,&00 ;  ; 64 @
 = &00,&7C,&C4,&84,&84,&86,&FF,&87,&87,&87,&87,&87,&00,&00,&00,&00 ;  ; 65 A
 = &00,&3F,&27,&27,&27,&27,&FF,&87,&87,&87,&87,&FF,&00,&00,&00,&00 ;  ; 66 B
 = &00,&FF,&E1,&E1,&E1,&01,&01,&01,&01,&81,&81,&FF,&00,&00,&00,&00 ;  ; 67 C
 = &00,&7F,&C7,&87,&87,&87,&87,&87,&87,&87,&87,&FF,&00,&00,&00,&00 ;  ; 68 D
 = &00,&FF,&01,&01,&01,&03,&3F,&07,&07,&07,&07,&FF,&00,&00,&00,&00 ;  ; 69 E
 = &00,&FF,&07,&07,&07,&07,&3E,&04,&04,&04,&04,&04,&00,&00,&00,&00 ;  ; 70 F
 = &00,&FF,&C7,&C7,&07,&07,&E7,&E7,&87,&87,&87,&FF,&00,&00,&00,&00 ;  ; 71 G
 = &00,&21,&21,&21,&21,&61,&FF,&E7,&E7,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 72 H
 = &00,&10,&10,&10,&10,&18,&1C,&1C,&1C,&1C,&1C,&1C,&00,&00,&00,&00 ;  ; 73 I
 = &00,&20,&20,&20,&20,&60,&E0,&E0,&E3,&E3,&E3,&FF,&00,&00,&00,&00 ;  ; 74 J
 = &00,&41,&21,&11,&09,&1F,&7F,&F7,&E7,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 75 K
 = &00,&04,&04,&04,&04,&06,&07,&07,&07,&07,&07,&FF,&00,&00,&00,&00 ;  ; 76 L
 = &00,&E3,&D5,&C9,&C1,&C1,&C1,&C1,&C1,&CD,&CD,&CD,&00,&00,&00,&00 ;  ; 77 M
 = &00,&87,&89,&91,&A1,&C1,&81,&81,&81,&99,&99,&99,&00,&00,&00,&00 ;  ; 78 N
 = &00,&7E,&C3,&81,&81,&C1,&E1,&E1,&E1,&E1,&F3,&7E,&00,&00,&00,&00 ;  ; 79 O
 = &00,&7F,&C1,&81,&81,&83,&FF,&07,&07,&07,&07,&07,&00,&00,&00,&00 ;  ; 80 P
 = &00,&7F,&C1,&81,&81,&83,&83,&83,&83,&83,&93,&FF,&C0,&00,&00,&00 ;  ; 81 Q
 = &00,&3F,&61,&41,&41,&C3,&FF,&C7,&C7,&C7,&C7,&C7,&00,&00,&00,&00 ;  ; 82 R
 = &00,&FF,&E1,&E1,&01,&01,&FF,&E0,&E0,&E1,&E1,&FF,&00,&00,&00,&00 ;  ; 83 S
 = &00,&7F,&04,&04,&04,&0C,&1C,&1C,&1C,&1C,&1C,&1C,&00,&00,&00,&00 ;  ; 84 T
 = &00,&81,&81,&81,&81,&C1,&E1,&E1,&E1,&E1,&E3,&FE,&00,&00,&00,&00 ;  ; 85 U
 = &00,&81,&81,&81,&81,&C1,&E1,&E1,&E1,&62,&24,&18,&00,&00,&00,&00 ;  ; 86 V
 = &00,&B3,&B3,&B3,&83,&83,&83,&83,&83,&93,&AB,&C7,&00,&00,&00,&00 ;  ; 87 W
 = &00,&81,&81,&81,&42,&3C,&18,&3C,&66,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 88 X
 = &00,&41,&41,&41,&22,&14,&1C,&1C,&1C,&1C,&1C,&1C,&00,&00,&00,&00 ;  ; 89 Y
 = &00,&FF,&41,&21,&10,&08,&04,&02,&E1,&E1,&E1,&FF,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&FE,&0E,&0E,&0E,&0E,&0E,&0E,&0E,&0E,&0E,&FE,&00,&00,&00,&00 ;  ; 91 [
 = &00,&80,&9C,&9D,&9D,&81,&81,&01,&01,&1D,&1D,&1C,&00,&00,&00,&00 ;  ; 92 \
 = &00,&FE,&E0,&E0,&E0,&E0,&E0,&E0,&E0,&E0,&E0,&FE,&00,&00,&00,&00 ;  ; 93 ]
 = &00,&18,&3C,&66,&C3,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&FF,&00,&00,&00,&00 ;  ; 95 _
 = &00,&70,&70,&40,&20,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&7C,&C4,&84,&84,&86,&FF,&87,&87,&87,&87,&87,&00,&00,&00,&00 ;  ; 97 a
 = &00,&3F,&27,&27,&27,&27,&FF,&87,&87,&87,&87,&FF,&00,&00,&00,&00 ;  ; 98 b
 = &00,&FF,&E1,&E1,&E1,&01,&01,&01,&01,&81,&81,&FF,&00,&00,&00,&00 ;  ; 99 c
 = &00,&7F,&C7,&87,&87,&87,&87,&87,&87,&87,&87,&FF,&00,&00,&00,&00 ;  ; 100 d
 = &00,&FF,&01,&01,&01,&03,&3F,&07,&07,&07,&07,&FF,&00,&00,&00,&00 ;  ; 101 e
 = &00,&FF,&07,&07,&07,&07,&3E,&04,&04,&04,&04,&04,&00,&00,&00,&00 ;  ; 102 f
 = &00,&FF,&C7,&C7,&07,&07,&E7,&E7,&87,&87,&87,&FF,&00,&00,&00,&00 ;  ; 103 g
 = &00,&21,&21,&21,&21,&61,&FF,&E7,&E7,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 104 h
 = &00,&10,&10,&10,&10,&18,&1C,&1C,&1C,&1C,&1C,&1C,&00,&00,&00,&00 ;  ; 105 i
 = &00,&20,&20,&20,&20,&60,&E0,&E0,&E3,&E3,&E3,&FF,&00,&00,&00,&00 ;  ; 106 j
 = &00,&41,&21,&11,&09,&1F,&7F,&F7,&E7,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 107 k
 = &00,&04,&04,&04,&04,&06,&07,&07,&07,&07,&07,&FF,&00,&00,&00,&00 ;  ; 108 l
 = &00,&E3,&D5,&C9,&C1,&C1,&C1,&C1,&C1,&CD,&CD,&CD,&00,&00,&00,&00 ;  ; 109 m
 = &00,&87,&89,&91,&A1,&C1,&81,&81,&81,&99,&99,&99,&00,&00,&00,&00 ;  ; 110 n
 = &00,&7E,&C3,&81,&81,&C1,&E1,&E1,&E1,&E1,&F3,&7E,&00,&00,&00,&00 ;  ; 111 o
 = &00,&7F,&C1,&81,&81,&83,&FF,&07,&07,&07,&07,&07,&00,&00,&00,&00 ;  ; 112 p
 = &00,&7F,&C1,&81,&81,&83,&83,&83,&83,&83,&93,&FF,&C0,&00,&00,&00 ;  ; 113 q
 = &00,&3F,&61,&41,&41,&C3,&FF,&C7,&C7,&C7,&C7,&C7,&00,&00,&00,&00 ;  ; 114 r
 = &00,&FF,&E1,&E1,&01,&01,&FF,&E0,&E0,&E1,&E1,&FF,&00,&00,&00,&00 ;  ; 115 s
 = &00,&7F,&04,&04,&04,&0C,&1C,&1C,&1C,&1C,&1C,&1C,&00,&00,&00,&00 ;  ; 116 t
 = &00,&81,&81,&81,&81,&C1,&E1,&E1,&E1,&E1,&E3,&FE,&00,&00,&00,&00 ;  ; 117 u
 = &00,&81,&81,&81,&81,&C1,&E1,&E1,&E1,&62,&24,&18,&00,&00,&00,&00 ;  ; 118 v
 = &00,&B3,&B3,&B3,&83,&83,&83,&83,&83,&93,&AB,&C7,&00,&00,&00,&00 ;  ; 119 w
 = &00,&81,&81,&81,&42,&3C,&18,&3C,&66,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 120 x
 = &00,&41,&41,&41,&22,&14,&1C,&1C,&1C,&1C,&1C,&1C,&00,&00,&00,&00 ;  ; 121 y
 = &00,&FF,&41,&21,&10,&08,&04,&02,&E1,&E1,&E1,&FF,&00,&00,&00,&00 ;  ; 122 z
 = &00,&38,&0C,&0C,&06,&06,&07,&06,&06,&0C,&0C,&38,&00,&00,&00,&00 ;  ; 123 {
 = &00,&00,&08,&08,&08,&E8,&E0,&E0,&E8,&08,&08,&08,&00,&00,&00,&00 ;  ; 124 |
 = &00,&1C,&30,&30,&70,&60,&E0,&60,&70,&30,&30,&1C,&00,&00,&00,&00 ;  ; 125 }
 = &00,&00,&00,&0E,&99,&70,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&33,&33,&CC,&CC,&33,&33,&CC,&CC,&33,&33,&CC,&CC,&00,&00,&00 ;  ; 127 
; FONT: <EVDIGIT>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7E,&81,&A5,&81,&81,&BD,&99,&81,&7E,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&6C,&FE,&FE,&FE,&FE,&7C,&38,&10,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&7C,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&18,&3C,&3C,&E7,&E7,&E7,&3C,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&E7,&C3,&81,&E7,&FF,&FF,&FF,&FF,&FF,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&3C,&66,&42,&42,&66,&3C,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&C3,&99,&BD,&BD,&99,&C3,&FF,&FF,&FF,&FF,&00,&00 ;  ; 10 
 = &00,&00,&60,&7C,&D0,&08,&3E,&41,&41,&41,&3E,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&3E,&41,&41,&41,&3E,&08,&7F,&08,&08,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&F8,&D8,&F8,&18,&18,&18,&1E,&1F,&0F,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FC,&CC,&FC,&CC,&CC,&CC,&CC,&EE,&6F,&06,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&E7,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0E,&3E,&FE,&3E,&0E,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&40,&60,&70,&7C,&7F,&7C,&70,&60,&40,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&66,&66,&66,&66,&66,&66,&00,&66,&66,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&FC,&DB,&DB,&DB,&DE,&D8,&D8,&D8,&D8,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&7C,&C6,&0C,&18,&6C,&C6,&C6,&6C,&30,&C0,&C6,&3C,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&00,&00,&FE,&FE,&FE,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&18,&18,&18,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&30,&60,&FF,&60,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&0C,&06,&FF,&06,&0C,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&06,&06,&06,&FE,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&00,&00,&08,&1C,&1C,&3E,&3E,&7F,&7F,&00,&00,&00,&00,&00,&00 ;  ; 30 
 = &00,&00,&00,&7F,&7F,&3E,&3E,&1C,&1C,&08,&00,&00,&00,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&3C,&3C,&3C,&38,&30,&20,&00,&38,&38,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&00,&E7,&E7,&66,&24,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&21,&21,&FF,&21,&63,&63,&FF,&63,&63,&00,&00,&00,&00,&00 ;  ; 35 #
 = &00,&08,&FF,&89,&09,&09,&FF,&C8,&C8,&C9,&FF,&08,&00,&00,&00,&00 ;  ; 36 $
 = &00,&9F,&99,&D9,&7F,&30,&18,&0C,&FE,&CB,&C9,&F9,&00,&00,&00,&00 ;  ; 37 %
 = &00,&7F,&41,&41,&21,&1E,&6F,&6B,&51,&E1,&43,&7F,&00,&00,&00,&00 ;  ; 38 &
 = &00,&00,&38,&38,&18,&08,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&10,&08,&04,&06,&06,&06,&06,&06,&04,&08,&10,&00,&00,&00 ;  ; 40 (
 = &00,&00,&08,&10,&20,&60,&60,&60,&60,&60,&20,&10,&08,&00,&00,&00 ;  ; 41 )
 = &00,&08,&6B,&1C,&7F,&1C,&6B,&08,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&10,&10,&10,&FE,&10,&10,&10,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&08,&04,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&FE,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&80,&80,&C0,&60,&30,&18,&0C,&06,&03,&01,&01,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&FF,&81,&81,&81,&C1,&C1,&C1,&C1,&FF,&00,&00,&00,&00,&00 ;  ; 48 0
 = &00,&00,&1C,&1C,&10,&10,&10,&10,&10,&7C,&7C,&00,&00,&00,&00,&00 ;  ; 49 1
 = &00,&00,&FF,&80,&80,&80,&FF,&01,&01,&01,&FF,&00,&00,&00,&00,&00 ;  ; 50 2
 = &00,&00,&7E,&40,&40,&40,&FE,&C0,&C0,&C0,&FE,&00,&00,&00,&00,&00 ;  ; 51 3
 = &00,&00,&03,&03,&03,&03,&C3,&FF,&C0,&C0,&C0,&00,&00,&00,&00,&00 ;  ; 52 4
 = &00,&00,&FF,&01,&01,&FF,&80,&80,&80,&80,&FF,&00,&00,&00,&00,&00 ;  ; 53 5
 = &00,&00,&7F,&41,&01,&01,&FF,&81,&81,&81,&FF,&00,&00,&00,&00,&00 ;  ; 54 6
 = &00,&00,&FF,&81,&41,&20,&10,&18,&0C,&0C,&06,&00,&00,&00,&00,&00 ;  ; 55 7
 = &00,&00,&7E,&42,&42,&42,&FF,&C3,&C3,&C3,&FF,&00,&00,&00,&00,&00 ;  ; 56 8
 = &00,&00,&FF,&81,&81,&81,&FF,&C0,&C0,&C0,&C0,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&00,&1C,&1C,&00,&00,&00,&1C,&1C,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&00,&1C,&1C,&00,&00,&00,&1C,&1C,&08,&04,&00,&00,&00 ;  ; 59 ;
 = &00,&60,&30,&18,&0C,&06,&03,&06,&0C,&18,&30,&60,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&00,&7F,&00,&7F,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&06,&0C,&18,&30,&60,&C0,&60,&30,&18,&0C,&06,&00,&00,&00,&00 ;  ; 62 >
 = &00,&00,&FF,&C1,&C0,&30,&08,&08,&00,&1C,&1C,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&00,&7E,&C3,&81,&F1,&93,&F7,&07,&07,&FF,&FF,&00,&00,&00,&00 ;  ; 64 @
 = &00,&00,&7E,&42,&42,&42,&FF,&C3,&C3,&C3,&C3,&00,&00,&00,&00,&00 ;  ; 65 A
 = &00,&00,&7F,&41,&41,&41,&FF,&83,&83,&83,&FF,&00,&00,&00,&00,&00 ;  ; 66 B
 = &00,&00,&FF,&C1,&C1,&01,&01,&01,&81,&81,&FF,&00,&00,&00,&00,&00 ;  ; 67 C
 = &00,&00,&FF,&81,&81,&81,&83,&83,&83,&83,&FF,&00,&00,&00,&00,&00 ;  ; 68 D
 = &00,&00,&FF,&01,&01,&01,&FF,&03,&03,&03,&FF,&00,&00,&00,&00,&00 ;  ; 69 E
 = &00,&00,&FF,&01,&01,&01,&FF,&03,&03,&03,&03,&00,&00,&00,&00,&00 ;  ; 70 F
 = &00,&00,&FF,&81,&01,&01,&E3,&83,&83,&83,&FF,&00,&00,&00,&00,&00 ;  ; 71 G
 = &00,&00,&42,&42,&42,&42,&7E,&C3,&C3,&C3,&C3,&00,&00,&00,&00,&00 ;  ; 72 H
 = &00,&00,&08,&08,&08,&08,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 73 I
 = &00,&00,&80,&80,&80,&80,&C0,&C0,&C1,&C1,&FF,&00,&00,&00,&00,&00 ;  ; 74 J
 = &00,&00,&41,&41,&41,&43,&3F,&43,&43,&43,&43,&00,&00,&00,&00,&00 ;  ; 75 K
 = &00,&00,&04,&04,&04,&06,&06,&06,&06,&06,&FE,&00,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&77,&49,&49,&49,&4B,&4B,&4B,&4B,&4B,&00,&00,&00,&00,&00 ;  ; 77 M
 = &00,&00,&43,&43,&45,&45,&4B,&4B,&53,&63,&63,&00,&00,&00,&00,&00 ;  ; 78 N
 = &00,&00,&FF,&81,&81,&81,&C1,&C1,&C1,&C1,&FF,&00,&00,&00,&00,&00 ;  ; 79 O
 = &00,&00,&FF,&81,&81,&81,&FF,&03,&03,&03,&03,&00,&00,&00,&00,&00 ;  ; 80 P
 = &00,&00,&FF,&81,&81,&81,&81,&81,&81,&E1,&FF,&60,&00,&00,&00,&00 ;  ; 81 Q
 = &00,&00,&7F,&41,&41,&41,&FF,&83,&83,&83,&83,&00,&00,&00,&00,&00 ;  ; 82 R
 = &00,&00,&FF,&81,&01,&01,&FF,&C0,&C0,&C1,&FF,&00,&00,&00,&00,&00 ;  ; 83 S
 = &00,&00,&FF,&F8,&08,&08,&08,&08,&08,&08,&08,&00,&00,&00,&00,&00 ;  ; 84 T
 = &00,&00,&81,&81,&81,&81,&83,&83,&83,&83,&FF,&00,&00,&00,&00,&00 ;  ; 85 U
 = &00,&00,&81,&81,&81,&81,&42,&42,&24,&24,&18,&00,&00,&00,&00,&00 ;  ; 86 V
 = &00,&00,&49,&49,&49,&49,&49,&4B,&4B,&4B,&77,&00,&00,&00,&00,&00 ;  ; 87 W
 = &00,&00,&C1,&C1,&42,&24,&18,&24,&42,&83,&83,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&00,&81,&81,&81,&FF,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 89 Y
 = &00,&00,&FF,&41,&20,&18,&0C,&06,&03,&83,&FF,&00,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&00,&7E,&06,&06,&06,&06,&06,&06,&06,&7E,&00,&00,&00,&00,&00 ;  ; 91 [
 = &00,&00,&5C,&5C,&5C,&41,&41,&01,&1D,&1D,&1C,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&7E,&60,&60,&60,&60,&60,&60,&60,&7E,&00,&00,&00,&00,&00 ;  ; 93 ]
 = &00,&00,&18,&3C,&66,&C3,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&FF,&00 ;  ; 95 _
 = &00,&00,&1C,&1C,&10,&08,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&7F,&40,&7F,&43,&43,&FF,&00,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&02,&02,&02,&FE,&86,&86,&86,&86,&FE,&00,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&FE,&C2,&02,&02,&82,&FE,&00,&00,&00,&00,&00 ;  ; 99 c
 = &00,&00,&80,&80,&80,&FE,&86,&86,&86,&86,&FE,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&FE,&86,&FE,&06,&86,&FE,&00,&00,&00,&00,&00 ;  ; 101 e
 = &00,&00,&F8,&98,&18,&7E,&18,&18,&18,&08,&08,&00,&00,&00,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&00,&FF,&41,&41,&7F,&02,&7F,&43,&7F,&00,&00,&00 ;  ; 103 g
 = &00,&00,&02,&02,&02,&FE,&86,&86,&86,&86,&86,&00,&00,&00,&00,&00 ;  ; 104 h
 = &00,&00,&10,&10,&00,&10,&10,&30,&30,&30,&30,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&10,&10,&00,&10,&10,&18,&18,&18,&18,&1A,&1E,&00,&00,&00 ;  ; 106 j
 = &00,&00,&02,&02,&02,&42,&46,&3E,&46,&46,&46,&00,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&08,&08,&08,&08,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&EE,&92,&96,&96,&96,&96,&00,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&FC,&84,&8C,&8C,&8C,&8C,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&FE,&82,&C2,&C2,&C2,&FE,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&FE,&82,&82,&82,&82,&FE,&06,&06,&00,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&00,&FE,&82,&82,&82,&82,&FE,&C0,&C0,&00,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&00,&7E,&42,&06,&06,&06,&06,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&FE,&02,&FE,&C0,&C2,&FE,&00,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&00,&10,&10,&7C,&70,&10,&10,&10,&70,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&82,&82,&86,&86,&86,&FE,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&00,&81,&81,&42,&24,&24,&18,&00,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&92,&92,&96,&96,&96,&EE,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&00,&C2,&64,&18,&18,&26,&43,&00,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&00,&81,&81,&81,&81,&81,&FF,&C0,&C0,&C0,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&00,&7E,&22,&10,&0C,&46,&7E,&00,&00,&00,&00,&00 ;  ; 122 z
 = &00,&00,&70,&18,&0C,&0C,&0E,&0C,&0C,&18,&70,&00,&00,&00,&00,&00 ;  ; 123 {
 = &00,&00,&00,&04,&04,&74,&70,&74,&04,&04,&04,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&00,&0E,&18,&30,&30,&70,&30,&30,&18,&0E,&00,&00,&00,&00,&00 ;  ; 125 }
 = &00,&00,&8E,&71,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&33,&33,&CC,&CC,&33,&33,&CC,&CC,&33,&33,&CC,&CC,&00,&00 ;  ; 127 
; FONT: <EVOLDENG>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 10 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 30 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&18,&1E,&0C,&0C,&1C,&04,&00,&1C,&0E,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&00,&EE,&66,&66,&44,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&6C,&6C,&FF,&36,&36,&36,&7F,&33,&33,&00,&00,&00,&00,&00 ;  ; 35 #
 = &00,&10,&7C,&D6,&5C,&3A,&76,&DC,&D0,&DE,&7B,&10,&00,&00,&00,&00 ;  ; 36 $
 = &00,&00,&FC,&93,&4E,&20,&10,&08,&64,&9A,&71,&00,&00,&00,&00,&00 ;  ; 37 %
 = &00,&00,&38,&C6,&66,&9C,&1C,&92,&A6,&4F,&BC,&00,&00,&00,&00,&00 ;  ; 38 &
 = &00,&00,&0C,&1C,&10,&08,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&C0,&30,&18,&0E,&0C,&0C,&0E,&18,&30,&C0,&00,&00,&00,&00 ;  ; 40 (
 = &00,&00,&06,&18,&30,&E0,&60,&60,&E0,&30,&18,&06,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&18,&DB,&2A,&1C,&2A,&DB,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&00,&18,&18,&18,&FF,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&38,&20,&10,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&00,&7F,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&38,&1C,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&C0,&60,&30,&30,&18,&0C,&0C,&06,&03,&00,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&18,&FF,&76,&66,&66,&66,&6E,&FF,&18,&00,&00,&00,&00,&00 ;  ; 48 0
 = &00,&00,&00,&18,&3C,&30,&30,&30,&30,&FC,&30,&00,&00,&00,&00,&00 ;  ; 49 1
 = &00,&00,&04,&3E,&77,&71,&38,&0C,&82,&FD,&73,&00,&00,&00,&00,&00 ;  ; 50 2
 = &00,&00,&0C,&FE,&63,&E0,&38,&E0,&E3,&F6,&0E,&00,&00,&00,&00,&00 ;  ; 51 3
 = &00,&00,&30,&F8,&74,&32,&33,&7F,&30,&7C,&30,&00,&00,&00,&00,&00 ;  ; 52 4
 = &00,&40,&7E,&3E,&02,&3E,&F9,&60,&63,&EE,&1C,&00,&00,&00,&00,&00 ;  ; 53 5
 = &00,&00,&18,&FF,&76,&06,&F6,&6E,&66,&E7,&18,&00,&00,&00,&00,&00 ;  ; 54 6
 = &00,&00,&8C,&FE,&41,&20,&18,&0C,&0C,&3C,&0C,&00,&00,&00,&00,&00 ;  ; 55 7
 = &00,&00,&18,&E7,&66,&EF,&3C,&E7,&66,&E7,&18,&00,&00,&00,&00,&00 ;  ; 56 8
 = &00,&00,&18,&E7,&66,&6F,&7C,&60,&66,&FF,&18,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&00,&00,&18,&18,&00,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&00,&00,&18,&18,&00,&00,&18,&18,&10,&08,&00,&00,&00 ;  ; 59 ;
 = &00,&00,&00,&C0,&30,&0C,&07,&0C,&30,&C0,&00,&00,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&00,&7F,&00,&7F,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&00,&00,&03,&0C,&30,&E0,&30,&0C,&03,&00,&00,&00,&00,&00,&00 ;  ; 62 >
 = &00,&00,&7C,&C2,&C6,&60,&10,&08,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&00,&3C,&42,&81,&B9,&A5,&79,&01,&06,&38,&00,&00,&00,&00,&00 ;  ; 64 @
 = &00,&00,&1E,&3B,&38,&7E,&6B,&7C,&64,&E6,&6D,&00,&00,&00,&00,&00 ;  ; 65 A
 = &00,&00,&16,&78,&DB,&DA,&3B,&7A,&E5,&EC,&76,&00,&00,&00,&00,&00 ;  ; 66 B
 = &00,&00,&64,&FA,&AF,&2B,&2B,&2B,&A7,&46,&38,&00,&00,&00,&00,&00 ;  ; 67 C
 = &00,&03,&7E,&F8,&CE,&FF,&CE,&FB,&C8,&DE,&73,&00,&00,&00,&00,&00 ;  ; 68 D
 = &00,&00,&E7,&7C,&14,&F6,&7F,&16,&11,&7C,&E7,&00,&00,&00,&00,&00 ;  ; 69 E
 = &00,&00,&E7,&F8,&2C,&EE,&37,&36,&34,&1F,&0C,&00,&00,&00,&00,&00 ;  ; 70 F
 = &00,&00,&64,&F2,&5B,&2B,&5B,&DB,&CB,&C6,&38,&00,&00,&00,&00,&00 ;  ; 71 G
 = &00,&00,&67,&D8,&0D,&4D,&FD,&6D,&6D,&60,&E7,&2C,&40,&60,&00,&00 ;  ; 72 H
 = &00,&00,&EE,&58,&58,&58,&5E,&58,&58,&58,&E6,&00,&00,&00,&00,&00 ;  ; 73 I
 = &00,&00,&96,&6D,&60,&6C,&6E,&6C,&60,&67,&1E,&00,&00,&00,&00,&00 ;  ; 74 J
 = &00,&00,&EF,&E0,&2A,&1B,&3B,&6A,&61,&6E,&CF,&00,&00,&00,&00,&00 ;  ; 75 K
 = &00,&00,&67,&D0,&1A,&1B,&1B,&0B,&01,&F8,&77,&00,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&B4,&FE,&B7,&B7,&B7,&B4,&B4,&B6,&FB,&00,&00,&00,&00,&00 ;  ; 77 M
 = &00,&00,&E6,&4D,&4C,&5C,&54,&74,&64,&64,&CE,&00,&00,&00,&00,&00 ;  ; 78 N
 = &00,&00,&54,&F6,&DF,&D3,&D3,&D3,&D3,&52,&3C,&00,&00,&00,&00,&00 ;  ; 79 O
 = &00,&00,&27,&FE,&66,&67,&66,&66,&FE,&27,&06,&06,&0F,&00,&00,&00 ;  ; 80 P
 = &00,&00,&54,&F2,&DF,&D3,&D3,&D3,&D7,&56,&38,&E0,&00,&00,&00,&00 ;  ; 81 Q
 = &00,&00,&4F,&F4,&DA,&6B,&1F,&35,&64,&74,&CF,&00,&00,&00,&00,&00 ;  ; 82 R
 = &00,&00,&5C,&63,&4E,&39,&76,&DC,&C8,&DE,&33,&00,&00,&00,&00,&00 ;  ; 83 S
 = &00,&00,&DE,&7B,&5A,&59,&59,&5B,&4B,&C6,&3C,&00,&00,&00,&00,&00 ;  ; 84 T
 = &00,&00,&63,&E6,&76,&66,&77,&66,&66,&FF,&CE,&00,&00,&00,&00,&00 ;  ; 85 U
 = &00,&00,&77,&EE,&66,&66,&67,&66,&66,&EF,&38,&00,&00,&00,&00,&00 ;  ; 86 V
 = &00,&00,&DB,&7E,&5A,&5A,&5B,&5A,&5A,&5A,&B7,&00,&00,&00,&00,&00 ;  ; 87 W
 = &00,&00,&C6,&6D,&28,&18,&FE,&18,&34,&62,&C3,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&00,&E3,&6E,&66,&66,&67,&66,&66,&3C,&0C,&78,&80,&00,&00,&00 ;  ; 89 Y
 = &00,&00,&FE,&46,&21,&7E,&18,&3F,&86,&FA,&6F,&00,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&00,&FC,&18,&18,&0C,&0C,&0C,&0C,&18,&18,&FC,&00,&00,&00,&00 ;  ; 91 [
 = &00,&00,&03,&06,&0C,&0C,&18,&30,&30,&60,&C0,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&7E,&30,&30,&60,&60,&60,&60,&30,&30,&7E,&00,&00,&00,&00 ;  ; 93 ]
 = &00,&00,&18,&3C,&66,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&FF,&00,&00 ;  ; 95 _
 = &00,&00,&18,&1C,&0C,&08,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&00,&3C,&66,&7C,&EB,&DE,&00,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&0C,&07,&06,&06,&3E,&F6,&66,&E7,&1C,&00,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&00,&3C,&E7,&06,&67,&3C,&00,&00,&00,&00,&00 ;  ; 99 c
 = &00,&00,&07,&0C,&18,&30,&FC,&67,&66,&67,&DC,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&00,&3C,&77,&1E,&47,&3E,&00,&00,&00,&00,&00 ;  ; 101 e
 = &00,&00,&B8,&6E,&0C,&0C,&3E,&0C,&0C,&3E,&18,&00,&00,&00,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&00,&00,&3C,&E7,&66,&27,&3C,&60,&EF,&3C,&00,&00 ;  ; 103 g
 = &00,&00,&0C,&07,&06,&06,&36,&EE,&66,&67,&EE,&10,&30,&08,&00,&00 ;  ; 104 h
 = &00,&00,&00,&38,&1C,&00,&38,&1C,&18,&78,&30,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&00,&70,&38,&00,&30,&78,&30,&30,&30,&70,&70,&38,&0C,&00 ;  ; 106 j
 = &00,&00,&0C,&07,&26,&76,&CE,&7E,&36,&EF,&4C,&00,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&30,&3C,&18,&18,&18,&18,&18,&7C,&38,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&00,&6B,&FF,&DB,&DB,&DB,&00,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&00,&73,&EF,&66,&EE,&67,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&00,&38,&E7,&66,&E7,&1C,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&00,&76,&EF,&66,&EE,&7F,&06,&0E,&03,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&00,&00,&38,&E7,&66,&67,&7C,&60,&E0,&70,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&00,&00,&36,&FF,&46,&0E,&07,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&00,&7C,&2E,&3C,&72,&3F,&00,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&00,&30,&18,&7E,&18,&18,&18,&3C,&70,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&00,&76,&E7,&66,&FF,&6C,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&00,&00,&66,&F7,&66,&FE,&18,&00,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&00,&DB,&5A,&5A,&FF,&36,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&00,&00,&C6,&6F,&18,&F6,&63,&00,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&00,&00,&66,&E7,&66,&7F,&6C,&C0,&7E,&1D,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&00,&00,&6E,&1B,&1E,&66,&3B,&00,&00,&00,&00,&00 ;  ; 122 z
 = &00,&00,&F0,&2C,&2C,&2C,&27,&26,&2C,&2C,&2C,&F0,&00,&00,&00,&00 ;  ; 123 {
 = &00,&00,&18,&18,&18,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&00,&0F,&34,&34,&34,&E4,&64,&34,&34,&34,&1F,&00,&00,&00,&00 ;  ; 125 }
 = &00,&00,&00,&00,&8E,&71,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&33,&33,&CC,&CC,&33,&33,&CC,&CC,&33,&33,&CC,&CC,&00,&00 ;  ; 127 
; FONT: <FUTURE>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7E,&81,&A5,&81,&99,&81,&BD,&99,&81,&7E,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&E7,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&EE,&FF,&FF,&FF,&FF,&FE,&7C,&38,&10,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&FF,&FE,&7C,&38,&10,&00,&00,&00,&00 ;  ; 4 
 = &00,&00,&00,&18,&3C,&3C,&E7,&E7,&E7,&18,&18,&3C,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&FF,&E7,&C3,&C3,&E7,&FF,&FF,&FF,&FF,&FF,&FF ;  ; 8 
 = &00,&00,&00,&00,&00,&3C,&66,&C3,&C3,&66,&3C,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&FF,&C3,&99,&3C,&3C,&99,&C3,&FF,&FF,&FF,&FF,&FF ;  ; 10 
 = &00,&00,&F8,&E0,&B0,&18,&7E,&C3,&C3,&C3,&C3,&7E,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&7E,&C3,&C3,&C3,&C3,&7E,&18,&7E,&18,&18,&00,&00,&00,&00 ;  ; 12 
 = &00,&FC,&8C,&8C,&FC,&0C,&0C,&0C,&0E,&0F,&0F,&07,&00,&00,&00,&00 ;  ; 13 
 = &00,&FE,&C6,&C6,&FE,&C6,&C6,&C6,&E6,&E7,&67,&03,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&00,&18,&18,&DB,&3C,&E7,&3C,&DB,&18,&18,&00,&00,&00,&00 ;  ; 15 
 = &00,&01,&03,&07,&0F,&3F,&FF,&3F,&0F,&07,&03,&01,&00,&00,&00,&00 ;  ; 16 
 = &00,&80,&C0,&E0,&F0,&FC,&FF,&FC,&F0,&E0,&C0,&80,&00,&00,&00,&00 ;  ; 17 
 = &00,&18,&3C,&7E,&DB,&18,&18,&18,&DB,&7E,&3C,&18,&00,&00,&00,&00 ;  ; 18 
 = &00,&C6,&C6,&C6,&C6,&C6,&C6,&C6,&C6,&00,&C6,&C6,&00,&00,&00,&00 ;  ; 19 
 = &00,&FE,&DB,&DB,&DB,&DB,&DE,&D8,&D8,&D8,&D8,&D8,&00,&00,&00,&00 ;  ; 20 
 = &00,&7E,&C3,&06,&3C,&66,&C3,&66,&3C,&60,&C3,&7E,&00,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&00,&00,&FF,&FF,&FF,&FF,&00,&00,&00,&00 ;  ; 22 
 = &00,&18,&3C,&7E,&DB,&18,&18,&18,&DB,&7E,&3C,&FF,&00,&00,&00,&00 ;  ; 23 
 = &00,&18,&3C,&7E,&DB,&18,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00 ;  ; 24 
 = &00,&18,&18,&18,&18,&18,&18,&18,&DB,&7E,&3C,&18,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&10,&30,&60,&FF,&FF,&60,&30,&10,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&08,&0C,&06,&FF,&FF,&06,&0C,&08,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&03,&03,&03,&FF,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&44,&C6,&FF,&FF,&C6,&44,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&00,&10,&38,&38,&7C,&7C,&FE,&FE,&FF,&FF,&00,&00,&00,&00,&00 ;  ; 30 
 = &00,&00,&FF,&FF,&FE,&FE,&7C,&7C,&38,&38,&10,&00,&00,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&00,&1C,&1C,&00,&00,&00,&00 ;  ; 33 !
 = &00,&E7,&E7,&E7,&66,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&66,&66,&FF,&FF,&66,&66,&FF,&FF,&66,&66,&00,&00,&00,&00,&00 ;  ; 35 #
 = &00,&1C,&1C,&7F,&43,&03,&3E,&60,&61,&7F,&1C,&1C,&00,&00,&00,&00 ;  ; 36 $
 = &00,&0E,&DB,&DB,&6E,&30,&18,&0C,&76,&DB,&DB,&70,&00,&00,&00,&00 ;  ; 37 %
 = &00,&3C,&66,&66,&66,&3C,&DE,&73,&63,&63,&73,&DE,&00,&00,&00,&00 ;  ; 38 &
 = &00,&38,&38,&38,&18,&0C,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&38,&1C,&0E,&07,&07,&07,&07,&07,&0E,&1C,&38,&00,&00,&00,&00 ;  ; 40 (
 = &00,&1C,&38,&70,&E0,&E0,&E0,&E0,&E0,&70,&38,&1C,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&5D,&7F,&3E,&7F,&7F,&3E,&7F,&5D,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&1C,&1C,&7F,&7F,&1C,&1C,&00,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&38,&38,&38,&18,&0C,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&7F,&7F,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&38,&38,&38,&00,&00,&00,&00 ;  ; 46 .
 = &00,&80,&C0,&E0,&F0,&78,&3C,&1E,&0F,&07,&03,&01,&00,&00,&00,&00 ;  ; 47 /
 = &00,&3C,&7E,&E7,&E7,&F7,&FF,&EF,&E7,&E7,&7E,&3C,&00,&00,&00,&00 ;  ; 48 0
 = &00,&1E,&3E,&38,&38,&38,&38,&38,&38,&F8,&F8,&F0,&00,&00,&00,&00 ;  ; 49 1
 = &00,&7F,&FF,&E0,&E0,&FE,&7F,&07,&07,&07,&FF,&FE,&00,&00,&00,&00 ;  ; 50 2
 = &00,&7F,&FF,&E0,&E0,&FE,&FE,&E0,&E0,&E0,&FF,&7F,&00,&00,&00,&00 ;  ; 51 3
 = &00,&E0,&70,&38,&1C,&0E,&E7,&FF,&FF,&E0,&E0,&E0,&00,&00,&00,&00 ;  ; 52 4
 = &00,&FE,&FF,&07,&07,&7F,&FE,&E0,&E0,&E0,&FF,&7F,&00,&00,&00,&00 ;  ; 53 5
 = &00,&7E,&7F,&07,&07,&7F,&FF,&E7,&E7,&E7,&FF,&7E,&00,&00,&00,&00 ;  ; 54 6
 = &00,&7F,&FF,&E0,&E0,&E0,&E0,&E0,&E0,&E0,&E0,&E0,&00,&00,&00,&00 ;  ; 55 7
 = &00,&7E,&FF,&E7,&E7,&E7,&FF,&E7,&E7,&E7,&FF,&7E,&00,&00,&00,&00 ;  ; 56 8
 = &00,&7E,&FF,&E7,&E7,&E7,&FF,&FE,&E0,&E0,&E0,&E0,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&00,&38,&38,&38,&00,&00,&38,&38,&38,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&00,&38,&38,&38,&00,&00,&38,&38,&38,&18,&0C,&00,&00 ;  ; 59 ;
 = &00,&C0,&E0,&70,&38,&1C,&0E,&1C,&38,&70,&E0,&C0,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&7F,&7F,&00,&00,&00,&7F,&7F,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&07,&0E,&1C,&38,&70,&E0,&70,&38,&1C,&0E,&07,&00,&00,&00,&00 ;  ; 62 >
 = &00,&7E,&FF,&07,&00,&E0,&70,&38,&38,&00,&38,&38,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&78,&FC,&CE,&C7,&F7,&F7,&F7,&07,&07,&FE,&7C,&00,&00,&00,&00 ;  ; 64 @
 = &00,&02,&07,&0E,&1C,&38,&76,&E7,&C3,&C3,&FF,&FF,&00,&00,&00,&00 ;  ; 65 A
 = &00,&1F,&3F,&70,&70,&3F,&7F,&E7,&C7,&C7,&FF,&7F,&00,&00,&00,&00 ;  ; 66 B
 = &00,&E0,&F0,&38,&1C,&0E,&07,&03,&03,&03,&FF,&FF,&00,&00,&00,&00 ;  ; 67 C
 = &00,&1F,&3F,&70,&E0,&C7,&C7,&C7,&C7,&E7,&FF,&7F,&00,&00,&00,&00 ;  ; 68 D
 = &00,&FF,&FF,&00,&00,&7F,&7F,&07,&07,&07,&FF,&FF,&00,&00,&00,&00 ;  ; 69 E
 = &00,&FF,&FF,&00,&00,&7F,&7F,&07,&07,&07,&07,&07,&00,&00,&00,&00 ;  ; 70 F
 = &00,&E0,&F0,&38,&1C,&0E,&07,&E3,&E3,&C3,&FF,&FF,&00,&00,&00,&00 ;  ; 71 G
 = &00,&C7,&C7,&C7,&C7,&DF,&DF,&C7,&C7,&C7,&C7,&C7,&00,&00,&00,&00 ;  ; 72 H
 = &00,&1C,&1C,&00,&00,&1C,&1C,&1C,&1C,&3C,&F8,&F0,&00,&00,&00,&00 ;  ; 73 I
 = &00,&1C,&1C,&00,&00,&1C,&1C,&1C,&1C,&1C,&1C,&1E,&0F,&07,&00,&00 ;  ; 74 J
 = &00,&07,&07,&67,&F7,&7F,&3F,&77,&E7,&C7,&C7,&C7,&00,&00,&00,&00 ;  ; 75 K
 = &00,&07,&07,&07,&07,&07,&07,&07,&07,&07,&FF,&FF,&00,&00,&00,&00 ;  ; 76 L
 = &00,&CC,&EC,&E6,&F6,&F3,&DB,&D9,&CD,&CC,&C6,&C6,&00,&00,&00,&00 ;  ; 77 M
 = &00,&C7,&CF,&DF,&F7,&E7,&C7,&C7,&C7,&C7,&C7,&C7,&00,&00,&00,&00 ;  ; 78 N
 = &00,&7C,&FE,&C0,&C0,&C7,&C7,&C7,&C7,&C7,&FE,&7C,&00,&00,&00,&00 ;  ; 79 O
 = &00,&7F,&FF,&E0,&E0,&FF,&7F,&07,&07,&07,&07,&07,&00,&00,&00,&00 ;  ; 80 P
 = &00,&7C,&FE,&C0,&C0,&C7,&C7,&C7,&C7,&DF,&FE,&7C,&E0,&00,&00,&00 ;  ; 81 Q
 = &00,&7F,&FF,&E0,&E0,&FF,&7F,&77,&E7,&C7,&C7,&C7,&00,&00,&00,&00 ;  ; 82 R
 = &00,&18,&3C,&1E,&3C,&78,&F0,&E0,&E0,&F0,&FF,&7F,&00,&00,&00,&00 ;  ; 83 S
 = &00,&FF,&FF,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&00,&00,&00,&00 ;  ; 84 T
 = &00,&E0,&E0,&E0,&E7,&E7,&E7,&E7,&E7,&E7,&FE,&7C,&00,&00,&00,&00 ;  ; 85 U
 = &00,&E0,&E0,&E0,&E7,&E7,&E7,&E7,&E7,&7E,&3C,&18,&00,&00,&00,&00 ;  ; 86 V
 = &00,&CC,&CC,&CC,&DD,&D9,&FB,&F3,&F6,&E6,&EC,&CC,&00,&00,&00,&00 ;  ; 87 W
 = &00,&E7,&E7,&E7,&66,&3C,&3C,&66,&E7,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 88 X
 = &00,&E7,&E7,&E7,&EE,&FC,&F8,&F0,&E0,&E0,&7F,&3F,&00,&00,&00,&00 ;  ; 89 Y
 = &00,&FF,&FF,&00,&00,&38,&1C,&0E,&07,&03,&FF,&FF,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&7E,&7E,&0E,&0E,&0E,&0E,&0E,&0E,&0E,&7E,&7E,&00,&00,&00,&00 ;  ; 91 [
 = &00,&01,&03,&07,&0F,&1E,&3C,&78,&F0,&E0,&C0,&80,&00,&00,&00,&00 ;  ; 92 \
 = &00,&7E,&7E,&70,&70,&70,&70,&70,&70,&70,&7E,&7E,&00,&00,&00,&00 ;  ; 93 ]
 = &00,&18,&3C,&7E,&E7,&C3,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&FF,&00,&00,&00,&00 ;  ; 95 _
 = &00,&00,&1C,&38,&70,&E0,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&02,&07,&0E,&1C,&38,&76,&E7,&C3,&C3,&FF,&FF,&00,&00,&00,&00 ;  ; 97 a
 = &00,&1F,&3F,&70,&70,&3F,&7F,&E7,&C7,&C7,&FF,&7F,&00,&00,&00,&00 ;  ; 98 b
 = &00,&E0,&F0,&38,&1C,&0E,&07,&03,&03,&03,&FF,&FF,&00,&00,&00,&00 ;  ; 99 c
 = &00,&1F,&3F,&70,&E0,&C7,&C7,&C7,&C7,&E7,&FF,&7F,&00,&00,&00,&00 ;  ; 100 d
 = &00,&FF,&FF,&00,&00,&7F,&7F,&07,&07,&07,&FF,&FF,&00,&00,&00,&00 ;  ; 101 e
 = &00,&FF,&FF,&00,&00,&7F,&7F,&07,&07,&07,&07,&07,&00,&00,&00,&00 ;  ; 102 f
 = &00,&E0,&F0,&38,&1C,&0E,&07,&E3,&E3,&C3,&FF,&FF,&00,&00,&00,&00 ;  ; 103 g
 = &00,&C7,&C7,&C7,&C7,&DF,&DF,&C7,&C7,&C7,&C7,&C7,&00,&00,&00,&00 ;  ; 104 h
 = &00,&1C,&1C,&00,&00,&1C,&1C,&1C,&1C,&3C,&F8,&F0,&00,&00,&00,&00 ;  ; 105 i
 = &00,&1C,&1C,&00,&00,&1C,&1C,&1C,&1C,&1C,&1C,&1E,&0F,&07,&00,&00 ;  ; 106 j
 = &00,&07,&07,&67,&F7,&7F,&3F,&77,&E7,&C7,&C7,&C7,&00,&00,&00,&00 ;  ; 107 k
 = &00,&07,&07,&07,&07,&07,&07,&07,&07,&07,&FF,&FF,&00,&00,&00,&00 ;  ; 108 l
 = &00,&CC,&EC,&E6,&F6,&F3,&DB,&D9,&CD,&CC,&C6,&C6,&00,&00,&00,&00 ;  ; 109 m
 = &00,&C7,&CF,&DF,&F7,&E7,&C7,&C7,&C7,&C7,&C7,&C7,&00,&00,&00,&00 ;  ; 110 n
 = &00,&7C,&FE,&C0,&C0,&C7,&C7,&C7,&C7,&C7,&FE,&7C,&00,&00,&00,&00 ;  ; 111 o
 = &00,&7F,&FF,&E0,&E0,&FF,&7F,&07,&07,&07,&07,&07,&00,&00,&00,&00 ;  ; 112 p
 = &00,&7C,&FE,&C0,&C0,&C7,&C7,&C7,&C7,&DF,&FE,&7C,&E0,&00,&00,&00 ;  ; 113 q
 = &00,&7F,&FF,&E0,&E0,&FF,&7F,&77,&E7,&C7,&C7,&C7,&00,&00,&00,&00 ;  ; 114 r
 = &00,&18,&3C,&1E,&3C,&78,&F0,&E0,&E0,&F0,&FF,&7F,&00,&00,&00,&00 ;  ; 115 s
 = &00,&FF,&FF,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&00,&00,&00,&00 ;  ; 116 t
 = &00,&E0,&E0,&E0,&E7,&E7,&E7,&E7,&E7,&E7,&FE,&7C,&00,&00,&00,&00 ;  ; 117 u
 = &00,&E0,&E0,&E0,&E7,&E7,&E7,&E7,&E7,&7E,&3C,&18,&00,&00,&00,&00 ;  ; 118 v
 = &00,&CC,&CC,&CC,&DD,&D9,&FB,&F3,&F6,&E6,&EC,&CC,&00,&00,&00,&00 ;  ; 119 w
 = &00,&E7,&E7,&E7,&66,&3C,&3C,&66,&E7,&E7,&E7,&E7,&00,&00,&00,&00 ;  ; 120 x
 = &00,&E7,&E7,&E7,&EE,&FC,&F8,&F0,&E0,&E0,&7F,&3F,&00,&00,&00,&00 ;  ; 121 y
 = &00,&FF,&FF,&00,&00,&38,&1C,&0E,&07,&03,&FF,&FF,&00,&00,&00,&00 ;  ; 122 z
 = &00,&30,&38,&1C,&1C,&0C,&0E,&0C,&1C,&1C,&38,&30,&00,&00,&00,&00 ;  ; 123 {
 = &00,&1C,&1C,&1C,&1C,&00,&00,&00,&1C,&1C,&1C,&1C,&00,&00,&00,&00 ;  ; 124 |
 = &00,&0C,&1C,&38,&38,&30,&70,&30,&38,&38,&1C,&0C,&00,&00,&00,&00 ;  ; 125 }
 = &00,&8C,&DE,&7B,&31,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&33,&33,&CC,&CC,&33,&33,&CC,&CC,&33,&33,&CC,&CC,&00 ;  ; 127 
; FONT: <GREEK>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7E,&81,&A5,&81,&81,&BD,&99,&81,&7E,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&6C,&FE,&FE,&FE,&FE,&7C,&38,&10,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&7C,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&18,&3C,&3C,&E7,&E7,&E7,&3C,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&E7,&C3,&81,&E7,&FF,&FF,&FF,&FF,&FF,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&3C,&66,&42,&42,&66,&3C,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&C3,&99,&BD,&BD,&99,&C3,&FF,&FF,&FF,&FF,&00,&00 ;  ; 10 
 = &00,&00,&F0,&E0,&B0,&98,&3C,&66,&66,&66,&3C,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&3C,&66,&66,&66,&3C,&18,&7E,&18,&18,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&FC,&CC,&FC,&0C,&0C,&0C,&0E,&0F,&07,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FE,&C6,&FE,&C6,&C6,&C6,&E6,&E7,&67,&06,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&E7,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0E,&3E,&FE,&3E,&0E,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&80,&C0,&E0,&F8,&FE,&F8,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&CC,&CC,&CC,&CC,&CC,&CC,&00,&CC,&CC,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&FE,&DB,&DB,&DB,&DE,&D8,&D8,&D8,&D8,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&7C,&C6,&0C,&38,&6C,&C6,&C6,&6C,&38,&60,&C6,&7C,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&00,&00,&FE,&FE,&FE,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&18,&3C,&7E,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&18,&18,&18,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&30,&60,&FF,&60,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&18,&0C,&FE,&0C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&06,&06,&06,&FE,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&00,&00,&10,&38,&38,&7C,&7C,&FE,&FE,&00,&00,&00,&00,&00,&00 ;  ; 30 
 = &00,&00,&00,&FE,&FE,&7C,&7C,&38,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&18,&3C,&3C,&3C,&18,&18,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&C6,&C6,&C6,&44,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&6C,&6C,&FE,&6C,&6C,&6C,&FE,&6C,&6C,&00,&00,&00,&00,&00 ;  ; 35 #
 = &30,&30,&7C,&C6,&86,&06,&7C,&C0,&C2,&C6,&7C,&30,&30,&00,&00,&00 ;  ; 36 $
 = &00,&00,&00,&00,&86,&C6,&60,&30,&18,&CC,&C6,&00,&00,&00,&00,&00 ;  ; 37 %
 = &00,&00,&38,&6C,&6C,&38,&DC,&76,&66,&66,&DC,&00,&00,&00,&00,&00 ;  ; 38 &
 = &00,&0C,&0C,&0C,&06,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&30,&18,&0C,&0C,&0C,&0C,&0C,&18,&30,&00,&00,&00,&00,&00 ;  ; 40 (
 = &00,&00,&18,&30,&60,&60,&60,&60,&60,&30,&18,&00,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&00,&00,&66,&3C,&FF,&3C,&66,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&18,&18,&18,&FF,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&18,&0C,&00,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&80,&C0,&60,&30,&18,&0C,&06,&03,&01,&00,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&7C,&C6,&E6,&F6,&DE,&CE,&C6,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 48 0
 = &00,&00,&30,&38,&3C,&30,&30,&30,&30,&30,&FC,&00,&00,&00,&00,&00 ;  ; 49 1
 = &00,&00,&7C,&C6,&C0,&60,&30,&18,&0C,&C6,&FE,&00,&00,&00,&00,&00 ;  ; 50 2
 = &00,&00,&7C,&C6,&C0,&C0,&78,&C0,&C0,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 51 3
 = &00,&00,&60,&70,&78,&6C,&66,&FE,&60,&60,&F0,&00,&00,&00,&00,&00 ;  ; 52 4
 = &00,&00,&FE,&06,&06,&06,&7E,&C0,&C0,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 53 5
 = &00,&00,&38,&0C,&06,&06,&7E,&C6,&C6,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 54 6
 = &00,&00,&FE,&C6,&C0,&60,&30,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 55 7
 = &00,&00,&7C,&C6,&C6,&C6,&7C,&C6,&C6,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 56 8
 = &00,&00,&7C,&C6,&C6,&C6,&FC,&C0,&C0,&60,&3C,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&18,&18,&00,&00,&18,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&18,&18,&00,&00,&00,&18,&18,&0C,&00,&00,&00,&00,&00 ;  ; 59 ;
 = &00,&00,&60,&30,&18,&0C,&06,&0C,&18,&30,&60,&00,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&00,&7E,&00,&00,&7E,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&00,&06,&0C,&18,&30,&60,&30,&18,&0C,&06,&00,&00,&00,&00,&00 ;  ; 62 >
 = &00,&00,&7C,&C6,&C6,&60,&30,&30,&00,&30,&30,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&00,&7C,&C6,&C6,&F6,&F6,&F6,&76,&06,&7C,&00,&00,&00,&00,&00 ;  ; 64 @
 = &00,&00,&10,&38,&6C,&C6,&C6,&FE,&C6,&C6,&C6,&00,&00,&00,&00,&00 ;  ; 65 A
 = &00,&00,&7E,&CC,&CC,&CC,&7C,&CC,&CC,&CC,&7E,&00,&00,&00,&00,&00 ;  ; 66 B
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 67 C
 = &00,&00,&10,&38,&6C,&C6,&C6,&C6,&C6,&C6,&FE,&00,&00,&00,&00,&00 ;  ; 68 D
 = &00,&00,&FE,&CC,&8C,&2C,&3C,&2C,&8C,&CC,&FE,&00,&00,&00,&00,&00 ;  ; 69 E
 = &00,&00,&3C,&66,&C3,&C3,&DB,&C3,&C3,&66,&3C,&00,&00,&00,&00,&00 ;  ; 70 F
 = &00,&00,&FE,&CC,&8C,&0C,&0C,&0C,&0C,&0C,&1E,&00,&00,&00,&00,&00 ;  ; 71 G
 = &00,&00,&C6,&C6,&C6,&C6,&FE,&C6,&C6,&C6,&C6,&00,&00,&00,&00,&00 ;  ; 72 H
 = &00,&00,&3C,&18,&18,&18,&18,&18,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 73 I
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 74 J
 = &00,&00,&CE,&CC,&6C,&6C,&3C,&6C,&6C,&CC,&CE,&00,&00,&00,&00,&00 ;  ; 75 K
 = &00,&00,&10,&38,&6C,&C6,&C6,&C6,&C6,&C6,&C6,&00,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&C3,&E7,&FF,&DB,&C3,&C3,&C3,&C3,&C3,&00,&00,&00,&00,&00 ;  ; 77 M
 = &00,&00,&C6,&CE,&DE,&FE,&F6,&E6,&C6,&C6,&C6,&00,&00,&00,&00,&00 ;  ; 78 N
 = &00,&00,&38,&6C,&C6,&C6,&C6,&C6,&C6,&6C,&38,&00,&00,&00,&00,&00 ;  ; 79 O
 = &00,&00,&FF,&66,&66,&66,&66,&66,&66,&66,&66,&00,&00,&00,&00,&00 ;  ; 80 P
 = &00,&00,&3C,&18,&7E,&DB,&DB,&DB,&7E,&18,&3C,&00,&00,&00,&00,&00 ;  ; 81 Q
 = &00,&00,&7E,&CC,&CC,&CC,&7C,&0C,&0C,&0C,&1E,&00,&00,&00,&00,&00 ;  ; 82 R
 = &00,&00,&FF,&C6,&8C,&18,&30,&18,&8C,&C6,&FF,&00,&00,&00,&00,&00 ;  ; 83 S
 = &00,&00,&FF,&DB,&99,&18,&18,&18,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 84 T
 = &00,&00,&C3,&E7,&24,&3C,&18,&18,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 85 U
 = &00,&00,&C3,&C3,&66,&3C,&18,&3C,&66,&C3,&C3,&00,&00,&00,&00,&00 ;  ; 86 V
 = &00,&00,&7C,&C6,&C6,&C6,&C6,&C6,&6C,&28,&EE,&00,&00,&00,&00,&00 ;  ; 87 W
 = &00,&00,&FE,&7F,&00,&00,&3C,&00,&00,&FE,&7F,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&00,&3C,&18,&DB,&DB,&DB,&5A,&3C,&18,&3C,&00,&00,&00,&00,&00 ;  ; 89 Y
 = &00,&00,&FE,&C6,&60,&30,&18,&0C,&06,&C6,&FE,&00,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&00,&3C,&0C,&0C,&0C,&0C,&0C,&0C,&0C,&3C,&00,&00,&00,&00,&00 ;  ; 91 [
 = &00,&00,&02,&06,&0E,&1C,&38,&70,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&3C,&30,&30,&30,&30,&30,&30,&30,&3C,&00,&00,&00,&00,&00 ;  ; 93 ]
 = &10,&38,&6C,&C6,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00 ;  ; 95 _
 = &18,&18,&30,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&DC,&36,&36,&36,&76,&DC,&00,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&00,&78,&CC,&CC,&7E,&C6,&C6,&C6,&7E,&06,&06,&04,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&7C,&C6,&8E,&3C,&38,&30,&1C,&00,&00,&00,&00 ;  ; 99 c
 = &00,&00,&3C,&66,&CC,&18,&78,&CC,&CC,&CC,&78,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&7C,&C6,&3E,&06,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 101 e
 = &00,&00,&38,&6C,&C6,&C6,&FE,&C6,&C6,&6C,&38,&00,&00,&00,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&00,&C6,&C6,&C6,&64,&3C,&18,&0C,&06,&06,&00,&00 ;  ; 103 g
 = &00,&00,&00,&00,&00,&76,&CC,&CC,&CC,&CC,&C0,&C0,&C0,&00,&00,&00 ;  ; 104 h
 = &00,&00,&00,&00,&00,&18,&18,&18,&18,&58,&70,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 106 j
 = &00,&00,&00,&00,&00,&C6,&66,&3E,&36,&66,&C6,&00,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&00,&03,&06,&0C,&0C,&1C,&34,&66,&C3,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&66,&66,&66,&66,&66,&DE,&06,&06,&06,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&CE,&CC,&CC,&CC,&6C,&3C,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&7C,&C6,&C6,&C6,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&FE,&6C,&6C,&6C,&6C,&6C,&00,&00,&00,&00,&00 ;  ; 112 p
 = &00,&00,&00,&18,&18,&18,&7E,&DB,&DB,&DB,&7E,&18,&18,&18,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&00,&7C,&C6,&C6,&C6,&CE,&76,&06,&06,&06,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&FC,&26,&66,&66,&66,&3C,&00,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&00,&00,&00,&FC,&36,&30,&30,&18,&18,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&CE,&CC,&CC,&CC,&CC,&78,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&83,&C3,&66,&3C,&18,&3C,&66,&C3,&C1,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&66,&C3,&C3,&DB,&DB,&7E,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&40,&7C,&66,&06,&3C,&06,&06,&06,&FC,&80,&70,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&18,&18,&18,&DB,&DB,&DB,&DB,&7E,&18,&18,&18,&00,&00 ;  ; 121 y
 = &00,&00,&CC,&F8,&30,&18,&0C,&06,&06,&0E,&FC,&80,&F8,&00,&00,&00 ;  ; 122 z
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 123 {
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 125 }
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 127 
; FONT: <EVHELV>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7E,&81,&A5,&81,&81,&BD,&99,&81,&7E,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&6C,&FE,&FE,&FE,&FE,&7C,&38,&10,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&7C,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&18,&3C,&3C,&E7,&E7,&E7,&3C,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&E7,&C3,&81,&E7,&FF,&FF,&FF,&FF,&FF,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&3C,&66,&42,&42,&66,&3C,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&C3,&99,&BD,&BD,&99,&C3,&FF,&FF,&FF,&FF,&00,&00 ;  ; 10 
 = &00,&00,&60,&7C,&D0,&08,&3E,&41,&41,&41,&3E,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&3E,&41,&41,&41,&3E,&08,&7F,&08,&08,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&F8,&88,&F8,&08,&08,&08,&0E,&0F,&07,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FC,&84,&FC,&84,&84,&84,&C4,&E6,&67,&03,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&C3,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0E,&1E,&3E,&1E,&0E,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&80,&C0,&E0,&F0,&F8,&F0,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&08,&1C,&3E,&08,&08,&08,&3E,&1C,&08,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&22,&22,&22,&22,&22,&22,&00,&22,&22,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&FC,&9F,&9F,&9F,&9E,&90,&90,&90,&90,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&7C,&82,&04,&18,&44,&82,&82,&44,&30,&80,&82,&3C,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&FE,&00,&FE,&00,&FE,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&08,&1C,&3E,&08,&08,&08,&3E,&1C,&08,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&00,&08,&1C,&3E,&08,&08,&08,&08,&08,&08,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&08,&08,&08,&08,&08,&08,&3E,&1C,&08,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&30,&60,&FF,&60,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&0C,&06,&FF,&06,&0C,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&02,&02,&02,&FE,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&03,&02,&82,&42,&22,&12,&48,&64,&52,&F9,&40,&40,&00,&00,&00 ;  ; 30 
 = &00,&03,&02,&82,&42,&22,&12,&68,&94,&82,&61,&10,&F0,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&08,&08,&08,&08,&08,&08,&00,&08,&08,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&00,&66,&66,&44,&44,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&48,&48,&FE,&24,&24,&7F,&24,&12,&12,&00,&00,&00,&00,&00 ;  ; 35 #
 = &00,&00,&10,&7C,&92,&12,&1C,&50,&92,&92,&7C,&10,&00,&00,&00,&00 ;  ; 36 $
 = &00,&00,&00,&8E,&51,&2E,&10,&08,&74,&8A,&71,&00,&00,&00,&00,&00 ;  ; 37 %
 = &00,&00,&1C,&22,&22,&14,&0C,&D2,&61,&41,&DE,&00,&00,&00,&00,&00 ;  ; 38 &
 = &00,&00,&18,&18,&10,&10,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&20,&08,&04,&02,&02,&02,&02,&02,&04,&08,&20,&00,&00,&00 ;  ; 40 (
 = &00,&00,&04,&10,&20,&40,&40,&40,&40,&40,&20,&10,&04,&00,&00,&00 ;  ; 41 )
 = &00,&00,&10,&92,&7C,&28,&44,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&10,&10,&10,&FE,&10,&10,&10,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&10,&10,&08,&00,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&FE,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&10,&10,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&40,&20,&10,&10,&08,&04,&04,&02,&01,&00,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&3C,&42,&81,&81,&81,&81,&81,&42,&3C,&00,&00,&00,&00,&00 ;  ; 48 0
 = &00,&00,&20,&3C,&20,&20,&20,&20,&20,&20,&20,&00,&00,&00,&00,&00 ;  ; 49 1
 = &00,&00,&7C,&82,&82,&80,&30,&04,&02,&02,&FE,&00,&00,&00,&00,&00 ;  ; 50 2
 = &00,&00,&3C,&42,&80,&40,&38,&40,&81,&41,&3E,&00,&00,&00,&00,&00 ;  ; 51 3
 = &00,&00,&60,&50,&48,&44,&42,&FF,&40,&40,&40,&00,&00,&00,&00,&00 ;  ; 52 4
 = &00,&00,&FC,&04,&02,&3E,&41,&80,&80,&41,&1E,&00,&00,&00,&00,&00 ;  ; 53 5
 = &00,&00,&38,&42,&01,&01,&3D,&43,&81,&42,&3C,&00,&00,&00,&00,&00 ;  ; 54 6
 = &00,&00,&FF,&40,&20,&10,&10,&08,&08,&04,&04,&00,&00,&00,&00,&00 ;  ; 55 7
 = &00,&00,&3C,&42,&81,&42,&3C,&42,&81,&42,&3C,&00,&00,&00,&00,&00 ;  ; 56 8
 = &00,&00,&3C,&42,&81,&C2,&BC,&80,&80,&42,&1C,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&00,&10,&10,&00,&00,&00,&10,&10,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&00,&10,&10,&00,&00,&00,&10,&10,&08,&00,&00,&00,&00 ;  ; 59 ;
 = &00,&00,&00,&40,&10,&04,&01,&04,&10,&40,&00,&00,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&00,&7E,&00,&7E,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&00,&00,&01,&04,&10,&40,&10,&04,&01,&00,&00,&00,&00,&00,&00 ;  ; 62 >
 = &00,&00,&7C,&82,&81,&20,&10,&10,&00,&10,&10,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&00,&3C,&42,&81,&B9,&A5,&D9,&01,&86,&38,&00,&00,&00,&00,&00 ;  ; 64 @
 = &00,&00,&10,&28,&28,&44,&44,&7C,&82,&82,&82,&00,&00,&00,&00,&00 ;  ; 65 A
 = &00,&00,&3E,&42,&82,&42,&3E,&42,&82,&42,&3E,&00,&00,&00,&00,&00 ;  ; 66 B
 = &00,&00,&78,&84,&02,&02,&02,&02,&82,&84,&78,&00,&00,&00,&00,&00 ;  ; 67 C
 = &00,&00,&3E,&42,&82,&82,&82,&82,&82,&42,&3E,&00,&00,&00,&00,&00 ;  ; 68 D
 = &00,&00,&FE,&02,&02,&02,&7E,&02,&02,&02,&FE,&00,&00,&00,&00,&00 ;  ; 69 E
 = &00,&00,&FE,&02,&02,&02,&7E,&02,&02,&02,&02,&00,&00,&00,&00,&00 ;  ; 70 F
 = &00,&00,&78,&84,&82,&02,&02,&F2,&82,&C4,&B8,&00,&00,&00,&00,&00 ;  ; 71 G
 = &00,&00,&42,&42,&42,&42,&7E,&42,&42,&42,&42,&00,&00,&00,&00,&00 ;  ; 72 H
 = &00,&00,&10,&10,&10,&10,&10,&10,&10,&10,&10,&00,&00,&00,&00,&00 ;  ; 73 I
 = &00,&00,&40,&40,&40,&40,&40,&40,&41,&41,&3E,&00,&00,&00,&00,&00 ;  ; 74 J
 = &00,&00,&82,&42,&22,&12,&0A,&16,&22,&42,&82,&00,&00,&00,&00,&00 ;  ; 75 K
 = &00,&00,&02,&02,&02,&02,&02,&02,&02,&02,&7E,&00,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&81,&81,&C3,&C3,&A5,&A5,&99,&99,&81,&00,&00,&00,&00,&00 ;  ; 77 M
 = &00,&00,&42,&46,&46,&4A,&52,&52,&62,&62,&42,&00,&00,&00,&00,&00 ;  ; 78 N
 = &00,&00,&3C,&42,&81,&81,&81,&81,&81,&42,&3C,&00,&00,&00,&00,&00 ;  ; 79 O
 = &00,&00,&7E,&82,&82,&82,&7E,&02,&02,&02,&02,&00,&00,&00,&00,&00 ;  ; 80 P
 = &00,&00,&3C,&42,&81,&81,&81,&81,&89,&52,&3C,&40,&80,&00,&00,&00 ;  ; 81 Q
 = &00,&00,&3E,&42,&42,&22,&1E,&22,&42,&42,&82,&00,&00,&00,&00,&00 ;  ; 82 R
 = &00,&00,&3C,&42,&02,&0C,&30,&40,&80,&42,&3C,&00,&00,&00,&00,&00 ;  ; 83 S
 = &00,&00,&FE,&10,&10,&10,&10,&10,&10,&10,&10,&00,&00,&00,&00,&00 ;  ; 84 T
 = &00,&00,&82,&82,&82,&82,&82,&82,&82,&82,&7C,&00,&00,&00,&00,&00 ;  ; 85 U
 = &00,&00,&82,&82,&82,&44,&44,&44,&28,&28,&10,&00,&00,&00,&00,&00 ;  ; 86 V
 = &00,&00,&81,&81,&81,&99,&99,&99,&A5,&66,&42,&00,&00,&00,&00,&00 ;  ; 87 W
 = &00,&00,&42,&42,&24,&24,&18,&24,&24,&42,&42,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&00,&82,&82,&44,&28,&10,&10,&10,&10,&10,&00,&00,&00,&00,&00 ;  ; 89 Y
 = &00,&00,&FE,&40,&20,&10,&08,&04,&02,&01,&7F,&00,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&00,&3C,&04,&04,&04,&04,&04,&04,&04,&04,&04,&3C,&00,&00,&00 ;  ; 91 [
 = &00,&00,&02,&04,&08,&08,&10,&20,&20,&40,&80,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&3C,&20,&20,&20,&20,&20,&20,&20,&20,&20,&3C,&00,&00,&00 ;  ; 93 ]
 = &00,&00,&10,&38,&44,&82,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00 ;  ; 95 _
 = &00,&00,&08,&08,&10,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&3C,&42,&78,&46,&41,&DE,&00,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&02,&02,&02,&3A,&46,&82,&82,&46,&3A,&00,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&78,&84,&02,&02,&84,&78,&00,&00,&00,&00,&00 ;  ; 99 c
 = &00,&00,&80,&80,&80,&B8,&C4,&82,&82,&C4,&B8,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&78,&84,&FE,&02,&84,&78,&00,&00,&00,&00,&00 ;  ; 101 e
 = &00,&00,&70,&08,&08,&7E,&08,&08,&08,&08,&08,&00,&00,&00,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&00,&BC,&C2,&81,&81,&C2,&BC,&80,&7C,&00,&00,&00 ;  ; 103 g
 = &00,&00,&02,&02,&02,&7A,&86,&82,&82,&82,&82,&00,&00,&00,&00,&00 ;  ; 104 h
 = &00,&00,&20,&20,&00,&20,&20,&20,&20,&20,&20,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&10,&10,&00,&10,&10,&10,&10,&10,&10,&10,&0E,&00,&00,&00 ;  ; 106 j
 = &00,&00,&02,&02,&02,&22,&12,&0A,&16,&22,&42,&00,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&08,&08,&08,&08,&08,&08,&08,&08,&08,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&6E,&92,&92,&92,&92,&92,&00,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&74,&8C,&84,&84,&84,&84,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&7C,&82,&82,&82,&82,&7C,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&7A,&86,&82,&82,&86,&3A,&02,&02,&00,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&00,&BC,&C2,&82,&82,&C2,&BC,&80,&80,&00,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&00,&7B,&06,&02,&02,&02,&02,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&7C,&82,&1C,&60,&82,&7C,&00,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&00,&10,&10,&7C,&10,&10,&10,&10,&60,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&82,&82,&82,&82,&C2,&BC,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&00,&82,&44,&44,&28,&28,&10,&00,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&99,&99,&99,&66,&66,&24,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&00,&42,&24,&18,&18,&24,&42,&00,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&00,&81,&42,&24,&18,&08,&04,&03,&00,&00,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&00,&7E,&20,&10,&08,&04,&7E,&00,&00,&00,&00,&00 ;  ; 122 z
 = &00,&00,&38,&04,&04,&04,&04,&03,&04,&04,&04,&04,&38,&00,&00,&00 ;  ; 123 {
 = &00,&00,&10,&10,&10,&10,&10,&10,&10,&10,&10,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&00,&0E,&10,&10,&10,&10,&60,&10,&10,&10,&10,&0E,&00,&00,&00 ;  ; 125 }
 = &00,&00,&9C,&72,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&00,&10,&28,&44,&82,&82,&82,&FE,&00,&00,&00,&00,&00 ;  ; 127 
; FONT: <EVHELVI>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7E,&81,&A5,&81,&81,&BD,&99,&81,&7E,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&6C,&FE,&FE,&FE,&FE,&7C,&38,&10,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&7C,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&18,&3C,&3C,&E7,&E7,&E7,&3C,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&E7,&C3,&81,&E7,&FF,&FF,&FF,&FF,&FF,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&3C,&66,&42,&42,&66,&3C,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&C3,&99,&BD,&BD,&99,&C3,&FF,&FF,&FF,&FF,&00,&00 ;  ; 10 
 = &00,&00,&60,&7C,&D0,&08,&3E,&41,&41,&41,&3E,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&3E,&41,&41,&41,&3E,&08,&7F,&08,&08,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&F8,&88,&F8,&08,&08,&08,&0E,&0F,&07,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FC,&84,&FC,&84,&84,&84,&C4,&E6,&67,&03,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&C3,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0E,&1E,&3E,&1E,&0E,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&80,&C0,&E0,&F0,&F8,&F0,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&08,&1C,&3E,&08,&08,&08,&3E,&1C,&08,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&44,&44,&44,&22,&22,&22,&00,&11,&11,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&F8,&BE,&9F,&5F,&5E,&50,&28,&28,&28,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&7C,&82,&04,&0C,&22,&41,&41,&22,&1C,&20,&21,&0E,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&FE,&00,&FE,&00,&FE,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&08,&1C,&3E,&08,&08,&08,&3E,&1C,&08,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&00,&08,&1C,&3E,&08,&08,&08,&08,&08,&08,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&08,&08,&08,&08,&08,&08,&3E,&1C,&08,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&30,&60,&FF,&60,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&0C,&06,&FF,&06,&0C,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&02,&02,&02,&FE,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&06,&04,&04,&42,&22,&12,&48,&32,&29,&7C,&10,&10,&00,&00,&00 ;  ; 30 
 = &00,&06,&04,&84,&42,&22,&12,&68,&8A,&41,&18,&04,&3C,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&10,&10,&10,&08,&08,&08,&00,&04,&04,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&00,&44,&44,&22,&22,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&90,&90,&FE,&24,&24,&24,&7F,&09,&09,&00,&00,&00,&00,&00 ;  ; 35 #
 = &00,&00,&20,&78,&92,&16,&38,&50,&49,&49,&3E,&04,&00,&00,&00,&00 ;  ; 36 $
 = &00,&00,&00,&BC,&51,&2E,&10,&08,&E4,&8A,&70,&00,&00,&00,&00,&00 ;  ; 37 %
 = &00,&00,&70,&88,&44,&1C,&0C,&D2,&31,&21,&6E,&00,&00,&00,&00,&00 ;  ; 38 &
 = &00,&00,&10,&10,&08,&08,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&40,&10,&08,&04,&02,&02,&02,&02,&02,&04,&10,&00,&00,&00 ;  ; 40 (
 = &00,&00,&08,&10,&20,&20,&20,&20,&20,&10,&08,&04,&02,&00,&00,&00 ;  ; 41 )
 = &00,&00,&10,&92,&3C,&14,&22,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&10,&10,&08,&7F,&08,&04,&04,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&10,&10,&08,&00,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&7E,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&10,&10,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&80,&40,&20,&20,&10,&08,&08,&04,&02,&00,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&78,&84,&82,&82,&81,&41,&41,&21,&1E,&00,&00,&00,&00,&00 ;  ; 48 0
 = &00,&00,&20,&3C,&20,&10,&10,&10,&08,&08,&08,&00,&00,&00,&00,&00 ;  ; 49 1
 = &00,&00,&7C,&82,&81,&40,&18,&02,&01,&01,&3F,&00,&00,&00,&00,&00 ;  ; 50 2
 = &00,&00,&78,&84,&80,&40,&38,&40,&40,&21,&1E,&00,&00,&00,&00,&00 ;  ; 51 3
 = &00,&00,&C0,&A0,&48,&44,&22,&FF,&20,&10,&10,&00,&00,&00,&00,&00 ;  ; 52 4
 = &00,&00,&F8,&08,&04,&3E,&41,&40,&40,&21,&0E,&00,&00,&00,&00,&00 ;  ; 53 5
 = &00,&00,&70,&88,&04,&02,&3F,&41,&41,&21,&1E,&00,&00,&00,&00,&00 ;  ; 54 6
 = &00,&00,&FE,&40,&20,&10,&08,&04,&04,&02,&02,&00,&00,&00,&00,&00 ;  ; 55 7
 = &00,&00,&78,&84,&82,&42,&3E,&41,&41,&21,&1E,&00,&00,&00,&00,&00 ;  ; 56 8
 = &00,&00,&7C,&82,&81,&81,&FE,&40,&40,&21,&0E,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&00,&10,&10,&00,&00,&00,&04,&04,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&00,&10,&10,&00,&00,&00,&04,&04,&02,&00,&00,&00,&00 ;  ; 59 ;
 = &00,&00,&00,&40,&10,&04,&01,&02,&04,&08,&00,&00,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&00,&7E,&00,&3F,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&00,&00,&04,&08,&20,&40,&10,&04,&01,&00,&00,&00,&00,&00,&00 ;  ; 62 >
 = &00,&00,&7C,&82,&81,&20,&08,&04,&00,&02,&02,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&00,&70,&8C,&82,&B9,&55,&3D,&01,&01,&1E,&00,&00,&00,&00,&00 ;  ; 64 @
 = &00,&00,&20,&30,&48,&44,&42,&7E,&42,&41,&41,&00,&00,&00,&00,&00 ;  ; 65 A
 = &00,&00,&7C,&84,&84,&42,&3E,&42,&41,&41,&3F,&00,&00,&00,&00,&00 ;  ; 66 B
 = &00,&00,&78,&84,&02,&01,&01,&01,&41,&21,&1E,&00,&00,&00,&00,&00 ;  ; 67 C
 = &00,&00,&7C,&84,&84,&82,&82,&42,&41,&21,&1F,&00,&00,&00,&00,&00 ;  ; 68 D
 = &00,&00,&FC,&04,&04,&02,&3E,&02,&01,&01,&3F,&00,&00,&00,&00,&00 ;  ; 69 E
 = &00,&00,&FC,&04,&04,&02,&3E,&02,&01,&01,&01,&00,&00,&00,&00,&00 ;  ; 70 F
 = &00,&00,&70,&84,&82,&02,&02,&71,&41,&62,&5C,&00,&00,&00,&00,&00 ;  ; 71 G
 = &00,&00,&84,&84,&84,&42,&7E,&42,&21,&21,&21,&00,&00,&00,&00,&00 ;  ; 72 H
 = &00,&00,&10,&10,&10,&08,&08,&08,&04,&04,&04,&00,&00,&00,&00,&00 ;  ; 73 I
 = &00,&00,&80,&80,&80,&40,&40,&40,&21,&21,&1E,&00,&00,&00,&00,&00 ;  ; 74 J
 = &00,&00,&84,&44,&24,&12,&0A,&16,&11,&21,&41,&00,&00,&00,&00,&00 ;  ; 75 K
 = &00,&00,&04,&04,&04,&02,&02,&02,&01,&01,&3F,&00,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&84,&CC,&EC,&BA,&9A,&92,&49,&49,&41,&00,&00,&00,&00,&00 ;  ; 77 M
 = &00,&00,&84,&84,&8C,&4A,&52,&32,&31,&21,&21,&00,&00,&00,&00,&00 ;  ; 78 N
 = &00,&00,&78,&84,&82,&81,&81,&81,&41,&21,&1E,&00,&00,&00,&00,&00 ;  ; 79 O
 = &00,&00,&7C,&84,&84,&42,&3E,&02,&01,&01,&01,&00,&00,&00,&00,&00 ;  ; 80 P
 = &00,&00,&78,&84,&82,&81,&81,&81,&45,&29,&1E,&20,&40,&00,&00,&00 ;  ; 81 Q
 = &00,&00,&7C,&84,&84,&42,&3E,&22,&21,&21,&41,&00,&00,&00,&00,&00 ;  ; 82 R
 = &00,&00,&78,&84,&82,&0C,&30,&40,&41,&21,&1E,&00,&00,&00,&00,&00 ;  ; 83 S
 = &00,&00,&FE,&10,&10,&08,&08,&08,&04,&04,&04,&00,&00,&00,&00,&00 ;  ; 84 T
 = &00,&00,&84,&84,&42,&42,&42,&21,&21,&21,&1E,&00,&00,&00,&00,&00 ;  ; 85 U
 = &00,&00,&84,&84,&84,&42,&42,&42,&24,&14,&08,&00,&00,&00,&00,&00 ;  ; 86 V
 = &00,&00,&82,&82,&92,&59,&59,&55,&35,&33,&21,&00,&00,&00,&00,&00 ;  ; 87 W
 = &00,&00,&84,&44,&28,&18,&18,&18,&14,&22,&21,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&00,&82,&42,&24,&14,&08,&08,&04,&04,&04,&00,&00,&00,&00,&00 ;  ; 89 Y
 = &00,&00,&FC,&80,&40,&20,&08,&04,&02,&01,&3F,&00,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&00,&78,&08,&08,&04,&04,&02,&02,&02,&01,&01,&1F,&00,&00,&00 ;  ; 91 [
 = &00,&00,&04,&04,&08,&08,&08,&10,&10,&20,&20,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&78,&40,&40,&40,&20,&20,&20,&10,&10,&10,&0F,&00,&00,&00 ;  ; 93 ]
 = &00,&00,&10,&28,&22,&21,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00 ;  ; 95 _
 = &00,&00,&08,&08,&10,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&78,&84,&F8,&46,&41,&DE,&00,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&04,&04,&04,&7A,&82,&82,&41,&43,&3D,&00,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&78,&84,&02,&81,&42,&3C,&00,&00,&00,&00,&00 ;  ; 99 c
 = &00,&00,&80,&80,&40,&5C,&62,&41,&21,&31,&2E,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&3C,&42,&7F,&01,&21,&1E,&00,&00,&00,&00,&00 ;  ; 101 e
 = &00,&00,&78,&84,&04,&02,&1F,&02,&02,&01,&01,&00,&00,&00,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&00,&F8,&84,&82,&42,&7C,&20,&21,&1E,&00,&00,&00 ;  ; 103 g
 = &00,&00,&04,&04,&04,&3A,&46,&42,&21,&21,&21,&00,&00,&00,&00,&00 ;  ; 104 h
 = &00,&00,&20,&20,&00,&10,&10,&10,&08,&08,&08,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&80,&80,&00,&40,&40,&40,&20,&20,&20,&11,&0E,&00,&00,&00 ;  ; 106 j
 = &00,&00,&04,&04,&04,&42,&12,&0E,&09,&11,&21,&00,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&20,&20,&20,&10,&10,&10,&08,&08,&08,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&6E,&92,&92,&49,&49,&49,&00,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&74,&84,&84,&42,&42,&42,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&7C,&82,&82,&41,&41,&3E,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&74,&84,&84,&42,&3A,&02,&01,&01,&00,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&00,&BC,&82,&81,&41,&5E,&40,&20,&20,&00,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&00,&F4,&04,&04,&02,&02,&02,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&7C,&82,&0C,&30,&41,&3E,&00,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&00,&08,&08,&3E,&04,&04,&02,&02,&1C,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&82,&82,&82,&41,&61,&5E,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&00,&82,&82,&42,&24,&14,&08,&00,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&82,&92,&92,&49,&55,&22,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&00,&82,&24,&18,&14,&22,&41,&00,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&00,&82,&44,&24,&18,&08,&04,&03,&00,&00,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&00,&7E,&20,&10,&04,&02,&3F,&00,&00,&00,&00,&00 ;  ; 122 z
 = &00,&00,&E0,&10,&08,&08,&08,&07,&04,&02,&02,&02,&1C,&00,&00,&00 ;  ; 123 {
 = &00,&00,&10,&10,&10,&10,&10,&10,&10,&10,&10,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&00,&70,&80,&40,&20,&20,&F0,&10,&10,&10,&08,&07,&00,&00,&00 ;  ; 125 }
 = &00,&00,&9C,&72,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&00,&10,&28,&44,&82,&82,&82,&FE,&00,&00,&00,&00,&00 ;  ; 127 
; FONT: <EVCOURI>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7E,&81,&A5,&81,&81,&BD,&99,&81,&7E,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&6C,&FE,&FE,&FE,&FE,&7C,&38,&10,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&7C,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&18,&3C,&3C,&E7,&E7,&E7,&3C,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&E7,&C3,&81,&E7,&FF,&FF,&FF,&FF,&FF,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&3C,&66,&42,&42,&66,&3C,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&C3,&99,&BD,&BD,&99,&C3,&FF,&FF,&FF,&FF,&00,&00 ;  ; 10 
 = &00,&00,&60,&7C,&D0,&08,&3E,&41,&41,&41,&3E,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&3E,&41,&41,&41,&3E,&08,&7F,&08,&08,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&F8,&88,&F8,&08,&08,&08,&0E,&0F,&07,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FC,&84,&FC,&84,&84,&84,&C4,&E6,&67,&03,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&C3,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0E,&1E,&3E,&1E,&0E,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&80,&C0,&E0,&F0,&F8,&F0,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&08,&1C,&3E,&08,&08,&08,&3E,&1C,&08,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&88,&88,&44,&44,&22,&22,&00,&11,&11,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&F8,&9E,&9F,&4F,&4E,&48,&24,&24,&24,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&78,&84,&04,&18,&64,&82,&82,&84,&70,&40,&42,&3C,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&FE,&00,&FE,&00,&FE,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&08,&1C,&3E,&08,&08,&08,&3E,&1C,&08,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&00,&08,&1C,&3E,&08,&08,&08,&08,&08,&08,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&08,&08,&08,&08,&08,&08,&3E,&1C,&08,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&30,&60,&FF,&60,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&0C,&06,&FF,&06,&0C,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&02,&02,&02,&FE,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&0E,&04,&84,&42,&22,&17,&88,&C4,&52,&F9,&20,&20,&00,&00,&00 ;  ; 30 
 = &00,&0E,&04,&84,&42,&22,&17,&68,&94,&42,&11,&08,&3C,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&40,&70,&78,&38,&18,&08,&00,&04,&04,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&00,&D8,&6C,&12,&09,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&48,&48,&FE,&24,&24,&24,&7F,&12,&12,&00,&00,&00,&00,&00 ;  ; 35 #
 = &20,&20,&70,&84,&82,&06,&38,&C0,&81,&41,&1E,&04,&04,&00,&00,&00 ;  ; 36 $
 = &00,&00,&00,&84,&45,&21,&10,&08,&84,&A2,&21,&00,&00,&00,&00,&00 ;  ; 37 %
 = &00,&00,&70,&88,&48,&18,&0C,&D2,&51,&21,&EE,&00,&00,&00,&00,&00 ;  ; 38 &
 = &00,&00,&30,&18,&04,&02,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&20,&08,&04,&02,&02,&02,&02,&04,&08,&00,&00,&00,&00,&00 ;  ; 40 (
 = &00,&00,&10,&20,&40,&40,&40,&40,&20,&08,&02,&00,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&84,&28,&FF,&14,&21,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&20,&20,&10,&FE,&10,&08,&08,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&08,&04,&00,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&FE,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&10,&10,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&80,&40,&20,&10,&08,&04,&02,&01,&00,&00,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&78,&84,&82,&82,&81,&41,&41,&21,&1E,&00,&00,&00,&00,&00 ;  ; 48 0
 = &00,&00,&38,&20,&20,&10,&10,&10,&08,&08,&3E,&00,&00,&00,&00,&00 ;  ; 49 1
 = &00,&00,&78,&84,&80,&40,&30,&08,&04,&42,&7F,&00,&00,&00,&00,&00 ;  ; 50 2
 = &00,&00,&78,&84,&80,&40,&38,&40,&40,&21,&1E,&00,&00,&00,&00,&00 ;  ; 51 3
 = &00,&00,&60,&50,&28,&24,&22,&7F,&10,&10,&3C,&00,&00,&00,&00,&00 ;  ; 52 4
 = &00,&00,&FC,&04,&02,&02,&7E,&80,&80,&41,&3E,&00,&00,&00,&00,&00 ;  ; 53 5
 = &00,&00,&E0,&08,&04,&02,&7D,&83,&81,&41,&3E,&00,&00,&00,&00,&00 ;  ; 54 6
 = &00,&00,&FE,&81,&40,&20,&10,&08,&04,&02,&02,&00,&00,&00,&00,&00 ;  ; 55 7
 = &00,&00,&78,&84,&82,&44,&3C,&42,&41,&21,&1E,&00,&00,&00,&00,&00 ;  ; 56 8
 = &00,&00,&78,&84,&82,&C2,&BC,&40,&20,&10,&07,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&00,&10,&10,&00,&00,&04,&04,&00,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&00,&10,&10,&00,&00,&04,&04,&02,&00,&00,&00,&00,&00 ;  ; 59 ;
 = &00,&00,&10,&08,&04,&02,&01,&01,&02,&04,&08,&00,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&00,&FE,&00,&00,&7F,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&00,&10,&20,&40,&80,&80,&40,&20,&10,&08,&00,&00,&00,&00,&00 ;  ; 62 >
 = &00,&00,&78,&82,&81,&40,&10,&08,&00,&04,&04,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&00,&60,&88,&84,&72,&4A,&79,&01,&01,&0E,&00,&00,&00,&00,&00 ;  ; 64 @
 = &00,&00,&78,&F0,&88,&88,&44,&7C,&22,&22,&33,&00,&00,&00,&00,&00 ;  ; 65 A
 = &00,&00,&7C,&88,&88,&44,&3C,&44,&42,&42,&3F,&00,&00,&00,&00,&00 ;  ; 66 B
 = &00,&00,&B0,&C8,&84,&02,&01,&01,&01,&21,&1E,&00,&00,&00,&00,&00 ;  ; 67 C
 = &00,&00,&7E,&84,&84,&42,&42,&42,&21,&21,&1F,&00,&00,&00,&00,&00 ;  ; 68 D
 = &00,&00,&FE,&84,&84,&22,&3E,&12,&41,&21,&3F,&00,&00,&00,&00,&00 ;  ; 69 E
 = &00,&00,&FE,&84,&84,&22,&3E,&12,&01,&01,&07,&00,&00,&00,&00,&00 ;  ; 70 F
 = &00,&00,&B8,&C4,&84,&02,&02,&F2,&41,&21,&1E,&00,&00,&00,&00,&00 ;  ; 71 G
 = &00,&00,&CC,&44,&44,&22,&3E,&22,&11,&11,&33,&00,&00,&00,&00,&00 ;  ; 72 H
 = &00,&00,&7C,&10,&10,&08,&08,&08,&04,&04,&1F,&00,&00,&00,&00,&00 ;  ; 73 I
 = &00,&00,&E0,&80,&80,&40,&40,&40,&21,&21,&1E,&00,&00,&00,&00,&00 ;  ; 74 J
 = &00,&00,&8C,&48,&28,&14,&0C,&0C,&12,&22,&63,&00,&00,&00,&00,&00 ;  ; 75 K
 = &00,&00,&1C,&08,&08,&04,&04,&04,&82,&42,&7F,&00,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&84,&84,&6C,&5A,&5A,&42,&21,&21,&63,&00,&00,&00,&00,&00 ;  ; 77 M
 = &00,&00,&88,&88,&88,&54,&54,&74,&22,&22,&23,&00,&00,&00,&00,&00 ;  ; 78 N
 = &00,&00,&78,&84,&84,&42,&42,&42,&21,&21,&1E,&00,&00,&00,&00,&00 ;  ; 79 O
 = &00,&00,&7E,&84,&84,&42,&3E,&02,&01,&01,&03,&00,&00,&00,&00,&00 ;  ; 80 P
 = &00,&00,&70,&88,&84,&84,&82,&42,&41,&21,&1E,&04,&1B,&00,&00,&00 ;  ; 81 Q
 = &00,&00,&7E,&84,&84,&42,&1E,&12,&21,&21,&63,&00,&00,&00,&00,&00 ;  ; 82 R
 = &00,&00,&B8,&84,&84,&08,&10,&20,&42,&43,&3D,&00,&00,&00,&00,&00 ;  ; 83 S
 = &00,&00,&FC,&92,&91,&08,&08,&08,&04,&04,&0F,&00,&00,&00,&00,&00 ;  ; 84 T
 = &00,&00,&C6,&84,&84,&42,&42,&42,&21,&21,&1E,&00,&00,&00,&00,&00 ;  ; 85 U
 = &00,&00,&C6,&84,&42,&42,&42,&21,&21,&0A,&06,&00,&00,&00,&00,&00 ;  ; 86 V
 = &00,&00,&C6,&84,&84,&46,&42,&5A,&3F,&33,&33,&00,&00,&00,&00,&00 ;  ; 87 W
 = &00,&00,&86,&84,&48,&28,&18,&1C,&12,&21,&61,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&00,&E7,&42,&24,&14,&08,&04,&04,&02,&07,&00,&00,&00,&00,&00 ;  ; 89 Y
 = &00,&00,&FC,&84,&42,&20,&10,&08,&84,&42,&7F,&00,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&00,&3C,&04,&04,&02,&02,&02,&01,&01,&0F,&00,&00,&00,&00,&00 ;  ; 91 [
 = &00,&00,&08,&08,&10,&10,&10,&20,&20,&20,&00,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&3C,&20,&20,&10,&10,&10,&08,&08,&0F,&00,&00,&00,&00,&00 ;  ; 93 ]
 = &00,&20,&70,&84,&81,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00 ;  ; 95 _
 = &00,&08,&08,&10,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&78,&80,&7C,&41,&21,&5E,&00,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&0E,&08,&04,&74,&8C,&82,&82,&42,&1B,&00,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&70,&84,&02,&01,&41,&3E,&00,&00,&00,&00,&00 ;  ; 99 c
 = &00,&00,&E0,&80,&40,&78,&42,&21,&21,&31,&6E,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&78,&84,&7E,&01,&21,&1E,&00,&00,&00,&00,&00 ;  ; 101 e
 = &00,&00,&F0,&08,&08,&04,&1E,&04,&02,&02,&0F,&00,&00,&00,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&00,&FC,&42,&41,&21,&3E,&20,&10,&0F,&00,&00,&00 ;  ; 103 g
 = &00,&00,&0E,&08,&04,&3C,&46,&42,&22,&21,&73,&00,&00,&00,&00,&00 ;  ; 104 h
 = &00,&00,&40,&40,&00,&38,&20,&10,&10,&08,&3E,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&80,&80,&00,&E0,&80,&80,&40,&40,&20,&20,&1F,&00,&00,&00 ;  ; 106 j
 = &00,&00,&1C,&08,&08,&E4,&24,&1C,&12,&22,&67,&00,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&38,&20,&20,&10,&10,&10,&08,&08,&3E,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&CC,&EC,&5A,&42,&21,&63,&00,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&6C,&88,&44,&44,&22,&77,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&7C,&82,&82,&41,&41,&3E,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&6C,&84,&84,&42,&3E,&02,&01,&07,&00,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&00,&D8,&42,&41,&21,&3E,&10,&10,&3C,&00,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&00,&EC,&98,&08,&04,&04,&0E,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&78,&84,&18,&20,&41,&3E,&00,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&10,&10,&08,&7C,&08,&08,&04,&44,&18,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&66,&44,&22,&22,&31,&6E,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&00,&C6,&84,&42,&22,&12,&0C,&00,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&C6,&44,&54,&7E,&33,&33,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&00,&C6,&24,&18,&18,&24,&63,&00,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&00,&CC,&88,&88,&48,&18,&04,&02,&07,&00,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&00,&FC,&44,&20,&08,&44,&7F,&00,&00,&00,&00,&00 ;  ; 122 z
 = &00,&00,&E0,&10,&10,&08,&0E,&08,&04,&04,&38,&00,&00,&00,&00,&00 ;  ; 123 {
 = &00,&00,&10,&10,&10,&10,&00,&10,&10,&10,&10,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&00,&38,&40,&40,&20,&C0,&20,&10,&10,&0E,&00,&00,&00,&00,&00 ;  ; 125 }
 = &00,&00,&9C,&72,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&00,&10,&28,&44,&82,&82,&82,&FE,&00,&00,&00,&00,&00 ;  ; 127 
; FONT: <EVCOUR>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7E,&81,&A5,&81,&81,&BD,&99,&81,&7E,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&6C,&FE,&FE,&FE,&FE,&7C,&38,&10,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&7C,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&18,&3C,&3C,&E7,&E7,&E7,&3C,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&E7,&C3,&81,&E7,&FF,&FF,&FF,&FF,&FF,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&3C,&66,&42,&42,&66,&3C,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&C3,&99,&BD,&BD,&99,&C3,&FF,&FF,&FF,&FF,&00,&00 ;  ; 10 
 = &00,&00,&60,&7C,&D0,&08,&3E,&41,&41,&41,&3E,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&3E,&41,&41,&41,&3E,&08,&7F,&08,&08,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&F8,&88,&F8,&08,&08,&08,&0E,&0F,&07,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FC,&84,&FC,&84,&84,&84,&C4,&E6,&67,&03,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&C3,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0E,&1E,&3E,&1E,&0E,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&80,&C0,&E0,&F0,&F8,&F0,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&08,&1C,&3E,&08,&08,&08,&3E,&1C,&08,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&22,&22,&22,&22,&22,&22,&00,&22,&22,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&FC,&9F,&9F,&9F,&9E,&90,&90,&90,&90,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&7C,&82,&04,&18,&64,&82,&82,&84,&70,&40,&82,&7C,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&FE,&00,&FE,&00,&FE,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&08,&1C,&3E,&08,&08,&08,&3E,&1C,&08,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&00,&08,&1C,&3E,&08,&08,&08,&08,&08,&08,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&08,&08,&08,&08,&08,&08,&3E,&1C,&08,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&30,&60,&FF,&60,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&0C,&06,&FF,&06,&0C,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&02,&02,&02,&FE,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&03,&02,&82,&42,&22,&17,&48,&64,&52,&F9,&40,&40,&00,&00,&00 ;  ; 30 
 = &00,&03,&02,&82,&42,&22,&17,&68,&94,&42,&21,&10,&F0,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&10,&38,&38,&38,&10,&10,&00,&10,&10,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&00,&66,&66,&44,&44,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&24,&24,&7E,&24,&24,&24,&7E,&24,&24,&00,&00,&00,&00,&00 ;  ; 35 #
 = &10,&10,&7C,&82,&82,&02,&7C,&80,&82,&82,&7C,&10,&10,&00,&00,&00 ;  ; 36 $
 = &00,&00,&00,&82,&45,&22,&10,&08,&44,&A2,&41,&00,&00,&00,&00,&00 ;  ; 37 %
 = &00,&00,&1C,&22,&12,&0C,&0C,&D2,&22,&42,&DC,&00,&00,&00,&00,&00 ;  ; 38 &
 = &00,&00,&0C,&0C,&08,&08,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&20,&08,&04,&04,&04,&04,&04,&08,&20,&00,&00,&00,&00,&00 ;  ; 40 (
 = &00,&00,&08,&20,&40,&40,&40,&40,&40,&20,&08,&00,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&42,&24,&FF,&24,&42,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&10,&10,&10,&FE,&10,&10,&10,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&08,&04,&00,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&FE,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&10,&10,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&80,&40,&20,&10,&08,&04,&02,&01,&00,&00,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&38,&44,&82,&82,&82,&82,&82,&44,&38,&00,&00,&00,&00,&00 ;  ; 48 0
 = &00,&00,&38,&20,&20,&20,&20,&20,&20,&20,&F8,&00,&00,&00,&00,&00 ;  ; 49 1
 = &00,&00,&7C,&82,&80,&40,&20,&10,&08,&84,&FE,&00,&00,&00,&00,&00 ;  ; 50 2
 = &00,&00,&7C,&82,&80,&80,&78,&80,&80,&82,&7C,&00,&00,&00,&00,&00 ;  ; 51 3
 = &00,&00,&60,&50,&48,&44,&42,&FE,&40,&40,&F0,&00,&00,&00,&00,&00 ;  ; 52 4
 = &00,&00,&FE,&02,&02,&02,&7E,&80,&80,&82,&7C,&00,&00,&00,&00,&00 ;  ; 53 5
 = &00,&00,&78,&04,&02,&02,&7E,&82,&82,&82,&7C,&00,&00,&00,&00,&00 ;  ; 54 6
 = &00,&00,&FE,&82,&80,&40,&20,&10,&08,&04,&04,&00,&00,&00,&00,&00 ;  ; 55 7
 = &00,&00,&7C,&82,&82,&82,&7C,&82,&82,&82,&7C,&00,&00,&00,&00,&00 ;  ; 56 8
 = &00,&00,&7C,&82,&82,&82,&FC,&80,&80,&40,&3C,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&00,&10,&10,&00,&00,&10,&10,&00,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&00,&10,&10,&00,&00,&10,&10,&08,&00,&00,&00,&00,&00 ;  ; 59 ;
 = &00,&00,&20,&10,&08,&04,&02,&04,&08,&10,&20,&00,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&00,&7E,&00,&00,&7E,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&00,&04,&08,&10,&20,&40,&20,&10,&08,&04,&00,&00,&00,&00,&00 ;  ; 62 >
 = &00,&00,&7C,&82,&82,&40,&20,&20,&00,&20,&20,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&00,&7C,&82,&82,&F2,&8A,&F2,&02,&02,&7C,&00,&00,&00,&00,&00 ;  ; 64 @
 = &00,&00,&1C,&38,&6C,&44,&44,&7C,&44,&44,&EE,&00,&00,&00,&00,&00 ;  ; 65 A
 = &00,&00,&7E,&84,&84,&84,&7C,&84,&84,&84,&7E,&00,&00,&00,&00,&00 ;  ; 66 B
 = &00,&00,&B8,&C4,&82,&02,&02,&02,&02,&84,&78,&00,&00,&00,&00,&00 ;  ; 67 C
 = &00,&00,&7E,&84,&84,&84,&84,&84,&84,&84,&7E,&00,&00,&00,&00,&00 ;  ; 68 D
 = &00,&00,&FE,&84,&84,&24,&3C,&24,&84,&84,&FE,&00,&00,&00,&00,&00 ;  ; 69 E
 = &00,&00,&FE,&84,&84,&24,&3C,&24,&04,&04,&1E,&00,&00,&00,&00,&00 ;  ; 70 F
 = &00,&00,&B8,&C4,&82,&02,&02,&F2,&82,&84,&78,&00,&00,&00,&00,&00 ;  ; 71 G
 = &00,&00,&EE,&44,&44,&44,&7C,&44,&44,&44,&EE,&00,&00,&00,&00,&00 ;  ; 72 H
 = &00,&00,&7C,&10,&10,&10,&10,&10,&10,&10,&7C,&00,&00,&00,&00,&00 ;  ; 73 I
 = &00,&00,&F0,&40,&40,&40,&40,&40,&42,&42,&3C,&00,&00,&00,&00,&00 ;  ; 74 J
 = &00,&00,&EE,&44,&24,&14,&0C,&14,&24,&64,&CE,&00,&00,&00,&00,&00 ;  ; 75 K
 = &00,&00,&0E,&04,&04,&04,&04,&04,&84,&84,&FE,&00,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&C3,&66,&66,&5A,&5A,&42,&42,&42,&E7,&00,&00,&00,&00,&00 ;  ; 77 M
 = &00,&00,&E6,&4C,&4C,&5C,&54,&74,&64,&64,&4E,&00,&00,&00,&00,&00 ;  ; 78 N
 = &00,&00,&7C,&82,&82,&82,&82,&82,&82,&82,&7C,&00,&00,&00,&00,&00 ;  ; 79 O
 = &00,&00,&7E,&84,&84,&84,&7C,&04,&04,&04,&0E,&00,&00,&00,&00,&00 ;  ; 80 P
 = &00,&00,&38,&44,&82,&82,&82,&82,&82,&44,&38,&10,&6C,&00,&00,&00 ;  ; 81 Q
 = &00,&00,&3F,&42,&42,&42,&3E,&22,&42,&42,&C7,&00,&00,&00,&00,&00 ;  ; 82 R
 = &00,&00,&BC,&C2,&82,&04,&38,&40,&82,&86,&7A,&00,&00,&00,&00,&00 ;  ; 83 S
 = &00,&00,&FE,&92,&92,&10,&10,&10,&10,&10,&38,&00,&00,&00,&00,&00 ;  ; 84 T
 = &00,&00,&E7,&42,&42,&42,&42,&42,&42,&42,&3C,&00,&00,&00,&00,&00 ;  ; 85 U
 = &00,&00,&E7,&42,&42,&42,&42,&42,&42,&24,&18,&00,&00,&00,&00,&00 ;  ; 86 V
 = &00,&00,&E7,&42,&42,&42,&42,&5A,&5A,&66,&66,&00,&00,&00,&00,&00 ;  ; 87 W
 = &00,&00,&E7,&42,&24,&24,&18,&24,&24,&42,&E7,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&00,&EE,&44,&44,&28,&10,&10,&10,&10,&38,&00,&00,&00,&00,&00 ;  ; 89 Y
 = &00,&00,&FE,&82,&42,&20,&10,&08,&84,&82,&FE,&00,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&00,&3C,&04,&04,&04,&04,&04,&04,&04,&3C,&00,&00,&00,&00,&00 ;  ; 91 [
 = &00,&00,&01,&02,&04,&08,&10,&20,&40,&80,&00,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&3C,&20,&20,&20,&20,&20,&20,&20,&3C,&00,&00,&00,&00,&00 ;  ; 93 ]
 = &00,&10,&28,&44,&82,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00 ;  ; 95 _
 = &00,&08,&08,&10,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&7C,&40,&7C,&42,&42,&BC,&00,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&06,&04,&04,&74,&8C,&84,&84,&8C,&76,&00,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&7C,&82,&02,&02,&82,&7C,&00,&00,&00,&00,&00 ;  ; 99 c
 = &00,&00,&60,&40,&40,&7C,&42,&42,&42,&62,&DC,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&3C,&42,&7E,&02,&42,&3C,&00,&00,&00,&00,&00 ;  ; 101 e
 = &00,&00,&78,&04,&04,&04,&3E,&04,&04,&04,&1E,&00,&00,&00,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&00,&FC,&42,&42,&42,&7C,&40,&40,&3C,&00,&00,&00 ;  ; 103 g
 = &00,&00,&06,&04,&04,&34,&4C,&44,&44,&44,&EE,&00,&00,&00,&00,&00 ;  ; 104 h
 = &00,&00,&00,&30,&00,&38,&20,&20,&20,&20,&F8,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&00,&60,&00,&70,&40,&40,&40,&40,&40,&3C,&00,&00,&00,&00 ;  ; 106 j
 = &00,&00,&06,&04,&04,&E4,&24,&1C,&24,&44,&EE,&00,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&38,&20,&20,&20,&20,&20,&20,&20,&F8,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&E7,&5A,&5A,&42,&42,&E7,&00,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&36,&4C,&44,&44,&44,&EE,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&7C,&82,&82,&82,&82,&7C,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&76,&8C,&84,&84,&7C,&04,&04,&1E,&00,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&00,&DC,&62,&42,&42,&7C,&40,&40,&F0,&00,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&00,&76,&4C,&04,&04,&04,&1E,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&7C,&C2,&1C,&70,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&08,&08,&08,&7E,&08,&08,&08,&88,&70,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&66,&44,&44,&44,&64,&DC,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&00,&E7,&42,&42,&42,&24,&18,&00,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&E7,&42,&5A,&5A,&66,&66,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&00,&EE,&44,&38,&28,&44,&EE,&00,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&00,&EE,&44,&44,&44,&28,&10,&08,&1E,&00,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&00,&FE,&42,&20,&10,&88,&FE,&00,&00,&00,&00,&00 ;  ; 122 z
 = &00,&00,&70,&08,&08,&08,&06,&08,&08,&08,&70,&00,&00,&00,&00,&00 ;  ; 123 {
 = &00,&00,&10,&10,&10,&10,&00,&10,&10,&10,&10,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&00,&0E,&10,&10,&10,&60,&10,&10,&10,&0E,&00,&00,&00,&00,&00 ;  ; 125 }
 = &00,&00,&9C,&72,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&00,&10,&28,&44,&82,&82,&82,&FE,&00,&00,&00,&00,&00 ;  ; 127 
; FONT: <EVCOURB>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7E,&81,&A5,&81,&81,&BD,&99,&81,&7E,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&6C,&FE,&FE,&FE,&FE,&7C,&38,&10,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&7C,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&18,&3C,&3C,&E7,&E7,&E7,&3C,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&E7,&C3,&81,&E7,&FF,&FF,&FF,&FF,&FF,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&3C,&66,&42,&42,&66,&3C,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&C3,&99,&BD,&BD,&99,&C3,&FF,&FF,&FF,&FF,&00,&00 ;  ; 10 
 = &00,&00,&60,&7C,&F0,&18,&3E,&63,&63,&63,&3E,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&7E,&C3,&C3,&C3,&7E,&18,&FF,&18,&18,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&FC,&CC,&FC,&0C,&0C,&0C,&0E,&0F,&07,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FC,&CC,&FC,&CC,&CC,&CC,&CC,&E6,&67,&03,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&C3,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0E,&1E,&3E,&1E,&0E,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&80,&C0,&E0,&F0,&F8,&F0,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&66,&66,&66,&66,&66,&66,&00,&66,&66,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&FC,&DF,&DF,&DF,&DE,&D8,&D8,&D8,&D8,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&7C,&C6,&0C,&18,&6C,&C6,&C6,&CC,&70,&60,&C6,&7C,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&FE,&00,&FE,&00,&FE,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&18,&18,&18,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&30,&60,&FF,&60,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&0C,&06,&FF,&06,&0C,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&06,&06,&06,&FE,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&07,&06,&06,&C6,&66,&37,&58,&6C,&76,&FB,&60,&60,&00,&00,&00 ;  ; 30 
 = &00,&07,&06,&06,&C6,&66,&3F,&6C,&D6,&C3,&60,&30,&F8,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&18,&3C,&3C,&3C,&18,&18,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&00,&E7,&E7,&C6,&C6,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&6C,&6C,&FE,&6C,&6C,&6C,&FE,&6C,&6C,&00,&00,&00,&00,&00 ;  ; 35 #
 = &30,&30,&7C,&C6,&C6,&06,&7C,&C0,&C6,&C6,&7C,&30,&30,&00,&00,&00 ;  ; 36 $
 = &00,&00,&00,&86,&CF,&66,&30,&18,&6C,&F6,&63,&01,&00,&00,&00,&00 ;  ; 37 %
 = &00,&00,&1C,&36,&1E,&0C,&0C,&DE,&B6,&66,&DC,&00,&00,&00,&00,&00 ;  ; 38 &
 = &00,&00,&1C,&1C,&18,&18,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&60,&18,&0C,&0C,&0C,&0C,&0C,&18,&60,&00,&00,&00,&00,&00 ;  ; 40 (
 = &00,&00,&0C,&30,&60,&60,&60,&60,&60,&30,&0C,&00,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&66,&3C,&FF,&3C,&66,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&18,&18,&18,&FF,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&1C,&1C,&0C,&06,&00,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&FE,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&80,&C0,&60,&30,&18,&0C,&06,&03,&01,&00,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&38,&6C,&C6,&C6,&C6,&C6,&C6,&6C,&38,&00,&00,&00,&00,&00 ;  ; 48 0
 = &00,&00,&3C,&30,&30,&30,&30,&30,&30,&30,&FC,&00,&00,&00,&00,&00 ;  ; 49 1
 = &00,&00,&7C,&C6,&C0,&60,&30,&18,&0C,&C6,&FE,&00,&00,&00,&00,&00 ;  ; 50 2
 = &00,&00,&7C,&C6,&C0,&C0,&78,&C0,&C0,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 51 3
 = &00,&00,&60,&70,&78,&6C,&66,&FE,&60,&60,&F0,&00,&00,&00,&00,&00 ;  ; 52 4
 = &00,&00,&FE,&06,&06,&06,&7E,&C0,&C0,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 53 5
 = &00,&00,&78,&0C,&06,&06,&7E,&C6,&C6,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 54 6
 = &00,&00,&FE,&C6,&C0,&60,&30,&18,&0C,&06,&06,&00,&00,&00,&00,&00 ;  ; 55 7
 = &00,&00,&7C,&C6,&C6,&C6,&7C,&C6,&C6,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 56 8
 = &00,&00,&7C,&C6,&C6,&C6,&FC,&C0,&C0,&60,&3C,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&00,&18,&18,&00,&00,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&00,&18,&18,&00,&00,&18,&18,&0C,&00,&00,&00,&00,&00 ;  ; 59 ;
 = &00,&00,&60,&30,&18,&0C,&06,&0C,&18,&30,&60,&00,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&00,&7E,&00,&00,&7E,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&00,&06,&0C,&18,&30,&60,&30,&18,&0C,&06,&00,&00,&00,&00,&00 ;  ; 62 >
 = &00,&00,&7C,&C6,&C6,&60,&30,&30,&00,&30,&30,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&00,&7C,&C6,&C6,&F6,&F6,&F6,&06,&06,&7C,&00,&00,&00,&00,&00 ;  ; 64 @
 = &00,&00,&1E,&3C,&66,&66,&66,&7E,&66,&66,&E7,&00,&00,&00,&00,&00 ;  ; 65 A
 = &00,&00,&7E,&CC,&CC,&CC,&7C,&CC,&CC,&CC,&7E,&00,&00,&00,&00,&00 ;  ; 66 B
 = &00,&00,&F8,&CC,&C6,&06,&06,&06,&06,&CC,&78,&00,&00,&00,&00,&00 ;  ; 67 C
 = &00,&00,&7E,&CC,&CC,&CC,&CC,&CC,&CC,&CC,&7E,&00,&00,&00,&00,&00 ;  ; 68 D
 = &00,&00,&FE,&CC,&0C,&6C,&7C,&6C,&0C,&CC,&FE,&00,&00,&00,&00,&00 ;  ; 69 E
 = &00,&00,&FE,&CC,&0C,&6C,&7C,&6C,&0C,&0C,&1E,&00,&00,&00,&00,&00 ;  ; 70 F
 = &00,&00,&F8,&CC,&C6,&06,&06,&F6,&C6,&CC,&78,&00,&00,&00,&00,&00 ;  ; 71 G
 = &00,&00,&E7,&66,&66,&66,&7E,&66,&66,&66,&E7,&00,&00,&00,&00,&00 ;  ; 72 H
 = &00,&00,&7E,&18,&18,&18,&18,&18,&18,&18,&7E,&00,&00,&00,&00,&00 ;  ; 73 I
 = &00,&00,&F8,&60,&60,&60,&60,&60,&66,&66,&3C,&00,&00,&00,&00,&00 ;  ; 74 J
 = &00,&00,&EF,&66,&36,&1E,&0E,&1E,&36,&66,&EF,&00,&00,&00,&00,&00 ;  ; 75 K
 = &00,&00,&1E,&0C,&0C,&0C,&0C,&0C,&CC,&CC,&FE,&00,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&C3,&E7,&FF,&DB,&DB,&C3,&C3,&C3,&E7,&00,&00,&00,&00,&00 ;  ; 77 M
 = &00,&00,&E7,&66,&6E,&7E,&7E,&7E,&76,&66,&67,&00,&00,&00,&00,&00 ;  ; 78 N
 = &00,&00,&7C,&C6,&C6,&C6,&C6,&C6,&C6,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 79 O
 = &00,&00,&7E,&CC,&CC,&CC,&7C,&0C,&0C,&0C,&1E,&00,&00,&00,&00,&00 ;  ; 80 P
 = &00,&00,&38,&6C,&C6,&C6,&C6,&C6,&C6,&6C,&38,&30,&DC,&00,&00,&00 ;  ; 81 Q
 = &00,&00,&3F,&66,&66,&66,&3E,&36,&36,&66,&EF,&00,&00,&00,&00,&00 ;  ; 82 R
 = &00,&00,&DC,&C6,&C6,&0C,&38,&60,&C6,&C6,&76,&00,&00,&00,&00,&00 ;  ; 83 S
 = &00,&00,&FF,&DB,&DB,&18,&18,&18,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 84 T
 = &00,&00,&E7,&C3,&C3,&C3,&C3,&C3,&C3,&C3,&7E,&00,&00,&00,&00,&00 ;  ; 85 U
 = &00,&00,&E7,&66,&66,&66,&66,&66,&66,&3C,&18,&00,&00,&00,&00,&00 ;  ; 86 V
 = &00,&00,&E7,&C3,&C3,&C3,&C3,&DB,&DB,&FF,&E7,&00,&00,&00,&00,&00 ;  ; 87 W
 = &00,&00,&E7,&C3,&66,&3C,&18,&3C,&66,&C3,&E7,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&00,&E7,&66,&66,&3C,&18,&18,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 89 Y
 = &00,&00,&FE,&C6,&66,&30,&18,&0C,&C6,&C3,&FF,&00,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&00,&3C,&0C,&0C,&0C,&0C,&0C,&0C,&0C,&3C,&00,&00,&00,&00,&00 ;  ; 91 [
 = &00,&00,&01,&03,&06,&0C,&18,&30,&60,&C0,&80,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&3C,&30,&30,&30,&30,&30,&30,&30,&3C,&00,&00,&00,&00,&00 ;  ; 93 ]
 = &00,&10,&38,&6C,&C6,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00 ;  ; 95 _
 = &00,&18,&18,&30,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&7C,&60,&7C,&66,&66,&DC,&00,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&07,&06,&06,&76,&CE,&C6,&C6,&CE,&77,&00,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&7C,&C6,&06,&06,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 99 c
 = &00,&00,&E0,&C0,&C0,&FC,&C6,&C6,&C6,&E6,&DC,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&3C,&66,&7E,&06,&66,&3C,&00,&00,&00,&00,&00 ;  ; 101 e
 = &00,&00,&78,&0C,&0C,&0C,&3E,&0C,&0C,&0C,&3E,&00,&00,&00,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&00,&FE,&63,&63,&63,&7E,&60,&60,&3E,&00,&00,&00 ;  ; 103 g
 = &00,&00,&07,&06,&06,&3E,&66,&66,&66,&66,&E7,&00,&00,&00,&00,&00 ;  ; 104 h
 = &00,&00,&30,&30,&00,&3C,&30,&30,&30,&30,&FC,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&30,&30,&00,&3C,&30,&30,&30,&30,&30,&30,&1E,&00,&00,&00 ;  ; 106 j
 = &00,&00,&07,&06,&06,&E6,&36,&1E,&36,&66,&EF,&00,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&3C,&30,&30,&30,&30,&30,&30,&30,&FC,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&E7,&FF,&DB,&C3,&C3,&E7,&00,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&37,&6E,&66,&66,&66,&EF,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&7C,&C6,&C6,&C6,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&77,&CE,&C6,&C6,&7E,&06,&06,&1F,&00,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&00,&DE,&63,&63,&63,&7E,&60,&60,&F8,&00,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&00,&77,&6E,&06,&06,&06,&1F,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&7C,&C6,&1C,&70,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&0C,&0C,&0C,&7F,&0C,&0C,&0C,&CC,&78,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&77,&66,&66,&66,&66,&DC,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&00,&E7,&66,&66,&66,&3C,&18,&00,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&E7,&C3,&DB,&FF,&E7,&E7,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&00,&E7,&66,&3C,&3C,&66,&E7,&00,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&00,&E7,&66,&66,&66,&3C,&18,&0C,&1F,&00,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&00,&FE,&66,&30,&18,&CC,&FE,&00,&00,&00,&00,&00 ;  ; 122 z
 = &00,&00,&70,&18,&18,&18,&0E,&18,&18,&18,&70,&00,&00,&00,&00,&00 ;  ; 123 {
 = &00,&00,&18,&18,&18,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&00,&0E,&18,&18,&18,&70,&18,&18,&18,&0E,&00,&00,&00,&00,&00 ;  ; 125 }
 = &00,&00,&DC,&76,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&00,&10,&38,&6C,&C6,&C6,&C6,&FE,&00,&00,&00,&00,&00 ;  ; 127 
; FONT: <MEDIEVAL>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7E,&81,&A5,&81,&81,&BD,&99,&81,&7E,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&6C,&FE,&FE,&FE,&FE,&7C,&38,&10,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&7C,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&18,&3C,&3C,&E7,&E7,&E7,&3C,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&E7,&C3,&81,&E7,&FF,&FF,&FF,&FF,&FF,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&3C,&66,&42,&42,&66,&3C,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&C3,&99,&BD,&BD,&99,&C3,&FF,&FF,&FF,&FF,&00,&00 ;  ; 10 
 = &00,&00,&F0,&E0,&B0,&98,&3C,&66,&66,&66,&3C,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&3C,&66,&66,&66,&3C,&18,&7E,&18,&18,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&FC,&CC,&FC,&0C,&0C,&0C,&0E,&0F,&07,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FE,&C6,&FE,&C6,&C6,&C6,&E6,&E7,&67,&06,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&E7,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0E,&3E,&FE,&3E,&0E,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&80,&C0,&E0,&F8,&FE,&F8,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&CC,&CC,&CC,&CC,&CC,&CC,&00,&CC,&CC,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&FE,&DB,&DB,&DB,&DE,&D8,&D8,&D8,&D8,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&7C,&C6,&0C,&38,&6C,&C6,&C6,&6C,&38,&60,&C6,&7C,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&00,&00,&FE,&FE,&FE,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&18,&3C,&7E,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&18,&18,&18,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&30,&60,&FF,&60,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&18,&0C,&FE,&0C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&06,&06,&06,&FE,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&00,&00,&10,&38,&38,&7C,&7C,&FE,&FE,&00,&00,&00,&00,&00,&00 ;  ; 30 
 = &00,&00,&00,&FE,&FE,&7C,&7C,&38,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&18,&3C,&3C,&3C,&18,&18,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&C6,&C6,&C6,&44,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&6C,&6C,&FE,&6C,&6C,&6C,&FE,&6C,&6C,&00,&00,&00,&00,&00 ;  ; 35 #
 = &30,&30,&7C,&C6,&86,&06,&7C,&C0,&C2,&C6,&7C,&30,&30,&00,&00,&00 ;  ; 36 $
 = &00,&00,&00,&00,&86,&C6,&60,&30,&18,&CC,&C6,&00,&00,&00,&00,&00 ;  ; 37 %
 = &00,&00,&38,&6C,&6C,&38,&DC,&76,&66,&66,&DC,&00,&00,&00,&00,&00 ;  ; 38 &
 = &00,&0C,&0C,&0C,&06,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&30,&18,&0C,&0C,&0C,&0C,&0C,&18,&30,&00,&00,&00,&00,&00 ;  ; 40 (
 = &00,&00,&18,&30,&60,&60,&60,&60,&60,&30,&18,&00,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&00,&00,&66,&3C,&FF,&3C,&66,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&18,&18,&18,&FF,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&18,&0C,&00,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&80,&C0,&60,&30,&18,&0C,&06,&03,&01,&00,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&7E,&E3,&C3,&C3,&C3,&C3,&C3,&C7,&7E,&00,&00,&00,&00,&00 ;  ; 48 0
 = &00,&00,&38,&30,&30,&30,&30,&30,&30,&30,&30,&10,&00,&00,&00,&00 ;  ; 49 1
 = &00,&00,&7E,&E3,&C3,&66,&34,&18,&0C,&86,&FF,&00,&00,&00,&00,&00 ;  ; 50 2
 = &00,&00,&7C,&C2,&C0,&60,&3C,&60,&C0,&C0,&C0,&62,&3C,&00,&00,&00 ;  ; 51 3
 = &00,&00,&60,&70,&78,&6C,&66,&FF,&60,&60,&60,&20,&00,&00,&00,&00 ;  ; 52 4
 = &00,&80,&7E,&06,&06,&1E,&32,&60,&C0,&C0,&C0,&62,&3C,&00,&00,&00 ;  ; 53 5
 = &00,&00,&7C,&06,&03,&7B,&C7,&C3,&C3,&66,&3C,&00,&00,&00,&00,&00 ;  ; 54 6
 = &00,&00,&FE,&C6,&C0,&60,&30,&18,&0C,&0C,&0C,&04,&00,&00,&00,&00 ;  ; 55 7
 = &00,&00,&7E,&E3,&C3,&C7,&7E,&E3,&C3,&C7,&7E,&00,&00,&00,&00,&00 ;  ; 56 8
 = &00,&00,&7C,&E6,&C3,&C3,&E3,&DE,&C0,&60,&3E,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&18,&18,&00,&00,&18,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&18,&18,&00,&00,&00,&18,&18,&0C,&00,&00,&00,&00,&00 ;  ; 59 ;
 = &00,&00,&60,&30,&18,&0C,&06,&0C,&18,&30,&60,&00,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&00,&7E,&00,&00,&7E,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&00,&06,&0C,&18,&30,&60,&30,&18,&0C,&06,&00,&00,&00,&00,&00 ;  ; 62 >
 = &00,&00,&7C,&C6,&C6,&60,&30,&30,&00,&30,&30,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&00,&7C,&C6,&C6,&F6,&F6,&F6,&76,&06,&7C,&00,&00,&00,&00,&00 ;  ; 64 @
 = &00,&00,&FE,&F0,&D8,&CC,&C6,&FF,&C3,&C3,&C3,&00,&00,&00,&00,&00 ;  ; 65 A
 = &00,&00,&7B,&C7,&C3,&E2,&7E,&C6,&C2,&E2,&7F,&00,&00,&00,&00,&00 ;  ; 66 B
 = &00,&00,&FE,&C3,&83,&03,&03,&03,&03,&87,&7E,&00,&00,&00,&00,&00 ;  ; 67 C
 = &00,&01,&7F,&C0,&FE,&E3,&C3,&C3,&43,&26,&1C,&00,&00,&00,&00,&00 ;  ; 68 D
 = &00,&00,&FE,&C3,&83,&03,&3F,&03,&03,&86,&7C,&00,&00,&00,&00,&00 ;  ; 69 E
 = &00,&00,&FF,&C6,&86,&06,&3E,&06,&06,&06,&06,&06,&06,&02,&00,&00 ;  ; 70 F
 = &00,&00,&FE,&C3,&83,&03,&03,&F3,&C3,&C6,&FC,&C0,&C0,&40,&00,&00 ;  ; 71 G
 = &00,&00,&07,&03,&7B,&C7,&C3,&C3,&C3,&63,&F3,&00,&00,&00,&00,&00 ;  ; 72 H
 = &00,&00,&3C,&18,&18,&18,&18,&18,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 73 I
 = &00,&00,&3C,&18,&18,&18,&18,&18,&18,&18,&18,&18,&0C,&04,&00,&00 ;  ; 74 J
 = &00,&00,&07,&E6,&C6,&66,&36,&3E,&76,&E6,&C7,&00,&00,&00,&00,&00 ;  ; 75 K
 = &00,&00,&0E,&0C,&0C,&0C,&0C,&0C,&8C,&CC,&FE,&00,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&C7,&EE,&FE,&D6,&C6,&C6,&C6,&C6,&C7,&00,&00,&00,&00,&00 ;  ; 77 M
 = &00,&00,&E7,&8E,&9E,&BE,&F6,&E6,&C6,&86,&87,&00,&00,&00,&00,&00 ;  ; 78 N
 = &00,&00,&7E,&E3,&C3,&C3,&C3,&C3,&43,&26,&1C,&00,&00,&00,&00,&00 ;  ; 79 O
 = &00,&00,&7B,&E7,&C3,&C3,&C3,&C3,&C7,&CF,&7B,&03,&01,&00,&00,&00 ;  ; 80 P
 = &00,&00,&DE,&E7,&C3,&C3,&C3,&C3,&C3,&F3,&DE,&C0,&40,&00,&00,&00 ;  ; 81 Q
 = &00,&00,&77,&CE,&C6,&E6,&3E,&66,&C6,&C6,&C6,&80,&00,&00,&00,&00 ;  ; 82 R
 = &00,&00,&BE,&E7,&C7,&0E,&7C,&E0,&C3,&47,&3D,&00,&00,&00,&00,&00 ;  ; 83 S
 = &00,&00,&FF,&19,&0C,&06,&03,&03,&03,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 84 T
 = &00,&00,&C7,&C6,&C6,&C6,&C6,&C6,&C6,&E6,&DC,&00,&00,&00,&00,&00 ;  ; 85 U
 = &00,&00,&C3,&C3,&C3,&C3,&C3,&C6,&6C,&38,&10,&00,&00,&00,&00,&00 ;  ; 86 V
 = &00,&00,&E7,&C6,&C6,&C6,&C6,&D6,&FE,&EE,&47,&00,&00,&00,&00,&00 ;  ; 87 W
 = &00,&00,&C3,&83,&46,&3C,&18,&3C,&62,&C1,&C3,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&00,&F3,&E3,&C3,&C3,&C6,&FC,&61,&63,&3E,&00,&00,&00,&00,&00 ;  ; 89 Y
 = &00,&00,&FF,&C1,&C0,&70,&3C,&0E,&03,&83,&FF,&00,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&00,&3C,&0C,&0C,&0C,&0C,&0C,&0C,&0C,&3C,&00,&00,&00,&00,&00 ;  ; 91 [
 = &00,&00,&02,&06,&0E,&1C,&38,&70,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&3C,&30,&30,&30,&30,&30,&30,&30,&3C,&00,&00,&00,&00,&00 ;  ; 93 ]
 = &10,&38,&6C,&C6,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00 ;  ; 95 _
 = &18,&18,&30,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&DE,&E3,&C3,&C3,&E7,&DE,&00,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&00,&7B,&C7,&C2,&7E,&C6,&C2,&E2,&7F,&00,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&FE,&C7,&03,&03,&C7,&FE,&00,&00,&00,&00,&00 ;  ; 99 c
 = &00,&00,&3E,&63,&C0,&FE,&E3,&C3,&C3,&C7,&7E,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&7E,&C3,&FF,&03,&87,&7E,&00,&00,&00,&00,&00 ;  ; 101 e
 = &00,&00,&00,&E0,&18,&0C,&0C,&0C,&0C,&8C,&FF,&0C,&0C,&0C,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&00,&7E,&E3,&C3,&C3,&C7,&FE,&E3,&C7,&7E,&00,&00 ;  ; 103 g
 = &00,&00,&03,&03,&03,&7B,&C7,&C3,&C3,&63,&F3,&00,&00,&00,&00,&00 ;  ; 104 h
 = &00,&00,&30,&18,&00,&1C,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&00,&00,&7C,&40,&40,&40,&42,&66,&3E,&00,&00,&00,&00,&00 ;  ; 106 j
 = &00,&00,&06,&06,&C6,&66,&36,&1E,&36,&66,&C6,&00,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&0E,&0C,&0C,&0C,&0C,&0C,&0C,&4C,&7C,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&77,&DB,&DB,&DB,&5B,&DB,&00,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&7B,&C7,&C3,&C3,&63,&F3,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&7E,&E3,&C3,&C3,&C7,&7E,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&7D,&E3,&C3,&C3,&C7,&7F,&03,&03,&03,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&00,&7E,&E3,&C3,&C3,&C7,&7E,&0C,&7E,&C0,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&00,&7B,&C7,&C3,&7F,&33,&E3,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&7E,&83,&7F,&C0,&C1,&7E,&00,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&08,&0C,&3F,&06,&06,&06,&06,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&CF,&C6,&C3,&C3,&E3,&DE,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&00,&6F,&C6,&C3,&C3,&63,&1E,&00,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&DB,&DA,&DB,&DB,&DB,&FC,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&00,&C6,&2C,&18,&38,&64,&C3,&00,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&00,&E3,&C3,&C6,&6C,&78,&30,&30,&19,&0F,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&00,&FF,&71,&38,&1C,&0E,&FF,&80,&00,&00,&00,&00 ;  ; 122 z
 = &00,&00,&70,&18,&18,&18,&0E,&18,&18,&18,&70,&00,&00,&00,&00,&00 ;  ; 123 {
 = &00,&00,&18,&18,&18,&18,&00,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&00,&0E,&18,&18,&18,&38,&18,&18,&18,&0E,&00,&00,&00,&00,&00 ;  ; 125 }
 = &00,&00,&DC,&76,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&00,&00,&10,&38,&6C,&C6,&C6,&C6,&FE,&00,&00,&00,&00 ;  ; 127 
; FONT: <SANSERIF>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7E,&81,&A5,&81,&81,&BD,&99,&81,&7E,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&6C,&FE,&FE,&FE,&FE,&7C,&38,&10,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&7C,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&18,&3C,&3C,&E7,&E7,&E7,&3C,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&E7,&C3,&81,&E7,&FF,&FF,&FF,&FF,&FF,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&3C,&66,&42,&42,&66,&3C,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&C3,&99,&BD,&BD,&99,&C3,&FF,&FF,&FF,&FF,&00,&00 ;  ; 10 
 = &00,&00,&F0,&E0,&B0,&98,&3C,&66,&66,&66,&3C,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&3C,&66,&66,&66,&3C,&18,&7E,&18,&18,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&FC,&CC,&FC,&0C,&0C,&0C,&0E,&0F,&07,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FE,&C6,&FE,&C6,&C6,&C6,&E6,&E7,&67,&06,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&E7,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0E,&3E,&FE,&3E,&0E,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&80,&C0,&E0,&F8,&FE,&F8,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&CC,&CC,&CC,&CC,&CC,&CC,&00,&CC,&CC,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&FE,&DB,&DB,&DB,&DE,&D8,&D8,&D8,&D8,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&7C,&C6,&0C,&38,&6C,&C6,&C6,&6C,&38,&60,&C6,&7C,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&00,&00,&FE,&FE,&FE,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&18,&3C,&7E,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&18,&18,&18,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&30,&60,&FF,&60,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&18,&0C,&FE,&0C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&06,&06,&06,&FE,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&00,&00,&10,&38,&38,&7C,&7C,&FE,&FE,&00,&00,&00,&00,&00,&00 ;  ; 30 
 = &00,&00,&00,&FE,&FE,&7C,&7C,&38,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&18,&3C,&3C,&3C,&18,&18,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&C6,&C6,&C6,&44,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&6C,&6C,&FE,&6C,&6C,&6C,&FE,&6C,&6C,&00,&00,&00,&00,&00 ;  ; 35 #
 = &30,&30,&7C,&C6,&86,&06,&7C,&C0,&C2,&C6,&7C,&30,&30,&00,&00,&00 ;  ; 36 $
 = &00,&00,&00,&00,&86,&C6,&60,&30,&18,&CC,&C6,&00,&00,&00,&00,&00 ;  ; 37 %
 = &00,&00,&38,&6C,&6C,&38,&DC,&76,&66,&66,&DC,&00,&00,&00,&00,&00 ;  ; 38 &
 = &00,&0C,&0C,&0C,&06,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&30,&18,&0C,&0C,&0C,&0C,&0C,&18,&30,&00,&00,&00,&00,&00 ;  ; 40 (
 = &00,&00,&18,&30,&60,&60,&60,&60,&60,&30,&18,&00,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&00,&00,&66,&3C,&FF,&3C,&66,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&18,&18,&18,&FF,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&18,&0C,&00,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&80,&C0,&60,&30,&18,&0C,&06,&03,&01,&00,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&7C,&C6,&E6,&F6,&DE,&CE,&C6,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 48 0
 = &00,&00,&30,&38,&3C,&30,&30,&30,&30,&30,&30,&00,&00,&00,&00,&00 ;  ; 49 1
 = &00,&00,&7E,&C0,&C0,&60,&30,&18,&0C,&06,&FE,&00,&00,&00,&00,&00 ;  ; 50 2
 = &00,&00,&7C,&C0,&C0,&C0,&78,&C0,&C0,&C0,&7C,&00,&00,&00,&00,&00 ;  ; 51 3
 = &00,&00,&60,&70,&78,&6C,&66,&FE,&60,&60,&60,&00,&00,&00,&00,&00 ;  ; 52 4
 = &00,&00,&FE,&06,&06,&06,&7E,&C0,&C0,&C0,&7E,&00,&00,&00,&00,&00 ;  ; 53 5
 = &00,&00,&38,&0C,&06,&06,&7E,&C6,&C6,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 54 6
 = &00,&00,&FE,&C0,&C0,&60,&30,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 55 7
 = &00,&00,&7C,&C6,&C6,&C6,&7C,&C6,&C6,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 56 8
 = &00,&00,&7C,&C6,&C6,&C6,&FC,&C0,&C0,&60,&3C,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&18,&18,&00,&00,&18,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&18,&18,&00,&00,&00,&18,&18,&0C,&00,&00,&00,&00,&00 ;  ; 59 ;
 = &00,&00,&60,&30,&18,&0C,&06,&0C,&18,&30,&60,&00,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&00,&7E,&00,&00,&7E,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&00,&06,&0C,&18,&30,&60,&30,&18,&0C,&06,&00,&00,&00,&00,&00 ;  ; 62 >
 = &00,&00,&7C,&C6,&C6,&60,&30,&30,&00,&30,&30,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&00,&7C,&C6,&C6,&F6,&F6,&F6,&76,&06,&7C,&00,&00,&00,&00,&00 ;  ; 64 @
 = &00,&00,&10,&38,&6C,&C6,&C6,&FE,&C6,&C6,&C6,&00,&00,&00,&00,&00 ;  ; 65 A
 = &00,&00,&7C,&CC,&CC,&CC,&7C,&CC,&CC,&CC,&7C,&00,&00,&00,&00,&00 ;  ; 66 B
 = &00,&00,&78,&CC,&86,&06,&06,&06,&86,&CC,&78,&00,&00,&00,&00,&00 ;  ; 67 C
 = &00,&00,&3C,&6C,&CC,&CC,&CC,&CC,&CC,&6C,&3C,&00,&00,&00,&00,&00 ;  ; 68 D
 = &00,&00,&FC,&0C,&0C,&0C,&7C,&0C,&0C,&0C,&FC,&00,&00,&00,&00,&00 ;  ; 69 E
 = &00,&00,&FC,&0C,&0C,&0C,&7C,&0C,&0C,&0C,&0C,&00,&00,&00,&00,&00 ;  ; 70 F
 = &00,&00,&78,&CC,&06,&06,&06,&F6,&C6,&CC,&78,&00,&00,&00,&00,&00 ;  ; 71 G
 = &00,&00,&C6,&C6,&C6,&C6,&FE,&C6,&C6,&C6,&C6,&00,&00,&00,&00,&00 ;  ; 72 H
 = &00,&00,&18,&18,&18,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 73 I
 = &00,&00,&60,&60,&60,&60,&60,&60,&66,&66,&3C,&00,&00,&00,&00,&00 ;  ; 74 J
 = &00,&00,&CC,&CC,&6C,&6C,&3C,&6C,&6C,&CC,&CC,&00,&00,&00,&00,&00 ;  ; 75 K
 = &00,&00,&0C,&0C,&0C,&0C,&0C,&0C,&0C,&0C,&FC,&00,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&C3,&E7,&FF,&DB,&C3,&C3,&C3,&C3,&C3,&00,&00,&00,&00,&00 ;  ; 77 M
 = &00,&00,&C6,&CE,&DE,&FE,&F6,&E6,&C6,&C6,&C6,&00,&00,&00,&00,&00 ;  ; 78 N
 = &00,&00,&38,&6C,&C6,&C6,&C6,&C6,&C6,&6C,&38,&00,&00,&00,&00,&00 ;  ; 79 O
 = &00,&00,&7C,&CC,&CC,&CC,&7C,&0C,&0C,&0C,&0C,&00,&00,&00,&00,&00 ;  ; 80 P
 = &00,&00,&7C,&C6,&C6,&C6,&C6,&C6,&F6,&7C,&60,&60,&00,&00,&00,&00 ;  ; 81 Q
 = &00,&00,&7C,&CC,&CC,&CC,&7C,&6C,&CC,&CC,&CC,&00,&00,&00,&00,&00 ;  ; 82 R
 = &00,&00,&7C,&C6,&C6,&0C,&38,&60,&C6,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 83 S
 = &00,&00,&FF,&18,&18,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 84 T
 = &00,&00,&C6,&C6,&C6,&C6,&C6,&C6,&C6,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 85 U
 = &00,&00,&C3,&C3,&C3,&C3,&C3,&C3,&66,&3C,&18,&00,&00,&00,&00,&00 ;  ; 86 V
 = &00,&00,&C3,&C3,&C3,&C3,&C3,&DB,&FF,&66,&66,&00,&00,&00,&00,&00 ;  ; 87 W
 = &00,&00,&C3,&C3,&66,&3C,&18,&3C,&66,&C3,&C3,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&00,&C3,&C3,&C3,&66,&3C,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 89 Y
 = &00,&00,&FF,&C0,&60,&30,&18,&0C,&06,&03,&FF,&00,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&00,&3C,&0C,&0C,&0C,&0C,&0C,&0C,&0C,&3C,&00,&00,&00,&00,&00 ;  ; 91 [
 = &00,&00,&02,&06,&0E,&1C,&38,&70,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&3C,&30,&30,&30,&30,&30,&30,&30,&3C,&00,&00,&00,&00,&00 ;  ; 93 ]
 = &10,&38,&6C,&C6,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00 ;  ; 95 _
 = &18,&18,&30,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&3C,&60,&7C,&66,&66,&7C,&00,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&0C,&0C,&0C,&3C,&6C,&CC,&CC,&CC,&76,&00,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&7C,&C6,&06,&06,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 99 c
 = &00,&00,&60,&60,&60,&78,&6C,&66,&66,&66,&DC,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&7C,&C6,&FE,&06,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 101 e
 = &00,&00,&38,&0C,&0C,&0C,&3E,&0C,&0C,&0C,&0C,&00,&00,&00,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&00,&DC,&66,&66,&66,&7C,&60,&60,&3C,&00,&00,&00 ;  ; 103 g
 = &00,&00,&0C,&0C,&0C,&6C,&DC,&CC,&CC,&CC,&CC,&00,&00,&00,&00,&00 ;  ; 104 h
 = &00,&00,&30,&30,&00,&30,&30,&30,&30,&30,&30,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&60,&60,&00,&60,&60,&60,&60,&60,&60,&3C,&00,&00,&00,&00 ;  ; 106 j
 = &00,&00,&0C,&0C,&0C,&CC,&6C,&3C,&6C,&CC,&CC,&00,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&30,&30,&30,&30,&30,&30,&30,&30,&30,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&67,&DB,&DB,&DB,&DB,&DB,&00,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&76,&CC,&CC,&CC,&CC,&CC,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&7C,&C6,&C6,&C6,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&76,&CC,&CC,&CC,&7C,&0C,&0C,&0C,&00,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&00,&DC,&66,&66,&66,&7C,&60,&60,&60,&00,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&00,&76,&CC,&CC,&0C,&0C,&0C,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&7C,&C6,&1C,&70,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&18,&18,&18,&7E,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&66,&66,&66,&66,&66,&DC,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&00,&C3,&C3,&C3,&66,&3C,&18,&00,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&C3,&C3,&DB,&DB,&7E,&66,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&00,&C6,&6C,&38,&38,&6C,&C6,&00,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&00,&C6,&C6,&C6,&C6,&FC,&C0,&60,&30,&00,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&00,&FE,&60,&30,&18,&0C,&FE,&00,&00,&00,&00,&00 ;  ; 122 z
 = &00,&00,&70,&18,&18,&18,&0E,&18,&18,&18,&70,&00,&00,&00,&00,&00 ;  ; 123 {
 = &00,&00,&18,&18,&18,&18,&00,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&00,&0E,&18,&18,&18,&38,&18,&18,&18,&0E,&00,&00,&00,&00,&00 ;  ; 125 }
 = &00,&00,&DC,&76,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&00,&00,&10,&38,&6C,&C6,&C6,&C6,&FE,&00,&00,&00,&00 ;  ; 127 
; FONT: <EVOUTLIN>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7E,&81,&A5,&81,&81,&BD,&99,&81,&7E,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&6C,&FE,&FE,&FE,&FE,&7C,&38,&10,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&7C,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&18,&3C,&3C,&E7,&E7,&E7,&3C,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&E7,&C3,&81,&E7,&FF,&FF,&FF,&FF,&FF,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&3C,&66,&42,&42,&66,&3C,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&C3,&99,&BD,&BD,&99,&C3,&FF,&FF,&FF,&FF,&00,&00 ;  ; 10 
 = &00,&00,&60,&7C,&D0,&08,&3E,&41,&41,&41,&3E,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&3E,&41,&41,&41,&3E,&08,&7F,&08,&08,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&FC,&CC,&FC,&0C,&0C,&0C,&0E,&0F,&07,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FE,&C6,&FE,&C6,&C6,&C6,&E6,&E7,&67,&06,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&E7,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0E,&3E,&FE,&3E,&0E,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&80,&C0,&E0,&F8,&FE,&F8,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&66,&66,&66,&66,&66,&66,&00,&66,&66,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&FE,&DB,&DB,&DB,&DE,&D8,&D8,&D8,&D8,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&7C,&C6,&0C,&38,&6C,&C6,&C6,&6C,&70,&C0,&C6,&7C,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&00,&00,&FF,&FF,&FF,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&18,&18,&18,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&30,&60,&FF,&60,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&0C,&06,&FF,&06,&0C,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&06,&06,&06,&FE,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&00,&00,&00,&08,&1C,&1C,&3E,&3E,&7F,&7F,&00,&00,&00,&00,&00 ;  ; 30 
 = &00,&00,&00,&00,&7F,&7F,&3E,&3E,&1C,&1C,&08,&00,&00,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&18,&3C,&3C,&3C,&18,&18,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&00,&C6,&C6,&C6,&84,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&6C,&6C,&FE,&6C,&6C,&FE,&6C,&6C,&6C,&00,&00,&00,&00,&00 ;  ; 35 #
 = &00,&30,&30,&7C,&C6,&86,&7C,&C0,&C2,&C6,&7C,&30,&30,&00,&00,&00 ;  ; 36 $
 = &00,&00,&00,&00,&07,&67,&30,&18,&0C,&E6,&E3,&00,&00,&00,&00,&00 ;  ; 37 %
 = &00,&00,&1C,&36,&36,&1C,&0E,&DF,&73,&63,&DE,&00,&00,&00,&00,&00 ;  ; 38 &
 = &00,&00,&18,&18,&0C,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&00,&18,&0C,&06,&06,&06,&06,&0C,&18,&00,&00,&00,&00,&00 ;  ; 40 (
 = &00,&00,&00,&18,&30,&60,&60,&60,&60,&30,&18,&00,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&66,&3C,&FF,&3C,&66,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&18,&18,&18,&FF,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&0C,&00,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&FE,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&60,&30,&30,&18,&18,&0C,&0C,&06,&06,&00,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&3C,&66,&B5,&B5,&A5,&AD,&AD,&66,&3C,&00,&00,&00,&00,&00 ;  ; 48 0
 = &00,&00,&3C,&28,&28,&28,&28,&28,&28,&28,&7C,&00,&00,&00,&00,&00 ;  ; 49 1
 = &00,&00,&7E,&A3,&A5,&A2,&50,&14,&0A,&FD,&C3,&00,&00,&00,&00,&00 ;  ; 50 2
 = &00,&00,&3C,&63,&A3,&A0,&BC,&A0,&A3,&63,&3E,&00,&00,&00,&00,&00 ;  ; 51 3
 = &00,&00,&60,&50,&58,&54,&52,&FF,&50,&50,&70,&00,&00,&00,&00,&00 ;  ; 52 4
 = &00,&00,&FF,&3B,&01,&3D,&63,&A0,&A3,&63,&3E,&00,&00,&00,&00,&00 ;  ; 53 5
 = &00,&00,&3C,&66,&66,&05,&75,&AD,&A5,&A5,&7E,&00,&00,&00,&00,&00 ;  ; 54 6
 = &00,&00,&9E,&FF,&91,&50,&48,&24,&12,&12,&1E,&00,&00,&00,&00,&00 ;  ; 55 7
 = &00,&00,&3C,&A5,&A5,&7E,&A5,&A5,&A5,&A5,&7E,&00,&00,&00,&00,&00 ;  ; 56 8
 = &00,&00,&7E,&A5,&A5,&A5,&BC,&A0,&A3,&63,&1E,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&00,&18,&18,&00,&00,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&00,&18,&18,&00,&00,&00,&18,&18,&0C,&00,&00,&00,&00 ;  ; 59 ;
 = &00,&00,&00,&30,&18,&0C,&06,&0C,&18,&30,&00,&00,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&00,&7E,&00,&7E,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&00,&00,&0C,&18,&30,&60,&30,&18,&0C,&00,&00,&00,&00,&00,&00 ;  ; 62 >
 = &00,&00,&7C,&C6,&C6,&60,&30,&30,&00,&30,&30,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&00,&7E,&C3,&C3,&FB,&FB,&7B,&03,&03,&3E,&00,&00,&00,&00,&00 ;  ; 64 @
 = &00,&00,&1C,&14,&14,&3A,&2A,&2A,&45,&7D,&47,&00,&00,&00,&00,&00 ;  ; 65 A
 = &00,&00,&7E,&8A,&8A,&8A,&7A,&8A,&8A,&8A,&7E,&00,&00,&00,&00,&00 ;  ; 66 B
 = &00,&00,&B8,&CC,&8A,&0A,&0A,&0A,&8A,&CC,&78,&00,&00,&00,&00,&00 ;  ; 67 C
 = &00,&00,&3E,&4A,&8A,&8A,&8A,&8A,&8A,&4A,&3E,&00,&00,&00,&00,&00 ;  ; 68 D
 = &00,&00,&FF,&C5,&85,&25,&3D,&25,&85,&C5,&FF,&00,&00,&00,&00,&00 ;  ; 69 E
 = &00,&00,&FF,&C5,&85,&05,&45,&7D,&45,&05,&07,&00,&00,&00,&00,&00 ;  ; 70 F
 = &00,&00,&B8,&CC,&8A,&0A,&0A,&EA,&8A,&CA,&BC,&00,&00,&00,&00,&00 ;  ; 71 G
 = &00,&00,&47,&45,&45,&45,&7D,&45,&45,&45,&47,&00,&00,&00,&00,&00 ;  ; 72 H
 = &00,&00,&1C,&14,&14,&14,&14,&14,&14,&14,&1C,&00,&00,&00,&00,&00 ;  ; 73 I
 = &00,&00,&70,&50,&50,&50,&50,&50,&53,&53,&3E,&00,&00,&00,&00,&00 ;  ; 74 J
 = &00,&00,&87,&45,&25,&15,&0D,&15,&25,&45,&87,&00,&00,&00,&00,&00 ;  ; 75 K
 = &00,&00,&07,&05,&05,&05,&05,&05,&45,&65,&7F,&00,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&DF,&A5,&A5,&A5,&A5,&85,&85,&85,&87,&00,&00,&00,&00,&00 ;  ; 77 M
 = &00,&00,&47,&4D,&4D,&55,&55,&55,&65,&65,&47,&00,&00,&00,&00,&00 ;  ; 78 N
 = &00,&00,&1C,&26,&45,&45,&45,&45,&45,&26,&1C,&00,&00,&00,&00,&00 ;  ; 79 O
 = &00,&00,&7F,&85,&85,&85,&7D,&05,&05,&05,&07,&00,&00,&00,&00,&00 ;  ; 80 P
 = &00,&00,&3E,&45,&85,&85,&85,&85,&9D,&76,&7C,&20,&C0,&00,&00,&00 ;  ; 81 Q
 = &00,&00,&3F,&45,&45,&45,&3D,&0D,&15,&25,&47,&00,&00,&00,&00,&00 ;  ; 82 R
 = &00,&00,&3E,&45,&05,&06,&38,&50,&50,&51,&3E,&00,&00,&00,&00,&00 ;  ; 83 S
 = &00,&00,&FE,&28,&28,&28,&28,&28,&28,&28,&38,&00,&00,&00,&00,&00 ;  ; 84 T
 = &00,&00,&46,&45,&45,&45,&45,&45,&45,&45,&3E,&00,&00,&00,&00,&00 ;  ; 85 U
 = &00,&00,&47,&45,&45,&4A,&4A,&32,&34,&24,&38,&00,&00,&00,&00,&00 ;  ; 86 V
 = &00,&00,&47,&45,&45,&55,&55,&5A,&3A,&2A,&2E,&00,&00,&00,&00,&00 ;  ; 87 W
 = &00,&00,&C7,&65,&3A,&34,&18,&3C,&2C,&52,&E3,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&00,&E7,&C5,&45,&2A,&14,&14,&14,&14,&1C,&00,&00,&00,&00,&00 ;  ; 89 Y
 = &00,&00,&FF,&A3,&51,&28,&14,&14,&8A,&CA,&FF,&00,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&00,&00,&3C,&0C,&0C,&0C,&0C,&0C,&0C,&3C,&00,&00,&00,&00,&00 ;  ; 91 [
 = &00,&00,&00,&02,&06,&0C,&18,&30,&60,&C0,&80,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&00,&3C,&30,&30,&30,&30,&30,&30,&3C,&00,&00,&00,&00,&00 ;  ; 93 ]
 = &00,&00,&10,&38,&6C,&C6,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&FF,&00,&00,&00,&00,&00 ;  ; 95 _
 = &00,&00,&18,&18,&18,&30,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&7E,&A5,&BE,&A5,&A5,&DE,&00,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&07,&05,&05,&35,&6D,&A5,&A5,&65,&3B,&00,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&78,&A6,&65,&05,&C6,&78,&00,&00,&00,&00,&00 ;  ; 99 c
 = &00,&00,&E0,&A0,&A0,&AC,&B6,&A5,&A5,&A6,&DC,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&7C,&A6,&FD,&05,&C6,&78,&00,&00,&00,&00,&00 ;  ; 101 e
 = &00,&00,&7C,&44,&74,&14,&77,&14,&14,&14,&1C,&00,&00,&00,&00,&00 ;  ; 102 f
 = &00,&00,&00,&E0,&A0,&3C,&66,&A5,&A5,&7E,&03,&7F,&83,&FC,&00,&00 ;  ; 103 g
 = &00,&00,&07,&05,&05,&75,&AD,&A5,&A5,&A5,&E7,&00,&00,&00,&00,&00 ;  ; 104 h
 = &00,&00,&38,&38,&00,&38,&28,&28,&28,&28,&38,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&70,&70,&00,&70,&50,&50,&50,&50,&50,&56,&66,&3C,&00,&00 ;  ; 106 j
 = &00,&00,&07,&05,&05,&E5,&35,&1D,&2D,&55,&E7,&00,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&38,&28,&28,&28,&28,&28,&28,&28,&38,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&67,&BD,&91,&A5,&B5,&E7,&00,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&77,&AD,&A5,&A5,&A5,&E7,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&3C,&66,&A5,&A5,&66,&3C,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&37,&6D,&A5,&A5,&6D,&35,&05,&05,&07,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&00,&EC,&B6,&A5,&A5,&B6,&AC,&A0,&A0,&E0,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&00,&7F,&7D,&05,&05,&05,&07,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&7C,&FA,&14,&50,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&10,&18,&14,&77,&14,&14,&14,&94,&78,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&E7,&A5,&A5,&A5,&B5,&EE,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&00,&E7,&C5,&4D,&2A,&1A,&0C,&00,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&E7,&B5,&AD,&AD,&5A,&66,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&00,&87,&4A,&34,&5C,&A2,&C1,&00,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&00,&E7,&CA,&94,&68,&70,&36,&16,&1E,&00,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&00,&FE,&7F,&14,&0A,&FD,&7F,&00,&00,&00,&00,&00 ;  ; 122 z
 = &00,&00,&00,&00,&38,&0C,&0C,&07,&0C,&0C,&38,&00,&00,&00,&00,&00 ;  ; 123 {
 = &00,&00,&00,&00,&18,&18,&18,&00,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&00,&00,&00,&0E,&18,&18,&70,&18,&18,&0E,&00,&00,&00,&00,&00 ;  ; 125 }
 = &00,&00,&DC,&76,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&00,&10,&38,&6C,&C6,&C6,&C6,&FE,&00,&00,&00,&00,&00 ;  ; 127 
; FONT: <SCRIPT>
 = &04,&0A,&11,&00,&00,&70,&88,&A8,&E8,&08,&08,&F0,&00,&00,&00,&00 ;  ; 0 
 = &04,&0A,&11,&00,&00,&20,&50,&88,&F8,&88,&88,&88,&00,&00,&00,&00 ;  ; 1 
 = &04,&0A,&11,&00,&00,&78,&88,&88,&78,&88,&88,&78,&00,&00,&00,&00 ;  ; 2 
 = &04,&0A,&11,&00,&00,&70,&88,&08,&08,&08,&88,&70,&00,&00,&00,&00 ;  ; 3 
 = &04,&0A,&11,&00,&00,&78,&88,&88,&88,&88,&88,&78,&00,&00,&00,&00 ;  ; 4 
 = &04,&0A,&11,&00,&00,&F8,&08,&08,&78,&08,&08,&F8,&00,&00,&00,&00 ;  ; 5 
 = &04,&0A,&11,&00,&00,&F8,&08,&08,&78,&08,&08,&08,&00,&00,&00,&00 ;  ; 6 
 = &04,&0A,&11,&00,&00,&70,&88,&08,&E8,&88,&88,&70,&00,&00,&00,&00 ;  ; 7 
 = &04,&0A,&11,&00,&00,&88,&88,&88,&F8,&88,&88,&88,&00,&00,&00,&00 ;  ; 8 
 = &04,&0A,&11,&00,&00,&70,&20,&20,&20,&20,&20,&70,&00,&00,&00,&00 ;  ; 9 
 = &04,&0A,&11,&00,&00,&E0,&40,&40,&40,&40,&48,&30,&00,&00,&00,&00 ;  ; 10 
 = &04,&0A,&11,&00,&00,&88,&48,&28,&18,&28,&48,&88,&00,&00,&00,&00 ;  ; 11 
 = &04,&0A,&11,&00,&00,&08,&08,&08,&08,&08,&08,&F8,&00,&00,&00,&00 ;  ; 12 
 = &04,&0A,&11,&00,&00,&88,&D8,&A8,&88,&88,&88,&88,&00,&00,&00,&00 ;  ; 13 
 = &04,&0A,&11,&00,&00,&88,&98,&A8,&C8,&88,&88,&88,&00,&00,&00,&00 ;  ; 14 
 = &04,&0A,&11,&00,&00,&70,&88,&88,&88,&88,&88,&70,&00,&00,&00,&00 ;  ; 15 
 = &04,&0A,&11,&00,&00,&78,&88,&88,&78,&08,&08,&08,&00,&00,&00,&00 ;  ; 16 
 = &04,&0A,&11,&00,&00,&70,&88,&88,&88,&A8,&90,&68,&00,&00,&00,&00 ;  ; 17 
 = &04,&0A,&11,&00,&00,&78,&88,&88,&78,&28,&48,&88,&00,&00,&00,&00 ;  ; 18 
 = &04,&0A,&11,&00,&00,&70,&88,&08,&70,&80,&88,&70,&00,&00,&00,&00 ;  ; 19 
 = &04,&0A,&11,&00,&00,&F8,&20,&20,&20,&20,&20,&20,&00,&00,&00,&00 ;  ; 20 
 = &04,&0A,&11,&00,&00,&88,&88,&88,&88,&88,&88,&70,&00,&00,&00,&00 ;  ; 21 
 = &04,&0A,&11,&00,&00,&88,&88,&88,&88,&50,&50,&20,&00,&00,&00,&00 ;  ; 22 
 = &04,&0A,&11,&00,&00,&88,&88,&88,&88,&A8,&D8,&88,&00,&00,&00,&00 ;  ; 23 
 = &04,&0A,&11,&00,&00,&88,&88,&50,&20,&50,&88,&88,&00,&00,&00,&00 ;  ; 24 
 = &04,&0A,&11,&00,&00,&88,&88,&88,&50,&20,&20,&20,&00,&00,&00,&00 ;  ; 25 
 = &04,&0A,&11,&00,&00,&F8,&80,&40,&20,&10,&08,&F8,&00,&00,&00,&00 ;  ; 26 
 = &04,&0A,&11,&00,&00,&70,&10,&10,&10,&10,&10,&70,&00,&00,&00,&00 ;  ; 27 
 = &04,&0A,&11,&00,&00,&08,&08,&10,&20,&40,&80,&80,&00,&00,&00,&00 ;  ; 28 
 = &04,&0A,&11,&00,&00,&70,&40,&40,&40,&40,&40,&70,&00,&00,&00,&00 ;  ; 29 
 = &04,&0A,&11,&00,&00,&20,&70,&A8,&20,&20,&20,&20,&00,&00,&00,&00 ;  ; 30 
 = &04,&0A,&11,&00,&00,&40,&20,&10,&F8,&10,&20,&40,&00,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&08,&08,&08,&08,&08,&08,&08,&00,&00,&08,&08,&00,&00,&00,&00 ;  ; 33 !
 = &00,&24,&24,&24,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&42,&42,&FF,&42,&42,&42,&FF,&42,&42,&00,&00,&00,&00,&00 ;  ; 35 #
 = &00,&08,&7E,&09,&09,&09,&3E,&48,&48,&48,&3F,&08,&00,&00,&00,&00 ;  ; 36 $
 = &00,&86,&89,&C9,&66,&30,&18,&0C,&66,&93,&91,&61,&00,&00,&00,&00 ;  ; 37 %
 = &00,&0E,&11,&11,&11,&0A,&06,&89,&51,&21,&52,&8C,&00,&00,&00,&00 ;  ; 38 &
 = &00,&18,&18,&08,&04,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&60,&18,&04,&02,&02,&02,&02,&02,&04,&18,&60,&00,&00,&00,&00 ;  ; 40 (
 = &00,&06,&18,&20,&40,&40,&40,&40,&40,&20,&18,&06,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&00,&08,&2A,&1C,&7F,&1C,&2A,&08,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&08,&08,&08,&7F,&08,&08,&08,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&08,&04,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&7F,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&00 ;  ; 46 .
 = &00,&80,&80,&C0,&60,&30,&18,&0C,&06,&03,&01,&01,&00,&00,&00,&00 ;  ; 47 /
 = &00,&7C,&82,&C1,&A1,&91,&89,&85,&83,&81,&41,&3E,&00,&00,&00,&00 ;  ; 48 0
 = &00,&10,&18,&10,&10,&10,&10,&10,&10,&10,&10,&38,&00,&00,&00,&00 ;  ; 49 1
 = &00,&7C,&82,&81,&80,&40,&20,&10,&08,&04,&3E,&C3,&00,&00,&00,&00 ;  ; 50 2
 = &00,&7C,&82,&81,&80,&40,&3C,&40,&80,&80,&41,&3E,&00,&00,&00,&00 ;  ; 51 3
 = &00,&84,&84,&42,&42,&42,&21,&FF,&20,&10,&10,&10,&00,&00,&00,&00 ;  ; 52 4
 = &00,&FC,&02,&02,&01,&3F,&40,&80,&80,&81,&42,&3C,&00,&00,&00,&00 ;  ; 53 5
 = &00,&F0,&08,&04,&02,&7A,&85,&83,&81,&81,&41,&3E,&00,&00,&00,&00 ;  ; 54 6
 = &00,&FE,&81,&40,&40,&20,&20,&10,&10,&08,&08,&04,&00,&00,&00,&00 ;  ; 55 7
 = &00,&7C,&82,&81,&81,&41,&7E,&82,&81,&81,&41,&3E,&00,&00,&00,&00 ;  ; 56 8
 = &00,&7C,&82,&81,&81,&E1,&5E,&40,&20,&20,&10,&10,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&18,&18,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&18,&18,&08,&04,&00,&00 ;  ; 59 ;
 = &00,&40,&20,&10,&08,&04,&02,&04,&08,&10,&20,&40,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&7F,&00,&00,&00,&7F,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&02,&04,&08,&10,&20,&40,&20,&10,&08,&04,&02,&00,&00,&00,&00 ;  ; 62 >
 = &00,&3C,&42,&41,&40,&20,&10,&08,&08,&00,&08,&08,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&78,&84,&82,&82,&B1,&A9,&A9,&79,&01,&01,&7E,&00,&00,&00,&00 ;  ; 64 @
 = &00,&78,&44,&42,&42,&41,&41,&41,&41,&61,&D1,&4E,&00,&00,&00,&00 ;  ; 65 A
 = &00,&7A,&86,&82,&83,&42,&3A,&42,&83,&9E,&82,&7D,&00,&00,&00,&00 ;  ; 66 B
 = &00,&9C,&62,&91,&61,&01,&01,&01,&01,&01,&82,&7C,&00,&00,&00,&00 ;  ; 67 C
 = &00,&3E,&46,&BA,&83,&82,&82,&82,&82,&82,&C2,&3D,&00,&00,&00,&00 ;  ; 68 D
 = &00,&7C,&A2,&61,&01,&02,&3C,&02,&01,&01,&82,&7C,&00,&00,&00,&00 ;  ; 69 E
 = &00,&FF,&43,&21,&20,&7C,&20,&20,&21,&2F,&E1,&1E,&00,&00,&00,&00 ;  ; 70 F
 = &00,&08,&94,&94,&C8,&B8,&88,&84,&83,&9E,&83,&7D,&00,&00,&00,&00 ;  ; 71 G
 = &00,&41,&42,&42,&42,&FE,&42,&42,&42,&42,&82,&01,&00,&00,&00,&00 ;  ; 72 H
 = &00,&10,&28,&28,&28,&28,&10,&11,&37,&51,&91,&0E,&00,&00,&00,&00 ;  ; 73 I
 = &00,&38,&44,&42,&42,&44,&C8,&70,&20,&70,&A8,&24,&14,&08,&00,&00 ;  ; 74 J
 = &00,&41,&22,&12,&0A,&06,&06,&06,&0A,&12,&A2,&41,&00,&00,&00,&00 ;  ; 75 K
 = &00,&61,&92,&7C,&10,&10,&10,&10,&10,&16,&89,&76,&00,&00,&00,&00 ;  ; 76 L
 = &00,&6D,&92,&92,&92,&92,&92,&92,&92,&92,&92,&12,&00,&00,&00,&00 ;  ; 77 M
 = &00,&3D,&42,&42,&42,&42,&42,&42,&42,&42,&82,&02,&00,&00,&00,&00 ;  ; 78 N
 = &00,&3C,&46,&B9,&81,&81,&81,&81,&81,&81,&42,&3C,&00,&00,&00,&00 ;  ; 79 O
 = &00,&7D,&82,&82,&82,&42,&3E,&02,&02,&02,&02,&02,&00,&00,&00,&00 ;  ; 80 P
 = &00,&3C,&52,&92,&8C,&80,&80,&40,&20,&1E,&99,&66,&00,&00,&00,&00 ;  ; 81 Q
 = &00,&7D,&82,&82,&82,&42,&3E,&0A,&12,&22,&C2,&82,&00,&00,&00,&00 ;  ; 82 R
 = &00,&10,&28,&28,&10,&30,&48,&85,&9F,&81,&82,&7C,&00,&00,&00,&00 ;  ; 83 S
 = &00,&FF,&43,&21,&20,&20,&20,&20,&21,&2F,&21,&1E,&00,&00,&00,&00 ;  ; 84 T
 = &00,&83,&83,&82,&82,&82,&82,&82,&82,&82,&C2,&3C,&00,&00,&00,&00 ;  ; 85 U
 = &00,&83,&43,&42,&22,&22,&12,&12,&0A,&0A,&06,&06,&00,&00,&00,&00 ;  ; 86 V
 = &00,&93,&9B,&99,&59,&55,&55,&35,&33,&33,&13,&11,&00,&00,&00,&00 ;  ; 87 W
 = &00,&C6,&25,&2E,&18,&18,&18,&18,&18,&74,&A4,&63,&00,&00,&00,&00 ;  ; 88 X
 = &00,&43,&43,&42,&42,&42,&62,&52,&4C,&C0,&40,&60,&50,&30,&00,&00 ;  ; 89 Y
 = &00,&3C,&52,&8C,&80,&80,&58,&24,&58,&40,&C0,&60,&50,&30,&00,&00 ;  ; 90 Z
 = &00,&7E,&02,&02,&02,&02,&02,&02,&02,&02,&02,&7E,&00,&00,&00,&00 ;  ; 91 [
 = &00,&01,&01,&03,&06,&0C,&18,&30,&60,&C0,&80,&80,&00,&00,&00,&00 ;  ; 92 \
 = &00,&7E,&40,&40,&40,&40,&40,&40,&40,&40,&40,&7E,&00,&00,&00,&00 ;  ; 93 ]
 = &00,&18,&24,&42,&81,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00 ;  ; 95 _
 = &00,&10,&20,&40,&80,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&00,&3C,&42,&41,&41,&E1,&5E,&00,&00,&00,&00 ;  ; 97 a
 = &00,&18,&14,&14,&14,&14,&0C,&24,&66,&A5,&24,&18,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&00,&5C,&22,&21,&01,&81,&7E,&00,&00,&00,&00 ;  ; 99 c
 = &00,&40,&40,&40,&40,&40,&5C,&62,&41,&41,&E1,&5E,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&00,&3C,&42,&31,&0D,&83,&7E,&00,&00,&00,&00 ;  ; 101 e
 = &00,&18,&28,&28,&28,&18,&08,&0C,&1A,&25,&E4,&24,&24,&18,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&00,&00,&3C,&42,&41,&61,&DE,&48,&24,&1C,&00,&00 ;  ; 103 g
 = &00,&04,&0A,&0A,&06,&02,&3A,&46,&42,&43,&C2,&42,&00,&00,&00,&00 ;  ; 104 h
 = &00,&00,&00,&00,&0C,&0C,&00,&0C,&0C,&0B,&88,&70,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&00,&00,&18,&18,&00,&18,&1C,&13,&F8,&14,&12,&0E,&00,&00 ;  ; 106 j
 = &00,&08,&14,&14,&0C,&04,&1C,&22,&12,&0F,&92,&62,&00,&00,&00,&00 ;  ; 107 k
 = &00,&18,&14,&14,&14,&0C,&04,&04,&06,&05,&84,&78,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&00,&35,&4B,&49,&49,&C9,&49,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&00,&3A,&46,&42,&43,&C2,&42,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&00,&3C,&52,&A1,&C1,&41,&3E,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&00,&7A,&86,&82,&83,&C6,&3B,&03,&03,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&00,&00,&3C,&42,&41,&41,&E1,&3E,&20,&30,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&00,&02,&1E,&22,&22,&21,&A0,&40,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&08,&18,&24,&42,&41,&C4,&38,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&08,&08,&08,&3E,&08,&0C,&0A,&09,&88,&70,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&00,&42,&42,&42,&43,&E2,&5C,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&00,&00,&42,&62,&52,&4B,&86,&02,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&00,&49,&6D,&6D,&5B,&DB,&09,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&00,&00,&46,&29,&11,&28,&A4,&42,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&00,&00,&41,&41,&41,&61,&FE,&48,&24,&1C,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&00,&00,&3E,&41,&38,&40,&F0,&48,&24,&1C,&00,&00 ;  ; 122 z
 = &00,&20,&10,&08,&08,&08,&04,&08,&08,&08,&10,&20,&00,&00,&00,&00 ;  ; 123 {
 = &00,&08,&08,&08,&08,&00,&00,&00,&08,&08,&08,&08,&00,&00,&00,&00 ;  ; 124 |
 = &00,&06,&08,&08,&08,&08,&10,&08,&08,&08,&08,&06,&00,&00,&00,&00 ;  ; 125 }
 = &00,&0E,&99,&70,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &33,&33,&CC,&CC,&33,&33,&CC,&CC,&33,&33,&CC,&CC,&00,&00,&00,&00 ;  ; 127 
; FONT: <HOLLOW>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7F,&81,&A5,&81,&81,&BD,&99,&81,&7E,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&6C,&FE,&FE,&FE,&FE,&7C,&38,&10,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&7C,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&18,&3C,&3C,&E7,&E7,&E7,&3C,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&E7,&C3,&81,&E7,&FF,&FF,&FF,&FF,&FF,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&3C,&66,&42,&42,&66,&3C,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&C3,&99,&BD,&BD,&99,&C3,&FF,&FF,&FF,&FF,&00,&00 ;  ; 10 
 = &00,&00,&F0,&E0,&B0,&98,&3C,&66,&66,&66,&3C,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&3C,&66,&66,&66,&3C,&18,&7E,&18,&18,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&FC,&CC,&FC,&0C,&0C,&0C,&0E,&0F,&07,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FE,&C6,&FE,&C6,&C6,&C6,&E6,&E7,&67,&06,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&E7,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0E,&3E,&FE,&3E,&0E,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&80,&C0,&E0,&F8,&FE,&F8,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&CC,&CC,&CC,&CC,&CC,&CC,&00,&CC,&CC,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&FE,&DB,&DB,&DB,&DE,&D8,&D8,&D8,&D8,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&7C,&C6,&0C,&38,&6C,&C6,&C6,&6C,&38,&60,&C6,&7C,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&00,&00,&FE,&FE,&FE,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&18,&3C,&7E,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&18,&18,&18,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&30,&60,&FF,&60,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&18,&0C,&FE,&0C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&06,&06,&06,&FE,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&00,&00,&10,&38,&38,&7C,&7C,&FE,&FE,&00,&00,&00,&00,&00,&00 ;  ; 30 
 = &00,&00,&00,&FE,&FE,&7C,&7C,&38,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&18,&3C,&3C,&3C,&18,&18,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&C6,&C6,&C6,&44,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&6C,&6C,&FE,&6C,&6C,&6C,&FE,&6C,&6C,&00,&00,&00,&00,&00 ;  ; 35 #
 = &30,&30,&7C,&C6,&86,&06,&7C,&C0,&C2,&C6,&7C,&30,&30,&00,&00,&00 ;  ; 36 $
 = &00,&00,&00,&00,&86,&C6,&60,&30,&18,&CC,&C6,&00,&00,&00,&00,&00 ;  ; 37 %
 = &00,&00,&38,&6C,&6C,&38,&DC,&76,&66,&66,&DC,&00,&00,&00,&00,&00 ;  ; 38 &
 = &00,&0C,&0C,&0C,&06,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&30,&18,&0C,&0C,&0C,&0C,&0C,&18,&30,&00,&00,&00,&00,&00 ;  ; 40 (
 = &00,&00,&18,&30,&60,&60,&60,&60,&60,&30,&18,&00,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&00,&00,&66,&3C,&FF,&3C,&66,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&18,&18,&18,&FF,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&18,&0C,&00,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&80,&C0,&60,&30,&18,&0C,&06,&03,&01,&00,&00,&00,&00,&00 ;  ; 47 /
 = &3C,&66,&A5,&A5,&B5,&B5,&AD,&AD,&A5,&66,&3C,&00,&00,&00,&00,&00 ;  ; 48 0
 = &3C,&28,&28,&28,&28,&28,&28,&28,&28,&28,&7C,&00,&00,&00,&00,&00 ;  ; 49 1
 = &7E,&A3,&A5,&A5,&A2,&D0,&68,&14,&2A,&FD,&C3,&00,&00,&00,&00,&00 ;  ; 50 2
 = &3E,&63,&A3,&A0,&A0,&BC,&A0,&A0,&A3,&63,&3E,&00,&00,&00,&00,&00 ;  ; 51 3
 = &60,&50,&58,&54,&52,&51,&DF,&50,&50,&50,&70,&00,&00,&00,&00,&00 ;  ; 52 4
 = &FF,&43,&3D,&01,&3D,&63,&A0,&A0,&A3,&63,&3E,&00,&00,&00,&00,&00 ;  ; 53 5
 = &3C,&66,&66,&05,&75,&AD,&A5,&A5,&A5,&A5,&7E,&00,&00,&00,&00,&00 ;  ; 54 6
 = &9E,&FF,&91,&50,&50,&28,&28,&14,&14,&16,&1C,&00,&00,&00,&00,&00 ;  ; 55 7
 = &7E,&A5,&A5,&A5,&7E,&A5,&A5,&A5,&A5,&A5,&7E,&00,&00,&00,&00,&00 ;  ; 56 8
 = &7E,&A5,&A5,&A5,&A5,&BE,&A0,&A0,&A3,&63,&3E,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&18,&18,&00,&00,&18,&18,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&18,&18,&00,&00,&00,&18,&18,&0C,&00,&00,&00,&00,&00,&00,&00 ;  ; 59 ;
 = &60,&30,&18,&0C,&06,&0C,&18,&30,&60,&00,&00,&00,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&7E,&00,&00,&7E,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &06,&0C,&18,&30,&60,&30,&18,&0C,&06,&00,&00,&00,&00,&00,&00,&00 ;  ; 62 >
 = &7C,&C6,&C6,&60,&30,&30,&00,&30,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &7C,&C6,&C6,&F6,&F6,&F6,&76,&06,&7C,&00,&00,&00,&00,&00,&00,&00 ;  ; 64 @
 = &1C,&14,&14,&14,&3A,&2A,&2A,&45,&45,&7D,&47,&00,&00,&00,&00,&00 ;  ; 65 A
 = &1F,&25,&45,&45,&45,&3D,&45,&45,&45,&45,&3F,&00,&00,&00,&00,&00 ;  ; 66 B
 = &58,&66,&45,&05,&05,&05,&05,&05,&45,&66,&1C,&00,&00,&00,&00,&00 ;  ; 67 C
 = &1F,&25,&45,&45,&45,&45,&45,&45,&45,&25,&1F,&00,&00,&00,&00,&00 ;  ; 68 D
 = &FF,&C5,&85,&05,&45,&7D,&45,&05,&85,&C5,&FF,&00,&00,&00,&00,&00 ;  ; 69 E
 = &FF,&C5,&85,&05,&45,&65,&7D,&65,&45,&05,&07,&00,&00,&00,&00,&00 ;  ; 70 F
 = &B8,&CC,&8A,&0A,&0A,&0A,&EA,&CA,&8A,&CA,&7C,&00,&00,&00,&00,&00 ;  ; 71 G
 = &8E,&8A,&8A,&8A,&8A,&8A,&FA,&8A,&8A,&8A,&8E,&00,&00,&00,&00,&00 ;  ; 72 H
 = &38,&28,&28,&28,&28,&28,&28,&28,&28,&28,&38,&00,&00,&00,&00,&00 ;  ; 73 I
 = &70,&50,&50,&50,&50,&50,&50,&50,&53,&57,&3E,&00,&00,&00,&00,&00 ;  ; 74 J
 = &87,&45,&25,&15,&0D,&05,&0D,&15,&25,&45,&87,&00,&00,&00,&00,&00 ;  ; 75 K
 = &07,&05,&05,&05,&05,&05,&05,&05,&45,&65,&7F,&00,&00,&00,&00,&00 ;  ; 76 L
 = &6F,&55,&55,&55,&55,&55,&45,&45,&45,&45,&47,&00,&00,&00,&00,&00 ;  ; 77 M
 = &47,&4D,&4D,&4D,&55,&55,&55,&65,&65,&65,&47,&00,&00,&00,&00,&00 ;  ; 78 N
 = &38,&4C,&4A,&8A,&8A,&8A,&8A,&8A,&4A,&4C,&38,&00,&00,&00,&00,&00 ;  ; 79 O
 = &3F,&45,&45,&45,&45,&3D,&05,&05,&05,&05,&07,&00,&00,&00,&00,&00 ;  ; 80 P
 = &3C,&4A,&4A,&8A,&8A,&8A,&8A,&BA,&6A,&6C,&78,&20,&C0,&00,&00,&00 ;  ; 81 Q
 = &3F,&45,&45,&45,&45,&3D,&0D,&15,&25,&25,&47,&00,&00,&00,&00,&00 ;  ; 82 R
 = &3C,&46,&05,&05,&06,&38,&50,&50,&50,&51,&3E,&00,&00,&00,&00,&00 ;  ; 83 S
 = &7F,&14,&14,&14,&14,&14,&14,&14,&14,&14,&1C,&00,&00,&00,&00,&00 ;  ; 84 T
 = &46,&45,&45,&45,&45,&45,&45,&45,&45,&66,&3C,&00,&00,&00,&00,&00 ;  ; 85 U
 = &47,&45,&45,&4A,&4A,&4A,&34,&34,&34,&28,&38,&00,&00,&00,&00,&00 ;  ; 86 V
 = &47,&45,&45,&45,&55,&55,&5B,&3A,&2A,&2A,&2E,&00,&00,&00,&00,&00 ;  ; 87 W
 = &C7,&45,&6A,&32,&34,&34,&28,&4C,&56,&A2,&E3,&00,&00,&00,&00,&00 ;  ; 88 X
 = &E7,&C5,&85,&4B,&2A,&12,&14,&14,&14,&14,&1C,&00,&00,&00,&00,&00 ;  ; 89 Y
 = &FF,&A3,&51,&50,&28,&28,&14,&14,&8A,&CA,&FF,&00,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&00,&3C,&0C,&0C,&0C,&0C,&0C,&0C,&0C,&3C,&00,&00,&00,&00,&00 ;  ; 91 [
 = &00,&00,&02,&06,&0E,&1C,&38,&70,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&3C,&30,&30,&30,&30,&30,&30,&30,&3C,&00,&00,&00,&00,&00 ;  ; 93 ]
 = &10,&38,&6C,&C6,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 95 _
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&7E,&A5,&A7,&BC,&A6,&A5,&A5,&DE,&00,&00,&00,&00,&00 ;  ; 97 a
 = &07,&05,&05,&35,&6D,&A5,&A5,&A5,&A5,&65,&3B,&00,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&78,&A6,&B5,&65,&05,&85,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 99 c
 = &E0,&A0,&A0,&AC,&B6,&A5,&A5,&A5,&A5,&A5,&DE,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&3C,&66,&A5,&A5,&FD,&05,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 101 e
 = &7C,&44,&74,&14,&77,&14,&14,&14,&14,&14,&1C,&00,&00,&00,&00,&00 ;  ; 102 f
 = &00,&E0,&A0,&3C,&66,&A5,&A5,&A5,&7E,&03,&3F,&41,&43,&7C,&00,&00 ;  ; 103 g
 = &07,&05,&05,&75,&AD,&A5,&A5,&A5,&A5,&A5,&E7,&00,&00,&00,&00,&00 ;  ; 104 h
 = &00,&38,&38,&00,&00,&38,&28,&28,&28,&28,&38,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&70,&70,&00,&00,&70,&50,&50,&50,&50,&50,&56,&66,&3C,&00,&00 ;  ; 106 j
 = &07,&05,&05,&E5,&C5,&65,&35,&2D,&6D,&D5,&E7,&00,&00,&00,&00,&00 ;  ; 107 k
 = &38,&28,&28,&28,&28,&28,&28,&28,&28,&28,&38,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&67,&BD,&91,&A5,&AD,&B5,&A5,&E7,&00,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&77,&AD,&A5,&A5,&A5,&A5,&A5,&E7,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&3C,&66,&A5,&A5,&A5,&A5,&66,&3C,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&37,&6D,&A5,&A5,&A5,&A5,&6D,&35,&05,&05,&07,&00,&00 ;  ; 112 p
 = &00,&00,&00,&EC,&B6,&A5,&A5,&A5,&A5,&B6,&AC,&A0,&A0,&E0,&00,&00 ;  ; 113 q
 = &00,&00,&00,&7F,&C5,&79,&05,&05,&05,&05,&07,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&7C,&82,&7A,&14,&28,&5E,&41,&3E,&00,&00,&00,&00,&00 ;  ; 115 s
 = &20,&30,&2C,&EF,&28,&28,&28,&28,&28,&A8,&F0,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&E7,&A5,&A5,&A5,&A5,&A5,&B5,&EE,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&E7,&C5,&85,&85,&4D,&2A,&1A,&0C,&00,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&E7,&A5,&B5,&AD,&A5,&89,&5A,&66,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&83,&C5,&6A,&34,&2C,&56,&A3,&C1,&00,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&E3,&C5,&8B,&96,&6C,&58,&30,&30,&36,&16,&1C,&00,&00 ;  ; 121 y
 = &00,&00,&00,&7E,&81,&5E,&28,&14,&7A,&81,&7E,&00,&00,&00,&00,&00 ;  ; 122 z
 = &00,&00,&70,&18,&18,&18,&0E,&18,&18,&18,&70,&00,&00,&00,&00,&00 ;  ; 123 {
 = &00,&00,&18,&18,&18,&18,&00,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&00,&0E,&18,&18,&18,&38,&18,&18,&18,&0E,&00,&00,&00,&00,&00 ;  ; 125 }
 = &00,&00,&DC,&76,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&00,&00,&10,&38,&6C,&C6,&C6,&C6,&FE,&00,&00,&00,&00 ;  ; 127 
; FONT: <BLOCK>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7E,&81,&A5,&81,&81,&BD,&99,&81,&7E,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&6C,&FE,&FE,&FE,&FE,&7C,&38,&10,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&7C,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&18,&3C,&3C,&E7,&E7,&E7,&3C,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&E7,&C3,&81,&E7,&FF,&FF,&FF,&FF,&FF,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&3C,&66,&42,&42,&66,&3C,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&C3,&99,&BD,&BD,&99,&C3,&FF,&FF,&FF,&FF,&00,&00 ;  ; 10 
 = &00,&00,&F0,&E0,&B0,&98,&3C,&66,&66,&66,&3C,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&3C,&66,&66,&66,&3C,&18,&7E,&18,&18,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&FC,&CC,&FC,&0C,&0C,&0C,&0E,&0F,&07,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FE,&C6,&FE,&C6,&C6,&C6,&E6,&E7,&67,&06,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&E7,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0E,&3E,&FE,&3E,&0E,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&80,&C0,&E0,&F8,&FE,&F8,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&CC,&CC,&CC,&CC,&CC,&CC,&00,&CC,&CC,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&FE,&DB,&DB,&DB,&DE,&D8,&D8,&D8,&D8,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&7C,&C6,&0C,&38,&6C,&C6,&C6,&6C,&38,&60,&C6,&7C,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&00,&00,&FE,&FE,&FE,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&18,&3C,&7E,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&18,&18,&18,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&30,&60,&FF,&60,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&18,&0C,&FE,&0C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&06,&06,&06,&FE,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&00,&00,&10,&38,&38,&7C,&7C,&FE,&FE,&00,&00,&00,&00,&00,&00 ;  ; 30 
 = &00,&00,&00,&FE,&FE,&7C,&7C,&38,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&18,&3C,&3C,&3C,&18,&18,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&C6,&C6,&C6,&44,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&6C,&6C,&FE,&6C,&6C,&6C,&FE,&6C,&6C,&00,&00,&00,&00,&00 ;  ; 35 #
 = &30,&30,&7C,&C6,&86,&06,&7C,&C0,&C2,&C6,&7C,&30,&30,&00,&00,&00 ;  ; 36 $
 = &00,&00,&00,&00,&86,&C6,&60,&30,&18,&CC,&C6,&00,&00,&00,&00,&00 ;  ; 37 %
 = &00,&00,&38,&6C,&6C,&38,&DC,&76,&66,&66,&DC,&00,&00,&00,&00,&00 ;  ; 38 &
 = &00,&0C,&0C,&0C,&06,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&30,&18,&0C,&0C,&0C,&0C,&0C,&18,&30,&00,&00,&00,&00,&00 ;  ; 40 (
 = &00,&00,&18,&30,&60,&60,&60,&60,&60,&30,&18,&00,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&00,&00,&66,&3C,&FF,&3C,&66,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&18,&18,&18,&FF,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&18,&0C,&00,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&80,&C0,&60,&30,&18,&0C,&06,&03,&01,&00,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&7E,&E7,&E7,&F7,&EF,&E7,&E7,&E7,&7E,&00,&00,&00,&00,&00 ;  ; 48 0
 = &00,&00,&38,&3C,&3C,&38,&38,&38,&38,&38,&7C,&00,&00,&00,&00,&00 ;  ; 49 1
 = &00,&00,&7E,&F7,&F7,&F7,&F0,&7E,&07,&FF,&FF,&00,&00,&00,&00,&00 ;  ; 50 2
 = &00,&00,&7E,&EF,&EF,&E0,&FE,&E0,&EF,&EF,&7E,&00,&00,&00,&00,&00 ;  ; 51 3
 = &00,&00,&F0,&F0,&F4,&F6,&FF,&F0,&F0,&F0,&F0,&00,&00,&00,&00,&00 ;  ; 52 4
 = &00,&00,&FF,&F7,&E7,&1F,&F0,&F7,&F7,&F7,&7E,&00,&00,&00,&00,&00 ;  ; 53 5
 = &00,&00,&7E,&F7,&F7,&07,&7F,&F7,&F7,&F7,&7E,&00,&00,&00,&00,&00 ;  ; 54 6
 = &00,&00,&FF,&F7,&F7,&F0,&F0,&F8,&7C,&3E,&1F,&00,&00,&00,&00,&00 ;  ; 55 7
 = &00,&00,&7E,&E7,&E7,&E7,&7E,&E7,&E7,&E7,&7E,&00,&00,&00,&00,&00 ;  ; 56 8
 = &00,&00,&7E,&F7,&F7,&F7,&FE,&F0,&F7,&F7,&7E,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&18,&18,&00,&00,&18,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&18,&18,&00,&00,&00,&18,&18,&0C,&00,&00,&00,&00,&00 ;  ; 59 ;
 = &00,&00,&60,&30,&18,&0C,&06,&0C,&18,&30,&60,&00,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&00,&7E,&00,&00,&7E,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&00,&06,&0C,&18,&30,&60,&30,&18,&0C,&06,&00,&00,&00,&00,&00 ;  ; 62 >
 = &00,&00,&7C,&C6,&C6,&60,&30,&30,&00,&30,&30,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&00,&7C,&C6,&C6,&F6,&F6,&F6,&76,&06,&7C,&00,&00,&00,&00,&00 ;  ; 64 @
 = &00,&00,&7E,&EF,&EF,&E0,&FE,&EF,&EF,&EF,&FE,&00,&00,&00,&00,&00 ;  ; 65 A
 = &00,&00,&7F,&F7,&F7,&77,&3F,&77,&F7,&F7,&7F,&00,&00,&00,&00,&00 ;  ; 66 B
 = &00,&00,&7E,&F7,&F7,&F7,&07,&F7,&F7,&F7,&7E,&00,&00,&00,&00,&00 ;  ; 67 C
 = &00,&00,&7F,&F7,&F7,&F7,&F7,&F7,&F7,&F7,&7F,&00,&00,&00,&00,&00 ;  ; 68 D
 = &00,&00,&7E,&EF,&EF,&FF,&0F,&EF,&EF,&EF,&7E,&00,&00,&00,&00,&00 ;  ; 69 E
 = &00,&00,&7E,&EF,&EF,&EF,&0F,&FF,&0F,&0F,&0F,&00,&00,&00,&00,&00 ;  ; 70 F
 = &00,&00,&7E,&EF,&EF,&EF,&0F,&EF,&EF,&EF,&FE,&00,&00,&00,&00,&00 ;  ; 71 G
 = &00,&00,&E7,&E7,&E7,&E7,&FF,&E7,&E7,&E7,&E7,&00,&00,&00,&00,&00 ;  ; 72 H
 = &00,&00,&3C,&3C,&3C,&3C,&3C,&3C,&3C,&3C,&3C,&00,&00,&00,&00,&00 ;  ; 73 I
 = &00,&00,&F0,&F0,&F0,&F0,&F0,&F7,&F7,&F7,&7E,&00,&00,&00,&00,&00 ;  ; 74 J
 = &00,&00,&EF,&EF,&EF,&EF,&7F,&EF,&EF,&EF,&EF,&00,&00,&00,&00,&00 ;  ; 75 K
 = &00,&00,&0F,&0F,&0F,&0F,&0F,&0F,&EF,&EF,&EF,&00,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&7F,&DB,&DB,&DB,&DB,&DB,&DB,&DB,&DB,&00,&00,&00,&00,&00 ;  ; 77 M
 = &00,&00,&7F,&F7,&F7,&F7,&F7,&F7,&F7,&F7,&F7,&00,&00,&00,&00,&00 ;  ; 78 N
 = &00,&00,&7E,&E7,&E7,&E7,&E7,&E7,&E7,&E7,&7E,&00,&00,&00,&00,&00 ;  ; 79 O
 = &00,&00,&7F,&DF,&DF,&DF,&7F,&0F,&0F,&0F,&0F,&00,&00,&00,&00,&00 ;  ; 80 P
 = &00,&00,&7E,&E7,&E7,&E7,&E7,&E7,&E7,&E7,&FE,&00,&00,&00,&00,&00 ;  ; 81 Q
 = &00,&00,&7F,&F7,&F7,&F7,&7F,&F7,&F7,&F7,&F7,&00,&00,&00,&00,&00 ;  ; 82 R
 = &00,&00,&7E,&EF,&EF,&0F,&FF,&E0,&EF,&EF,&7E,&00,&00,&00,&00,&00 ;  ; 83 S
 = &00,&00,&0F,&FF,&0F,&0F,&0F,&EF,&EF,&EF,&7E,&00,&00,&00,&00,&00 ;  ; 84 T
 = &00,&00,&EF,&EF,&EF,&EF,&EF,&EF,&EF,&EF,&7E,&00,&00,&00,&00,&00 ;  ; 85 U
 = &00,&00,&EF,&EF,&EF,&EF,&EF,&EF,&EF,&6E,&3C,&00,&00,&00,&00,&00 ;  ; 86 V
 = &00,&00,&DB,&DB,&DB,&DB,&DB,&DB,&DB,&DB,&7E,&00,&00,&00,&00,&00 ;  ; 87 W
 = &00,&00,&EF,&EF,&EE,&EC,&7E,&37,&77,&F7,&F7,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&00,&F7,&F7,&F7,&FE,&F0,&F7,&F7,&F7,&7E,&00,&00,&00,&00,&00 ;  ; 89 Y
 = &00,&00,&FF,&EF,&F7,&7B,&BD,&DE,&EF,&F7,&FF,&00,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&00,&3C,&0C,&0C,&0C,&0C,&0C,&0C,&0C,&3C,&00,&00,&00,&00,&00 ;  ; 91 [
 = &00,&00,&02,&06,&0E,&1C,&38,&70,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&3C,&30,&30,&30,&30,&30,&30,&30,&3C,&00,&00,&00,&00,&00 ;  ; 93 ]
 = &10,&38,&6C,&C6,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00 ;  ; 95 _
 = &18,&18,&30,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&7C,&F6,&F0,&FC,&F6,&FC,&00,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&0E,&0E,&0E,&7E,&DE,&DE,&DE,&DE,&7E,&00,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&7C,&DE,&1E,&DE,&DE,&7C,&00,&00,&00,&00,&00 ;  ; 99 c
 = &00,&00,&E0,&E0,&E0,&FC,&F6,&F6,&F6,&F6,&FC,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&7C,&DE,&FE,&1E,&DE,&7C,&00,&00,&00,&00,&00 ;  ; 101 e
 = &00,&00,&7C,&DE,&DE,&1E,&FE,&1E,&1E,&1E,&1E,&00,&00,&00,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&00,&7C,&EE,&EE,&EE,&FC,&E0,&EE,&EE,&7C,&00,&00 ;  ; 103 g
 = &00,&00,&0E,&0E,&0E,&7E,&EE,&EE,&EE,&EE,&EE,&00,&00,&00,&00,&00 ;  ; 104 h
 = &00,&00,&38,&38,&00,&38,&38,&38,&38,&38,&38,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&E0,&E0,&00,&E0,&E0,&E0,&E0,&E0,&EE,&EE,&EE,&7C,&00,&00 ;  ; 106 j
 = &00,&00,&0E,&0E,&0E,&EE,&EE,&7E,&EE,&EE,&EE,&00,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&1C,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&7F,&DB,&DB,&DB,&DB,&DB,&00,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&7E,&EE,&EE,&EE,&EE,&EE,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&7C,&EE,&EE,&EE,&EE,&7C,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&7E,&DE,&DE,&DE,&DE,&7E,&0E,&0E,&0E,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&00,&FC,&F6,&F6,&F6,&F6,&FC,&E0,&E0,&E0,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&00,&7E,&EE,&EE,&0E,&0E,&0E,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&7C,&06,&FE,&C0,&DE,&7C,&00,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&1C,&1C,&1C,&FE,&1C,&DC,&DC,&FC,&78,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&EE,&EE,&EE,&EE,&EE,&FC,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&00,&EE,&EE,&EE,&EE,&6C,&38,&00,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&DB,&DB,&DB,&DB,&DB,&7E,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&00,&DF,&DF,&F8,&1F,&F7,&F7,&00,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&00,&EE,&EE,&EE,&FC,&E0,&EE,&EE,&EE,&7C,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&00,&FE,&F6,&7A,&BC,&DE,&FE,&00,&00,&00,&00,&00 ;  ; 122 z
 = &00,&00,&70,&18,&18,&18,&0E,&18,&18,&18,&70,&00,&00,&00,&00,&00 ;  ; 123 {
 = &00,&00,&18,&18,&18,&18,&00,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&00,&0E,&18,&18,&18,&38,&18,&18,&18,&0E,&00,&00,&00,&00,&00 ;  ; 125 }
 = &00,&00,&DC,&76,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&00,&00,&10,&38,&6C,&C6,&C6,&C6,&FE,&00,&00,&00,&00 ;  ; 127 
; FONT: <EVSCRIBB>
 = &61,&91,&91,&92,&92,&92,&64,&04,&08,&08,&08,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&00,&42,&00,&00,&00,&81,&42,&3C,&00,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&00,&42,&00,&00,&00,&3C,&42,&81,&00,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&42,&00,&00,&00,&00,&7E,&00,&00,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&44,&00,&00,&00,&00,&6C,&92,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&00,&00,&44,&00,&00,&00,&00,&92,&6C,&00,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&00,&44,&00,&00,&00,&00,&7C,&82,&7C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&44,&00,&00,&00,&7C,&82,&82,&7C,&00,&00,&00,&00,&00 ;  ; 7 
 = &00,&00,&00,&44,&00,&00,&00,&38,&44,&44,&38,&00,&00,&00,&00,&00 ;  ; 8 
 = &00,&00,&00,&28,&A8,&A9,&A9,&FF,&FF,&FE,&7C,&00,&00,&00,&00,&00 ;  ; 9 
 = &00,&00,&00,&14,&15,&95,&95,&FF,&7F,&3F,&1E,&00,&00,&00,&00,&00 ;  ; 10 
 = &00,&00,&70,&60,&50,&08,&1E,&21,&21,&21,&1E,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&7C,&82,&82,&82,&7C,&10,&7C,&10,&10,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&F8,&88,&F8,&08,&04,&04,&04,&07,&07,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FE,&82,&FE,&82,&82,&82,&82,&E2,&E3,&03,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&E7,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0A,&12,&22,&12,&0A,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&80,&C0,&A0,&90,&88,&90,&A0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&18,&24,&42,&00,&00,&00,&42,&24,&18,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&44,&44,&44,&44,&44,&44,&00,&44,&44,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&7E,&51,&51,&51,&5E,&50,&50,&50,&50,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&7C,&82,&04,&38,&44,&82,&82,&44,&38,&40,&82,&7C,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&00,&FE,&FE,&FE,&00,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&18,&24,&42,&18,&18,&18,&42,&24,&18,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&00,&00,&08,&14,&22,&00,&08,&08,&08,&08,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&00,&00,&08,&08,&08,&08,&22,&14,&08,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&20,&40,&8E,&40,&20,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&08,&04,&72,&04,&08,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &7C,&44,&22,&22,&F7,&82,&41,&47,&24,&14,&0A,&06,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &04,&0A,&11,&00,&00,&20,&70,&A8,&20,&20,&20,&20,&00,&00,&00,&00 ;  ; 30 
 = &04,&0A,&11,&00,&00,&40,&20,&10,&F8,&10,&20,&40,&00,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&10,&10,&10,&10,&10,&10,&10,&10,&00,&00,&10,&10,&00,&00,&00 ;  ; 33 !
 = &00,&24,&24,&24,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&42,&42,&FF,&42,&42,&42,&FF,&42,&42,&00,&00,&00,&00,&00 ;  ; 35 #
 = &20,&3C,&62,&11,&11,&12,&1C,&28,&48,&48,&27,&1C,&04,&00,&00,&00 ;  ; 36 $
 = &00,&C6,&B9,&49,&26,&10,&08,&04,&7E,&93,&90,&60,&00,&00,&00,&00 ;  ; 37 %
 = &00,&0E,&11,&11,&11,&0A,&46,&49,&51,&21,&52,&8C,&00,&00,&00,&00 ;  ; 38 &
 = &00,&18,&18,&08,&04,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&60,&18,&04,&02,&02,&02,&02,&02,&04,&18,&60,&00,&00,&00,&00 ;  ; 40 (
 = &00,&06,&18,&20,&40,&40,&40,&40,&40,&20,&18,&06,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&00,&08,&2A,&1C,&7F,&1C,&2A,&08,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&08,&08,&08,&7F,&08,&08,&08,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&08,&04,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&7F,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&80,&40,&20,&10,&08,&04,&02,&01,&00,&00,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&00,&00,&3C,&42,&81,&91,&89,&81,&42,&3C,&00,&00,&00,&00 ;  ; 48 0
 = &00,&20,&30,&28,&20,&20,&10,&10,&10,&08,&08,&08,&00,&00,&00,&00 ;  ; 49 1
 = &00,&78,&84,&82,&80,&80,&40,&20,&10,&0E,&39,&C6,&00,&00,&00,&00 ;  ; 50 2
 = &00,&7C,&82,&80,&80,&40,&38,&40,&80,&80,&40,&21,&1E,&00,&00,&00 ;  ; 51 3
 = &00,&20,&22,&11,&11,&11,&7E,&08,&08,&08,&04,&04,&00,&00,&00,&00 ;  ; 52 4
 = &00,&00,&02,&F9,&01,&06,&18,&20,&40,&80,&80,&42,&3C,&00,&00,&00 ;  ; 53 5
 = &00,&00,&20,&10,&08,&04,&04,&3A,&46,&82,&82,&7C,&00,&00,&00,&00 ;  ; 54 6
 = &00,&FE,&80,&40,&40,&20,&7C,&10,&10,&08,&08,&04,&00,&00,&00,&00 ;  ; 55 7
 = &00,&0E,&10,&24,&22,&12,&0C,&1C,&22,&22,&22,&1C,&00,&00,&00,&00 ;  ; 56 8
 = &00,&0E,&11,&01,&31,&2E,&20,&10,&10,&10,&08,&08,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&18,&18,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&18,&18,&08,&04,&00,&00 ;  ; 59 ;
 = &00,&40,&20,&10,&08,&04,&02,&04,&08,&10,&20,&40,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&7F,&00,&00,&00,&7F,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&02,&04,&08,&10,&20,&40,&20,&10,&08,&04,&02,&00,&00,&00,&00 ;  ; 62 >
 = &00,&3C,&42,&41,&40,&20,&10,&08,&08,&00,&08,&08,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&78,&84,&82,&82,&B1,&A9,&A9,&79,&01,&01,&7E,&00,&00,&00,&00 ;  ; 64 @
 = &00,&30,&48,&48,&44,&44,&5F,&42,&42,&41,&41,&41,&00,&00,&00,&00 ;  ; 65 A
 = &00,&1F,&21,&21,&11,&0D,&11,&21,&41,&41,&41,&3F,&00,&00,&00,&00 ;  ; 66 B
 = &00,&3C,&42,&01,&01,&01,&01,&01,&01,&01,&42,&3C,&00,&00,&00,&00 ;  ; 67 C
 = &00,&5C,&22,&5D,&41,&41,&41,&41,&41,&45,&4B,&31,&00,&00,&00,&00 ;  ; 68 D
 = &00,&7F,&00,&01,&01,&01,&3D,&01,&01,&01,&02,&7C,&00,&00,&00,&00 ;  ; 69 E
 = &00,&7F,&00,&01,&01,&01,&1D,&01,&01,&01,&01,&01,&00,&00,&00,&00 ;  ; 70 F
 = &00,&3C,&42,&01,&01,&01,&01,&39,&45,&41,&42,&3C,&00,&00,&00,&00 ;  ; 71 G
 = &00,&41,&41,&41,&41,&7F,&41,&41,&41,&41,&41,&41,&00,&00,&00,&00 ;  ; 72 H
 = &00,&3E,&08,&08,&08,&08,&08,&08,&08,&08,&08,&3E,&00,&00,&00,&00 ;  ; 73 I
 = &00,&40,&40,&40,&40,&40,&40,&40,&40,&41,&22,&1C,&00,&00,&00,&00 ;  ; 74 J
 = &00,&41,&21,&11,&09,&05,&03,&03,&05,&09,&11,&21,&41,&00,&00,&00 ;  ; 75 K
 = &00,&01,&01,&01,&01,&01,&01,&01,&01,&05,&0B,&71,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&35,&4B,&49,&49,&49,&49,&49,&49,&49,&49,&00,&00,&00,&00 ;  ; 77 M
 = &00,&3D,&42,&42,&42,&42,&42,&42,&42,&42,&42,&42,&00,&00,&00,&00 ;  ; 78 N
 = &00,&5C,&22,&5D,&41,&41,&41,&41,&41,&41,&41,&3E,&00,&00,&00,&00 ;  ; 79 O
 = &00,&3F,&41,&41,&41,&21,&1F,&01,&01,&01,&01,&01,&00,&00,&00,&00 ;  ; 80 P
 = &00,&3C,&42,&41,&41,&41,&43,&43,&45,&49,&32,&3C,&40,&00,&00,&00 ;  ; 81 Q
 = &00,&3F,&41,&41,&41,&21,&1F,&03,&05,&09,&11,&21,&41,&00,&00,&00 ;  ; 82 R
 = &00,&3C,&42,&01,&01,&02,&1C,&20,&40,&40,&21,&1E,&00,&00,&00,&00 ;  ; 83 S
 = &00,&7F,&08,&08,&08,&08,&08,&08,&08,&08,&08,&08,&08,&00,&00,&00 ;  ; 84 T
 = &00,&00,&41,&41,&41,&41,&41,&41,&41,&41,&21,&52,&4C,&00,&00,&00 ;  ; 85 U
 = &00,&00,&82,&82,&82,&82,&82,&82,&44,&44,&28,&28,&10,&00,&00,&00 ;  ; 86 V
 = &00,&00,&41,&41,&41,&41,&41,&41,&49,&4D,&53,&51,&21,&00,&00,&00 ;  ; 87 W
 = &00,&00,&80,&42,&24,&28,&18,&18,&24,&44,&82,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&41,&42,&42,&24,&28,&10,&10,&08,&04,&04,&02,&01,&00,&00,&00 ;  ; 89 Y
 = &00,&00,&7E,&40,&20,&10,&7E,&04,&04,&02,&01,&7E,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&7E,&02,&02,&02,&02,&02,&02,&02,&02,&02,&7E,&00,&00,&00,&00 ;  ; 91 [
 = &00,&00,&01,&03,&06,&0C,&18,&30,&60,&C0,&80,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&7E,&40,&40,&40,&40,&40,&40,&40,&40,&40,&7E,&00,&00,&00,&00 ;  ; 93 ]
 = &00,&18,&24,&42,&81,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00 ;  ; 95 _
 = &00,&10,&20,&40,&80,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&00,&1C,&22,&01,&21,&51,&8E,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&04,&04,&04,&04,&34,&4C,&84,&84,&44,&30,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&00,&1C,&22,&01,&01,&21,&1E,&00,&00,&00,&00 ;  ; 99 c
 = &00,&40,&A0,&A0,&A0,&A0,&A0,&AC,&52,&42,&A2,&9C,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&1C,&22,&21,&1D,&01,&02,&3C,&00,&00,&00,&00 ;  ; 101 e
 = &00,&10,&28,&08,&08,&08,&2B,&08,&08,&08,&08,&08,&08,&04,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&00,&0C,&12,&42,&2C,&10,&28,&24,&24,&18,&00,&00 ;  ; 103 g
 = &00,&00,&02,&02,&02,&02,&02,&02,&32,&4A,&86,&82,&00,&00,&00,&00 ;  ; 104 h
 = &00,&00,&00,&00,&08,&08,&00,&08,&14,&24,&42,&81,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&00,&00,&08,&08,&00,&0E,&11,&10,&10,&10,&12,&0C,&00,&00 ;  ; 106 j
 = &00,&02,&02,&02,&02,&02,&02,&1A,&26,&22,&12,&62,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&00,&18,&24,&24,&24,&14,&08,&0C,&12,&21,&40,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&00,&49,&B6,&92,&92,&92,&92,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&00,&31,&4A,&46,&42,&42,&82,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&00,&98,&64,&5A,&42,&42,&3C,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&00,&38,&44,&46,&4D,&35,&05,&06,&04,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&00,&00,&0C,&22,&22,&32,&2C,&A0,&60,&20,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&00,&00,&18,&24,&22,&42,&41,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&00,&30,&4C,&43,&40,&24,&18,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&00,&00,&08,&08,&08,&3E,&14,&14,&22,&41,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&00,&00,&02,&41,&41,&21,&5E,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&00,&00,&42,&42,&45,&24,&14,&0C,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&00,&49,&49,&49,&55,&66,&22,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&00,&40,&23,&14,&08,&14,&12,&61,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&00,&42,&42,&62,&5C,&40,&40,&20,&10,&00,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&00,&1C,&20,&10,&3E,&04,&02,&3C,&00,&00,&00,&00 ;  ; 122 z
 = &00,&30,&08,&08,&08,&04,&02,&04,&08,&08,&08,&30,&00,&00,&00,&00 ;  ; 123 {
 = &00,&08,&08,&08,&08,&00,&00,&00,&08,&08,&08,&08,&00,&00,&00,&00 ;  ; 124 |
 = &00,&06,&08,&08,&08,&10,&20,&10,&08,&08,&08,&06,&00,&00,&00,&00 ;  ; 125 }
 = &00,&0E,&91,&60,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &33,&33,&CC,&CC,&33,&33,&CC,&CC,&33,&33,&CC,&CC,&00,&00,&00,&00 ;  ; 127 
; FONT: <EVSCRIPT>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &04,&0A,&11,&00,&20,&50,&88,&F8,&88,&88,&88,&00,&00,&00,&00,&00 ;  ; 1 
 = &04,&0A,&11,&00,&78,&88,&88,&78,&88,&88,&78,&00,&00,&00,&00,&00 ;  ; 2 
 = &04,&0A,&11,&00,&70,&88,&08,&08,&08,&88,&70,&00,&00,&00,&00,&00 ;  ; 3 
 = &04,&0A,&11,&00,&78,&88,&88,&88,&88,&88,&78,&00,&00,&00,&00,&00 ;  ; 4 
 = &04,&0A,&11,&00,&F8,&08,&08,&F8,&08,&08,&F8,&00,&00,&00,&00,&00 ;  ; 5 
 = &04,&0A,&11,&00,&70,&88,&08,&E8,&88,&88,&70,&00,&00,&00,&00,&00 ;  ; 6 
 = &04,&0A,&11,&00,&88,&88,&88,&F8,&88,&88,&88,&00,&00,&00,&00,&00 ;  ; 7 
 = &04,&0A,&11,&00,&70,&20,&20,&20,&20,&20,&70,&00,&00,&00,&00,&00 ;  ; 8 
 = &04,&0A,&11,&00,&E0,&40,&40,&40,&40,&48,&30,&00,&00,&00,&00,&00 ;  ; 9 
 = &04,&0A,&11,&00,&88,&48,&28,&18,&28,&48,&88,&00,&00,&00,&00,&00 ;  ; 10 
 = &04,&0A,&11,&00,&08,&08,&08,&08,&08,&08,&F8,&00,&00,&00,&00,&00 ;  ; 11 
 = &04,&0A,&11,&00,&88,&D8,&A8,&88,&88,&88,&88,&00,&00,&00,&00,&00 ;  ; 12 
 = &04,&0A,&11,&00,&88,&98,&A8,&C8,&8C,&8C,&88,&00,&00,&00,&00,&00 ;  ; 13 
 = &04,&0A,&11,&00,&70,&88,&88,&88,&88,&88,&70,&00,&00,&00,&00,&00 ;  ; 14 
 = &04,&0A,&11,&00,&78,&88,&88,&78,&08,&08,&08,&00,&00,&00,&00,&00 ;  ; 15 
 = &04,&0A,&11,&00,&70,&88,&88,&88,&A8,&90,&68,&04,&00,&00,&00,&00 ;  ; 16 
 = &04,&0A,&11,&00,&78,&88,&88,&78,&28,&48,&88,&00,&00,&00,&00,&00 ;  ; 17 
 = &04,&0A,&11,&00,&70,&88,&08,&70,&80,&88,&70,&00,&00,&00,&00,&00 ;  ; 18 
 = &04,&0A,&11,&00,&F8,&20,&20,&20,&20,&20,&20,&00,&00,&00,&00,&00 ;  ; 19 
 = &04,&0A,&11,&00,&88,&88,&88,&88,&88,&88,&70,&00,&00,&00,&00,&00 ;  ; 20 
 = &04,&0A,&11,&00,&88,&88,&88,&88,&50,&50,&20,&00,&00,&00,&00,&00 ;  ; 21 
 = &04,&0A,&11,&00,&88,&88,&88,&88,&A8,&D8,&88,&00,&00,&00,&00,&00 ;  ; 22 
 = &04,&0A,&11,&00,&88,&88,&50,&20,&50,&88,&88,&00,&00,&00,&00,&00 ;  ; 23 
 = &04,&0A,&11,&00,&88,&88,&88,&50,&20,&20,&20,&00,&00,&00,&00,&00 ;  ; 24 
 = &04,&0A,&11,&00,&F8,&80,&40,&20,&10,&08,&F8,&00,&00,&00,&00,&00 ;  ; 25 
 = &04,&0A,&11,&00,&E0,&20,&20,&20,&20,&20,&E0,&00,&00,&00,&00,&00 ;  ; 26 
 = &04,&0A,&11,&00,&08,&08,&10,&20,&40,&80,&80,&00,&00,&00,&00,&00 ;  ; 27 
 = &04,&0A,&11,&00,&20,&70,&A8,&20,&20,&20,&20,&00,&00,&00,&00,&00 ;  ; 28 
 = &04,&0A,&11,&00,&20,&10,&F8,&10,&20,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 30 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&10,&10,&10,&10,&10,&10,&00,&10,&10,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&00,&44,&44,&44,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&42,&42,&FF,&42,&42,&42,&FF,&42,&42,&00,&00,&00,&00,&00 ;  ; 35 #
 = &00,&08,&7E,&09,&09,&09,&3E,&48,&48,&48,&3F,&08,&00,&00,&00,&00 ;  ; 36 $
 = &00,&86,&89,&C9,&66,&30,&18,&0C,&66,&93,&91,&61,&00,&00,&00,&00 ;  ; 37 %
 = &00,&00,&0E,&11,&11,&0A,&0E,&91,&61,&61,&8E,&00,&00,&00,&00,&00 ;  ; 38 &
 = &00,&00,&18,&18,&08,&04,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&60,&18,&04,&02,&02,&02,&02,&04,&18,&60,&00,&00,&00,&00 ;  ; 40 (
 = &00,&00,&06,&18,&20,&40,&40,&40,&40,&20,&18,&06,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&2A,&1C,&7F,&1C,&2A,&08,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&08,&08,&08,&7F,&08,&08,&08,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&08,&04,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&FE,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&80,&C0,&60,&30,&18,&0C,&06,&03,&01,&00,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&7C,&82,&C1,&A1,&91,&89,&85,&43,&3E,&00,&00,&00,&00,&00 ;  ; 48 0
 = &00,&00,&10,&18,&10,&10,&10,&10,&10,&10,&38,&00,&00,&00,&00,&00 ;  ; 49 1
 = &00,&00,&7C,&82,&81,&40,&10,&08,&04,&3E,&C3,&00,&00,&00,&00,&00 ;  ; 50 2
 = &00,&00,&7C,&82,&81,&40,&38,&40,&80,&41,&3E,&00,&00,&00,&00,&00 ;  ; 51 3
 = &00,&00,&84,&84,&42,&42,&21,&FF,&20,&10,&10,&00,&00,&00,&00,&00 ;  ; 52 4
 = &00,&00,&FC,&02,&02,&3F,&40,&80,&80,&C1,&3E,&00,&00,&00,&00,&00 ;  ; 53 5
 = &00,&00,&78,&04,&02,&7A,&83,&81,&81,&41,&3E,&00,&00,&00,&00,&00 ;  ; 54 6
 = &00,&00,&FE,&81,&40,&40,&20,&10,&10,&08,&04,&00,&00,&00,&00,&00 ;  ; 55 7
 = &00,&00,&7C,&82,&81,&41,&7E,&82,&81,&41,&3E,&00,&00,&00,&00,&00 ;  ; 56 8
 = &00,&00,&7C,&82,&81,&E1,&5E,&20,&20,&10,&10,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&00,&18,&18,&00,&00,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&00,&18,&18,&00,&00,&00,&18,&18,&08,&04,&00,&00,&00 ;  ; 59 ;
 = &00,&00,&20,&10,&08,&04,&02,&04,&08,&10,&20,&00,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&00,&7E,&00,&00,&7E,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&00,&04,&08,&10,&20,&40,&20,&10,&08,&04,&00,&00,&00,&00,&00 ;  ; 62 >
 = &00,&00,&7C,&82,&81,&20,&10,&10,&00,&10,&10,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&00,&78,&84,&82,&B2,&A9,&79,&01,&01,&7E,&00,&00,&00,&00,&00 ;  ; 64 @
 = &00,&00,&78,&44,&42,&42,&41,&41,&61,&D1,&4E,&00,&00,&00,&00,&00 ;  ; 65 A
 = &00,&00,&7A,&86,&82,&43,&3A,&43,&9E,&82,&7D,&00,&00,&00,&00,&00 ;  ; 66 B
 = &00,&00,&9C,&62,&91,&61,&01,&01,&01,&82,&7C,&00,&00,&00,&00,&00 ;  ; 67 C
 = &00,&00,&3E,&46,&BA,&83,&82,&82,&82,&C2,&3D,&00,&00,&00,&00,&00 ;  ; 68 D
 = &00,&00,&7C,&A2,&61,&02,&3C,&02,&01,&82,&7C,&00,&00,&00,&00,&00 ;  ; 69 E
 = &00,&00,&FF,&43,&21,&20,&7C,&21,&2F,&E1,&1E,&00,&00,&00,&00,&00 ;  ; 70 F
 = &00,&00,&88,&94,&C8,&B8,&88,&83,&9E,&83,&7D,&00,&00,&00,&00,&00 ;  ; 71 G
 = &00,&00,&41,&42,&42,&FE,&42,&42,&42,&82,&01,&00,&00,&00,&00,&00 ;  ; 72 H
 = &00,&00,&10,&28,&28,&10,&11,&37,&51,&91,&0E,&00,&00,&00,&00,&00 ;  ; 73 I
 = &00,&00,&38,&44,&42,&42,&C4,&78,&20,&70,&A8,&24,&14,&08,&00,&00 ;  ; 74 J
 = &00,&00,&41,&22,&12,&0A,&06,&0A,&12,&A2,&41,&00,&00,&00,&00,&00 ;  ; 75 K
 = &00,&00,&61,&92,&7C,&10,&10,&10,&16,&89,&76,&00,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&6D,&92,&92,&92,&92,&92,&92,&92,&12,&00,&00,&00,&00,&00 ;  ; 77 M
 = &00,&00,&3D,&42,&42,&42,&42,&42,&42,&82,&02,&00,&00,&00,&00,&00 ;  ; 78 N
 = &00,&00,&3C,&46,&B9,&81,&81,&81,&81,&42,&3C,&00,&00,&00,&00,&00 ;  ; 79 O
 = &00,&00,&7D,&82,&82,&42,&3E,&02,&02,&02,&02,&00,&00,&00,&00,&00 ;  ; 80 P
 = &00,&00,&3C,&52,&92,&8C,&40,&20,&1E,&99,&66,&00,&00,&00,&00,&00 ;  ; 81 Q
 = &00,&00,&7D,&82,&82,&42,&3E,&12,&22,&C2,&82,&00,&00,&00,&00,&00 ;  ; 82 R
 = &00,&00,&10,&28,&28,&10,&38,&45,&8F,&81,&7C,&00,&00,&00,&00,&00 ;  ; 83 S
 = &00,&00,&FF,&43,&21,&20,&20,&21,&2F,&21,&1E,&00,&00,&00,&00,&00 ;  ; 84 T
 = &00,&00,&83,&83,&82,&82,&82,&82,&82,&C2,&3C,&00,&00,&00,&00,&00 ;  ; 85 U
 = &00,&00,&83,&43,&42,&22,&22,&12,&0A,&0E,&06,&00,&00,&00,&00,&00 ;  ; 86 V
 = &00,&00,&93,&9B,&59,&59,&55,&35,&33,&13,&11,&00,&00,&00,&00,&00 ;  ; 87 W
 = &00,&00,&C6,&25,&2E,&18,&18,&18,&74,&A4,&63,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&00,&43,&43,&42,&42,&62,&52,&4C,&C0,&40,&60,&50,&30,&00,&00 ;  ; 89 Y
 = &00,&00,&3C,&52,&8C,&80,&58,&24,&58,&40,&C0,&60,&50,&30,&00,&00 ;  ; 90 Z
 = &00,&00,&7E,&02,&02,&02,&02,&02,&02,&02,&7E,&00,&00,&00,&00,&00 ;  ; 91 [
 = &00,&00,&01,&03,&06,&0C,&18,&30,&60,&C0,&80,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&7E,&40,&40,&40,&40,&40,&40,&40,&7E,&00,&00,&00,&00,&00 ;  ; 93 ]
 = &00,&00,&18,&24,&42,&81,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00,&00 ;  ; 95 _
 = &00,&00,&10,&20,&40,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&3C,&42,&41,&41,&E1,&5E,&00,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&18,&14,&14,&14,&0C,&26,&65,&A4,&18,&00,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&5C,&22,&21,&01,&81,&7C,&00,&00,&00,&00,&00 ;  ; 99 c
 = &00,&00,&40,&40,&40,&5C,&62,&41,&41,&E1,&5E,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&3C,&42,&31,&0D,&83,&7E,&00,&00,&00,&00,&00 ;  ; 101 e
 = &00,&00,&18,&28,&28,&28,&18,&0C,&0A,&19,&24,&E4,&24,&18,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&00,&3C,&42,&41,&61,&DE,&48,&24,&1C,&00,&00,&00 ;  ; 103 g
 = &00,&00,&04,&0A,&0A,&06,&3A,&46,&43,&C2,&42,&00,&00,&00,&00,&00 ;  ; 104 h
 = &00,&00,&00,&0C,&0C,&00,&0C,&0C,&0B,&88,&70,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&00,&0C,&0C,&00,&0C,&0C,&0B,&FC,&0A,&09,&07,&00,&00,&00 ;  ; 106 j
 = &00,&00,&08,&14,&0C,&1C,&22,&12,&0F,&92,&62,&00,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&18,&14,&14,&0C,&04,&06,&05,&84,&78,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&35,&4B,&49,&49,&C9,&49,&00,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&3A,&46,&42,&42,&C3,&42,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&3C,&52,&A1,&C1,&41,&3C,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&7A,&86,&83,&C6,&3A,&03,&03,&03,&00,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&00,&3C,&42,&41,&E1,&5E,&40,&70,&00,&00,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&02,&1E,&22,&22,&21,&A0,&40,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&08,&1C,&42,&41,&C4,&38,&00,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&08,&08,&08,&7E,&08,&0C,&0A,&89,&70,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&42,&42,&42,&43,&E2,&5C,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&00,&42,&62,&52,&4B,&86,&02,&00,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&49,&6D,&6D,&5B,&DB,&09,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&00,&46,&29,&11,&28,&A4,&42,&00,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&00,&41,&41,&41,&61,&FE,&48,&24,&1C,&00,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&00,&3E,&41,&38,&40,&F0,&48,&24,&1C,&00,&00,&00 ;  ; 122 z
 = &00,&20,&10,&08,&08,&08,&06,&08,&08,&08,&10,&20,&00,&00,&00,&00 ;  ; 123 {
 = &00,&00,&10,&10,&10,&10,&00,&10,&10,&10,&10,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&04,&08,&10,&10,&10,&60,&10,&10,&10,&08,&04,&00,&00,&00,&00 ;  ; 125 }
 = &00,&0E,&99,&70,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&33,&33,&CC,&CC,&33,&33,&CC,&CC,&00,&00,&00,&00,&00 ;  ; 127 
; FONT: <COURIER>
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 0 
 = &00,&00,&7E,&81,&A5,&81,&81,&BD,&99,&81,&7E,&00,&00,&00,&00,&00 ;  ; 1 
 = &00,&00,&7E,&FF,&DB,&FF,&FF,&C3,&E7,&FF,&7E,&00,&00,&00,&00,&00 ;  ; 2 
 = &00,&00,&00,&6C,&FE,&FE,&FE,&FE,&7C,&38,&10,&00,&00,&00,&00,&00 ;  ; 3 
 = &00,&00,&00,&10,&38,&7C,&FE,&7C,&38,&10,&00,&00,&00,&00,&00,&00 ;  ; 4 
 = &00,&18,&3C,&3C,&E7,&E7,&E7,&3C,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 5 
 = &00,&00,&18,&3C,&7E,&FF,&FF,&7E,&18,&18,&3C,&00,&00,&00,&00,&00 ;  ; 6 
 = &00,&00,&00,&00,&00,&18,&3C,&3C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 7 
 = &FF,&FF,&FF,&FF,&FF,&E7,&C3,&81,&E7,&FF,&FF,&FF,&FF,&FF,&00,&00 ;  ; 8 
 = &00,&00,&00,&00,&3C,&66,&42,&42,&66,&3C,&00,&00,&00,&00,&00,&00 ;  ; 9 
 = &FF,&FF,&FF,&FF,&C3,&99,&BD,&BD,&99,&C3,&FF,&FF,&FF,&FF,&00,&00 ;  ; 10 
 = &00,&00,&F0,&E0,&B0,&98,&3C,&66,&66,&66,&3C,&00,&00,&00,&00,&00 ;  ; 11 
 = &00,&00,&3C,&66,&66,&66,&3C,&18,&7E,&18,&18,&00,&00,&00,&00,&00 ;  ; 12 
 = &00,&00,&FC,&CC,&FC,&0C,&0C,&0C,&0E,&0F,&07,&00,&00,&00,&00,&00 ;  ; 13 
 = &00,&00,&FE,&C6,&FE,&C6,&C6,&C6,&E6,&E7,&67,&06,&00,&00,&00,&00 ;  ; 14 
 = &00,&00,&18,&18,&DB,&3C,&E7,&3C,&DB,&18,&18,&00,&00,&00,&00,&00 ;  ; 15 
 = &00,&00,&02,&06,&0E,&3E,&FE,&3E,&0E,&06,&02,&00,&00,&00,&00,&00 ;  ; 16 
 = &00,&00,&80,&C0,&E0,&F8,&FE,&F8,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 17 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 18 
 = &00,&00,&CC,&CC,&CC,&CC,&CC,&CC,&00,&CC,&CC,&00,&00,&00,&00,&00 ;  ; 19 
 = &00,&00,&FE,&DB,&DB,&DB,&DE,&D8,&D8,&D8,&D8,&00,&00,&00,&00,&00 ;  ; 20 
 = &00,&7C,&C6,&0C,&38,&6C,&C6,&C6,&6C,&38,&60,&C6,&7C,&00,&00,&00 ;  ; 21 
 = &00,&00,&00,&00,&00,&00,&00,&00,&FE,&FE,&FE,&00,&00,&00,&00,&00 ;  ; 22 
 = &00,&00,&18,&3C,&7E,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 23 
 = &00,&18,&3C,&7E,&18,&18,&18,&18,&18,&18,&00,&00,&00,&00,&00,&00 ;  ; 24 
 = &00,&00,&18,&18,&18,&18,&18,&18,&7E,&3C,&18,&00,&00,&00,&00,&00 ;  ; 25 
 = &00,&00,&00,&00,&30,&60,&FF,&60,&30,&00,&00,&00,&00,&00,&00,&00 ;  ; 26 
 = &00,&00,&00,&00,&18,&0C,&FE,&0C,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 27 
 = &00,&00,&00,&00,&00,&06,&06,&06,&FE,&00,&00,&00,&00,&00,&00,&00 ;  ; 28 
 = &00,&00,&00,&00,&24,&66,&FF,&66,&24,&00,&00,&00,&00,&00,&00,&00 ;  ; 29 
 = &00,&03,&02,&82,&42,&22,&17,&48,&64,&52,&F9,&40,&40,&00,&00,&00 ;  ; 30 
 = &00,&03,&02,&82,&42,&22,&17,&68,&94,&42,&21,&10,&F0,&00,&00,&00 ;  ; 31 
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 32  
 = &00,&00,&10,&38,&38,&38,&10,&10,&00,&10,&10,&00,&00,&00,&00,&00 ;  ; 33 !
 = &00,&66,&66,&44,&44,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 34 "
 = &00,&00,&24,&24,&7E,&24,&24,&24,&7E,&24,&24,&00,&00,&00,&00,&00 ;  ; 35 #
 = &30,&30,&FC,&C2,&82,&02,&7C,&80,&82,&82,&7C,&30,&30,&00,&00,&00 ;  ; 36 $
 = &00,&00,&00,&00,&86,&C6,&60,&30,&18,&CC,&C6,&00,&00,&00,&00,&00 ;  ; 37 %
 = &00,&00,&1C,&22,&12,&0C,&04,&4A,&32,&62,&9C,&00,&00,&00,&00,&00 ;  ; 38 &
 = &00,&06,&06,&04,&04,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 39 '
 = &00,&00,&30,&08,&04,&04,&04,&04,&04,&08,&30,&00,&00,&00,&00,&00 ;  ; 40 (
 = &00,&00,&18,&20,&40,&40,&40,&40,&40,&20,&18,&00,&00,&00,&00,&00 ;  ; 41 )
 = &00,&00,&00,&00,&66,&3C,&FF,&3C,&66,&00,&00,&00,&00,&00,&00,&00 ;  ; 42 *
 = &00,&00,&00,&10,&10,&10,&FE,&10,&10,&10,&00,&00,&00,&00,&00,&00 ;  ; 43 +
 = &00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&18,&0C,&00,&00,&00,&00 ;  ; 44 ,
 = &00,&00,&00,&00,&00,&00,&00,&FE,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 45 -
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&18,&18,&00,&00,&00,&00,&00 ;  ; 46 .
 = &00,&00,&80,&C0,&60,&30,&18,&0C,&06,&03,&01,&00,&00,&00,&00,&00 ;  ; 47 /
 = &00,&00,&38,&44,&82,&82,&82,&82,&82,&44,&38,&00,&00,&00,&00,&00 ;  ; 48 0
 = &00,&00,&38,&20,&20,&20,&20,&20,&20,&20,&F8,&00,&00,&00,&00,&00 ;  ; 49 1
 = &00,&00,&7C,&82,&80,&40,&20,&10,&08,&84,&FE,&00,&00,&00,&00,&00 ;  ; 50 2
 = &00,&00,&7C,&82,&80,&80,&78,&80,&80,&82,&7C,&00,&00,&00,&00,&00 ;  ; 51 3
 = &00,&00,&60,&50,&48,&44,&42,&FE,&40,&40,&F0,&00,&00,&00,&00,&00 ;  ; 52 4
 = &00,&00,&FE,&02,&02,&02,&7E,&80,&80,&82,&7C,&00,&00,&00,&00,&00 ;  ; 53 5
 = &00,&00,&78,&04,&02,&02,&7E,&82,&82,&82,&7C,&00,&00,&00,&00,&00 ;  ; 54 6
 = &00,&00,&FE,&82,&80,&40,&20,&10,&08,&04,&04,&00,&00,&00,&00,&00 ;  ; 55 7
 = &00,&00,&7C,&82,&82,&82,&7C,&82,&82,&82,&7C,&00,&00,&00,&00,&00 ;  ; 56 8
 = &00,&00,&7C,&82,&82,&82,&FC,&80,&80,&40,&3C,&00,&00,&00,&00,&00 ;  ; 57 9
 = &00,&00,&00,&18,&18,&00,&00,&18,&18,&00,&00,&00,&00,&00,&00,&00 ;  ; 58 :
 = &00,&00,&00,&18,&18,&00,&00,&00,&18,&18,&0C,&00,&00,&00,&00,&00 ;  ; 59 ;
 = &00,&00,&60,&30,&18,&0C,&06,&0C,&18,&30,&60,&00,&00,&00,&00,&00 ;  ; 60 <
 = &00,&00,&00,&00,&00,&7E,&00,&00,&7E,&00,&00,&00,&00,&00,&00,&00 ;  ; 61 =
 = &00,&00,&06,&0C,&18,&30,&60,&30,&18,&0C,&06,&00,&00,&00,&00,&00 ;  ; 62 >
 = &00,&00,&7C,&82,&82,&60,&20,&20,&00,&70,&70,&00,&00,&00,&00,&00 ;  ; 63 ?
 = &00,&00,&7C,&82,&82,&F2,&92,&F2,&02,&02,&7C,&00,&00,&00,&00,&00 ;  ; 64 @
 = &00,&00,&10,&38,&6C,&44,&44,&7C,&44,&44,&EE,&00,&00,&00,&00,&00 ;  ; 65 A
 = &00,&00,&7E,&84,&84,&84,&7C,&84,&84,&84,&7E,&00,&00,&00,&00,&00 ;  ; 66 B
 = &00,&00,&78,&C4,&82,&02,&02,&02,&02,&84,&78,&00,&00,&00,&00,&00 ;  ; 67 C
 = &00,&00,&7E,&84,&84,&84,&84,&84,&84,&84,&7E,&00,&00,&00,&00,&00 ;  ; 68 D
 = &00,&00,&FE,&84,&84,&24,&3C,&24,&84,&84,&FE,&00,&00,&00,&00,&00 ;  ; 69 E
 = &00,&00,&FE,&84,&84,&24,&3C,&24,&04,&04,&1E,&00,&00,&00,&00,&00 ;  ; 70 F
 = &00,&00,&78,&C4,&82,&02,&02,&F2,&82,&84,&78,&00,&00,&00,&00,&00 ;  ; 71 G
 = &00,&00,&EE,&44,&44,&44,&7C,&44,&44,&44,&EE,&00,&00,&00,&00,&00 ;  ; 72 H
 = &00,&00,&7C,&10,&10,&10,&10,&10,&10,&10,&7C,&00,&00,&00,&00,&00 ;  ; 73 I
 = &00,&00,&F0,&40,&40,&40,&40,&40,&42,&42,&3C,&00,&00,&00,&00,&00 ;  ; 74 J
 = &00,&00,&EE,&44,&24,&14,&0C,&14,&24,&44,&CE,&00,&00,&00,&00,&00 ;  ; 75 K
 = &00,&00,&0E,&04,&04,&04,&04,&04,&84,&84,&FE,&00,&00,&00,&00,&00 ;  ; 76 L
 = &00,&00,&C3,&66,&66,&5A,&5A,&42,&42,&42,&E7,&00,&00,&00,&00,&00 ;  ; 77 M
 = &00,&00,&E6,&4C,&4C,&5C,&54,&74,&64,&64,&4E,&00,&00,&00,&00,&00 ;  ; 78 N
 = &00,&00,&7C,&82,&82,&82,&82,&82,&82,&82,&7C,&00,&00,&00,&00,&00 ;  ; 79 O
 = &00,&00,&7E,&84,&84,&84,&7C,&04,&04,&04,&0E,&00,&00,&00,&00,&00 ;  ; 80 P
 = &00,&00,&38,&44,&82,&82,&82,&82,&82,&6C,&90,&6C,&00,&00,&00,&00 ;  ; 81 Q
 = &00,&00,&7E,&84,&84,&84,&7C,&44,&84,&84,&8E,&00,&00,&00,&00,&00 ;  ; 82 R
 = &00,&00,&BC,&C2,&82,&04,&38,&40,&82,&86,&7A,&00,&00,&00,&00,&00 ;  ; 83 S
 = &00,&00,&FE,&92,&92,&10,&10,&10,&10,&10,&38,&00,&00,&00,&00,&00 ;  ; 84 T
 = &00,&00,&E7,&42,&42,&42,&42,&42,&42,&42,&3C,&00,&00,&00,&00,&00 ;  ; 85 U
 = &00,&00,&E7,&42,&42,&42,&42,&42,&42,&24,&18,&00,&00,&00,&00,&00 ;  ; 86 V
 = &00,&00,&E7,&42,&42,&42,&42,&5A,&5A,&66,&66,&00,&00,&00,&00,&00 ;  ; 87 W
 = &00,&00,&E7,&42,&24,&24,&18,&24,&24,&42,&E7,&00,&00,&00,&00,&00 ;  ; 88 X
 = &00,&00,&EE,&44,&44,&28,&10,&10,&10,&10,&38,&00,&00,&00,&00,&00 ;  ; 89 Y
 = &00,&00,&FE,&82,&42,&20,&10,&08,&84,&82,&FE,&00,&00,&00,&00,&00 ;  ; 90 Z
 = &00,&00,&3C,&0C,&0C,&0C,&0C,&0C,&0C,&0C,&3C,&00,&00,&00,&00,&00 ;  ; 91 [
 = &00,&00,&02,&06,&0E,&1C,&38,&70,&E0,&C0,&80,&00,&00,&00,&00,&00 ;  ; 92 \
 = &00,&00,&3C,&30,&30,&30,&30,&30,&30,&30,&3C,&00,&00,&00,&00,&00 ;  ; 93 ]
 = &10,&38,&6C,&C6,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 94 ^
 = &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&FF,&00,&00,&00,&00 ;  ; 95 _
 = &18,&18,&30,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 96 `
 = &00,&00,&00,&00,&00,&7C,&40,&7C,&42,&42,&BC,&00,&00,&00,&00,&00 ;  ; 97 a
 = &00,&00,&06,&04,&04,&7C,&84,&84,&84,&8C,&76,&00,&00,&00,&00,&00 ;  ; 98 b
 = &00,&00,&00,&00,&00,&7C,&82,&02,&02,&82,&7C,&00,&00,&00,&00,&00 ;  ; 99 c
 = &00,&00,&60,&40,&40,&78,&44,&42,&42,&62,&DC,&00,&00,&00,&00,&00 ;  ; 100 d
 = &00,&00,&00,&00,&00,&7C,&82,&FE,&02,&82,&7C,&00,&00,&00,&00,&00 ;  ; 101 e
 = &00,&00,&78,&04,&04,&04,&3E,&04,&04,&04,&1E,&00,&00,&00,&00,&00 ;  ; 102 f
 = &00,&00,&00,&00,&00,&FC,&42,&42,&42,&7C,&40,&40,&3C,&00,&00,&00 ;  ; 103 g
 = &00,&00,&06,&04,&04,&34,&4C,&44,&44,&44,&EE,&00,&00,&00,&00,&00 ;  ; 104 h
 = &00,&00,&00,&30,&00,&38,&20,&20,&20,&20,&F8,&00,&00,&00,&00,&00 ;  ; 105 i
 = &00,&00,&00,&60,&00,&70,&40,&40,&40,&40,&40,&3C,&00,&00,&00,&00 ;  ; 106 j
 = &00,&00,&06,&04,&E4,&44,&24,&1C,&24,&44,&EE,&00,&00,&00,&00,&00 ;  ; 107 k
 = &00,&00,&38,&20,&20,&20,&20,&20,&20,&20,&F8,&00,&00,&00,&00,&00 ;  ; 108 l
 = &00,&00,&00,&00,&00,&67,&5A,&5A,&42,&42,&C7,&00,&00,&00,&00,&00 ;  ; 109 m
 = &00,&00,&00,&00,&00,&36,&4C,&44,&44,&44,&EE,&00,&00,&00,&00,&00 ;  ; 110 n
 = &00,&00,&00,&00,&00,&7C,&82,&82,&82,&82,&7C,&00,&00,&00,&00,&00 ;  ; 111 o
 = &00,&00,&00,&00,&00,&76,&8C,&84,&84,&7C,&04,&04,&1E,&00,&00,&00 ;  ; 112 p
 = &00,&00,&00,&00,&00,&DC,&62,&42,&42,&7C,&40,&40,&F0,&00,&00,&00 ;  ; 113 q
 = &00,&00,&00,&00,&00,&76,&4C,&04,&04,&04,&1E,&00,&00,&00,&00,&00 ;  ; 114 r
 = &00,&00,&00,&00,&00,&7C,&C2,&1C,&70,&C6,&7C,&00,&00,&00,&00,&00 ;  ; 115 s
 = &00,&00,&08,&08,&08,&7E,&08,&08,&08,&88,&70,&00,&00,&00,&00,&00 ;  ; 116 t
 = &00,&00,&00,&00,&00,&66,&44,&44,&44,&64,&DC,&00,&00,&00,&00,&00 ;  ; 117 u
 = &00,&00,&00,&00,&00,&E7,&42,&42,&42,&24,&18,&00,&00,&00,&00,&00 ;  ; 118 v
 = &00,&00,&00,&00,&00,&E7,&42,&5A,&5A,&66,&66,&00,&00,&00,&00,&00 ;  ; 119 w
 = &00,&00,&00,&00,&00,&EE,&44,&38,&28,&44,&EE,&00,&00,&00,&00,&00 ;  ; 120 x
 = &00,&00,&00,&00,&00,&EE,&44,&44,&44,&28,&10,&08,&1E,&00,&00,&00 ;  ; 121 y
 = &00,&00,&00,&00,&00,&FE,&82,&60,&18,&84,&FE,&00,&00,&00,&00,&00 ;  ; 122 z
 = &00,&00,&70,&18,&18,&18,&0E,&18,&18,&18,&70,&00,&00,&00,&00,&00 ;  ; 123 {
 = &00,&00,&18,&18,&18,&18,&00,&18,&18,&18,&18,&00,&00,&00,&00,&00 ;  ; 124 |
 = &00,&00,&0E,&18,&18,&18,&38,&18,&18,&18,&0E,&00,&00,&00,&00,&00 ;  ; 125 }
 = &00,&00,&DC,&76,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00 ;  ; 126 ~
 = &00,&00,&00,&00,&00,&10,&38,&6C,&C6,&C6,&C6,&FE,&00,&00,&00,&00 ;  ; 127 
	]	; EOF (fontcode)

	; ---------------------------------------------------------------------

ScreenImage	; running calvin (16bytes-by-130rasters)
        =       &00,&00,&00,&00,&00,&00,&00,&00,&80,&01,&00,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&00,&00,&60,&01,&00,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&00,&00,&30,&01,&80,&07,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&00,&00,&18,&01,&C0,&06,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&00,&00,&0C,&01,&30,&02,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&00,&01,&06,&01,&18,&03,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&80,&01,&83,&00,&06,&01,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&40,&81,&81,&00,&83,&01,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&30,&C1,&80,&80,&80,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&18,&41,&80,&40,&80,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&18,&21,&80,&30,&40,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&08,&11,&80,&18,&40,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&0C,&19,&80,&0C,&20,&00,&0F,&00,&00,&00
        =       &00,&00,&00,&00,&00,&04,&04,&09,&80,&07,&20,&C0,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&06,&06,&05,&00,&01,&20,&38,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&02,&02,&03,&00,&00,&00,&18,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&02,&03,&01,&00,&00,&00,&34,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&82,&01,&00,&00,&00,&00,&22,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&82,&01,&00,&E0,&01,&3C,&20,&00,&00,&00,&00
        =       &00,&00,&00,&00,&08,&83,&00,&00,&1E,&00,&03,&40,&00,&00,&00,&00
        =       &00,&00,&00,&00,&18,&87,&00,&80,&03,&C0,&00,&40,&00,&00,&00,&00
        =       &00,&00,&00,&00,&10,&06,&00,&60,&00,&30,&00,&80,&18,&00,&00,&00
        =       &00,&00,&00,&00,&10,&06,&00,&30,&00,&08,&00,&80,&0F,&00,&00,&00
        =       &00,&00,&00,&00,&20,&06,&00,&18,&00,&00,&00,&80,&00,&00,&00,&00
        =       &00,&00,&00,&00,&20,&06,&00,&0C,&00,&00,&00,&C0,&00,&00,&00,&00
        =       &00,&00,&00,&00,&60,&0E,&00,&04,&00,&00,&00,&20,&01,&00,&00,&00
        =       &00,&00,&00,&00,&60,&02,&00,&00,&00,&00,&00,&20,&01,&00,&00,&00
        =       &00,&00,&00,&00,&C0,&02,&00,&00,&00,&00,&00,&00,&01,&00,&00,&00
        =       &00,&00,&00,&00,&C0,&00,&00,&00,&00,&00,&00,&00,&01,&00,&00,&00
        =       &00,&00,&00,&00,&C0,&00,&00,&00,&00,&00,&00,&00,&01,&00,&00,&00
        =       &00,&00,&00,&00,&C2,&00,&00,&00,&00,&00,&00,&00,&01,&00,&00,&00
        =       &00,&00,&00,&00,&06,&00,&00,&00,&00,&00,&00,&00,&01,&00,&00,&00
        =       &00,&00,&00,&00,&04,&00,&00,&00,&00,&00,&00,&00,&01,&00,&00,&00
        =       &00,&00,&00,&00,&08,&00,&00,&00,&00,&00,&00,&00,&01,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&01,&00,&00,&00
        =       &00,&00,&00,&00,&30,&00,&00,&00,&00,&00,&00,&00,&01,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&02,&00,&00,&00,&00,&00,&01,&00,&00,&00
        =       &00,&00,&00,&00,&01,&00,&09,&00,&00,&00,&00,&0C,&01,&00,&00,&00
        =       &00,&00,&00,&00,&0F,&80,&04,&00,&00,&00,&0C,&1E,&01,&00,&00,&00
        =       &00,&00,&00,&00,&38,&40,&02,&00,&00,&00,&1E,&12,&01,&00,&0F,&00
        =       &00,&00,&00,&00,&60,&20,&01,&00,&00,&00,&12,&96,&81,&FB,&10,&00
        =       &00,&00,&00,&00,&40,&80,&00,&00,&00,&00,&16,&9E,&C0,&64,&10,&00
        =       &00,&00,&00,&00,&40,&00,&00,&00,&00,&00,&1E,&7F,&40,&32,&08,&00
        =       &00,&00,&00,&00,&20,&00,&00,&00,&00,&00,&9E,&C1,&60,&12,&0C,&00
        =       &00,&00,&00,&00,&30,&00,&00,&00,&00,&00,&0C,&80,&21,&00,&06,&00
        =       &00,&00,&00,&00,&18,&03,&00,&00,&00,&00,&00,&00,&61,&00,&02,&00
        =       &00,&00,&00,&00,&00,&06,&00,&00,&00,&00,&00,&00,&41,&00,&04,&00
        =       &00,&00,&00,&00,&00,&18,&00,&00,&00,&00,&00,&00,&C1,&00,&0C,&00
        =       &00,&00,&00,&00,&00,&18,&00,&00,&00,&00,&00,&00,&61,&00,&18,&00
        =       &00,&00,&00,&00,&00,&0C,&00,&00,&00,&00,&00,&00,&31,&00,&30,&00
        =       &00,&00,&00,&00,&00,&16,&00,&00,&00,&00,&00,&80,&1B,&F0,&20,&00
        =       &00,&00,&00,&00,&00,&03,&00,&00,&00,&00,&00,&80,&0E,&98,&21,&00
        =       &00,&00,&00,&00,&00,&01,&00,&00,&00,&00,&00,&E0,&19,&0C,&1F,&00
        =       &00,&00,&00,&00,&80,&01,&00,&00,&00,&00,&00,&3C,&63,&07,&00,&00
        =       &00,&00,&00,&00,&80,&01,&00,&00,&00,&00,&00,&00,&C6,&01,&00,&00
        =       &00,&00,&3C,&00,&00,&01,&00,&00,&00,&00,&00,&00,&8C,&00,&00,&00
        =       &00,&00,&E6,&00,&00,&01,&00,&00,&F0,&FF,&FF,&1F,&84,&01,&00,&00
        =       &00,&00,&83,&3F,&00,&01,&00,&00,&F8,&FF,&FF,&3F,&0C,&01,&00,&00
        =       &00,&00,&03,&E0,&01,&06,&01,&00,&FC,&FF,&FF,&3F,&16,&01,&00,&00
        =       &00,&00,&06,&00,&01,&84,&03,&00,&7E,&E2,&FF,&0F,&A3,&01,&00,&00
        =       &00,&00,&0C,&00,&07,&F8,&04,&00,&7E,&C0,&FF,&87,&A1,&00,&00,&00
        =       &00,&00,&F8,&01,&1C,&00,&0C,&00,&3E,&C0,&FF,&E3,&C2,&00,&00,&00
        =       &00,&00,&90,&01,&60,&F0,&18,&00,&7E,&E0,&FF,&30,&44,&00,&00,&00
        =       &00,&00,&B0,&00,&80,&31,&37,&00,&FE,&FF,&3F,&4C,&34,&00,&00,&00
        =       &00,&00,&C0,&60,&00,&1E,&47,&00,&FE,&FF,&0F,&86,&18,&00,&00,&00
        =       &00,&00,&80,&B0,&01,&8C,&99,&01,&FE,&FF,&01,&0B,&1D,&00,&00,&00
        =       &00,&00,&80,&11,&07,&8C,&69,&02,&FC,&7F,&80,&11,&0D,&00,&00,&00
        =       &00,&00,&00,&1F,&1C,&C4,&98,&0D,&F0,&07,&C0,&E2,&02,&00,&00,&00
        =       &00,&00,&00,&00,&70,&64,&8C,&3A,&00,&00,&70,&62,&00,&00,&00,&00
        =       &00,&00,&00,&00,&C0,&27,&CC,&E0,&01,&00,&18,&1C,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&33,&C6,&07,&07,&00,&86,&0C,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&1B,&23,&F8,&7D,&C0,&83,&07,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&8B,&31,&00,&F7,&0F,&83,&01,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&8D,&19,&00,&E0,&FF,&C1,&01,&00,&00,&00,&00
        =       &00,&00,&00,&00,&80,&C7,&FC,&01,&00,&00,&F0,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&80,&67,&04,&FE,&03,&00,&58,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&7F,&03,&00,&FC,&3F,&0C,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&FC,&01,&00,&00,&C0,&0F,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&C0,&00,&00,&00,&00,&04,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&E0,&FF,&01,&00,&00,&04,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&30,&00,&FE,&3F,&00,&06,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&10,&00,&00,&C0,&FF,&03,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&18,&00,&00,&00,&00,&01,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&18,&00,&00,&00,&80,&00,&00,&3F,&00,&00,&00
        =       &00,&00,&00,&00,&00,&0C,&00,&E0,&FF,&FF,&00,&C0,&FF,&00,&00,&00
        =       &00,&00,&00,&00,&00,&FE,&FF,&1F,&00,&40,&00,&60,&80,&01,&00,&00
        =       &00,&00,&00,&00,&00,&03,&00,&00,&00,&60,&00,&18,&00,&02,&00,&00
        =       &00,&00,&00,&00,&00,&01,&00,&00,&00,&20,&00,&0E,&00,&04,&00,&00
        =       &00,&00,&00,&00,&C0,&00,&00,&00,&C0,&3F,&00,&0B,&00,&04,&00,&00
        =       &00,&00,&00,&00,&C0,&00,&00,&FC,&3F,&10,&00,&05,&00,&08,&00,&00
        =       &00,&00,&00,&00,&60,&C0,&FF,&03,&00,&18,&00,&05,&00,&08,&00,&00
        =       &00,&00,&00,&00,&F0,&3F,&00,&00,&00,&18,&00,&05,&00,&08,&00,&00
        =       &00,&00,&00,&00,&30,&00,&00,&00,&C0,&0F,&00,&05,&00,&08,&00,&00
        =       &00,&00,&00,&38,&18,&00,&00,&00,&3F,&0C,&00,&05,&00,&08,&00,&00
        =       &00,&00,&00,&CF,&08,&00,&00,&FC,&00,&EC,&0F,&09,&00,&08,&00,&00
        =       &00,&00,&80,&70,&09,&00,&F8,&03,&80,&F3,&1F,&09,&00,&08,&00,&00
        =       &00,&00,&60,&1C,&0F,&E0,&07,&00,&E0,&F9,&3F,&09,&00,&08,&00,&00
        =       &00,&00,&30,&1F,&86,&1F,&00,&00,&F8,&FC,&FF,&09,&00,&08,&00,&00
        =       &00,&00,&08,&23,&7F,&00,&00,&00,&7C,&FE,&FF,&09,&00,&08,&00,&00
        =       &00,&00,&84,&22,&02,&00,&00,&00,&FF,&FF,&FF,&0D,&00,&04,&00,&00
        =       &00,&00,&C6,&A2,&03,&00,&00,&F8,&FF,&FF,&FF,&FE,&00,&04,&00,&00
        =       &00,&00,&63,&9C,&00,&00,&E0,&FF,&FF,&FF,&7F,&02,&3F,&04,&00,&00
        =       &00,&00,&A1,&C1,&00,&80,&FF,&FF,&FF,&FF,&7F,&FF,&C0,&03,&00,&00
        =       &00,&80,&11,&C1,&00,&FE,&FF,&FF,&FF,&FF,&7F,&01,&3F,&02,&00,&00
        =       &00,&80,&50,&C6,&F8,&FF,&FF,&FF,&FF,&FF,&FF,&00,&C0,&03,&00,&00
        =       &00,&40,&88,&F8,&FF,&FF,&FF,&FF,&FF,&FF,&7F,&00,&00,&02,&00,&00
        =       &00,&40,&08,&E7,&FF,&FF,&FF,&FF,&FF,&FF,&7F,&00,&00,&01,&00,&00
        =       &00,&20,&18,&FC,&FF,&FF,&FF,&FF,&7F,&00,&5C,&00,&00,&01,&00,&00
        =       &00,&20,&64,&E0,&FF,&FF,&FF,&FF,&0F,&00,&58,&00,&80,&00,&00,&00
        =       &00,&10,&84,&EF,&FF,&FF,&FF,&FF,&00,&00,&68,&00,&80,&00,&00,&00
        =       &00,&10,&04,&E8,&FF,&FF,&FF,&3F,&00,&00,&70,&00,&40,&00,&00,&00
        =       &00,&10,&FE,&F9,&FF,&FF,&FF,&03,&00,&00,&30,&00,&20,&00,&00,&00
        =       &00,&10,&02,&FE,&FF,&FF,&FF,&00,&00,&00,&30,&00,&30,&00,&00,&00
        =       &00,&10,&00,&18,&FF,&FF,&0F,&00,&00,&00,&20,&00,&18,&00,&00,&00
        =       &00,&10,&00,&10,&FF,&FF,&01,&00,&00,&00,&20,&00,&06,&00,&00,&00
        =       &00,&10,&00,&20,&FC,&7F,&00,&00,&00,&00,&C0,&C0,&01,&00,&00,&00
        =       &00,&10,&00,&10,&00,&00,&00,&00,&00,&00,&00,&3F,&00,&00,&00,&00
        =       &00,&20,&00,&10,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00
        =       &00,&60,&00,&08,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00
        =       &00,&80,&00,&F4,&1F,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00
        =       &00,&C0,&03,&FF,&FF,&FF,&01,&00,&00,&00,&00,&00,&00,&00,&00,&00
        =       &00,&F0,&FF,&FF,&FF,&FF,&7F,&00,&00,&00,&00,&00,&00,&00,&00,&00
        =       &80,&FF,&FF,&FF,&FF,&FF,&FF,&FF,&FF,&01,&00,&00,&00,&00,&00,&00
        =       &C0,&C0,&FF,&C0,&FF,&FF,&FF,&FF,&FF,&FF,&03,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&00,&FF,&FF,&FF,&FF,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&00,&E0,&FF,&FF,&FF,&FF,&7F,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&00,&00,&80,&FF,&FF,&FF,&FF,&FF,&FF,&FF
        =       &FF,&01,&00,&00,&00,&00,&00,&00,&C0,&C0,&FF,&C0,&FF,&FF,&FF,&FF
        =       &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00
        =       &00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00,&00

	; ---------------------------------------------------------------------
        END
