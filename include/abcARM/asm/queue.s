        SUBT    Helios Kernel queue data structures             > queue/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900108  JGSmith
        ;
        ; This file defines the Queue data structures and kernel jump table
        ; offsets.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (structs_s)     ; ensure "structs.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including queue.s"
        GBLL    queue_s
queue_s SETL    {TRUE}

        ; ---------------------------------------------------------------------
        ; List structure

        struct          "List"
        struct_word     "Head"          ; pointer to first node on list
        struct_word     "Earth"         ; always NULL
        struct_word     "Tail"          ; pointer to last node on list
        struct_end

        ; ---------------------------------------------------------------------

        struct          "Node"
        struct_word     "Next"          ; pointer to next node in list
        struct_word     "Prev"          ; pointer to previous node in list
        struct_end

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF queue.s
