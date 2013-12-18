        SUBT    Helios Kernel root data structure               > root/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900420  JGSmith
        ;
        ; Root data structure
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (structs_s)     ; ensure "structs.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including root.s"
        GBLL    root_s
root_s  SETL    {TRUE}

        ; ---------------------------------------------------------------------

        !       0,"MemStart not defined for ARM version"
        !       0,"GetRoot not defined for ARM version"

        ; ---------------------------------------------------------------------

        struct          "Root"
        struct_word     "Flags"                 ; flag bits 
        struct_word     "PortTable"             ; pointer to port table
        struct_word     "PTSize"                ; size of port table in words
        struct_word     "PTFreeq"               ; port table free queue
        struct_word     "Links"                 ; pointer to link table
        struct_struct   "Pool","SysPool"        ; allocated system memory
        struct_word     "FreePool"              ; free memory list
        struct_word     "Incarnation"           ; our incarnation number
        struct_struct   "List","BufferPool"     ; pending delivery buffer pool
        struct_word     "BuffPoolSize"          ; no. of free slots in pool
        struct_word     "LoadAverage"           ; low pri load average
        struct_word     "Latency"               ; hi pri scheduling latency
        struct_word     "TraceVec"              ; pointer to trace vector
        struct_struct   "List","EventList"      ; list of event routines
        struct_word     "EventCount"            ; number of events seen
        struct_word     "Time"                  ; current system time
        struct_struct   "Pool","FastPool"       ; fast RAM pool
        struct_word     "MaxLatency"            ; maximum latency seen
        struct_struct   "Sem","UIDebugLock"     ; lock for all IOdebugs
        struct_word     "MachineType"           ; processor type code
        struct_vec      34,"Reserved"           ; bring up to 50 words
        struct_end

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF root.s
