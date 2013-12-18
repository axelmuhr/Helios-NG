        TTL     HEVAL prototype based Helios Executive             > hdrHEVAL/s
        ; ---------------------------------------------------------------------
        ; This file controls the switches necessary to make a HEVAL prototype
	; based Executive.
        ; ---------------------------------------------------------------------

		GBLL	activebook
activebook	SETL	{FALSE}		; NOT an Active Book

		GBLL	heval
heval		SETL	{TRUE}		; this is a HEVAL version

		GBLL	hydra
hydra		SETL	{FALSE}		; do we provide HYDRA initialisation?

        ; ---------------------------------------------------------------------
        LNK     loexec.s
