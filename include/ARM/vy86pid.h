/*
 * File:	vy86pid.h
 * Subsystem:	Helios-ARM Executive
 * Author:	JGSmith
 * Date:	930310
 *
 * Description: Hardware description of the VY86PID board.
 *		Memory maps.
 *		INTC interrupt controller manifests.
 *		VL16C551 Asynchronous Communications device manifests.
 *
 * Copyright (c) 1993, VLSI Technology Inc.
 * All Rights Reserved.
 */
/*---------------------------------------------------------------------------*/

#ifndef	__vy86pid_h
#define	__vy86pid_h

/*---------------------------------------------------------------------------*/
/* We do not use bit-fields to describe registers in the following
 * file since some of them are Write-Only (and the compiler would always
 * generate a load before manipulating an individual bit-field).
 */

/* The following "type" is used for byte wide I/O registers */
typedef volatile unsigned char vubyte ;

/*---------------------------------------------------------------------------*/
/* The VY86PID memory map is structured as follows. Only the lo-64MBytes of
 * the address space is allocated. The ROM bank is mapped at 0x00000000
 * (over the main DRAM bank) by RESET. The DRAM ghosts within its bank.
 */

#define DRAM_base	(0x00000000)		  /* normal DRAM location */
#define DRAM_size	(16 << 20)		  /* maximum DRAM size */

#define DRAM2_base	(DRAM_base + DRAM_size)	  /* unused DRAM bank */
#define DRAM2_size	(16 << 20)

#define IO_base		(DRAM2_base + DRAM2_size) /* physical base of I/O */
#define IO_size		(16 << 20)

#define ROM_base	(IO_base + IO_size) 	  /* ROM/EPROM etc. */
#define ROM_size	(16 << 20)

/*---------------------------------------------------------------------------*/
/*-- I/O memory map ---------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#define IO_base_internal	(IO_base + 0x00000000)
#define IO_base_expslot_DMA	(IO_base + 0x00400000)
#define IO_base_expslot_IO	(IO_base + 0x00800000)
#define IO_base_expslot_MEM	(IO_base + 0x00C00000)

/*---------------------------------------------------------------------------*/
/* INTC PGA Registers */

/* The following structure is based at hw_INTC */
typedef struct hw_intc
{
       vubyte IRQS ; 	 /* RO : IRQ Status register */
 const vubyte _pad1[3] ;
       vubyte IRQM ;	 /* WO : IRQ Mask register */
 const vubyte _pad2[3] ;
 const vubyte FIQS ; 	 /* RO : FIQ Status register */
 const vubyte _pad3[3] ;
       vubyte FIQM ; 	 /* WO : FIQ Mask register */
 const vubyte _pad4[3] ;
} hw_intc ;
#define IRQRST IRQS	 /* WO : IRQ ReSeT register */

/* Provide direct access to the INTC I/O device */
#define hw_INTC ((hw_intc *)(IO_base_internal + 0x00))

/* IRQS : IRQ status register */
/* This register contains the raw interrupt input (without masking).
 * This register is ANDed with the IRQM (mask). An IRQ is generated if
 * the result is non-zero.
 */
#define hw_intc_irq_serial1	(1 << 0) /* Serial port 1 */
#define hw_intc_irq_timer	(1 << 1) /* Timer */
#define hw_intc_irq_parallel	(1 << 2) /* Parallel port */
#define hw_intc_irq_expslot3	(1 << 3) /* expansion slot irq 3 */
#define hw_intc_irq_expslot4	(1 << 4) /* expansion slot irq 4 */
#define hw_intc_irq_expslot5	(1 << 5) /* expansion slot irq 5 */
#define hw_intc_irq_expslot6	(1 << 6) /* expansion slot irq 6 */
#define hw_intc_irq_panic	(1 << 7) /* PANIC button */

/* IRQRST : IRQ reset register */
/* Writing to the following bits will reset the corresponding
 * interrupt. The other interrupt sources are cleared by the relevant I/O
 * device.
 */
#define hw_intc_resetirq_timer	(hw_intc_irq_timer)
#define hw_intc_resetirq_panic	(hw_intc_irq_panic)

