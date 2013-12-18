        SUBT    Helios Kernel configuration structure definitions    > config/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900420  JGSmith
        ;
        ; This file defines the Configuration information structure.
        ; This is loaded by the booting processor into a well known
        ; memory location where the kernel can get at it.
        ; Normally this data will be derived from a configuration file
        ; on the booting processor.
        ; ---------------------------------------------------------------------
        ; This file relies on the "include" file "structs.s" being loaded

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including config.s"

        ; ---------------------------------------------------------------------
	; This structure should be kept in-step with the TRUE structure
	; defined in "config.h".

        struct          "Config"
        struct_word     "PortTabSize"   ; Number of slots in port table
        struct_word     "Incarnation"   ; what booter believes our incarn. is
        struct_word     "LoadBase"      ; address at which system was loaded
        struct_word     "ImageSize"     ; size of system image
        struct_word     "Date"          ; current system date
        struct_word     "FirstProg"     ; offset of initial program
        struct_word     "MemSize"       ; if > 0, first byte of unused mem
	struct_word	"Flags"		; see "root.h"
        struct_word     "Spare"         ; some spare bytes
        struct_word     "MyName"        ; full path name
        struct_word     "ParentName"    ; ditto
        struct_word     "NLinks"        ; number of links
        struct_word     "LinkConf0"     ; first of NLinks LinkConf structs
        struct_end

        ; ---------------------------------------------------------------------
        ; The LinkConf structures must be in the same order as the link
        ; channels in low memory.

        struct          "LinkConf"
        struct_byte     "Flags"         ; initial flags
        struct_byte     "Mode"          ; link mode
        struct_byte     "State"         ; initial state
        struct_byte     "Id"            ; link id
        struct_end

        ; Image vector

        struct          "IVec"
        struct_word     "Size"          ; size of load image in bytes
        struct_word     "Kernel"        ; RPTR to kernel module
        struct_word     "Syslib"        ; RPTR to system library module
        struct_word     "Servlib"       ; RPTR to server library module
        struct_word     "Util"          ; RPTR to Util library
        struct_word     "FpLib"         ; RPTR to Floating Point library
	struct_word	"Posix"		; RPTR to POSIX library
	struct_word	"CLib"		; RPTR to ANSI C library
	struct_word	"Fault"		; RPTR to Fault library
	struct_word	"ABClib"	; RPTR to ABC specific library
	struct_word	"PatchLib"	; RPTR to ABC patch library
        struct_word     "ProcMan"       ; RPTR to processor manager
        struct_word     "Tasks"         ; first task
        struct_end

        ;----------------------------------------------------------------------

        OPT     (old_opt)

        ;----------------------------------------------------------------------
        END     ; EOF config/s
