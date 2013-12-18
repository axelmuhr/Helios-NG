_TEXT    SEGMENT WORD public 'code'
        ASSUME cs:_TEXT

PUBLIC	_test_int, _call_int, _get_int

Wakeup_int	EQU	061H
DoInt		EQU	021H
GetInt		EQU	035H
GetInDOS	EQU	034H
MAGIC		EQU	4321H
arg1		equ	4

_test_int	PROC	NEAR
	push	di
	push	es

	mov	ah,GetInt	; see if already installed
	mov	al,Wakeup_int
	int	DoInt
	xor	ax,ax
	mov	di,bx
	mov	bx,es:[di-2]
	cmp	bx,MAGIC
	je	done
     	mov	ax,1
done:
	pop	es
	pop	di
	ret	
_test_int	ENDP

_get_int	PROC	NEAR
	push	bp
	mov	bp,sp
	push	es
	push	di
	push	si

	mov	ah,GetInt	; see if already installed
	mov	al,Wakeup_int
	int	DoInt

	mov	si,arg1[bp]
	mov	di,bx

	mov	ax,es:[di-12]
	mov	ds:[si+0],ax
	mov	ax,es:[di-10]
	mov	ds:[si+2],ax
	mov	ax,es:[di-8]
	mov	ds:[si+4],ax
	mov	ax,es:[di-6]
	mov	ds:[si+6],ax
	mov	ax,es:[di-4]
	mov	ds:[si+8],ax
	mov	ax,es:[di-2]
	mov	ds:[si+10],ax

	mov	ah,GetInDOS
	int	21H
	mov	al,es:[bx]
	xor	ah,ah
	mov	ds:[si+12],ax

	pop	si
	pop	di
	pop	es
	pop	bp
	ret	
_get_int	ENDP


_call_int	PROC	NEAR
	int	Wakeup_int
	ret
_call_int	ENDP

_TEXT	ENDS
	END
