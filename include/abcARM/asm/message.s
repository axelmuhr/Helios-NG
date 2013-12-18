        SUBT    Helios Kernel message and control structures    > message/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900420  JGSmith
        ;
        ; Message and message control block structures
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (structs_s)     ; ensure "structs.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including message.s"
                GBLL    message_s
message_s       SETL    {TRUE}

        ; ---------------------------------------------------------------------
        ; Message header

        struct          "MsgHdr"
        struct_byte     "DataSizeLo"    ; 16 bit data size
        struct_byte     "DataSizeHi"
        struct_byte     "ContSize"      ; control vector size
        struct_byte     "Flags"         ; flag byte
        struct_word     "Dest"          ; destination port descriptor
        struct_word     "Reply"         ; reply port descriptor
        struct_word     "FnRc"          ; function/return code
        struct_end

        ; ---------------------------------------------------------------------

MsgHdr_DataSize_mask    *       &FFFF   ; mask for data size field

MsgHdr_Flags_nothdr     *       &80     ; used by kernel
MsgHdr_Flags_preserve   *       &40     ; preserve destination route
MsgHdr_Flags_exception  *       &20     ; exception message type
MsgHdr_Flags_uselink    *       &10     ; reveive data from link
MsgHdr_Flags_link       *       &0F     ; link id

Max_Cont_size           *       16      ; maximum size of control vector
Max_Data_size           *       1024    ; maximum size of data vector

        ; ---------------------------------------------------------------------
        ; Message Control Block
        ; note that the first 4 words are identical to a MsgHdr

        struct          "MCB"
        struct_byte     "DataSizeLo"    ; 16 bit data size
        struct_byte     "DataSizeHi"
        struct_byte     "ContSize"      ; control vector size
        struct_byte     "Flags"         ; flag byte
        struct_word     "Dest"          ; destination port descriptor
        struct_word     "Reply"         ; reply port descriptor
        struct_word     "FnRc"          ; function/return code

        struct_word     "Timeout"       ; message timeout
        struct_word     "Control"       ; pointer to control buffer
        struct_word     "Data"          ; pointer to data buffer
        struct_end

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF message.s
