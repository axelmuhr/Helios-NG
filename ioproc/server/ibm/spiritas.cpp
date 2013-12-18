;*---------------------------------------------------------------------------*;
;--                        HELIOS PC  I/O SYSTEM                            --;
;--                        =====================                            --;
;--                                                                         --;
;--             Copyright (C) 1993, Perihelion Software Ltd.                --;
;--               All Rights Reserved.                                      --;
;--                                                                         --;
;--                                                                         --;
;--                                                                         --;
;-- MODULE NAME: spiritas.cpp                                               --;
;--                                                                         --;
;-- AUTHOR : NHG  (based on code supplied by Sonitech)                      --;
;-----------------------------------------------------------------------------;
;-- RcsId: $Id: spiritas.cpp,v 1.1 1993/03/10 15:34:52 bart Exp $	--;
;-- Copyright (C) 1988, Perihelion Software Ltd.   			    --;
;

.286

CODE    SEGMENT WORD public 'CODE'
        ASSUME cs:CODE
;
;       ----------------------------------------------
;                      asm_dl_data
;       ----------------------------------------------
;       Calling sequence from 'C' source:
;
;       asm_dl_data(DATA PORT ADDRESS, COUNT, ARRAY)
;
;          unsigned   DATA PORT ADDRESS
;          unsigned   COUNT
;          long      *ARRAY
;       ----------------------------------------------
;
;       [BP+8]   =   DATA PORT ADDRESS
;       [BP+10]  =   COUNT : No of 16 bit data to download.
;       [BP+12]  =   ARRAY offset
;       [BP+14]  =   ARRAY segment
;
;
	PUBLIC  _asm_dl_data

;
_asm_dl_data    PROC   FAR
;
; --------------- save registers on stack ---------------
;
        PUSH    BP              ;; save BP 
        PUSH    DS              ;; save DS
        MOV     BP,SP           ;; point BP at top of stack
                                ;   [BP+0] = old DS
                                ;   [BP+2] = old BP
                                ;   [BP+4] = offset of return address
                                ;   [BP+6] = segment of return address
        PUSH    SI              ;; save SI
        PUSH    DX              ;; save DX
;
; --- get DATA PORT ADDRESS, COUNT and ARRAY address -------------
;
        MOV     DX,[BP+8]       ;; data port address of SPIRIT-40 port.
        MOV     CX,[BP+10]      ;; get count from the stack 
        OR      CX,CX           ;;
;
        JZ      DLLONG_OVER     ;;
        MOV     BX,[BP+14]      ;; get array segment address 
        MOV     DS,BX           ;; load it in DS
        MOV     SI,[BP+12]      ;; get array offset address

        CLD                     ;; set direction flag for auto 
                                ;   increment of index register
;
; --- do continuous write ---------------------------------------
;
        REP     OUTSW           ;; continuous write of COUNT words
;
; ------------- restore saved registers from stack -------------
;
DLLONG_OVER:
        POP     DX              ;; restore DX
        POP     SI              ;; restore SI
        POP     DS              ;; restore DS
        POP     BP              ;; restore BP
        RET                     ;; return
;
_asm_dl_data      ENDP
;
; ****************** end of asm_dl_data() *******************
;
;
;       ----------------------------------------------
;                      asm_ul_data
;       ----------------------------------------------
;       Calling sequence from 'C' source:
;
;       asm_ul_data(DATA PORT ADDRESS, COUNT, ARRAY)
;
;          unsigned   DATA PORT ADDRESS
;          unsigned   COUNT
;          long      *ARRAY
;       ----------------------------------------------
;       stack structure same as asm_dl_data()
;
	PUBLIC  _asm_ul_data
;
_asm_ul_data     PROC  FAR
;
; --------------- save registers on stack ---------------
;
        PUSH    BP              ;; save BP 
        PUSH    ES              ;; save ES
        MOV     BP,SP           ;; point BP at top of stack
        PUSH    DI              ;; save DI
        PUSH    DX              ;; save DX
;
; ---- GET DATA PORT ADDRESS, COUNT and ARRAY address -------------
;
        MOV     DX,[BP+8]       ;; get data port address.
        MOV     CX,[BP+10]      ;; get count from the stack 
        OR      CX,CX           ;;
;
        JZ      ULLONG_OVER     ;;
        MOV     BX,[BP+14]      ;; get array segment address 
        MOV     ES,BX           ;; load it in ES
        MOV     DI,[BP+12]      ;; get array offset address
;
        CLD                     ;; set direction flag for auto 
                                ;; increment of index register
;
; --- do continuous read -------
;
        REP     INSW           ;; continuous read of COUNT words
;
; ------------- restore saved registers from stack -------------
;
ULLONG_OVER:
        POP     DX              ;; restore DX
        POP     DI              ;; restore DI
        POP     ES              ;; restore ES
        POP     BP              ;; restore BP
        RET                     ;; return
;
_asm_ul_data      ENDP
;
; ****************** end of asm_ul_data() *******************



CODE    ENDS

        END

