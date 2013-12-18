;**********************************************************
; GetCnfg    Get all configuration Information for 
;            the Interface chips -- 583,584,593 and 594
;
;
; Return: 
;      Ax -- 0, get configuration successfully.
;         -- 01, bourd_found but didn't get any information.
;         -- ffff, didn't find the board.
;      All registers except ax will be preserved.
;
;      All configuration information is put into
;      the configuration structure. 
;
;************************************************************

IFDEF	CODE386
wdm_gc_base_ptr		equ	ebp
wdm_gc_struc_ptr	equ	[ebp]
ELSE
wdm_gc_base_ptr		equ	bp
wdm_gc_struc_ptr	equ	ds:[bp]
ENDIF
;
; Adapter POS ID's
;
CNFG_ID_8003E		equ	6fc0h
CNFG_ID_8003S		equ	6fc1h
CNFG_ID_8003W		equ	6fc2h
CNFG_ID_8013E		equ	61C8h
CNFG_ID_8013W		equ	61C9h
CNFG_ID_BISTRO03E	equ	0EFE5h
CNFG_ID_BISTRO13E	equ	0EFD5h
CNFG_ID_BISTRO13W	equ	0EFD4h	
;
; 583 & 584 registers, needed for getinfo
;
CNFG_MSR_583		equ	0
CNFG_ICR_583		equ	1
CNFG_IAR_583		equ	2
CNFG_BIO_583		equ	3
CNFG_IRR_583		equ	4
CNFG_LAAR_584		equ	5
CNFG_GP2		equ	7
CNFG_LAAR_MASK		equ	01Fh
CNFG_LAAR_ZWS		equ	020h
CNFG_ICR_IR2_584	equ	04h
CNFG_IRR_IRQS		equ	060h
CNFG_IRR_IEN		equ	080h
CNFG_IRR_ZWS		equ	01h
CNFG_GP2_BOOT_NIBBLE	equ	0Fh
;
CNFG_SIZE_8kb		equ	8
CNFG_SIZE_16kb		equ	16
CNFG_SIZE_32kb		equ	32
CNFG_SIZE_64kb		equ	64
;
ROM_DISABLE		equ	0
;
CNFG_SLOT_ENABLE_BIT	equ	08h
;
CNFG_POS_CONTROL_REG	equ	096h
CNFG_POS_REG0		equ	100h
CNFG_POS_REG1		equ	101h
CNFG_POS_REG2		equ	102h
CNFG_POS_REG3		equ	103h
CNFG_POS_REG4		equ	104h
CNFG_POS_REG5		equ	105h

;******************************************************************************
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public WDM_GetCnfg
ENDIF
WDM_GetCnfg	proc	near

IFNDEF	CODE386
	push	ds
	push	es
ENDIF
	push	wdm_gc_base_ptr
	push	bx
	push	cx
	push	dx

	cmp	wdm_gc_struc_ptr.cnfg_bus,1
	jz	wdm_gc_mca_type
	call	wdm_gc_get_at_config
	jmp	wdm_gc_at_exit
wdm_gc_mca_type:
	call	wdm_gc_get_mca_config
wdm_gc_at_exit:

	pop	dx
	pop	cx
	pop	bx
	pop	wdm_gc_base_ptr
IFNDEF	CODE386
	pop	es
	pop	ds
ENDIF
	ret

WDM_GetCnfg	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_disable_slot
ENDIF
wdm_gc_disable_slot	proc	near

	push	ax
	mov	al,0
	out	CNFG_POS_CONTROL_REG, al
	jmp	$+2
	pop	ax
	ret

wdm_gc_disable_slot	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_enable_slot
ENDIF
wdm_gc_enable_slot	proc	near

	push	ax
	mov	ax, wdm_gc_struc_ptr.cnfg_slot
	or	ax, ax
	jz	wdm_gc_protect_slot
	dec	ax
wdm_gc_protect_slot:
	or	ax, CNFG_SLOT_ENABLE_BIT
	out	CNFG_POS_CONTROL_REG,al
	jmp	$+2
	pop	ax
	ret

wdm_gc_enable_slot	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;	Returns: AX = POS ID
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_get_pos_id
ENDIF
wdm_gc_get_pos_id	proc	near

	push	dx
	mov	dx, CNFG_POS_REG1
	in	al, dx
	jmp	$+2
	mov	ah, al
	mov	dx, CNFG_POS_REG0
	in	al, dx
	jmp	$+2
	pop	dx
	ret

