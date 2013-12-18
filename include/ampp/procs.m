--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- procs.m                                                              --
--                                                                      --
--      Structured programming and procedure defintion macros.          --
--                                                                      --
--      Author: NHG 20-May-87						--
--                                                                      --
--	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include procs.m]
_def 'procs.m_flag 1

_if _defp 'helios.TRAN [		-- Tranputer macros only

include basic.m
include structs.m
include module.m

-- The following macros implement temporary labels for the structured
-- programming constructs.

_set  '_tempn 1
_defq 'deftemp['deftemp_body] [
        _def '_temp [..$_tempn]
        _set '_tempn _add _tempn 1
        deftemp_body
        _undef '_temp
]
_defq 'templab[n] [_temp$n]

-- Structured programming macros

-- Declare a critical section, no jumps
_defq 'critical['critical_body]
[
        _defq '_incrit [x]
        critical_body
        _undef '_incrit
]

-- Execute body at high priority
_defq 'hipri['hipri_body]
[
deftemp [
        ldpri ldlp 0 or stl -3			-- save old workspace
        ldc templab 1 - 2                       -- point to high pri code
        ldpi stl -5                             -- set in new stack frame
        ldlp -4                                 -- point to new workspace
        runp                                    -- run the process
        stopp                                   -- halt here
        j templab 2                             -- and go on when restarted
	align
templab 1:
        _set '_stackinc _add _stackinc 4        -- up stack increment
        hipri_body                              -- code to run
        _set '_stackinc _sub _stackinc 4        -- reset increment
        ldl 1                                   -- workspace of creator
        runp stopp                              -- run him & stop us
	align
templab 2:
        ]
]

-- The following macro places the current process at the end of the
-- scheduling list. Note that by the time the process restarts the
-- correct Iptr value will have been planted by stopp.

_defq 'yeild [ ldlp 0 ldpri or runp stopp ]

-- fork into two processes and rejoin later
-- roughly equivalent to an occam PAR construct

_defq 'fork['fork_lstack 'fork_left 'fork_right]
[
deftemp [
	_set 'lst _add fork_lstack 5			-- form stack inc 
	ajw -2						-- space for control data
	ldc 2 stl 1					-- number of processes
	ldc templab 3 -2 ldpi stl 0			-- set continue address
	ldc templab 1 - templab 2			-- entry point of process 2
	ldlp -lst					-- workspace of process 2
	startp						-- start it
templab 2:
        _set '_stackinc _add _stackinc 2		-- up stack increment
	fork_left					-- left hand process
        _set '_stackinc _sub _stackinc 2		-- reset increment
	ldlp 0 endp					-- and terminate
	align
templab 1:
        _set '_stackinc _add _stackinc _add 2 lst	-- up stack increment
        fork_right					-- right hand process
        _set '_stackinc _sub _stackinc _add 2 lst	-- reset increment
	ldlp lst
	endp						-- terminate
	align
templab 3:
	ajw 2						-- we will finally continue here
	]
]

_defq 'if['if_cond 'if_then]                    -- one way conditional
[
deftemp [
        if_cond
        cj templab 1
        if_then
	align
templab 1:
        ]
]

