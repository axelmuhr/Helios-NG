        SUBT    Helios Executive SWIs                   > SWIinfo/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; Provide a wrapper file for the "SWI.s" include file. This is
        ; required due to the dual nature of the "SWI.s" include file:
        ;       SWItable = TRUE         builds a jump table
        ;       SWItable = FALSE        generates SWI manifests
        ; ---------------------------------------------------------------------

        ASSERT  (exmacros_s)    ; ensure "exmacros.s" is included

        ; ---------------------------------------------------------------------

                GBLL    SWItable
SWItable        SETL    {FALSE}         ; just generate the SWI manifests

        ; ---------------------------------------------------------------------

        GET     SWI.s                   ; include the real definition file

        ; ---------------------------------------------------------------------
        END
