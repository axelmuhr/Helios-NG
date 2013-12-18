        SUBT ARM PCS register definitions                           > PCSregs/s
        ;    Copyright (c) 1989, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; The Acorn FPE source uses some of the following registers names
        ; (attached to different registers) in its source. This file is
        ; provided seperately from "arm.s" so that we do not need to change
        ; the FPE source.
        ; ---------------------------------------------------------------------

        ASSERT  (arm_s)         ; ensure "arm.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"including PCSregs.s"
                GBLL    PCSregs_s
PCSregs_s       SETL    {TRUE}

        ; ---------------------------------------------------------------------

a1      RN      r0			; PCS register based argument
a2      RN      r1			; PCS register based argument
a3      RN      r2			; PCS register based argument
a4      RN      r3			; PCS register based argument

v1      RN      r4			; C "register" variable
v2      RN      r5			; C "register" variable
v3      RN      r6			; C "register" variable
v4      RN      r7			; C "register" variable
v5      RN      r8			; C "register" variable

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF PCSregs/s
