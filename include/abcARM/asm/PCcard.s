	TTL	Created by "genhdr"	> PCcard/s

old_opt	SETA	{OPT}
	OPT	(opt_off)

; -- from "PCcard.h" -----------------------
; CISTPL_CODES
CISTPL_NULL           * &00000000
CISTPL_DEVICE         * &00000001
CISTPL_CHECKSUM       * &00000010
CISTPL_LONGLINK_A     * &00000011
CISTPL_LONGLINK_C     * &00000012
CISTPL_LINKTARGET     * &00000013
CISTPL_NO_LINK        * &00000014
CISTPL_VERS_1         * &00000015
CISTPL_ALTSTR         * &00000016
CISTPL_DEVICE_A       * &00000017
CISTPL_JEDEC_C        * &00000018
CISTPL_JEDEC_A        * &00000019
CISTPL_VERS_2         * &00000040
CISTPL_FORMAT         * &00000041
CISTPL_GEOMETRY       * &00000042
CISTPL_BYTEORDER      * &00000043
CISTPL_DATE           * &00000044
CISTPL_BATTERY        * &00000045
CISTPL_ORG            * &00000046
CISTPL_END            * &000000FF

; link TAG identifier
TPLTG_TAG_ID          * &00534943

; level 1 information
L1_format_major       * &00000004
L1_format_minor       * &00000000

; device speed and type information
DS_Reserved0          * &00000000
DS_250ns              * &00000001
DS_200ns              * &00000002
DS_150ns              * &00000003
DS_100ns              * &00000004
DS_Reserved5          * &00000005
DS_Reserved6          * &00000006
DS_Extend             * &00000007
DT_Null               * &00000000
DT_MASK_ROM           * &00000001
DT_OTPROM             * &00000002
DT_EPROM              * &00000003
DT_EEPROM             * &00000004
DT_FLASH              * &00000005
DT_SRAM               * &00000006
DT_DRAM               * &00000007
DT_IO                 * &0000000D
DT_EXTEND             * &0000000E
DT_Reserved           * &0000000F
DeviceSpeed_mask      * &00000007
DeviceSpeed_shift     * &00000000
WPS_mask              * &00000008
WPS_shift             * &00000003
DeviceType_mask       * &0000000F
DeviceType_shift      * &00000004

; extended device speed and type information
DS_MT_Reserved        * &00000000
DS_MT_1p0             * &00000001
DS_MT_1p2             * &00000002
DS_MT_1p3             * &00000003
DS_MT_1p5             * &00000004
DS_MT_2p0             * &00000005
DS_MT_2p5             * &00000006
DS_MT_3p0             * &00000007
DS_MT_3p5             * &00000008
DS_MT_4p0             * &00000009
DS_MT_4p5             * &0000000A
DS_MT_5p0             * &0000000B
DS_MT_5p5             * &0000000C
DS_MT_6p0             * &0000000D
DS_MT_7p0             * &0000000E
DS_MT_8p0             * &0000000F
DS_EX_1ns             * &00000000
DS_EX_10ns            * &00000001
DS_EX_100ns           * &00000002
DS_EX_1us             * &00000003
DS_EX_10us            * &00000004
DS_EX_100us           * &00000005
DS_EX_1ms             * &00000006
DS_EX_10ms            * &00000007
DSExponent_mask       * &00000007
DSExponent_shift      * &00000000
DSMantissa_mask       * &00000078
DSMantissa_shift      * &00000003
SpeedExtend_shift     * &00000007
SpeedExtend_bit       * &00000080
TypeExtend_shift      * &00000007
TypeExtend_bit        * &00000080

; Device size information
SizeExponent_mask     * &00000007
SizeExponent_shift    * &00000000
SizeMantissa_mask     * &000000F8
SizeMantissa_shift    * &00000003

; structures
TPL_CODE              * &00000000
TPL_LINK              * &00000001
; the following are offsets from after TPL_LINK
; longlink
TPLL_ADDR             * &00000000
; link target
TFLTG_TAG             * &00000000
; checksum
TPLCKS_ADDR           * &00000000
TPLCKS_LEN            * &00000002
TPLCKS_CS             * &00000004
; level 1 information
TPLLV1_MAJOR          * &00000000
TPLLV1_MINOR          * &00000001
TPLLV1_INFO           * &00000002
; alternative strings
TPL_ALTSTR_ESC        * &00000000
; JEDEC information
manufacturerID       * &00000000
deviceID             * &00000001
; level 2 information
TPLLV2_VERS          * &00000000
TPLLV2_COMPLY        * &00000001
TPLLV2_DINDEX        * &00000002
TPLLV2_RSV           * &00000004
TPLLV2_VSPEC         * &00000006
TPLLV2_NHDR          * &00000008
TPLLV2_OEM           * &00000009

	OPT	(old_opt)
	END