wdm_gc_get_pos_id	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_get_mca_config
ENDIF
wdm_gc_get_mca_config	proc	near

	call	wdm_gc_enable_slot
	call	wdm_gc_get_pos_id	; returns AX = POS ID
	call	wdm_gc_check_594_group
	jz	wdm_gc_594_card
	call	wdm_gc_check_593_group
	jz	wdm_gc_593_card
	mov	ax, 0ffffh
	jmp	short wdm_gc_bad_exit
wdm_gc_593_card:
	call	wdm_gc_get_593_cnfg
	jmp	short wdm_gc_good_exit
wdm_gc_594_card:
	call	wdm_gc_get_594_cnfg
wdm_gc_good_exit:
	push	ax
	mov	ax, 1
	call	wdm_gc_get_bid
	pop	ax
wdm_gc_bad_exit:
	call	wdm_gc_disable_slot
	or	ax, ax
	ret

wdm_gc_get_mca_config	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_get_bid
ENDIF
wdm_gc_get_bid	proc	near

	push	ax
	push	dx
	mov	dx, wdm_gc_struc_ptr.cnfg_base_io
	call	GetBoardID
	mov	wdm_gc_struc_ptr.cnfg_bid, ax
	mov	wdm_gc_struc_ptr.cnfg_extra_info, dx
	pop	dx
	pop	ax
	ret

wdm_gc_get_bid	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_get_594_cnfg
ENDIF
wdm_gc_get_594_cnfg	proc	near

	call	wdm_gc_594_io
	call	wdm_gc_594_irq
	call	wdm_gc_594_ram_base
	call	wdm_gc_594_ram_size
	call	wdm_gc_594_rom_base
	call	wdm_gc_594_rom_size
	xor	ax,ax
	ret

wdm_gc_get_594_cnfg	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_get_593_cnfg
ENDIF
wdm_gc_get_593_cnfg	proc	near

	call	wdm_gc_593_io
	call	wdm_gc_593_irq
	call	wdm_gc_593_ram_base
	call	wdm_gc_593_ram_size
	call	wdm_gc_593_rom_base
	call	wdm_gc_593_rom_size
	xor	ax,ax
	ret

wdm_gc_get_593_cnfg	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_593_io
ENDIF
wdm_gc_593_io	proc	near

	mov	dx, CNFG_POS_REG2
	in	al, dx
	jmp	$+2
	and	al, 0FEh
	xor	ah, ah
	mov	cl, 4
	shl	ax, cl
	mov	wdm_gc_struc_ptr.cnfg_base_io, ax
	ret

wdm_gc_593_io	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_593_irq
ENDIF
wdm_gc_593_irq	proc	near

	mov	dx, CNFG_POS_REG5
	in	al, dx
	jmp	$+2
	mov	ah, 3
	and	al, 03h
	jz	wdm_gc_593_irq_out
	mov	ah, 4
	dec	al
	jz	wdm_gc_593_irq_out
	mov	ah, 10
	dec	al
	jz	wdm_gc_593_irq_out
	mov	ah, 15
	dec	al
wdm_gc_593_irq_out:
	xchg	ah, al
	mov	wdm_gc_struc_ptr.cnfg_irq_line, ax
	ret

wdm_gc_593_irq	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_593_ram_base
ENDIF
wdm_gc_593_ram_base	proc	near

	mov	dx, CNFG_POS_REG3
	in	al, dx
	jmp	$+2
	and	al, 0FCh
	xor	ah, ah
	mov	cl, 4
	shl	ax, cl
	xor	dx, dx
	xchg	dl, ah
	mov	cl, 8
	shl	ax, cl
	mov	word ptr wdm_gc_struc_ptr.cnfg_ram_base, ax
	mov	word ptr wdm_gc_struc_ptr.cnfg_ram_base+2, dx
	ret

wdm_gc_593_ram_base	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_593_ram_size
ENDIF
wdm_gc_593_ram_size	proc	near

	mov	wdm_gc_struc_ptr.cnfg_ram_size, CNFG_SIZE_16kb
	ret

