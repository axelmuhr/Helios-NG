	TTL  Prefix file for serial driver for ARM Helios	> fdcprefix.s
	SUBT Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom
	; ---------------------------------------------------------------------

	GET	listopts.s	; assembly listing control directives
	GET	arm.s		; ARM processor description
	GET	basic.s		; default labels and variables
	GET	structs.s	; structure construction MACROs
	GET	module.s	; Helios object module construction MACROs
	GET	queue.s		; Kernel queue data structures
	GET	device.s	; Kernel device driver definitions

	; ---------------------------------------------------------------------

	LIB	; bad choice of directive name (we are not really a library)

	; ---------------------------------------------------------------------

	Device	Serial.Device,1000	; name and version

	; ---------------------------------------------------------------------
	; all the code is in serialdev.c

	uses	Kernel
	uses	SysLib
	uses	Util
	uses	ABClib

	; ---------------------------------------------------------------------

	; functions from Kernel
	IMPORT	Signal
	IMPORT	Wait
	IMPORT	SchedulerDispatch
	IMPORT	Resume
	IMPORT	InitSemaphore
	IMPORT	SetEvent
	IMPORT	RemEvent
	IMPORT	IntsOff
	IMPORT	IntsOn
	IMPORT	__divide
	IMPORT	__uremainder
	IMPORT	_memcpy

	; functions from ABClib
	IMPORT	EnableIRQ
	IMPORT	DisableIRQ
	IMPORT	execSWI
	; debugging functions from ABClib
	IMPORT	Output
	IMPORT	WriteHex8
	IMPORT	NewLine

	; functions from Util
	IMPORT	IOdebug

	; functions from SysLib
	IMPORT	Malloc
	IMPORT	Free
	IMPORT	Delay
	IMPORT	GetInputSpeed
	IMPORT	SetInputSpeed
	IMPORT	GetOutputSpeed
	IMPORT	SetOutputSpeed
	IMPORT	AddAttribute
	IMPORT	IsAnAttribute

	; ---------------------------------------------------------------------
	END
