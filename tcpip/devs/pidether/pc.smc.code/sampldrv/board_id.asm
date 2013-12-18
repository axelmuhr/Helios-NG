;******************************************************************************
;******************************************************************************
;	A George Kalwitz Production, 1990
;******************************************************************************
;******************************************************************************
;
;******************************************************************************
; BOARD ID USAGE
;
;	GetBoardID
;
;	Near CALL
;
;		ON ENTRY:	DX = Base I/O Address of Board in Question
;				AX = 1 if the machine is Micro Channel
;				     0 if the machine is AT
;
;		On EXIT:	AX = Board ID Feature Bits
;				DX = Extra Info Bits
;
;				All Registers are preserved except:
;				BX and CX
;
;******************************************************************************
;
;******************************************************************************
; BOARD ID DEFINITIONS
;
; Two 16 bit values will be returned 
;
;	The first word (in AX) contains Feature Bits which make up a boards
;	unique ID.
;
;		e.g. STARLAN MEDIA, INTERFACE_CHIP, MICROCHANNEL
;
;	The next word (in DX) contains Extra Bits which do not change the
;	boards ID.
;
;		e.g. RAM SIZE, 16 BIT SLOT, ALTERNATE IRQ
;
;******************************************************************************
;
;******************************************************************************
;	General register definitions for identifying board types
;******************************************************************************
BID_REG_0	equ	00h	;registers in LAN adapter
BID_REG_1	equ	01h
BID_REG_2	equ	02h
BID_REG_3	equ	03h
BID_REG_4	equ	04h
BID_REG_5	equ	05h
BID_REG_6	equ	06h
BID_REG_7	equ	07h
BID_LAR_0	equ	08h	;LAN address ROM registers
BID_LAR_1	equ	09h
BID_LAR_2	equ	0Ah
BID_LAR_3	equ	0Bh
BID_LAR_4	equ	0Ch
BID_LAR_5	equ	0Dh

BID_BOARD_ID_BYTE	equ	0Eh
BID_CHCKSM_BYTE		equ	0Fh

BID_LAR_OFFSET	equ	08h	;offset for aliasing check

;******************************************************************************
;	General definitions
;******************************************************************************
BID_MSZ_583_BIT		equ	08h
BID_SIXTEEN_BIT_BIT	equ	01h

;******************************************************************************
;	Mask for extracting the board revision number
;******************************************************************************
BID_BOARD_REV_MASK	equ	1Eh

;******************************************************************************
;	Definitions for board rev numbers greater that 1
;******************************************************************************
BID_MEDIA_TYPE_BIT	equ	01h
BID_SOFT_CONFIG_BIT	equ	20h
BID_RAM_SIZE_BIT	equ	40h
BID_BUS_TYPE_BIT	equ	80h

;******************************************************************************
;	Defs for identifying the 690
;******************************************************************************
BID_CR		equ	10h		; Command Register
BID_TXP		equ	04h		; Transmit Packet Command
BID_TCR_DIFF	equ	0Dh		; Transmit Configuration Register
BID_TCR_VAL	equ	18h		; Value to Test 8390 or 690
BID_PS0		equ	00h		; Register Page Select 0
BID_PS1		equ	40h		; Register Page Select 1
BID_PS2		equ	80h		; Register Page Select 2
BID_PS_MASK	equ	3Fh		; For Masking Off Page Select Bits

;******************************************************************************
;	Defs for manipulating the 584
;******************************************************************************
BID_EEPROM_0			equ	08h
BID_EEPROM_1			equ	09h
BID_EEPROM_2			equ	0Ah
BID_EEPROM_3			equ	0Bh
BID_EEPROM_4			equ	0Ch
BID_EEPROM_5			equ	0Dh
BID_EEPROM_6			equ	0Eh
BID_EEPROM_7			equ	0Fh

