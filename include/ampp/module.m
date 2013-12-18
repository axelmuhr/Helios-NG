--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987 - 1993, Perihelion Software Ltd.      --
--                        All Rights Reserved.                          --
--                                                                      --
-- module.m                                                             --
--                                                                      --
--      Module definition macros                                        --
--	Keep in step with library.m					--
--                                                                      --
--      Author:  NHG 08-July-87						--
--      Updates: PAB June 89 for Helios/ARM                             --
--		 PAB 15/8/90 ARM Split module table			--
--		 PAB December 91 for Helios/C40				--
--                                                                      --
--	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include module.m]
_def 'module.m_flag 1

_if _defp 'helios.C40 [
	include c40.m
	include c40mtab.m
]
include structs.m


struct Module [
	word		Type		-- module type = T.Module
	word		Size		-- size of module in bytes
	vec	32	Name		-- module name
	word		Id		-- module table index
	word		Version		-- version number of this module
	word		MaxData		-- highest data offset (>= 0)
	word		Init		-- root of init chain
	_if _defp '__SMT
	[	word	MaxCodeP]
]

struct ResRef [
	word		Type		-- T.ResRef
	word		Size		-- = ResRef.sizeof
	vec	32	Name		-- name of module required
	word		Id		-- module table index
	word		Version		-- version number of module required
	word		Module		-- pointer to module, installed by loader
]

_test _defp 'helios.TRAN [
	_def    T.Program               [#60f060f0]
	_def    T.Module                [#60f160f1]
	_def    T.ResRef                [#60f260f2]
	_def    T.Proc                  [#60f360f3]
	_def    T.Code                  [#60f460f4]
	_def    T.Stack                 [#60f560f5]
	_def    T.Static                [#60f660f6]
	_def	T.ProcInfo		[#60f760f7]
	_def	T.Device		[#60f860f8]
][
	_def    T.Program               [0x60f060f0]
	_def    T.Module                [0x60f160f1]
	_def    T.ResRef                [0x60f260f2]
	_def    T.Proc                  [0x60f360f3]
	_def    T.Code                  [0x60f460f4]
	_def    T.Stack                 [0x60f560f5]
	_def    T.Static                [0x60f660f6]
	_def	T.ProcInfo		[0x60f760f7]
	_def	T.Device		[0x60f860f8]
]

_defq 'StartModule['name 'id 'version]
[
        align
        module  id
.ModStart:
        word    T.Module
	_test _defp 'helios.TRAN [
		word    .ModEnd-.ModStart
	][
		word modsize
	]
	blkb    31, "name"
	byte 0

	word	modnum
	word	version
	_test _defp 'helios.TRAN [
		word	.MaxData
	][
		word	datasymb(.MaxData)
	]
        init
	_if _defp '__SMT
		[word	codesymb(.MaxCodeP)]
]

_defq 'StartProgram['name 'id 'version]
[
        align
        module  id
.ModStart:
        word    T.Program
	_test _defp 'helios.TRAN [
		word    .ModEnd-.ModStart
	][
		word modsize
	]
        blkb    31, "name"
	byte 0

	word	modnum
	word	version
	_test _defp 'helios.TRAN
		[word    .MaxData]
		[word	datasymb(.MaxData)]
        init
	_if _defp '__SMT
		[word	codesymb(.MaxCodeP)]
]

_defq ResRef['name 'id 'version]
[
	align
	module	id
	word	T.ResRef
	word	ResRef.sizeof
	blkb    31, "name" byte 0
	word	modnum
	word	version
	word	0
]

_defq 'Function['name]
[
	_if _defp 'SYSDEB [
		_test _defp 'helios.TRAN [
			align
			'word T.Proc, name
			'byte "name", 0
			.$name$:
			name$:
		][
			align
		__procname_$name$:
			'byte	"$name$", 0
			align
			_test _defp 'helios.C40 [
			      'word 	0xff000000 | - __procname_$name * 4
			][
			      'word 	0xff000000 | - __procname_$name
			]
			export	.$name$
		]
	]
	.$name$:
	name$:
]

