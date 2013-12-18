        SUBT    objasm OPT directive values                     > listopts/s
        ;       (c) 1989, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; listopts.s
        ; ----------
        ; Author:       JGSmith 900105
        ;
        ; This file should be included before including any other objasm
        ; "include" files. This file defines the OPT directive parameters
        ; that control listing when the "-print" command line option is
        ; specified.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        !       0,"Including listopts.s"
                GBLL         listopts_s
listopts_s      SETL         {TRUE}

        ; ---------------------------------------------------------------------
        ; Printing options

opt_on          *       (1 :SHL: 0)     ; listing on
opt_off         *       (1 :SHL: 1)     ; listing off
opt_ff          *       (1 :SHL: 2)     ; form-feed
opt_reset       *       (1 :SHL: 3)     ; reset line number to zero
opt_von         *       (1 :SHL: 4)     ; variable listing on
opt_voff        *       (1 :SHL: 5)     ; variable listing off
opt_mon         *       (1 :SHL: 6)     ; macro expansion on
opt_moff        *       (1 :SHL: 7)     ; macro expansion off
opt_mcon        *       (1 :SHL: 8)     ; macro calls on
opt_mcoff       *       (1 :SHL: 9)     ; macro calls off
opt_p1on        *       (1 :SHL: 10)    ; pass 1 listing on
opt_p1off       *       (1 :SHL: 11)    ; pass 1 listing off
opt_con         *       (1 :SHL: 12)    ; conditional directives on
opt_coff        *       (1 :SHL: 13)    ; conditional directives off

        OPT     (opt_on :OR: opt_voff :OR: opt_moff :OR: opt_mcon :OR: opt_p1off :OR: opt_coff)
        GBLA    old_opt
old_opt SETA    {OPT}

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        END     ; listopts.s