BID_OTHER_BIT			equ	02h
BID_ICR_MASK			equ	0Ch
BID_EAR_MASK			equ	0Fh
BID_ENGR_PAGE			equ	0A0h
BID_RLA				equ	10h
BID_EA6				equ	80h
BID_RECALL_DONE_MASK		equ	010h
BID_BID_EEPROM_OVERRIDE		equ	0FFB0h
BID_EXTRA_EEPROM_OVERRIDE	equ	0FFD0h
BID_EEPROM_MEDIA_MASK		equ	07h
BID_STARLAN_TYPE		equ	00h
BID_ETHERNET_TYPE		equ	01h
BID_TP_TYPE			equ	02h
BID_EW_TYPE			equ	03h
BID_EEPROM_IRQ_MASK		equ	18h
BID_PRIMARY_IRQ			equ	00h
BID_ALTERNATE_IRQ_1		equ	08h
BID_ALTERNATE_IRQ_2		equ	10h
BID_ALTERNATE_IRQ_3		equ	18h
BID_EEPROM_RAM_SIZE_MASK	equ	0E0h
BID_EEPROM_RAM_SIZE_RES1	equ	00h
BID_EEPROM_RAM_SIZE_RES2	equ	20h
BID_EEPROM_RAM_SIZE_8K		equ	40h
BID_EEPROM_RAM_SIZE_16K		equ	60h
BID_EEPROM_RAM_SIZE_32K		equ	80h
BID_EEPROM_RAM_SIZE_64K		equ	0A0h
BID_EEPROM_RAM_SIZE_RES3	equ	0C0h
BID_EEPROM_RAM_SIZE_RES4	equ	0E0h
BID_EEPROM_BUS_TYPE_MASK	equ	07h
BID_EEPROM_BUS_TYPE_AT		equ	00h
BID_EEPROM_BUS_TYPE_MCA		equ	01h
BID_EEPROM_BUS_TYPE_EISA	equ	02h
BID_EEPROM_BUS_SIZE_MASK	equ	18h
BID_EEPROM_BUS_SIZE_8BIT	equ	00h
BID_EEPROM_BUS_SIZE_16BIT	equ	08h
BID_EEPROM_BUS_SIZE_32BIT	equ	10h
BID_EEPROM_BUS_SIZE_64BIT	equ	18h

;******************************************************************************
;	Defs for local variables
;******************************************************************************
BID_LOCAL_BID		equ	2
BID_LOCAL_EXTRA		equ	4
BID_LOCAL_CR		equ	2
BID_LOCAL_TCR		equ	4

IFDEF	CODE386
	BASE_PTR	equ	ebp
	STACK_PTR	equ	esp
ELSE
	BASE_PTR	equ	bp
	STACK_PTR	equ	sp
ENDIF

;******************************************************************************
;	GetBoardID
;
;	Purpose: To identify which WD80XX board is being used 
;
;	Entry:	DX = Base I/O address of the board
;		AX = 1 if the machine is Micro Channel
;		     0 if the machine is AT
;
;	Exit:	AX = Board ID Feature Bits
;		DX = Extra Bits
;
;		All registers are preserved except BX and CX
;******************************************************************************
IFDEF	LARGE
GetBoardID	proc	far
ELSE
GetBoardID	proc	near
ENDIF

	push	BASE_PTR
	mov	BASE_PTR,STACK_PTR		;space for local variables
	sub	STACK_PTR,4

	mov	bx,dx				;save a copy of base i/o address
	mov	dx,ax				;save MC flag
	xor	ax,ax				;init return values
	mov	WORD PTR [BASE_PTR-BID_LOCAL_BID],ax
	mov	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],ax
	push	dx				;MC flag
	call	bid_get_board_rev_number
	pop	dx
	or	ax,ax				;is the rev number zero
	jnz	good_rev_num
	xor	dx,dx				;show nothing found
	xor	ax,ax
	jmp	bid_exit
good_rev_num:
	mov	cx,ax				;save rev number
	or	dx,dx				;MC flag
	jz	bid_not_micro_channel
	or	WORD PTR [BASE_PTR-BID_LOCAL_BID],MICROCHANNEL ;add feature bit
bid_not_micro_channel:
	mov	ax,[BASE_PTR-BID_LOCAL_BID]	;needed for 'bid_get_base_info'
	call	bid_get_base_info		;AX=current ID, BX=base I/O
	or	WORD PTR [BASE_PTR-BID_LOCAL_BID],ax	;or in new feature bits
	or	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],dx	;or in new extra bits
	call	bid_get_media_type		;get the media type
	or	WORD PTR [BASE_PTR-BID_LOCAL_BID],ax	;or in feature bits
	mov	ax,[BASE_PTR-BID_LOCAL_BID]
	cmp	cx,2				;is rev num >= 2?
	jl	bid_got_all_info		;no so skip extra info
	call	bid_get_id_byte_info	;AX=curr ID, CX=rev num, BX=base I/O
	or	WORD PTR [BASE_PTR-BID_LOCAL_BID],ax	;or in new feature bits
	or	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],dx	;or in new extra bits
bid_no_id_byte_info:
	cmp	cx,3				;is rev num >= 3?
	jl	bid_got_all_info
	test	WORD PTR [BASE_PTR-BID_LOCAL_BID],MICROCHANNEL
	jz	bid_has_eeprom_info
	or	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],INTERFACE_594_CHIP
	jmp	short bid_got_all_info
