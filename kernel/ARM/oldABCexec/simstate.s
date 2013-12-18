        TTL	ARM2/3 emulator manifests and data structures    > simstate/s
        ; Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom.
        ; -------------------------------------------------------------------
        ; This file describes the ARM processor hardware state and the
        ; emulation control state.
        ; -------------------------------------------------------------------
        ; condition execution masks
        ; -------------------------
c_execute       *       &10000000       ; decide if instruction is executed
c_flags         *       &E0000000       ; which flags are used
c_flags_shift   *       29              ; where the flags live in a word

        ; -------------------------------------------------------------------
        ; Emulator status flags

        ; hardware (bits 31..28)
FIQline         bit     31    ; hardware FIQ asserted to processor (inverted)
IRQline         bit     30    ; hardware IRQ asserted to processor (inverted)
        ; emulator control (bits 27..24)
doSWIs          bit     27    ; 0=emulate SWI instructions; 1=call real SWIs
verbose         bit     26    ; 0=quiet mode; 1=verbose mode
fnnames         bit     25    ; 0=nothing; 1=display C object function names
        ; unused (bits 23..0)

        ; -------------------------------------------------------------------
        ; ARM processor state description
        ; -------------------------------

        struct          "ARMstate"
        struct_vec      (15 * word),"regs"      ; current processor mode regs
        struct_word     "cPC"                   ; r15 (available all the time)
        struct_null     "USRregs"               ; USR mode r8..r14
        struct_word     "USR_r8"
        struct_word     "USR_r9"
        struct_word     "USR_r10"
        struct_word     "USR_r11"
        struct_word     "USR_r12"
        struct_word     "USR_r13"
        struct_word     "USR_r14"
        struct_word     "USR_nul"               ; padding word (easier access)
        struct_null     "FIQregs"               ; FIQ mode r8..r14
        struct_word     "FIQ_r8"
        struct_word     "FIQ_r9"
        struct_word     "FIQ_r10"
        struct_word     "FIQ_r11"
        struct_word     "FIQ_r12"
        struct_word     "FIQ_r13"
        struct_word     "FIQ_r14"
        struct_word     "FIQ_nul"               ; padding word (easier access)
        struct_null     "IRQregs"               ; IRQ mode r8..r14
        struct_word     "IRQ_r8"
        struct_word     "IRQ_r9"
        struct_word     "IRQ_r10"
        struct_word     "IRQ_r11"
        struct_word     "IRQ_r12"
        struct_word     "IRQ_r13"
        struct_word     "IRQ_r14"
        struct_word     "IRQ_nul"               ; padding word (easier access)
        struct_null     "SVCregs"               ; SVC mode r8..r14
        struct_word     "SVC_r8"
        struct_word     "SVC_r9"
        struct_word     "SVC_r10"
        struct_word     "SVC_r11"
        struct_word     "SVC_r12"
        struct_word     "SVC_r13"
        struct_word     "SVC_r14"
        struct_word     "SVC_nul"               ; padding word (easier access)
        struct_word     "FLAGS"                 ; emulator status flags
        struct_word     "regsmap"               ; mode of currently mapped regs
        struct_word     "regswap"               ; mode of regs to be mapped
        struct_word     "icount"                ; instruction count (fetched)
        struct_word     "nexec"                 ; instructions executed
        struct_null     "size"                  ; size of the processor state
	struct_end

        ; NOTE: "regsmap" contains the mode of the register set that is
        ;       currently used, this may NOT be the same as the current
        ;       processor mode. If the modes are different then "regswap"
        ;       contains the processor mode of the register set that is to
        ;       be selected after the next instruction emulation.
        ;       "regsmap" == processor mode and "regswap" == -1 means that
        ;       the register set is valid. The emulator should generate a
        ;       warning if a mapped register is used when a mapped out state
        ;       exists.

        ; -------------------------------------------------------------------
        ; Emulator output

        ; DISPLAY = write the given text to the output media
        MACRO
$label  DISPLAY $text
$label
        STMFD   sp!,{r0,link}
        ADR     r0,%FT01
        SWI     exec_Output
        LDMFD   sp!,{r0,link}
        B       %FT02
01
        =       "$text",&00
        ALIGN
02
        MEND

        ; LOCATION_READ = notify user of memory read
        MACRO
$label  LOCATION_READ   $reg
$label
        STMFD   sp!,{r0,link}
        ADR     r0,%FT01
        SWI     exec_Output
        [       ($reg = r0)
        LDMFD   sp,{r0}
        |
        MOV     r0,$reg
        ]
        SWI     exec_WriteHex8
        ADR     r0,%FT02
        SWI     exec_Output
        LDMFD   sp!,{r0,link}
        B       %FT03
01
        =       " Location &",&00
02
        =       " read",&0A,&00
        ALIGN
03
        MEND

        ; LOCATION_WRITE = notify user of memory update
        MACRO
$label  LOCATION_WRITE  $reg,$value
$label
        STMFD   sp!,{r0,link}
        ADR     r0,%FT01
        SWI     exec_Output
        [       ($reg = r0)
        LDMFD   sp,{r0}
        |
        MOV     r0,$reg
        ]
        SWI     exec_WriteHex8
        ADR     r0,%FT02
        SWI     exec_Output
        [       ($value = r0)
        LDMFD   sp,{r0}
        |
        MOV     r0,$value
        ]
        SWI     exec_WriteHex8
        SWI     exec_NewLine
        LDMFD   sp!,{r0,link}
        B       %FT03
01
        =       " Location &",&00
02
        =       " written with &",&00
        ALIGN
03
        MEND

        ; ---------------------------------------------------------------------
        ; Manifests for the disassembler

MLA_opcode      *       ("M" :OR: ("L" :SHL: 8) :OR: ("A" :SHL: 16))    ; MLA
MUL_opcode      *       ("M" :OR: ("U" :SHL: 8) :OR: ("L" :SHL: 16))    ; MUL
SWP_opcode      *       ("S" :OR: ("W" :SHL: 8) :OR: ("P" :SHL: 16))    ; SWP
STM_opcode      *       ("S" :OR: ("T" :SHL: 8) :OR: ("M" :SHL: 16))    ; STM
LDM_opcode      *       ("L" :OR: ("D" :SHL: 8) :OR: ("M" :SHL: 16))    ; LDM
STR_opcode      *       ("S" :OR: ("T" :SHL: 8) :OR: ("R" :SHL: 16))    ; STR
LDR_opcode      *       ("L" :OR: ("D" :SHL: 8) :OR: ("R" :SHL: 16))    ; LDR
STC_opcode      *       ("S" :OR: ("T" :SHL: 8) :OR: ("C" :SHL: 16))    ; STC
LDC_opcode      *       ("L" :OR: ("D" :SHL: 8) :OR: ("C" :SHL: 16))    ; LDC
MRC_opcode      *       ("M" :OR: ("R" :SHL: 8) :OR: ("C" :SHL: 16))    ; MRC
CDP_opcode      *       ("C" :OR: ("D" :SHL: 8) :OR: ("P" :SHL: 16))    ; CDP
SWI_opcode      *       ("S" :OR: ("W" :SHL: 8) :OR: ("I" :SHL: 16))    ; SWI

        ; -------------------------------------------------------------------
        END     ; simstate/s
