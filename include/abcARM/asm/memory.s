        SUBT    Helios Kernel memory control structures         > memory/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900420  JGSmith
        ;
        ; Memory control structures
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (structs_s)     ; ensure "structs.s" is included
        ASSERT  (queue_s)       ; ensure "queue.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including memory.s"
                GBLL         memory_s
memory_s        SETL         {TRUE}

        ; ---------------------------------------------------------------------

        struct          "Pool"
        struct_struct   "Node","Node"           ; queueing node
        struct_struct   "List","Memory"         ; list of memory blocks
        struct_word     "Blocks"                ; number of blocks in pool
        struct_word     "Size"                  ; total size of pool
        struct_word     "Max"                   ; largest block in pool
        struct_end

        ; ---------------------------------------------------------------------

        struct          "Memory"
        struct_struct   "Node","Node"           ; link in pool list
        struct_word     "Size"                  ; block size + alloc bits
        struct_word     "Pool"                  ; pointer to owning pool
        struct_end

        ; ---------------------------------------------------------------------

Memory_Size_BitMask     *       15      ; mask for allocation bits
Memory_Size_BwdBit      *       2       ; allocation state of predecessor
Memory_Size_FwdBit      *       1       ; allocation state of this block
Memory_Size_Fast        *       4       ; block is for a fast RAM area

epsilon                 *       Memory_sizeof

        ; ---------------------------------------------------------------------
        ; Fast Ram carrier

        struct          "Carrier"
        struct_word     "Addr"          ; Fast RAM block address
        struct_word     "Size"          ; and its size
        struct_end

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF memory.s