bid_has_eeprom_info:
	and	WORD PTR [BASE_PTR-BID_LOCAL_BID],BID_BID_EEPROM_OVERRIDE
	and	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],BID_EXTRA_EEPROM_OVERRIDE
	or	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],INTERFACE_584_CHIP
	mov	ax,[BASE_PTR-BID_LOCAL_BID]
	call	bid_get_eeprom_info
	or	WORD PTR [BASE_PTR-BID_LOCAL_BID],ax	;or in new feature bits
	or	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],dx	;or in new extra bits
	jmp	bid_do_690_check
bid_got_all_info:
	mov	ax,[BASE_PTR-BID_LOCAL_BID]	;needed for 'bid_get_ram_size'
	mov	dx,[BASE_PTR-BID_LOCAL_EXTRA]
	call	bid_get_ram_size ;AX=curr ID, DX=extra, CX=rev num, BX=base I/O
	or	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],ax ;or in new extra bits
bid_do_690_check:
	call	bid_check_for_690		;BX=base I/O
	jz	bid_no_690			;no 690 present
	or	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],NIC_690_BIT
bid_no_690:
	mov	ax,[BASE_PTR-BID_LOCAL_BID]		;get feature bits
	mov	dx,[BASE_PTR-BID_LOCAL_EXTRA]		;get extra bits
	or	ax,ax				;clear carry flag
bid_exit:
	mov	STACK_PTR,BASE_PTR
	pop	BASE_PTR
	ret

GetBoardID	endp

;******************************************************************************
;	bid_get_board_rev_number
;
;	Purpose: To find the board revision number from the hardware ID byte
;
;	Entry:	BX = Base I/O address of the board
;
;	Exit:	AX = Board Revision Number
;******************************************************************************
bid_get_board_rev_number	proc	near

	push	dx
	mov	dx,bx
	add	dx,BID_BOARD_ID_BYTE
	in	al,dx
	and	al,BID_BOARD_REV_MASK
	shr	al,1				;right justify result
	xor	ah,ah				;clear hob and ccf
	pop	dx
	ret

bid_get_board_rev_number	endp

;******************************************************************************
;	bid_get_base_info
;
;	Purpose: To identify which WD80XX board is being used 
;
;	Entry:	AX = Current Board ID
;		BX = Base I/O address of the board
;
;	Exit:	AX = Board ID Feature Bits
;		DX = Extra Bits
;******************************************************************************
bid_get_base_info	proc	near

	push	cx
	xor	cx,cx			;zero out Board ID
	xor	dx,dx			;zero out Extra Bits

	test	ax,MICROCHANNEL		;is this a micro channel?
	jz	check_aliasing		;no so continue
	call	bid_interface_chip	;check for interface chip
	jz	bid_exit_base_info
	or	cx,INTERFACE_CHIP	;add feature to type
	jmp	bid_exit_base_info
check_aliasing:
	call	bid_check_aliasing	;check for register aliasing
	jnz	bid_exit_base_info	;aliasing is true, so exit
	call	bid_interface_chip	;does it have an interface chip???
	jz	not_interface_chip	; no
	or	cx,INTERFACE_CHIP	; yes, so add in feature
	jmp	bid_exit_base_info
not_interface_chip:
	call	bid_board_16bit		;is the board an 8013ebt???
	jz	bid_exit_base_info	; no, exit
	or	cx,BOARD_16BIT		; yes, so add in feature
	call	bid_slot_16bit		;is 16 bit board in 16 bit slot???
	jz	bid_exit_base_info	; no, so we are done
	or	dx,SLOT_16BIT		; yes, so add in extra bit
	jmp	bid_exit_base_info
bid_exit_base_info: 
	mov	ax,cx
	pop	cx
	ret

bid_get_base_info	endp

;******************************************************************************
;	bid_check_aliasing
;
;	Purpose:  checks for register aliasing
;
;	Entry:  BX = Base I/O Address
;
;	Exit:	AX = -1 if aliasing
;		AX = 0 if not
;******************************************************************************
bid_check_aliasing	proc	near

	push	cx
	push	dx
	mov	dx,bx			;get input of reg 1
	add	dx,BID_REG_1
	mov	cx,4			;do next 5 registers (1 - 5)
