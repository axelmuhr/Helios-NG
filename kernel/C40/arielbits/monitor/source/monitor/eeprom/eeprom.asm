******************************************************
* FUNCTION DEF : _ReadEEPROM
******************************************************
_ReadEEPROM:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
**************************************************************************
*       Routine that reads from the EEPROM                               *
**************************************************************************
		push	r1		;Save regesters
		push	r2
		push	r3
		push	r4
		push	ar0
		push	ar1
		push	ar2

		or	20h,iif		;Make IIOF1 an output
		or	40h,iif		;Output a 1 so that a transiton to
					;low, while SCL is high, will signal
					;a read cycle

		ldi	@CONST+1,ar1	;Load the control register's address
		ldi	*ar1,r2		;Read its current value
		ldi	@CONST,r3	;Used to toggle the EEPROM clock
		or	r3,r2
		sti	r2,*ar1		;Clock is now HIGH
		rpts	125		;5 micro sec start condition setup time
		nop

		xor	40h,iif		;Toggle data line to *LOW* to
					;start a read cycle
		rpts	125		;5 micro sec start condition hold time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW

;The following block writes the X242's device id into the chip.
;The sequence 1010 indicates an X242.
		ldi	4,ar2
ID_TOP1		xor	40h,iif		;Toggle data line
		rpts	125		;5 micro sec data in setup time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW
		dbz	ar2,ID_TOP1

;The following block writes the X242 device address(0 for Hydra) to the chip
		ldi	3,ar2
DEV_ADDR_TOP	xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW
		rpts	125		;5 micro sec clock low time
		nop
		dbz	ar2,DEV_ADDR_TOP

;The following block indicates a write cycle
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1         ;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW

;Wait for acknowledgement
		xor	20h,iif		;Make IIOF1 an input
		rpts	7		;250 nSec data setup delay
		nop
START_WAIT_ACK1	ldi	iif,r0
		and	40h,r0		;Check for acknowledgement
		bnz	START_WAIT_ACK1
                xor	r3,r2		;Toggle clock
                sti	r2,*ar1         ;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW


;Write the byte address into the chip
		;>>>> 		i = addr;
	LDI	*-FP(2),R0
		ldi	*-FP(2),r0	;Get the data address from the stack
		or	20h,iif		;Make IIF1 in output
		ldi	8,ar2
DATA_ADDR_TOP1	rpts	125		;5 micro sec clock low time		nop
		ldi	1,r1
		and	r0,r1		;Test for 1 or 0 in the LSB
		lsh	6,r1		;Align the current address bit
					;with the flag1 bit of IIOF
		or	r1,iif		;Put next address bit into IIOF
		rpts	7       	;280 nSec data setup delay
		nop
		lsh	-1,r0		;Put next address bit into LSB
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW
		rpts	125		;5 micro sec clock low time
		nop
		dbz	ar2,DATA_ADDR_TOP1

;Wait for acknowledgement
		xor	20h,iif		;Make IIOF1 an input
ADDR_WAIT_ACK	ldi	iif,r0
		and	40h,r0		;Check for acknowledgement
		bnz	ADDR_WAIT_ACK
                xor	r3,r2		;Toggle clock
                sti	r2,*ar1         ;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW


;Transmit another start bit
		or	20h,iif		;Make IIOF1 an output
		or	40h,iif		;Output a 1 so that a transiton to
					;low, while SCL is high, will signal
					;a read cycle

		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now HIGH
		rpts	125		;5 micro sec start condition setup time
		nop

		xor	40h,iif		;Toggle data line to *LOW* to
					;start a read cycle
		rpts	125		;5 micro sec start condition hold time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW

;Write the device id again
		ldi	4,ar2
ID_TOP2		xor	40h,iif		;Toggle data line
		rpts	125		;5 micro sec data in setup time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW
		dbz	ar2,ID_TOP2

;Write the device address again
		ldi	3,ar2
DEV_ADDR_TOP2	xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW
		rpts	125		;5 micro sec clock low time
		nop
		dbz	ar2,DEV_ADDR_TOP2

;The following block indicates a read cycle
		xor	20h,iif		;Make IIOF1 an input
					;(pull-up will make SDA high)
		rpts	7		;250 nSec data setup delay
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1         ;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW

;Wait for acknowledgement
START_WAIT_ACK2 ldi	iif,r0
		and	40h,r0		;Check for acknowledgement
		bnz	START_WAIT_ACK2
                xor	r3,r2		;Toggle clock
		sti	r2,*ar1         ;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW


;Read data from EEPROM
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now HIGH
		rpts    125		;5 micro sec clock high time
		nop
		ldi	0,r0
		ldi	-6,r4
		ldi	8,ar2
DATA_READ	xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW
		rpts	125		;5 micro sec clock low time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now HIGH
		lsh3	r4,iif,r1	;Get next data bit
		ror	r1		;Put data bit in carry flag
		rolc	r0		;Rotate in current data bit from carry
		dbz	ar2,DATA_READ

		and	0FFh,r0		;Strip off all but last eight bits