wdm_gc_593_ram_size	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_593_rom_base
ENDIF
wdm_gc_593_rom_base	proc	near

	mov	dx, CNFG_POS_REG4
	in	al, dx
	jmp	$+2
	and	al, 0FCh
	xor	ah, ah
	mov	cl, 4
	shl	ax, cl
	xor	dx, dx
	xchg	dl, ah
	mov	cl, 8
	shl	ax, cl
	mov	word ptr wdm_gc_struc_ptr.cnfg_rom_base, ax
	mov	word ptr wdm_gc_struc_ptr.cnfg_rom_base+2, dx
	ret

wdm_gc_593_rom_base	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_593_rom_size
ENDIF
wdm_gc_593_rom_size	proc	near

	mov	dx, CNFG_POS_REG4
	in	al, dx
	jmp	$+2
	mov	ah, CNFG_SIZE_16kb
	and	al, 03h
	jz	wdm_gc_593_rs_out
	mov	ah, CNFG_SIZE_32kb
	dec	al
	jz	wdm_gc_593_rs_out
	mov	ah, ROM_DISABLE
	dec	al
	jz	wdm_gc_593_rs_out
	mov	ah, CNFG_SIZE_64kb
	dec	al
wdm_gc_593_rs_out:
	xchg	ah, al
	mov	wdm_gc_struc_ptr.cnfg_rom_size, ax
	ret

wdm_gc_593_rom_size	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_594_io
ENDIF
wdm_gc_594_io	proc	near

	mov	dx, CNFG_POS_REG2
	in	al, dx
	jmp	$+2
	and	al, 0F0h
	mov	ah, al
	xor	al, al
	add	ax, 0800h
	mov	wdm_gc_struc_ptr.cnfg_base_io, ax
	ret

wdm_gc_594_io	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_594_irq
ENDIF
wdm_gc_594_irq	proc	near

	mov	dx, CNFG_POS_REG5
	in	al,dx
	jmp	$+2
	and	al, 0Ch
	shr	al, 1
	shr	al, 1
	mov	ah, 3
	or	al, al
	jz	wdm_gc_594_irq_out
	mov	ah, 4
	dec	al
	jz	wdm_gc_594_irq_out
	mov	ah, 10
	dec	al
	jz	wdm_gc_594_irq_out
	mov	ah, 14
	dec	al
wdm_gc_594_irq_out:
	xchg	al, ah
	mov	wdm_gc_struc_ptr.cnfg_irq_line, ax
	ret

wdm_gc_594_irq	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_594_ram_base
ENDIF
wdm_gc_594_ram_base	proc	near

	mov	dx, CNFG_POS_REG3
	in	al, dx
	jmp	$+2
	mov	dx, 000Dh
	test	al, 08h
	jnz	above_cseg
	mov	dx, 000Ch
above_cseg:
	test	al, 080h
	jz	below_1mg
	or	dx, 00F0h
below_1mg:
	xor	ah, ah
	and	al, 07h
	mov	cl, 13
	shl	ax, cl
	mov	word ptr wdm_gc_struc_ptr.cnfg_ram_base, ax
	mov	word ptr wdm_gc_struc_ptr.cnfg_ram_base+2, dx
	ret

wdm_gc_594_ram_base	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_594_ram_size
ENDIF
wdm_gc_594_ram_size	proc	near

	mov	dx, CNFG_POS_REG3
	in	al, dx
	jmp	$+2
	and	al, 030h
	mov	cl, 4
	shr	al, cl
	mov	cl, al
	mov	ax, CNFG_SIZE_8kb
	shl	ax, cl
	mov	wdm_gc_struc_ptr.cnfg_ram_size, ax
	ret

wdm_gc_594_ram_size	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_594_rom_base
ENDIF
wdm_gc_594_rom_base	proc	near

	mov	dx, CNFG_POS_REG4
	in	al, dx
	jmp	$+2
	mov	dx, 0Dh
	test	al, 08h
	jnz	rom_above_cseg
	mov	dx, 0Ch
rom_above_cseg:
	xor	ah, ah
	and	al, 07h
	mov	cl, 13
	shl	ax, cl
	mov	word ptr wdm_gc_struc_ptr.cnfg_rom_base, ax
	mov	word ptr wdm_gc_struc_ptr.cnfg_rom_base+2, dx
	ret

wdm_gc_594_rom_base	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_594_rom_size
ENDIF
wdm_gc_594_rom_size	proc	near

	mov	dx, CNFG_POS_REG4
	in	al, dx
	jmp	$+2
	mov	cl, 4
	shr	al, cl
	mov	ah, CNFG_SIZE_8kb
	or	al, al
	jz	wdm_gc_594_rs_out
	mov	ah, CNFG_SIZE_16kb
	dec	al
	jz	wdm_gc_594_rs_out
	mov	ah, CNFG_SIZE_32kb
	dec	al
	jz	wdm_gc_594_rs_out
	mov	ah, ROM_DISABLE
	dec	al
