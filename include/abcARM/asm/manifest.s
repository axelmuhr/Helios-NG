	TTL	Created by "genhdr"	> manifest/s

old_opt	SETA	{OPT}
	OPT	(opt_off)

; -- from "config.h" -------------------------
; IVec indices
IVecISize             * 0
IVecKernel            * 1
IVecSysLib            * 2
IVecServLib           * 3
IVecUtil              * 4
IVecABClib            * 5
IVecPosix             * 6
IVecCLib              * 7
IVecFault             * 8
IVecFPLib             * 9
IVecPatchLib          * 10
IVecProcMan           * 11
IVecServers           * 12
IVecLoader            * 12
IVecKeyboard		* 13
IVecWindow            * 14
IVecRom               * 15
IVecRam               * 16
IVecNull              * 17
IVecHelios            * 18
; list terminator
IVecTotal             * 18
; -- from "manifest.h" -----------------------
; Timer related manifests
TickChunk             * &000003E8
TickSize              * &00002700
TicksPerSlice         * &00007500
; Process priority related manifests
log2_numpris          * 3
NumberPris            * 8
; ROM and CARD location manifests
loc_internal          * &00
loc_CARD1             * &01
loc_internalFlash     * &FF
loc_limit             * &01
; -- from "ABClib.h" -------------------------

;RESET information
ShellBootKey          * 0
EEPROM_ServerID       * 0
EEPROM_ServerIndexDefault  * &00
EEPROM_ServerIndexIOServer * &01
EEPROM_ServerIndexSysROM   * &0A
EEPROM_ServerIndexROMCard1 * &0B
EEPROM_ServerIndexROMCard2 * &0C
EEPROM_ServerIndexROMCard3 * &0D
EEPROM_ServerIndexROMCard4 * &0E
EEPROM_ServerIndexROMCard5 * &0F
EEPROM_ServerIndexROMCard6 * &10
EEPROM_ServerIndexROMCard7 * &11
EEPROM_ServerIndexROMCard8 * &12
EEPROM_ServerIndexROMCard9 * &13
EEPROM_ServerIndexSysRAM   * &14
EEPROM_ServerIndexRAMCard1 * &15
EEPROM_ServerIndexRAMCard2 * &16
EEPROM_ServerIndexRAMCard3 * &17
EEPROM_ServerIndexRAMCard4 * &18
EEPROM_ServerIndexRAMCard5 * &19
EEPROM_ServerIndexRAMCard6 * &1A
EEPROM_ServerIndexRAMCard7 * &1B
EEPROM_ServerIndexRAMCard8 * &1C
EEPROM_ServerIndexRAMCard9 * &1D
EEPROM_ServerIndexFS       * &1E
EEPROM_ServerIndexMSDOSa   * &64
EEPROM_ServerIndexMSDOSb   * &65
EEPROM_ServerIndexMSDOSc   * &66
EEPROM_ServerIndexMSDOSd   * &67
EEPROM_ServerIndexMSDOSe   * &68
EEPROM_ServerIndexMSDOSf   * &69
EEPROM_ServerIndexMSDOSg   * &6A
EEPROM_ServerIndexMSDOSh   * &6B
EEPROM_ServerIndexMSDOSi   * &6C
EEPROM_ServerIndexMSDOSj   * &6D
EEPROM_ServerIndexMSDOSk   * &6E
EEPROM_ServerIndexMSDOSl   * &6F
EEPROM_ServerIndexMSDOSm   * &70
EEPROM_ServerIndexMSDOSn   * &71
EEPROM_ServerIndexMSDOSo   * &72
EEPROM_ServerIndexMSDOSp   * &73
EEPROM_ServerIndexMSDOSq   * &74
EEPROM_ServerIndexMSDOSr   * &75
EEPROM_ServerIndexMSDOSs   * &76
EEPROM_ServerIndexMSDOSt   * &77
EEPROM_ServerIndexMSDOSu   * &78
EEPROM_ServerIndexMSDOSv   * &79
EEPROM_ServerIndexMSDOSw   * &7A
EEPROM_ServerIndexMSDOSx   * &7B
EEPROM_ServerIndexMSDOSy   * &7C
EEPROM_ServerIndexMSDOSz   * &7D
EEPROM_ServerIndexShell    * &FF

;CARD information
CardEvent_Node_Next   * &00000000
CardEvent_Node_Prev   * &00000004
CardEvent_Type        * &00000008
CardEvent_Handler     * &0000000C
CardEvent_data        * &00000010
CardEvent_ModTab      * &00000014
CARDerr_none          * &00000000
CARDerr_badslot       * &FFFFFFFF
CARDerr_nocard        * &FFFFFFFE
CARDerr_badformat     * &FFFFFFFD
CARDerr_badsum        * &FFFFFFFC
CARDerr_badarea       * &FFFFFFFB

	OPT	(old_opt)
	END
