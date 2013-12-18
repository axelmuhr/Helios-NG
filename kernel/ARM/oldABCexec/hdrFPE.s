        TTL     item based Helios Floating Point Emulator            > hdrFPE/s
        SUBT    (c) 1989, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; This file controls the switches and definitions necessary to make
        ; an item based FPE.
        ; ---------------------------------------------------------------------

                GBLS    fpendfile
fpendfile       SETS    "fpeend.s"

        ; ---------------------------------------------------------------------
        ; Ensure the header files support the FPE.
        ; This is a BODGE, since these conditionals need to be kept in step
        ; with those used to assemble the corresponding hi and lo execs

	; The "heval" and "hydra" variables are lo-Executive only.
	; They should not be used in these modules, but are required for
	; certain system header files.

		GBLL	activebook
activebook	SETL	{FALSE}		; NOT an Active Book
		GBLL	heval
heval		SETL	{FALSE}		; NOT a HEVAL
		GBLL	hydra
hydra		SETL	(heval)		; no HYDRA present

        ; ---------------------------------------------------------------------

        GET     listopts.s
        GET     fixes.s
        GET     basic.s
        GET     arm.s
        GET     exmacros.s
        GET     structs.s
        GET     exstruct.s
        GET     SWIinfo.s
        GET     ROMitems.s
        GET     hardABFP.s
        GET     manifest.s
	GET	timedate.s
	GET	microlink.s	; for "execwork.s"
        GET     execwork.s
	GET	queue.s
	GET	memory.s
	GET	task.s

        ; ---------------------------------------------------------------------
        LNK     fpehtop.s
