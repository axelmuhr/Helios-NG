        SUBT    Helios Kernel port definition MACROs    > ports/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900420  JGSmith
        ;
        ; Port definition MACROs
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (structs_s)     ; ensure "structs.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including ports.s"
        GBLL    ports_s
ports_s SETL    {TRUE}

        ; ---------------------------------------------------------------------
        ; Port descriptor

        struct          "Port"
        struct_byte     "IndexLo"       ; 16 bit index into table
        struct_byte     "IndexHi"
        struct_byte     "Cycle"         ; table entry cycle
        struct_byte     "Flags"         ; flag byte
        struct_end

Port_Index_mask         *       &FFFF   ; mask for index

Port_Cycle_shift        *       16      ; shift for cycle field

Port_Flags_mb1          *       &80     ; so zero is never a valid port
Port_Flags_Tx           *       &40     ; 0=rx port, 1=tx port
Port_Flags_Remote       *       &20     ; set if port is surrogate

        ; ---------------------------------------------------------------------
        ; Port table entry

        struct          "PTE"
        struct_byte     "Type"          ; entry type
        struct_byte     "Cycle"         ; entry  cycle
        struct_byte     "Flags"         ; flag byte
        struct_byte     "Age"           ; GCTicks since last use
        struct_word     "Owner"         ; owning task or surrogate port
        struct_word     "TxId"          ; pointer to transmitter process
        struct_word     "RxId"          ; pointer to receiver process
        struct_end

        ; ---------------------------------------------------------------------
        ; Table entry types

T_Free                  *       0       ; unused slot
T_Local                 *       1       ; a local port
T_Surrogate             *       2       ; a surrogate port
T_Trail                 *       3       ; an intermediate route entry

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF ports.s
