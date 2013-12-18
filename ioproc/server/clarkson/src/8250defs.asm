;8250 definitions
;Control/status register offsets from base address
THR	equ	0		;Transmitter holding register
RBR	equ	0		;Receiver buffer register
DLL	equ	0		;Divisor latch LSB
DLM	equ	1		;Divisor latch MSB
IER	equ	1		;Interrupt enable register
IIR	equ	2		;Interrupt ident register
FCR	equ	2		;16550 FIFO control register
LCR	equ	3		;Line control register
MCR	equ	4		;Modem control register
LSR	equ	5		;Line status register
MSR	equ	6		;Modem status register

;8250 Line Control Register
LCR_5BITS	equ	0	;5 bit words
LCR_6BITS	equ	1	;6 bit words
LCR_7BITS	equ	2	;7 bit words
LCR_8BITS	equ	3	;8 bit words
LCR_NSB		equ	4	;Number of stop bits
LCR_PEN		equ	8	;Parity enable
LCR_EPS		equ	10h	;Even parity select
LCR_SP		equ	20h	;Stick parity
LCR_SB		equ	40h	;Set break
LCR_DLAB	equ	80h	;Divisor Latch Access Bit

;8250 Line Status Register
LSR_DR	equ	1	;Data ready
LSR_OE	equ	2	;Overrun error
LSR_PE	equ	4	;Parity error
LSR_FE	equ	8	;Framing error
LSR_BI	equ	10h	;Break interrupt
LSR_THRE equ	20h	;Transmitter line holding register empty
LSR_TSRE equ	40h	;Transmitter shift register empty

;8250 Interrupt Identification Register
IIR_IP		equ	1	;0 if interrupt pending
IIR_ID		equ	6	;Mask for interrupt ID
IIR_RLS		equ	6	;Receiver Line Status interrupt
IIR_RDA		equ	4	;Receiver data available interrupt
IIR_THRE	equ	2	;Transmitter holding register empty int
IIR_MSTAT	equ	0	;Modem status interrupt
IIR_FIFO_TIMEOUT  equ   008h    ;FIFO timeout interrupt pending - 16550 only
IIR_FIFO_ENABLED  equ   080h    ;FIFO enabled (FCR0 = 1) - 16550 only

;8250 interrupt enable register bits
IER_DAV	equ	1	;Data available interrupt
IER_TxE	equ	2	;Tx buffer empty interrupt
IER_RLS	equ	4	;Receive line status interrupt
IER_MS	equ	8	;Modem status interrupt

;8250 Modem control register
MCR_DTR	equ	1	;Data Terminal Ready
MCR_RTS	equ	2	;Request to Send
MCR_OUT1 equ	4	;Out 1 (not used)
MCR_OUT2 equ	8	;Master interrupt enable (actually OUT 2)
MCR_LOOP equ	10h	;Loopback test mode

;8250 Modem Status Register
MSR_DCTS equ	1	;Delta Clear-to-Send
MSR_DDSR equ	2	;Delta Data Set Ready
MSR_TERI equ	4	;Trailing edge ring indicator
MSR_DRLSD equ	8	;Delta Rx Line Signal Detect
MSR_CTS equ	10h	;Clear to send
MSR_DSR equ	20h	;Data set ready
MSR_RI	equ	40h	;Ring indicator
MSR_RLSD equ	80h	;Received line signal detect

pr_ch_al	macro		alvalue
ifdef TRACEON
	mov	al,alvalue
	call	pr_ch
endif
	endm
