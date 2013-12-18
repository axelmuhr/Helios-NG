	SUBT Generic device driver end module			> modend/s
	;    Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom
	; ---------------------------------------------------------------------
	; The object code generated from this file should be the last file
	; linked when creating the device driver.
	; ---------------------------------------------------------------------

	GET	listopts.s	; assembly listing control directives
	GET	arm.s		; ARM processor description
	GET	basic.s		; default labels and variables
	GET	structs.s	; structure construction MACROs
	GET	module.s	; Helios object module construction MACROs

	; ---------------------------------------------------------------------

	EndModule		; terminate the module cleanly

	; ---------------------------------------------------------------------
	END