_defq 'test['test_cond 'test_then 'test_else]    -- two way conditional
[
deftemp [
        test_cond
        cj templab 1
        test_then
        _test [_defp '_incrit] [ldc 0 cj templab 2] [j templab 2]
	align
templab 1:
        test_else
	align
templab 2:
        ]
]

_defq 'while['while_cond 'while_body]           -- while loop
[
deftemp [
        _def 'break [j templab 2]
	_def 'continue [j templab 1]
	align				-- force loop label to word boundary
templab 1:
        while_cond
        cj templab 2
        while_body
        _test [_defp '_incrit] [ldc 0 cj templab 1] [j templab 1]
	align
templab 2:
	_undef 'break
	_undef 'continue
        ]
]

_defq 'forever['body]
[
deftemp [
        _def 'break [j templab 2]
	_def 'continue [j templab 1]
	align				-- force loop label to word boundary
templab 1:
	body
	j templab 1
	align
templab 2:
	]
	_undef 'break
	_undef 'continue
]

-- conditions for use in the above macros

_defq ceq0['expr] [expr eqc 0]
_defq cne0['expr] [expr]
_defq ceq['left 'right] [left right diff eqc 0]
_defq cne['left 'right] [left right diff]
_defq cgt['left 'right] [left right gt]
_defq clt['left 'right] [right left gt]
_defq cge['left 'right] [right left gt eqc 0]
_defq cle['left 'right] [left right gt eqc 0]
_defq after['left 'right] [left right diff ldc 0 gt eqc 0]

-- alternate construct
-- used only for channel and timer alts without variable guards

_defq alternate['alt_body]
[
deftemp [
--        ajw -1 _set '_stackinc _add _stackinc 1
        talt
        _defq 'channel['chan 'body]
        [ chan ldc 1 enbc ]
        _defq 'timer['time 'body]
        [ time ldc 1 enbt ]
        alt_body
        taltwt
        _undef 'channel _undef 'timer
        _defq 'channel['chan 'body]
        [ chan ldc 1 ldc templab tempid - templab 0 disc _set 'tempid _add tempid 1]
        _defq 'timer['time 'body]
        [ time ldc 1 ldc templab tempid - templab 0 dist _set 'tempid _add tempid 1]
        _set 'tempid 2
        alt_body
        altend
templab 0:
        _undef 'channel _undef 'timer
        _defq 'channel['chan 'body]
        [
        templab tempid:
                body
                _set 'tempid _add tempid 1
        ]
        _defq 'timer['time 'body]
        [
        templab tempid:
                body
                _set 'tempid _add tempid 1
        ]
        _defq 'break  [j templab 1]
        _set 'tempid 2
        alt_body
templab 1:
--        ajw 1 _set '_stackinc _sub _stackinc 1
        ]
        _undef 'channel _undef 'timer
]

-- procedure definitions

_defq 'proc[proc_name 'proc_args 'proc_locals 'proc_body]
[
_report [Defining procedure proc_name]
        _defq 'word[''arg_name]
        [
                _def arg_name ['[_acctype$l$_ptrtype] '_add _nlocals '_stackinc]
--                stacksym arg_name _nlocals
                _set '_nlocals _add _nlocals 1
        ]
	_defq 'vec[vec_size ''vec_name]
	[
		_def vec_name [ldlp '_add _nlocals '_stackinc]
--                stacksym vec_name _nlocals
                _set '_nlocals _add _nlocals _div _add 3 vec_size 4
	]
	_defq 'struct['struct_name ''arg_name]
	[
                _def arg_name [ldlp '_add _nlocals '_stackinc]
--                stacksym arg_name _nlocals
                _set '_nlocals _add _nlocals _div _add 3 _eval[_eval[struct_name$.sizeof]] 4
	]
        align
        procsym [_eval[.$proc_name]]
        _set '_nlocals 0
        proc_locals
        _set '_sfsize _nlocals
	_set '_Link _nlocals
	_test _defp '_kprocp
	[
		_set '_nlocals _add _nlocals 1
	]
        [
		_set '_ModTab _add _nlocals 1
		_set '_nlocals _add _nlocals 2
	]
        _set '_stackinc 0
        proc_args
.$proc_name:
        ajw -_sfsize
        proc_body
        return
        _undef 'word _undef 'vec _undef 'struct
        _defq 'word[''arg_name] [_undef arg_name]
	_defq 'vec[vec_size ''vec_name] [_undef vec_name]
	_defq 'struct['struct_name ''arg_name] [_undef arg_name]
        proc_locals
        proc_args
        _undef 'word _undef 'vec _undef 'struct
	_genstubs
]

_defq 'kproc['_a '_b '_c '_d] 
[
_def '_kprocp 1
proc _a _b _c _d
_undef '_kprocp
]

_defq 'return [ajw _sfsize ret]			-- return from proc

_defq 'invoke['pname]				-- call external procedure
[
	ldl _add _stackinc _ModTab 		-- module table
	call .$pname				-- call proc
]

_defq 'fakecall['pname]
[
	ldl _add _stackinc _ModTab		-- fake module table
	call .$pname				-- call proc
]

-- local call, calln allows n arguments to be passed
_defq call0['pname] 
[ 
	ldl _add _stackinc _ModTab
	call .$pname 
]

_defq call1['pname 'arg1]
[
	arg1 
	ldl _add _stackinc _ModTab
	call .$pname
]

_defq call2['pname 'arg1 'arg2]
[
	arg2
	arg1
	ldl _add _stackinc _ModTab
	call .$pname
]

_defq call3['pname 'arg1 'arg2 'arg3]
[
	push 1
	arg3
	stl 0
	arg2
	arg1
	ldl _add _stackinc _ModTab
	call .$pname
	pop 1
]

_defq push[_inc] [ajw -_inc _set '_stackinc _add _stackinc _inc]

_defq pop[_inc] [ajw _inc _set '_stackinc _sub _stackinc _inc]

-- External variable access

_defq 'external['ext_name]
[
_def ext_name
[ldl '_add '_stackinc '_ModTab ldnl 0 ldnl @_$ext_name '[_acctype$nl$_ptrtype] _$ext_name]
]

-- External procedure calling macros
-- callx may only be used from a proc
-- only two arguments may be passed

_defq 'callx['name]
[
	ldl _ModTab			-- module table
	call .$name			-- call stub code
	_def '_call_stub
	[				-- stack a stub for this procedure
		'_if '_not '_defp ['']name$.stubflag
		[			-- only generate if not one already done
	.$name:
		ldl 1
		ldnl 0
		ldnl @_$name
		ldnl _$name
		gcall
		'_defq name$.stubflag 1	-- flag that a stub has been generated
		]
	]
]

_defq '_genstubs
[
	_if _defp '_call_stub		-- any to do ?
	[
		_call_stub		-- generate a stub
		_undef '_call_stub	-- and pop it off stack
		_genstubs		-- this is a recursive macro !!
	]
]

_defq 'loop['loop_var 'loop_count 'loop_body]
[
	deftemp
	[
	templab 1:
		loop_body
		ptr loop_var
		ldc templab 2 - templab 1
		lend
	templab 2:
	]
]

] -- end of transputer macros


-- End of procs.m
