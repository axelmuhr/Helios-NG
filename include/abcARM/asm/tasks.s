        SUBT    Helios Kernel task structure definitions        > tasks/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900420  JGSmith
        ;
        ; Task structure definitions
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (structs_s)     ; ensure "structs.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including tasks.s"
        GBLL    tasks_s
tasks_s SETL    {TRUE}

        ; ---------------------------------------------------------------------

        struct          "Task"
        struct_struct   "Node","Node"           ; queueing node
        struct_word     "Program"               ; pointer to program structure
        struct_struct   "Pool","MemPool"        ; task private memory pool
        struct_word     "Port"                  ; initial message port
        struct_word     "Parent"                ; parent's message port
        struct_word     "IOCPort"               ; tasks IOC port
        struct_word     "Flags"                 ; task control flags
        struct_word     "ExceptCode"            ; exception routine
        struct_word     "ExceptData"            ; data for same
        struct_word     "HeapBase"              ; base of task heap area
        struct_word     "ModTab"                ; module table pointer
        struct_word     "TaskEntry"             ; procman control structure
        struct_end

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF tasks.s
