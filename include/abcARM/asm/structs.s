        SUBT    Helios Kernel data structure definition macros  > structs/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900105  JGSmith
        ;
        ; This file defines the MACROs used to build data structures.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (arm_s)         ; ensure "arm.s" is included
        ASSERT  (basic_s)       ; ensure "basic.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including structs.s"
                GBLL    structs_s
structs_s       SETL    {TRUE}

        ; ---------------------------------------------------------------------
        ; Structure definition. A structure can contain fields of the
        ; following types:
        ; struct_word   4bytes                                  word-aligned
        ; struct_aptr   4bytes                                  word-aligned
        ; struct_struct foo_sizeof      (size of structure foo) word-aligned
        ; struct_vec    nbytes                                  word-aligned
        ; struct_byte   1bytes                                  byte-aligned
        ; struct_null   0bytes (i.e. same as the next entry)    byte-aligned
        ;
        ; Each data structure entry has an expansion that contains the offset
        ; from the start of the structure.
        ;
        ; Every data structure has as its last entry an entry called:
        ;       <struct name>_sizeof
        ; which expands to the structure size.
        ;
        ; NOTE: The struct_struct MACRO currently only defines the space
        ;       occupied by the named structure. It does not define
        ;       sub_elements with the structure field names.

                GBLL    dstruct_active
                GBLA    dstruct_offset
                GBLS    dstruct_name

dstruct_active  SETL    {FALSE}                 ; structure definition active
dstruct_offset  SETA    0                       ; current structure index
dstruct_name    SETS    "<Undefined>"           ; current structure name

        ; declare a new structure
        MACRO
        struct          $name
        ASSERT  (:LNOT: dstruct_active)         ; check not within definition
dstruct_active  SETL    {TRUE}
dstruct_offset  SETA    0
dstruct_name    SETS    "$name"
        MEND

        ; terminate the structure (defining the "sizeof" element)
        MACRO
        struct_end
        ASSERT  (dstruct_active)                ; check within definition
$dstruct_name._sizeof   *       dstruct_offset
dstruct_active  SETL    {FALSE}
        MEND

        MACRO
        struct_word     $element
        ASSERT  (dstruct_active)                ; check within definition
dstruct_offset  SETA    (((dstruct_offset + 3) / 4) * 4)
$dstruct_name._$element *       dstruct_offset
dstruct_offset  SETA    (dstruct_offset + 4)
        MEND

        MACRO
        struct_aptr     $element
        ASSERT  (dstruct_active)                ; check within definition
dstruct_offset  SETA    (((dstruct_offset + 3) / 4) * 4)
$dstruct_name._$element *       dstruct_offset
dstruct_offset  SETA    (dstruct_offset + 4)
        MEND

        MACRO
        struct_struct   $name,$element
        ASSERT  (dstruct_active)                ; check within definition
dstruct_offset  SETA    (((dstruct_offset + 3) / 4) * 4)
$dstruct_name._$element *       dstruct_offset
dstruct_offset  SETA    (dstruct_offset + $name._sizeof)
        MEND

        MACRO
        struct_vec      $size,$element
        ASSERT  (dstruct_active)                ; check within definition
dstruct_offset  SETA    (((dstruct_offset + 3) / 4) * 4)
$dstruct_name._$element *       dstruct_offset
dstruct_offset  SETA    (dstruct_offset + $size)
        MEND

        MACRO
        struct_byte     $element
        ASSERT  (dstruct_active)                ; check within definition
$dstruct_name._$element *       dstruct_offset
dstruct_offset  SETA    (dstruct_offset + 1)
        MEND

        MACRO
        struct_null     $element
        ASSERT  (dstruct_active)                ; check within definition
$dstruct_name._$element *       dstruct_offset
        MEND

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF structs/s