;Send a stop bit
		sti	r2,*ar1         ;Clock is now LOW
		rpts	125		;5 micro sec clock high time
		nop
		or      40h,iif		;Set IIOF1 to 1
		xor	40h,iif		;Set IIOF1 to 0
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now HIGH
		xor	40h,iif		;Toggle IIOF1 to signal stop

;Restore regester set and return
		pop	ar2
		pop	ar1
		pop	ar0
		pop	r4
		pop	r3
		pop	r2
		pop	r1

	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS

	.globl	_write_eeprom
******************************************************
* FUNCTION DEF : _WriteEEPROM
******************************************************
_WriteEEPROM:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
**************************************************************************
*       Routine that writes to the EEPROM                                *
**************************************************************************
		push	r1		;Save regesters
		push	r2
		push	r3
		push	r4
		push	r5
		push	ar0
		push	ar1
		push	ar2

		or	20h,iif		;Make IIOF1 an output
		or	40h,iif		;Output a 1 so that a transiton to
					;low, while SCL is high, will signal
					;a read cycle

;>>>> 		i = cont_addr;
		ldi	@CONST+1,ar1
		ldi	*ar1,r2		;Read its current value
		ldi	@CONST,r3	;Used to toggle the EEPROM clock
		or	r3,r2
		sti	r2,*ar1		;Clock is now HIGH
		rpts	125		;5 micro sec start condition setup time
		nop

		xor	40h,iif		;Toggle data line to *LOW* to
					;start a read cycle
		rpts	125		;5 micro sec start condition hold time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW

;The following block writes the X242's device id into the chip.
;The sequence 1010 indicates an X242.
		ldi	4,ar2
W_ID_TOP1		xor	40h,iif		;Toggle data line
		rpts	125		;5 micro sec data in setup time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW
		dbz	ar2,W_ID_TOP1

;The following block writes the X242 device address(0 for Hydra) to the chip
		ldi	3,ar2
W_DEV_ADDR_TOP	xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW
		rpts	125		;5 micro sec clock low time
		nop
		dbz	ar2,W_DEV_ADDR_TOP

;The following block indicates a write cycle
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1         ;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW

;Wait for acknowledgement
		xor	20h,iif		;Make IIOF1 an input
		rpts	7		;250 nSec data setup delay
		nop
W_ST_WAIT_ACK1	ldi	iif,r0
		and	40h,r0		;Check for acknowledgement
		bnz	W_ST_WAIT_ACK1
                xor	r3,r2		;Toggle clock
                sti	r2,*ar1         ;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW


;Write the byte address into the chip
;>>>> 		i = addr;
	LDI	*-FP(3),R0
		or	20h,iif		;Make IIF1 in output
		ldi	8,ar2
W_D_ADDR_TOP1	rpts	125		;5 micro sec clock low time		nop
		ldi	1,r1
		and	r0,r1		;Test for 1 or 0 in the LSB
		lsh	6,r1		;Align the current address bit
					;with the flag1 bit of IIOF
		or	r1,iif		;Put next address bit into IIOF
		rpts	7       	;280 nSec data setup delay
		nop
		lsh	-1,r0		;Put next address bit into LSB
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW
		rpts	125		;5 micro sec clock low time
		nop
		dbz	ar2,W_D_ADDR_TOP1

;Wait for acknowledgement
		xor	20h,iif		;Make IIOF1 an input
W_ADDR_WAIT_ACK	ldi	iif,r0
		and	40h,r0		;Check for acknowledgement
		bnz	W_ADDR_WAIT_ACK
                xor	r3,r2		;Toggle clock
                sti	r2,*ar1         ;Clock is now HIGH
		rpts	125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW

;Write data to EEPROM
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now HIGH
		rpts    125		;5 micro sec clock high time
		nop
;>>>> 		i = data;
	LDI	*-FP(2),r0
		ldi	-6,r4		;Used as shift count to extract IIOF1
		ldi	8,ar2		;Loop counter to read 8 bits
W_DATA_WRITE	rpts    125		;5 micro sec clock high time
		nop
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now LOW
		rpts    125		;5 micro sec clock low time
		nop
		rorc	r0		;Rotate out current data bit to carry
		rolc	r1		;Rotate in data bit from carry flag
		and	1,r1		;Strip off all but the LSB
		lsh	r4,r1		;Align data bit with IIOF1
		or	r1,iif		;Write data bit
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now HIGH
		dbz	ar2,W_DATA_WRITE

		ldi	r5,r0		;Return the same data

;Send a stop bit
		sti	r2,*ar1         ;Clock is now LOW
		rpts	125		;5 micro sec clock high time
		nop
		or      40h,iif		;Set IIOF1 to 1
		xor	40h,iif		;Set IIOF1 to 0
		xor	r3,r2		;Toggle clock
		sti	r2,*ar1		;Clock is now HIGH
		xor	40h,iif		;Toggle IIOF1 to signal stop

;>>>> 		i = data;
	LDI	*-FP(2),r0		;Return data value
;Restore regester set and return
		pop	ar2
		pop	ar1
		pop	ar0
		pop	r5
		pop	r4
		pop	r3
		pop	r2
		pop	r1

EPI0_2:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,2
	.sect	".cinit"
	.word	2,CONST
	.word	800000h
	.word	0bffd8000h
	.end
