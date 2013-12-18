#*-------------------------------------------------------------------------
#--                                                                      --
#--             H E L I O S   A T A R I   I / O   S Y S T E M            --
#--             ---------------------------------------------            --
#--                                                                      --
#--             Copyright (C) 1987, Perihelion Software Ltd.             --
#--                        All Rights Reserved.                          --
#--                                                                      --
#--      sunasm.s                                                        --
#--                                                                      --
#--         Basic coroutine library functions for SUN, based on          --
#--                                                                      --
#--         version for Atari ST using Mark Williams Assembler, which    --
#--                                                                      --
#--         was ported by BLV from a version for the Amiga by Nick       --
#--                                                                      --
#--         Garnett.                                                     --
#--                                                                      --
#--     Author:  DJCH                                                    --
#--                                                                      --
#------------------------------------------------------------------------*/
#* Copyright (C) 1987, Perihelion Software Ltd. */
 
#   STRUCTURE co_routine,0    coroutine structure definition using Amiga macros
#      APTR   co_link      link in chain (must stay here)
#      LONG   co_size      size of stack allocated to co-routine
#      APTR   co_sp        stack pointer for this co-routine
#      APTR   co_parent    parent co-routine (who called)
#      APTR   co_func      initial function when starting up
#      LABEL  co_SIZEOF
#
#     equivalent of above for Atari and SUN
co_link    = 0          | offsets within a structure
co_size    = 4
co_sp      = 8
co_parent  = 12
co_func    = 16
co_SIZEOF  = 20         | size of the structure
.text
# external routines used
.globl   _malloc,_free

# these allow access from C
.globl   _InitCo,_CreateCo,_CallCo,_WaitCo,_DeleteCo,_ResumeCo
.globl   _RootCo,_CurrentCo


_CreateCo:
      movl  sp@(4),a0         | function
      movl  sp@(8),d0         | NB a .long, not an int

#==============================================================================
# coroutine = CreateCo( function,size )
#    d0                   a0     d0
#
# Creates and adds a co-routine with the required stacksize and start function.
# size is in bytes
#==============================================================================
CreateCo:  moveml d3-d7/a3-a6,sp@-      | save all registers
           movl   a0,a3                 | stash start routine address
           addl   #co_SIZEOF,d0         | add coroutine structure size
           movw   d0,sp@-
           movl   d0,sp@-               | SUN push arg to stack

           jsr    _malloc               | get the memory from the system
           clrl   d2                    | clear leading bits
           addql  #4,sp                 |SUN rtn
           movw   sp@+,d2
           tstl   d0                    | if any is left
           bnes   L2                    | got it with no problems
           moveml sp@+,d3-d7/a3-a6      | else, restore registers
           rts                          | return with d0 = 0

L2:        movl   d0,a1                 | a1 = new coroutine base
           movl   _CurrentCo,a0         | a0 = current coroutine
           movl   a0@(co_link),a1@(co_link) | link new coroutine into chain
           movl   a1,a0@(co_link)
           movl   d2,a1@(co_size)       | set stack size
           movl   a0,a1@(co_parent)     | set parent to current co
           movl   a3,a1@(co_func)       | set function pointer
           addl   a1,d2                 | d2 = sp of new co-routine
           movl   a1,_CurrentCo         | make new coroutine current
           movl   sp,a0@(co_sp)         | save sp, parent is now suspended
           movl   d2,sp                 | swap to new stack
           movl   a1,d0                 | return new coroutine address
           movl   a1,a3                 | and save it for us too
L1:        bsrs   WaitCo                | waitco back to parent
           movl   a3@(co_func),a0       | a0 = initial function
           movl   d0,sp@-               | SUN arg to stack
           jsr    a0@                   | call it (Arg in d0)
           addqw  #4,sp                 | SUN stack chop
           bras   L1                    | and loop forever if it returns

_CallCo:
           movl   sp@(4),a0
           movl   sp@(8),d0             | NB a .long, not an int

