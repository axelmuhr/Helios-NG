	TTL	Created by "genhdr"	> ROMitems/s

old_opt	SETA	{OPT}
	OPT	(opt_off)


; ITEM structure manifests
ITEMMagic             * &6D657469
defaultITEMaccess     * &01010101
ITEMhdrROM            * &00000001
ITEMhdrRAM            * &00000002
ITEMhdrBRANCH         * &00000100
ITEMROMend            * &00000008
ITEMRAMend            * &00000014

; ITEM structures
ITEMID                * &00000000
ITEMLength            * &00000004
OBJECTOffset          * &00000008
OBJECTLength          * &0000000C
ITEMAccess            * &00000010
ITEMDate              * &00000014
ITEMExtensions        * &0000001C
ITEMNameLength        * &00000020
ITEMName              * &00000021
OBJECTInit            * &00000000
ITEMVersion           * &00000004
sizeof_ROMITEMstruct  * &00000008
ITEMCheck             * &00000000
ITEMHdrSeq1           * &00000003
OBJECTUsed            * &00000004
OBJECTSize            * &00000007
OBJECTRef             * &00000008
ITEMHdrSeq2           * &0000000B
OBJECTRefLast         * &0000000C
ITEMSpare1            * &0000000F
ITEMNumber            * &00000010
ITEMRAMhdrend         * &0000000C

	OPT	(old_opt)
	END