/* IRQM : IRQ mask register */
/* An interrupt source is enabled by writing a 1 into the
 * corresponding bit. This register is reset by power-on.
 * Bit 7 (corresponding to the PANIC IRQ is always enabled).
 */
#define hw_intc_enableirq_serial1	(hw_intc_irq_serial1)
#define hw_intc_enableirq_timer		(hw_intc_irq_timer)
#define hw_intc_enableirq_parallel	(hw_intc_irq_parallel)
#define hw_intc_enableirq_expslot3	(hw_intc_irq_expslot3)
#define hw_intc_enableirq_expslot4	(hw_intc_irq_expslot4)
#define hw_intc_enableirq_expslot5	(hw_intc_irq_expslot5)
#define hw_intc_enableirq_expslot6	(hw_intc_irq_expslot6)

/* FIQS : FIQ status register */
/* This register contains the raw fast-interrupt input (without masking).
 * This register is ANDed with the FIQM (mask). A FIQ is generated if
 * the result is non-zero.
 */
#define hw_intc_fiq_serial1_tx	(1 << 0) /* Serial port TX DMA */
#define hw_intc_fiq_serial1_rx	(1 << 1) /* Serial port RX DMA */
#define hw_intc_fiq_expslot	(1 << 2) /* Expansion slot */
#define hw_intc_fiq_nmi		(1 << 3) /* LSA port */
#define hw_intc_fiq_expslot3	(1 << 4) /* Expansion slot DMA */
#define hw_intc_fiq_expslot4	(1 << 5) /* Expansion slot DMA */
#define hw_intc_fiq_expslot5	(1 << 6) /* Expansion slot DMA */
#define hw_intc_fiq_expslot6	(1 << 7) /* Expansion slot DMA */

/* FIQM : FIQ mask register */
/* An interrupt source is enabled by writing a 1 into the
 * corresponding bit. This register is reset by power-on.
 */
#define hw_intc_enablefiq_serial1_tx	(hw_intc_enablefiq_serial1_tx)
#define hw_intc_enablefiq_serial1_rx	(hw_intc_enablefiq_serial1_rx)
#define hw_intc_enablefiq_expslot	(hw_intc_enablefiq_expslot)
#define hw_intc_enablefiq_nmi		(hw_intc_enablefiq_nmi)
#define hw_intc_enablefiq_expslot3	(hw_intc_enablefiq_expslot3)
#define hw_intc_enablefiq_expslot4	(hw_intc_enablefiq_expslot4)
#define hw_intc_enablefiq_expslot5	(hw_intc_enablefiq_expslot5)
#define hw_intc_enablefiq_expslot6	(hw_intc_enablefiq_expslot6)

/*---------------------------------------------------------------------------*/
/* VL16C551 Serial Port Registers */

/* The following registers are offset from "hw_SERIAL" */
typedef struct hw_serial
{
 vubyte RBR ;	/* RO : Receive Buffer Register */
 vubyte _pad1[3] ;
 vubyte IER ;	/* RW : Interrupt Enable Register */
 vubyte _pad2[3] ;
 vubyte IIR ;	/* RO : Interrupt Identification Register */
 vubyte _pad3[3] ;
 vubyte LCR ;	/* RW : Line Control Register */
 vubyte _pad4[3] ;
 vubyte MCR ;	/* RW : Modem Control Register */
 vubyte _pad5[3] ;
 vubyte LSR ;	/* RW : Line Status Register */
 vubyte _pad6[3] ;
 vubyte MSR ;	/* RW : Modem Status Register */
 vubyte _pad7[3] ;
 vubyte SCR ;	/* RW : Scratch Register */
 vubyte _pad8[3] ;
} hw_serial ;
#define THR RBR	/* WO : Transmitter Holding Register */
#define FCR IIR	/* WO : FIFO Control Register */
#define DLL RBR	/* RW : Divisor Latch LSB */
#define DLM IER	/* RW : Divisor Latch MSB */

#define hw_SERIAL ((hw_serial *)(IO_base_internal + 0x20))

