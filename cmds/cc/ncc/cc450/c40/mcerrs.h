/*
 * c40/mcerrs.h - prototype for machine-specific error messages file
 */

%O  /* Ordinary error messages - mapped onto numeric codes */

/*
 * The following message is always machine specific since it may include
 * information about the source of support for the product.
 */
#define misc_disaster_banner   "\n\
*************************************************************************\n\
* The compiler has detected an internal inconsistency.  This can occur  *\n\
* because it has run out of a vital resource such as memory or disk     *\n\
* space or because there is a fault in it.  If you cannot easily alter  *\n\
* your program to avoid causing this rare failure, please contact your  *\n\
* dealer.  The dealer may be able to help you immediately and will be   *\n\
* able to report a suspected compiler fault to the support centre.      *\n\
*************************************************************************\n\
\n"

#define syserr_unknown_indirect_mode    "unknown indirect addressing mode %lx"
#define syserr_destination_not_FP	"destination of FP op (%s) is not an FP register (%s)"
#define syserr_bad_source_value		"bad value in source field of register addressed op %s (%lx)"
#define syserr_source_not_FP		"non FP source register %s for op %s (%lx)"
#define syserr_no_triadic_FP_imm	"triadic floating point immediate operations are impossible"
#define syserr_mislaid_a_symbol		"mislaid a symbol!"
#define syserr_local_address 		"local_address %lx"
#define syserr_decode_branch_address	"decode_branch_address %.8lx"
#define syserr_unknown_diadic_op	"unknown diadic op code %02lx"
#define syserr_unknown_triadic_op	"unknown triadic op code %02lx"
#define syserr_unknown_triadic_address	"unknown form of triadic addressing, instruction = %lx"
#define syserr_unknown_parallel_op	"unknown parallel op code %lx"
#define syserr_bad_parallel_addressing	"malformed parallel addressing discovered, instruction = %lx"
#define syserr_unknown_op_code		"unknown op code %lx"
#define syserr_asmlab 			"odd asmlab(%lx)"
#define syserr_display_asm 		"display_asm(%lx)"
#define syserr_asm_trailer 		"asm_trailer(%ld)"
#define syserr_datalen 			"asm_data len=%ld"
#define syserr_asm_trailer1 		"asm_trailer(%ldF%ld)"
#define syserr_asm_trailer2 		"asm_trailer(LIT_ADCON rpt)"
#define syserr_asm_confused 		"Assembler output confused - find '?'"
#define syserr_unknown_addressing_mode	"Unknown addressing mode - %x"
#define syserr_not_address_register	"relative reference through non-address register %d"
#define syserr_unsupported_branch_type	"unsopported branch type %ld"
#define syserr_bad_block_length		"negative block length %ld"
#define syserr_large_shift		"attempting to shift by unreasonably large amount %d"
#define syserr_bad_addressing_mode	"asked to build an invalid addressing mode"
#define syserr_bad_arg 			"Bad arg %lx"
#define syserr_local_addr 		"local_addr"
#define syserr_firstbit			"warning: attempting to find first bit set of 0 !"
#define syserr_displacement 		"displacement out of range %ld"
#define syserr_labref 			"unknown label reference type %.8lx"
#define syserr_enter 			"emit(J_ENTER %ld)"
#define syserr_data_sym			"Unable to find another data symbol at %ld"
#define syserr_show_inst 		"show_instruction (%#lx)"
#define syserr_movr 			"movr r,r"
#define syserr_offset_out_of_range	"offset (%d) is too big"
#define syserr_cannot_do_block_move	"cannot do block move"
#define syserr_bad_length		"bad length for block operation => %ld"
#define syserr_local_base 		"local_base %lx"
#define syserr_setsp_confused 		"SETSP confused %ld!=%ld %ld"
#define syserr_illegal_register         "hardware_register: trying to evaluate illegal register 0x%lx"
#define syserr_illegal_register2	"real_register: trying to evaluate illegal register 0x%lx"
#define syserr_illegal_displacement	"illegal displacement in indirect/indirect triadic addressing"
#define syserr_illegal_displacement2	"illegal displacement (non-0) in indirect triadic addressing"
#define syserr_illegal_displacement3	"illegal displacement (not 1) in indirect triadic addressing"
#define syserr_signed_val_too_large	"immediate value too large for signed triadic immediate addressing"
#define syserr_unsigned_val_too_large   "immediate value too large for unsigned triadic immediate addressing"
#define syserr_unknown_triadic		"unknown kind of triadic addressing %d"
#define syserr_displacement_out_of_range "displacement out of range for indirect/indirect triadic"
#define syserr_illegal_indirect		 "illegal indirect triadic addressing mode"
#define syserr_triadic_disp_oor		 "triadic displacement out of range"
#define syserr_unknown_op		 "out_triadic_op: what is this mess? %d"
#define syserr_non_word_offset		 "trying to add a non-word offset to a word pointer"
#define syserr_FP_value_not_fit		 "asked to perform an op with a floating point value that does not fit"
#define syserr_offset_reg_conflict	 "offset register conflict"
#define syserr_bad_stack_move		 "stack move by a non-multiple of %d"
#define syserr_too_many_to_skip		 "too many instructions to skip! (%ld)"
#define syserr_cannot_cond_branch	 "cannot conditionally branch to offset %lx"
#define syserr_not_identifier		 "is_function: not passed an identifier"
#define syserr_fixed_SP			 "loading stack pointer with a constant address is not yet supported"
#define syserr_offset_from_fn		 "it is illegal to take an offset from a function pointer"
#define syserr_offset_too_big		 "external data: offset too big (offset = %ld)"
#define syserr_need_patch		 "assembly code output is missing a patch directive"
#define syserr_no_data_to_init		 "block_data_init: no data to init (%ld)"
#define syserr_store_zero		 "asked to store 0 into already zero'ed memory"
#define syserr_export_non_data		 "export_data_symbol: asked to export non-data: %s"
#define syserr_offset_too_large		 "cannot initialise large data offset"
#define syserr_cannot_call_offset	 "cannot call to offset %lx"
#define syserr_already_set_label	 "trying to set an already set label!"
#define syserr_copy_less_than_four	 "copying less than 4 bytes of memory (%ld)"
#define syserr_non_word_multiple	 "copy memory asked to copy a non word multiple %ld"
#define syserr_urg			 "urg - this should not happen"
#define syserr_init_failed		 "failed to initialise saved_regs"
#define syserr_stack_mis_aligned	 "stack mis-aligned\n"
#define syserr_not_match_pending	 "branch %lx does not match pending condition %lx\n"
#define syserr_adjust_non_word		 "trying to use a non-word offset with a word pointer"
#define syserr_no_byte_short_sign_extend "sign extend byte to short not yet supported"
#define syserr_unknown_sign_extend_mode	 "unknown sign extend mode %ld"
  
