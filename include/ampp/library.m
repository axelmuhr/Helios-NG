--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--            Copyright (C) 1987-93, Perihelion Software Ltd.           --
--                        All Rights Reserved.                          --
--                                                                      --
-- library.m								--
--                                                                      --
--	Resident library definition macros.				--
--	Must be kept in step with module.m				--
--                                                                      --
--      Author: NHG 28-July-87						--
--      Update: PAB 23/6/89 Helios/ARM                                  --
--		PAB 15/8/90 ARM Split module table			--
--              PAB Dec '91 Helios/C40                                  --
--		PAB Oct '92 trap init/stubs/headers			--
--                                                                      --
-- RCS Id: $Id: library.m,v 1.13 1993/08/11 14:26:25 nickc Exp $	--
--------------------------------------------------------------------------
-- These macros are used to process a library input file and output a
-- variety of different output files. These output files are used either
-- to define the library interface, or help initialise it.
--
-- Several different flags determine the type of the output:
-- 
-- 	default (no flags set):
--			Outputs module header and initialisation code.
--			This installs pointers to the exported functions
--			into the module table, or modtabs code pointer
--			table in the case of split module tables.
--
--	make.def = 1
--			Outputs library definition files. These files define
--			libraries module table slot and the offset of the code
--			pointers held in the module table. The exported
--			symbols that define this offset are the functions
--			name prefixed with an '_'.
--
--	make.traps = 1 (ARM only)
--			Outputs trap table initialisation code. The trap
--			table is initialised to point at the trap functions
--			defined in the library input file.
--
--	make.trapstubs = 1 (ARM only)
--			Outputs a series of trap stub functions. The stub
--			functions simply trap to their trap_name and then
--			return to the caller. The stubs name being placed
--			in the module table by the initialisation code, rather
--			than the true function address.
--
--			These stubs are being used temporarily until we can
--			call traps directly from C, rather than having
--			to indirect through the module table.
--	make.trapCdefs = 1 (ARM only)
--			Outputs a C header file defining the trap number to
--			use for each trap function.
--	make.trapAdefs = 1 (ARM only)
--			Outputs an ampp header file defining the trap number to
--			use for each trap function.
--
--------------------------------------------------------------------------

_report ['include library.m]
_def 'library.m_flag 1

include basic.m
include module.m

_test _defp 'helios.TRAN [
	_report '[Making Transputer Helios version]
][
	_test _defp 'helios.arm	[
		_report '[Making Helios-ARM version]
		include arm.m
	][
		_test _defp 'helios.C40
		[
			_report '[Making Helios-C40 version]
		][
			_test _defp 'helios.68K
			[
				_report '[*DANGER* Support for 68K not yet complete]
			][
				_report '[*DANGER* making undefined processor version]
			]
		]
	]
]

_defq 'Resident['resident.body]
[
	_test _defp 'make.def [
		_report '[Making library definition file]
	][
	 _test _defp 'make.traps [
		_report '[Making trap table initialisation code]
	 ][
	  _test _defp 'make.trapCdefs [
		_report '[Making trap number C header file]
	  ][
	   _test _defp 'make.trapAdefs [
		_report '[Making trap number AMPP header file]
	   ][
	    _test _defp 'make.trapstubs [
		_report '[Making trap stubs code]
	    ][
		_report '[Making library startup and initialisation file]
	    ]
	   ]
	  ]
	 ]
	]

	_defq 'res_name Unknown.Name
	_defq 'res_slot Unknown.Slot
	_defq 'res_version Unknown.Version

	_defq 'name['arg] [_def 'res_name arg]
	_defq 'slot['arg] [_def 'res_slot arg]
	_defq 'version['arg] [_def 'res_version arg]

	resident.body

	_undef 'name
	_undef 'slot
	_undef 'version

]

