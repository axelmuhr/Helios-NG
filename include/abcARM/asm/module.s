        SUBT    Helios Kernel module definition macros          > module/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900105  JGSmith
        ; history:      900815  JGSmith Provide SMT and non-SMT support
        ;               900529  JGSmith Update to new "hobjasm" style.
        ;
        ; This file provides the Helios Module definition macros. The MACROs
        ; in this file will generate standard object modules, library object
        ; modules and library definition files.
        ;
        ; Certain of these MACROs require the logical variables "make_def"
        ; and "make_SMT" to be defined:
        ;
        ;       make_def TRUE   construct a library definition file
        ;       make_def FALSE  construct a library or module object file
        ;
        ;       make_SMT TRUE   construct SMT-style modules
        ;       make_SMT FALSE  construct old-style modules
        ;
        ; Notes: on the construction of standard modules and libraries.
        ;        Stub functions are automatically created for all symbols
        ;        that are IMPORTed. This is the standard method of referencing
        ;        and calling external functions. Libraries are standard modules
        ;        that are concatenated by the linker. Normally undefined
        ;        labels will generate assembly errors, however, the LIB
        ;        directive can be used to suppress these errors, and generate
        ;        linker patches that will reference the symbols in a PC
        ;        relative manner. Note: This only occurs for branch
        ;        instructions.
        ;
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (arm_s)         ; ensure "arm.s" is included
        ASSERT  (structs_s)     ; ensure "structs.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including module.s"
                GBLL    module_s
module_s        SETL    {TRUE}

        ; ---------------------------------------------------------------------

        [       (make_SMT)
        !       0,"Making SMT-style modules"
        |
        !       0,"Making old-style modules"
        ]       ; EOF (make_SMT)

        ; ---------------------------------------------------------------------

        struct          "Module"
        struct_word     "Type"     ; module type = T_Module
        struct_word     "Size"     ; size of module in bytes
        struct_vec      32,"Name"  ; module name
        struct_word     "Id"       ; module table index
        struct_word     "Version"  ; version number of this module
        struct_word     "MaxData"  ; highest data offset (>= 0)
        struct_word     "Init"     ; root of init chain
        [       (make_SMT)
        struct_word     "MaxCodeP" ; highest code offset (>= 0)
        ]       ; EOF (make_SMT)
        struct_end

        ; ---------------------------------------------------------------------

        struct          "ResRef"
        struct_word     "Type"     ; T.ResRef
        struct_word     "Size"     ; ResRef_sizeof
        struct_vec      32,"Name"  ; name of module required
        struct_word     "Id"       ; module table index
        struct_word     "Version"  ; version number of module required
        struct_word     "Module"   ; pointer to module, installed by loader
        struct_end

        ; ---------------------------------------------------------------------

T_Program       *       &60F060F0
T_Module        *       &60F160F1
T_ResRef        *       &60F260F2
T_Proc          *       &60F360F3
T_Code          *       &60F460F4
T_Stack         *       &60F560F5
T_Static        *       &60F660F6
T_ProcInfo      *       &60F760F7
T_Device        *       &60F860F8

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Generate a module header. Note: This MACRO calculates the module
        ; size from the information provided by the corresponding "EndModule"
        ; MACRO. This is so that multiple module images can be concatenated
        ; by the linker (armlink). A special MACRO "StartLibrary" exists
        ; so that the "EndModule" can appear in a seperate assembly.

        MACRO
$label  StartModule     $name,$id,$version
        MODULE          $id
$label
        ALIGN
ModStart
        &               T_Module
        &               (ModEnd - ModStart)     ; assemble-time module length
        =               "$name",Null
        %               (32 - ((:LEN: "$name") + 1))
        MODNUM                          ; module table number
        &               $version
        OFFSET          MaxData         ; size of our static data
        INIT
        [       (make_SMT)
        OFFSET          MaxCodeP        ; size of the code pointer area
        ]       ; EOF (make_SMT)
ModHdrEnd
	ASSERT	((ModHdrEnd - ModStart) = Module_sizeof)
        MEND

        ; ---------------------------------------------------------------------

        MACRO
$label  ResRef  $name,$id,$version
$label
        ALIGN
        MODULE          $id
        &               T_ResRef
        &               ResRef_sizeof
        =               "$name",Null
        %               (32 - ((:LEN: "$name") + 1))
        MODNUM
        &               $version
        &               &00000000
        MEND

        ; ---------------------------------------------------------------------

        MACRO
