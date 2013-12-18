        SUBT    Helios Kernel semaphore structure definition    > sem/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900420  JGSmith
        ;
        ; Semaphore structure definition
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (structs_s)     ; ensure "structs.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including sem.s"
        GBLL    sem_s
sem_s   SETL    {TRUE}

        ; ---------------------------------------------------------------------

        struct          "Sem"
        struct_word     "Count"         ; semaphore counter
        struct_word     "Head"          ; head of process list
        struct_word     "Tail"          ; tail of process list
        struct_end

        ; ---------------------------------------------------------------------

        OPT     (opt_off)

        ; ---------------------------------------------------------------------
        END     ; EOF sem.s