bid_alias_loop:
	in	al,dx
	mov	ah,al
	push	dx
	add	dx,BID_LAR_OFFSET
	in	al,dx
	cmp	al,ah			;if reg 1 != lan addr 1
	pop	dx
	jne	no_aliasing		; then there is no aliasing
	inc	dx
	loop	bid_alias_loop

	mov	dx,bx			;get input of reg 7
	add	dx,BID_REG_7
	in	al,dx
	mov	ah,al
	add	dx,BID_LAR_OFFSET
	in	al,dx
	cmp	al,ah			;if reg 7 != lan addr 7
	jne	no_aliasing		; then there is no aliasing

	xor	ax,ax			;aliasing is true,
	dec	ax
	jmp	bid_exit_aliasing
no_aliasing:
	xor	ax,ax
bid_exit_aliasing:
	pop	dx
	pop	cx
	ret
					; so exit now with enough info 
bid_check_aliasing	endp


;******************************************************************************
;	bid_interface_chip
;
;	Purpose:  checks for the presence of an interface chip
;
;	Entry:  BX = Base I/O Address
;
;	Exit:	AX = -1 if interface chip is present
;		AX = 0 if not 
;******************************************************************************
bid_interface_chip	proc	near

	push	cx
	push	dx
	mov	dx,bx			;write and read register 7 (GP2)
	add	dx,BID_REG_7
	in	al,dx			;save original value
	mov	cl,al
	mov	al,35h
	out	dx,al
	jmp	$+2
	push	dx			;put something else on bus
	mov	dx,bx
	in	al,dx
	pop	dx
	jmp	$+2
	in	al,dx
	cmp	al,35h			;did it write???
	jne	no_interface_chip
	mov	al,3Ah			;try another value to make sure
	out	dx,al
	jmp	$+2
	push	dx			;put something else on bus
	mov	dx,bx
	in	al,dx
	pop	dx
	jmp	$+2
	in	al,dx
	cmp	al,3Ah			;did it write???
	jne	no_interface_chip
	mov	al,cl			;restore original value
	out	dx,al
	xor	ax,ax
	dec	ax
	jmp	bid_exit_intrfc_chip
no_interface_chip:
	xor	ax,ax
bid_exit_intrfc_chip:
	pop	dx
	pop	cx
	ret

bid_interface_chip	endp

;******************************************************************************
;	bid_board_16bit
;
;	Purpose:  To sense if this board has 16 bit capability
;
;	Entry:  BX = Base I/O Address
;
;	Exit:	AX = -1 if 16 bit capability
;		AX = 0 if not
;******************************************************************************
bid_board_16bit	proc	near

	push	bx			; preserve all registers
	push	cx
	push	dx
	mov	dx,bx			; register 1 has bit to test
	add	dx,BID_REG_1
	in	al,dx			; save previous value
	mov	cl,al			;  into cl
	mov	bl,al			; want only to compare with lob
	and	bl,BID_SIXTEEN_BIT_BIT	;  so mask it out
	xor	al,BID_SIXTEEN_BIT_BIT	; flip bit in question
	out	dx,al			; write new value
	jmp	$+2
	push	dx			;put something else on bus
	mov	dx,bx
	in	al,dx
	pop	dx
	in	al,dx			; read it back
	and	al,BID_SIXTEEN_BIT_BIT	; only care about lob
	cmp	al,bl			; did it stick???
	je	is_16bit_board		;   no, so show 16 bit board
	mov	al,cl			;   yes, so put back original value
	out	dx,al
	xor	ax,ax			; and show not 16 bit board
	jmp	bid_exit_board_16bit
is_16bit_board:
	and	cl,0FEh			;mask bit one if 16 bit board
	mov	al,cl			; put back original value just in case
	out	dx,al
	xor	ax,ax
	dec	ax
bid_exit_board_16bit:
	pop	dx
	pop	cx
	pop	bx
	ret

bid_board_16bit	endp

;******************************************************************************
;	bid_slot_16bit
;
;	Purpose:  To sense if this 16 bit board is in a 16 bit slot
;
;	Entry:  BX = Base I/O Address
;
;	Exit:	AX = -1 if 16 bit board is in a 16 bit slot
;		AX = 0 if not
;******************************************************************************
bid_slot_16bit	proc	near

	push	dx
	mov	dx,bx			; register 1 has bit to test
	add	dx,BID_REG_1
	in	al,dx
	and	al,BID_SIXTEEN_BIT_BIT	; is it in a 16 bit slot???
	jz	not_16bit_slot
	xor	ax,ax
	dec	ax
	jmp	bid_exit_slot_16bit
not_16bit_slot:
	xor	ax,ax
bid_exit_slot_16bit:
	pop	dx
	ret

bid_slot_16bit	endp

