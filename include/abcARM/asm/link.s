        SUBT    Helios Kernel link control structures           > link/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      900420  JGSmith
        ;
        ; Link control structures
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (struct_s)      ; ensure "structs.s" is included
        ASSERT  (sem_s)         ; ensure "sem.s" is included
        ASSERT  (queue_s)       ; ensure "queue.s" is included
        ASSERT  (message_s)     ; ensure "message.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including link.s"
        GBLL    link_s
link_s  SETL    {TRUE}

        ; ---------------------------------------------------------------------
        ; link control structure

        struct          "Link"
        struct_byte     "Flags"         ; flag byte
        struct_byte     "Mode"          ; link mode/type
        struct_byte     "State"         ; link state
        struct_byte     "Id"            ; link id used in ports etc.
        struct_word     "TxChan"        ; address of transmission channel
        struct_word     "RxChan"        ; address of reception channel
        struct_word     "TxUser"        ; Id for tx channel
        struct_word     "RxUser"        ; Id for rx channel
        struct_word     "MsgsIn"        ; number of input messages
        struct_word     "MsgsOut"       ; number of output messages
        struct_word     "TxQueue"       ; queue of waiting transmitters
        struct_struct   "Sem","TxMutex" ; Mutual exclusion for transmitters
        struct_word     "RxSync"        ; sync channel for direct link rx
        struct_word     "LocalIOCPort"  ; port to be used by our LinkIOC
        struct_word     "RemoteIOCPort" ; port to remote IOC
        struct_word     "Incarnation"   ; remote processors incarnation number
        struct_word     "MsgsLost"      ; messages lost/destroyed
        struct_word     "spare1"
        struct_word     "spare2"
        struct_end

        ; ---------------------------------------------------------------------
        ; Flag bits

Link_Flags_bootable     *       &80     ; set if remote processor is bootable
Link_Flags_parent       *       &40     ; set if this link booted us
Link_Flags_ioproc       *       &20     ; indicates an io processor

        ; ---------------------------------------------------------------------
        ; Link Modes

Link_Mode_Null          *       0       ; the link is not connected to anything
Link_Mode_Dumb          *       1       ; connected to a dumb device
Link_Mode_Intelligent   *       2       ; connected to an intelligent device

        ; ---------------------------------------------------------------------
        ; Link states

Link_State_Starting     *       0       ; link is in process of starting up
Link_State_Booting      *       1       ; link is booting remote processor
Link_State_Dumb         *       2       ; serving a dumb device
Link_State_Running      *       3       ; serving as network driver
Link_State_Timedout     *       4       ; timeout on link has expired
Link_State_Crashed      *       5       ; the link has crashed
Link_State_Dead         *       6       ; there is nothing on this link

        ; ---------------------------------------------------------------------
        ; Timeout values

Timeout_Idle            *       10000000        ; 10 second timeout
Timeout_Short           *       1000000         ; 1 second timeout
Timeout_Tx              *       11000000        ; transmission timeout

        ; ---------------------------------------------------------------------

LGStackSize     *       1024            ; size of link guardian stack

Proto_Null      *       3               ; initial value of type word
Proto_Sync      *       &F0F0F0F0       ; link sync value
Proto_Write     *       0               ; debug write command
Proto_Read      *       1               ; debug read command
Proto_Info      *       &F0             ; first byte of sync value
Proto_Msg       *       2               ; message header
Proto_Dead      *       &61             ; result of probing a dead link
Proto_Alive     *       &9E             ; result of probing a live link
Proto_Term      *       4               ; machine terminate message

Probe_Value     *       &61616161       ; value for probes

        struct          "InfoMsg"
        struct_byte     "TxInc"         ; transmitters incarnation number
        struct_byte     "RxInc"         ; receivers incarnation number
        struct_byte     "ReplyReq"      ; non zero if reply required
        struct_word     "IOCPort"       ; port desc for his Link IOC
        struct_end

        ; ---------------------------------------------------------------------
        ; Pending message buffer structure
        ; This is the size of a pool buffer, but this structure is also
        ; mapped onto buffers of other sizes.

        struct          "MsgBuf"
        struct_struct   "Node","Node"           ; queuing node
        struct_word     "Type"                  ; 0=pool 1=special
        struct_struct   "MCB","MCB"             ; control structure
        struct_vec      1056,"Msg"              ; buffer
        struct_end

MsgBuf_Overhead         *       (Node_sizeof + MCB_sizeof + 4)

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF link/s