/* LCR : Line Control Register */
#define hw_serial_LCR_WLS_mask	(0x03)	/* mask for word-length field */
#define hw_serial_LCR_WL_5bits	(0x0)
#define hw_serial_LCR_WL_6bits	(0x1)
#define hw_serial_LCR_WL_7bits	(0x2)
#define hw_serial_LCR_WL_8bits	(0x3)
#define hw_serial_LCR_SBSEL	(1 << 2) /* 0 = 1 stop bit; 1 = 2 stop bits */
#define hw_serial_LCR_PAREN	(1 << 3) /* 0 = no parity; 1 = parity */
#define hw_serial_LCR_EVPAR	(1 << 4) /* 0 = odd parity; 1 = even parity */
#define hw_serial_LCR_STICK	(1 << 5) /* 0 = disabled; 1 = enabled */
#define hw_serial_LCR_BREAK	(1 << 6) /* 0 = disable break; 1 = enable break */
#define hw_serial_LCR_DLAB	(1 << 7) /* 0 = data; 1 = divisor latches */

/* The following is a reasonable default setup for LCR:
 *	8bits; 1 stop bit; no parity; divisor latch access off
 */
#define hw_serial_LCR_default	(hw_serial_LCR_WL_8bits)

/* DLL and DLM : Divisor latch registers */
/* This is viewed as a 16bit register. The registers are only
 * accessible when the "hw_serial_LCR_DLAB" bit is set in LCR.
 * The VY86PID uses a 1.8432MHz input clock.
 */
#define hw_serial_DLR_300	(384)
#define hw_serial_DLR_1200	(96)
#define hw_serial_DLR_2400	(48)
#define hw_serial_DLR_4800	(24)
#define hw_serial_DLR_9600	(12)
#define hw_serial_DLR_19200	(6)
#define hw_serial_DLR_38400	(3)
#define hw_serial_DLR_56000	(2)	/* error = 2.85% */

/* IER : Interrupt Enable Register */
#define hw_serial_IER_ERBFI	(1 << 0) /* 1 = enable Rx data available interrupt */
#define hw_serial_IER_ETBEI	(1 << 1) /* 1 = enable Tx holding reg. empty interrupt */
#define hw_serial_IER_ELSI	(1 << 2) /* 1 = enable Rx line status interrupt */
#define hw_serial_IER_EDSSI	(1 << 3) /* 1 = enable modem status interrupt */

/* IIR : Interrupt Identification Register */
#define hw_serial_IIR_IPENDN	(1 << 0)   /* 0 = interrupt pending; 1 = none */
#define hw_serial_IIR_IID_mask	(0x3 << 1) /* pending interrupts */
#define hw_serial_IIR_IID_MS	(0x0 << 1) /* Modem Status */
#define hw_serial_IIR_IID_THRE	(0x1 << 1) /* Tx Holding Reg. Empty */
#define hw_serial_IIR_IID_RDA	(0x2 << 1) /* Rx Data Available */
#define hw_serial_IIR_IID_RLS	(0x3 << 1) /* Rx Line Status */
/* NOTE: The following is the status when the "hw_serial_IIR_FIFEN" bit is
 * also set.
 */
#define hw_serial_IIR_IID_FS	(0x2 << 1) /* FIFO Status; FIFO thresh; wait timeout */
#define hw_serial_IIR_FIFEN	(1 << 3)   /* 0 = 16C450 mode; 1 = 16C550 mode */
#define hw_serial_IIR_FE_mask	(0x3 << 6) /* FIFO enable information */

/* LSR : Line Status Register */
#define hw_serial_LSR_RDR	(1 << 0) /* Rx Data Ready */
#define hw_serial_LSR_EROVR	(1 << 1) /* Error Rx OVerRun */
#define hw_serial_LSR_ERPAR	(1 << 2) /* Error Rx PARity */
#define hw_serial_LSR_ERFRM	(1 << 3) /* Error Rx FRaMe */
#define hw_serial_LSR_ERBRK	(1 << 4) /* Error Rx BReaK */
#define hw_serial_LSR_THRE	(1 << 5) /* Tx Holding Register Empty */
#define hw_serial_LSR_TEMT	(1 << 6) /* Tx EMpTy */
#define hw_serial_LSR_ERFIFO	(1 << 7) /* Error Rx FIFO full */