-- if make.def is defined, generate a file which can be assembled to
-- a .def file.
_if _defp 'make.def [
	_defq 'uses['libname]
	[
		ref	[libname$.library]
	]

	-- static just defines external labels
	_defq 'static['initcode]
	[
		ResRef res_name res_slot res_version
res_name$.library:
		global	[res_name$.library]

		_defq 'extern [ _def '_extern 1 ] -- make following item external
		_defq 'trap['name]		  -- define trap
		[
			_test _defp 'helios.arm	[
				-- Currently only ARM CPU implements traps.
				-- All others define traps as normal functions
				-- by default and never get called with a
				-- make.traps / make.trapXdefs pass

				_if _true [

				-- @@@ temporarily define function names as
				-- normal, until we get some form of _trap()
				-- function working from C so user mode code
				-- can call the traps directly. When this is
				-- the case this condition should be a no-op.

					codetable [_$name]
					_if _defp '_extern [
						global [_$name]
						_undef '_extern
					]

				][
					_if _defp '_extern [
						_undef '_extern
					]
				]
				
			][
				_test _defp '__SMT [
					codetable _$name
				][
					_test _defp 'helios.TRAN
						[data _$name 1]
						[data _$name, 4]
				]
				_if _defp '_extern [global [_$name] _undef '_extern]
			]
		]
		_defq 'func['name]		  -- define function
		[
			_test _defp '__SMT [
				codetable [_$name]
			][
				_test _defp 'helios.TRAN
					[data _$name 1]
					[data _$name, 4]
			]
			_if _defp '_extern [global [_$name] _undef '_extern]
		]
		_defq 'word['name]		-- define word
		[
			_test _defp 'helios.TRAN
				[data _$name 1]
				[data _$name, 4]
			_if _defp '_extern [global [_$name] _undef '_extern]
		]
		_defq 'struct['type 'name]	-- define structure
		[
			_test _defp 'helios.TRAN
				[data _$name _div _eval[_eval[type$.sizeof]] 4]
				[data [_$name], [_eval[_eval[type$.sizeof]]]]
			_if _defp '_extern [global [_$name] _undef '_extern]
		]
		_defq 'vec['sizexxx namexxx]		-- define vector of bytes
		[
			_test _defp 'helios.TRAN
				[data _$namexxx _div sizexxx 4]
				[data [_$namexxx], sizexxx]
-- THIS DONT WORK!!!!!!!	[data [_$namexxx], size]
			_if _defp '_extern [global [_$namexxx] _undef '_extern]
		]
		_defq 'redefine['name]		-- redefine external procedure
		[]
		_defq 'initptr['name]		-- initialise a pointer to data in code
		[]
		_defq 'inittab['name 'size]	-- copy a table from code to data space
		[]
		_defq 'initword['name 'value]		-- initialise a word to a value
		[]
		_defq 'initptrtab['name 'items 'stride]	-- init a table of ptrs to strings
		[]
		_defq 'code['code_body]			-- general assembly code
		[]

		initcode
		_undef 'extern
		_undef 'trap
		_undef 'func
		_undef 'word
		_undef 'struct
		_undef 'vec
		_undef 'redefine
		_undef 'initptr
		_undef 'inittab
		_undef 'initword
		_undef 'initptrtab
		_undef 'code
	]

	-- no stubs
	_defq 'stubs['body]
	[ ]

	-- no data
	_defq 'LibData['body]
	[]

]


-- Default to making library startup and initialisation file
-- Used to be simply: _if _not _defp 'make.def [

