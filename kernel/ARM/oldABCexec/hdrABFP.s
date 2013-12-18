        TTL     Functional Prototype based Helios Executive         > hdrABFP/s
        ; ---------------------------------------------------------------------
        ; This file controls the switches necessary to make a Functional
        ; Prototype based Executive.
        ; ---------------------------------------------------------------------
	; this is an AB1FP version
	
		GBLL	activebook
activebook	SETL	{FALSE}		; NOT an Active Book
		GBLL	heval
heval		SETL	{FALSE}		; NOT a HEVAL
		GBLL	hydra
hydra		SETL	(heval)

        ; ---------------------------------------------------------------------
        LNK     loexec.s
