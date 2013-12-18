        SUBT    Helios Executive SWIs                              > SWIinfo
        ; ------------------------------------------------------------------
        ; Provide a wrapper file for the "SWI.s" include file. This is
        ; required due to the dual nature of the "SWI.s" include file:
        ;       SWItable = TRUE         builds a jump table
        ;       SWItable = FALSE        generates SWI manifests
        ; ------------------------------------------------------------------

        ASSERT  (exmacros_s)    ; ensure "exmacros.s" is included

        ; ------------------------------------------------------------------

                GBLL    SWItable
SWItable        SETL    {FALSE}         ; just generate the SWI manifests

                GBLL    SWIdisplay
SWIdisplay      SETL    {FALSE}         ; display SWI information as C defines

        ; ------------------------------------------------------------------

        GET     SWI.s                   ; include the real definition file

        ; ------------------------------------------------------------------
        END
