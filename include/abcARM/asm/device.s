        SUBT    Helios Kernel device driver header file         > device/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900108  JGSmith
        ;
        ; This file defines the structures used to pass information to a
        ; Helios device driver.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (structs_s)     ; ensure "structs.s" is included
        ASSERT  (queue_s)       ; ensure "queue.s" is included
        ASSERT  (module_s)      ; ensure "module.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including device.s"
                GBLL         device_s
device_s        SETL         {TRUE}

        ; ---------------------------------------------------------------------

        struct          "Device"
        struct_word     "Type"          ; module type = T_Device
        struct_word     "Size"          ; size of the device in bytes
        struct_vec      32,"Name"       ; device name
        struct_word     "Id"            ; not used (compatability)
        struct_word     "Version"       ; version number of this device
        struct_word     "Open"          ; offset of open routine
        struct_end

        ; ---------------------------------------------------------------------

        struct          "DCB"
        struct_word     "Device"                ; pointer to "Device" struct
        struct_word     "Operate"               ; action entry point
        struct_struct   "List","Requests"       ; pending request queue
        struct_struct   "List","Replies"        ; completed request queue
        struct_end

        ; ---------------------------------------------------------------------

        MACRO
$label  Device  $name,$version
$label
        MODULE          -1              ; module number undefined
ModStart
        &               T_Device
        IMSIZE
        =               "$name",&00
        %               (32 - ((:LEN: "$name") + 1))
        &               &00000000       ; does not have a module number
        &               $version
        LABEL           DevOpen
        MEND

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF device/s
