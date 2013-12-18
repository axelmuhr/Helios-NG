        SUBT    2-dimensional block copy                        > bytblt/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; void bytblt(
        ;             byte *source,     /* pointer to source array */
        ;             byte *dest,       /* pointer to destination array */
        ;             word  sourceix,   /* index into source array */
        ;             word  destix,     /* index into destination array */
        ;             word  sstride,    /* stride of source array */
        ;             word  dstride,    /* stride of destination array */
        ;             word  length,     /* number of rows to copy */
        ;             word  width,      /* width of each row */
        ;             word  op          /* operation code */
        ;            )
        ;
        ; "source" and "dest" point to 2-dimensional byte arrays with row
        ; widths of "sstride" and "dstride" respectively. "sourceix" and
        ;"destix" indicate the starting points within these arrays of a
        ; block of "length" by "width" bytes. "op" indicates how the data
        ; will be copied:
        ; "op" == 0 => move all bytes
        ;         1 => move non-zero bytes only
        ;         2 => move zero bytes only
        ;
        ;
        ;       <--- sstride --->               <---- dstride ---->
        ;       +---------------+               +-----------------+
        ;       |               |               |                 |
        ;       |               |               |    Y-------+    | <--+
        ;       |   X-------+   | <--+          |    |       |    |    |
        ;       |   |       |   |    |          |    |       |    |    |
        ;       |   |       |   |    |          |    |       |    |  length
        ;       |   |       |   |  length       |    |       |    |    |
        ;       |   |       |   |    |          |    |       |    |    |
        ;       |   |       |   |    |          |    +-------+    | <--+
        ;       |   +-------+   | <--+          |    <-width->    |
        ;       |   <-width->   |               |                 |
        ;       |               |               |                 |
        ;       +---------------+               +-----------------+
        ;
        ; Assuming point X = (4,3) in the source array
        ;      and point Y = (5,2) in the destination array
        ;
        ;       "sourceix" = (4 + (3 * sstride))
        ;       "destix"   = (5 + (2 * dstride))
        ;
        ; The algorithm used by the Transputer instruction is as follows:
        ;
        ;       sptr = source + sourceix ;
        ;       dptr = dest + destix ;
        ;       for (loop = 1; (loop < length); loop++)
        ;        {
        ;         copy "width" bytes from "sptr" to "dptr"
        ;         sptr += sstride ;
        ;         dptr += dstride ;
        ;        }
        ;
        ; PCS conformant function "bytblt"
bytblt
        ; in:   a1         = source array pointer
        ;       a2         = destination array pointer
        ;       a3         = source index
        ;       a4         = destination index
        ;       [sp + &00] = source stride
        ;       [sp + &04] = destination stride
        ;       [sp + &08] = length
        ;       [sp + &0C] = width
        ;       [sp + &10] = operation
        ;
        ; out:  a1,a2,a3,a4,ip undefined
        ;       v1,v2,v3,v4,v5,dp,sl,fp,sp preserved
        MOV     ip,sp
        STMFD   sp!,{v1,v2,v3,v4,v5,dp,fp,ip,lk,pc}
        SUB     fp,ip,#&04
        CMP     sp,sl
        BLLT    __stack_overflow

        ; We should try to use word copies wherever possible.
        ; (Unfortunately the data is byte aligned)
        ; We should try to avoid branches.

        LDMIA   ip,{v1,v2,v3,v4,v5}
        ; a1 = source array byte pointer (source)
        ; a2 = destination array byte pointer (dest)
        ; a3 = starting byte index into source array (sourceix)
        ; a4 = starting byte index into destination array (destix)
        ; v1 = byte width of source array (sstride)
        ; v2 = byte width of destination array (dstride)
        ; v3 = number of rasters to copy (length)
        ; v4 = number of bytes per raster (width)
        ; v5 = operation to be performed (op)

        CMP     v3,#&00000000           ; check for zero number of rasters
        CMPNE   v4,#&00000000           ; check for zero raster width
        BEQ     bytblt_done             ; then nothing to copy

        ADD     a1,a1,a3                ; initial source byte address
        ADD     a2,a2,a4                ; initial destination byte address

        CMP     v5,#&01
        BEQ     bytblt_loop_non_zero    ; == &01 then non-zero bytes only
        BCS     bytblt_loop_zero        ; == &02 then zero bytes only

bytblt_loop
        ; copy "v4" bytes from "a1" to "a2"
        MOV     a3,v4                   ; initial loop count
bytblt_inner_loop
        LDRB    a4,[a1,a3]              ; load source byte
        STRB    a4,[a2,a3]              ; and store into destination

        SUBS    a3,a3,#&01              ; byte copied
        BNE     bytblt_inner_loop

        ADD     a1,a1,v1                ; step onto next source raster
        ADD     a2,a2,v2                ; step onto next destination raster

        SUBS    v3,v3,#&01              ; completed raster, so check for end...
        BNE     bytblt_loop             ; and around again if not finished
        LDMEA   fp,{v1,v2,v3,v4,v5,dp,fp,sp,pc}^

        ; This loop copies only non-zero bytes
bytblt_loop_non_zero
        ; copy "v4" non-zero bytes from "a1" to "a2"
        MOV     a3,v4                   ; initial loop count
bytblt_inner_loop_non_zero
        LDRB    a4,[a1,a3]              ; load source byte
        TEQ     a4,#&00                 ; is this byte zero?
        STRNEB  a4,[a2,a3]              ; and store into destination if not

        SUBS    a3,a3,#&01              ; byte copied
        BNE     bytblt_inner_loop_non_zero

        ADD     a1,a1,v1                ; step onto next source raster
        ADD     a2,a2,v2                ; step onto next destination raster

        SUBS    v3,v3,#&01              ; completed raster, so check for end...
        BNE     bytblt_loop_non_zero    ; and around again if not finished
        LDMEA   fp,{v1,v2,v3,v4,v5,dp,fp,sp,pc}^

        ; This loop copies only zero bytes
bytblt_loop_zero
        ; copy "v4" zero bytes from "a1" to "a2"
        MOV     a3,v4                   ; initial loop count
bytblt_inner_loop_zero
        LDRB    a4,[a1,a3]              ; load source byte
        TEQ     a4,#&00                 ; is this byte zero?
        STREQB  a4,[a2,a3]              ; and store into destination if so

        SUBS    a3,a3,#&01              ; byte copied
        BNE     bytblt_inner_loop_zero

        ADD     a1,a1,v1                ; step onto next source raster
        ADD     a2,a2,v2                ; step onto next destination raster

        SUBS    v3,v3,#&01              ; completed raster, so check for end...
        BNE     bytblt_loop_zero        ; and around again if not finished
bytblt_done
        LDMEA   fp,{v1,v2,v3,v4,v5,dp,fp,sp,pc}^

        ; ---------------------------------------------------------------------
        END     ; bytblt.s