wdm_gc_594_rs_out:
	xchg	ah, al
	mov	wdm_gc_struc_ptr.cnfg_rom_size, ax
	ret

wdm_gc_594_rom_size	endp

;**************************************
;	on entry ax: POSID
;
;	return:	zero flag is set if the
;	ID is in the group
;***************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_check_593_group
ENDIF
wdm_gc_check_593_group	proc	near

IFNDEF	BISTRO
	cmp	ax, CNFG_ID_8003E
	jz	Checked_593_out
	cmp	ax, CNFG_ID_8003S
	jz	Checked_593_out
	cmp	ax, CNFG_ID_8003W
	jz	Checked_593_out
ENDIF
	cmp	ax, CNFG_ID_BISTRO03E
	jz	Checked_593_out
Checked_593_out:
	ret

wdm_gc_check_593_group	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_check_594_group
ENDIF
wdm_gc_check_594_group	proc	near

IFNDEF	BISTRO
	cmp	ax, CNFG_ID_8013E
	jz	Checked_594_out
	cmp	ax, CNFG_ID_8013W
	jz	Checked_594_out
ENDIF
	cmp	ax, CNFG_ID_BISTRO13E
	jz	Checked_594_out
	cmp	ax, CNFG_ID_BISTRO13W
Checked_594_out:
	ret

wdm_gc_check_594_group	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_get_at_config
ENDIF
wdm_gc_get_at_config	proc	near

	mov	dx, wdm_gc_struc_ptr.cnfg_base_io
	call	wdm_gc_is_board_there
	jnz	wdm_gc_at_no_board
	mov	ax, 0
	call	wdm_gc_get_bid
	call	wdm_gc_copy_bid_ram_size
	test	wdm_gc_struc_ptr.cnfg_bid, INTERFACE_CHIP
	jz	wdm_gc_at_no_chip
	call	wdm_gc_58x_irq
	call	wdm_gc_58x_irq_status
	call	wdm_gc_58x_ram_base
	call	wdm_gc_58x_rom_base
	call	wdm_gc_58x_rom_size
	call	wdm_gc_58x_boot_status
	call	wdm_gc_58x_zero_wait_state
	jmp	get_at_exit
wdm_gc_at_no_chip:
	mov	ax, 01h
	ret
wdm_gc_at_no_board:
	mov	ax, 0ffffh
	ret
get_at_exit:
	xor	ax, ax
	ret

wdm_gc_get_at_config	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_copy_bid_ram_size
ENDIF
wdm_gc_copy_bid_ram_size	proc	near

	mov	ax, wdm_gc_struc_ptr.cnfg_extra_info
	and	ax, RAM_SIZE_MASK
	cmp	ax, RAM_SIZE_8K
	jb	wdm_gc_cbidrs_exit
	cmp	ax, RAM_SIZE_64K
	ja	wdm_gc_cbidrs_exit
	mov	cl, al
	dec	cl
	dec	cl
	mov	ax, CNFG_SIZE_8kb
	shl	ax, cl
	mov	wdm_gc_struc_ptr.cnfg_ram_size, ax
wdm_gc_cbidrs_exit:
	ret

wdm_gc_copy_bid_ram_size	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_58x_irq
ENDIF
wdm_gc_58x_irq	proc	near
	mov	dx, wdm_gc_struc_ptr.cnfg_base_io
	mov	ax, wdm_gc_struc_ptr.cnfg_extra_info
	mov	ch, 0
	and	ax, INTERFACE_CHIP_MASK
	cmp	ax, INTERFACE_5X3_CHIP
	je	wdm_gc_not_584_chip
	push	dx
	add	dx, CNFG_ICR_583
	in	al, dx
	jmp	$+2
	pop	dx
	and	al, CNFG_ICR_IR2_584
	mov	ch, al
wdm_gc_not_584_chip:
	add	dx, CNFG_IRR_583
	in	al,dx
	jmp	$+2
	and	al, CNFG_IRR_IRQS
	mov	cl, 5
	shr	al, cl
	cmp	al, 0
	jne	wdm_not_irq_0
	or	ch, ch
	jz	wdm_gc_no_adjust_1
	mov	ah, 10
	jmp	short wdm_gc_load_58x_irq