_if _not _or [_or _defp 'make.def _defp 'make.traps]
	     [ _or [_or _defp 'make.trapCdefs _defp 'make.trapAdefs]
		   [_defp 'make.trapstubs]
	     ]
[

	_defq 'uses['libname]
	[
		ref	[libname$.library]
	]

	_defq 'static['initcode]
	[
		StartModule res_name res_slot res_version
res_name$.library:
		global	[res_name$.library]

		_if _defp 'headcode [
			-- Insert any headcode at start of module.
			-- Used by kernel to insert initial kernel
			-- entry branch.

			headcode

			align

			_undef 'headcode
		]

		-- Cut Here -- module.m -- Cut Here -- module.m -- Cut Here --

		-- ***********************************************************
		-- These macros are ALMOST identical to the ones defined
		-- in module.m. Be aware of this and update both in concert.
		-- Note also that they differ slightly in that the transputer
		-- versions of module.m also implement data access macros,
		-- initfunc macro and initcode tail that shouldn't find their
		-- way into the library.m file.
		-- ***********************************************************

		_defq 'extern [ _def '_extern 1 ] -- make following item external

		_defq 'trap['name]		  -- define trap
		[
			_test _defp 'helios.arm	[
				-- Currently only cpu type to implement traps.
				-- All others define traps as normal functions
				-- by default and never get called with a
				-- make.traps / make.trapdefs pass.

				_if _true [

				-- @@@ temporarily initialise as normal, but
				-- using trap stub names rather than the true
				-- function name. This is until we get some
				-- form of _trap() function working from C so
				-- user mode code can call the traps directly.
				-- When this is the case this condition should
				-- be a no-op.

					codetable _$name

					_if _defp '_extern [
						global [_$name]
						_undef '_extern
					]

					-- r2 points to static data area
					-- in SMT r4 points to code pointer table
					ldr	ip, (pc, _taddressof_$name)
					add	ip, pc, ip

					b	_tpassaddr_$name
				_taddressof_$name:
					-- note _trap_stub address not functions
					int	labelref(_trapstub_$name)
				_tpassaddr_$name:
					patchinstr(patcharmdt, codesymb(_$name),
						str ip,(r4,#0))
				]
			][
				_test _defp '__SMT [
					codetable _$name
				][
					_test _defp 'helios.TRAN
						[data _$name 1]
						[data _$name, 4]
				]
				_if _defp '_extern [
					global [_$name]
					_undef '_extern
				]

				_test _or _defp 'helios.C40 _defp 'helios.68K
				[
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
		_defq 'func['name]		  -- define function
		[
			_test _defp '__SMT [
				codetable _$name
			][
				_test _defp 'helios.TRAN
					[data _$name 1]
					[data _$name, 4]
			]
			_if _defp '_extern [
				global [_$name]
				_undef '_extern
			]

			_test _defp 'helios.arm	[
				-- r2 points to static data area
				-- in SMT r4 points to code pointer table
				-- commented out instr works only with hobjasm
				ldr	ip, (pc, _addressof_$name)
				add	ip, pc, ip
				b	_passaddr_$name
			_addressof_$name:
				int	labelref(.$name)
			_passaddr_$name:
				patchinstr(patcharmdt, codesymb(_$name),
					str ip, (r4, #0))
			][
				_test _defp 'helios.C40 [
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
					-- ldabs32 [.$name] R_ADDR3 -- addr of name
					--					
					-- addr of name into its codetab offset
					--
					-- ldi	R_ADDR2, R_ATMP
					--
					-- NB/ must use 16 bits as RmLib has > 255 functions
					-- patchinstr(PATCHC40MASK16ADD,
					--	shift(-2, codesymb(_$name)),
					--	addi	0, R_ATMP)
					--	sti	R_ADDR3, *R_ATMP
					--
					--	no_codeinit_$name:
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
			_if _defp '_extern
				[global [_$name] _undef '_extern]
			-- !!! do not add transputer data access macro here !!!
		]

		_defq 'struct['type 'name]	-- define structure
		[
			_test _defp 'helios.TRAN [
				data _$name _div _eval[_eval[type$.sizeof]] 4
			][
				data [_$name], [_eval[_eval[type$.sizeof]]]
			]
			_if _defp '_extern
				[global [_$name] _undef '_extern]
			-- !!! do not add transputer data access macro here !!!
		]

		_defq 'vec['sizexxx namexxx]		-- define vector of bytes
		[
			_test _defp 'helios.TRAN [
				data _$namexxx _div sizexxx 4
			][
				data [_$namexxx], sizexxx
			]
			_if _defp '_extern
				[global [_$namexxx] _undef '_extern]
			-- !!! do not add transputer data access macro here !!!
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
				ldr	ip, (pc, _addressofD_$name)
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

					ldabs32_unsafe [.$name] R_ADDR3 -- addr of name

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
				ldr	ip, (pc, _addressofT_$name
				add	ip, pc, ip
				b	_passTaddr_$name
			_addressofT_$name:
				int	labelref(.$name)
			_passTaddr_$name:
	
				-- get address of target into r3
				-- hopefully addressability works out ok
				-- r2 points at our static area
				patchintr(patcharmdp, datasymb(_$name),
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

					-- copy code data to static data
					ldi	(size >> 2) - 1, rc
					rptb	end_loop_$name
						ldi	*R_ADDR3++(1), R_FT1
					end_loop_$name:	sti	R_FT1, *R_ADDR4++(1)
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
					-- only initialise on first data pass
					-- cmpi	0, R_A1
					-- bne	no_wordinit_$name

					ldi32	value R_T1
					-- get addr of name in static data
					StaticDataAddress name R_ATMP
					-- dump value into it
					sti	R_T1, *R_ATMP
					-- no_wordinit_$name:
				][
					-- transputer code
					ldc value
					ldl 0 stnl [_$name]
				]
			]
		]

		_defq 'initptrtab['name 'items 'stride]	-- init a table of ptrs to strings
		[
			_test _defp 'helios.arm	[
				-- get address of source into ip
				-- r2 points to static data area
				ldr	ip, (pc, _addressofp_$name)
				add	ip, pc, ip
				b	_passpaddr_$name
			_addressofp_$name:
				int	labelref(.$name)
			_passpaddr_$name:
	
				-- get address of target into r3
				-- hopefully addressability works out ok
				-- r2 points at our static area
				patchinstr(patcharmdp, datasymb(_$name),
					add	r3, r2, #0)
	
				-- hopefully addressability works out ok
				-- else use ldr r1,#items -- code const in pool
				mov	r1,#items
	
			_initptrtab_loop_$name:
				-- put ptr to source in target
				str	ip,(r3),#4	-- post inc to next
				add	ip,ip,#stride 	-- add stride to source
				subs	r1,r1	,#1
				bgt _initptrtab_loop_$name
			][
				_test _defp 'helios.C40 [
					-- data must be word aligned

					-- only initialise on first data pass
					-- cmpi	0, R_A1
					-- bne	no_ptrtabinit_$name

					-- addr of src name
					ldabs32_unsafe [.$name] R_ADDR3
					
					-- addr of dst name
					StaticDataAddress name R_ADDR4

					-- copy code data to static data
					ldi	items - 1, rc
					rptb	end2rloop_$name
						-- C40CAddress R_ADDR3, R_ATMP
						subi	R_BASE, R_ADDR3, R_ATMP
						lsh	2, R_ATMP
						sti	R_ATMP, *R_ADDR4++(1)
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

		-- ONLY initfunc ISN'T DEFINED by library.m
--		_defq 'initfunc['name]		  -- initialize function
--		[ -- functionally the same as initptr macro
--			_test _defp 'helios.arm	[
--				-- r2 points to static data area
--				-- in smt r4 points to code pointer table
--				ldr	ip, (pc, _addressoff_$name)
--				add	ip, pc, ip
--				b	_passfaddr_$name
--			_addressoff_$name:
--				.patch_word [labelref [.$name]]
--			_passfaddr_$name:
--				_test _defp '__SMT [
--					.patch_armdt [codesymb [_$name]]
--					str ip,(r4,#0)
--				][
--					.patch_armdt [datasymb [_$name]]
--					str ip,(r2,#0)
--				]
--			][
--				ldc .$name-2 ldpi ldl 0 stnl _$name
--			]
--		]

		_defq 'code['code_body]			-- general assembly code
		[
			code_body
		]
	
		_defq '_gencodetable [
			_if _defp '_codetableentry [	-- any to do ?
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
				--cmp r0,#2
				--movnes pc,lr
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
				Beq	_CodeTableInit

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

				-- Initialse code pointer table from _FuncTable, until NULL entry

				_CodeTableInit:

				-- R_ADDR1 == address base of modules code table
				patchinstr(PATCHC40MASK8ADD,
					shift(1, modnum),
					ldi	*+R_MT(1), R_ADDR1)

				ldabs16 [_FuncTableEnd] R_ADDR2 -- get hold of address of end of table
				B	_Loop1Start		-- jump into middle of loop

				_Loop1:				-- start of loop
				ADDI	R_ADDR2, R_TMP1		-- add address of field to contents of field
				STI	R_TMP1,	*R_ADDR1++(1)	-- store in module's code table
				_Loop1Start:			-- middle of loop
				LDI *'-'-R_ADDR2, R_TMP1	-- get value out of table of function offsets
				Bne	_Loop1	    		-- continue until 0 entry found
				B	R_LR			-- return				

				_FuncTable:			-- start of function offset table
					int 0			-- NULL entry in table indicates end of table
					_gencodetable		-- generate a table of function offsets
				_FuncTableEnd:			-- end of function offset table

			][
				-- transputer code
				-- !!! do not alter when updating from module.m
				ajw -1
				ldl 2 ldnl modnum stl 0
				ldl 3
				eqc 0
				cj ..INIT.0
				initcode
			..INIT.0:
				ajw 1
				ret
			]
		]
	
		_undef 'extern
		_undef 'trap
		_undef 'func
		_undef 'word
		_undef 'struct
		_undef 'vec
		_undef 'redefine
		_undef 'initptr
		_undef 'inittab
		_undef 'initword
		_undef 'initptrtab
		_undef 'code
	]
		-- Cut Here -- module.m -- Cut Here -- module.m -- Cut Here --
	
