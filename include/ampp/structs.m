--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- structs.m                                                            --
--                                                                      --
--      Data structuring macros                                         --
--                                                                      --
--      Author: NHG 20-May-87						--
--                                                                      --
--	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include structs.m]
_def 'structs.m_flag 1

include basic.m

_if _defp 'helios.TRAN [
	-- transputer data access modifiers

	_defq '_byte_access lb                  -- changed to sb by store
	_defq '_acctype ld                      -- changed to st by store

	_defq 'store['store_var] [              -- modify variable access to store it
		_defq '_acctype st
		_defq '_byte_access sb
		store_var
		_undef '_acctype
		_undef '_byte_access
	]

	_defq '_ptrtype []                      -- changed to p by ptr
	_defq 'ptr['ptr_var] [                  -- modify access to load pointer
		_defq '_ptrtype ['p]
			_defq '_byte_access []
			ptr_var
			_undef '_ptrtype
			_undef '_byte_access
	]

	-- get pointer to function
	_defq 'fnptr['func] [ldc .$func - 2 ldpi ]
]


-- Structure macros
--
-- Tranputer version defines struct.element to access contents of element.
-- Other processors define struct.element to return the offset of the
-- element within the structure.
-- The 'C40 version doesn't allow byte offsets all offsets returned
-- are in terms of words apart from the .sizeof which is still defined in bytes

_defq 'struct[struct_name 'struct_def]
[
	_if _defp 'helios.C40 [
	        _set '_byte_struct_offset 0
	]
        _set '_struct_offset 0

        _defq 'word['element] [
		_test _defp 'helios.C40 [
			_if [_ne _byte_struct_offset 0] [
				_report ['Error: 'word 'not 'aligned 'to 'word 'address]
				'Error: 'word 'not 'aligned 'to 'word 'address
			]
		][
			-- align to word boundary
        	        _set '_struct_offset [_mul 4 [_div [_add 3 _struct_offset] 4]]
		]
		_test _defp 'helios.TRAN [
	                _def [struct_name$.$element]
				['[_acctype$nl$_ptrtype] _div _struct_offset 4]
		][
	                _def [struct_name$.$element]
				[_struct_offset]
		]
		_test _defp 'helios.C40 [
	                _set '_struct_offset [_add _struct_offset 1]
		][
	                _set '_struct_offset [_add _struct_offset 4]
		]
        ]
        _defq 'struct['name 'element]
        [
		_test _defp 'helios.C40 [
			_if [_ne _byte_struct_offset 0] [
				_report ['Error: 'struct 'not 'aligned 'for 'word 'address]
				'Error: 'struct 'not 'aligned 'for 'word 'address
			]
		][
			-- align to word boundary
		        _set '_struct_offset [_mul 4 [_div [_add 3 _struct_offset] 4]]
		]
		_test _defp 'helios.TRAN [
	                _def [struct_name$.$element]
	        	        [ldnlp _div _struct_offset 4]
		][
	                _def [struct_name$.$element]
				[_struct_offset]
		]

		_test _defp 'helios.C40 [
	                _set '_struct_offset [
				_add _struct_offset _div [_eval[_eval[name$.sizeof]]] 4
			]
		][
	                _set '_struct_offset
				[_add _struct_offset _eval[_eval[name$.sizeof]]]
		]
        ]
	-- 'C40 vectors (arrays) size are still defined in terms of bytes but
	-- the sizes are word aligned and word offsets are returned
        _defq 'vec['size 'element]
        [
		_test _defp 'helios.C40 [
			_if [_ne _byte_struct_offset 0] [
				_report ['Error: 'vec 'not 'aligned 'for 'word 'address]
				'Error: 'vec 'not 'aligned 'for 'word 'address
			]
		][
			-- align to word boundary
	                _set '_struct_offset [_mul 4 [_div [_add 3 _struct_offset] 4]]
		]
		_if _defp 'helios.TRAN [
	                _def [struct_name$.$element]
	        	        [ldnlp _div _struct_offset 4]
		][
	                _def [struct_name$.$element]
				[_struct_offset]
		]
		_test _defp 'helios.C40 [
	                _set '_struct_offset [_add _struct_offset [_div [_add 3 size] 4]]
		][
	                _set '_struct_offset [_add _struct_offset size]
		]
        ]
	_defq 'byte['element] [
		_test _defp 'helios.C40 [
	                _def [struct_name$.$element$.w_offset]
				[_struct_offset]

	                _def [struct_name$.$element$.shift]
				[_mul _byte_struct_offset 8]

			_test [_ge _byte_struct_offset 3] [
	        	        _set '_byte_struct_offset 0
	        	        _set '_struct_offset [_add _struct_offset 1]
			][
	        	        _set '_byte_struct_offset [_add _byte_struct_offset 1]
			]

		][
			_test _defp 'helios.TRAN [
		                _def [struct_name$.$element]
					[adc _struct_offset '_byte_access]
			][
		                _def [struct_name$.$element]
					[_struct_offset]
			]
        	        _set '_struct_offset [_add _struct_offset 1]
	       	]
	]
	_defq 'short['element] [
		_test _defp 'helios.C40 [
	                _def [struct_name$.$element$.w_offset]
				[_struct_offset]

	                _def [struct_name$.$element$.shift]
				[_mul _byte_struct_offset 8]

			_test [_ge _byte_struct_offset 1] [
	        	        _set '_byte_struct_offset 0
	        	        _set '_struct_offset [_add _struct_offset 1]
			][
	        	        _set '_byte_struct_offset [_add _byte_struct_offset 2]
			]
		][
	                _def [struct_name$.$element]
				[_struct_offset]

        	        _set '_struct_offset [_add _struct_offset 2]
	       	]
	]
        struct_def
	_test _defp 'helios.C40 [
		-- 'C40 offsets in terms of words, sizeof in bytes!
	        _def [struct_name$.sizeof] [_mul _struct_offset 4]
	][
	        _def [struct_name$.sizeof] [_struct_offset]
	]
        _undef 'word
        _undef 'struct
	_undef 'vec
        _undef 'byte
]


-- End of structs.m