$label  codesym $name
$label
        ALIGN
        &       T_Code
        LABEL   $name
        =       "$name",Null
        MEND

        ; ---------------------------------------------------------------------

        MACRO
$label  stacksym        $name,$offset
$label
        ALIGN
        &       T_Stack
        LABEL   $name
        =       "$name",Null
        MEND

        ; ---------------------------------------------------------------------

        MACRO
$label  staticsym       $name
$label
        ALIGN
        &       T_Static
        LABEL   $name
        =       "$name",Null
        MEND

        ; ---------------------------------------------------------------------

                GBLS    res_name
                GBLS    res_slot
                GBLA    res_version
res_name        SETS    "noname"                ; default library name
res_slot        SETS    "undef"                 ; default library slot
res_version     SETA    1                       ; default library version

        MACRO
$label  Resident        $name,$slot,$version
$label
        [       (make_def)
        !       0,"Making library definition file"
        |
        !       0,"Making library declaration file"
        ]       ; (make_def)
        [       ("$name" <> "")
res_name        SETS    "$name"
        ]
        [       ("$slot" <> "")
res_slot        SETS    "$slot"
        ]
        [       ("$version" <> "")
res_version     SETA    $version
        ]
        MEND

        ; ---------------------------------------------------------------------
        ; Generate a reference to specified library. Note: with the current
        ; assembler, this will generate a STUB to the given library name.
        ; However, this does ensure that the relevant ".def" file is linked
        ; with this object.
        MACRO
$label  uses    $libname
$label
        [       {TRUE}
        ; should be:
        ; LABREF        $vertbar.$libname.$dotchar.library$vertbar
	; The vertical bars are required to get around the "hobjasm"
	; limitation of not allowing full-stops in label names (which
	; the Helios library naming relies on).
        |
        IMPORT  $libname._library
        ]
        MEND

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Module static data definition and initialisation.

        GBLL    static_active
static_active   SETL    {FALSE}                 ; static area definition active

        ; declare a new module static area
        MACRO
$label  static
$label
        ASSERT  (:LNOT: static_active)          ; check not within definition