-- This macro just allows stubs for particular procedures to
-- be generated without any calling code.

_test _not _defp 'helios.C40	    -- or any CPU version that implements code stubs
[
	_defq 'stubs['body]
	[
		_defq 'stub['name]
		[
			_def '_call_stub
			[
				-- stack a stub for this procedure
				'_if '_not '_defp ['']name$.stubflag [
					-- only generate if not one already done
					.$name:
					_test _defp 'helios.arm	[
						patchinstr(patcharmdt, datamodule(_$name),
							ldr ip,(dp,#0))
						patchinstr(patcharmdt, codesymb(_$name),
							ldr ip,(ip,#0))
						mov pc,ip
					][
						_test _defp 'helios.TRAN [
							-- transputer code
							ldl 1
							ldnl 0
							ldnl @_$name
							ldnl _$name
							gcall
						][
							-- c40 stub generation
							ExternBigBranch name
						]
					]
				'_defq name$.stubflag 1	-- flag that a stub has been generated
				]
			]
		]
	
		body
		_genstubs
		_undef 'stub
	]

	_defq '_genstubs
	[
		_if _defp '_call_stub [		-- any to do ?
			_call_stub		-- generate a stub
			_undef '_call_stub	-- and pop it off stack
			_genstubs		-- this is a recursive macro !!
		]
	]
][
	_defq 'stubs['body] []
]

	-- static initialisation data, plus any code which may be added
	-- in the form of procedures.
	_defq 'LibData['body]
	[
		body
	]
]


