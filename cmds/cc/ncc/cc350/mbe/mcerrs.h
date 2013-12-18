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


#define syserr_local_address 		"local_address %lx"
#define syserr_decode_branch_address	"decode_branch_address %.8lx"
#define syserr_unknown_diadic_op	"unknown diadic op code %02lx"
#define syserr_unknown_triadic_op	"unknown triadic op code %02lx"
#define syserr_non_float_dest		"destination of floating point op is not an extended precision register! (%s)"
#define syserr_unknown_indirect_mode	"unknown indirect addressing mode %lx"
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
#define syserr_unknown_addressing_mode	"Unknown addressing mode - %lx"
#define syserr_not_address_register	"relative reference through non-address register %ld"
#define syserr_unsupported_branch_type	"unsupported branch type %ld"
#define syserr_bad_block_length		"negative block length %ld"
#define syserr_large_shift		"attempting to shift by unreasonably large amount %ld"
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

#ifdef DEBUG
#define peep_null_format		"NULL format string"
#define peep_null_list			"passed NULL list"
#define peep_null_parameter		"passed NULL parameter"
#define peep_urg			"peepholer: urg urg"
#define peep_peepholer_empty		"attempting to emit from EMPTY peepholer!"
#define gen_bad_float_push		"back end: trying to float push a non-floating point register"
#define gen_bad_float_pop		"back end: trying to float pop a non-floating point register"
#define gen_non_FP_source		"back end: trying to perform a floating point operation with a non floating point source"
#define gen_non_FP_dest			"back end: trying to perform a floating point operation with a non floating point destination"
#define gen_NULL_param			"gen.c: passed a NULL parameter"
#endif /* DEBUG */
  
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

#define asm_unknown_indirect_addr	"disassembler: unknown indirect addressing mode %lx"
#define asm_dest_not_FP_reg		"disassembler: destination of FP op (%s) is not an FP register (%s)"
#define asm_bad_source_field		"disassembler: bad value in source field of register addressed op %s (%lx)"
#define asm_source_not_FP_reg		"disassembler: non FP source register %s for op %s"
#define asm_no_triadic_FP_immediates	"disassembler: triadic floating point immediate operations are impossible"
#define asm_lost_symbol			"disassembler: mislaid a symbol!"

#define heliobj_conflicting_offsets	"heliobj: symbol %s has offset %ld and follows symbol %s with offset %ld"
#define heliobj_repeated_export		"heliobj: cannot (yet) repeatedly export the same symbol"
#define heliobj_bad_sized_FP		"heliobj: bad size of floating point number %ld"
#define heliobj_unknown_data_type	"heliobj: unknown data type %lx"
#define heliobj_bad_length		"heliobj: bad length for packed data (%ld)"

#define gen_illegal_register		"back end: trying to evaluate illegal register 0x%lx"
#define gen_bad_addr_mode		"back end: bad addressing mode %ld"
#define gen_bad_displacement		"back end: illegal displacement"
#define gen_value_too_big		"back end: immediate value too large"
#define gen_unknown_addressing		"back end: unknown kind of triadic addressing %ld"
#define gen_non_word_offset		"back end: trying to add a non-word offset to a word pointer"
#define gen_FP_value_not_fit		"back end: float_immediate_op: asked to perform an op with an FP value that does not fit"
#define gen_void_compare		"back end: asked to perform void comparision"
#define gen_offset_reg_conflict		"back end: offset register conflict"
#define gen_too_many_to_skip		"back end: conditional_skip_instructions: too many instructions to skip! (%ld)"
#define gen_cannot_branch		"back end: cannot conditionally branch to offset %lx"
#define gen_not_identifier		"back end: is_function: not passed an identifier"
#define gen_not_supported		"back end: loading stack pointer with a constant address is not yet supported"
#define gen_address_of_non_function	"back end: trying to take address of non-function!"
#define gen_ptr_not_aligned		"back end: function pointer not word aligned!"
#define gen_offset_from_ptr		"back end: it is illegal to take an offset from a function pointer"
#define gen_offset_too_big		"back end: offset too big (offset = %ld)"
#define gen_need_patch			"back end: assembly code output is missing a patch directive"
#define gen_no_data_to_init		"back end: block_data_init: no data to init (%ld)"
#define gen_already_zero		"back end: asked to store 0 into already zero'ed memory"
#define gen_not_data			"back end: export_data_symbol: asked to export non-data: %s"
#define gen_fn_call_in_stub		"back end: call: function call to %s whilst generating a stub"
#define gen_cannot_call_offset		"back end: cannot call to offset %lx"
#define gen_already_set_label		"back end: trying to set an already set label!"
#define gen_copy_too_small		"back end: copying less than 4 bytes of memory (%ld)"
#define gen_copy_non_word_multiple	"back end: copy memory asked to copy a non word multiple %ld"
#define gen_URG				"back end: urg - this should not happen"
#ifdef TARGET_LACKS_UNSIGNED_FIX
#define gen_no_unsigned_fix		"back end: unsigned FIX not supported"
#endif
#define gen_failed_to_init		"back end: failed to initialise saved_regs"
#define gen_CALL_misaligned		"back end: {TAIL}CALL[KR] stack mis-aligned"
#ifdef TARGET_HAS_COND_EXEC
#define gen_mismatched_pending		"back end: branch %lx does not match pending condition %lx"
#define gen_pending_cond_exec		"back end: attempted to conditionally execute J_opcode %ld"
#endif
#define gen_no_byte_to_short		"back end: sign extend byte to short not yet supported"
#define gen_unknown_extend		"back end: unknown sign extend mode %ld"

#define debugger_cannot_open_output	"debugger: failed to open debug output file %s"
#define debugger_NULL_pointer		"debugger: passed a NULL pointer"
#define debugger_type			"debugger: db_type( %ld,0x%lx )"
#define debugger_no_block		"debugger: no current block or variable"
#define debugger_already_associated	"debugger: trying to add an already associated local variable"
#define debugger_unknown_storage	"debugger: unknown storage type"
#define debugger_bad_scope		"debugger: bad scope"
#define debugger_unresolved_variable	"debugger: unresolved variable class for %s"
#define debugger_fn_with_no_blocks	"debugger: function with no blocks"
#define heliobj_unknown_ref		"heliobj: unknown cross refernce type %lx"
#define heliobj_no_output_stream	"heliobj: no output stream"
  
/* end of mbe/mcerrs.h */
