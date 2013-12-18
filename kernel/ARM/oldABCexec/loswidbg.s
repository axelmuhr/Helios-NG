        SUBT Executive debugging SWI routines                      > loswidbg/s
        ;    Copyright (c) 1989, James G. Smith, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; This can be heavily optimised (some time when there is no rush).
        ; ---------------------------------------------------------------------
        ; -- exec_Disassemble -------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Disassemble the passed instruction. The disassembly is currently
        ; sent to the 2nd (debugging) link adaptor.
        ; in:   r0  = instruction
        ;       r1  = address
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r1 and r12)
        ;       r14 = callers return address
        ; out:  r0  = number of characters printed
        ;       r1  = undefined (corrupted)
        ;       callers processor state restored

code_exec_Disassemble
	[	(dbgsim)
        STMFD   sp!,{r2,r3,r4,r5,lk}    ; store work registers

        MOV     r4,r1                   ; address
        MOV     r5,r0                   ; instruction
        MOV     r1,#&00                 ; initialise character count
        ADRL    r12,cond_names
        MOV     r3,r5,LSR #cond_shift   ; r3 = condition code index
        ADD     r12,r12,r3,LSL #1       ; r12 = address of condition code name
        AND     r3,r5,#op_mask
        ADD     pc,pc,r3,LSR #(op_shift - 2)
        NOP
        B       disass_op_regxreg1
        B       disass_op_regxreg2
        B       disass_op_regximm1
        B       disass_op_regximm2
        B       disass_op_postimm
        B       disass_op_preimm
        B       disass_op_postreg
        B       disass_op_prereg
        B       disass_lsmultiple
        B       disass_lsmultiple
        B       disass_op_b
        B       disass_op_bl
        B       disass_cpdt_post
        B       disass_cpdt_pre
        B       disass_cpdort
        B       disass_swi

        ; ---------------------------------------------------------------------

disass_op_regxreg1
disass_op_regxreg2
        AND     r0,r5,#&00000090
        TEQ     r0,#&00000090
        BEQ     disass_special

        BL      decode_dataproc
        BL      decode_Rm

        MOV     r0,r1                          ; number of characters printed
        LDMFD   sp!,{r2,r3,r4,r5,lk}
        LDMFD   sp!,{r1,r12}
        BICS    pc,lk,#Vbit

        ; ---------------------------------------------------------------------

disass_special
        LDR     r0,=&0F0000F0
        AND     r0,r5,r0
        CMP     r0,#&00000090
        BEQ     disass_multiply

        LDR     r11,=&01000090
        CMP     r0,r11
        BEQ     disass_semaphore

disass_undefined
        ; undefined instruction
        ADRL    r0,UNDEF_opcode
        BL      print_string
        MOV     r0,r5
        BL      local_WriteHex8
        ADD     r1,r1,#8                      ; 8 HEX ASCII characters

        MOV     r0,r1                          ; number of characters printed
        LDMFD   sp!,{r2,r3,r4,r5,lk}
        LDMFD   sp!,{r1,r12}
        BICS    pc,lk,#Vbit

        ; ---------------------------------------------------------------------

        LTORG

        ; ---------------------------------------------------------------------

disass_multiply
        TST     r5,#&0FC00000
        BNE     disass_undefined

        TST     r5,#&00200000
        LDRNE   r0,=MLA_opcode
        LDREQ   r0,=MUL_opcode
        BL      print_opcode
        BL      print_cond_code

        TST     r5,#scc                         ; check set-condition-codes bit
        MOVNE   r0,#"S"
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#1

        BL      pad8

        MOV     r0,r5,LSR #16
        AND     r0,r0,#&0F                      ; r0 = Rn register number
        BL      print_reg

        MOV     r0,#","
        BL      local_TerminalOut
        ADD     r1,r1,#1

        AND     r0,r5,#&0F                      ; r0 = Rm register number
        BL      print_reg

        MOV     r0,#","
        BL      local_TerminalOut
        ADD     r1,r1,#1

        MOV     r0,r5,LSR #8
        AND     r0,r0,#&0F                      ; r0 = Rs register number
        BL      print_reg

        TST     r5,#&00200000
        MOVEQ   r0,r1                          ; number of characters printed
        LDMEQFD sp!,{r2,r3,r4,r5,lk}
        LDMEQFD sp!,{r1,r12}
        BICEQS  pc,lk,#Vbit

        MOV     r0,#","
        BL      local_TerminalOut
        ADD     r1,r1,#1

        MOV     r0,r5,LSR #12
        AND     r0,r0,#&0F                      ; r0 = Rd register number
        BL      print_reg

        MOV     r0,r1                          ; number of characters printed
        LDMFD   sp!,{r2,r3,r4,r5,lk}
        LDMFD   sp!,{r1,r12}
        BICS    pc,lk,#Vbit

        ; ---------------------------------------------------------------------

disass_semaphore
        LDR     r0,=&00B00F00
        TST     r5,r0
        BNE     disass_undefined

        LDR     r0,=SWP_opcode
        BL      print_opcode
        BL      print_cond_code

        TST     r5,#bwb                         ; check byte/word bit
        MOVNE   r0,#"B"
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#&01

        BL      pad8

        MOV     r0,r5,LSR #12
        AND     r0,r0,#&0F                      ; r0 = Rd register number
        BL      print_reg

        MOV     r0,#","
        BL      local_TerminalOut
        ADD     r1,r1,#1

        AND     r0,r5,#&0F                      ; r0 = Rm register number
        BL      print_reg
        ADRL    r0,comma_bracket
        BL      print_string

        MOV     r0,r5,LSR #16
        AND     r0,r0,#&0F                      ; r0 = Rn register number
        BL      print_reg

        MOV     r0,#"]"
        BL      local_TerminalOut
        ADD     r1,r1,#1

        MOV     r0,r1                          ; number of characters printed
        LDMFD   sp!,{r2,r3,r4,r5,lk}
        LDMFD   sp!,{r1,r12}
        BICS    pc,lk,#Vbit

        ; ---------------------------------------------------------------------

        LTORG

        ; ---------------------------------------------------------------------
disass_op_regximm1
disass_op_regximm2
        BL      decode_dataproc
        AND     r11,r5,#imm_shiftmask
        MOV     r11,r11,LSR #imm_shiftshift
        AND     r2,r5,#imm_valuemask
        MOV     r3,r2,LSR r11
        RSB     r11,r11,#32
        MOV     r2,r2,LSL r11
        ORR     r2,r2,r3
        ADRL    r0,hash_amp
        BL      print_string
        MOV     r0,r2
        BL      local_WriteHex8
        ADD     r1,r1,#8                      ; 8 HEX ASCII characters

        MOV     r0,r1                          ; number of characters printed
        LDMFD   sp!,{r2,r3,r4,r5,lk}
        LDMFD   sp!,{r1,r12}
        BICS    pc,lk,#Vbit

        ; ---------------------------------------------------------------------
disass_op_postimm
        BL      decode_ldrstr
        ADRL    r0,bracket_closeh
        BL      print_string

        TST     r5,#udb                         ; check up/down bit
        MOVEQ   r0,#"-"
        BLEQ    local_TerminalOut
        ADDEQ   r1,r1,#1

        LDR     r0,=ls_offset_mask
        AND     r0,r5,r0
        BL      local_WriteDecimal

        MOV     r0,r1                          ; number of characters printed
        LDMFD   sp!,{r2,r3,r4,r5,lk}
        LDMFD   sp!,{r1,r12}
        BICS    pc,lk,#Vbit

        ; ---------------------------------------------------------------------
disass_op_preimm
        BL      decode_ldrstr
        ADRL    r0,comma_hash
        BL      print_string

        TST     r5,#udb                         ; check up/down bit
        MOVEQ   r0,#"-"
        BLEQ    local_TerminalOut
        ADDEQ   r1,r1,#1        

        LDR     r0,=ls_offset_mask
        AND     r0,r5,r0
        BL      local_WriteDecimal

        MOV     r0,#"]"
        BL      local_TerminalOut
        ADD     r1,r1,#1

        TST     r5,#wbb                         ; check write-back bit
        MOVNE   r0,#"!"
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#1

        MOV     r0,r1                          ; number of characters printed
        LDMFD   sp!,{r2,r3,r4,r5,lk}
        LDMFD   sp!,{r1,r12}
        BICS    pc,lk,#Vbit

        ; ---------------------------------------------------------------------

        LTORG

        ; ---------------------------------------------------------------------
disass_op_postreg
        BL      decode_ldrstr
        ADRL    r0,bracket_comma
        BL      print_string

        TST     r5,#udb                         ; check up/down bit
        MOVEQ   r0,#"-"
        BLEQ    local_TerminalOut
        ADDEQ   r1,r1,#1

        BL      decode_Rm

        MOV     r0,r1                           ; number of characters printed
        LDMFD   sp!,{r2,r3,r4,r5,lk}
        LDMFD   sp!,{r1,r12}
        BICS    pc,lk,#Vbit

        ; ---------------------------------------------------------------------
disass_op_prereg
        BL      decode_ldrstr
        MOV     r0,#","
        BL      local_TerminalOut
        ADD     r1,r1,#1

        TST     r5,#udb                         ; check up/down bit
        MOVEQ   r0,#"-"
        BLEQ    local_TerminalOut
        ADDEQ   r1,r1,#1

        BL      decode_Rm

        MOV     r0,#"]"
        BL      local_TerminalOut
        ADD     r1,r1,#1
        TST     r5,#wbb
        MOVNE   r0,#"!"
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#1

        MOV     r0,r1                           ; number of characters printed
        LDMFD   sp!,{r2,r3,r4,r5,lk}
        LDMFD   sp!,{r1,r12}
        BICS    pc,lk,#Vbit

        ; ---------------------------------------------------------------------

disass_lsmultiple
        TST     r5,#lsb                         ; check load/store bit
        LDREQ   r0,=STM_opcode
        LDRNE   r0,=LDM_opcode   
        BL      print_opcode
        BL      print_cond_code

        AND     r11,r5,#&0F900000                ; direction information

        CMP     r11,#op_LDMFA
        CMPNE   r11,#op_STMFA
        ADREQL  r0,FA_opcode
        BEQ     disass_lsmultiple_direction

        CMP     r11,#op_LDMEA
        CMPNE   r11,#op_STMEA
        ADREQL  r0,EA_opcode
        BEQ     disass_lsmultiple_direction

        CMP     r11,#op_LDMFD
        CMPNE   r11,#op_STMFD
        ADREQL  r0,FD_opcode
        BEQ     disass_lsmultiple_direction

        CMP     r11,#op_LDMED
        CMPNE   r11,#op_STMED
        ADRL    r0,ED_opcode
disass_lsmultiple_direction
        ; r0 = stack type string
        BL      print_string    
        BL      pad8

        MOV     r0,r5,LSR #16
        AND     r0,r0,#&0F              ; r0 = Rn register number
        BL      print_reg

        TST     r5,#wbb                 ; check write-back bit
        MOVNE   r0,#"!"
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#1

        ADRL    r0,comma_curly
        BL      print_string

        LDR     r11,=regmask                    ; register bit mask
        AND     r11,r5,r11
        MOV     r2,#&00                         ; start with register zero
disass_lsmultiple_loop
        TST     r11,#&01                        ; check for this bit
        BNE     disass_lsmultiple_print
        ; no register printed
        ADD     r2,r2,#&01                      ; increment the register number
        MOVS    r11,r11,LSR #1			; shift down the register mask
        BNE     disass_lsmultiple_loop
        B       disass_lsmultiple_exit          ; NO more registers to print

disass_lsmultiple_print
        MOV     r0,r2
        BL      print_reg                       ; show this register
        ADD     r2,r2,#&01                      ; increment the register number
        MOVS    r11,r11,LSR #1                    ; shift down the register mask
        MOVNE   r0,#","                         ; more registers,
        BLNE    local_TerminalOut               ; so print a comma
        ADDNE   r1,r1,#1
        BNE     disass_lsmultiple_loop          ; and go around again
        ; and fall through to...
disass_lsmultiple_exit
        MOV     r0,#"}"
        BL      local_TerminalOut
        ADD     r1,r1,#&01

        TST     r5,#psrfu                       ; check PSR update bit
        MOVNE   r0,#"^"
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#1

        MOV     r0,r1                           ; number of characters printed
        LDMFD   sp!,{r2,r3,r4,r5,lk}
        LDMFD   sp!,{r1,r12}
        BICS    pc,lk,#Vbit

        ; ---------------------------------------------------------------------

        LTORG

        ; ---------------------------------------------------------------------
disass_op_b
disass_op_bl
        MOV     r0,#"B"
        BL      local_TerminalOut
        ADD     r1,r1,#&01
        AND     r0,r5,#op_mask
        TEQ     r0,#op_b                        ; branch instruction
        MOVNE   r0,#"L"                         ; NO, it is a branch-with-link
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#1
        BL      print_cond_code
        BL      pad8
        MOV     r0,#"&"
        BL      local_TerminalOut
        AND     r0,r5,#&00FFFFFF        
        MOV     r0,r0,LSL #2                    ; address = (address * 4)
        ADD     r0,r0,#8                        ; deal with pipe-lining
        ADD     r0,r0,r4                        ; and with the passed address
        BIC     r0,r0,#HIflags                  ; and bring back into range
        BL      local_WriteHex8
        ADD     r1,r1,#(1 + 8)                  ; "&" + 8 HEX ASCII characters

        MOV     r0,r1                           ; number of characters printed
        LDMFD   sp!,{r2,r3,r4,r5,lk}
        LDMFD   sp!,{r1,r12}
        BICS    pc,lk,#Vbit

        ; ---------------------------------------------------------------------
disass_cpdt_post        ; post-indexed data transfer
        BL      decode_ldcstc
        ADRL    r0,bracket_chash
        BL      print_string
        TST     r5,#udb         ; check up/down bit
        MOVEQ   r0,#"-"
        BLEQ    local_TerminalOut
        ADDEQ   r1,r1,#1
        AND     r0,r5,#cp_offset_mask
        BL      local_WriteDecimal

        MOV     r0,r1                           ; number of characters printed
        LDMFD   sp!,{r2,r3,r4,r5,lk}
        LDMFD   sp!,{r1,r12}
        BICS    pc,lk,#Vbit

        ; ---------------------------------------------------------------------
disass_cpdt_pre         ; pre-indexed data transfer
        BL      decode_ldcstc
        ADRL    r0,comma_hash
        BL      print_string
        TST     r5,#udb         ; check up/down bit
        MOVEQ   r0,#"-"
        BLEQ    local_TerminalOut
        ADDEQ   r1,r1,#1
        AND     r0,r5,#cp_offset_mask
        BL      local_WriteDecimal
        MOV     r0,#"]"
        BL      local_TerminalOut
        ADD     r1,r1,#1
        TST     r5,#wbb         ; check write-back bit
        MOVNE   r0,#"!"
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#1

        MOV     r0,r1                           ; number of characters printed
        LDMFD   sp!,{r2,r3,r4,r5,lk}
        LDMFD   sp!,{r1,r12}
        BICS    pc,lk,#Vbit

        ; ---------------------------------------------------------------------
disass_cpdort           ; data operation or register transfer
        TST     r5,#regtrans
        BEQ     disass_cpdort_data_op

        LDR     r0,=MRC_opcode
        BL      print_opcode
        BL      print_cond_code
        BL      pad8
        
        ADRL    r0,cp_prefix
        BL      local_Output     

        AND     r0,r5,#cpnum_mask
        MOV     r0,r0,LSR #8
        BL      local_WriteDecimal              ; co-processor number

        MOV     r0,#","
        BL      local_TerminalOut

        AND     r0,r5,#cpopcode_mask
        MOV     r0,r0,LSR #20
        BL      local_WriteDecimal              ; co-processor opcode number

        MOV     r0,#","
        BL      local_TerminalOut

        ADD     r1,r1,#(2 + 2)                  ; "cp" + "," + ","

        MOV     r0,r5,LSR #12
        AND     r0,r0,#&0F                      ; r0 = Rd register number
        BL      print_reg
        B       disass_cpdort_finish

        ; ---------------------------------------------------------------------

disass_cpdort_data_op
        LDR     r0,=CDP_opcode
        BL      print_opcode
        BL      print_cond_code
        BL      pad8
        
        ADRL    r0,cp_prefix
        BL      local_Output     

        AND     r0,r5,#cpnum_mask
        MOV     r0,r0,LSR #8
        BL      local_WriteDecimal              ; co-processor number

        MOV     r0,#","
        BL      local_TerminalOut

        AND     r0,r5,#cpopcode_mask
        MOV     r0,r0,LSR #20
        BL      local_WriteDecimal              ; co-processor opcode number

        MOV     r0,#"c"
        BL      local_TerminalOut
        MOV     r0,r5,LSR #12
        AND     r0,r0,#&0F                      ; r0 = Rd register number
        BL      local_WriteDecimal

        ADD     r1,r1,#(2 + 1 + 1)              ; "cp" + "," + "c"
        ; and fall through to...
disass_cpdort_finish
        ADRL    r0,comma_prefix
        BL      local_Output
        MOV     r0,r5,LSR #16
        AND     r0,r0,#&0F                      ; r0 = Rn register number
        BL      local_WriteDecimal

        ADRL    r0,comma_prefix
        BL      local_Output
        AND     r0,r5,#&0F                      ; r0 = Rm register number
        BL      local_WriteDecimal

        MOV     r0,#","
        BL      local_TerminalOut

        AND     r0,r5,#cpinfo_mask
        MOV     r0,r0,LSR #5                    ; co-processor information
        BL      local_WriteDecimal

        ADD     r1,r1,#(2 + 2 + 1)              ; ",c" + ",c" + ","
        MOV     r0,r1                           ; number of characters printed
        LDMFD   sp!,{r2,r3,r4,r5,lk}
        LDMFD   sp!,{r1,r12}
        BICS    pc,lk,#Vbit

        ; ---------------------------------------------------------------------
disass_swi
        LDR     r0,=SWI_opcode
        BL      print_opcode
        BL      print_cond_code
        BL      pad8
        MOV     r0,#"&"
        BL      local_TerminalOut
        AND     r0,r5,#&00FFFFFF        
        BL      local_WriteHex8
        ADD     r1,r1,#(1 + 8)                  ; "&" + 8 HEX ASCII characters

        MOV     r0,r1                           ; number of characters printed
        LDMFD   sp!,{r2,r3,r4,r5,lk}
        LDMFD   sp!,{r1,r12}
        BICS    pc,lk,#Vbit

        ; ---------------------------------------------------------------------
        ; Decode the single register load and store instructions
decode_ldrstr
        STMFD   sp!,{lk}

        TST     r5,#lsb                 ; check load/store bit
        LDREQ   r0,=STR_opcode
        LDRNE   r0,=LDR_opcode
        BL      print_opcode
        BL      print_cond_code
        TST     r5,#bwb                 ; check byte/word bit
        MOVNE   r0,#"B"
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#&01
        TST     r5,#ppi                 ; check pre/post-indexing bit
        BNE     no_user_trans

        ; post-indexed instruction : always has write-back so
        TST     r5,#wbb                 ; check write-back bit
        MOVNE   r0,#"T"                 ; force user address space translation
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#&01
no_user_trans
        BL      pad8
        
        MOV     r0,r5,LSR #12
        AND     r0,r0,#&0F              ; r0 = Rd register number
        BL      print_reg

        ADRL    r0,comma_bracket
        BL      print_string

        MOV     r0,r5,LSR #16
        AND     r0,r0,#&0F              ; r0 = Rn register number
        BL      print_reg

        LDMFD   sp!,{pc}^

        ; ---------------------------------------------------------------------
        ; Decode the co-processor load and store instructions
decode_ldcstc
        STMFD   sp!,{lk}

        TST     r5,#lsb                 ; check load/store bit
        LDREQ   r0,=STC_opcode
        LDRNE   r0,=LDC_opcode
        BL      print_opcode
        BL      print_cond_code
        TST     r5,#bwb                 ; check byte/word bit
        MOVNE   r0,#"L"
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#&01
        BL      pad8
        
        ADRL    r0,cp_prefix
        BL      local_Output     
        AND     r0,r5,#cpnum_mask
        MOV     r0,r0,LSR #8
        BL      local_WriteDecimal      ; co-processor number
        ADRL    r0,comma_prefix
        BL      local_Output
        MOV     r0,r5,LSR #12
        AND     r0,r0,#&0F              ; r0 = Rd register number
        BL      local_WriteDecimal
        ADRL    r0,comma_bracket
        BL      local_Output
        ADD     r1,r1,#(2 + 2 + 2)      ; "cp" + ",c" + ",["
        MOV     r0,r5,LSR #16
        AND     r0,r0,#&0F              ; r0 = Rn register number
        BL      print_reg
        LDMFD   sp!,{pc}^

        ; ---------------------------------------------------------------------

decode_dataproc
        STMFD   sp!,{r11,lk}

        MOV     r11,r5,LSR #dp_shift
        AND     r11,r11,#&0F

        ADRL    r0,dataproc_opcodes
        ADD     r0,r0,r11,LSL #2        ; r0 = (table + (opcode index * 4))
        BL      print_string
        BL      print_cond_code

        CMP     r11,#dp_tst
        RSBGES  r0,r11,#dp_cmn
        BGE     decode_dataproc_iscmp

        TST     r5,#scc                 ; test set-condition-codes bit
        MOVNE   r0,#"S"
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#1

        BL      pad8

        MOV     r0,r5,LSR #12
        AND     r0,r0,#&0F              ; r0 = Rd register number
        BL      print_reg

        MOV     r0,#","
        BL      local_TerminalOut
        ADD     r1,r1,#1

        CMP     r11,#dp_mov
        CMPNE   r11,#dp_mvn
        LDMEQFD sp!,{r11,pc}^           ; return early (no Rn)

        MOV     r0,r5,LSR #16
        AND     r0,r0,#&0F              ; r0 = Rn register number
        BL      print_reg

        MOV     r0,#","
        BL      local_TerminalOut
        ADD     r1,r1,#1

        LDMFD   sp!,{r11,pc}^

        ; ---------------------------------------------------------------------

decode_dataproc_iscmp
        MOV     r0,r5,LSR #12
        AND     r0,r0,#&0F
        TEQ     r0,#15                  ; Rd == PC
        MOVEQ   r0,#"P"
        BLEQ    local_TerminalOut
        ADDEQ   r1,r1,#1
        BL      pad8

        CMP     r11,#dp_mov
        CMPNE   r11,#dp_mvn
        LDMEQFD sp!,{r11,pc}^

        MOV     r0,r5,LSR #16
        AND     r0,r0,#&0F              ; r0 = Rn register number
        BL      print_reg

        MOV     r0,#","
        BL      local_TerminalOut
        ADD     r1,r1,#1

        LDMFD   sp!,{r11,pc}^

        ; ---------------------------------------------------------------------

decode_Rm
        STMFD   sp!,{r11,lk}

        AND     r11,r5,#simm_mask
        MOV     r11,r11,LSR #7

        AND     r0,r5,#&0F              ; r0 = Rm register number
        BL      print_reg

        MOV     r0,r5,LSR #4
        AND     r0,r0,#&07              ; shift type
        ADD     pc,pc,r0,LSL #2         ; index through the jump table
        NOP
        B       decode_Rm_LSL_imm
        B       decode_Rm_LSL_reg
        B       decode_Rm_LSR_imm
        B       decode_Rm_LSR_reg
        B       decode_Rm_ASR_imm
        B       decode_Rm_ASR_reg
        B       decode_Rm_ROR_imm
        B       decode_Rm_ROR_reg

        ; ---------------------------------------------------------------------

decode_Rm_LSL_imm
        TEQ     r11,#0
        LDMEQFD sp!,{r11,pc}^            ; exit immediately
        ADRL    r0,shift_LSL_hash
        B       decode_Rm_imm_exit

        ; ---------------------------------------------------------------------

decode_Rm_LSR_imm
        TEQ     r11,#0
        MOVEQ   r11,#32
        ADRL    r0,shift_LSR_hash
        B       decode_Rm_imm_exit

        ; ---------------------------------------------------------------------

decode_Rm_ASR_imm
        TEQ     r11,#0
        MOVEQ   r11,#32
        ADRL    r0,shift_ASR_hash
        B       decode_Rm_imm_exit

        ; ---------------------------------------------------------------------

decode_Rm_ROR_imm
        TEQ     r11,#&00
        ADREQL  r0,shift_RRX
        LDMEQFD sp!,{r11,pc}^            ; and exit

        ADRL    r0,shift_ROR_hash
decode_Rm_imm_exit
        BL      print_string
        MOV     r0,r11
        BL      local_WriteDecimal
        LDMFD   sp!,{r11,pc}^

        ; ---------------------------------------------------------------------

decode_Rm_LSL_reg
        ADRL    r0,shift_LSL
        B       decode_Rm_reg_exit

        ; ---------------------------------------------------------------------

decode_Rm_LSR_reg
        ADRL    r0,shift_LSR
        B       decode_Rm_reg_exit

        ; ---------------------------------------------------------------------

decode_Rm_ASR_reg
        ADRL    r0,shift_ASR
        B       decode_Rm_reg_exit

        ; ---------------------------------------------------------------------

decode_Rm_ROR_reg
        ADRL    r0,shift_ROR
decode_Rm_reg_exit
        BL      print_string
        MOV     r0,r11
        BL      print_reg

        LDMFD   sp!,{r11,pc}^

        ; ---------------------------------------------------------------------

print_opcode
        ; in:   r0 = 3bytes of opcode name
        ;       r1 = number of characters printed
        ; out:  r0 = corrupted
        ;       r1 = updated number of characters printed

        STMFD   sp!,{r0,lk}

        BL      local_TerminalOut       ; bits 0..7
        MOV     r0,r0,LSR #8
        BL      local_TerminalOut       ; bits 8..15
        MOV     r0,r0,LSR #8
        BL      local_TerminalOut       ; bits 16..23

        ADD     r1,r1,#3              ; and 3 characters printed
        LDMFD   sp!,{r0,pc}^

        ; ---------------------------------------------------------------------

print_string
        ; in:   r0 = NULL terminated string
        ;       r1 = number of characters printed
        ; out:  r0 = corrupted
        ;       r1 = updated number of characters printed

        STMFD   sp!,{r11,lk}
        MOV     r11,r0
print_string_loop
        LDRB    r0,[r11],#&01
        TEQ     r0,#&00
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#1
        BNE     print_string_loop
        LDMFD   sp!,{r11,pc}^

        ; ---------------------------------------------------------------------

print_cond_code
        ; in:   r1 = number of characters printed
        ;       r12 = address of two byte condition code to display
        ; out:  r1 = updated number of characters printed
        ;       r12 = preserved
        STMFD   sp!,{r0,lk}
        LDRB    r0,[r12,#&00]
        TEQ     r0,#&00
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#1
        LDRB    r0,[r12,#&01]
        TEQ     r0,#&00
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#1
        LDMFD   sp!,{r0,pc}^

        ; ---------------------------------------------------------------------

print_reg
        ; in:   r0 = register number
        ;       r1 = number of characters printed
        ; out:  r0 = preserved
        ;       r1 = updated number of characters printed

        STMFD   sp!,{r11,lk}
        ADRL    r11,reg_names
        ADD     r11,r11,r0,LSL #1         ; r11 = r11 + (r0 * 2)
        LDR     r0,[r11,#&00]
        TEQ     r0,#&00
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#1
        LDR     r0,[r11,#&01]
        TEQ     r0,#&00
        BLNE    local_TerminalOut
        ADDNE   r1,r1,#1
        LDMFD   sp!,{r11,pc}^

        ; ---------------------------------------------------------------------

pad8    ; Pad to the next column (where columns are 8 characters wide)
        ; in:   r1 = number of characters printed
        ; out:  r1 = updated number of characters printed

        STMFD   sp!,{r0,r11,lk}
        ADD     r11,r1,#8               ; add on 8
        BIC     r11,r11,#(8 - 1)          ; and mask out excess bits
        MOV     r0,#" "
pad_loop
        TEQ     r1,r11                  ; check for column reached
        BLNE    local_TerminalOut       ; display the tab padding character
        ADDNE   r1,r1,#&01            ; increment the character count
        BNE     pad_loop                ; and around again

        LDMFD   sp!,{r0,r11,pc}^

        ; ---------------------------------------------------------------------
        ; The condition codes are stored as two byte strings (no termination)
cond_names
        =       "EQ"
        =       "NE"
        =       "CS"
        =       "CC"
        =       "MI"
        =       "PL"
        =       "VS"
        =       "VC"
        =       "HI"
        =       "LS"
        =       "GE"
        =       "LT"
        =       "GT"
        =       "LE"
        =       &00,&00 ; special case for "AL"
        =       "NV"

        ; The register names are stored as two byte strings (no termination)
reg_names
        =       "a1"
        =       "a2"
        =       "a3"
        =       "a4"
        =       "v1"
        =       "v2"
        =       "v3"
        =       "v4"
        =       "v5"
        =       "dp"
        =       "sl"
        =       "fp"
        =       "ip"
        =       "sp"
        =       "lk"
        =       "pc"

cp_prefix       =       "cp",&00
comma_prefix    =       ",c",&00
comma_bracket   =       ",[",&00
bracket_comma   =       "],",&00
bracket_closeh  =       "],#",&00
bracket_chash   =       "["                     ; follows on to next line
comma_hash      =       ",#",&00
comma_curly     =       ",{",&00
hash_amp        =       "#&",&00
two_spaces      =       "  ",&00

shift_LSL_hash  =       ",LSL #",&00
shift_LSR_hash  =       ",LSR #",&00
shift_ASR_hash  =       ",ASR #",&00
shift_ROR_hash  =       ",ROR #",&00

shift_LSL       =       ",LSL ",&00
shift_LSR       =       ",LSR ",&00
shift_ASR       =       ",ASR ",&00
shift_ROR       =       ",ROR ",&00

shift_RRX       =       ",RRX",&00

FA_opcode       =       "FA",&00
EA_opcode       =       "EA",&00
FD_opcode       =       "FD",&00
ED_opcode       =       "ED",&00

dataproc_opcodes
                =       "AND",&00
                =       "EOR",&00
                =       "SUB",&00
                =       "RSB",&00
                =       "ADD",&00
                =       "ADC",&00
                =       "SBC",&00
                =       "RSC",&00
                =       "TST",&00
                =       "TEQ",&00
                =       "CMP",&00
                =       "CMN",&00
                =       "ORR",&00
                =       "MOV",&00
                =       "BIC",&00
                =       "MVN",&00

UNDEF_opcode    =       "UNDEFINED &",&00
        ALIGN
	|	; middle (dbgsim)
	LDMFD	sp!,{r11,r12}
	ORRS	pc,lk,#Vbit		; return with V set	
	]	; (dbgsim)

        ; ---------------------------------------------------------------------
        ; -- exec_SingleStep --------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Execute the ARM instruction referenced by the passed ARM processor
	; state. Output is sent to the 2nd (debugging) link adaptor.
        ; in:   r0  = ARMstate structure pointer
	;	r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r1 and r12)
        ;       r14 = callers return address
        ; out:  r0  = execution state
        ;       callers processor state restored

code_exec_SingleStep
	[	(dbgsim)
	; Local register definitions
t0	RN	r0	; temporary register
t1	RN	r1	; temporary register
t2	RN	r2	; temporary register
t3	RN	r3	; temporary register
t4	RN	r4	; temporary register
t5	RN	r5	; temporary register
t6	RN	r6	; temporary register
sptr	RN	r7	; ARMstate structure pointer
lPC	RN	r11	; loaded PC (current simulator PC and PSR)
cIN	RN	r12	; current instruction
	STMFD	sp!,{r1,r2,r3,r4,r5,r6,r7,lk}	; work registers

        MOV     sptr,r0                 	; sptr = current ARM state

        ; Check to see if we need to take a FIQ
        LDR     lPC,[sptr,#ARMstate_cPC]	; load the current PC + PSR
        TST     lPC,#Fbit                       ; check if FIQs enabled
        LDREQ   t0,[sptr,#ARMstate_FLAGS]       ; load the current flags
        TSTEQ   t0,#FIQline                     ; FIQ line asserted
        BEQ     take_FIQ                        ; clear - then FIQ asserted

        ; check to see if we need to take an IRQ
        TST     lPC,#Ibit                       ; check if IRQs enabled
        TSTEQ   t0,#IRQline                     ; IRQ line asserted
        BEQ     take_IRQ                        ; clear - then IRQ asserted

        LDR     t0,[sptr,#ARMstate_icount]
        ADD     t0,t0,#1
        STR     t0,[sptr,#ARMstate_icount]

        ; Load from memory (checking for pre-fetch abort)
        BIC     t0,lPC,#PSRflags        ; take the clean (word-aligned) address
        SUB     t0,t0,#8                ; and remove the pipe-lining value
        ; **** THIS INSTRUCTION MAY ABORT ****
        LDR     cIN,[t0,#&00]            ; fetch the instruction
        ; We should "B take_prefetch" if a pre-fetch abort has occurred

        ; sptr = current ARM state
        ; cIN  = instruction to execute
        ; lPC  = current PC value

        ; check the flags the instruction conditionally executes on
        MOV     t0,#&00000000           ; default is not met
        AND     t1,cIN,#c_flags
        ADD     pc,pc,t1,LSR #(c_flags_shift - 2)
        NOP
        B       exec_cond0              ; EQ/NE
        B       exec_cond1              ; CS/CC
        B       exec_cond2              ; MI/PL
        B       exec_cond3              ; VS/VC
        B       exec_cond4              ; HI/LS
        B       exec_cond5              ; GE/LT
        B       exec_cond6              ; GT/LE
        B       exec_cond7              ; AL/NV

        ; -------------------------------------------------------------------

exec_cond0      ; EQ/NE
        TST     lPC,#Zbit
        BNE     condition_met
        B       check_execute

        ; -------------------------------------------------------------------

exec_cond1      ; CS/CC
        TST     lPC,#Cbit
        BNE     condition_met
        B       check_execute

        ; -------------------------------------------------------------------

exec_cond2      ; MI/PL
        TST     lPC,#Nbit
        BNE     condition_met
        B       check_execute

        ; -------------------------------------------------------------------

exec_cond3      ; VS/VC
        TST     lPC,#Vbit
        BNE     condition_met
        B       check_execute

        ; -------------------------------------------------------------------

exec_cond4      ; HI/LS
        ; C and !Z
        TST     lPC,#Cbit
        BEQ     check_execute   ; condition not met (C clear)
        TST     lPC,#Zbit
        MOVEQ   t0,#&FFFFFFFF   ; condition met (Z clear)
        B       check_execute

        ; -------------------------------------------------------------------

exec_cond5      ; GE/LT
        ; (N and V) or (!N and !V)
        TST     lPC,#Nbit
        BEQ     exec_cond5_Nclear
        TST     lPC,#Vbit
        MOVNE   t0,#&FFFFFFFF   ; condition met (N and V set)
        B       check_execute

exec_cond5_Nclear
        TST     lPC,#Vbit
        MOVEQ   t0,#&FFFFFFFF   ; condition met (N and V clear)
        B       check_execute

        ; -------------------------------------------------------------------

exec_cond6      ; GT/LE
        ; ((N and V) or (!N and !V)) and !Z
        TST     lPC,#Zbit
        BNE     check_execute   ; condition not met (Z set)
        B       exec_cond5      ; test as above

        ; -------------------------------------------------------------------

exec_cond7      ; AL/NV
condition_met
        MOV     t0,#&FFFFFFFF           ; condition always met
        ; fall through to...
        ; -------------------------------------------------------------------
        ; check if the instruction needs to be executed
check_execute
        ; t0 = (&00000000 == FALSE; &FFFFFFFF = TRUE)
        ; cIN = instruction to be decoded
        ; lPC = current PC value

        AND     t1,cIN,#c_execute       ; execute polarity
        TEQ     t1,#&00000000           ; clear
        MVNNE   t0,t0                   ; invert the sense of the flag
        TEQ     t0,#&00000000
        BEQ     not_executed            ; this instruction is being ignored

        LDR     t0,[sptr,#ARMstate_nexec]
        ADD     t0,t0,#1                ; increment count of instructions
        STR     t0,[sptr,#ARMstate_nexec]

        ; decode and execute relevant instruction classes
        LDR     t0,=&0E000010
        LDR     t1,=&06000010
        AND     t0,cIN,t0
        TEQ     t0,t1
        BEQ     take_undefined          ; undefined instruction exit

        AND     t0,cIN,#op_mask
        ADD     pc,pc,t0,LSR #(op_shift - 2)
        NOP
        B       exec_op_regxreg1
        B       exec_op_regxreg2
        B       exec_op_regximm1
        B       exec_op_regximm2
        B       exec_op_postimm
        B       exec_op_preimm
        B       exec_op_postreg
        B       exec_op_prereg
        B       lsmultiple              ; should return to "execute_continue"
        B       lsmultiple              ; should return to "execute_continue"
        B       exec_op_b
        B       exec_op_bl
        B       cpdt                    ; should return to "execute_continue"
        B       cpdt                    ; should return to "execute_continue"
        B       cpdort                  ; should return to "execute_continue"
        B       swi                     ; should return to "execute_continue"

        ; -------------------------------------------------------------------

exec_op_regxreg1        ; fall through to...
exec_op_regxreg2        ; data processing register/register

        LDR     t0,=&0F000090
        LDR     t1,=&01000090
        AND     t0,cIN,t0
        TEQ     t0,#&00000090
        BEQ     multiply                ; should return to "execute_continue"

        TEQ     t0,t1
        BEQ     semaphore               ; should return to "execute_continue"

        AND     t0,lPC,#Cbit            ; t0 = Cout = current C flag status
        ; cIN  = instruction
        BL      genshift
        ; t0   = modified Cout
        ; t1   = undefined
        ; t2   = undefined
        ; t3   = result from genshift
        ; t4   = undefined
        ; t5   = undefined
        ; t6   = undefined
        ; cIN  = instruction
        ; lPC  = current PC and PSR
        ; sptr = current ARM state
        B       dpcom                   ; should return to "execute_continue"

        ; -------------------------------------------------------------------

exec_op_regximm1        ; fall through to...
exec_op_regximm2        ; data processing register/immediate
        AND     t0,lPC,#Cbit                    ; current C flag status
        MOV     t3,#&00000000                   ; current result

        AND     t1,cIN,#imm_valuemask           ; op1
        AND     t2,cIN,#imm_shiftmask
        MOV     t2,t2,LSR #imm_shiftshift       ; op2
        ; t0   = Cout
        ; t1   = op1
        ; t2   = op2
        BL      dror2
        ; t0   = modified Cout
        ; t1   = undefined
        ; t2   = undefined
        ; t3   = result from "dror2"
        ; t4   = undefined
        ; t5   = undefined
        ; t6   = undefined
        ; cIN  = instruction
        ; lPC  = current PC and PSR
        ; sptr = current ARM state
        B       dpcom                   ; should return to "execute_continue"

        ; -------------------------------------------------------------------

exec_op_postimm         ; fall through to...
exec_op_preimm          ; load/store immediate offset
        LDR     t0,=ls_offset_mask
        AND     t1,cIN,t0
        ; t1  = op1
        ; cIN = instruction
        B       lscom                   ; should return to "execute_contonue"

        ; -------------------------------------------------------------------

exec_op_postreg         ; fall through to...
exec_op_prereg          ; load/store register offset
        TST     cIN,#&00000010
        MOVNE   t0,#warn_badshift
        BLNE    warning
        MOV     t0,#&00000000           ; default Cout
        BL      genshift                ; t0 = Cout; cIN = instruction
        MOV     t1,t3                   ; t1 = op1
        ; cIN = instruction
        B       lscom                   ; should return to "execute_continue"

        ; -------------------------------------------------------------------

exec_op_bl              ; branch with link
        ; The correct r14 is always written (regardless of the last
        ; instruction (mode change)), since there is a tick while 4 is
        ; subtracted from r15 before copying into r14.

        LDR     t0,[sptr,#ARMstate_regswap]
        MOV     t2,#&FFFFFFFF
        TEQ     t0,t2
        BEQ     exec_op_bl_nomap

        AND     t1,t0,#MODEmask
        LDR     t2,[sptr,#ARMstate_regsmap]     ; current mode

        ; t1 = new MODE
        ; t2 = current MODE
        BL      register_mapping

exec_op_bl_nomap
        ; cIN = instruction
        ; lPC = current PC and PSR

        SUB     t0,lPC,#&04             ; subtract 4 from PC pipelining
        STR     t0,[sptr,#(r14 * word)] ; update the current modes r14
        ; and fall through to...

exec_op_b               ; branch
        ; cIN = instruction
        ; lPC = current PC and PSR

        AND     t0,cIN,#&00FFFFFF       ; mask out delta
        ADD     t0,lPC,t0,LSL #2        ; and add onto the current address
        STMFD   sp!,{t0}                ; new address
        BL      new_pc                  ; move to the new PC
        LDR     t0,[sptr,#ARMstate_FLAGS]       ; load control flags
        TST     t0,#fnnames             ; are we displaying function names
        LDMFD   sp!,{t0}                ; recover new address (always)
        BLNE    function_name           ; and display function name if required
        ; The above function will have destroyed r0..r3 and r12

        MOV     r0,#&FFFFFFFF           ; instruction executed
        B       execute_continue

        ; -------------------------------------------------------------------

not_executed
        MOV     r0,#&00000000           ; return FALSE
execute_updatePC
        ; r0 = return value
        ADD     lPC,lPC,#word           ; update the PC
        STR     lPC,[sptr,#ARMstate_cPC]	; and update ARM state copy
        ; and fall through to...
        ; -------------------------------------------------------------------

execute_continue
        ; in:   r0   = return value
        ;       lPC  = current PC and PSR
        ;       sptr = current ARM state
        ;
        ; If the "ARMstate_regsmap" value is different from the PC mode value
        ; and the "ARMstate_regswap" flag == -1, then we have just changed
        ; processor mode so we should update the processor state flags
        ; accordingly.
        ; NOTE: this only notices processor mode changes that occur without
        ;       the PC being updated (only the PSR)

        AND     t1,lPC,#MODEmask        ; t1 = current processor mode
        LDR     t2,[sptr,#ARMstate_regswap]
        MOV     t3,#&FFFFFFFF
        TEQ     t2,t3
        BNE     check_mode_change

        LDR     t2,[sptr,#ARMstate_regsmap]
        TEQ     t2,t1
        BEQ     check_regmapping

        DISPLAY " ** MODE change **"    ; should also display new mode
        B       update_regswap

check_mode_change
        ; t1 = current processor mode
        ; t2 = desired processor mode
        AND     t2,t2,#MODEmask
        TEQ     t1,t2
        BEQ     check_regmapping
        ; another processor mode change while waiting to re-map
update_regswap
        STR     t1,[sptr,#ARMstate_regswap]  ; mode we want to go to
check_regmapping
        ; check if the register set needs to be mapped in
        LDR     t2,[sptr,#ARMstate_regswap]
        MOV     t3,#&FFFFFFFF
        TEQ     t2,t3
	BEQ	SingleStep_exit

        ; pending register map swap, so check if we wait an instruction
        AND     t3,t2,#MODEmask
        TEQ     t2,t3
        BNE     map_registers                   ; map registers immediately

        ; we need to wait another instruction cycle before re-mapping
        ORR     t2,t2,#&00008000        ; instruction executed
        STR     t2,[sptr,#ARMstate_regswap]
        ; r0 contains the executed state
SingleStep_exit
	LDMFD	sp!,{r1,r2,r3,r4,r5,r6,r7,lk}
	LDMFD	sp!,{r11,r12}
	BICS	pc,lk,#Vbit		; return with V clear

        ; ---------------------------------------------------------------------

map_registers
        ; Map in the register set.
        ; This involves copying the banked registers that are currently
        ; mapped in to their relative hiding place, and copying out the new
        ; register values from theirs.

        LDR     t2,[sptr,#ARMstate_regsmap]     ; load the current mapping
        ; t1 = processor mode we are swapping to
        ; t2 = old MODE
        BL      register_mapping
        ; "r0" contains the executed state
	LDMFD	sp!,{r1,r2,r3,r4,r5,r6,r7,lk}
	LDMFD	sp!,{r11,r12}
	BICS	pc,lk,#Vbit		; return with V clear

        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------
        ; This code is called whenever a FIQ occurs
take_FIQ
        ; in:   lPC  = current PC and PSR
        ;       sptr = current ARM state
        ; out:  return to caller directly

        DISPLAY " ** FIQ **"

        SUB     lPC,lPC,#&04            ; subtract 4 from the PC
        STR     lPC,[sptr,#ARMstate_FIQ_r14]    ; store into FIQ_r14
        ; lPC = current PC and PSR
        BIC     lPC,lPC,#MODEmask       ; clear the processor mode bits
        ORR     lPC,lPC,#(FIQmode :OR: Fbit :OR: Ibit)

        LDR     t0,[sptr,#ARMstate_FLAGS]
        BIC     t0,t0,#FIQline          ; clear the FIQ generator
        STR     t0,[sptr,#ARMstate_FLAGS]

        MOV     t0,#vec_FIQ
        BL      new_pc                  ; update the PC and swap registers

        MOV     r0,#&00000000           ; no instruction executed
	LDMFD	sp!,{r1,r2,r3,r4,r5,r6,r7,lk}
	LDMFD	sp!,{r11,r12}
	BICS	pc,lk,#Vbit		; return with V clear

        ; -------------------------------------------------------------------
        ; This code is called whenever an IRQ occurs
take_IRQ
        ; in:   lPC  = current PC and PSR
        ;       sptr = current ARM state
        ; out:  return to caller directly

        DISPLAY " ** IRQ **"

        SUB     lPC,lPC,#4              ; subtract 4 from the PC
        STR     lPC,[sptr,#ARMstate_IRQ_r14]    ; store into IRQ_r14
        ; lPC = current PC and PSR
        BIC     lPC,lPC,#MODEmask       ; clear the processor mode bits
        ORR     lPC,lPC,#(IRQmode :OR: Ibit)

        LDR     t0,[sptr,#ARMstate_FLAGS]
        BIC     t0,t0,#IRQline          ; clear the IRQ generator
        STR     t0,[sptr,#ARMstate_FLAGS]

        MOV     t0,#vec_IRQ
        BL      new_pc                  ; update the PC and swap registers

        MOV     r0,#&00000000           ; no instruction executed
	LDMFD	sp!,{r1,r2,r3,r4,r5,r6,r7,lk}
	LDMFD	sp!,{r11,r12}
	BICS	pc,lk,#Vbit		; return with V clear

        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------
        ; This is called when an "instruction pre-fetch abort" occurs
take_prefetch
        ; in:   lPC  = current PC and PSR
        ;       sptr = current ARM state
        ; out:  return to caller directly

        DISPLAY " ** Prefetch Abort **"

        MOV     t0,#vec_prefetch_abort
        B       take_abort

        ; -------------------------------------------------------------------
        ; This is called when an "undefined instruction" is discoverd
take_undefined
        ; in:   lPC  = current PC and PSR
        ;       cIN  = instruction
        ;       sptr = current ARM state
        ; out:  return to caller directly

        DISPLAY "** Undefined Instruction **"

        MOV     t0,#vec_undefined_instruction
        ; and fall through to...
        ; -------------------------------------------------------------------
take_abort
        ; in:   t0   = vector address

        SUB     lPC,lPC,#&04            ; subtract 4 from the PC
        STR     lPC,[sptr,#ARMstate_SVC_r14]    ; store into SVC_r14
        ; lPC = current PC and PSR
        BIC     lPC,lPC,#MODEmask       ; clear the processor mode bits
        ORR     lPC,lPC,#(SVCmode :OR: Ibit)

        ; t0 = address (loaded above)
        BL      new_pc

        MOV     r0,#&00000000           ; no instruction executed
	LDMFD	sp!,{r1,r2,r3,r4,r5,r6,r7,lk}
	LDMFD	sp!,{r11,r12}
	BICS	pc,lk,#Vbit		; return with V clear

        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------
        ; This code is called when a "data fetch abort" has occurred.
data_abort
        ; in:   lPC  = current PC and PSR
        ;       sptr = current ARM state
        ; out:  t0   = corrupted
        ;       t1   = corrupted
        ;       t2   = corrupted
        ;       t3   = corrupted
        ;       t4   = corrupted
        ;       t5   = corrupted
        ;       t6   = corrupted
        ;       lPC  = modified PC and PSR
        ;       sptr = preserved

        DISPLAY " ** Data Abort **"

        MOV     t0,#vec_data_abort
        B       take_vector             ; enter the vector

        ; -------------------------------------------------------------------
        ; This code is called when an "address exception" has occurred
address_abort
        ; in:   lPC  = current PC and PSR
        ;       sptr = current ARM state
        ; out:  t0   = corrupted
        ;       t1   = corrupted
        ;       t2   = corrupted
        ;       t3   = corrupted
        ;       t4   = corrupted
        ;       t5   = corrupted
        ;       t6   = corrupted
        ;       lPC  = modified PC and PSR
        ;       sptr = preserved

        DISPLAY " ** Address Exception **"

        MOV     t0,#vec_address_exception
        ; and fall through to...
        ; -------------------------------------------------------------------
take_vector
        ; in:   t0   = address of the vector to be taken
        ;       lPC  = current PC and PSR
        ;       sptr = current ARM state
        ; out:  t0   = corrupted
        ;       t1   = corrupted
        ;       t2   = corrupted
        ;       t3   = corrupted
        ;       t4   = corrupted
        ;       t5   = corrupted
        ;       t6   = corrupted
        ;       lPC  = modified PC and PSR
        ;       sptr = preserved

        STR     lPC,[sptr,#ARMstate_SVC_r14]    ; store current PC into SVC_r14
        ; lPC = current PC and PSR
        BIC     lPC,lPC,#MODEmask       ; clear the processor mode bits
        ORR     lPC,lPC,#(SVCmode :OR: Ibit)

        ; and fall through to...
        ; -------------------------------------------------------------------
        ; Place a new value in the PC (where the next instruction will be
        ; fetched from).
new_pc
        ; in:   t0   = desired address
        ;       lPC  = currently loaded PC and PSR
        ;       sptr = current ARM state
        ; out:  t0   = corrupted
        ;       t1   = corrupted
        ;       t2   = corrupted
        ;       t3   = corrupted
        ;       t4   = corrupted
        ;       t5   = corrupted
        ;       t6   = corrupted
        ;       lPC  = modified PC and PSR
        ;       sptr = preserved
        ;
        ; At the moment this code assumes that if the address is invalid,
        ; the instruction execution code will spot the "prefetch" abort.
        ; This is not ideal, since it means the wrong address may be
        ; copied into svc_r14 when taking the "prefetch" abort vector.

        AND     lPC,lPC,#PSRflags       ; remove the address bits from the PC
        BIC     t1,t0,#PSRflags         ; clean up the passed address
        ADD     t1,t1,#8                ; account for prefetch (pipelining)
        ORR     lPC,lPC,t1              ; merge the PSR and new address
        STR     lPC,[sptr,#ARMstate_cPC]	; store the new PC value

        AND     t1,lPC,#MODEmask        ; get the (possibly) new MODE
        LDR     t2,[sptr,#ARMstate_regsmap]
        TEQ     t1,t2                   ; see if we have changed mode
        MOVEQS  pc,lk                   ; NO MODE change, so return to caller

        ; and fall through to...
register_mapping
        ; in:   t0 = undefined
        ;       t1 = new MODE
        ;       t2 = current MODE
        ; out:  t0 = preserved
        ;       t1 = corrupted
        ;       t2 = corrupted
        ;       t3 = corrupted
        ;       t4 = corrupted
        ;       t5 = corrupted
        ;       t6 = corrupted

        ; All modes             -- swap specific r13 and r14
        STR     t1,[sptr,#ARMstate_regsmap]     ; update the current mapping

        ADD     t3,sptr,#ARMstate_USRregs       ; base of mapped out registers
        ADD     t3,t3,t2,LSL #5         ; * 32 (8 registers)
        ; t3   = base of desired register set (r8..r14)
        ; sptr = base of current register set (r0..r15)
        ADD     t3,t3,#(5 * 4)          ; step over r8,r9,r10,r11 and r12
        ADD     t4,sptr,#(13 * 4)       ; step over r0..r12
        LDMIA   t4,{t5,t6}              ; old r13 and r14 registers loaded
        STMIA   t3,{t5,t6}              ; old r13 and r14 registers written
        ADD     t3,sptr,#ARMstate_USRregs       ; base of mapped out registers
        ADD     t3,t3,t1,LSL #5         ; * 32 (8 registers)
        ADD     t3,t3,#(5 * 4)          ; step over r8,r9,r10,r11 and r12
        LDMIA   t3,{t5,t6}              ; new r13 and r14 registers loaded
        STMIA   t4,{t5,t6}              ; new r13 and r14 registers written

        ; t0 = undefined (must be preserved)
        ; t1 = new MODE
        ; t2 = current MODE
        ; t3 = pointer to new MODE r13 and r14
        ; t4 = pointer to current r13 and r14
        ; t5 = undefined
        ; t6 = undefined
        TEQ     t1,#FIQmode
        BEQ     register_mapping_toFIQ
        TEQ     t2,#FIQmode
        BNE     register_mapping_completed      
register_mapping_fromFIQ
        ; FIQ to USR/IRQ/SVC    -- swap specific r8..r12
        SUB     t3,t3,#(5 * 4)          ; reference new MODE r8..r12
        ADD     t4,sptr,#(8 * 4)        ; reference current FIQ r8..r12
        ADD     t1,sptr,#ARMstate_FIQregs       ; reference stored FIQ r8..r12

        STMFD   sp!,{lPC,cIN}

        LDMIA   t4,{lPC,cIN,t2,t5,t6}
        STMIA   t1,{lPC,cIN,t2,t5,t6}
        LDMIA   t3,{lPC,cIN,t2,t5,t6}
        STMIA   t4,{lPC,cIN,t2,t5,t6}

        LDMFD   sp!,{lPC,cIN}

        B       register_mapping_completed

register_mapping_toFIQ
        ; USR/IRQ/SVC to FIQ    -- copy current r8..r12 to USR/IRQ/SVC
        ADD     t1,sptr,#ARMstate_FIQregs       ; reference stored FIQ r8..r12
        ADD     t4,sptr,#(8 * 4)        ; reference current MODE r8..r12

        STMFD   sp!,{lPC,cIN}

        LDMIA   t4,{lPC,cIN,t2,t5,t6}   ; current r8,r9,r10,r11 and r12
        ADD     t3,sptr,#ARMstate_USRregs       ; reference USR
        STMIA   t3,{lPC,cIN,t2,t5,t6}   ; written to USR bank
        ADD     t3,sptr,#ARMstate_IRQregs       ; reference IRQ
        STMIA   t3,{lPC,cIN,t2,t5,t6}   ; written to IRQ bank
        ADD     t3,sptr,#ARMstate_SVCregs       ; reference SVC
        STMIA   t3,{lPC,cIN,t2,t5,t6}   ; written to SVC bank

        ; copy FIQ registers to current
        LDMIA   t1,{lPC,cIN,t2,t5,t6}
        STMIA   t4,{lPC,cIN,t2,t5,t6}

        LDMFD   sp!,{lPC,cIN}

        ; and fall through to...
register_mapping_completed
        MOV     t1,#&FFFFFFFF
        STR     t1,[sptr,#ARMstate_regswap]     ; and "regsmap" is valid
        MOVS    pc,lk                   ; return to caller

        ; -------------------------------------------------------------------

        LTORG

        ; -------------------------------------------------------------------
        ; -- Instruction processing -----------------------------------------
        ; -------------------------------------------------------------------

lsmultiple
        ; in:   cIN  = instruction
        ;       lPC  = current PC and PSR
        ;       sptr = current ARM state
        ; out:  exit through "execute_continue"

        MOV     t6,cIN,LSR #16
        AND     t6,t6,#&F
        LDR     t5,[sptr,t6,LSL #2]
        ; t5 = Rn register contents
        ; t6 = Rn register number

        ; Count the set bits in the register bitmap
        LDR     t0,=regmask                     ; all registers
        AND     t1,cIN,t0                       ; t1 = register bitmask
        MOV     t0,#0                           ; t0 = loop counter
        MOVS    t2,t1                           ; t2 = register bitmask copy
        BEQ     end_of_count_loop
count_loop
        ADD     t0,t0,#1                        ; count this bit
        SUB     t3,t2,#1                        ; current mask - 1
        ANDS    t2,t2,t3                        ; and mask out
        BNE     count_loop
end_of_count_loop
        ; t0 = number of registers in bitmask (k)
        ; t1 = register bitmask (bdop)
        ; t2 = undefined
        ; t3 = undefined
        ; t4 = undefined
        ; t5 = Rn register contents
        ; t6 = Rn register number

        TST     cIN,#udb                        ; test up/down bit
        BNE     do_up
do_down
        SUB     t4,t5,t0,LSL #2                 ; t4 = t5 - (t0 * 4)
        MOV     t3,t4

        TST     cIN,#ppi
        SUBNE   t5,t5,#4                        ; t5 = t5 - 4   pre-decrement
        ADDNE   t4,t4,#4                        ; t4 = t4 + 4   post-decrement
        B       lsmultiple_continue

do_up
        ADD     t4,t5,t0,LSL #2                 ; t4 = t5 + (t0 * 4)
        MOV     t3,t4

        TST     cIN,#ppi
        ADDNE   t5,t5,#4                        ; t5 = t5 + 4   pre-increment
        SUBNE   t4,t4,#4                        ; t4 = t4 - 4   post-increment
        ; fall through to...
lsmultiple_continue
        ; t0 = number of registers
        ; t1 = bitmask of registers
        ; t2 = undefined
        ; t3 = result (possible write-back value)
        ; t4 = address2
        ; t5 = address1
        ; t6 = Rn register number

        ; now check for aborts
        TST     t5,#HIflags                     ; initial address (address1)
        BLNE    address_abort                   ; address exception
        BNE     lsmultiple_abort_generated

        [       {TRUE}
        ; check for "t5/address1" or "t4/address2" generating a data abort
        !       0,"TODO: check for data abort in lsmultiple"
        ; At the moment these will never be executed because of the
        ; instructions above...
        BLNE    data_abort
        BNE     lsmultiple_abort_generated
        ]

after_lsmultiple_aborts

        ; t0 = number of registers
        ; t1 = bitmask of registers
        ; t2 = undefined
        ; t3 = result (possible write-back value)
        ; t4 = address2
        ; t5 = address1
        ; t6 = Rn register number

        TST     cIN,#lsb                        ; check load/store bit
        BEQ     lsmultiple_store                ; we are performing a store
lsmultiple_load
        ; t0 = number of registers
        ; t1 = bitmask of registers
        ; t2 = undefined
        ; t3 = result (possible write-back value)
        ; t4 = address2
        ; t5 = address1
        ; t6 = Rn register number

        !       0,"TODO: lsmultiple register set re-mapping during operation"
        ; The following is NOT quite correct:
        ; The registers are only mapped out (if at all) for the first
        ; register transfer, after that the correct register set is
        ; available.

        TST     cIN,#psrfu                      ; check PSR and force USR mode
        BEQ     lsmultiple_notFU                ; not Force User mode transfer
        TST     t1,#(1 :SHL: 15)                ; check for PC in regset
        BNE     lsmultiple_notFU                ; not Force User mode transfer

lsmultiple_FU                                   ; forced USR mode transfers
        TST     cIN,#wbb
        BEQ     lsmultiple_no_writeback_FU
        TEQ     t6,#15                          ; check for the PC as source
        BEQ     lsmultiple_no_writeback_FU

        ; writeback "result" t3
        STR     t3,[sptr,t6,LSL #2]
        ; At the moment this writes back to the CURRENT mode and not to
        ; USR mode, since this is what the ARM processor will do (I seem
        ; to recall).

lsmultiple_no_writeback_FU

        CMP     t5,t4                   ; (address1 < address2)
        MOVCC   t2,t5                   ; address1
        MOVCS   t2,t4                   ; address2
        ; t0 = number of registers
        ; t1 = register bitmask
        ; t2 = transfer address
        ; t3 = result (for Rn if write-back)
        ; t4 = address2
        ; t5 = address1
        ; t6 = Rn register

        STMFD   sp!,{t4,t5}             ; preserve over following code

        MOV     t3,#2_0000000000000001  ; loop index
        MOV     t6,#0                   ; register index
lsmultiple_load_loop_FU
        TST     t1,t3
        BEQ     lsmultiple_next_loop_FU ; bit not set in bitmask

        ; **** THIS INSTRUCTION MAY CAUSE AN ABORT ****
        LDR     t0,[t2,#&00]            ; t0 = word at address t2

        CMP     t6,#15
        STREQ   t0,[sptr,t6,LSL #2]     ; r15
        BEQ     lsmultiple_load_continue_FU

        CMP     t6,#8
        STRCC   t0,[sptr,t6,LSL #2]     ; r0..r7
        BCC     lsmultiple_load_continue_FU

        CMP     t6,#13
        ADDCS   t4,sptr,#((ARMstate_USRregs + (5 * 4)) - (13 * 4))
        STRCS   t0,[t4,t6,LSL #2]       ; r13..r14
        BCS     lsmultiple_load_continue_FU
        ; r8..r12 -- therefore must update IRQ, SVC and USR copies
        SUB     t5,t6,#8
        ADD     t4,sptr,#ARMstate_USRregs
        STR     t0,[t4,t5,LSL #2]
        ADD     t4,sptr,#ARMstate_IRQregs
        STR     t0,[t4,t5,LSL #2]
        ADD     t4,sptr,#ARMstate_SVCregs
        STR     t0,[t4,t5,LSL #2]
        ; and continue
lsmultiple_load_continue_FU
        MOV     t0,t6
        BL      reg_update              ; register t6 updated

        ADD     t2,t2,#&04              ; increment address

lsmultiple_next_loop_FU
        MOV     t3,t3,LSL #1            ; shift up the bit
        ADD     t6,t6,#1                ; step onto the next register
        CMP     t3,#2_1000000000000000  ; check for end
        BNE     lsmultiple_load_loop_FU

        LDMFD   sp!,{t4,t5}             ; recover registers

        B       lsmultiple_load_continue

        ; ---------------------------------------------------------------------

lsmultiple_notFU
        [       {TRUE}
        ADRL    t0,ddd_mess
        SWI     exec_Output
        B       ddd_over
ddd_mess        =       "lsmultiple NOT forced user",&0A,&00
        ALIGN
ddd_over
        ]

        TST     cIN,#wbb
        BEQ     lsmultiple_no_writeback_notFU
        TEQ     t6,#15                          ; check for the PC as source
        BEQ     lsmultiple_no_writeback_notFU

        ; writeback "result" t3
        STR     t3,[sptr,t6,LSL #2]

lsmultiple_no_writeback_notFU

        CMP     t5,t4                   ; (address1 < address2)
        MOVCC   t2,t5                   ; address1
        MOVCS   t2,t4                   ; address2
        ; t0 = number of registers
        ; t1 = register bitmask
        ; t2 = transfer address
        ; t3 = result (for Rn if write-back)
        ; t4 = address2
        ; t5 = address1
        ; t6 = Rn register

        MOV     t3,#2_0000000000000001  ; loop index
        MOV     t6,#0                   ; register index
lsmultiple_load_loop_notFU
        TST     t1,t3
        BEQ     lsmultiple_next_loop_notFU      ; bit not set in bitmask

        ; **** THIS INSTRUCTION MAY CAUSE AN ABORT ****
        LDR     t0,[t2,#&00]            ; t0 = word at address t2
        STR     t0,[sptr,t6,LSL #2]     ; and update this register
        MOV     t0,t6
        BL      reg_update              ; register t6 updated

        ADD     t2,t2,#&04              ; increment address

lsmultiple_next_loop_notFU
        MOV     t3,t3,LSL #1            ; shift up the bit
        ADD     t6,t6,#1                ; step onto the next register
        CMP     t3,#2_1000000000000000  ; check for end
        BNE     lsmultiple_load_loop_notFU

lsmultiple_load_continue
        TST     t1,#2_1000000000000000  ; PC in list
        BEQ     lsmultiple_exit         ; NO - then we can exit immediately

        ; **** THIS INSTRUCTION MAY CAUSE AN ABORT ****
        LDR     t0,[t2,#&00]            ; t0 = word at address t2
        LOCATION_READ   t2

        ADD     t2,t2,#&04              ; increment address

        TST     cIN,#psrfu              ; check PSR and force USR mode bit
        MOVEQ   t1,#PSRflags            ; full mask
        BEQ     lsmultiple_loadPC_notFU

        TST     lPC,#MODEmask
        MOVNE   t1,#0                   ; NULL mask
        MOVEQ   t1,#(INTflags :OR: MODEmask)

lsmultiple_loadPC_notFU
        AND     lPC,lPC,t1              ; PC = PC & mask
        MVN     t3,t1                   ; invert mask
        AND     t3,t0,t3                ; new address AND inverted mask
        ORR     lPC,lPC,t3
        ; t0  = the new PC address
        ; lPC = modified PSR
        BL      new_pc

        [       {TRUE}                  ; emulator error handling
        ; t2 = final address
        CMP     t5,t4                   ; (address1 > address2)
        MOVCS   t1,t5                   ; address1
        MOVCC   t1,t4                   ; address2
        TEQ     t2,t1

        ; ** DEBUG **
        SWI     exec_NewLine
        ADR     t0,lsdbg_1
        SWI     exec_Output
        MOV     t0,t5
        SWI     exec_WriteHex8
        SWI     exec_NewLine

        ADR     t0,lsdbg_2
        SWI     exec_Output
        MOV     t0,t4
        SWI     exec_WriteHex8
        SWI     exec_NewLine

        ADR     t0,lsdbg_3
        SWI     exec_Output
        MOV     t0,t2
        SWI     exec_WriteHex8
        SWI     exec_NewLine

        ADR     t0,lsdbg_4
        SWI     exec_Output
        MOV     t0,t1
        SWI     exec_WriteHex8
        SWI     exec_NewLine
        B       lsdbg_over

lsdbg_1 =       "t5 (address1) = &",&00
lsdbg_2 =       "t4 (address2) = &",&00
lsdbg_3 =       "t2 (final)    = &",&00
lsdbg_4 =       "t1 (test)     = &",&00
        ALIGN

lsdbg_over
        ; ** DEBUG **

        MOVNE   t0,#warn_emerror
        BNE     warning
        ]

        ; and continue after updating the PC
        MOV     r0,#&FFFFFFFF           ; instruction executed
        B       execute_continue

        ; ---------------------------------------------------------------------

lsmultiple_store
        ; t0 = number of registers
        ; t1 = bitmask of registers
        ; t2 = undefined
        ; t3 = result (possible write-back value)
        ; t4 = address2
        ; t5 = address1
        ; t6 = Rn register number

        !       0,"TODO: lsmultiple register set re-mapping during operation"
        ; The following is NOT quite correct:
        ; The registers are only mapped out (if at all) for the first
        ; register transfer, after that the correct register set is
        ; available.

        TST     cIN,#psrfu
        BEQ     lsmultiple_store_notFU          ; not Force User mode transfer
        TST     t1,#(1 :SHL: 15)
        BNE     lsmultiple_store_notFU          ; not Force User mode transfer

lsmultiple_store_FU
        TST     cIN,#wbb
        MOVEQ   t0,#&00000000                   ; t0 = wb = FALSE
        BEQ     lsmultiple_store_continue_FU

        TEQ     t6,#15
        MOVEQ   t0,#&00000000                   ; t0 = wb = FALSE
        BEQ     lsmultiple_store_continue_FU

        LDR     t0,[sptr,t6,LSL #2]             ; t0 = old contents
        STR     t3,[sptr,t6,LSL #2]             ; perform write-back now
        MOV     t3,t0                           ; t3 = old contents
        ; This does not take into account USR mode forced transfer, since
        ; writeback goes to the CURRENT mode and not USR mode.

        MOV     t0,#&FFFFFFFF                   ; t0 = wb = TRUE
lsmultiple_store_continue_FU
        CMP     t5,t4                   ; (address1 < address2)
        MOVCC   t2,t5                   ; address1
        MOVCS   t2,t4                   ; address2
        ; t0 = write-back flag
        ; t1 = register bitmask
        ; t2 = transfer address
        ; t3 = undefined or old contents if writing back
        ; t4 = address2
        ; t5 = address1 (original Rn contents)
        ; t6 = Rn register number

        STMFD   sp!,{t3,t5,t6}          ; temporary store

        MOV     t3,#2_0000000000000001  ; loop index
        MOV     t5,#0                   ; register index
lsmultiple_store_loop_FU
        TST     t1,t3
        BEQ     lsmultiple_next_store_loop_FU    ; bit not set in bitmask

        CMP     t5,#15
        LDREQ   t6,[sptr,t5,LSL #2]     ; r15
        BEQ     lsmultiple_store_loop_continue

        CMP     t5,#8
        LDRCC   t6,[sptr,t5,LSL #2]     ; r0..r7
        BCC     lsmultiple_store_loop_continue  
        
        ; r8..r14
        ADD     sptr,sptr,#(ARMstate_USRregs - (8 * 4))
        LDR     t6,[sptr,t5,LSL #2]     ; load register value
        SUB     sptr,sptr,#(ARMstate_USRregs - (8 * 4))

lsmultiple_store_loop_continue
        ; **** THIS INSTRUCTION MAY CAUSE AN ABORT ****
        STR     t6,[t2,#&00]    ; t6 written at address t2

        LOCATION_WRITE  t2,t6

        ADD     t2,t2,#&04

lsmultiple_next_store_loop_FU
        MOV     t3,t3,LSL #1            ; shift up the bit
        ADD     t5,t5,#1                ; step onto the next register
        CMP     t3,#2_1000000000000000  ; check for end
        BNE     lsmultiple_store_loop_FU

        LDMFD   sp!,{t3,t5,t6}          ; recover stored registers

        B       lsmultiple_store_continue

        ; --------------------------------------------------------------------

lsmultiple_store_notFU
        TST     cIN,#wbb
        MOVEQ   t0,#&00000000                   ; t0 = wb = FALSE
        BEQ     lsmultiple_store_continue_notFU

        TEQ     t6,#15
        MOVEQ   t0,#&00000000                   ; t0 = wb = FALSE
        BEQ     lsmultiple_store_continue_notFU

        LDR     t0,[sptr,t6,LSL #2]             ; t0 = old contents
        STR     t3,[sptr,t6,LSL #2]             ; perform write-back now
        MOV     t3,t0                           ; t3 = old contents

        MOV     t0,#&FFFFFFFF                   ; t0 = wb = TRUE
lsmultiple_store_continue_notFU
        CMP     t5,t4                   ; (address1 < address2)
        MOVCC   t2,t5                   ; address1
        MOVCS   t2,t4                   ; address2
        ; t0 = write-back flag
        ; t1 = register bitmask
        ; t2 = transfer address
        ; t3 = undefined or old contents if writing back
        ; t4 = address2
        ; t5 = address1 (original Rn contents)
        ; t6 = Rn register number

        STMFD   sp!,{t3,t5,t6}          ; temporary store

        MOV     t3,#2_0000000000000001  ; loop index
        MOV     t5,#0                   ; register index
lsmultiple_store_loop_notFU
        TST     t1,t3
        BEQ     lsmultiple_next_store_loop_notFU    ; bit not set in bitmask

        LDR     t6,[sptr,t5,LSL #2]     ; load register value
        ; **** THIS INSTRUCTION MAY CAUSE AN ABORT ****
        STR     t6,[t2,#&00]            ; t6 written at address t2

        LOCATION_WRITE  t2,t6

        ADD     t2,t2,#&04

lsmultiple_next_store_loop_notFU
        MOV     t3,t3,LSL #1            ; shift up the bit
        ADD     t5,t5,#1                ; step onto the next register
        CMP     t3,#2_1000000000000000  ; check for end
        BNE     lsmultiple_store_loop_notFU

        LDMFD   sp!,{t3,t5,t6}          ; recover stored registers

lsmultiple_store_continue
        [       {TRUE}
        MVNS    t0,t0                   ; check for t0 = &FFFFFFFF
        BNE     lsmultiple_store_no_write_back
        |
        TEQ     t0,#&FFFFFFFF
        BNE     lsmultiple_store_no_write_back
        ]

        ; ** write-back has happened to the register **
        ; If the written back register was the lowest in the list, we must
        ; redo the first store to contain the original value.

        MOV     lk,#&01                 ; lk = single bit
        MOV     t0,lk,LSL t6            ; t0 = (1 << t6)
        TST     t1,t0
        BEQ     lsmultiple_store_no_write_back ; register not in list

        SUB     t0,t0,#&01              ; t0 = ((1 << t6) - 1)
        TST     t1,t0
        BNE     lsmultiple_store_no_write_back ; reg not at front of list

        ; ** DEBUG **
        ADR     t0,lsswb_mess
        SWI     exec_Output
        B       lsswb_over
lsswb_mess      =       "lsmultiple store write-back",&0A,&00
        ALIGN
lsswb_over
        ; ** DEBUG **

        CMP     t5,t4                   ; (address1 < address2)
        MOVCC   t0,t5                   ; address1
        MOVCS   t0,t4                   ; address2

        ; **** THIS INSTRUCTION MAY CAUSE AN ABORT ****
        STR     t3,[t0,#&00]            ; write t3 at address t0
lsmultiple_store_no_write_back
        ; If the PC was specified in the list then write it to memory
        TST     t1,#2_1000000000000000  ; PC in list
        BEQ     lsmultiple_exit

        ; ** DEBUG **
        ADR     t0,lspc_wb
        SWI     exec_Output
        B       lspc_wb_over
lspc_wb =       "lsmultiple store PC write-back",&0A,&00
        ALIGN
lspc_wb_over
        ; ** DEBUG **

        ; **** THIS INSTRUCTION MAY CAUSE AN ABORT ****
        STR     lPC,[t2,#&00]           ; lPC written at address t2
        LOCATION_WRITE  t2,lPC
        ADD     t2,t2,#&04
lsmultiple_exit
        ; t0 = undefined
        ; t1 = undefined
        ; t2 = last address written + 4
        ; t3 = undefined
        ; t4 = address2
        ; t5 = address1
        ; t6 = undefined

        [       {TRUE}                  ; emulator error handling
        ; t2 = final address
        CMP     t5,t4                   ; (address1 > address2)
        MOVCS   t1,t5                   ; address1
        MOVCC   t1,t4                   ; address2
        TEQ     t2,t1

        ; ** DEBUG **
        SWI     exec_NewLine
        ADRL    t0,lsdbg_1
        SWI     exec_Output
        MOV     t0,t5
        SWI     exec_WriteHex8
        SWI     exec_NewLine

        ADRL    t0,lsdbg_2
        SWI     exec_Output
        MOV     t0,t4
        SWI     exec_WriteHex8
        SWI     exec_NewLine

        ADRL    t0,lsdbg_3
        SWI     exec_Output
        MOV     t0,t2
        SWI     exec_WriteHex8
        SWI     exec_NewLine

        ADRL    t0,lsdbg_4
        SWI     exec_Output
        MOV     t0,t1
        SWI     exec_WriteHex8
        SWI     exec_NewLine
        ; ** DEBUG **

        MOVNE   t0,#warn_emerror
        BLNE    warning
        ]

        ; and continue after updating the PC
        MOV     r0,#&FFFFFFFF           ; instruction executed
        B       execute_updatePC

        ; -------------------------------------------------------------------

lsmultiple_abort_generated
        ; t0 = number of registers
        ; t1 = bitmask of registers
        ; t2 = undefined
        ; t3 = result (possible write-back value)
        ; t4 = address2
        ; t5 = address1
        ; t6 = Rn register number

        TST     cIN,#wbb
        MOVEQ   r0,#&FFFFFFFF           ; instruction executed
        BEQ     execute_continue
        TEQ     t6,#15                  ; check for the PC as source
        MOVEQ   r0,#&FFFFFFFF           ; instruction executed
        BEQ     execute_continue

        ; writeback "result" t3 (to USR mode if forced)
        TST     cIN,#psrfu              ; check PSR and force USR mode
        STREQ   t3,[sptr,t6,LSL #2]     ; not forced USR write-back
        BEQ     lsmultiple_exit_abort

        TST     t1,#(1 :SHL: 15)        ; check for PC in regset
        STRNE   t3,[sptr,t6,LSL #2]     ; not forced USR write-back
        BNE     lsmultiple_exit_abort

        ; USR mode forced write-back    
        LDR     t2,[sptr,#ARMstate_regsmap]     ; currently mapped MODE
        TEQ     t2,#FIQmode             ; are we in FIQ mode
        BEQ     lsmultiple_abort_inFIQ  ; YES - so perform special code

        ; we are currently in USR, IRQ or SVC MODE
        CMP     t6,#13                  ; are we writing back to r13 or r14
        STRCC   t3,[sptr,t6,LSL #2]     ; write-back to r0..r12
        BCC     lsmultiple_exit_abort
lsmultiple_abort_nonFIQ
        ; We are currently in USR, IRQ or SVC MODE, with a write-back
        ; request to either USR r13 or r14.
        TEQ     t2,#USRmode
        ADDNE   t2,sptr,#((ARMstate_USRregs + (5 * 4)) - (13 * 4))
        MOVEQ   t2,sptr                                         ; USR mode
        STR     t3,[t2,t6,LSL #2]       ; write-back to USR r13 or r14
        B       lsmultiple_exit_abort

lsmultiple_abort_inFIQ
        ; We are currently in FIQ MODE with a write-back any of the USR
        ; bank registers.
        CMP     t6,#8                   ; are we writing back to r8..r14
        STRCC   t3,[sptr,t6,LSL #2]     ; write back to r0..r7
        BCC     lsmultiple_exit_abort

        CMP     t6,#13                  ; are we writing back to r13 or r14
        ADDCS   t2,sptr,#((ARMstate_USRregs + (5 * 4)) - (13 * 4))
        STRCS   t3,[t2,t6,LSL #2]       ; write-back to r13 or r14
        BCS     lsmultiple_exit_abort

        ; It is between USR r8 and r12, so we need to also update the SVC and
        ; IRQ copies.
        SUB     t6,t6,#8                ; make range r8..r12 indexed from 0
        ADD     t2,sptr,#ARMstate_USRregs
        STR     t3,[t2,t6,LSL #2]       ; write-back to USR r8..r12
        ADD     t2,sptr,#ARMstate_IRQregs
        STR     t3,[t2,t6,LSL #2]       ; write-back to IRQ r8..r12
        ADD     t2,sptr,#ARMstate_SVCregs
        STR     t3,[t2,t6,LSL #2]       ; write-back to SVC r8..r12

        ; and fall through to...
lsmultiple_exit_abort
        MOV     r0,#&FFFFFFFF           ; instruction executed
        B       execute_continue

        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------

lscom
        ; in:   t1   = op1
        ;       cIN  = instruction
        ;       lPC  = current PC and PSR
        ;       sptr = current ARM state
        ; out:  exit through "execute_continue"

        ; t1 = op1 (offset value)
        AND     t6,cIN,#Rn_regmask
        MOV     t6,t6,LSR #Rn_regshift          ; t6 = Rn register number
        AND     t5,cIN,#Rd_regmask
        MOV     t5,t5,LSR #Rd_regshift          ; t5 = Rd register number
        LDR     t4,[sptr,t5,LSL #2]             ; t4 = Rd register contents
        ; We load the "Rd" value in-case write-back occurs before it is stored
        
        LDR     t3,[sptr,t6,LSL #2]             ; t3 = Rn register contents
        TEQ     t6,#15                          ; PC as address
        BICEQ   t3,t3,#PSRflags                 ; clear the PSR flags

        TST     cIN,#udb                        ; check up/down bit
        ADDNE   t2,t3,t1                        ; UP   : t2 = t3 + t1
        SUBEQ   t2,t3,t1                        ; DOWN : t2 = t3 - t1

        TST     cIN,#ppi                        ; check pre-index bit
        MOVNE   t3,t2

        TST     t3,#HIflags                     ; check for address exception
        BLNE    address_abort                   ; which we found
        MOVNE   r0,#&FFFFFFFF
        BNE     execute_continue                ; and exit

        ; data abort checking should happen at this point (before we change
        ; the processor state)

        TST     cIN,#ppi                        ; check pre-index bit
        BEQ     lscom_write_back

        TST     cIN,#wbb                        ; check write-back bit
        BEQ     lscom_no_write_back
lscom_write_back
        TEQ     t6,#15                          ; write_back to PC
        MOVEQ   t6,#&F0                         ; flag write-back to PC
        STRNE   t2,[sptr,t6,LSL #2]             ; store, before possible load
lscom_no_write_back
        ; we are going to perform a memory transfer
        LOCATION_READ   t3

        TST     cIN,#lsb                        ; check load/store bit
        BEQ     lscom_store
lscom_load
        TST     cIN,#bwb                        ; check byte/word bit
        BNE     lscom_load_byte_transfer
lscom_load_word_transfer
        BIC     t0,t3,#(word - 1)               ; get word-aligned address
        LDR     t1,[t0,#&00]                    ; load the value

        ANDS    t0,t3,#(word - 1)               ; get off alignment
        BEQ     lscom_load_continue             ; was word-aligned
        MOV     t0,t0,LSL #3                    ; make into byte shift

        MOV     t3,t1,LSR t0                    ; t3 = (t1 >> t0)
        RSB     t0,t0,#32                       ; t0 = (32 - t0)
        ORR     t1,t3,t1,LSL t0                 ; t1 = t3 | (t1 << t0)
        B       lscom_load_continue

lscom_load_byte_transfer
        LDRB    t1,[t3,#&00]                    ; load BYTE from the address
        BIC     t0,t3,#(word - 1)               ; get word alignment
lscom_load_continue
        ; t1 = value read

        TEQ     t5,#15                  ; destination is PC
        STRNE   t1,[sptr,t5,LSL #2]     ; not the PC
        MOVNE   t0,t5                   ; register number updated (not PC)
        BLNE    reg_update              ; notify user
        BNE     lscom_continue          ; and continue processing
        ; PC update     
        MOV     t0,t1
        BL      new_pc                  ; the new PC
        MOV     r0,#&FFFFFFFF
        B       execute_continue

        ; ---------------------------------------------------------------------

lscom_store
        TST     cIN,#bwb                        ; check byte/word bit
        MOVEQ   t0,t4
        BEQ     lscom_store_continue
lscom_store_byte
        BIC     t0,t3,#(word - 1)               ; get word-aligned address
        LDR     t0,[t0,#&00]                    ; load 32bit value

        MOV     t1,#&FF                         ; byte mask
        AND     t4,t4,t1                        ; byte value
        AND     lk,t3,#(word - 1)               ; get off-alignment
        MOV     lk,lk,LSL #3                    ; make into byte shift
        BIC     t0,t0,t1,LSL lk                 ; clear destination bits
        ORR     t0,t0,t4,LSL lk                 ; and add into the value
lscom_store_continue
        ; we are about to perform a memory transfer
        LOCATION_WRITE  t3,t0

        STR     t0,[t3,#&00]            ; and write the value to memory
lscom_continue
        TEQ     t6,#&F0                 ; there was a PC write-back
        MOVNE   r0,#&FFFFFFFF           ; instruction executed
        BNE     execute_updatePC        ; normal exit
        ; write-back to the PC

        MOV     t0,t2                   ; new address
        BL      new_pc
        MOV     r0,#&FFFFFFFF
        B       execute_continue

        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------

multiply
        ; in:   cIN  = instruction
        ;       lPC  = current PC and PSR
        ;       sptr = current ARM state
        ; out:  exit through "execute_continue"

        LDR     t0,=&0FC000F0
        AND     t0,cIN,t0

        TEQ     t0,#&00000090
        MOVNE   t0,#warn_badmopcode
        BLNE    warning

        MOV     t1,cIN,LSR #16
        AND     t1,t1,#&F               ; Rn register
        MOV     t6,cIN,LSR #12
        AND     t6,t6,#&F               ; Rd register
        MOV     t5,cIN,LSR #8
        AND     t5,t5,#&F               ; Rs register
        AND     t4,cIN,#&F              ; Rm register

        TEQ     t5,#r15                 ; check for PC
        LDR     t3,[sptr,t5,LSL #2]     ; load the register anyway
        BICEQ   t3,t3,#PSRflags         ; YES, then we treat it as a 24bit PC
        ; t3 = Rs register value

        TEQ     t4,t1                   ; Rn register == Rm register
        LDRNE   t0,[sptr,t4,LSL #2]     ; t0 = Rm value
        MULNE   t2,t3,t0                ; t2 = Rm * Rs
        MOVEQ   t2,#&00000000
        MOVEQ   t0,#warn_rmrn
        BLEQ    warning

        TST     cIN,#ab                  ; Accumulate Bit
        LDRNE   t0,[sptr,t6,LSL #2]     ; t0 = Rd value
        ADDNE   t2,t2,t0                ; t2 = (Rm * Rs) + Rd

        TEQ     t1,#r15
        BEQ     multiply_r15_dest

        STR     t2,[sptr,t1,LSL #2]     ; store result if not PC
        ; register Rn updated (generate suitable message to the punter)
        MOV     t0,t1                   ; t0 = register number
        BL      reg_update

        ; always set Z and N from the result
        TST     cIN,#scc                ; Set Condition Codes
        MOVEQ   r0,#&FFFFFFFF           ; instruction executed
        BEQ     execute_updatePC        ; immediate exit if not updating PSR

        CMP     t2,#&00000000           ; t2 = result
        BICNE   lPC,lPC,#Zbit
        ORREQ   lPC,lPC,#Zbit           ; mul = 0
        BICCC   lPC,lPC,#Nbit
        ORRCS   lPC,lPC,#Nbit           ; mul < 0

        ; V is not affected, but C is scrambled
        TST     lPC,#Cbit               ; we will just invert it
        ORREQ   lPC,lPC,#Cbit
        BICNE   lPC,lPC,#Cbit

        STR     lPC,[sptr,#ARMstate_cPC]	; store updated PC

        MOV     r0,#&FFFFFFFF           ; instruction executed
        B       execute_updatePC        ; falls through to "execute_continue"

multiply_r15_dest
        ; cIN = current instruction
        ; lPC = current PC and PSR
        MOV     t0,t2                   ; new address
        TST     cIN,#scc                ; Set Condition Codes
        BEQ     multiply_take_r15

        AND     t0,lPC,#MODEmask        ; processor mode
        TEQ     t0,#USRmode             ; priviledged mode
        BICEQ   lPC,lPC,#Aflags
        ANDEQ   t0,t2,#Aflags
        BICNE   lPC,lPC,#PSRflags
        ANDNE   t0,t2,#PSRflags
        ORR     lPC,lPC,t0              ; insert the bits we are allowed to
multiply_take_r15
        BL      new_pc                  ; update PC and swap registers

        MOV     r0,#&FFFFFFFF           ; instruction executed
        B       execute_continue        ; PC already updated

        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------

semaphore
        ; in:   cIN  = instruction
        ;       lPC  = current PC
        ;       sptr = current ARM state
        ; out:  exit through "execute_continue"

        LDR     t0,=&0FB00FF0
        LDR     t1,=&01000090
        AND     t0,cIN,t0
        TEQ     t0,t1
        MOVNE   t0,#warn_badsopcode
        BLNE    warning

        MOV     t6,cIN,LSR #16
        AND     t6,t6,#&F               ; Rn register
        LDR     t3,[sptr,t6,LSL #2]     ; Rn register contents (address)
        ; t3 = address
        ; t6 = Rn register number

        TST     t3,#&FC000000           ; invalid address?
        BLNE    address_abort           ; address exception
        MOVNE   r0,#&FFFFFFFF           ; instruction executed (failed)
        BNE     execute_continue        ; leave quickly

        !       0,"TODO: check for data abort in semaphore"
        [       {FALSE}
        BLVS    data_abort              ; data fetch abort
        MOVVS   r0,#&FFFFFFFF           ; instruction executed (failed)
        BVS     execute_continue        ; leave quickly
        ]

        TST     cIN,#bwb                ; check byte/word bit
        BNE     semaphore_byte_transfer

        BIC     t5,t3,#(word - 1)       ; get the word-aligned address
        LDR     t1,[t5,#&00]            ; load the word
        TST     t3,#(word - 1)          ; word-aligned address?
        BEQ     semaphore_word_aligned

        AND     t4,t3,#(word - 1)       ; get displacement
        MOV     t4,t4,LSL #3            ; and turn into byte shift
        
        MOV     t0,t1,LSR t4            ; t0 = (oldvalue >> shift)
        RSB     t4,t4,#32               ; shift = (32 - shift)
        ORR     t1,t0,t1,LSL t4         ; oldvalue = (oldvalue << shift) | t0
semaphore_word_aligned
        ; notify the user of what has happened
        LOCATION_READ   t3
        ; now the write operation
        AND     t2,cIN,#Rm_regmask              ; Rm register number
        LDR     t2,[sptr,t2,LSL #2]             ; t2 = Rm register contents
        STR     t2,[t5,#&00]                    ; write to word-aligned address
        LOCATION_WRITE  t3,t2
        B       semaphore_continue

semaphore_byte_transfer
        LDRB    t1,[t3,#&00]            ; load BYTE from address
        LOCATION_READ   t3              ; notify user of memory transfer
        BIC     t3,t3,#(word - 1)       ; get the word-aligned address
        LDR     t2,[t3,#&00]            ; and read the word at this address
        MOV     t0,#&FF                 ; load the byte mask
        AND     t4,t3,#(word - 1)       ; get the displacement
        MOV     t4,t4,LSL #3            ; and make into a byte shift value
        BIC     t2,t2,t0,LSL t4         ; clear the bits in our byte
        AND     t6,cIN,#Rm_regmask      ; get the Rm register number
        LDR     t6,[sptr,t6,LSL #2]     ; load the Rm register contents
        AND     t6,t6,t0                ; clear the top (unused) bytes  
        ORR     t2,t2,t6,LSL t4         ; and add in the Rm byte
        ; and fall through to
        STR     t2,[t3,#&00]            ; and store at the word-aligned address
        LOCATION_WRITE  t3,t2
semaphore_continue
        AND     t0,cIN,#Rd_regmask
        MOV     t0,t0,LSR #Rd_regshift  ; t0 = Rd register number
        TEQ     t0,#15
        BNE     semaphore_update_destination

        MOV     t0,t1                   ; old value read
        BL      new_pc
        MOV     r0,#&FFFFFFFF           ; instruction executed
        B       execute_continue

semaphore_update_destination
        STR     t1,[sptr,t0,LSL #2]
        BL      reg_update

        MOV     r0,#&FFFFFFFF           ; instruction executed
        B       execute_updatePC        ; falls through to "execute_continue"

        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------

dpcom
        ; in:   t0   = Cout
        ;       t3   = result
        ;       cIN  = instruction
        ;       sptr = current ARM state
        ; out:  exit through "execute_continue"

        TST     cIN,#scc
        BEQ     over_this1

        MOV     t2,lPC          ; current PC and PSR
        TEQ     t0,#&00000000   ; check entry Carry state
        ORRNE   t2,t2,#Cbit     ; set carry-to-be
        BICNE   t2,t2,#Cbit     ; clear carry
over_this1
        ; t0 = undefined
        ; t2 = flags
        ; t3 = entry result (op1 = Rm or shift-immediate)
        MOV     t6,cIN,LSR #16
        AND     t6,t6,#&0F
        LDR     t4,[sptr,t6,LSL #2]
        ; t4 = Rn register contents (op2)
        ; t6 = Rn register number
        TEQ     t6,#15
        BICEQ   t4,t4,#PSRflags ; Rn is the PC, so remove the PSR bits

        ; vector to the correct opcode handling code
        AND     t5,cIN,#dp_mask
        ADD     pc,pc,t5,LSR #(dp_shift - 2)
        NOP
        B       dp_and_code
        B       dp_eor_code
        B       dp_sub_code
        B       dp_rsb_code
        B       dp_add_code
        B       dp_adc_code
        B       dp_sbc_code
        B       dp_rsc_code
        B       dp_tst_code
        B       dp_teq_code
        B       dp_cmp_code
        B       dp_cmn_code
        B       dp_orr_code
        B       dp_mov_code
        B       dp_bic_code
        B       dp_mvn_code

dp_and_code
dp_tst_code
        AND     t1,t4,t3                ; value = op2 & op1
        B       break_processing

dp_eor_code
dp_teq_code
        EOR     t1,t4,t3                ; value = op2 ^ op1
        B       break_processing

dp_sub_code
dp_cmp_code
        SUB     t1,t4,t3                ; value = op2 - op1
        TST     cIN,#scc
        EORNE   t4,t4,t3
        EORNE   t3,t4,t3
        EORNE   t4,t4,t3
        ; t1 = value
        ; t2 = flags
        ; t3 = op2 = arg1
        ; t4 = op1 = arg2
        BLNE    cv_sub
        B       break_processing

dp_rsb_code
        SUB     t1,t3,t4                ; value = op1 - op2
        TST     cIN,#scc
        ; t1 = value
        ; t2 = flags
        ; t3 = op1 = arg1
        ; t4 = op2 = arg2
        BLNE    cv_sub
        B       break_processing

dp_add_code
dp_cmn_code
        ADD     t1,t4,t3                ; value = op2 + op1
        TST     cIN,#scc
        EORNE   t4,t4,t3
        EORNE   t3,t4,t3
        EORNE   t4,t4,t3
        ; t1 = value
        ; t2 = flags
        ; t3 = op2 = arg1
        ; t4 = op1 = arg2
        BLNE    cv_add
        B       break_processing

dp_adc_code
        ADD     t1,t4,t3                ; value = op2 + op1
        TST     lPC,#Cbit               ; Carry set in PSR?
        ADDNE   t1,t1,#1                ; add in the Carry
        TST     cIN,#scc
        EORNE   t4,t4,t3
        EORNE   t3,t4,t3
        EORNE   t4,t4,t3
        ; t1 = value
        ; t2 = flags
        ; t3 = op2 = arg1
        ; t4 = op1 = arg2
        BLNE    cv_add
        B       break_processing

dp_sbc_code
        SUB     t1,t4,t3                ; value = op2 - op1
        TST     lPC,#Cbit               ; Carry set in PSR?
        SUBEQ   t1,t1,#1                ; no then subtract a Carry
        TST     cIN,#scc
        EORNE   t4,t4,t3
        EORNE   t3,t4,t3
        EORNE   t4,t4,t3
        ; t1 = value
        ; t2 = flags
        ; t3 = op2 = arg1
        ; t4 = op1 = arg2
        BLNE    cv_sub
        B       break_processing

dp_rsc_code
        SUB     t1,t3,t4                ; value = op1 - op2
        TST     lPC,#Cbit               ; Carry set in PSR?
        SUBEQ   t1,t1,#1                ; no then subtract a Carry
        TST     cIN,#scc
        ; t1 = value
        ; t2 = flags
        ; t3 = op1 = arg1
        ; t4 = op2 = arg2
        BLNE    cv_sub
        B       break_processing

dp_orr_code
        ORR     t1,t4,t3                ; value = op2 | op1
        B       break_processing

dp_mov_code
        MOV     t1,t3                   ; value = op1
        B       break_processing

dp_bic_code
        BIC     t1,t4,t3                ; value = op2 & ~op1
        B       break_processing

dp_mvn_code
        MVN     t1,t3                   ; value = ~op1
        ; and fall through to..
break_processing
        ; t1 = value
        ; t2 = flags
        ; always set Z and N from the calculated value
        TST     cIN,#scc
        BEQ     over_this2

        CMP     t1,#&00000000
        ORREQ   t2,t2,#Zbit             ; zero value
        BICNE   t2,t2,#Zbit             ; non-zero value

        ORRMI   t2,t2,#Nbit             ; value < 0
        BICPL   t2,t2,#Nbit             ; value >= 0

over_this2
        MOV     t6,cIN,LSR #12
        AND     t6,t6,#&0F
        ; t5 = opcode index
        ; t6 = Rd register

        CMP     t5,#(dp_tst :SHL: dp_shift)
        RSBGES  t0,t5,#(dp_cmn :SHL: dp_shift)
        BGE     value_not_written       ; test operation

        ; ((opcode < dp_tst) || (opcode > dp_cmn))
        TEQ     t6,#15 
        STRNE   t1,[sptr,t6,LSL #2]     ; result is not the PC
        ANDEQ   lPC,lPC,#PSRflags       ; preserve PSR state
        BICEQ   t0,t1,#PSRflags         ; remove PSR flag bits from the value
        ORREQ   lPC,lPC,t0              ; and add into the PC and PSR state

        MOV     t0,t6                   ; register number updated
        BL      reg_update
value_not_written
        TST     cIN,#scc
        BEQ     over_this3

        TST     lPC,#MODEmask           ; zero if USR mode
        MOVEQ   t0,#Aflags              ; USRmode updates arithmetic flags only
        MOVNE   t0,#PSRflags            ; all other modes update complete PSR
        BIC     lPC,lPC,t0              ; clear relevant PSR bits
        TEQ     t6,#15
        ANDEQ   t0,t1,t0                ; value & mask
        ANDNE   t0,t2,t0                ; flags & mask
        ORR     lPC,lPC,t0              ; new PC and PSR state

over_this3
        TEQ     t6,#15                  ; are we writing to the PC
        BNE     no_modifiedPC
        ; We are writing to the PC, but check if we are processing a special
        ; instruction.
        CMP     t5,#(dp_tst :SHL: dp_shift)
        RSBGES  t0,t5,#(dp_cmn :SHL: dp_shift)
        BGE     no_modifiedPC
        ; ((opcode < dp_tst) || (opcode > dp_cmn)) and write-back to PC
modifiedPC
        ; ** DEBUG **
        ADR     t0,mPC_mess
        SWI     exec_Output
        MOV     t0,lPC
        SWI     exec_WriteHex8
        SWI     exec_NewLine
        B       mPC_over
mPC_mess        =       "modifiedPC: &",&00
        ALIGN
mPC_over
        ; ** DEBUG **

        MOV     t0,lPC                  ; desired address
        BL      new_pc
        MOV     r0,#&FFFFFFFF           ; instruction executed
        B       execute_continue

no_modifiedPC
        MOV     r0,#&FFFFFFFF           ; instruction executed
        B       execute_updatePC

        ; ---------------------------------------------------------------------

cv_add
        ; in:   t0 = undefined
        ;       t1 = value
        ;       t2 = flags
        ;       t3 = arg1
        ;       t4 = arg2
        ;       t5 = opcode
        ;       t6 = undefined
        ; out:  t0 = undefined
        ;       t1 = preserved
        ;       t2 = preserved
        ;       t3 = undefined
        ;       t4 = undefined
        ;       t5 = preserved
        ;       t6 = undefined

        CMP     t3,#&00000000
        BMI     cv_add_arg_minus_check
        ; t3(arg1) >= 0
        CMP     t4,#&00000000
        BICMI   t2,t2,#Vbit             ; signs different, so no overflow
        BMI     cv_add_signs_continue
        B       cv_add_arg_signs_same
cv_add_arg_minus_check
        ; t3(arg1) < 0
        CMP     t4,#&00000000
        BICPL   t2,t2,#Vbit             ; signs different, so no overflow
        BPL     cv_add_signs_continue
cv_add_arg_signs_same
        CMP     t1,#&00000000
        BMI     cv_add_value_minus_check
        ; t1(value) >= 0
        CMP     t3,#&00000000
        BICPL   t2,t2,#Vbit             ; signs same, resultSign == argSign,
        BPL     cv_add_signs_continue   ; so no overflow
        B       cv_add_value_signs_same
cv_add_value_minus_check
        ; t1(value) < 0
        CMP     t3,#&00000000
        BICMI   t2,t2,#Vbit             ; signs same, resultSign == argSign,
        BMI     cv_add_signs_continue   ; so no overflow
        ; signs same but resultSign != argSign there overflow has occurred
cv_add_value_signs_same
        ORR     t2,t2,#Vbit             ; set the oVerflow flag
cv_add_signs_continue

        CMP     t1,t3
        BNE     cv_add_test2            ; value != arg1
        TEQ     t4,#&00000000           ; arg2 == 0
        ORRNE   t2,t2,#Cbit             ; NO  - set the Carry flag
        BICEQ   t2,t2,#Cbit             ; YES - clear the Carry flag
        MOVS    pc,lk
cv_add_test2
        ORRCC   t2,t2,#Cbit             ; value < arg1  = set the Carry flag
        BICCS   t2,t2,#Cbit             ; value >= arg1 = clear the Carry flag
        MOVS    pc,lk

        ; ---------------------------------------------------------------------

cv_sub
        ; in:   t0 = undefined
        ;       t1 = value
        ;       t2 = flags
        ;       t3 = arg1
        ;       t4 = arg2
        ;       t5 = opcode
        ;       t6 = undefined
        ; out:  t0 = undefined
        ;       t1 = preserved
        ;       t2 = preserved
        ;       t3 = undefined
        ;       t4 = undefined
        ;       t5 = preserved
        ;       t6 = undefined

        CMP     t3,#&00000000
        BMI     cv_sub_arg_minus_check
        ; t3(arg1) >= 0
        CMP     t4,#&00000000
        BICMI   t2,t2,#Vbit             ; signs different, so no overflow
        BMI     cv_sub_signs_continue
        B       cv_sub_arg_signs_same
cv_sub_arg_minus_check
        ; t3(arg1) < 0
        CMP     t4,#&00000000
        BICPL   t2,t2,#Vbit             ; signs different, so no overflow
        BPL     cv_sub_signs_continue
cv_sub_arg_signs_same
        CMP     t1,#&00000000
        BMI     cv_sub_value_minus_check
        ; t1(value) >= 0
        CMP     t3,#&00000000
        BICPL   t2,t2,#Vbit             ; signs same, resultSign == argSign,
        BPL     cv_sub_signs_continue   ; so no overflow
        B       cv_sub_value_signs_same
cv_sub_value_minus_check
        ; t1(value) < 0
        CMP     t3,#&00000000
        BICMI   t2,t2,#Vbit             ; signs same, resultSign == argSign,
        BMI     cv_sub_signs_continue   ; so no overflow
        ; signs same but resultSign != argSign there overflow has occurred
cv_sub_value_signs_same
        ORR     t2,t2,#Vbit             ; set the oVerflow flag
cv_sub_signs_continue

        CMP     t1,t3
        BNE     cv_sub_test2            ; value != arg1
        TEQ     t4,#&00000000           ; arg2 == 0
        ORREQ   t2,t2,#Cbit             ; NO  - set the Carry flag
        BICNE   t2,t2,#Cbit             ; YES - clear the Carry flag
        MOVS    pc,lk
cv_sub_test2
        BICGT   t2,t2,#Cbit             ; value > arg1  = clear the Carry flag
        ORRLE   t2,t2,#Cbit             ; value <= arg1 = set the Carry flag
        MOVS    pc,lk

        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------

swi
        ; in:   cIN  = instruction
        ;       lPC  = current PC and PSR
        ;       sptr = current ARM state
        ; out:  exit through "execute_continue"

        ; We could decode the SWI number here and provide the user with
        ; a textual equivalent (though this should probably be done in
        ; the disassembler).

        LDR     t0,[sptr,#ARMstate_FLAGS]       ; current control flags
        TST     t0,#doSWIs                      ; are we simulating SWIs?
        BNE     execute_realSWI                 ; set then perform real SWI
simulateSWI
        ; Enter the SWI vector
        SUB     lPC,lPC,#&04            ; subtract 4 from the PC
        STR     lPC,[sptr,#ARMstate_SVC_r14]    ; store into SVC_r14
        ; lPC = current PC and PSR
        BIC     lPC,lPC,#MODEmask       ; clear the processor mode bits
        ORR     lPC,lPC,#(SVCmode :OR: Ibit)

        MOV     t0,#vec_SWI
        BL      new_pc                  ; update the PC and swap registers

        MOV     r0,#&FFFFFFFF           ; instruction executed
        B       execute_continue

execute_realSWI
        ; We are not emulating the SWI instruction, so execute a real
        ; SWI (under the host operating system). This is possible as
        ; long as we are simulating in the correct address space.

        DISPLAY " ** real SWI execution to be done **"

        MOV     r0,#&FFFFFFFF           ; instruction executed
        B       execute_updatePC

        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------

cpdt
        ; in:   cIN  = instruction
        ;       lPC  = current PC and PSR
        ;       sptr = current ARM state
        ; out:  exit through "execute_continue"

        DISPLAY " ** cpdt not yet implemented **"

        MOV     r0,#&FFFFFFFF           ; instruction executed
        B       execute_updatePC

        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------
        ; -------------------------------------------------------------------

cpdort
        ; in:   cIN  = instruction
        ;       lPC  = current PC and PSR
        ;       sptr = current ARM state
        ; out:  exit through "execute_continue"

        DISPLAY " ** cpdort not yet implemented **"

        MOV     r0,#&FFFFFFFF           ; instruction executed
        B       execute_updatePC

        ; -------------------------------------------------------------------

	LTORG

	; ---------------------------------------------------------------------
        ; ROtate Right
dror2
	MOV	t6,lk			; return address copy
dror2_internal				; this entry point for this file only
        ; in:   t0   = Cout
        ;       t1   = op1
        ;       t2   = op2
	;	t6   = callers return address
        ;       sptr = current ARM state
        ; out:  t0   = modified Cout
	;	t1   = preserved
	;	t2   = corrupted
        ;       t3   = result
	;	t4   = corrupted
	;	sptr = preserved
	;
        TST     t2,#&FF
        MOVEQ   t3,t1                   ; no shift so,
        MOVEQS  pc,lk                   ; exit immediately

        AND     t2,t2,#&1F              ; t2 = shift
        MOV     t4,t1,LSR t2            ; t4 = op1 >> shift(t2)
        RSB     t2,t2,#32               ; shift(t2) = 32 - shift(t2)
        ORR     t3,t4,t1,LSL t2         ; result = t4 | (op1 << shift(t2))
        CMP     t3,#&00000000           ; Carry out if (result < 0)
        MOVCS   t0,#&FFFFFFFF           ; TRUE  (result < 0)
        MOVCC   t0,#&00000000           ; FALSE (result >= 0)
        MOVS    pc,lk

        ; -------------------------------------------------------------------
        ; Perform a general barrel shifter operation

genshift
        ; in:   t0   = Cout
	;	lPC  = current PC and PSR
        ;       cIN  = instruction
        ;       sptr = current ARM state
        ; out:  t0   = modified Cout
	;	t1   = corrupted
	;	t2   = corrupted
	;	t3   = result
	;	t4   = corrupted
	;	t6   = corrupted
	;	lPC  = preserved
	;	cIN  = preserved
	;	sptr = preserved

	MOV	t6,lk			; callers return address

        AND     t1,cIN,#&F              ; t1 = Rm
        LDR     t1,[sptr,t1,LSL #2]     ; load the Rm register contents
	MOV	t3,t1			; default result is (op1)

        AND     t4,cIN,#reg_shiftmask
        MOV     t4,t4,LSR #reg_shiftshift
        TEQ     t4,#&00000000           ; shift = 0
	MOVEQS	pc,t6			; YES, then return to the caller

        TEQ     t4,#&06                 ; RRX
        BEQ     drrx			; special shift form

        TST     t4,#&01
        BEQ     shiftimm

	; shift by register
        MOV     t5,t4,LSR #4
        AND     t5,t5,#&F
        LDR     t2,[sptr,t5,LSL #2]     ; load the required register

        TST     t4,#&08
	BEQ	shiftprocess

	STMFD	sp!,{t0}
	MOV	t0,#warn_badregshift
        BL      warning
	LDMFD	sp!,{t0}
	MOV	t4,#&08
        B       shiftprocess

	; ---------------------------------------------------------------------

shiftimm
        ; immediate shift
        MOV     t2,t4,LSR #3
        AND     t2,t2,#&1F
        TEQ     t2,#&00000000
        BNE     shiftprocess

        TST     t4,#&06
        MOVNE   t2,#32                  ; It isn't LSL, so op2 = 0-->32
shiftprocess
	; t0 = Cout
        ; t1 = op1
        ; t2 = op2
	; t3 = default result
	; t6 = callers return address

        MOV     t4,t4,LSR #1
        AND     t4,t4,#&03
        ADD     pc,pc,t4,LSL #2
        NOP
        B       dlsl2
        B       dlsr2
        B       dasr2
        B       dror2

        ; -------------------------------------------------------------------
        ; Logical Shift Left (only called within this file)

dlsl2
        ; in:   t0   = Cout
	;	t1   = op1
        ;       t2   = op2
	;	t6   = callers return address
        ;       sptr = current ARM state
        ; out:  t0   = modified Cout
	;	t1   = corrupted
	;	t2   = corrupted
        ;       t3   = result
	;	t4   = corrupted
	;	t6   = preserved
        ;       sptr = preserved

        AND     t4,t2,#&FF              ; t4 = shift
        MOV     t3,t1,LSL t4            ; result = op1 << shift
        TEQ     t4,#&00000000
	MOVEQS	pc,t6			; 0 shift (Cout not changed)

        RSB     t4,t4,#32               ; t4 = (32 - shift)
        MOV     t2,#1
        AND     t0,t1,t2,LSL t4         ; Cout = op1 & (1 << (32 - shift))
	MOVS	pc,t6

        ; -------------------------------------------------------------------
        ; Logical Shift Right (only called within this file)

dlsr2
        ; in:   t0   = Cout
        ;       t1   = op1
        ;       t2   = op2
	;	t6   = callers return address
        ;       sptr = current ARM state
        ; out:  t0   = modified Cout
	;	t1   = corrupted
	;	t2   = corrupted
        ;       t3   = result
	;	t4   = corrupted
	;	t6   = preserved
        ;       sptr = preserved

        AND     t4,t2,#&FF              ; t4 = shift
        MOV     t3,t1,LSR t4            ; result = (op1 >> shift)
        TEQ     t4,#&00000000
	MOVEQS	pc,t6			; 0 shift

        MOV     t2,#1
        SUB     t4,t4,#1
        AND     t0,t1,t2,LSL t4         ; Cout = op1 & (1 << (shift - 1))
	MOVS	pc,t6			; return to the caller

        ; -------------------------------------------------------------------
        ; Arithmetic Shift Right (only called within this file)
dasr2
        ; in:   t0   = Cout
        ;       t1   = op1
        ;       t2   = op2
	;	t6   = callers return address
        ;       sptr = current ARM state
        ; out:  t0   = modified Cout
	;	t1   = corrupted
	;	t2   = corrupted
        ;       t3   = result
	;	t4   = corrupted
	;	t6   = preserved
        ;       sptr = preserved

        AND     t4,t2,#&FF              ; t4 = shift
        MOV     t3,t1,LSR t4            ; result  = (op1 >> shift)
        TEQ     t4,#&00000000
	MOVEQS	pc,t6			; 0 shift

        CMP     t4,#31
        MOVGT   t4,#31
        SUBLE   t4,t4,#1
        MOV     t2,#1
        AND     t0,t1,t2,LSL t4
	MOVS	pc,t6			; return to the caller

        ; -------------------------------------------------------------------
        ; Rotate Right with eXtension (only called within this file)

drrx
        ; in:   t0   = Cout
	;	t1   = op1
	;	t3   = current result
	;	t6   = callers return address
	;	lPC  = current PC
        ;       sptr = current ARM state
        ; out:  t0   = modified Cout
	;	t1   = corrupted
        ;       t3   = result
	;	t6   = preserved
	;	lPC  = preserved
	;	sptr = preserved

	MOV	t3,t1			; result = op1
        AND     t0,t3,#&01              ; Cout = (result AND &01)
        MOV     t3,t3,LSR #1            ; result = (result >> 1)
        TST     lPC,#Cbit
	MOVNES	pc,t6			; no carry, so exit
        ORR     t3,t3,#(1 :SHL: 31)     ; input carry
	MOVS	pc,t6			; return to the caller

        ; -------------------------------------------------------------------

reg_update
        ; in:   t0   = registers number updated
        ;       sptr = current ARM state
        ; out:  all registers preserved
        STMFD   sp!,{t0,t1,lk}
        MOV     t1,t0
        ADR     t0,ru_text1
        SWI     exec_Output
        CMP     t1,#10
        MOVCS   t0,#"1"
        SWICS   exec_TerminalOut
        SUBCS   t0,t1,#10
        MOVCC   t0,t1
        ORR     t0,t0,#"0"
        SWI     exec_TerminalOut
        MOVCC   t0,#" "
        SWICC   exec_TerminalOut
        ADR     t0,ru_text2
        SWI     exec_Output
        LDR     t0,[sptr,t1,LSL #2]     ; load the register value
        SWI     exec_WriteHex8
        LDMFD   sp!,{t0,t1,pc}^

ru_text1        =       " r",&00
ru_text2        =       " = &",&00
        ALIGN

        ; -------------------------------------------------------------------

function_name   FnHead
        ; in:   t0 = address
        ; out:  all registers preserved
	STMFD	sp!,{t0,t1,t2,t3,lk}

        BIC     t1,t0,#PSRflags
        SUB     t1,t1,#4
        MOV     t3,#&FF000000
        ORR     t3,t3,t3,LSR #8
        LDR     t2,[t1,#&00]
        AND     t0,t2,t3
        TEQ     t0,#&FF000000
        LDMNEFD sp!,{t0,t1,t2,t3,pc}^

        MOV     t0,#&22
        SWI     exec_TerminalOut

        AND     t0,t2,t3,LSR #16
        SUB     t0,t1,t0
        SWI     exec_Output

        MOV     t0,#&22
        SWI     exec_TerminalOut

        LDMFD	sp!,{t0,t1,t2,t3,pc}^

        ; -------------------------------------------------------------------

warning
        ; in:   t0 = warning message number
        ;       sptr = current ARM state
        ; out:  all registers preserved

        STMFD   sp!,{t0,t1,lk}
        ADR     t1,errtable
        ADD     t0,t1,t0,LSL #2         ; t0 = t1 + (t0 * 4)
        LDR     t0,[t0,#&00]            ; load the word at this table index
        ADD     t0,t1,t0                ; get the TRUE address
        SWI     exec_Output
        LDMFD   sp!,{t0,t1,pc}^

warning_message =       "Warning &",&00
        ALIGN

        ; We should really tie the "index" and "table" closer together

                        ^       &00
warn_badshift           #       &01
warn_emerror            #       &01
warn_badmopcode         #       &01
warn_badsopcode         #       &01
warn_rmrn               #       &01
warn_badregshift        #       &01

errtable
                        &       (mess_badshift    - errtable)
                        &       (mess_emerror     - errtable)
                        &       (mess_badmopcode  - errtable)
                        &       (mess_badsopcode  - errtable)
                        &       (mess_rmrn        - errtable)
                        &       (mess_badregshift - errtable)
errtable_end

        ; warning messages
mess_badshift           =       " ** bad shift in LDR/STR **",&00
mess_emerror            =       " ** emulator error in LDM/STM **",&00
mess_badmopcode         =       " ** bad opcode in multiply **",&00
mess_badsopcode         =       " ** bad opcode in semaphore **",&00
mess_rmrn               =       " ** Rn == Rm in multiply operation **",&00
mess_badregshift        =       " ** bad register shift called **",&00
        ALIGN
	|	; middle (dbgsim) ---------------------------------------------
	LDMFD	sp!,{r11,r12}
	ORRS	pc,lk,#Vbit		; return with V set
	]	; (dbgsim)

        ; ---------------------------------------------------------------------
        ; -- exec_ResetARM ----------------------------------------------------
        ; ---------------------------------------------------------------------
	; RESET the simulator processor state.
        ; in:   r0  = ARMstate structure pointer
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r1 and r12)
        ;       r14 = callers return address
        ; out:  r0  = preserved
        ;       callers processor state restored

code_exec_ResetARM
	[	(dbgsim)
	STMFD	sp!,{r1,r2,r3,r4,r5,r6,r7,lk}	; work regs (as SingleStep)

        MOV     sptr,r0                 	; sptr = current ARM state

        MOV     r0,#&00000000
        STR     r0,[sptr,#ARMstate_nexec]       ; instructions executed
        STR     r0,[sptr,#ARMstate_icount]      ; instructions fetched

        MOV     r0,#(FIQline :OR: IRQline)
        STR     r0,[sptr,#ARMstate_FLAGS]       ; no FIQ or IRQ out-standing

        MOV     r0,#SVCmode
        STR     r0,[sptr,#ARMstate_regsmap]     ; register set mapped in
        MOV     r0,#&FFFFFFF            	; -1
        STR     r0,[sptr,#ARMstate_regswap]     ; no register set swap pending

        MOV     r0,#vec_reset           	; where we start execution from
        MOV     lPC,#(Ibit :OR: Fbit :OR: SVCmode)
        BL      new_pc

        MOV     r1,#14                  ; number of registers to randomise
randomise_loop
        ORR     r0,r1,#&A5000000        ; address exceptional value
        STR     r0,[sptr,r1,LSL #2]     ; and store this value
        SUBS    r1,r1,#1
        BPL     randomise_loop

	LDMFD	sp!,{r1,r2,r3,r4,r5,r6,r7,lk}
	LDMFD	sp!,{r11,r12}
	BICS	pc,lk,#Vbit
	|	; middle (dbgsim)
	LDMFD	sp!,{r11,r12}
	ORRS	pc,lk,#Vbit		; return with V set
	]	; (dbgsim)

        ; -------------------------------------------------------------------
        ; -- exec_SetNewPC --------------------------------------------------
        ; -------------------------------------------------------------------
	; Update the simulator PC (preserving the PSR).
        ; in:   r0  = ARMstate structure pointer
	;	r1  = new PC address
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r1 and r12)
        ;       r14 = callers return address
        ; out:  r0  = preserved
	;	r1  = preserved
        ;       callers processor state restored

code_exec_SetNewPC
	[	(dbgsim)
	STMFD	sp!,{r1,r2,r3,r4,r5,r6,r7,lk}	; work regs (as SingleStep)

        MOV     sptr,r0		                ; sptr = current ARM state
        MOV     r0,r1
        LDR     lPC,[sptr,#ARMstate_cPC]	; current PC and PSR
        BL      new_pc

	LDMFD	sp!,{r1,r2,r3,r4,r5,r6,r7,lk}
	LDMFD	sp!,{r11,r12}
	BICS	pc,lk,#Vbit
	|	; middle (dbgsim)
	LDMFD	sp!,{r11,r12}
	ORRS	pc,lk,#Vbit		; return with V set
	]	; (dbgsim)

        ; -------------------------------------------------------------------
        ; -- exec_RegisterDump ----------------------------------------------
        ; -------------------------------------------------------------------
	; Dump the simulator registers to the 2nd (debugging) link.
        ; in:   r0  = ARMstate structure pointer
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r1 and r12)
        ;       r14 = callers return address
        ; out:  r0  = preserved
        ;       callers processor state restored

code_exec_RegisterDump
	[	(dbgsim)
	STMFD	sp!,{r0,r1,r2,r3,lk}		; work registers

	MOV	r1,r0				; current ARM processor state
        ADRL    r2,reg_name_table		; register names
        MOV     r3,#0				; register index
register_dump_loop
        ; display register "r3"
        MOV     r0,r2
        BL      local_Output
        LDR     r0,[r1,r3,LSL #2]
	BL	local_WriteHex8  

        ADD     r2,r2,#12               	; step over this name
        ADD     r3,r3,#1
        TST     r3,#(word - 1)
        BLEQ 	local_NewLine
        TEQ     r3,#16
        BNE     register_dump_loop

        ADRL    r0,flags_text
        BL      local_Output
        LDR     r0,[r1,#ARMstate_cPC]
        BL      local_FlagStatus

        ADRL    r0,mode_text
        BL      local_Output
        LDR     r0,[r1,#ARMstate_cPC]
        BL      local_ModeStatus

        ADRL    r0,mode_text_end
 	BL      local_Output

	LDMFD	sp!,{r0,r1,r2,r3,lk}
	LDMFD	sp!,{r11,r12}
	BICS	pc,lk,#Vbit

reg_name_table
        =       "  r0/a1 = &",&00       ; 12bytes
        =       "  r1/a2 = &",&00
        =       "  r2/a3 = &",&00
        =       "  r3/a4 = &",&00
        =       "  r4/v1 = &",&00
        =       "  r5/v2 = &",&00
        =       "  r6/v3 = &",&00
        =       "  r7/v4 = &",&00
        =       "  r8/v5 = &",&00
        =       "  r9/dp = &",&00
        =       " r10/sl = &",&00
        =       " r11/fp = &",&00
        =       " r12/ip = &",&00
        =       " r13/sp = &",&00
        =       " r14/lk = &",&00
        =       " r15/pc = &",&00
reg_name_table_end
        ASSERT  ((reg_name_table_end - reg_name_table) = (16 * 12))

flags_text      =       "Flags: ",&00
mode_text       =       " (Mode ",&00
mode_text_end   =       ")",&0A,&00
        ALIGN
	|	; middle (dbgsim)
	LDMFD	sp!,{r11,r12}
	ORRS	pc,lk,#Vbit		; return with V set
	]	; (dbgsim)

        ; -------------------------------------------------------------------
        ; -- exec_FlagStatus ------------------------------------------------
        ; -------------------------------------------------------------------
	; Display the processor PSR flags to the 2nd (debugging) link.
        ; in:   r0  = current PC and PSR
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r1 and r12)
        ;       r14 = callers return address
        ; out:  r0  = preserved
        ;       callers processor state restored

local_FlagStatus
	STMFD	sp!,{r11,r12}
code_exec_FlagStatus
	[	(dbgsim)
	STMFD	sp!,{r0,r1,r2,r3,lk}

        AND     r3,r0,#HIflags          ; clear MODE and address bits
        MOV     r1,#6                   ; number of flags
        ADRL    r2,flag_status_flags    ; the flag names
flag_status_loop
        MOVS    r3,r3,LSL #1            ; shift top-bit into Carry
        LDRB    r0,[r2],#&01            ; get this character
        ANDCS   r0,r0,#&DF              ; upper-case if Carry set
	BL	local_TerminalOut	; and display the character
        SUBS    r1,r1,#1
        BNE     flag_status_loop

	LDMFD	sp!,{r0,r1,r2,r3,lk}
	LDMFD	sp!,{r11,r12}
	BICS	pc,lk,#Vbit

flag_status_flags       =       "nzcvif"
	|	; middle (dbgsim)
	LDMFD	sp!,{r11,r12}
	ORRS	pc,lk,#Vbit		; return with V set
	]	; (dbgsim)

        ; -------------------------------------------------------------------
	; -- exec_ModeStatus ------------------------------------------------
        ; -------------------------------------------------------------------
	; Display the processor mode to the 2nd (debugging) link.
        ; in:   r0  = current PC and PSR
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r1 and r12)
        ;       r14 = callers return address
        ; out:  r0  = preserved
        ;       callers processor state restored
local_ModeStatus
	STMFD	sp!,{r11,r12}
code_exec_ModeStatus
	[	(dbgsim)
	STMFD	sp!,{r0,r1,lk}

        ANDS    r1,r0,#MODEmask         ; r1 = processor mode
        ADRL    r0,mode_status_modes    ; and address the table
        ADD     r0,r0,r1,LSL #2         ; r0 = table + (mode * 4)
	BL 	local_Output            ; and display the mode

	LDMFD	sp!,{r0,r1,lk}
	LDMFD	sp!,{r11,r12}
	BICS	pc,lk,#Vbit

mode_status_modes                       ; each of these should be 4bytes
        =       "USR",&00
        =       "FIQ",&00
        =       "IRQ",&00
        =       "SVC",&00
	|	; middle (dbgsim)
	LDMFD	sp!,{r11,r12}
	ORRS	pc,lk,#Vbit		; return with V set
	]	; (dbgsim)

        ; ---------------------------------------------------------------------
        LNK     losched.s