;******************************************************************************
;	bid_get_media_type
;
;	Purpose:  To find the media type of the board
;
;	Entry:  BX = Base I/O Address
;		CX = Board Revision Number
;
;	Exit:	AX = Media Type Bits
;******************************************************************************
bid_get_media_type	proc	near

	push	dx
	mov	dx,bx
	add	dx,BID_BOARD_ID_BYTE		;get the hardware ID byte
	in	al,dx
	test	al,BID_MEDIA_TYPE_BIT		;is it set?
	jz	bid_star_twisted		;no
	mov	ax,ETHERNET_MEDIA		;yes, show ethernet media
	jmp	bid_exit_media_type
bid_star_twisted:
	cmp	cx,1				;is it an old rev board?
	je	bid_show_starlan		;yes, must be starlan
	mov	ax,TWISTED_PAIR_MEDIA		;no, must be twisted pair
	jmp	bid_exit_media_type
bid_show_starlan:
	mov	ax,STARLAN_MEDIA		;show starlan
bid_exit_media_type:
	pop	dx
	ret

bid_get_media_type	endp

;******************************************************************************
;	bid_get_id_byte_info
;
;	Purpose: To extract information about the board using the
;		 hardware ID byte in the boards LAN address ROM
;
;	Entry:  AX = Current Board ID
;		BX = Base I/O Address
;
;	Exit:	AX = New Board ID Bits
;		DX = New Extra Bits
;******************************************************************************
bid_get_id_byte_info	proc	near

	push	BASE_PTR
	mov	BASE_PTR,STACK_PTR		;space for local vars
	sub	STACK_PTR,4

	xor	dx,dx				;init return values
	mov	WORD PTR [BASE_PTR-BID_LOCAL_BID],ax	;use current board ID
	mov	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],dx ;start new for extra
	mov	dx,bx				;get base I/O address
	add	dx,BID_BOARD_ID_BYTE		;read hardware board ID byte
	in	al,dx
	test	al,BID_BUS_TYPE_BIT		;is it a MCA adapter?
	jz	bid_not_mca_bus
	or	WORD PTR [BASE_PTR-BID_LOCAL_BID],MICROCHANNEL
bid_not_mca_bus:
	test	al,BID_SOFT_CONFIG_BIT		;is the soft config bit set?
	jz	bid_get_extra_exit		;no
						;yes, so interpret soft cfg bit
	cmp	WORD PTR [BASE_PTR-BID_LOCAL_BID],WD8003EB	;EB board?
	jz	bid_show_alt_irq			;yes, show alt irq
	cmp	WORD PTR [BASE_PTR-BID_LOCAL_BID],WD8003W	;W board?
	jz	bid_show_alt_irq			;yes, show alt irq
	jmp	bid_get_extra_exit
bid_show_alt_irq:
	or	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],ALTERNATE_IRQ_BIT
bid_get_extra_exit:
	mov	ax,WORD PTR [BASE_PTR-BID_LOCAL_BID]	;use current board ID
	mov	dx,WORD PTR [BASE_PTR-BID_LOCAL_EXTRA] ;start new

	mov	STACK_PTR,BASE_PTR
	pop	BASE_PTR
	ret

bid_get_id_byte_info	endp

;******************************************************************************
;	bid_get_eeprom_info
;
;	Purpose: To extract information about the board using the
;		 ID bytes in the EEPROM
;
;	Entry:  AX = Current Board ID
;		BX = Base I/O Address
;
;	Exit:	AX = New Board ID Bits
;		DX = New Extra Bits
;******************************************************************************
bid_get_eeprom_info	proc	near

	push	BASE_PTR
	mov	BASE_PTR,STACK_PTR		;space for local vars
	sub	STACK_PTR,4

	xor	dx,dx				;init return values
	mov	WORD PTR [BASE_PTR-BID_LOCAL_BID],ax	;use current board ID
	mov	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],dx ;start new for extra
	call	bid_recall_engr_eeprom
	mov	dx,bx				;get base io address
	add	dx,BID_EEPROM_1
	in	al,dx
	mov	ah,al
	and	al,BID_EEPROM_BUS_TYPE_MASK
	cmp	al,BID_EEPROM_BUS_TYPE_MCA
	jne	bid_not_mca
	or	WORD PTR [BASE_PTR-BID_LOCAL_BID],MICROCHANNEL
	jmp	bid_got_the_bus_type
bid_not_mca:
bid_set_default_bus_type:
bid_got_the_bus_type:
	mov	al,ah				;get io value back
	and	al,BID_EEPROM_BUS_SIZE_MASK
	cmp	al,BID_EEPROM_BUS_SIZE_16BIT
	jne	bid_not_16bit_bus
	or	WORD PTR [BASE_PTR-BID_LOCAL_BID],BOARD_16BIT
	call	bid_slot_16bit		;is 16 bit board in 16 bit slot???
	jz	bid_got_the_bus_size	; no
	or	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],SLOT_16BIT
	jmp	bid_got_the_bus_size