-- Defines a trap number for each exported trap and initialises the trap table
_if  _defp 'make.traps [
	_set '_trap_number_offset 1

	_defq 'uses['libname] []

	_defq 'static['initcode] [
		_defq 'extern []	-- all traps are exported automatically

		_defq 'trap['name] [		  -- define trap

			-- define trap names trap number
			_defq [mytrap_$name] _trap_number_offset

			_test _defp 'helios.arm	[
				-- tmp points to trap table
				ldr	a1, (pc, _trapaddressof_$name)
				add	a1, pc, a1
				b	_trappassaddr_$name
			_trapaddressof_$name:
				int	labelref(.$name)
			_trappassaddr_$name:
				str	a1, (tmp, _eval [mytrap_$name] << 2)

			-- increment trap number
	                _set '_trap_number_offset [_add _trap_number_offset 1]

			][
				error "unknown cpu type for trap definition"
			]
		]

		-- ignore all other types of definition
		_defq 'func['name] []
		_defq 'word['name] []
		_defq 'struct['type 'name] []
		_defq 'vec['sizexxx namexxx] []
		_defq 'redefine['name] []
		_defq 'initptr['name] []
		_defq 'inittab['name 'size] []
		_defq 'initword['name 'value] []
		_defq 'initptrtab['name 'items 'stride] []
		_defq 'code['code_body] []
	
		_test _defp 'helios.arm	[
			-- start of trap table initialisation code