/* FCR : FIFO Control Register */
#define hw_serial_FCR_FIFOEN	(1 << 0)   /* 1 = enable Rx and Tx FIFOs */
#define hw_serial_FCR_RFCLR	(1 << 1)   /* 1 = clear Rx FIFO */
#define hw_serial_FCR_TFCLR	(1 << 2)   /* 1 = clear Tx FIFO */
#define hw_serial_FCR_RMODE	(1 << 3)   /* changes RXRDY and TXRDY modes */
#define hw_serial_FCR_FTL_mask	(0x3 << 6) /* Rx FIFO trigger level */
#define hw_serial_FCR_FTL_1	(0x0)	   /* 1 byte */
#define hw_serial_FCR_FTL_4	(0x1)	   /* 4 bytes */
#define hw_serial_FCR_FTL_8	(0x2)	   /* 8 bytes */
#define hw_serial_FCR_FTL_14	(0x3)      /* 14 bytes */

/* MCR : Modem Control Register */
#define hw_serial_MCR_DTR	(1 << 0) /* 1 = DTR forced low */
#define hw_serial_MCR_RTS	(1 << 1) /* 1 = RTS forced low */
#define hw_serial_MCR_OUT2	(1 << 3) /* 0 = OUT2 high; 1 = OUT2 low */
#define hw_serial_MCR_LBACK	(1 << 4) /*  LoopBACK enable (testing) */

/* MSR : Modem Status Register */
#define hw_serial_MSR_DCTS	(1 << 0) /* Delta CTS */
#define hw_serial_MSR_DDSR	(1 << 1) /* Delta DSR */ 
#define hw_serial_MSR_TERI	(1 << 2) /* Trailing Edge Ring Indicator */
#define hw_serial_MSR_CTS	(1 << 4) /* Clear To Send */
#define hw_serial_MSR_DSR	(1 << 5) /* Data Set Ready */
#define hw_serial_MSR_RI	(1 << 6) /* Ring Indicator */

/*---------------------------------------------------------------------------*/
/* VL16C551 Parallel Port Registers */

typedef struct hw_parallel
{
 vubyte PDR ;		/* RO : Parallel Data Read */
 vubyte _pad1[3] ;
 vubyte PSR ;		/* RO : Parallel Status Register */
 vubyte _pad2[3] ;
 vubyte PCR ;		/* RW : Parallel Control Register */
 vubyte _pad3[3] ;
 vubyte GPIO ;		/* RW : General Purpose I/O */
 vubyte _pad4[3] ;
} hw_parallel ;
#define PDW PDR 	/* WO : Parallel Data Write */

#define hw_PARALLEL ((hw_parallel *)(IO_base_internal + 0x60))

/* GPIO : General Purpose I/O register */
#define hw_parallel_GPIO_IN1	(1 << 0) /* RO : input bit */
#define hw_parallel_GPIO_IN2	(1 << 1) /* RO : input bit */
#define hw_parallel_GPIO_IN3	(1 << 2) /* RO : input bit */
#define hw_parallel_GPIO_IN4	(1 << 3) /* RO : input bit */
#define hw_parallel_GPIO_LED1	(1 << 4) /* WO : LED control */
#define hw_parallel_GPIO_LED2	(1 << 5) /* WO : LED control */
#define hw_parallel_GPIO_LED3	(1 << 6) /* WO : LED control */
#define hw_parallel_GPIO_LED4	(1 << 7) /* WO : LED control */

/* A simple macro to place a value into the LEDs */
#define hw_pid_setLED(n) (hw_PARALLEL->GPIO = (((char)(~n)) << 4))
void hw_setLED(int leds);

/*---------------------------------------------------------------------------*/

#endif /* __vy86pid_h */

/*---------------------------------------------------------------------------*/
/*> EOF vy86pid.h <*/
