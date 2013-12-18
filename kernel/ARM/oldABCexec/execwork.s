        SUBT Executive workspace definitions                       > execwork/s
        ;    Copyright (c) 1989, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; System workspace definitions
        ;
	; Notes:
	; 	The current memory system uses the memory un-mapped at 32MB.
	;	This means that the Executive workspace is high in memory and
	;	all references are via literal addresses.
	;
	;	At the moment the following code sequence is used a lot:
	;		LDR	<reg>,=<location>
	;		LDR	<data>,[<reg>,#&00]
	;
	;	This could be optimised by using:
	;		MOV	<reg>,#<workspace base>	; 8bit constant
	;		LDR	<data>,[<reg>,#<offset>]
	;
	;	Luckily 32MB is a 8bit constant value.
	;
	;	We will not be able to use Hercules memory mapping due to
	;	the page-boundary-mapping bug, so the complete system will
	;	execute in physical mode.
        ; ---------------------------------------------------------------------

	ASSERT	(exstruct_s)	; ensure "exstruct.s" is included

old_opt SETA    {OPT}
        OPT     (opt_on)       

        ; ---------------------------------------------------------------------

	!	0,"Including execwork.s"
		GBLL	execwork_s
execwork_s	SETL	{TRUE}

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

	[	(hercules :LOR: activebook)
dataRAMbase	*	RAM_base
	|
dataRAMbase	*	dataRAMblk
	]	; (hercules :LOR: activebook)

        ; ---------------------------------------------------------------------
        ; The first few words in the FastRAM hold the vectors.
        ; There follows some space for a small FIQ routine.
        ; An allocation of 100 words is available for users who need fast
	; memory. The end of the FastRAM area is allocated to the Executive.

        ; The undefined instruction software vector is here rather than
        ; with the other software vectors so that it is addressable
        ; by LDR from the hardware vector (at &00000004).
	[	{TRUE}
	[	(activebook)
FastRAM_size	*	(SRAM_size)	; fixed by the processor hardware
	|	; middle (activebook)
FastRAM_size	*	(512)		; bytes (128 words)
	]	; EOF (activebook)

software_vector_undef	*	(FastRAM_size - 4)	; last word of FastRAM
	|
software_vector_undef	*	&000001FC
	]	; EOF {boolean}
	; During POR (Power On Reset) the ROM is mapped completely throughout
	; the world (this includes covering the FastRAM). To map the ROM out
	; two words need to be written over a page boundary. This means the
	; BTZ (Branch Through Zero) and the last word in the FastRAM are
	; corrupted (since they cannot be read).
	;
	[	(speedup)
	; At the moment FindExecRoot, GetRoot and GetSysBase take a relatively
	; long time to execute. The SWI handler entry eats about 80cycles. The
	; system is currently constructed so that everything above the
	; loExecutive is ARM hardware architecture independant. The
	; hiExecutive, Kernel and all other Helios/ARM BINARIES are currently
	; portable across any ARM system providing the Helios/ARM loExecutive.
	; This is achieved by all calls to the loExecutive being performed via
	; SWIs. Unfortunately this does entail a sizeable hit for frequently
	; called operations (such as those above). The ARM processor has eight
	; (8) fixed address vectors: reset/BTZ, undefined instruction, SWI,
	; prefetch abort, data abort, address exception, IRQ and FIQ. All of
	; these vectors normally contain instructions to branch to a handler
	; for the particular hardware event. The FIQ vector is a special case,
	; in that the code is usually copied directly to the FIQ vector
	; address to avoid the overhead of a pipe-line break on FIQ entry.
	; Since any reasonable ARM hardware implementation would have RAM in
	; the low-address area to allow vectors to be dynamically updated, and
	; for various FIQ handlers to be copied down we could use this argument
	; to modify the system to provide quicker access to the structures
	; mentioned at the start of this paragraph. To minimise the amount of
	; FastRAM "wasted" on a Hercules system, we could place a single vector
	; into the low-address RAM. This vector would give the address of the
	; ExecRoot structure. This can then be used to directly access the
        ; ExecRoot structure. The other structures would be held in locations
	; at fixed offsets within the ExecRoot structure.
	; Accessing these structures could then be performed via MACROs that
	; explicitly loaded the addresses from the fixed vector addresses. This
	; would cut the calls from around 100 instructions to the following:
	;   MOV  rN,#special_low_address_vector ; address special vector
	;   LDR  rN,[rN,#&00]                   ; load address of ExecRoot
	;   LDR  rN,[rN,#ExecRoot_xxxxxx]	; load required address
	;
	; Having this double indirection means that a minimal amount ot the
	; FastRAM is used. Since ideally code should be the major user of the
	; FastRAM.
	;
	; This is a NASTY constant that will be exported outside the Executive.
	; It should stay FIXED for all hardware versions of Helios/ARM.
fast_structure_ptr	*	(software_vector_undef - 4)
	!	0,"fast_structure_pointer at &" :CC: (:STR: fast_structure_ptr)
	;
	; At the moment the MACROs in "root.h" still call functions in the
	; hiExecutive. These should be changed to directly load the address
	; from the fixed address vector, and indirect through to the correct
	; address location.
	]	; EOF (speedup)

user_fast_ram_size	*	(100 * 4)	; user allocated FastRAM
	[	(speedup)
	; There are now two words required by the Executive at the end of the
	; FastRAM.
user_fast_ram_end	*	fast_structure_ptr
	|	; middle (speedup)
user_fast_ram_end	*	software_vector_undef
	]	; EOF (speedup)
user_fast_ram_base	*	(user_fast_ram_end - user_fast_ram_size)

fast_FIQ_base		*	vec_FIQ
fast_FIQ_end		*	user_fast_ram_base

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Executive workspace base address:
	; This is a NASTY use of an ABSOLUTE address.

systemWSP	*	dataRAMbase

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
	; These values are pretty arbitrary at the moment. Users of the IRQ
	; stack (interrupt handlers only) should be careful (ie. no allocating
	; large local arrays, or recursive functions).
	;
	; The IRQ stack now has to support upto "num_IRQs" active IRQ
	; frames (since the IRQ handler is re-entrant).
size_IRQ_stack	*	&00001000	; 1024 words of IRQ stack
size_IRQ_entry	*	(5 * word)	;    5 words for the IRQ entry code

        ; ---------------------------------------------------------------------

			^	&00	; offset into "hardware_regs"
MEMMAP_data             #       word    ; byte
CLOCK_data              #       word    ; byte
BANK0_data              #       word    ; byte
BANK1_data              #       word    ; byte
BANK2_data              #       word    ; byte
BANK3_data              #       word    ; byte
BANK4_data              #       word    ; byte
BANK5_data              #       word    ; byte
BANK6_data              #       word    ; byte
BANK7_data              #       word    ; byte
INTtest_data		#	word	; halfword
IRQ_data                #       word    ; halfword
FIQ_data                #       word    ; halfword
TIMER_data              #       word    ; byte
LCDcontrol_data         #       word    ; word
LCDlinelength_data	#	word	; byte
LCDlinerate_data	#	word	; byte
LCDnumlines_data	#	word	; byte
DMArouting_data		#	word	; halfword
Control_data		#	word	; byte
hardware_regs_list_end	#	&00

        ; ---------------------------------------------------------------------
        ; This is the main system workspace
	!	0,"TODO: place all data locations into the ExecRoot structure"

                        ^       systemWSP
workspace_start         #       &00

        ; ---------------------------------------------------------------------
        ; software vectors
        ; ----------------
        ; The vectors simply hold the destination addresses.
        ; Before initialisation all the vectors should point to the system
        ; error handler.
        ; The vectors are taken by the code sequence:
        ;       LDR     pc,[<<software_vectors>>,#(<<index>> * &04)]
software_vectors        #       (number_of_vectors * &04)

        ; ---------------------------------------------------------------------

fp_old_vector           #       word	; old "undefined instruction" vector

        ; ---------------------------------------------------------------------

        ; temporary location used during SWI entry
entryword               #       &04

        ; ---------------------------------------------------------------------

register_dump           #       &00     ; contents at instance of exception
dumped_r0               #       &04
dumped_r1               #       &04
dumped_r2               #       &04
dumped_r3               #       &04
dumped_r4               #       &04
dumped_r5               #       &04
dumped_r6               #       &04
dumped_r7               #       &04
dumped_r8               #       &04
dumped_r9               #       &04
dumped_r10              #       &04
dumped_r11              #       &04
dumped_r12              #       &04
dumped_r13              #       &04
dumped_r14              #       &04     ; r14 (if not in SVC mode at the time)
dumped_r15              #       &04     ; PC and PSR at time of exception
        ; If the exception occurs while in SVC mode, the "svc_r14" register
        ; will have been corrupted. In such cases the dumped "r14" is set to
        ; the value &DEADDEAD

        ; ---------------------------------------------------------------------
        ; -- Helios Executive static workspace --------------------------------
        ; ---------------------------------------------------------------------
        ; "ExecRoot" structure (layout defined in header file "exstruct.s")

ROOT_start              #       (ExecRoot_size)			; statics
                        #       (ProcessQ_size * NumberPris)	; queues

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; --------- Machine/Processor variants after this point only ----------
	; ------- NO processor/machine conditionals should occur above --------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

link_IRQ_workspace      #       &00
; -- Tx --
txbuffer_address        #       &04     ; transmit data buffer address
txbuffer_count          #       &04     ; bytes waiting to send
txbuffer_savestate      #       &04     ; blocked process state
; -- Rx --
rxbuffer_address        #       &04     ; receive data buffer address
rxbuffer_count          #       &04     ; bytes waiting for
rxbuffer_savestate      #       &04     ; blocked process state
end_link_IRQ_workspace  #       &00

        ; ---------------------------------------------------------------------
	; Workspace used by microlink interrupt routines (or link0 
	; substitute on functional prototype).

ML_Workspace_Start	#	&00

ML_TxBuf_Ptr		#	&04	; addr of next byte to be transmitted
ML_TxBuf_End		#	&04	; addr of byte after last to be sent
ML_TxBuf_SaveState	#	&04	; addr of SaveState to be resumed

ML_RxBuf_Ptr		#	&04	; addr of next byte for reception
ML_RxBuf_End		#	&04	; addr of byte after last in message

        [       (hercules :LOR: activebook)
	; Real microlink
	; Two reception buffers are used, so the FIQ handler can receive
	; a new message while the IRQ handler processes the previous one.
	; The contents of ML_Rx_CurBuf and ML_Rx_OtherBuf are swapped
	; after each message is received.
ML_Rx_CurBuf		#	&04	; start addr of rx buf being filled
ML_Rx_OtherBuf		#	&04	; start addr of other rx buf
ML_Rx_IRQArg		#	&04	; argument from FIQ rtn to IRQ rtn

        |       ; middle (hercules :LOR: activebook)

	; On the functional prototype, transputer link 0 is used as a
	; substitute for microlink. Only IRQ is used for reception, and
	; there is only one buffer.
ML_SendSoftBreak	#	&04	; set when soft break msg to be sent
        ]       ; EOF (hercules :LOR: activebook)

ML_MaxRxReqs		*	 5	; max simultaneous reception requests

ML_RxRequests		#	(ML_RxReq_sizeof * ML_MaxRxReqs); array base
ML_RxRequestsEnd	#	&00	; end of rx requests array

ML_MsgHdlrList		#	&04	; linked list of msg handler structs

ML_SerialNo		#	&04	; serial number to make handles unique

ML_State		#	&04	; current driver state (0 = `normal')

	; Buffers used for receiving msgs: ensure length is a whole 
	; number of words to avoid misalignment of following variables.
	; Note that the microlink server task assumes that these buffers
	; begin on a word boundary, so expect trouble if you misalign it!
ML_RxBuffer1		#	((ML_MaxMsgLen+3)/4)*4
ML_RxBuffer2		#	((ML_MaxMsgLen+3)/4)*4

	; Message handler descriptors used by Executive
ML_PowerFailBuffer	#	ML_MsgHdlr_sizeof

ML_Workspace_End	#	&00

        ; ---------------------------------------------------------------------
        ; -- DMA support ------------------------------------------------------
        ; ---------------------------------------------------------------------

	[	(hercules)
	; Each DMA channel has an allocation word. The top bit (bit31)
	; signifies the state of this channel. The remaining 31bits are
	; used as an access count (to generate the "unique" allocation
	; handles).
DMAchannel_allocated	bit	31	; Set if DMA channel in use
DMAchannel_states	#	(DMAchannel_maximum * word)
	]	; EOF (hercules)

        ; ---------------------------------------------------------------------
        ; -- External interrupt support ---------------------------------------
        ; ---------------------------------------------------------------------

	[	(hercules)
	; Each external interrupt source has an allocation word. The top bit
	; (bit31) signifies the state of this channel. The remaining 31bits are
	; used as an access count (to generate the "unique" allocation
	; handles).
extsource_allocated	bit	31	; Set if external source in use
extsource_states	#	(extsourceMAX * word)
	]	; EOF (hercules)

        ; ---------------------------------------------------------------------
        ; -- Processor re-start -----------------------------------------------
        ; ---------------------------------------------------------------------

	[	((activebook) :LAND: (shutdown))
	; This word is used (along with RestartId2) to validate the contents
	; of the "CompleteState" structure. The value
RestartId1	#	word		; "Identity" word
	; This is held as a constant allocation in the Executive since we
	; cannot afford an allocation failure during the PowerFail code.
RestartState	#	CompleteState_sizeof
	; This word is used (along with RestartId1) to validate the contents
	; of the "CompleteState" structure.
RestartId2	#	word		; "Identity" word
	;
	; If either location contains the value "InvalidRestartMarker" then
	; the "CompleteState" information is invalid. If the locations contain
	; different values then the "CompleteState" information is invalid. The
	; ID locations should contain the processor centi-second timer at the
	; point of PowerDown.
InvalidRestartMarker	*	&C0DEDEAD
	]	; EOF ((activebook) :LAND: (shutdown))

        ; ---------------------------------------------------------------------
	!	0,"TODO: remove DummySpace allocation"
	[	{TRUE}
	; Temporary debugging space (when we cannot use the standard stack)
DummySpace_bottom	#	(2 * 1024)	; startup space for debugging
DummySpace		#	&00
	]	; EOF {boolean}

        ; ---------------------------------------------------------------------

        [       (hercules :LOR: activebook)
        ; Executive variables:
        ;  This is a contiguous list of exported Executive variables.
	;  (The means of exporting these variables is still to be coded)

software_regs           #       null

sysRAMtop               #       word    ; end of system RAM + 1
userRAMbase             #       word    ; base address of user RAM
userRAMtop              #       word    ; end of user RAM + 1

software_regs_end       #       null

        ; Hercules registers:
        ;  This is a contiguous list of soft-copy values for the write-only
	;  locations.
	;
	; start of the data/address pairs
hardware_regs           #       hardware_regs_list_end
hardware_regs_end       #       null    ; end of the data/address pairs

        |       ; middle (hercules :LOR: activebook)
sysRAMtop               #       &04     ; end of system RAM + 1
userRAMbase             #       &04     ; base address of user RAM
userRAMtop              #       &04     ; end of user RAM + 1

        ; Soft copies of write-only interrupt mask registers (which
        ; are really only one byte in size in the functional prototype).

IRQ_mask_copy           #       &04     ; Soft copy of IRQ mask register
FIQ_mask_copy           #       &04     ; Soft copy of FIQ mask register
        ]       ; EOF (hercules :LOR: activebook)

	[	(hercules :LOR: activebook)
num_IRQs		*	15		; Hercules IRQ sources
	|
num_IRQs                *       &00000010       ; we support 16 IRQ sources
	]	; (hercules :LOR: activebook)

        ; branch table to IRQ interrupt handlers
IRQ_vectors             #       (num_IRQs * &04)
IRQ_vecs_end            #       &00

        ; ---------------------------------------------------------------------

	; In this system we do NOT use the IVecISize field, however the
	; code that accesses the IVec structure assumes that the word is
	; present.
SYSBASE_start	#	word			; IVecISize word
		#	(IVecTotal * word)	; IVec RPTRs
		#	word			; terminating NULL

        ; ---------------------------------------------------------------------
        ; -- FIQ workspace ----------------------------------------------------
        ; ---------------------------------------------------------------------

single_FIQ_code	*	fast_FIQ_base	; base address of FIQ handler code
top_FIQ_stack	*	fast_FIQ_end	; default FIQ r13 (sp) address
base_FIQ_stack	*	single_FIQ_code	; bottom of possible FIQ stack

	[	(hercules :LOR: activebook)
FIQ_handlers    *       FIQ_sources     ; number of FIQ handlers allowed
	|
FIQ_handlers	*	8		; number of FIQ handlers allowed
	]	; EOF (hercules :LOR: activebook)

	ASSERT	(FIQ_handlers = 8)	; the following manifests assume this
        ; Active handler workspace area (array for FIQ handler code/data)
FIQ_single		#	(word)	; 0 if multiple; non-0 if single
	; The "FIQ_single" location should be turned into a single bit in the
	; Executive workspace somewhere (a suitable "flags" word should be
	; generated).
	!	0,"TODO: optimise memory usage (FIQ_single)"
FIQ_regs		*	(6 * word)	; r8,r9,r10,r11,r12 and r13
FIQ_array_entry_size    NPOW2   (FIQ_regs + (top_FIQ_stack - single_FIQ_code))
FIQ_array               #       (FIQ_handlers * FIQ_array_entry_size)
FIQ_multiple_regs	#	(4 * word)	; used by the system FIQ router

        ; ---------------------------------------------------------------------
        ; -- Stacks -----------------------------------------------------------
        ; ---------------------------------------------------------------------

        [       (({VAR} :MOD: &0100) <> 0)
spare_space             #       (&0100 - ({VAR} :MOD: &0100))
spare_space_end         #       &00
        |
spare_space             #       &00
spare_space_end         #       &00
        ]

        ; ---------------------------------------------------------------------
        ; The IRQ stack needs to be large enough to store state while
	; discovering the interrupt source, and, if unknown, perform the
	; Helios Event handler system.

bottom_IRQ_stack        #       (size_IRQ_stack)
top_IRQ_stack           #       &00
	; The "entered_IRQ" word is used to mark wether we have an outstanding
	; SVC IRQ handler thread. If zero then we can assume there are no
	; threads active.
entered_IRQ		#	word
	; This little stack is used by the IRQ handler entry code. It is
	; purely single-threaded and must never be read/written with IRQs
	; enabled.
bottom_IRQ_entry	#	(size_IRQ_entry)
top_IRQ_entry		#	&00	; defined IRQ r13 address on IRQ entry

        ; ---------------------------------------------------------------------

end_of_workspace        #       &00

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        !       0,"execwork.s: &" :CC: (:STR: (end_of_workspace - workspace_start)) :CC: " bytes of workspace"

	; Round the workspace allocation upto the nearest 1K boundary
workspace_size	* (((end_of_workspace - dataRAMbase) + 1023) :AND: ~1023) ;

        !       0,"execwork.s: &" :CC: (:STR: workspace_size) :CC: " bytes of allocated workspace"

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF execwork.s
