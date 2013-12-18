        SUBT    Executive assembly control definition file      > fixes/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; started:      900214  JGSmith         (St.Valentine's day)
        ; history:      900214  Define global assembly control flags
        ; ---------------------------------------------------------------------

old_opt SETA    {OPT}
        OPT     (opt_off)

        ; ---------------------------------------------------------------------

                GBLL    generrIRQoff
generrIRQoff    SETL    {FALSE}         ; disable IRQs on GenerateError entry

                GBLL    sysyield
sysyield        SETL    {FALSE}		; always perform Yield in "System" exit

        ; ---------------------------------------------------------------------

		GBLL	memmap
memmap		SETL	{FALSE}		; memory map swapping support

        ; ---------------------------------------------------------------------

		GBLL	dbgsim
dbgsim		SETL	{TRUE}		; disassembler etc.

        ; ---------------------------------------------------------------------

		GBLL	fpmlink
fpmlink		SETL	{TRUE}		; ABFP micro-link on 2nd Inmos link

        ; ---------------------------------------------------------------------

		GBLL	hercmlink
hercmlink	SETL	{TRUE}		; Hercules (Heval/AB) microlink support

        ; ---------------------------------------------------------------------

	; set to {TRUE} to trap USR mode processes that de-reference NULL
		GBLL	page0trap
page0trap	SETL	{FALSE}

        ; ---------------------------------------------------------------------

		GBLL	haltmode	; halt the processor when entering IDLE

        ; ---------------------------------------------------------------------

		GBLL	monitor		; built-in monitor control
monitor		SETL	{TRUE}		; include startup monitor code

        ; ---------------------------------------------------------------------

		GBLL	shutdown	; hercules shutdown code included

        ; ---------------------------------------------------------------------

		GBLL	hercules	; hercules processor system
					; initialised elsewhere

        ; ---------------------------------------------------------------------

		GBLL	dynlcd		; dynamic LCD base addressing
					; initialised elsewhere

        ; ---------------------------------------------------------------------

	; This should be set to {TRUE} for public releases. NO DEBUGGING etc.
		GBLL	release
release		SETL	{FALSE}
		[	(release)
fpmlink		SETL	{TRUE}	; required for ABFP releases at the moment
dbgsim		SETL	{FALSE}	; waste of space in releases
		]

        ; ---------------------------------------------------------------------

		GBLL	kbdserv
kbdserv		SETL	{FALSE}		; use local keyboard server

        ; ---------------------------------------------------------------------

		GBLL	softbreak
softbreak	SETL	{FALSE}		; whether soft-break is acted upon

        ; ---------------------------------------------------------------------

		GBLL	newbreak
newbreak	SETL	{TRUE}		; new micro-link break protocol, now
					; directly linked to PowerFail.

        ; ---------------------------------------------------------------------

		GBLL	newpdcode	; new PowerDown code

        ; ---------------------------------------------------------------------

		GBLL	newsys
newsys		SETL	{TRUE}		; new System code in HiExecutive

        ; ---------------------------------------------------------------------

		GBLL	speedup
speedup		SETL	{TRUE}		; provide a slightly non-portable for
					; improving the performance of
					; FindExecRoot, GetRoot and GetSysBase.

        ; ---------------------------------------------------------------------

		GBLL	fix0001
fix0001		SETL	{TRUE}	; loss of "timeslice remaining" information bug

        ; ---------------------------------------------------------------------

        OPT     (old_opt)       

        ; ---------------------------------------------------------------------
        END     ; EOF fixes.s
