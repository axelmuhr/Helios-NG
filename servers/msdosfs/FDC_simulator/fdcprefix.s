	TTL  Prefix file for floppy disc driver for ARM Helios	> fdcprefix.s
	SUBT Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom
	; ---------------------------------------------------------------------

	GET	listopts.s	; assembley listing control directives
	GET	arm.s		; ARM processor description
	GET	basic.s		; default labels and variables
	GET	structs.s	; structure construction MACROs
	GET	module.s	; Helios object module construction MACROs
	GET	queue.s		; Kernel queue data structures
	GET	device.s	; Kernel device driver definitions

	; ---------------------------------------------------------------------

	LIB	; bad choice of directive name (we are not really a library)

	; ---------------------------------------------------------------------

	Device	Disc.Device,1000	; name and version

	; ---------------------------------------------------------------------
	; all the code is in fdcdev.c and fdcfiq.s

	uses	Kernel
	uses	SysLib
	uses	Util
	uses	ABClib

	; ---------------------------------------------------------------------

	; functions from Kernel
	IMPORT	Signal
	IMPORT	Wait
	IMPORT	HardenedSignal
	IMPORT	HardenedWait
	IMPORT	InitSemaphore
	IMPORT	SetEvent
	IMPORT	RemEvent
	IMPORT	__divide
	IMPORT	__divtest
	IMPORT	__uremainder
	IMPORT	__remainder
	IMPORT	_memcpy
	IMPORT	_cputime

	; functions from ABClib
	IMPORT	EnableIRQ
	IMPORT	DisableIRQ
	
	; functions from Util
	IMPORT	IOdebug
	IMPORT	Fork

	; functions from SysLib
	IMPORT	Malloc
	IMPORT	Free
	IMPORT	Delay

	; ---------------------------------------------------------------------
	END