\-- File:	trapinit.a
\-- Subsystem:	Helios-ARM header files.
\-- Author:	P.A.Beskeen (Kernel library make system)
\-- Date:	As timestamped
\--
\-- Description: *AUTOMATICALLY* produced 'trap 'table 'initialisation.
\--		Never alter these definitions by hand. Always update
\--		the library definition file.
\--
\--
\-- (C) Copyright 1992 Perihelion Software Ltd.
\--     All Rights Reserved.
\
\include armexec.m
\
	Function InitTrapTable
\	
	\-- PCS style call
        mov     ip,sp
	stmfd   sp!,{a1,a2,a3,a4,v1,fp,ip,lk,pc}
	sub     fp,ip,#04
\	
	mov	tmp, SWI_table_base
\
			-- trap table initialisation code gets expanded here
			initcode
\	
	\-- PCS style return
	ldmea fp,{a2,a3,a4,v1,fp,sp,pc}^
\
\
\-- end of trapinit.a
\

		][
			error "Start of init code not defined for this cpu type"
		]
	
		_undef 'extern
		_undef 'trap
		_undef 'func
		_undef 'word
		_undef 'struct
		_undef 'vec
		_undef 'redefine
		_undef 'initptr
		_undef 'inittab
		_undef 'initword
		_undef 'initptrtab
		_undef 'code
	]
	
	-- no stubs
	_defq 'stubs['body]
	[ ]

	-- no data
	_defq 'LibData['body]
	[]
]


-- Outputs the trap stubs. These stubs are used when users call via the
-- module table to access the kernel functions.

-- @@@ This should be a temporary measure until we get some form of _trap()
-- function working from C so users can call the traps directly as if they
-- were functions.

_if  _defp 'make.trapstubs [

	_defq 'uses['libname] []

	_defq 'static['initcode] [
		_defq 'extern []	-- all traps are exported automatically

		_defq 'trap['name] [		  -- define trap stub
			_test _defp 'helios.arm	[
				_trapstub_$name:
					tst	lr, ModeMask
					swieq	TRAP_$name
					moveqs	pc, lr
					-- if called in non user mode save
					-- the link reg (note this will corrupt
					-- any arguments not passed in regs.
					stmfd	sp!, {lr}
					swi	TRAP_$name
					ldmfd	sp!, {pc}
			][
				error "unknown cpu type for trap stub definition"
			]
		]

		-- ignore all other types of definition
		_defq 'func['name] []
		_defq 'word['name] []
		_defq 'struct['type 'name] []
		_defq 'vec['sizexxx namexxx] []
		_defq 'redefine['name] []
		_defq 'initptr['name] []
		_defq 'inittab['name 'size] []
		_defq 'initword['name 'value] []
		_defq 'initptrtab['name 'items 'stride] []
		_defq 'code['code_body] []

\-- File:	trapstubs.a
\-- Subsystem:	Helios-ARM header files.
\-- Author:	P.A.Beskeen (Kernel library make system)
\-- Date:	As timestamped
\--
\-- Description: *AUTOMATICALLY* produced 'trap 'stubs
\--		Never alter these definitions by hand. Always update
\--		the library definition file.
\--
\--
\-- (C) Copyright 1992 Perihelion Software Ltd.
\--     All Rights Reserved.
\
\include trapdefs.m	-- include before ARM/trap.m to get latest trap defn's
\include ARM/trap.m
\
		align
\
		-- trap stubs get expanded here
		initcode
\
\-- end of 'trapstubs.m
\
		_undef 'extern
		_undef 'trap
		_undef 'func
		_undef 'word
		_undef 'struct
		_undef 'vec
		_undef 'redefine
		_undef 'initptr
		_undef 'inittab
		_undef 'initword
		_undef 'initptrtab
		_undef 'code
	]

	-- no stubs
	_defq 'stubs['body]
	[ ]

	-- no data
	_defq 'LibData['body]
	[]
]


-- Defines a trap number for each exported trap and outputs this as a C
-- header file.

