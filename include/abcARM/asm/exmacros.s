        SUBT Helios Executive OBJASM macros                        > exmacros/s
        ;    Copyright (c) 1989, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ;
        ; Author:               James G Smith
        ;
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

old_opt SETA    {OPT}
        OPT     (opt_off)

        ; ---------------------------------------------------------------------

        !       0,"Including exmacros.s"
                GBLL    exmacros_s
exmacros_s      SETL    {TRUE}

        ; ---------------------------------------------------------------------
        ; -- ERROR construction -----------------------------------------------
        ; ---------------------------------------------------------------------

        ; The following MACRO constructs the error calling code
        MACRO
$label  ERROR   $number,$text
$label
        SWI     exec_GenerateError
        &       $number
        =       "$text",null
        ALIGN
        MEND

        ; ---------------------------------------------------------------------
        ; -- GENERAL MACROs ---------------------------------------------------
        ; ---------------------------------------------------------------------
        ; A "NOP" instruction.
        ; This can be removed completely in static ARM2s (since they perform
        ; processor mode changes immediately). For normal ARM2/ARM3s this
        ; instruction can be optimised to one that uses the least amount of
        ; power.
        MACRO
$label  NOP
$label  MOVNV   r0,r0
        MEND

        ; ---------------------------------------------------------------------
        ; FnHead
        ; Provide a C PCS style function name structure for the named routine.
        ; This is functionally identical to the MACRO "procsym" defined in
        ; "module.s", but without the "T_Proc" header (which seems to be
        ; unnecessary). For final release (and space saving) we can remove the
	; function name completely.

        MACRO
$label  FnHead
        ALIGN
        [       {TRUE}          ; set to {FALSE} to save code space
01
        =       "$label",&00
        ALIGN
02
        &       (&FF000000 + (%BT02 - %BT01))
        ]
$label
        MEND

        ; ---------------------------------------------------------------------
        ; bit
        ; This provides a shorthand method of describing individual bit
        ; positions.
        MACRO
$label  bit     $index
$label  *       (1 :SHL: $index)
        MEND

        ; ---------------------------------------------------------------------
	; Calculate the next-power-of-2 number above the value given.
	MACRO
$label	NPOW2	$value
	LCLA	newval
newval	SETA	1
	WHILE	(newval < $value)
newval	SETA	(newval :SHL: 1)
	WEND
$label	*	(newval)
	; Allow the user to see how much "wasted" space is being generated.
	!	0,"NPOW2: original &" :CC: (:STR: $value) :CC: " new &" :CC: (:STR: newval) :CC: " wasted &" :CC: (:STR: (newval - $value))
	MEND

        ; ---------------------------------------------------------------------
        ; -- SWI generation ---------------------------------------------------
        ; ---------------------------------------------------------------------
        ; defSWI
        ; Construct a SWI constant or a branch table entry

        ; Variable used to hold next unique SWI number
        GBLA    swi_N
swi_N   SETA    0

        MACRO
$label  defSWI  $name
$label
        [       (SWItable)
        B       code_$name
        |
$name   *       (swi_exec :OR: swi_N :OR: swi_os_helios)
swi_N   SETA    (swi_N + 1)
        ]
        MEND

        ; ---------------------------------------------------------------------
        ; -- FPE support ------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; MACROs required due to FPE source supporting older assemblers.

        MACRO
$label  ADDR    $reg,$dest,$cond
$label  ADR$cond.L $reg,$dest
        MEND

        MACRO
$label  BADDR   $reg,$dest,$cond
$label  ADR$cond.L $reg,$dest
        MEND

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; NOTE: The header "ROMitems.s" must be included before this MACRO
        ;       can be used.
        ; ---------------------------------------------------------------------
        ; ITEM descriptor building MACROs

        ; ROMITEM
        ;       $name     :     ASCII string
        ;       $initcode :     Label of initialisation code
        ;       $version  :     16bit BCD value (treated as "hh.ll")
        ;       $date     :     32bit seconds since 00:00:00 1st Jan 1970
        ;       $access   :     32bit access matrix value
        ;       $length   :     Use this value as the length of the object
        ;       $ext      :     Extra extension flags (over ITEMhdrROM)

        MACRO
$label  ROMITEM	$name,$initcode,$version,$date,$access,$length,$ext
$label
LocalITEMStart
        &       ITEMMagic                               ; magic number
        [       ("$length" <> "")
        &       ($length)                               ; total length
        |
        &       (LocalITEMEnd - LocalITEMStart)         ; total length
        ]
        [       ("$length" <> "")
        &       (LocalDataStart - LocalITEMStart)               ; data offset
        &       ($length - (LocalDataStart - LocalITEMStart))   ; data length
        |
        &       (LocalDataStart - LocalITEMStart)       ; data offset
        &       (LocalITEMEnd - LocalDataStart)         ; data length
        ]
        &       ($access)                               ; access matrix
        &       &00000000                               ; date lo
        [       ("$date" <> "")
        &       ($date)                                 ; date hi
        |
        &       &00000000                               ; date hi
        ]
        [       ("$ext" <> "")
        &       (ITEMhdrROM :OR: $ext)                  ; ROM ITEM plus flags
        |
        &       (ITEMhdrROM)                            ; ROM ITEM hdr follows
        ]
        =       (LocalITEMHeader - LocalITEMName)       ; length of "ITEMName"
LocalITEMName
        ASSERT  ((LocalITEMName - LocalITEMStart) = ITEMName)
        =       "$name",&00                             ; NULL terminated
        ALIGN                                           ; to next word boundary
LocalITEMHeader
        [       ("$initcode" <> "")
        &       ($initcode - LocalITEMStart)            ; offset to code
        |
        &       &00000000                               ; no initialisation
        ]
        [       ("$version" <> "")
        &       ($version :AND: &0000FFFF)              ; only the lo 16bits
        |
        &       &00000000                               ; BCD "0.00"
        ]
LocalDataStart  
        ; private ITEM information starts here
        MEND

        ; Generate a label that can be used to find the end of the ROM Item
        MACRO
$label  ITEMEnd
$label
LocalITEMEnd
        MEND

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF exmacros/s