bid_not_16bit_bus:
bid_set_default_bus_size:
bid_got_the_bus_size:
	mov	dx,bx				;get base io address
	add	dx,BID_EEPROM_0
	in	al,dx
	mov	ah,al
	and	al,BID_EEPROM_MEDIA_MASK
	cmp	al,BID_STARLAN_TYPE
	jne	bid_not_starlan
	or	WORD PTR [BASE_PTR-BID_LOCAL_BID],STARLAN_MEDIA
	jmp	bid_got_the_media
bid_not_starlan:
	cmp	al,BID_TP_TYPE
	jne	bid_not_tp
	or	WORD PTR [BASE_PTR-BID_LOCAL_BID],TWISTED_PAIR_MEDIA
	jmp	bid_got_the_media
bid_not_tp:
	cmp	al,BID_EW_TYPE
	jne	bid_not_ew
	or	WORD PTR [BASE_PTR-BID_LOCAL_BID],EW_MEDIA
	jmp	bid_got_the_media
bid_not_ew:
	cmp	al,BID_ETHERNET_TYPE
	jne	bid_not_ethernet_media
	or	WORD PTR [BASE_PTR-BID_LOCAL_BID],ETHERNET_MEDIA
	jmp	bid_got_the_media
bid_not_ethernet_media:
bid_set_default_media:
	or	WORD PTR [BASE_PTR-BID_LOCAL_BID],ETHERNET_MEDIA
bid_got_the_media:
	mov	al,ah				;get io value back
	and	al,BID_EEPROM_IRQ_MASK
	cmp	al,BID_ALTERNATE_IRQ_1
	jne	bid_set_default_irq
	or	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],ALTERNATE_IRQ_BIT
;	jmp	bid_got_the_irq
bid_set_default_irq:
bid_got_the_irq:
	mov	al,ah				;get io value back
	and	al,BID_EEPROM_RAM_SIZE_MASK
	cmp	al,BID_EEPROM_RAM_SIZE_8K
	jne	bid_not_8k_ram
bid_set_8k_ram:
	or	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],RAM_SIZE_8K
	jmp	bid_got_the_ram
bid_not_8k_ram:
	cmp	al,BID_EEPROM_RAM_SIZE_16K
	jne	bid_eeprom_not_16k_ram
	test	WORD PTR [BASE_PTR-BID_LOCAL_BID],BOARD_16BIT
	jz	bid_just_set_16k
	test	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],SLOT_16BIT
	jz	bid_set_8k_ram
bid_just_set_16k:
	or	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],RAM_SIZE_16K
	jmp	bid_got_the_ram
bid_eeprom_not_16k_ram:
	cmp	al,BID_EEPROM_RAM_SIZE_32K
	jne	bid_not_32k_ram
bid_set_32k_ram:
	or	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],RAM_SIZE_32K
	jmp	bid_got_the_ram
bid_not_32k_ram:
	cmp	al,BID_EEPROM_RAM_SIZE_64K
	jne	bid_not_64k_ram
	test	WORD PTR [BASE_PTR-BID_LOCAL_BID],BOARD_16BIT
	jz	bid_just_set_64k
	test	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],SLOT_16BIT
	jz	bid_set_32k_ram
bid_just_set_64k:
	or	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],RAM_SIZE_64K
	jmp	bid_got_the_ram
bid_not_64k_ram:
bid_set_default_ram:
	or	WORD PTR [BASE_PTR-BID_LOCAL_EXTRA],RAM_SIZE_UNKNOWN
bid_got_the_ram:
	call	bid_recall_lan_address
	mov	ax,WORD PTR [BASE_PTR-BID_LOCAL_BID]	;use current board ID
	mov	dx,WORD PTR [BASE_PTR-BID_LOCAL_EXTRA] ;start new

	mov	STACK_PTR,BASE_PTR
	pop	BASE_PTR
	ret

bid_get_eeprom_info	endp

