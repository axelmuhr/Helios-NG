;written by Russell Nelson, improved by Jan Engvald to drain capacitive mem.

memory_test:
;enter with ax = segment of memory to test, cx = number of bytes to test.
;exit with ne, bx->failing byte if the memory test failed.
        push    ds
        mov     ds,ax
        mov     bx,-1
memory_test_1:
        inc     bx
        mov     al,ds:[bx]              ;get a copy of the location.
        mov     ah,ds:[bx+2]            ;get a copy of the location.
        not     al
        mov     ds:[bx],al              ;try to store the complement.
        mov     ds:[bx+2],ah            ;drain any capacitive memory
        mov     ah,al
        not     ah
        cmp     ds:[bx],al              ;did the store work?
        mov     ds:[bx],ah              ;(in any case, restore the original)
        loope   memory_test_1           ;keep going if the store worked.
        pop     ds
        ret
