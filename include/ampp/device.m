--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- device.m								--
--                                                                      --
--	Device header							--
--                                                                      --
--      Author: NHG 28-July-87						--
--                                                                      --
--	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include device.m]
_def 'device.m_flag 1

include structs.m
include queue.m
include library.m

struct Device [
	word		Type		-- module type = T.Device
	word		Size		-- size of device in bytes
	vec	32	Name		-- device name
	word		Id		-- not used (compatability)
	word		Version		-- version number of this device
	word		Open		-- offset of open routine
]

struct DCB [
	word		Device		-- pointer to Device struct
	word		Operate		-- action entry point
	word		Close		-- close device routine
	word		Code		-- open code stream
]

_test _defp 'helios.TRAN [
	_defq 'Device['dev_name 'dev_version]
	[
		module -1
	.ModStart:
		word	#60f860f8
		word	.ModEnd-.ModStart
		blkb	31,"dev_name" byte 0
		word	0
		word	dev_version
		word	.DevOpen
	]
][
	_defq 'Device['dev_name 'dev_version]
	[
		module -1
		.ModStart:
		word	0x60f860f8
		word	modsize
		blkb	31,"dev_name" byte 0
		word	0
		word	dev_version
		word	labelref(.DevOpen)
	]
]


-- End of device.m