;******************************************************************************
;	bid_recall_engr_eeprom
;
;	Purpose: To recall the reserved Engineering bytes from the EEPROM
;
;	Entry:  BX = Base I/O Address
;
;	Exit:	All registers preserved
;
;******************************************************************************
bid_recall_engr_eeprom	proc	near

	push	ax
	push	dx
	mov	dx, bx
	add	dx, BID_REG_1
	in	al, dx
	and	al, BID_ICR_MASK
	or	al, BID_OTHER_BIT
	out	dx, al
	mov	dx, bx
	add	dx, BID_REG_3
	in	al, dx
	and	al, BID_EAR_MASK
	or	al, BID_ENGR_PAGE
	out	dx, al
	mov	dx, bx
	add	dx, BID_REG_1
	in	al, dx
	and	al, BID_ICR_MASK
	or	al, (BID_RLA OR BID_OTHER_BIT)
	out	dx, al
	call	bid_wait_for_recall
	pop	dx
	pop	ax
	ret

bid_recall_engr_eeprom	endp

;******************************************************************************
;	bid_recall_lan_address
;
;	Purpose: To recall the LAN Address bytes from the EEPROM
;
;	Entry:  BX = Base I/O Address
;
;	Exit:	All registers preserved
;
;******************************************************************************
bid_recall_lan_address	proc	near

	push	ax
	push	dx
	mov	dx, bx
	add	dx, BID_REG_1
	in	al, dx
	and	al, BID_ICR_MASK
	or	al, BID_OTHER_BIT
	out	dx, al
	mov	dx, bx
	add	dx, BID_REG_3
	in	al, dx
	and	al, BID_EAR_MASK
	or	al, BID_EA6
	out	dx, al
	mov	dx, bx
	add	dx, BID_REG_1
	in	al, dx
	and	al, BID_ICR_MASK
	or	al, BID_RLA
	out	dx, al
	call	bid_wait_for_recall
	pop	dx
	pop	ax
	ret

bid_recall_lan_address	endp

;******************************************************************************
;	bid_wait_for_recall
;
;	Purpose: To wait for the recall operation to complete
;
;	Entry:  BX = Base I/O Address
;
;	Exit:	All registers preserved
;
;******************************************************************************
bid_wait_for_recall	proc	near

	push	ax
	push	dx
	mov	dx, bx
	add	dx, BID_REG_1
bid_recall_loop:
	in	al, dx
	and	al, BID_RECALL_DONE_MASK
	jnz	bid_recall_loop
	pop	dx
	pop	ax
	ret

bid_wait_for_recall	endp

;******************************************************************************
;	bid_get_ram_size
;
;	Purpose: To figure out, if possible, the size of the RAM 
;		 available on this board
;
;	Entry:  AX = Current Board ID
;		BX = Base I/O Address
;		CX = Board Revision Number
;		DX = Extra Info
;
;	Exit:	AX = RAM Size Code
;******************************************************************************
bid_get_ram_size	proc	near

	push	cx			;preserve registers
	push	dx
	cmp	cx,2			;is the rev number less than 2?
	jge	bid_ram_new_rev		;no, so use RAM_SIZE_BIT in hardware
	test	ax,MICROCHANNEL		;is it a microchannel board?
	jnz	bid_show_16k_ram
	test	ax,BOARD_16BIT		;is it our 16 bit board?
	jz	bid_not_16k_ram
	test	dx,SLOT_16BIT		;is it in a 16 bit slot?
	jz	bid_show_8k_ram
bid_show_16k_ram:
	mov	ax,RAM_SIZE_16K		;yes, show 16K RAM
	jmp	bid_ram_size_exit
bid_show_8k_ram:
	mov	ax,RAM_SIZE_8K
	jmp	bid_ram_size_exit
bid_not_16k_ram:
	test	ax,INTERFACE_CHIP	;is this an EB board?
	jnz	bid_get_ifchp_ram_size	;yes
	mov	ax,RAM_SIZE_UNKNOWN	;no, so I cant tell the RAM size
	jmp	bid_ram_size_exit
bid_get_ifchp_ram_size:
	mov	dx,bx			;get base I/O address
	add	dx,BID_REG_1		;look at memory size bit in reg 1
	in	al,dx			;get register value
	test	al,BID_MSZ_583_BIT	;is the MSZ bit set?
	mov	ax,RAM_SIZE_8K		;pre-load return value
	jz	bid_ram_size_exit	;no, so show 8k RAM
	mov	ax,RAM_SIZE_32K		;yes, so show 32k RAM
	jmp	bid_ram_size_exit
