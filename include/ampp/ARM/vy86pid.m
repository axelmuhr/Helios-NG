-- File:	vy86pid.m
-- Subsystem:	Helios/ARM implementation
-- Author:	JGSmith
-- Date:	930310
--
-- Description: Hardware description of the VY86PID board.
--		Memory maps.
--		INTC interrupt controller manifests.
--		VL16C551 Asynchronous Communications device manifests.
--
-- Copyright (c) 1993, VLSI Technology Inc.
-- All Rights Reserved.

_report ['include vy86pid.m]
_def 'ARM/vy86pid.m_flag 1

-----------------------------------------------------------------------------
-- The VY86PID memory map is structured as follows. Only the lo-64MBytes of
-- the address space is allocated. The ROM bank is mapped at 0x00000000
-- (over the main DRAM bank) by RESET. The DRAM ghosts within its bank.

_def 'DRAM_base		0x00000000		  -- normal DRAM location 
_def 'DRAM_size		[(16 << 20)]		  -- maximum DRAM size 

_def 'DRAM2_base	[(DRAM_base + DRAM_size)]  -- unused DRAM bank 
_def 'DRAM2_size	[(16 << 20)]

_def 'IO_base		[(DRAM2_base + DRAM2_size)] -- physical base of I/O 
_def 'IO_size		[(16 << 20)]

_def 'ROM_base		[(IO_base + IO_size)] 	  -- ROM/EPROM etc. 
_def 'ROM_size		[(16 << 20)]

-------------------------------------------------------------------------------
---- I/O memory map -----------------------------------------------------------
-------------------------------------------------------------------------------

_def 'IO_base_internal		[(IO_base + 0x00000000)]
_def 'IO_base_expslot_DMA	[(IO_base + 0x00400000)]
_def 'IO_base_expslot_IO	[(IO_base + 0x00800000)]
_def 'IO_base_expslot_MEM	[(IO_base + 0x00C00000)]

-------------------------------------------------------------------------------
-- INTC PGA Registers 

-- The following structure is based at hw_INTC 
struct hw_intc [
	byte IRQS 	 -- RO : IRQ Status register 
		byte pad0a	byte pad0b	byte pad0c
	byte IRQM 	 -- WO : IRQ Mask register 
		byte pad1a	byte pad1b	byte pad1c
	byte FIQS  	 -- RO : FIQ Status register 
		byte pad2a	byte pad2b	byte pad2c
	byte FIQM  	 -- WO : FIQ Mask register 
		byte pad3a	byte pad3b	byte pad3c
]
_def 'hw_intc.IRQRST hw_intc.IRQS        -- WO : IRQ ReSeT register 

-- Provide direct access to the INTC I/O device 
_def 'hw_INTC [(IO_base_internal + 0x00)]

-- IRQS : IRQ status register 
-- This register contains the raw interrupt input (without masking).
-- This register is ANDed with the IRQM (mask). An IRQ is generated if
-- the result is non-zero.
 
_def 'hw_intc_irq_serial1       [(1 << 0)] -- Serial port 1 
_def 'hw_intc_irq_timer         [(1 << 1)] -- Timer 
_def 'hw_intc_irq_parallel      [(1 << 2)] -- Parallel port 
_def 'hw_intc_irq_expslot3      [(1 << 3)] -- expansion slot irq 3 
_def 'hw_intc_irq_expslot4      [(1 << 4)] -- expansion slot irq 4 
_def 'hw_intc_irq_expslot5      [(1 << 5)] -- expansion slot irq 5 
_def 'hw_intc_irq_expslot6      [(1 << 6)] -- expansion slot irq 6 
_def 'hw_intc_irq_panic         [(1 << 7)] -- PANIC button 

-- IRQRST : IRQ reset register 
-- Writing to the following bits will reset the corresponding
-- interrupt. The other interrupt sources are cleared by the relevant I/O
-- device.
 
_def 'hw_intc_resetirq_timer    [hw_intc_irq_timer]
_def 'hw_intc_resetirq_panic    [hw_intc_irq_panic]

-- IRQM : IRQ mask register 
-- An interrupt source is enabled by writing a 1 into the
-- corresponding bit. This register is reset by power-on.
-- Bit 7 (corresponding to the PANIC IRQ is always enabled).
 
_def 'hw_intc_enableirq_serial1         [hw_intc_irq_serial1]
_def 'hw_intc_enableirq_timer           [hw_intc_irq_timer]
_def 'hw_intc_enableirq_parallel        [hw_intc_irq_parallel]
_def 'hw_intc_enableirq_expslot3        [hw_intc_irq_expslot3]
_def 'hw_intc_enableirq_expslot4        [hw_intc_irq_expslot4]
_def 'hw_intc_enableirq_expslot5        [hw_intc_irq_expslot5]
_def 'hw_intc_enableirq_expslot6        [hw_intc_irq_expslot6]

-- FIQS : FIQ status register 
-- This register contains the raw fast-interrupt input (without masking).
-- This register is ANDed with the FIQM (mask). A FIQ is generated if
-- the result is non-zero.
 
_def 'hw_intc_fiq_serial1_tx    [(1 << 0)] -- Serial port TX DMA 
_def 'hw_intc_fiq_serial1_rx    [(1 << 1)] -- Serial port RX DMA 
_def 'hw_intc_fiq_expslot       [(1 << 2)] -- Expansion slot 
_def 'hw_intc_fiq_nmi           [(1 << 3)] -- LSA port 
_def 'hw_intc_fiq_expslot3      [(1 << 4)] -- Expansion slot DMA 
_def 'hw_intc_fiq_expslot4      [(1 << 5)] -- Expansion slot DMA 
_def 'hw_intc_fiq_expslot5      [(1 << 6)] -- Expansion slot DMA 
_def 'hw_intc_fiq_expslot6      [(1 << 7)] -- Expansion slot DMA 

-- FIQM : FIQ mask register 
-- An interrupt source is enabled by writing a 1 into the
-- corresponding bit. This register is reset by power-on.
 
_def 'hw_intc_enablefiq_serial1_tx      [hw_intc_enablefiq_serial1_tx]
_def 'hw_intc_enablefiq_serial1_rx      [hw_intc_enablefiq_serial1_rx]
_def 'hw_intc_enablefiq_expslot         [hw_intc_enablefiq_expslot]
_def 'hw_intc_enablefiq_nmi             [hw_intc_enablefiq_nmi]
_def 'hw_intc_enablefiq_expslot3        [hw_intc_enablefiq_expslot3]
_def 'hw_intc_enablefiq_expslot4        [hw_intc_enablefiq_expslot4]
_def 'hw_intc_enablefiq_expslot5        [hw_intc_enablefiq_expslot5]
_def 'hw_intc_enablefiq_expslot6        [hw_intc_enablefiq_expslot6]

-------------------------------------------------------------------------------
-- VL16C551 Serial Port Registers 

-- The following registers are offset from "hw_serial_port_base" 
struct hw_serial [
        byte RBR                -- RO : Receive Buffer Register 
                byte pad0a      byte pad0b      byte pad0c
        byte IER                -- RW : Interrupt Enable Register 
                byte pad1a      byte pad1b      byte pad1c
        byte IIR                -- RO : Interrupt Identification Register 
                byte pad2a      byte pad2b      byte pad2c
        byte LCR                -- RW : Line Control Register 
                byte pad3a      byte pad3b      byte pad3c
        byte MCR                -- RW : Modem Control Register 
                byte pad4a      byte pad4b      byte pad4c
        byte LSR                -- RW : Line Status Register 
                byte pad5a      byte pad5b      byte pad5c
        byte MSR                -- RW : Modem Status Register 
                byte pad6a      byte pad6b      byte pad6c
        byte SCR                -- RW : Scratch Register 
                byte pad7a      byte pad7b      byte pad7c
]

_def 'hw_serial.THR hw_serial.RBR   -- WO : Transmitter Holding Register 
_def 'hw_serial.FCR hw_serial.IIR   -- WO : FIFO Control Register 
_def 'hw_serial.DLL hw_serial.RBR   -- RW : Divisor Latch LSB 
_def 'hw_serial.DLM hw_serial.IER   -- RW : Divisor Latch MSB 

_def 'hw_SERIAL [(IO_base_internal + 0x20)]

-- LCR : Line Control Register 
_def 'hw_serial_LCR_WLS_mask    [0x03]  -- mask for word-length field 
_def 'hw_serial_LCR_WL_5bits    [0x0]
_def 'hw_serial_LCR_WL_6bits    [0x1]
_def 'hw_serial_LCR_WL_7bits    [0x2]
_def 'hw_serial_LCR_WL_8bits    [0x3]
_def 'hw_serial_LCR_SBSEL       [(1 << 2)] -- 0 = 1 stop bit; 1 = 2 stop bits 
_def 'hw_serial_LCR_PAREN       [(1 << 3)] -- 0 = no parity; 1 = parity 
_def 'hw_serial_LCR_EVPAR       [(1 << 4)] -- 0 = odd parity; 1 = even parity 
_def 'hw_serial_LCR_STICK       [(1 << 5)] -- 0 = disabled; 1 = enabled 
_def 'hw_serial_LCR_BREAK       [(1 << 6)] -- 0 = disable break; 1 = enable break 
_def 'hw_serial_LCR_DLAB        [(1 << 7)] -- 0 = data; 1 = divisor latches 

-- The following is a reasonable default setup for LCR:
--      8bits; 1 stop bit; no parity; divisor latch access off
 
_def 'hw_serial_LCR_default     [hw_serial_LCR_WL_8bits]

-- DLL and DLM : Divisor latch registers 
-- This is viewed as a 16bit register. The registers are only
-- accessible when the "hw_serial_LCR_DLAB" bit is set in LCR.
-- The VY86PID uses a 1.8432MHz input clock.
 
_def 'hw_serial_DLR_300         [384]
_def 'hw_serial_DLR_1200        [96]
_def 'hw_serial_DLR_2400        [48]
_def 'hw_serial_DLR_4800        [24]
_def 'hw_serial_DLR_9600        [12]
_def 'hw_serial_DLR_19200       [6]
_def 'hw_serial_DLR_38400       [3]
_def 'hw_serial_DLR_56000       [2]     -- error = 2.85% 

-- IER : Interrupt Enable Register 
_def 'hw_serial_IER_ERBFI       [(1 << 0)] -- 1 = enable Rx data available interrupt 
_def 'hw_serial_IER_ETBEI       [(1 << 1)] -- 1 = enable Tx holding reg. empty interrupt 
_def 'hw_serial_IER_ELSI        [(1 << 2)] -- 1 = enable Rx line status interrupt 
_def 'hw_serial_IER_EDSSI       [(1 << 3)] -- 1 = enable modem status interrupt 

-- IIR : Interrupt Identification Register 
_def 'hw_serial_IIR_IPENDN      [1]          -- 0 = interrupt pending; 1 = none 
_def 'hw_serial_IIR_IID_mask	[(0x3 << 1)] -- pending interrupts 
_def 'hw_serial_IIR_IID_MS	[(0x0 << 1)] -- Modem Status 
_def 'hw_serial_IIR_IID_THRE	[(0x1 << 1)] -- Tx Holding Reg. Empty 
_def 'hw_serial_IIR_IID_RDA	[(0x2 << 1)] -- Rx Data Available 
_def 'hw_serial_IIR_IID_RLS	[(0x3 << 1)] -- Rx Line Status 
-- NOTE: The following is the status when the "hw_serial_IIR_FIFEN" bit is
-- also set.
 
_def 'hw_serial_IIR_IID_FS	[(0x2 << 1)] -- FIFO Status; FIFO thresh; wait timeout 
_def 'hw_serial_IIR_FIFEN	[(1 << 3)]   -- 0 = 16C450 mode; 1 = 16C550 mode 
_def 'hw_serial_IIR_FE_mask	[(0x3 << 6)] -- FIFO enable information 

-- LSR : Line Status Register 
_def 'hw_serial_LSR_RDR		[(1 << 0)]   -- Rx Data Ready 
_def 'hw_serial_LSR_EROVR	[(1 << 1)]   -- Error Rx OVerRun 
_def 'hw_serial_LSR_ERPAR	[(1 << 2)]   -- Error Rx PARity 
_def 'hw_serial_LSR_ERFRM	[(1 << 3)]   -- Error Rx FRaMe 
_def 'hw_serial_LSR_ERBRK	[(1 << 4)]   -- Error Rx BReaK 
_def 'hw_serial_LSR_THRE	[(1 << 5)]   -- Tx Holding Register Empty 
_def 'hw_serial_LSR_TEMT	[(1 << 6)]   -- Tx EMpTy 
_def 'hw_serial_LSR_ERFIFO	[(1 << 7)]   -- Error Rx FIFO full 

-- FCR : FIFO Control Register 
_def 'hw_serial_FCR_FIFOEN	[(1 << 0)]   -- 1 = enable Rx and Tx FIFOs 
_def 'hw_serial_FCR_RFCLR	[(1 << 1)]   -- 1 = clear Rx FIFO 
_def 'hw_serial_FCR_TFCLR	[(1 << 2)]   -- 1 = clear Tx FIFO 
_def 'hw_serial_FCR_RMODE	[(1 << 3)]   -- changes RXRDY and TXRDY modes 
_def 'hw_serial_FCR_FTL_mask	[(0x3 << 6)] -- Rx FIFO trigger level 
_def 'hw_serial_FCR_FTL_1	[0x0]	     -- 1 byte 
_def 'hw_serial_FCR_FTL_4	[0x1]	     -- 4 bytes 
_def 'hw_serial_FCR_FTL_8	[0x2]	     -- 8 bytes 
_def 'hw_serial_FCR_FTL_14	[0x3]        -- 14 bytes 

-- MCR : Modem Control Register 
_def 'hw_serial_MCR_DTR		[(1 << 0)]   -- 1 = DTR forced low 
_def 'hw_serial_MCR_RTS		[(1 << 1)]   -- 1 = RTS forced low 
_def 'hw_serial_MCR_OUT2	[(1 << 3)]   -- 0 = OUT2 high; 1 = OUT2 low 
_def 'hw_serial_MCR_LBACK	[(1 << 4)]   --  LoopBACK enable (testing) 

-- MSR : Modem Status Register 
_def 'hw_serial_MSR_DCTS	[(1 << 0)]   -- Delta CTS 
_def 'hw_serial_MSR_DDSR	[(1 << 1)]   -- Delta DSR  
_def 'hw_serial_MSR_TERI	[(1 << 2)]   -- Trailing Edge Ring Indicator 
_def 'hw_serial_MSR_CTS		[(1 << 4)]   -- Clear To Send 
_def 'hw_serial_MSR_DSR		[(1 << 5)]   -- Data Set Ready 
_def 'hw_serial_MSR_RI		[(1 << 6)]   -- Ring Indicator 

-------------------------------------------------------------------------------
-- VL16C551 Parallel Port Registers 

struct hw_parallel [
	byte PDR 		-- RO : Parallel Data Read 
                byte pad0a      byte pad0b      byte pad0c
	byte PSR 		-- RO : Parallel Status Register 
                byte pad1a      byte pad1b      byte pad1c
	byte PCR 		-- RW : Parallel Control Register 
                byte pad2a      byte pad2b      byte pad2c
	byte GPIO 		-- RW : General Purpose I/O 
                byte pad3a      byte pad3b      byte pad3c
]
_def 'hw_parallel.PDW hw_parallel.PDR	-- WO : Parallel Data Write 

_def 'hw_PARALLEL [(IO_base_internal + 0x60)]

-----------------------------------------------------------------------------
-- GPIO : General Purpose I/O register 
_def 'hw_parallel_GPIO_IN1	[(1 << 0)]   -- RO : input bit 
_def 'hw_parallel_GPIO_IN2	[(1 << 1)]   -- RO : input bit 
_def 'hw_parallel_GPIO_IN3	[(1 << 2)]   -- RO : input bit 
_def 'hw_parallel_GPIO_IN4	[(1 << 3)]   -- RO : input bit 
_def 'hw_parallel_GPIO_LED1	[(1 << 4)]   -- WO : LED control 
_def 'hw_parallel_GPIO_LED2	[(1 << 5)]   -- WO : LED control 
_def 'hw_parallel_GPIO_LED3	[(1 << 6)]   -- WO : LED control 
_def 'hw_parallel_GPIO_LED4	[(1 << 7)]   -- WO : LED control 

-----------------------------------------------------------------------------

-- end of vy86pid.m
