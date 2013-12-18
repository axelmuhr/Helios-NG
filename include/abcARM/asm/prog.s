        SUBT    Helios Kernel program structure         > prog/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900420  JGSmith
        ;
        ; Program structure
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (structs_s)     ; ensure "structs.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including prog.s"
        GBLL    prog_s
prog_s  SETL    {TRUE}

        ; ---------------------------------------------------------------------

        struct          "Program"
        struct_struct   "Module","Module"       ; start of module
        struct_word     "Stacksize"             ; size of initial stack
        struct_word     "Heapsize"              ; size of program heap
        struct_word     "Main"                  ; offset of main entry point
        struct_end

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF prog.s
