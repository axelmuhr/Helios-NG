        SUBT    Helios Kernel general macro definitions         > basic/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900105  JGSmith
        ;
        ; This file contains general MACRO and MANIFEST definitions.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including basic.s"
        GBLL    basic_s
basic_s SETL    {TRUE}

        ; ---------------------------------------------------------------------

                GBLL    debugging
debugging       SETL    {TRUE}          ; set if we are debugging

                GBLL    addon
addon           SETL    {TRUE}          ; if assembling for the add-on board

                GBLL    HeliosArm
HeliosArm       SETL    {TRUE}          ; generating ARM version
                GBLL    helios_arm
helios_arm      SETL    HeliosArm       ; synonym for the above label

        ; ---------------------------------------------------------------------

Null    *       &00
null    *       Null
byte    *       1       ; 8bits
word    *       4       ; 32bits

        ; ---------------------------------------------------------------------

        ; Place a SCCS identity string into the source
        MACRO
$label  sccsid  $arg
$label
        =       "$arg",Null
        MEND

        ; ---------------------------------------------------------------------
        ; String (label) manipulation variables

        GBLS    vertbar
        GBLS    dotchar
vertbar SETS    "|"             ; Vertical bar
dotchar SETS    "."             ; fullstop

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF basic/s
