; --   -------------------------------------------------------------------------
; --                                                                            
; --      ISERVER  -  INMOS standard file server                                
; --                                                                            
; --      b010asm.asm                                                           
; --                                                                            
; --      8086 assembler routines for b004link.c                                
; --      Generic MS-DOS version written for NEC PC                             
; --                                                                            
; --      Copyright (c) INMOS Ltd., 1988, 1990.
; --      All Rights Reserved.                                                  
; --                                                                            
; --   -------------------------------------------------------------------------
                                                                                
                                                                                
                DOSSEG                                                          
                .MODEL  SMALL                                                   
                                                                                
                                                                                
                .DATA                                                           
                                                                                
extrn           _C012_idr:word                                                  
extrn           _C012_odr:word                                                  
extrn           _C012_isr:word                                                  
extrn           _C012_osr:word                                                  
                                                                                
                .CODE                                                           
                                                                                
                ASSUME CS:_TEXT, DS:DGROUP                                      
                                                                                
PUBLIC          _ReadLink                                                       
                                                                                
readtime        macro                                                           
                push    cx                                                      
                push    dx                                                      
                ; read system time (DH = seconds, DL = hundredths)              
                mov     ah, 2ch                                                 
                int     21h                                                     
                ; convert to hundredths of a second: AX := (DH * 100) + DL      
                mov     al, dh  ; AX := DH * 100                                
                mov     cl, 100                                                 
                mul     cl                                                      
                mov     dh, 0   ; AX := AX + DL                                 
                add     ax, dx                                                  
                pop     dx                                                      
                pop     cx                                                      
                endm                                                            
                                                                                
timeout         dw      0                                                       
timeout_fl      db      0                                                       
last_count      dw      0                                                       
                                                                                
_ReadLink       proc    ;ReadLink( int linkid, char *buffer, int count, int time
                push    bp                                                      
                mov     bp, sp                                                  
                push    es                                                      
                push    si                                                      
                push    di                                                      
                                                                                
                push    ds                                                      
                pop     es                                                      
                mov     ax, [bp+10]     ;timeout                                
                mov     bx, 10          ;multiply timeout by 10 to get time in h
                mul     bx                                                      
                mov     cs:[timeout], ax                                        
                                                                                
                mov     cx, [bp+8]      ;count                                  
                mov     bx, [bp+6]      ;buffer                                 
                                                                                
                mov     cs:[last_count], ax                                     
                                                                                
                                                                                
                                                                                
                mov     dx, _C012_isr                                           
                mov     di, bx                                                  
                mov     bx, _C012_idr                                           
                readtime                                                        
                add     ax, cs:[timeout]                                        
                dec     ax                                                      
                mov     si, ax                                                  
                mov     cs:[timeout_fl], 0                                      
                jcxz    rd2                                                     
rd1:            in      al, dx                                                  
                test    al, 1                                                   
                jz      rnot_ready                                              
                xchg    dx, bx                                                  
                in      al, dx                                                  
                stosb                                                           
                xchg    bx, dx                                                  
                loop    rd1             ; and do next                           
                jmp     rd2                                                     
rnot_ready:                                                                     
                ; if timeout is zero then no timeout                            
                mov     ax, cs:[timeout]                                        
                and     ax, ax                                                  
                jz      rd1                                                     
                ; If a timeout hasn't happened, get back into the polling loop  
                readtime                                                        
                cmp     si, ax                                                  
                jge     rd1                                                     
                                                                                
        ;Is the count the same as it was at the last timeout                    
                                                                                
                cmp     cx, cs:[last_count]                                     
                je      r_get_out       ;Yes  -- exit                           
                                                                                
        ;Record the new count and reset the timeout                             
                                                                                
                mov     cs:[last_count], cx                                     
                readtime                                                        
                add     ax, cs:[timeout]                                        
                dec     ax                                                      
                mov     si, ax                                                  
                jmp     rd1                                                     
                                                                                
r_get_out:      mov     cs:[timeout_fl], 2                                      
                mov     ax, [bp+8]                                              
                sub     ax, cx                                                  
                jmp     rdterm                                                  
                                                                                
rd2:            mov     ax, [bp+8]                                              
rdterm:         pop     di                                                      
                pop     si                                                      
                pop     es                                                      
                pop     bp                                                      
                ret                                                             
_ReadLink       endp                                                            
                                                                                
PUBLIC          _WriteLink                                                      
_WriteLink      proc    near ;Writelink( int linkid, char *buffer, int count, in
                push    bp                                                      
                mov     bp, sp                                                  
                                                                                
                push    es                                                      
                push    si                                                      
                push    di                                                      
                push    ds                                                      
                pop     es                                                      
                mov     ax, [bp+10]     ;timeout                                
                mov     bx, 10          ;multiply timeout by 10 to get time in h
                mul     bx                                                      
                mov     cs:[timeout], ax                                        
                                                                                
                mov     cx, [bp+8]      ;count                                  
                mov     bx, [bp+6]      ;buffer                                 
                                                                                
                mov     cs:[last_count], cx                                     
                                                                                
                mov     dx, _C012_osr                                           
                mov     si, bx                                                  
                mov     bx, _C012_odr                                           
                mov     ax, 40h                                                 
                mov     es, ax                                                  
                readtime                                                        
                add     ax, cs:[timeout]                                        
                dec     ax                                                      
                mov     di, ax                                                  
                mov     cs:[timeout_fl], 0                                      
                mov     di, ax                                                  
                jcxz    wr2                                                     
wr1:            in      al, dx                                                  
                test    al, 1                                                   
                jz      not_ready                                               
                xchg    dx, bx                                                  
                lodsb                                                           
                out     dx, al                                                  
                xchg    bx, dx                                                  
                loop    wr1             ; and do next                           
                jmp     wr2                                                     
not_ready:                                                                      
                ; if timeout is zero then no timeout                            
                mov     ax, cs:[timeout]                                        
                and     ax, ax                                                  
                jz      wr1                                                     
                readtime                                                        
                cmp     di, ax                                                  
                jge     wr1                                                     
                                                                                
        ;Is the count the same as it was at the last timeout                    
                                                                                
                cmp     cx, cs:[last_count]                                     
                je      wr_get_out      ;Yes  -- exit                           
                                                                                
        ;Record the new count and reset the timeout                             
                                                                                
                mov     cs:[last_count], cx                                     
                readtime                                                        
                add     ax, cs:[timeout]                                        
                dec     ax                                                      
                mov     di, ax                                                  
                jmp     wr1                                                     
                                                                                
wr_get_out:                                                                     
                mov     cs:[timeout_fl], 2                                      
                mov     ax, [bp+8]                                              
                sub     ax, cx                                                  
                jmp     wrterm                                                  
wr2:                                                                            
                mov     ax, [bp+8]                                              
wrterm:         pop     di                                                      
                pop     si                                                      
                pop     es                                                      
                pop     bp                                                      
                ret                                                             
_WriteLink       endp                                                           
                                                                                
                END                                                             
                                                                                
                                                                                
;                                                                               
;   Eof                                                                         
;                                                                               

