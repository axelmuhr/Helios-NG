import ___stack_overflow_1
import ___uremainder


_initsym:
        mov     r12, r13
        stmfd   r13!, {r4,r5,r11,r12,lr,pc}
        sub     r11, r12, #4
        sub     r12, r13, #12
        cmp     r12, r10
        bllt    0
        mov     r4, #0
        mov     r0, r4
        ldr     r1, [r9, #0]
        add     r1, r1, #0
        add     r1, r1, #0
        ldr     r2, [r9, #0]
        add     r2, r2, #0
        add     r2, r2, #0
        mov     r3, #29
        add     r3, r3, #512
        ldr     r1, [r1, #0]