_defq 'procsym['name]
[
	_test _defp 'helios.TRAN [
		align
		'word T.Proc, name
		'byte "name", 0
	][
		align
	__procname_$name$:
		'byte	"$name$", 0
		align
		_test _defp 'helios.C40 [
			'word 	0xff000000 | - __procname_$name * 4
		][
			'word 	0xff000000 | - __procname_$name
		]
	]
]

_defq 'codesym['name]
[
	_test _defp 'helios.TRAN [
		align
		'word T.Code, name
		byte "name",0
	][
		align
		'word T.Code, labelref( name )
		byte "name", 0
	]
]

_defq 'stacksym['name 'offset]
[
        align
	'word T.Stack, offset
	byte "name", 0
]

_defq 'staticsym['name]
[
	_test _defp 'helios.TRAN [
		align
		'word T.Static, name
		byte "name", 0
	][
		align
		'word T.Static, labelref( name )
		byte "name", 0
	]
]

_defq  'EndModule
[
	_test _defp 'helios.TRAN [
		data .MaxData 0
	][
		data .MaxData, 0
	]
	_if _defp '__SMT [
		codetable .MaxCodeP
	]
	align		-- force end to word boundary
.ModEnd:
	_if _not _defp 'helios.TRAN [
		end
	]
]


-- Module static data definition and initialisation.

