
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

-- End of device.m