_if  _defp 'make.trapCdefs [

	_defq 'uses['libname] []

	_defq 'static['initcode]
	[

		_defq 'extern []	-- all traps are exported automatically

                _set '_trap_number_offset 1

		_defq 'trap['name] [		  -- define trap stub

\#define TRAP_$name		(_trap_number_offset | TRAP_STDHELIOS)

			-- increment trap number
	                _set '_trap_number_offset [_add _trap_number_offset 1]
		]

		-- ignore all other types of definition
		_defq 'func['name] []
		_defq 'word['name] []
		_defq 'struct['type 'name] []
		_defq 'vec['sizexxx namexxx] []
		_defq 'redefine['name] []
		_defq 'initptr['name] []
		_defq 'inittab['name 'size] []
		_defq 'initword['name 'value] []
		_defq 'initptrtab['name 'items 'stride] []
		_defq 'code['code_body] []

/*
 * File:	trapdefs.h
 * Subsystem:	Helios-ARM header files.
 * Author:	P.A.Beskeen (Kernel library make system)
 * Date:	As timestamped
 *
 * Description: *AUTOMATICALLY* produced 'trap number definitions.
 *
 * WARNING:	Never alter these definitions by hand. Always update
 *		the library definition file.
 *
 *		<ARM/trap.h> should always be included in preference
 *		to this file.
 *
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 *     All Rights Reserved.
 */
\
\
\#ifndef __trapdefs_h
\#define __trapdefs_h
\
\#'include <'ARM/'trap.h>
\
\#define TRAP_USERSTACKSAVEAREA	0
\
\
		-- trap definitions get expanded here
		initcode
\
\#define TRAP_LASTTRAPNUMBER	_sub _trap_number_offset 1
\
\
\#endif /*__trapdefs_h*/
\
/* end of trapdefs.h */

		_undef 'extern
		_undef 'trap
		_undef 'func
		_undef 'word
		_undef 'struct
		_undef 'vec
		_undef 'redefine
		_undef 'initptr
		_undef 'inittab
		_undef 'initword
		_undef 'initptrtab
		_undef 'code
	]

	-- no stubs
	_defq 'stubs['body]
	[ ]

	-- no data
	_defq 'LibData['body]
	[]
]


-- Defines a trap number for each exported trap and outputs this as an ampp
-- header file.

_if  _defp 'make.trapAdefs [

        _set '_trap_number_offset 1

	_defq 'uses['libname] []

	_defq 'static['initcode]
	[

		_defq 'extern []	-- all traps are exported automatically

		_defq 'trap['name] [		  -- define trap stub

'_'defq ''TRAP_$name		\[(_trap_number_offset | 'TRAP_STDHELIOS)\]

			-- increment trap number
	                _set '_trap_number_offset [_add _trap_number_offset 1]
		]

		-- ignore all other types of definition
		_defq 'func['name] []
		_defq 'word['name] []
		_defq 'struct['type 'name] []
		_defq 'vec['sizexxx namexxx] []
		_defq 'redefine['name] []
		_defq 'initptr['name] []
		_defq 'inittab['name 'size] []
		_defq 'initword['name 'value] []
		_defq 'initptrtab['name 'items 'stride] []
		_defq 'code['code_body] []
	

\-- File:	trapdefs.m
\-- Subsystem:	Helios-ARM header files.
\-- Author:	P.A.Beskeen (Kernel library make system)
\-- Date:	As timestamped
\--
\-- Description: *AUTOMATICALLY* produced 'trap number definitions.
\--
\-- WARNING:	Never alter these definitions by hand. Always update
\--		the library definition file.
\--
\--		trap.m should always be included in preference to this file.
\--
\--
\-- (C) Copyright 1992 Perihelion Software Ltd.
\--     All Rights Reserved.
\
'_report \[''include 'trapdefs.m\]
'_def ''trapdefs.m_flag 1
\
'_'defq ''TRAP_USERSTACKSAVEAREA	0
\
\
		-- trap definitions get expanded here
		initcode
\
'_'defq ''TRAP_LASTTRAPNUMBER	_sub _trap_number_offset 1
\
\
\-- end of trapdefs.m
	
		_undef 'extern
		_undef 'trap
		_undef 'func
		_undef 'word
		_undef 'struct
		_undef 'vec
		_undef 'redefine
		_undef 'initptr
		_undef 'inittab
		_undef 'initword
		_undef 'initptrtab
		_undef 'code
	]
	
	-- no stubs
	_defq 'stubs['body]
	[ ]

	-- no data
	_defq 'LibData['body]
	[]

]

-- End of library.m