wdm_gc_no_adjust_1:
	mov	ah, 2
	jmp	short wdm_gc_load_58x_irq
wdm_not_irq_0:
	cmp	al, 1
	jne	wdm_gc_not_irq1
	or	ch, ch
	jz	wdm_gc_no_adjust_2
	mov	ah, 11
	jmp	short wdm_gc_load_58x_irq
wdm_gc_no_adjust_2:
	mov	ah, 3
	jmp	short wdm_gc_load_58x_irq
wdm_gc_not_irq1:
	cmp	al, 2
	jne	wdm_gc_not_irq_2
	or	ch, ch
	jz	wdm_gc_no_adjust_3
	mov	ah, 15
	jmp	short wdm_gc_load_58x_irq
wdm_gc_no_adjust_3:
	mov	ah, 4
	test	wdm_gc_struc_ptr.cnfg_extra_info, ALTERNATE_IRQ_BIT
	jz	wdm_gc_load_58x_irq
	mov	ah, 5
	jmp	short wdm_gc_load_58x_irq
wdm_gc_not_irq_2:
	cmp	al, 3
	jne	wdm_gc_irq_error
	or	ch, ch
	jz	wdm_gc_no_adjust_4
	mov	ah, 4
	jmp	short wdm_gc_load_58x_irq
wdm_gc_no_adjust_4:
	mov	ah, 7
	jmp	short wdm_gc_load_58x_irq
wdm_gc_irq_error:
	mov	ah, 03h
wdm_gc_load_58x_irq:
	xor	al, al
	xchg	ah, al
	mov	wdm_gc_struc_ptr.cnfg_irq_line, ax
	ret

wdm_gc_58x_irq	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public	wdm_gc_58x_irq_status
ENDIF
wdm_gc_58x_irq_status	proc	near
	mov	bx, wdm_gc_struc_ptr.cnfg_mode_bits1
	and	bx, NOT INTERRUPT_STATUS_BIT
	mov	dx, wdm_gc_struc_ptr.cnfg_base_io
	add	dx, CNFG_IRR_583
	in	al,dx
	jmp	$+2
	and	al, CNFG_IRR_IEN
	jz	wdm_gc_58x_is_disabled
	or	bx, INTERRUPT_STATUS_BIT
wdm_gc_58x_is_disabled:
	mov	wdm_gc_struc_ptr.cnfg_mode_bits1, bx
	ret

wdm_gc_58x_irq_status	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public	wdm_gc_58x_boot_status
ENDIF
wdm_gc_58x_boot_status	proc	near
	mov	bx, wdm_gc_struc_ptr.cnfg_mode_bits1
	and	bx, NOT BOOT_STATUS_MASK
	mov	dx, wdm_gc_struc_ptr.cnfg_base_io
	add	dx, CNFG_GP2
	in	al,dx
	jmp	$+2
	and	al, CNFG_GP2_BOOT_NIBBLE
	jnz	wdm_gc_58x_bs_disabled
	or	bx, BOOT_TYPE_1
wdm_gc_58x_bs_disabled:
	mov	wdm_gc_struc_ptr.cnfg_mode_bits1, bx
	ret

wdm_gc_58x_boot_status	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_58x_zero_wait_state
ENDIF
wdm_gc_58x_zero_wait_state	proc	near
	mov	bx, wdm_gc_struc_ptr.cnfg_mode_bits1
	and	bx, NOT ZERO_WAIT_STATE_MASK
	mov	dx, wdm_gc_struc_ptr.cnfg_base_io
	push	dx
	add	dx, CNFG_IRR_583
	in	al, dx
	jmp	$+2
	pop	dx
	and	al, CNFG_IRR_ZWS
	jz	wdm_gc_58x_zws8_disabled
	or	bx, ZERO_WAIT_STATE_8_BIT
wdm_gc_58x_zws8_disabled:
	test	wdm_gc_struc_ptr.cnfg_bid, BOARD_16BIT
	jz	wdm_gc_58x_zws16_disabled
	add	dx, CNFG_LAAR_584
	in	al, dx
	and	al, CNFG_LAAR_ZWS
	jz	wdm_gc_58x_zws16_disabled
	or	bx, ZERO_WAIT_STATE_16_BIT