bid_ram_new_rev:
	push	dx
	mov	cx,ax			;put current board ID into cx
	mov	dx,bx			;get base I/O address
	add	dx,BID_BOARD_ID_BYTE	;read hardware ID byte
	in	al,dx
	pop	dx
	cmp	cx,WD8003E		;is it a simple E board?
	je	bid_ram_8k_32k		;yes, so make the choice
	cmp	cx,WD8003S		;is it a simple S board?
	je	bid_ram_8k_32k		;yes, so make choice
	cmp	cx,WD8003WT
	je	bid_ram_8k_32k
	cmp	cx,WD8003W
	je	bid_ram_8k_32k
	cmp	cx,WD8003EB
	je	bid_ram_8k_32k
	test	cx,MICROCHANNEL		;is it a micro channel board
	jnz	bid_ram_16k_64k		;yes, so show 16k
	cmp	cx,WD8013EBT		;is it a 16 bit board?
	je	bid_ram_8k_64k		;yes
	mov	ax,RAM_SIZE_UNKNOWN	;default to unknown
	jmp	bid_ram_size_exit
bid_ram_8k_32k:
	test	al,BID_RAM_SIZE_BIT	;does the hardware say large RAM?
	mov	ax,RAM_SIZE_8K		;pre-load return value
	jz	bid_ram_size_exit
	mov	ax,RAM_SIZE_32K		;show large ram
	jmp	bid_ram_size_exit
bid_ram_8k_64k:
	test	dx,SLOT_16BIT
	jz	bid_ram_8k_32k
bid_ram_16k_64k:
	test	al,BID_RAM_SIZE_BIT	;does the hardware say large RAM?
	mov	ax,RAM_SIZE_16K		;show 16k RAM
	jz	bid_ram_size_exit
	mov	ax,RAM_SIZE_64K		;show 64k RAM
	jmp	bid_ram_size_exit
bid_ram_size_exit:
	pop	dx			;restore registers
	pop	cx
	ret

bid_get_ram_size	endp

;******************************************************************************
;	bid_check_for_690
;
;	Purpose: To identify the NIC as a 690 or an 8390
;
;	Entry:  BX = Base I/O Address
;
;	Exit:	AX = -1 if 690
;		     0 if 8390
;******************************************************************************
bid_check_for_690	proc	near

	push	BASE_PTR
	mov	BASE_PTR,STACK_PTR	;space for local variables
	sub	STACK_PTR,4

	push	bx			;preserve registers
	push	cx
	push	dx
	mov	dx,bx			;get copy of base I/O address
	add	dx,BID_CR		;ready for NIC command register
	in	al,dx			;get current register contents
	mov	cl,BID_TXP		;need to mask CR with ~BID_TXP
	xor	cl,0FFh			;invert CL contents
	and	cl,al			;CR without BID_TXP bit
	mov	WORD PTR [BASE_PTR-BID_LOCAL_CR],cx	;save this value
	and	cl,BID_PS_MASK		;CR without PS bits
	mov	al,cl
	or	al,BID_PS2		;switch to page 2 registers
	out	dx,al
	add	dx,BID_TCR_DIFF		;address TCR
	in	al,dx
	mov	WORD PTR [BASE_PTR-BID_LOCAL_TCR],ax	;save this value
	sub	dx,BID_TCR_DIFF		;address CR
	mov	al,cl			;get CR without TXP or PS bits
	out	dx,al			;now at page 0 registers
	add	dx,BID_TCR_DIFF		;address TCR
	mov	al,BID_TCR_VAL		;test value
	out	dx,al
	sub	dx,BID_TCR_DIFF		;address CR
	mov	al,cl			;get CR without TXP or PS bits
	or	al,BID_PS2		;or in Page Select 2 bits
	out	dx,al
	add	dx,BID_TCR_DIFF		;address TCR
	in	al,dx
	and	al,BID_TCR_VAL		;only use the test bits
	cmp	al,BID_TCR_VAL		;does it equal the test value???
	mov	bx,0			;pre-load 8390 answer
	je	bid_chck_690_exit	;yes, must be 8390
	mov	bx,0FFFFh		;load 690 answer
bid_chck_690_exit:
	mov	al,cl			;get CR without TXP or PS bits
	sub	dx,BID_TCR_DIFF		;address CR
	out	dx,al			;select page 0
	add	dx,BID_TCR_DIFF		;address TCR
	mov	ax,WORD PTR [BASE_PTR-BID_LOCAL_TCR]	;get saved value
	out	dx,al			;restore TCR
	sub	dx,BID_TCR_DIFF		;address CR
	mov	ax,WORD PTR [BASE_PTR-BID_LOCAL_CR]	;get saved value
	out	dx,al			;restore CR

	mov	ax,bx			;put return code in AX
	or	ax,ax			;set flags
	pop	dx			;restore registers
	pop	cx
	pop	bx
	mov	STACK_PTR,BASE_PTR	;restore stack
	pop	BASE_PTR
	ret

bid_check_for_690	endp