#==============================================================================
# Result = CallCo( coroutine,arg )
#   d0                 a0    d0
#
# Starts up a coroutine that was just created or did a WaitCo to return an arg.
#==============================================================================
CallCo:    movl   a0,a1                 | stash entering address
           movl   _CurrentCo,a0         | a0 = current coroutine
           movl   a0,a1@(co_parent  )   | adopt the coroutine

# coming here we assume:
# D0 = return/argument value
# A0 = coroutine we are leaving
# A1 = coroutine we are entering
coenter:   moveml d3-d7/a3-a6,sp@-      | save this coroutines registers
           movl   sp,a0@(co_sp)         | save stack pointer
           movl   a1@(co_sp),sp         | move to new coroutine stack
           moveml sp@+,d3-d7/a3-a6      | restore registers
           movl   a1,_CurrentCo         | set new current coroutine
           rts                          | and go into it

_WaitCo:
           movl   sp@(4),d0             | NB a .long, not an int

#==============================================================================
# Arg = WaitCo( arg )
# d0            d0
#
# Returns control back to the parent with required argument/return code in d0.
# Arg will eventually be returned when the coroutine doing the WaitCo is called
# again with CallCo(coroutine,ARG) or ResumeCo(coroutine,ARG)
#==============================================================================
WaitCo:
           movl   _CurrentCo,a0         | a0 = current coroutine
           movl   a0@(co_parent),a1     | a1 = parent
           clrl   a0@(co_parent)        | become an orphan
           bras   coenter               | go do switch

_ResumeCo:
           movl   sp@(4),a0
           movl   sp@(8),d0

#==============================================================================
# arg = ResumeCo( coroutine, arg )
#  d0                a0      d0
#
# Passes control across to a waiting co-routine on the same level as current.
#==============================================================================
ResumeCo: 
           movl   a0,a1                 | save routine we are going to
           movl   _CurrentCo,a0         | a0 = current coroutine
           movl   a0@(co_parent),a1@(co_parent)
                                        | get adopted parent
           clrl   a0@(co_parent)        | and this one is orphaned now
           bras   coenter               | go call it

_DeleteCo:
           movl   sp@(4),a0

#==============================================================================
# success = DeleteCo( coroutine )
#   d0                 a0
#
# Deletes the stack area being used by a coroutine that is no longer needed.
#==============================================================================
DeleteCo:
           movel  #0,d0                 | assume failure
           cmpl   #0,a0                 | in case we are passed a 0
           beq    DeleteFail            | we were, quit now
           tstl   a0@(co_parent)        | active ?
           bne    DeleteFail            | yes, can't delete it
           movl   _RootCo,a1            | a1 = coroutine chain
           bras   L15                   | skip first link

L10:       movl   a1@,a1                | indirect
L15:       cmpl   a1@,a0                | is next the one we want ?
           bnes   L10                   | if not loop

           movl   a0@,a1@               | unlink from co-routine list
           movl   a0,sp@- 
           jsr    _free                 | free it using C's free()
           addql  #4,sp   
           movel  #-1,d0                | return TRUE
DeleteFail:   rts


#==============================================================================
# Success = InitCo()
#   d0
#
# Initialises a root co-routine that never goes away.  It corresponds directly
# to the main level of the program and is really just a list header for all
# other co-routines that get started.  The memory allocation could go in the
# main allocator in Init() but I've left it here for clarity.
#==============================================================================
_InitCo: 
           movl   a6,sp@-
           movel  #co_SIZEOF,d0         | get root co-routine
           movw   d0,sp@-
           movl   d0,sp@-
           jsr    _malloc               | obtain the memory  
           addql  #4,sp
           movw   sp@+,d2
           tstl   d0
           beqs   L20                   | it didn't work so fail
           movl   d0,a0
           movl   a0,_RootCo            | this will always point here
           movl   a0,_CurrentCo         | and sometimes here
           movl   a0,a0@(co_parent)     | I'm my own parent
           movel  #-1,d0                | return success
L20:       movl   sp@+,a6
           rts

.data     
_RootCo:   .long  1                     | space for two static variables
_CurrentCo:.long  1


