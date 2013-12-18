_report ['include i860.m]
_def 'i860.m_flag  1

_def 'pte_p              [0x001]          -- Present
_def 'pte_w              [0x002]          -- Writable
_def 'pte_u              [0x004]          -- User page
_def 'pte_wt             [0x008]          -- Write through
_def 'pte_cd             [0x010]          -- Cache disable
_def 'pte_a              [0x020]          -- Accessed
_def 'pte_d              [0x040]          -- Dirty
_def 'pte_avlshft        9              -- Shift to programmer bits
_def 'pte_avlmsk         [0x3]            -- and width
_def 'pte_pfamask        [0xfffff000]     -- Mask for Page Frame address

-- Processor status register

_def 'psr_pmshft         24             -- Pixel Mask shift
_def 'psr_pmmsk          [0xff]           -- and width
_def 'psr_psshft         22             -- Pixel size shift
_def 'psr_psmsk          [0x3]            -- and width
_def 'psr_scshft         17             -- Sift count shift
_def 'psr_scmsk          [0x1f]           -- and width
_def 'psr_knf            [0x8000]         -- Kill next floating ins.
_def 'psr_dim            [0x4000]         -- Dual Instruction mode
_def 'psr_ds             [0x2000]         -- Delayed switch
_def 'psr_ft             [0x1000]         -- Floating trap
_def 'psr_dat            [0x0800]         -- Data access trap
_def 'psr_iat            [0x0400]         -- Instruction access trap
_def 'psr_in             [0x0200]         -- Interrupt
_def 'psr_it             [0x0100]         -- Instruction trap
_def 'psr_pu             [0x0080]         -- Previous user mode
_def 'psr_u              [0x0040]         -- User Mode
_def 'psr_pim            [0x0020]         -- Previous interrupt mode
_def 'psr_im             [0x0010]          -- Interrupt mode
_def 'psr_lcc            [0x0008]         -- Loop condition code
_def 'psr_cc             [0x0004]         -- Condition code
_def 'psr_bw             [0x0002]         -- Break write
_def 'psr_br             [0x0001]         -- Break read

-- Extended processor status register

_def 'epsr_ptypeshft     0
_def 'epsr_ptypemsk      [0xff]
_def 'epsr_stepshft      8
_def 'epsr_stepmsk       [0x1f]
_def 'epsr_il            [0x00002000]        -- Interlock
_def 'epsr_wp            [0x00004000]        -- Write-protect mode
--                 [0x00008000]        -- reserved
--                 [0x00001000]        -- reserved
_def 'epsr_int           [0x00020000]        -- Interrupt
_def 'epsr_dcsshft       18                -- Data cache size
_def 'epsr_dcsmsk        [0xf]               -- and width
_def 'epsr_pbm           [0x00400000]        -- Page table bit mode
_def 'epsr_be            [0x00800000]        -- Big-endian mode
_def 'epsr_of            [0x01000000]        -- Overflow flag
-- all other bits reserved

-- Directory base register

_def 'db_ate             [0x0001]            -- Address translate enable
_def 'db_dpsshft         1
_def 'db_dpsmsk          [0x7]
_def 'db_bl              [0x0010]
_def 'db_iti             [0x0020]            -- I-cache, TLB invalidate
--                 [0x0040            -- reserved
_def 'db_cs8             [0x0080]            -- Code size 8
_def 'db_rbshft          8                 -- Replacement block
_def 'db_rbmsk           3
_def 'db_rcshft          10                -- Replacement control
_def 'db_rcmsk           3
_def 'db_dtbmsk          [0xfffff000]        -- DTB mask

-- Floating status register

_def 'fsr_fz             [0x00000001]        -- Flush zero
_def 'fsr_ti             [0x00000002]        -- Trap inexeact
_def 'fsr_rmshft         2                 -- Rounding mode shift
_def 'fsr_rmmsk          3                 -- and width
_def 'fsr_u              [0x00000010]        -- Update
_def 'fsr_fte            [0x00000020]        -- Floating-point trap enable
--                 [0x00000040        -- reserved
_def 'fsr_si             [0x00000080]        -- Sticky inexact flag
_def 'fsr_se             [0x00000100]        -- Source exception
_def 'fsr_mu             [0x00000200]        -- Multiplier underflow
_def 'fsr_mo             [0x00000400]        -- Multiplier overflow
_def 'fsr_mi             [0x00000800]        -- Multiplier inexact
_def 'fsr_ma             [0x00001000]        -- Multiplier add one
_def 'fsr_au             [0x00002000]        -- Adder underflow
_def 'fsr_ao             [0x00004000]        -- Adder overflow
_def 'fsr_ai             [0x00008000]        -- Adder inexact
_def 'fsr_aa             [0x00010000]        -- Adder add one
_def 'fsr_rrshft         17                -- Result register mask
_def 'fsr_rrmsk          [0x1f]              -- and width
_def 'fsr_aeshft         22                -- Adder exponent
_def 'fsr_aemsk          [0x7]               -- and width
--                 [0x02000000]        -- reserved
_def 'fsr_lrp            [0x04000000]        -- Load pipe result precision
_def 'fsr_irp            [0x08000000]        -- Integer pipe result precision
_def 'fsr_mrp            [0x10000000]       -- Multiplier pipe result precision
_def 'fsr_arp            [0x20000000]        -- Adder pipe result precision

-- bits 30-31 reserved

_def 'rz               r0
_def 'rl               r1                -- return link reg
_def 'sp               r2
_def 'fp               r3
_def 'rl0              r4
_def 'rp0              r16
_def 'rr               r16
_def 'rt0              r28
_def 'rat              r31

_def 'fz               f0                -- And 1
_def 'fl0              f2                -- and 3 ...
_def 'fp0              f16
_def 'fr               f16
_def 'ft0              f28

