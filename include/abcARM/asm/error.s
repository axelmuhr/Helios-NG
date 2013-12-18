        SUBT    Helios Kernel error codes                       > error/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900420  JGSmith
        ;
        ; Error codes
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including error.s"

        ; ---------------------------------------------------------------------

ErrBit          *       &80000000
SS_Kernel       *       &01000000

EC_Recover      *       ErrBit                  ; a retry might succeed
EC_Warn         *       (ErrBit + &20000000)    ; recover & retry
EC_Error        *       (ErrBit + &40000000)    ; client fatal
EC_Fatal        *       (ErrBit + &60000000)    ; system/server fatal

EG_Mask         *       &00FF0000               ; mask to isolate

EG_NoMemory     *       (&00010000)             ; memory allocation failure
EG_Create       *       (&00020000)             ; failed to create
EG_Delete       *       (&00030000)             ; failed to delete
EG_Protected    *       (&00040000)             ; object is protected
EG_Timeout      *       (&00050000)             ; timeout
EG_Unknown      *       (&00060000)             ; object not found
EG_FnCode       *       (&00070000)             ; unknown function code
EG_Name         *       (&00080000)             ; mal-formed name
EG_Invalid      *       (&00090000)             ; invalid/corrupt object
EG_InUse        *       (&000A0000)             ; object in use/locked
EG_Congested    *       (&000B0000)             ; server/route overloaded
EG_Broken       *       (&000D0000)             ; object broken in some way
EG_Exception    *       (&000E0000)             ; exception message

EO_Message      *       (&00008001)             ; error refers to a message
EO_Task         *       (&00008002)             ; error refers to a task
EO_Port         *       (&00008003)             ; error refers to a port
EO_Route        *       (&00008004)             ; error refers to a route
EO_Link         *       (&00008012)             ; error refers to a phys. link

EE_Kill         *       (&00000004)             ; kill exception code
EE_Abort        *       (&00000005)             ; abort exception code
EE_Interrupt    *       (&00000008)             ; console interrupt

        MACRO
$label  Error   $name,$code
Err_$name	*	$code
        MEND

        Error   Null,0
        Error   Timeout,(EC_Recover + SS_Kernel + EG_Timeout + EO_Message)
        Error   BadPort,(EC_Warn + SS_Kernel + EG_Invalid + EO_Port)
	;Error   Aborted,(EC_Error + SS_Kernel + EG_Aborted + EO_Port)
        Error   InUse,(EC_Recover + SS_Kernel + EG_InUse + EO_Port)
        Error   BadSurrogate,(EC_Warn + SS_Kernel + EG_Invalid + EO_Port)
        Error   BadRoute,(EC_Warn + SS_Kernel + EG_Invalid + EO_Route)
        Error   NoMemory,(EC_Warn + SS_Kernel + EG_NoMemory)
        Error   Congestion,(EC_Recover + SS_Kernel + EG_Congested + EO_Route)
        Error   Kill,(EC_Fatal + SS_Kernel + EG_Exception + EE_Kill)
        Error   Abort,(EC_Error + SS_Kernel + EG_Exception + EE_Abort)
        Error   NotReady,(EC_Recover + SS_Kernel + EG_Congested + EO_Port)
        Error   BadLink,(EC_Warn + SS_Kernel + EG_Invalid + EO_Link)

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF error/s
