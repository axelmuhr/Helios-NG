        SUBT    Helios Kernel Event structure                   > event/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; --------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900420  JGSmith
        ;
        ; Event structure
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (structs_s)     ; ensure "structs.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including event.s"
                GBLL    event_s
event_s         SETL    {TRUE}

        ; ---------------------------------------------------------------------

        struct          "Event"
        struct_struct   "Node","Node"   ; link in event chain
	struct_word	"Vector"	; interrupt vector
        struct_word     "Pri"           ; priority in chain
        struct_word     "Code"          ; event routine
        struct_word     "Data"          ; data for this
        struct_word     "ModTab"        ; pointer to module table
        struct_end

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF event/s