static_active   SETL    {TRUE}
        ALIGN
        INIT

        STMFD   sp!,{r4,lk}     ; extra work register and return address

        ; Start of the initialiasation code. We may need some more code here.
        ; "dp" register holds module table base pointer.
        ; "a1"/"r0" holds the initialisation parameter:
        ;       = 0     Initialise static data area table
        ;       = 1     Initialise static data area table
        ;       = 2     Initialise function pointer (code) table
        ; The exact code used depends also on wether we are generating
        ; old-style modules or SMT modules. When generating SMT modules
        ; the data index is at 0, the function pointers are at 4.
        LDR     r2,[dp,#{MODOFF}]       ; r2 points to our static data
        [       (make_SMT)
        LDR     r4,[dp,#{MODOFF} + 4]   ; for function elements
        ]       ; EOF (make_SMT)
        MEND

        ; declare a library static area
        MACRO
$label  lib_static
        ASSERT  (:LNOT: static_active)  ; check not within definition
static_active   SETL    {TRUE}
        MODULE          $res_slot
$label
        LIB                             ; we are defining a library
        ALIGN
$vertbar.$res_name.$dotchar.library$vertbar	EXTERN	; export the name
        [       (make_def)
        ; create "ResRef" module to resident library
        &               T_ResRef
        &               ResRef_sizeof
        =               "$res_name",&00
        %               (32 - ((:LEN: "$res_name") + 1))
        MODNUM
        &               $res_version
        &               &00000000
        |
ModStart
        &               T_Module
        IMSIZE
        =               "$res_name",&00
        %               (32 - ((:LEN: "$res_name") + 1))
        MODNUM
        &               $res_version
        OFFSET          MaxData
        INIT
        [       (make_SMT)
        OFFSET          MaxCodeP
        ]       ; EOF (make_SMT)
        ALIGN
        INIT

        STMFD   sp!,{r4,lk}     ; extra work register and return address

        ; Start of the initialisation code. We may need some more code here.
        ; "dp" register holds module table base pointer.
        ; "a1"/"r0" holds the initialisation parameter:
        ;       = 0     Initialise static data area table
        ;       = 1     Initialise static data area table
        ;       = 2     Initialise function pointer (code) table
        LDR     r2,[dp,#{MODOFF}]       ; for data elements
        [       (make_SMT)
        LDR     r4,[dp,#{MODOFF} + 4]   ; for function elements
        ]       ; EOF (make_SMT)
        ; The initialisation code gets placed after this point.
        ]       ; (make_def)
        MEND

        ; ---------------------------------------------------------------------

        MACRO
$label  static_end				; end of module static area
$label
        ASSERT  (static_active)                 ; check within definition
        [       (:LNOT: make_def)
        LDMFD   sp!,{r4,pc}^                    ; exit the initialisation code
        LTORG
        ]       ; (:LNOT: make_def)
static_active   SETL    {FALSE}
        MEND

        MACRO
$label  lib_static_end				; end of library static area
$label
        ASSERT  (static_active)                 ; check within definition
        [       (:LNOT: make_def)
        LDMFD   sp!,{r4,pc}^                    ; exit the initialisation code
        LTORG
        ]       ; (:LNOT: make_def)
static_active   SETL    {FALSE}
	DATA	MaxData,0			; static data area end marker
	[	(make_SMT)
	CODE	MaxCodeP			; function ptr end marker
	]	; (make_SMT)
        MEND

        ; ---------------------------------------------------------------------
        ; The following MACROs can be used for all module generation cases
	; ---------------------------------------------------------------------
        ; Local object MACROs
        MACRO
$label  static_func     $name
$label
        [       (make_SMT)
        CODE    $name
        |
        DATA    $name,4
        ]       ; EOF (make_SMT)
        [       (:LNOT: make_def)
        ; r2 points to static data area
	; r4 points to function pointer area
        LDR     ip,[pc,#&04]
        ADD     ip,pc,ip
        B       passaddr_$name
addressof_$name
        LABEL   $name
passaddr_$name
	[	(make_SMT)
	STR	ip,[r4,#:OFFSET: $name]
	|
        STR     ip,[r2,#:OFFSET: $name]
	]
        ]       ; (:LNOT: make_def)
        MEND

        MACRO
$label  static_word     $name
$label
        DATA    $name,4
        MEND

        ; define structure
        MACRO
$label  static_struct   $type,$name
        DATA    $name,$type._sizeof
        MEND

        MACRO
$label  static_vec      $size,$name
$label
        DATA            $name,$size
        MEND

        ; ---------------------------------------------------------------------
        ; External (EXPORTed) object MACROs
        MACRO
$label  static_extern_func      $name
$label
        [       (make_SMT)
        CODE    $name
        |
        DATA    $name,4
        ]       ; EOF (make_SMT)
        EXPORT  $name
        [       (:LNOT: make_def)
        ; r2 points to static data area
	; r4 points to function pointer area
        LDR     ip,[pc,#&04]
        ADD     ip,pc,ip
        B       passaddr_$name
addressof_$name
        LABEL   $name
passaddr_$name
        [       (make_SMT)
        STR     ip,[r4,#:OFFSET: $name]
        |
        STR     ip,[r2,#:OFFSET: $name]
        ]       ; (make_SMT)
        ]       ; (:LNOT: make_def)
        MEND

        MACRO
$label  static_extern_word      $name
$label
        DATA    $name,4
        EXPORT  $name
        MEND

        ; define structure
        MACRO
$label  static_extern_struct    $type,$name
        DATA    $name,$type._sizeof
        EXPORT  $name
        MEND

        MACRO
$label  static_extern_vec       $size,$name
$label
        DATA            $name,$size
        EXPORT          $name
        MEND

        ; ---------------------------------------------------------------------
        ; Object initialisation MACROs

        MACRO
$label  static_initptr  $name
$label
        [       (:LNOT: make_def)
        ; r2 points to static data area
	; r4 points to function pointer area
        LDR     ip,[pc,#&04]
        ADD     ip,pc,ip
        B       passDaddr_$name
addressofD_$name
        LABEL   $name
passDaddr_$name
        [       (make_SMT)
        STR     ip,[r4,#:OFFSET: $name]
        |
        STR     ip,[r2,#:OFFSET: $name]
        ]       ; (make_SMT)
        ]       ; (:LNOT: make_def)
        MEND

        ; initialise a word (32bit value)
        MACRO
$label  static_initword $name,$value
$label
        [       (:LNOT: make_def)
        LDR     ip,=$value      ; load from constant at LTORG
        STR     ip,[r2,#:OFFSET: $name]
        ]       ; (:LNOT: make_def)
        MEND

        ; copy a table from code area to data area
        MACRO
$label  static_inittab  $name,$size
        [       (:LNOT: make_def)
        ; NOTE: data must be word aligned and word multiple sized
        ; get address of source into ip
        LDR     ip,[pc,#&04]
        ADD     ip,pc,ip
        B       passTaddr_$name
addressofT_$name
        LABEL   $name
passTaddr_$name
        ; get address of target into r3
        ADD     r3,r2,#:OFFSET: $name   ; r2 points at our static area
        MOV     r1,#$size
inittab_loop_$name
        ; copy $size information
        LDR     r0,[ip],#&04
        STR     r0,[r3],#&04
        SUBS    r1,r1,#&04
        BGT     inittab_loop_$name
        ]       ; (:LNOT: make_def)
        MEND

        ; init a table of pointers to strings
        MACRO
$label  static_initptrtab       $name,$items,$stride
        [       (:LNOT: make_def)
        ; get address of source into ip
        ; r2 points to static data area
        LDR     ip,[pc,#&04]
        ADD     ip,pc,ip
        B       passPaddr_$name
addressofP_$name
        LABEL   $name
passPaddr_$name
        ; get address of target into r3
        ADD     r3,r2,#:OFFSET: $name   ; r2 points at our static area
        ; hopefully addressability works out ok
        ; else use LDR r1,=$items -- code const in pool
        MOV     r1,#$items

initptrtab_loop_$name
        ; put ptr to source in target
        STR     ip,[r3],#&04    ; post inc to next
        ADD     ip,ip,#$stride  ; add stride to source
        SUBS    r1,r1,#&01
        BGT     initptrtab_loop_$name
        ]       ; (:LNOT: make_def)
        MEND

        ; Introduce some assembly directly into the static area initialisation
        ; This is just a header for marking code we are including in the
        ; static data area initialisation sequence.
        MACRO
$label  static_code
$label
        MEND

        ; ---------------------------------------------------------------------

        MACRO
$label  LibData
$label
        [       (make_def)
        ; no data
        |
        ; Static initialisation data, plus any code which may be added in
        ; the form of procedures, follows here.
        ]       ; (make_def)
        MEND

        ; ---------------------------------------------------------------------
        ; Terminate an object module cleanly.

        MACRO
$label  EndModule
$label
        ASSERT  (:LNOT: static_active) ; check NOT within static definitions
        DATA    MaxData,0       ; static data area end marked (NULL length)
        [       (make_SMT)
        CODE    MaxCodeP        ; function pointer area end marker (4bytes)
        ]       ; EOF (make_SMT)
        ALIGN                   ; force end to word boundary
        LTORG                   ; and ensure literals are included in the size
ModEnd
        MEND


        MACRO
$label  EndLibrary
$label
        ASSERT  (:LNOT: static_active) ; check NOT within static definitions
        ALIGN                   ; force end to word boundary
        LTORG                   ; and ensure literals are included in the size
ModEnd
        MEND

        ; ---------------------------------------------------------------------
        ; Provide a PCS conformant (C style) function header in the code

                GBLA    NprocsymN
NprocsymN       SETA    1

        MACRO
$name   procsym
        ALIGN
        &       T_Proc
        LABEL   $name
Xfoo$NprocsymN
        =       "$name",Null
        ALIGN
        LABEL   Xfoo$NprocsymN          ; PCS conformant
NprocsymN       SETA    (NprocsymN + 1)
$name
        MEND

        ; ---------------------------------------------------------------------
        !       0,"**BODGE** SADR, SADRM and SADRL MACROs"
        ; These will eventually be provided as direct pseudo-ops by the
        ; assembler. They should be used when the address of a static
        ; data area symbol is required.

        MACRO
$label  SADR    $reg,$name
$label
        ; load the address of the static data area location "$label"
        LDR     $reg,[dp,#:MODOFF: $name]
        ADD     $reg,$reg,#:OFFSET: $name
        MEND

        MACRO
$label  SADRM   $reg,$name
        ; load the address of the static data area location "$label"
        LDR     $reg,[dp,#:MODOFF: $name]
        ADD     $reg,$reg,#:LSBOFF: $name
        ADD     $reg,$reg,#:MSBOFF: $name
        MEND    

        MACRO
$label  SADRL   $reg,$name
$label
        ; load the address of the static data area location "$label"
        LDR     $reg,[dp,#:MODOFF: $name]
        ADD     $reg,$reg,#:LSBOFF: $name
        ADD     $reg,$reg,#:MIDOFF: $name
        ADD     $reg,$reg,#:MSBOFF: $name
        MEND

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF module.s
