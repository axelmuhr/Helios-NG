        TTL     Active Book Helios Executive                         > hdrAB1/s
        ; ---------------------------------------------------------------------
        ; This file controls the switches necessary to make an Active Book
	; based Executive.
        ; ---------------------------------------------------------------------

		GBLL	activebook
activebook	SETL	{TRUE}		; this is an Active Book version

		GBLL	heval
heval		SETL	{FALSE}		; NOT a HEVAL
		GBLL	hydra
hydra		SETL	(heval)		; HYDRA controlled by uController

        ; ---------------------------------------------------------------------
        LNK     loexec.s