#ifdef TARGET_HAS_COND_EXEC
#define syserr_no_fast_compare		 "cannot do fast conditional compares"
#define syserr_cond_exec		 "attempted to conditionally execute J_opcode %ld\n"
#endif
  
#ifdef TARGET_LACKS_UNSIGNED_FIX
#define syserr_no_unsigned_fix		 "unsigned FIX not supported"
#endif
  
#define syserr_non_float_dest		 "trying to perform a floating point operation with a non floating point destination"

#ifdef DEBUG
#define syserr_push_non_float		 "trying to float push a non-floating point register %x"
#define syserr_pop_non_float		 "trying to float pop a non-floating point register %x"
#define syserr_push_non_double		 "trying to double push a non-floating point register %x"
#define syserr_pop_non_double		 "trying to double pop a non-floating point register %x"
#define syserr_non_float_source		 "trying to perform a floating point operation with a non floating point source"
#define syserr_void_compare		 "asked to perform void comparision"  
#define syserr_null_pointer		 "gen.c: is_function: passed a NULL pointer"  
#define syserr_non_function		 "load_address_constant: trying to take address of non-function!"
#define syserr_non_aligned_fn		 "load_address_constant: function pointer not word aligned!"
#define syserr_fn_call_in_stub		 "call: function call to %s whilst generating a stub"
#define debug_null_type_expr		 "db_type: passed a NULL TypeExpr pointer"
#define debug_null_parameter		 "update_var: NULL parameter"
#endif /* DEBUG */

#define debug_cannot_open_output	 "failed to open debug output file %s"
#define debug_db_type			 "db_type( %ld,0x%lx )"
#define debug_no_block			 "no current block or variable"
#define debug_already_associated	 "trying to add an already associated local variable"
#define debug_unknown_storage_type	 "unknown storage type"
#define debug_unknown_auto_storage	 "unknown type of auto storage"
#define debug_bad_scope			 "bad scope"
#define debug_unresolved_var_class	 "unresolved variable class for %s"
#define debug_no_blocks			 "function with no blocks"

#define peep_null_format		"NULL format string"
#define peep_null_list			"passed NULL list"
#define peep_null_parameter		"passed NULL parameter"
#define peep_urg			"peepholer: urg urg"
#define peep_peepholer_empty		"attempting to emit from EMPTY peepholer!"
#define peep_unknown_push		"peepholer: unknown push type"
#define peep_out_of_memory		"peepholer: out of memory"
#define peep_no_offset			"peepholer: no offset in branch!"
#define peep_unknown_delay_type		"peepholer: unknown delay type"
#define peep_unknown_type		"peepholer: asked to emit an unknown type %d"
#define peep_unexpected_back_ref	"peepholer: unexpected backwards reference"
#define peep_fwd_and_cross_ref		"peepholer: pending forward and cross references for same instruction!"
#define peep_special_pending_cross	"peepholer: special instruction type had a pending cross reference!"
#define peep_special_pending_fwd	"peepholer: special instruction type had a pending forward reference!"
#define peep_elim_clash			"peepholer: attempting to eliminate instruction whilst a reference is still pending"
#define peep_non_existant_xfer		"peepholer: trying to eliminate non-existant register transfer"
#define peep_cross_ref_pending		"peepholer: attempting to flush peepholer whilst a cross ref is still pending"
#define peep_fwd_ref_pending		"peepholer: attempting to flush peepholer whilst a forward ref is still pending"
#define peep_failed_reset_count		"peepholer: flush has failed to reset peep count"

  
/* end of mbe/mcerrs.h */
