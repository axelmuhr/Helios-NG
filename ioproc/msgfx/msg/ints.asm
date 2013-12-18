CODE	SEGMENT	WORD public 'CODE'
	ASSUME	cs:CODE
	PUBLIC	_EN_I,_DIS_I
_EN_I	PROC	FAR
	sti
	ret
_EN_I	ENDP

_DIS_I	PROC	FAR
	cli
	ret
_DIS_I  ENDP
CODE	ENDS
	end