_defq static['initcode]
[
	-- ***********************************************************
	-- These macros are ALMOST identical to the ones defined
	-- in library.m. Be aware of this and update both in concert.
	-- Note also that they differ slightly in that the transputer
	-- versions of module.m also implement data access macros,
	-- initfunc macro and initcode tail that shouldn't find their
	-- way into the library.m file.
	-- ***********************************************************

	-- Cut Here -- module.m -- Cut Here -- module.m -- Cut Here --

	_defq 'extern [ _def '_extern 1 ] -- make following item external
	_defq 'func['name]		  -- define function
	[
		_test _defp '__SMT [
			codetable _$name
		][
			_test _defp 'helios.TRAN [
				data _$name 1
			][
				data _$name, 4
			]
		]
		_if _defp '_extern [
			global [_$name]
			_undef '_extern
		]
		_test _defp 'helios.arm	[
			-- r2 points to static data area
			-- in SMT r4 points to code pointer table
			ldr	ip, (pc,_addressof_$name)
			add	ip, pc, ip
			b	_passaddr_$name
		_addressof_$name:
			int	labelref(.$name)
		_passaddr_$name:
			patchinstr(patcharmdt, codesymb(_$name),
				str ip, (r4, #0))
		][
			_test _defp 'helios.C40	[
				-- only initialise on code pass
				-- cmpi	2, R_A1
				-- bne	no_codeinit_$name

				-- New table based code pointer table initialisation:
				-- Generate a macro (which gets pushed on the macro stack)
				-- that inserts the offset of the function.  When this
				-- macro is evaluated (in _gencodetable) it creates an
				-- entry in the _FuncTable array.

				_def '_codetableentry
				[
					int shift(-2, labelref(.$name))
				]

				-- old version follows ...
				--
				-- ldabs32 [.$name] R_ADDR3	-- addr of name
				--
				-- ldi	R_ADDR2, R_ATMP
				--
				-- NB/ must use 16 bits as RmLib has > 255 functions
				-- patchinstr(PATCHC40MASK16ADD,
				--	shift(-2, codesymb(_$name)),
				--	addi	0, R_ATMP)
				--	sti	R_ADDR3, *R_ATMP
				--
				-- no_codeinit_$name:
			][
				-- transputer code
				ldc .$name-2 ldpi ldl 0 stnl _$name
			]
		]
	]
	_defq 'word['name]		-- define word
	[
		_test _defp 'helios.TRAN
			[data _$name 1]
			[data [_$name], 4]
		_if _defp '_extern [global [_$name] _undef '_extern]
		_if _defp 'helios.TRAN [
			-- !!! do not add data access macro to library.m !!!
			_def name [ldl '_add '_stackinc '_ModTab ldnl 0
			ldnl modtab '[_acctype$nl$_ptrtype] _$ext_name]
		]
	]
	_defq 'struct['type 'name]	-- define structure
	[
		_test _defp 'helios.TRAN
			[data _$name _div _eval[_eval[type$.sizeof]] 4]
			[data [_$name], [_eval[_eval[type$.sizeof]]]]
		_if _defp '_extern [global [_$name] _undef '_extern]
		_if _defp 'helios.TRAN [
			-- !!! do not add data access macro to library.m !!!
			_def name [ldl '_add '_stackinc '_ModTab ldnl 0
			ldnl modtab ldnlp _$ext_name]
		]
	]
	_defq 'vec['sizexxx namexxx]		-- define vector of bytes
	[
		_test _defp 'helios.TRAN
			[data _$namexxx _div sizexxx 4]
			[data [_$namexxx], sizexxx]
		_if _defp '_extern [global [_$namexxx] _undef '_extern]
		_if _defp 'helios.TRAN [
			-- !!! do not add data access macro to library.m !!!
			_def name [ldl '_add '_stackinc '_ModTab ldnl 0
			ldnl modtab ldnlp _$ext_name]
		]
	]
	_defq 'redefine['name]		-- redefine external procedure
	[
		_test _defp 'helios.TRAN [
			ldc .$name - 2 ldpi
			ldl 2		-- what is here?
			ldnl @_$name
			stnl _$name
		][
			_report['Error: 'redefine 'must 'not 'be 'used 'in 'this 'implementation 'of 'Helios]
		]
	]
	_defq 'initptr['name]		-- initialise a pointer to data in code
	[
		_test _defp 'helios.arm	[
			-- r2 points to static data area
			ldr	ip,(pc, _addressofD_$name)
			add	ip, pc, ip
			b	_passDaddr_$name
		_addressofD_$name:
			int	labelref(.$name)
		_passDaddr_$name:
			patchinstr(patcharmdt, datasymb(_$name),
				str ip, (r2, #0))
		][
			_test _defp 'helios.C40 [
				-- only initialise on first data pass
				-- cmpi	0, R_A1
				-- bne	no_dataptrinit_$name

				ldabs32_unsafe [.$name] R_ADDR3	-- addr of name
					
				C40CAddress R_ADDR3

				-- addr of name into its data offset
				patchinstr(PATCHC40MASK8ADD,
					shift(-2, datasymb(_$name)),
					sti R_ADDR3, *+R_ADDR1(0))
					
				-- no_dataptrinit_$name:
			][
				-- transputer code
				ldc .$name - 2 ldpi  -- why the -2? (pipeline?)
				ldl 0 stnl _$name
			]
		]
	]
	_defq 'inittab['name 'size]	-- copy a table from code to data space
	[
		_test _defp 'helios.arm	[
			-- !!! data must be word aligned and word multiple sized
			-- get address of source into ip
			ldr	ip, (pc, _addressofT_$name)
			add	ip, pc, ip
			b	_passTaddr_$name
		_addressofT_$name:
			int	labelref(.$name)
		_passTaddr_$name:

			-- get address of target into r3
			-- hopefully addressability works out ok
			-- r2 points at our static area
			patchinstr(patcharmdp, datasymb(_$name),
				add	r3, r2, #0)

			-- hopefully addressability works out ok
			-- else use ldr r1,#size -- code const in pool
			mov	r1,#size

			_inittab_loop_$name:
				-- copy size .name to _name
				ldr	r0,(ip),#4
				str	r0,(r3),#4
				subs	r1,r1,#4
			bgt _inittab_loop_$name
		][
			_test _defp 'helios.C40 [
				-- data must be word aligned
				-- and word multiple sized
				-- size is specified in bytes, so / 4

				-- only initialise on first data pass
				-- cmpi	0, R_A1
				-- bne	no_tabinit_$name

				-- addr of src name
				ldabs32_unsafe [.$name] R_ADDR3
					
				-- addr of dst name
				StaticDataAddress name R_ADDR4

				-- copy code data to statc data
				ldi	(size >> 2) - 1, rc
				rptb	end_loop_$name
					ldi	*R_ADDR3++(1), R_T2
				end_loop_$name:	sti	R_T2, *R_ADDR4++(1)
				-- no_tabinit_$name:
			][
				-- transputer code
				ldc .$name - 2 ldpi
				ldl 0 ldnlp _$name
				ldc size
				move
			]
		]
	]
	_defq 'initword['name 'value]		-- initialise a word to a value
	[
		_test _defp 'helios.arm	[
				ldr ip, _initword_$name
				b _passby_iw_$name
			_initword_$name:
				int value
			_passby_iw_$name:
				patchinstr(patcharmdt, datasymb(_$name),
					str ip,(r2,#0))
		][
			_test _defp 'helios.C40 [
				-- 		only initialise on first data pass
				-- cmpi	0, R_A1
				-- bne	no_wordinit_$name

				ldi32	value, R_T1

				--		get addr of name in static data
				StaticDataAddress name R_ATMP

				--		dump value into it
				sti	R_T1, *R_ATMP

				-- no_wordinit_$name:
			][
				-- transputer code
				ldc value
				ldl 0 stnl _$name
			]
		]
	]
	_defq 'initptrtab['name 'items 'stride]	-- init a table of ptrs to strings
	[
		_test _defp 'helios.arm	[
			-- get address of source into ip
			-- r2 points to static data area
			ldr	ip, (pc, _addressofP_$name)
			add	ip, pc, ip
			b	_passPaddr_$name
		_addressofP_$name:
			int	labelref(.$name)
		_passPaddr_$name:

			-- get address of target into r3
			-- hopefully addressability works out ok
			-- r2 points at our static area
			patchinstr(patcharmdp, datasymb(_$name),
				add	r3, r2, #0)

			-- hopefully addressability works out ok
			-- else use ldr r1,#items -- code const in pool
			mov	r1, #items

		_initptrtab_loop_$name:
			-- put ptr to source in target
			str	ip, (r3), #4	-- post inc to next
			add	ip, ip, #stride 	-- add stride to source
			subs	r1, r1, #1
			bgt	_initptrtab_loop_$name
		][
			_test _defp 'helios.C40 [
				-- data must be word aligned

				-- 		only initialise on first data pass
				-- cmpi	0, R_A1
				-- bne	no_ptrtabinit_$name

				-- 		addr of src name
				ldabs32_unsafe [.$name] R_ADDR3
					
				-- 		addr of dst name
				StaticDataAddress name R_ADDR4

				-- copy code data to statc data
				ldi	items - 1, rc
				rptb	end2loop_$name
					sti	R_ADDR3, *R_ADDR4++(1)
				end2loop_$name:	addi	stride, R_ADDR3
				-- no_ptrtabinit_$name:
			][
				-- transputer code
				ajw -1
				ldc 0 stl 0
				while[cne [ldl 0] [ldc items] ]
				[
					ldl 0 ldc stride mul		-- offset of next item
					ldc .$name - 2 ldpi		-- base of string table
					bsub				-- address of item
					ldl 0				-- index in table
					ldl 1 ldnlp _$name		-- base of ptr table
					wsub				-- address of item
					stnl 0				-- store it
					ldl 0 adc 1 stl 0		-- inc index
				]
				ajw 1
			]
		]
	]
	-- only initfunc isn't defined by library.m
	_defq 'initfunc['name]		  -- initialize function
	[ -- functionally the same as initptr macro
		_test _defp 'helios.arm	[
			-- r2 points to static data area
			-- in SMT r4 points to code pointer table
			ldr	ip, (pc, _addressofF_$name)
			add	ip, pc, ip
			b	_passFaddr_$name
		_addressofF_$name:
			int	labelref(.$name)
		_passFaddr_$name:
			patchinstr(patch_armdt, codesymb(_$name),
				str ip, (r4, #0))
		][
			_test _defp 'helios.TRAN [
				ldc .$name-2 ldpi ldl 0 stnl _$name
			][
				-- C40
				-- 		only initialise on code pass
				-- cmpi	2, R_A1
				-- bne	no_codeinit_$name

				-- New table based code pointer table initialisation:
				-- Generate a macro (which gets pushed on the macro stack)
				-- that inserts the offset of the function.  When this
				-- macro is evaluated (in _gencodetable) it creates an
				-- entry in the _FuncTable array.

				_def '_codetableentry
				[
					int shift(-2, labelref(.$name))
				]

				-- 	old version follows
				--
				-- ldabs32 [.$name] R_ADDR3	-- addr of name
				--	
				-- 	addr of name into its codetab offset
				-- patchinstr(PATCHC40MASK8ADD,
				--	shift(-2, codesymb(_$name)),
				--	sti R_ADDR3, *+R_ADDR2(0))
				-- no_codeinit_$name:
			]
		]
	]
	_defq 'code['code_body]			-- general assembly code
	[
		code_body
	]

	_defq '_gencodetable
	[
		_if _defp '_codetableentry 	-- any to do ?
		[				--   
			_codetableentry		-- expand entry
			_undef '_codetableentry	-- and pop it off stack
			_gencodetable		-- this is a recursive macro !!
		]
	]

	align
	init

	_test _defp 'helios.arm	[
		-- start of initialisation code
		-- dp reg holds module table base pointer

		_if _defp 'AUTOLIB [
			-- only initialise code pointer tables
			-- C code should initialise data
			cmp r0,#2
			movnes pc,lr
		]
		-- PCS style call
	        mov     ip,sp
       		stmfd   sp!,{a1,a2,a3,a4,v1,fp,ip,lk,pc}
       		sub     fp,ip,#04

		-- r2 points to our static data
		patchinstr(patcharmdt, shift (3, modnum),
			ldr r2,(dp,#0))
		-- r4 points to our code pointer table
		patchinstr(patcharmdt, shift (3, modnum),
			ldr r4,(dp,#4))

		-- initialisation code gets expanded here
		initcode

		-- PCS style return
		ldmea fp,{a2,a3,a4,v1,fp,sp,pc}^
	][
		_test _defp 'helios.C40 [
			-- prefix to init code
			-- assumes R_MT == module table
			-- r0 = initialisation pass argument
			--	2 = init code
			--	0 = first data pass
			--	1 = second data pass
			-- should only init data on 0/1 and code on 2

			-- check for stage 2 initialisation
			CMPI	2, R_A1

			Beq	_MCodeTableInit


			-- R_ADDR1 = base of modules static data area
			patchinstr(PATCHC40MASK8ADD,
				shift(1, modnum),
				ldi	*+R_MT(0), R_ADDR1)

			C40WordAddress R_ADDR1

			-- preserve the return address

			LDI	R_LR,	R_FT2

			initcode

			-- postfix to initcode
			b	R_FT2		-- return

			_MCodeTableInit:

			_test _defp '_codetableentry
			[
				-- R_ADDR1 == address base of modules code table
				patchinstr(PATCHC40MASK8ADD,
					shift(1, modnum),
					ldi	*+R_MT(1), R_ADDR1)
					ldabs16 [_FuncTableEnd] R_ADDR2 -- get hold of address of end of table
				B	_Loop1Start		-- jump into middle of loop

				_Loop1:				-- start of loop
				ADDI	 R_ADDR2, R_TMP1	-- add address of field to contents of field
				STI	 R_TMP1, *R_ADDR1++(1)	-- store in module's code table
				_Loop1Start:			-- middle of loop
				LDI *'-'-R_ADDR2, R_TMP1	-- get value out of table of function offsets
				Bne	_Loop1	    		-- continue until 0 entry found
				B	 R_LR			-- return				

				_FuncTable:			-- start of function offset table
					int 0			-- NULL entry in table indicates end of table
					_gencodetable		-- generate a table of function offsets
				_FuncTableEnd:			-- end of function offset table
			]
			[
				B	R_LR
			]
		][
			-- transputer code
			-- !!! do not add this code to library.m !!!
			ajw -1
			ldl 2 ldnl modnum stl 0
			_set '_ModTab 2
			initcode
			ajw 1
			ret
		]
	]

	_undef 'extern
	_undef 'func
	_undef 'word
	_undef 'struct
	_undef 'vec
	_undef 'redefine
	_undef 'initptr
	_undef 'inittab
	_undef 'initword
	_undef 'initptrtab
	_undef 'initfunc
	_undef 'code
]

		-- Cut Here -- Cut Here -- Cut Here -- Cut Here -- Cut Here --


-- End of module.m