wdm_gc_58x_zws16_disabled:
	mov	wdm_gc_struc_ptr.cnfg_mode_bits1, bx
	ret

wdm_gc_58x_zero_wait_state	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_58x_ram_base
ENDIF
wdm_gc_58x_ram_base	proc	near

	mov	dx, wdm_gc_struc_ptr.cnfg_base_io
	in	al, dx
	jmp	$+2
	and	al, 03Fh
	mov	cx, wdm_gc_struc_ptr.cnfg_extra_info
	and	cx, INTERFACE_CHIP_MASK
	cmp	cx, INTERFACE_5X3_CHIP
	je	wdm_gc_no_laar
	mov	ah, al
	add	dx, CNFG_LAAR_584
	in	al, dx
	jmp	$+2
	and	al, CNFG_LAAR_MASK
	mov	cl, 3
	shl	al, cl
	mov	dl, al
	xor	dh, dh
	mov	bl, ah
	and	bl, 038h
	mov	cl, 3
	shr	bl, cl
	or	dl, bl
	and	ah, 07h
	mov	cl, 5
	shl	ah, cl
	xor	al, al
	jmp	short wdm_gc_load_base
wdm_gc_no_laar:
	or	al, 040h
	mov	cl, 5
	shl	ax, cl
	xor	dx, dx
	xchg	dl, ah
	mov	cl, 8
	shl	ax, cl
wdm_gc_load_base:
	mov	word ptr wdm_gc_struc_ptr.cnfg_ram_base, ax
	mov	word ptr wdm_gc_struc_ptr.cnfg_ram_base+2, dx
	ret

wdm_gc_58x_ram_base	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_58x_rom_base
ENDIF
wdm_gc_58x_rom_base	proc	near

	mov	dx, wdm_gc_struc_ptr.cnfg_base_io
	add	dx, CNFG_BIO_583
	in	al, dx
	jmp	$+2
	xor	ah, ah
	and	al, 03eh
	or	al, 040h
	mov	cl, 5
	shl	ax, cl
	xor	dx, dx
	xchg	dl, ah
	mov	cl, 8
	shl	ax, cl
	mov	word ptr wdm_gc_struc_ptr.cnfg_rom_base, ax
	mov	word ptr wdm_gc_struc_ptr.cnfg_rom_base+2, dx
	ret

wdm_gc_58x_rom_base	endp

;******************************************************************************
;
;	Assumes: DS:BP -> CNFG_Structure
;
;******************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_58x_rom_size
ENDIF
wdm_gc_58x_rom_size	proc	near

	mov	dx, wdm_gc_struc_ptr.cnfg_base_io
	xor	ax, ax
	add	dx, CNFG_BIO_583
	in	al, dx
	jmp	$+2
	and	al, 0C0h
	jz	wdm_gc_inv_rom_size
	mov	cl, 6
	shr	al, cl
	mov	cl, al
	mov	ax, CNFG_SIZE_8kb
	shl	ax, cl
	mov	wdm_gc_struc_ptr.cnfg_rom_size, ax
	ret
wdm_gc_inv_rom_size:
	mov	wdm_gc_struc_ptr.cnfg_rom_size, ROM_DISABLE
	ret

wdm_gc_58x_rom_size	endp

;*************************************************************************
;
;	wdm_gc_is_board_there
;
;	Given the I/O Base IsBoardThere checks for an adapter 
;	by computing a checksum on the LAN address bytes .
;	The eight bytes starting at offset 8 should total FFh.
;
;	assumes: dx has the I/O Base Address
;
;	returns: flag = zero if our board
;
;*************************************************************************
IFDEF	WDM_GC_DEBUG
	public wdm_gc_is_board_there
ENDIF
wdm_gc_is_board_there	proc	near
	push	dx
	push	cx
	push	bx
	push	ax
	xor	ah,ah		;zero counter
	add	dx,08h		;LANAddressZero
	mov	cx, 8		; loop count
wdm_gc_ibt_loop:
	in	al,dx
	add	ah,al		; compute checksum
	inc	dx		;bump i/o pointer
	jmp	$+2
	jmp	$+2
	jmp	$+2
	jmp	$+2
	jmp	$+2
	jmp	$+2
	jmp	$+2
	loop	wdm_gc_ibt_loop
	cmp	ah,0ffH
	pop	ax
	pop	bx
	pop	cx
	pop	dx
	ret

wdm_gc_is_board_there	endp
