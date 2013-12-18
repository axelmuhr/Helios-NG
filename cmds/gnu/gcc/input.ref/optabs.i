
extern int target_flags;

enum reg_class { NO_REGS, LO_FPA_REGS, FPA_REGS, FP_REGS,
  FP_OR_FPA_REGS, DATA_REGS, DATA_OR_FPA_REGS, DATA_OR_FP_REGS,
  DATA_OR_FP_OR_FPA_REGS, ADDR_REGS, GENERAL_REGS,
  GENERAL_OR_FPA_REGS, GENERAL_OR_FP_REGS, ALL_REGS,
  LIM_REG_CLASSES };

extern enum reg_class regno_reg_class[];

enum rtx_code  {

  UNKNOWN , 

  NIL , 

  EXPR_LIST , 

  INSN_LIST , 

  MATCH_OPERAND , 

  MATCH_DUP , 

  MATCH_OPERATOR , 

  DEFINE_INSN , 

  DEFINE_PEEPHOLE , 

  DEFINE_COMBINE , 

  DEFINE_EXPAND , 

  SEQUENCE , 

  ADDRESS , 

  INSN , 

  JUMP_INSN , 

  CALL_INSN , 

  BARRIER , 

  CODE_LABEL , 

  NOTE , 

  INLINE_HEADER , 

  PARALLEL , 

  ASM_INPUT , 

  ASM_OPERANDS , 

  ADDR_VEC , 

  ADDR_DIFF_VEC , 

  SET , 

  USE , 

  CLOBBER , 

  CALL , 

  RETURN , 

  CONST_INT , 

  CONST_DOUBLE , 

  CONST , 

  PC , 

  REG , 

  SUBREG , 

  STRICT_LOW_PART , 

  MEM , 

  LABEL_REF , 

  SYMBOL_REF , 

  CC0 , 

  QUEUED , 

  IF_THEN_ELSE , 

  COMPARE , 

  PLUS , 

  MINUS , 

  NEG , 

  MULT , 

  DIV , 
  MOD , 

  UMULT , 
  UDIV , 
  UMOD , 

  AND , 

  IOR , 

  XOR , 

  NOT , 

  LSHIFT , 
  ASHIFT , 
  ROTATE , 

  ASHIFTRT , 
  LSHIFTRT , 
  ROTATERT , 

  PRE_DEC , 
  PRE_INC , 
  POST_DEC , 
  POST_INC , 

  NE , 
  EQ , 
  GE , 
  GT , 
  LE , 
  LT , 
  GEU , 
  GTU , 
  LEU , 
  LTU , 

  SIGN_EXTEND , 

  ZERO_EXTEND , 

  TRUNCATE , 

  FLOAT_EXTEND , 
  FLOAT_TRUNCATE , 

  FLOAT , 

  FIX , 

  UNSIGNED_FLOAT , 

  UNSIGNED_FIX , 

  ABS , 

  SQRT , 

  FFS , 

  SIGN_EXTRACT , 

  ZERO_EXTRACT , 

  LAST_AND_UNUSED_RTX_CODE};	 

extern int rtx_length[];

extern char *rtx_name[];

extern char *rtx_format[];

enum machine_mode {

 VOIDmode, 

 QImode, 		 
 HImode, 

 PSImode, 
 SImode, 
 PDImode, 
 DImode, 
 TImode, 
 QFmode, 
 HFmode, 		 
 SFmode, 
 DFmode, 
 XFmode, 	 
 TFmode, 
 CQImode, 
 CHImode, 	 
 CSImode, 
 CDImode, 
 CTImode, 
 CQFmode, 
 CHFmode, 	 
 CSFmode, 
 CDFmode, 
 CXFmode, 
 CTFmode, 

 BImode, 	 

 BLKmode, 

 EPmode, 

MAX_MACHINE_MODE };

extern char *mode_name[];

enum mode_class { MODE_RANDOM, MODE_INT, MODE_FLOAT,
		  MODE_COMPLEX_INT, MODE_COMPLEX_FLOAT, MODE_FUNCTION };

extern enum mode_class mode_class[];

extern int mode_size[];

extern int mode_unit_size[];

typedef union rtunion_def
{
  int rtint;
  char *rtstr;
  struct rtx_def *rtx;
  struct rtvec_def *rtvec;
  enum machine_mode rttype;
} rtunion;

typedef struct rtx_def
{

  enum rtx_code code : 16;

  enum machine_mode mode : 8;

  unsigned int jump : 1;
  unsigned int call : 1;

  unsigned int unchanging : 1;

  unsigned int volatil : 1;

  unsigned int in_struct : 1;

  unsigned int used : 1;

  unsigned integrated : 1;

  rtunion fld[1];
} *rtx;

typedef struct rtvec_def{
  unsigned num_elem;		 
  rtunion elem[1];
} *rtvec;

enum reg_note { REG_DEAD = 1, REG_INC = 2, REG_EQUIV = 3, REG_WAS_0 = 4,
		REG_EQUAL = 5, REG_RETVAL = 6, REG_LIBCALL = 7,
		REG_NONNEG = 8 };

extern char *reg_note_name[];

extern char *note_insn_name[];

extern rtx rtx_alloc ();
extern rtvec rtvec_alloc ();
extern rtx find_reg_note ();
extern rtx gen_rtx ();
extern rtx copy_rtx ();
extern rtvec gen_rtvec ();
extern rtvec gen_rtvec_v ();
extern rtx gen_reg_rtx ();
extern rtx gen_label_rtx ();
extern rtx gen_inline_header_rtx ();
extern rtx gen_lowpart ();
extern rtx gen_highpart ();
extern int subreg_lowpart_p ();
extern rtx make_safe_from ();
extern rtx memory_address ();
extern rtx get_insns ();
extern rtx get_last_insn ();
extern rtx start_sequence ();
extern rtx gen_sequence ();
extern rtx expand_expr ();
extern rtx output_constant_def ();
extern rtx immed_real_const ();
extern rtx immed_real_const_1 ();
extern rtx immed_double_const ();
extern rtx force_const_double_mem ();
extern rtx force_const_mem ();
extern rtx get_parm_real_loc ();
extern rtx assign_stack_local ();
extern rtx protect_from_queue ();
extern void emit_queue ();
extern rtx emit_move_insn ();
extern rtx emit_insn ();
extern rtx emit_jump_insn ();
extern rtx emit_call_insn ();
extern rtx emit_call_insn_before ();
extern rtx emit_insn_before ();
extern rtx emit_insn_after ();
extern rtx emit_label ();
extern rtx emit_barrier ();
extern rtx emit_note ();
extern rtx emit_line_note ();
extern rtx emit_line_note_force ();
extern rtx prev_real_insn ();
extern rtx next_real_insn ();
extern rtx next_nondeleted_insn ();
extern rtx plus_constant ();
extern rtx find_equiv_reg ();
extern rtx delete_insn ();
extern rtx adj_offsetable_operand ();

extern int max_parallel;

extern int asm_noperands ();
extern char *decode_asm_operands ();

extern enum reg_class reg_preferred_class ();

extern rtx get_first_nonparm_insn ();

extern rtx pc_rtx;
extern rtx cc0_rtx;
extern rtx const0_rtx;
extern rtx const1_rtx;
extern rtx fconst0_rtx;
extern rtx dconst0_rtx;

extern rtx stack_pointer_rtx;
extern rtx frame_pointer_rtx;
extern rtx arg_pointer_rtx;
extern rtx struct_value_rtx;
extern rtx struct_value_incoming_rtx;
extern rtx static_chain_rtx;
extern rtx static_chain_incoming_rtx;

enum tree_code {

  ERROR_MARK, 

  IDENTIFIER_NODE, 

  OP_IDENTIFIER, 

  TREE_LIST, 

  VOID_TYPE, 	 

  INTEGER_TYPE, 

  REAL_TYPE, 

  COMPLEX_TYPE, 

  ENUMERAL_TYPE, 

  BOOLEAN_TYPE, 

  CHAR_TYPE, 

  POINTER_TYPE, 

  OFFSET_TYPE, 

  REFERENCE_TYPE, 

  METHOD_TYPE, 

  FILE_TYPE, 

  ARRAY_TYPE, 

  SET_TYPE, 

  STRING_TYPE, 

  RECORD_TYPE, 

  UNION_TYPE, 	 

  FUNCTION_TYPE, 

  LANG_TYPE, 

  LABEL_STMT, 

  GOTO_STMT, 

  RETURN_STMT, 

  EXPR_STMT, 

  WITH_STMT, 

  LET_STMT, 

  IF_STMT, 

  EXIT_STMT, 

  CASE_STMT, 

  LOOP_STMT, 

  COMPOUND_STMT, 

  ASM_STMT, 

  INTEGER_CST, 

  REAL_CST, 

  COMPLEX_CST, 

  STRING_CST, 

  FUNCTION_DECL, 
  LABEL_DECL, 
  CONST_DECL, 
  TYPE_DECL, 
  VAR_DECL, 
  PARM_DECL, 
  RESULT_DECL, 
  FIELD_DECL, 
  FRIEND_DECL, 

  COMPONENT_REF, 

  INDIRECT_REF, 

  OFFSET_REF, 

  BUFFER_REF, 

  ARRAY_REF, 

  CONSTRUCTOR, 

  COMPOUND_EXPR, 

  MODIFY_EXPR, 

  INIT_EXPR, 

  NEW_EXPR, 

  DELETE_EXPR, 

  COND_EXPR, 

  CALL_EXPR, 

  METHOD_CALL_EXPR, 

  WITH_CLEANUP_EXPR, 

  PLUS_EXPR, 
  MINUS_EXPR, 
  MULT_EXPR, 

  TRUNC_DIV_EXPR, 

  CEIL_DIV_EXPR, 

  FLOOR_DIV_EXPR, 

  ROUND_DIV_EXPR, 

  TRUNC_MOD_EXPR, 
  CEIL_MOD_EXPR, 
  FLOOR_MOD_EXPR, 
  ROUND_MOD_EXPR, 

  RDIV_EXPR, 

  FIX_TRUNC_EXPR, 
  FIX_CEIL_EXPR, 
  FIX_FLOOR_EXPR, 
  FIX_ROUND_EXPR, 

  FLOAT_EXPR, 

  EXPON_EXPR, 

  NEGATE_EXPR, 

  MIN_EXPR, 
  MAX_EXPR, 
  ABS_EXPR, 
  FFS_EXPR, 

  LSHIFT_EXPR, 
  RSHIFT_EXPR, 
  LROTATE_EXPR, 
  RROTATE_EXPR, 

  BIT_IOR_EXPR, 
  BIT_XOR_EXPR, 
  BIT_AND_EXPR, 
  BIT_ANDTC_EXPR, 
  BIT_NOT_EXPR, 

  TRUTH_ANDIF_EXPR, 
  TRUTH_ORIF_EXPR, 
  TRUTH_AND_EXPR, 
  TRUTH_OR_EXPR, 
  TRUTH_NOT_EXPR, 

  LT_EXPR, 
  LE_EXPR, 
  GT_EXPR, 
  GE_EXPR, 
  EQ_EXPR, 
  NE_EXPR, 

  IN_EXPR, 
  SET_LE_EXPR, 
  CARD_EXPR, 
  RANGE_EXPR, 

  CONVERT_EXPR, 

  NOP_EXPR, 

  SAVE_EXPR, 

  RTL_EXPR, 

  ADDR_EXPR, 

  REFERENCE_EXPR, 

  WRAPPER_EXPR, 
  ANTI_WRAPPER_EXPR, 

  ENTRY_VALUE_EXPR, 

  COMPLEX_EXPR, 

  CONJ_EXPR, 

  REALPART_EXPR, 
  IMAGPART_EXPR, 

  PREDECREMENT_EXPR, 
  PREINCREMENT_EXPR, 
  POSTDECREMENT_EXPR, 
  POSTINCREMENT_EXPR, 

  LAST_AND_UNUSED_TREE_CODE	 

};

extern char *tree_code_type[];

extern int tree_code_length[];

enum built_in_function
{
  NOT_BUILT_IN,
  BUILT_IN_ALLOCA,
  BUILT_IN_ABS,
  BUILT_IN_FABS,
  BUILT_IN_LABS,
  BUILT_IN_FFS,
  BUILT_IN_DIV,
  BUILT_IN_LDIV,
  BUILT_IN_FFLOOR,
  BUILT_IN_FCEIL,
  BUILT_IN_FMOD,
  BUILT_IN_FREM,
  BUILT_IN_MEMCPY,
  BUILT_IN_MEMCMP,
  BUILT_IN_MEMSET,
  BUILT_IN_FSQRT,
  BUILT_IN_GETEXP,
  BUILT_IN_GETMAN,
  BUILT_IN_SAVEREGS,

  BUILT_IN_NEW,
  BUILT_IN_VEC_NEW,
  BUILT_IN_DELETE,
  BUILT_IN_VEC_DELETE,
};

typedef union tree_node *tree;

struct tree_common
{
  int uid;
  union tree_node *chain;
  union tree_node *type;
  enum tree_code code : 8;

  unsigned external_attr : 1;
  unsigned public_attr : 1;
  unsigned static_attr : 1;
  unsigned volatile_attr : 1;
  unsigned packed_attr : 1;
  unsigned readonly_attr : 1;
  unsigned literal_attr : 1;
  unsigned nonlocal_attr : 1;
  unsigned permanent_attr : 1;
  unsigned addressable_attr : 1;
  unsigned regdecl_attr : 1;
  unsigned this_vol_attr : 1;
  unsigned unsigned_attr : 1;
  unsigned asm_written_attr: 1;
  unsigned inline_attr : 1;
  unsigned used_attr : 1;
  unsigned lang_flag_1 : 1;
  unsigned lang_flag_2 : 1;
  unsigned lang_flag_3 : 1;
  unsigned lang_flag_4 : 1;
};

struct tree_int_cst
{
  char common[sizeof (struct tree_common)];
  long int_cst_low;
  long int_cst_high;
};

extern double ldexp ();

extern double atof ();

union real_extract 
{
  double  d;
  int i[sizeof (double ) / sizeof (int)];
};

struct tree_real_cst
{
  char common[sizeof (struct tree_common)];
  struct rtx_def *rtl;	 

  double  real_cst;
};

struct tree_string
{
  char common[sizeof (struct tree_common)];
  struct rtx_def *rtl;	 

  int length;
  char *pointer;
};

struct tree_complex
{
  char common[sizeof (struct tree_common)];
  struct rtx_def *rtl;	 

  union tree_node *real;
  union tree_node *imag;
};

struct tree_identifier
{
  char common[sizeof (struct tree_common)];
  int length;
  char *pointer;
  union tree_node *global_value;
  union tree_node *local_value;
  union tree_node *label_value;
  union tree_node *implicit_decl;
  union tree_node *error_locus;
};

struct tree_list
{
  char common[sizeof (struct tree_common)];
  union tree_node *purpose;
  union tree_node *value;
};

struct tree_exp
{
  char common[sizeof (struct tree_common)];
  int complexity;
  union tree_node *operands[1];
};

struct tree_type
{
  char common[sizeof (struct tree_common)];
  union tree_node *values;
  union tree_node *sep;
  union tree_node *size;

  enum machine_mode mode : 8;
  unsigned char size_unit;
  unsigned char align;
  unsigned char sep_unit;

  union tree_node *pointer_to;
  union tree_node *reference_to;
  int parse_info;
  int symtab_address;
  union tree_node *name;
  union tree_node *max;
  union tree_node *next_variant;
  union tree_node *main_variant;
  union tree_node *basetypes;
  union tree_node *noncopied_parts;
  struct lang_type *lang_specific;
};

struct tree_decl
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *size;
  enum machine_mode mode;
  unsigned char size_unit;
  unsigned char align;
  unsigned char voffset_unit;
  union tree_node *name;
  union tree_node *context;
  int offset;
  union tree_node *voffset;
  union tree_node *arguments;
  union tree_node *result;
  union tree_node *initial;
  struct rtx_def *rtl;	 

  int frame_size;		 
  struct rtx_def *saved_insns;	 

  int block_symtab_address;
  struct lang_decl *lang_specific;
};

struct tree_stmt
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *body;
};

struct tree_if_stmt
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *cond, *thenpart, *elsepart;
};

struct tree_bind_stmt
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *body, *vars, *supercontext, *bind_size, *type_tags;
};

struct tree_case_stmt
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *index, *case_list;
};

union tree_node
{
  struct tree_common common;
  struct tree_int_cst int_cst;
  struct tree_real_cst real_cst;
  struct tree_string string;
  struct tree_complex complex;
  struct tree_identifier identifier;
  struct tree_decl decl;
  struct tree_type type;
  struct tree_list list;
  struct tree_exp exp;
  struct tree_stmt stmt;
  struct tree_if_stmt if_stmt;
  struct tree_bind_stmt bind_stmt;
  struct tree_case_stmt case_stmt;
};
extern char *oballoc ();
extern char *permalloc ();

extern tree make_node ();

extern tree copy_node ();

extern tree get_identifier ();

extern tree build_int_2 ();
extern tree build_real ();
extern tree build_real_from_string ();
extern tree build_real_from_int_cst ();
extern tree build_complex ();
extern tree build_string ();
extern tree build ();
extern tree build_nt ();
extern tree build_tree_list ();
extern tree build_op_identifier ();
extern tree build_decl ();
extern tree build_let ();

extern tree make_signed_type ();
extern tree make_unsigned_type ();
extern void fixup_unsigned_type ();
extern tree build_pointer_type ();
extern tree build_reference_type ();
extern tree build_index_type ();
extern tree build_array_type ();
extern tree build_function_type ();
extern tree build_method_type ();
extern tree build_offset_type ();
extern tree array_type_nelts ();

extern tree build_binary_op ();
extern tree build_indirect_ref ();
extern tree build_unary_op ();

extern tree build_type_variant ();

extern void layout_type ();

extern tree type_hash_canon ();

extern void layout_decl ();

extern tree fold ();

extern tree combine ();

extern tree convert ();
extern tree convert_units ();
extern tree size_in_bytes ();
extern tree genop ();
extern tree build_int ();
extern tree get_pending_sizes ();

extern tree sizetype;

extern tree chainon ();

extern tree tree_cons (), perm_tree_cons (), temp_tree_cons ();
extern tree saveable_tree_cons ();

extern tree tree_last ();

extern tree nreverse ();

extern int list_length ();

extern int integer_zerop ();

extern int integer_onep ();

extern int integer_all_onesp ();

extern int type_unsigned_p ();

extern int staticp ();

extern int lvalue_or_else ();

extern tree save_expr ();

extern tree stabilize_reference ();

extern tree get_unwidened ();

extern tree get_narrower ();

extern tree type_for_size ();

extern tree unsigned_type ();

extern tree signed_type ();

extern tree get_floating_type ();

extern char *function_cannot_inline_p ();

extern tree integer_zero_node;

extern tree integer_one_node;

extern tree size_zero_node;

extern tree size_one_node;

extern tree null_pointer_node;

extern tree error_mark_node;

extern tree void_type_node;

extern tree integer_type_node;

extern tree unsigned_type_node;

extern tree char_type_node;

extern char *input_filename;

extern int pedantic;

extern int immediate_size_expand;

extern tree current_function_decl;

extern int current_function_calls_setjmp;

extern int all_types_permanent;

extern tree expand_start_stmt_expr ();
extern tree expand_end_stmt_expr ();
extern void expand_expr_stmt(), clear_last_expr();
extern void expand_label(), expand_goto(), expand_asm();
extern void expand_start_cond(), expand_end_cond();
extern void expand_start_else(), expand_end_else();
extern void expand_start_loop(), expand_start_loop_continue_elsewhere();
extern void expand_loop_continue_here();
extern void expand_end_loop();
extern int expand_continue_loop();
extern int expand_exit_loop(), expand_exit_loop_if_false();
extern int expand_exit_something();

extern void expand_start_delayed_expr ();
extern tree expand_end_delayed_expr ();
extern void expand_emit_delayed_expr ();

extern void expand_null_return(), expand_return();
extern void expand_start_bindings(), expand_end_bindings();
extern void expand_start_case(), expand_end_case();
extern int pushcase(), pushcase_range ();
extern void expand_start_function(), expand_end_function();

extern char *main_input_filename;

enum debugger { NO_DEBUG = 0, GDB_DEBUG = 1, DBX_DEBUG = 2, SDB_DEBUG = 3,
		EXTENDED_DBX_DEBUG = 4 };

extern enum debugger write_symbols;

extern int use_gdb_dbx_extensions;

extern int optimize;

extern int obey_regdecls;

extern int quiet_flag;

extern int inhibit_warnings;

extern int extra_warnings;

extern int warn_unused;

extern int warn_shadow;

extern int warn_switch;

extern int warn_id_clash;
extern int id_clash_len;

extern int profile_flag;

extern int profile_block_flag;

extern int pedantic;

extern int flag_caller_saves;

extern int flag_pcc_struct_return;

extern int flag_force_mem;

extern int flag_force_addr;

extern int flag_defer_pop;

extern int flag_float_store;

extern int flag_combine_regs;

extern int flag_strength_reduce;

extern int flag_writable_strings;

extern int flag_no_function_cse;

extern int flag_omit_frame_pointer;

extern int frame_pointer_needed;

extern int flag_no_peephole;

extern int flag_volatile;

extern int flag_inline_functions;

extern int flag_keep_inline_functions;

extern int flag_syntax_only;

extern int flag_shared_data;

extern rtx gen_tstsi ();

extern rtx gen_tsthi ();

extern rtx gen_tstqi ();

extern rtx gen_tstsf ();

extern rtx gen_tstdf ();

extern rtx gen_cmpsi ();

extern rtx gen_cmphi ();

extern rtx gen_cmpqi ();

extern rtx gen_cmpdf ();

extern rtx gen_cmpsf ();

extern rtx gen_movsi ();

extern rtx gen_movhi ();

extern rtx gen_movstricthi ();

extern rtx gen_movqi ();

extern rtx gen_movstrictqi ();

extern rtx gen_movsf ();

extern rtx gen_movdf ();

extern rtx gen_movdi ();

extern rtx gen_pushasi ();

extern rtx gen_truncsiqi2 ();

extern rtx gen_trunchiqi2 ();

extern rtx gen_truncsihi2 ();

extern rtx gen_zero_extendhisi2 ();

extern rtx gen_zero_extendqihi2 ();

extern rtx gen_zero_extendqisi2 ();

extern rtx gen_extendhisi2 ();

extern rtx gen_extendqihi2 ();

extern rtx gen_extendqisi2 ();

extern rtx gen_extendsfdf2 ();

extern rtx gen_truncdfsf2 ();

extern rtx gen_floatsisf2 ();

extern rtx gen_floatsidf2 ();

extern rtx gen_floathisf2 ();

extern rtx gen_floathidf2 ();

extern rtx gen_floatqisf2 ();

extern rtx gen_floatqidf2 ();

extern rtx gen_ftruncdf2 ();

extern rtx gen_ftruncsf2 ();

extern rtx gen_fixsfqi2 ();

extern rtx gen_fixsfhi2 ();

extern rtx gen_fixsfsi2 ();

extern rtx gen_fixdfqi2 ();

extern rtx gen_fixdfhi2 ();

extern rtx gen_fixdfsi2 ();

extern rtx gen_fix_truncsfsi2 ();

extern rtx gen_fix_truncdfsi2 ();

extern rtx gen_addsi3 ();

extern rtx gen_addhi3 ();

extern rtx gen_addqi3 ();

extern rtx gen_adddf3 ();

extern rtx gen_addsf3 ();

extern rtx gen_subsi3 ();

extern rtx gen_subhi3 ();

extern rtx gen_subqi3 ();

extern rtx gen_subdf3 ();

extern rtx gen_subsf3 ();

extern rtx gen_mulhi3 ();

extern rtx gen_mulhisi3 ();

extern rtx gen_mulsi3 ();

extern rtx gen_umulhi3 ();

extern rtx gen_umulhisi3 ();

extern rtx gen_umulsi3 ();

extern rtx gen_muldf3 ();

extern rtx gen_mulsf3 ();

extern rtx gen_divhi3 ();

extern rtx gen_divhisi3 ();

extern rtx gen_divsi3 ();

extern rtx gen_udivhi3 ();

extern rtx gen_udivhisi3 ();

extern rtx gen_udivsi3 ();

extern rtx gen_divdf3 ();

extern rtx gen_divsf3 ();

extern rtx gen_modhi3 ();

extern rtx gen_modhisi3 ();

extern rtx gen_umodhi3 ();

extern rtx gen_umodhisi3 ();

extern rtx gen_divmodsi4 ();

extern rtx gen_udivmodsi4 ();

extern rtx gen_andsi3 ();

extern rtx gen_andhi3 ();

extern rtx gen_andqi3 ();

extern rtx gen_iorsi3 ();

extern rtx gen_iorhi3 ();

extern rtx gen_iorqi3 ();

extern rtx gen_xorsi3 ();

extern rtx gen_xorhi3 ();

extern rtx gen_xorqi3 ();

extern rtx gen_negsi2 ();

extern rtx gen_neghi2 ();

extern rtx gen_negqi2 ();

extern rtx gen_negsf2 ();

extern rtx gen_negdf2 ();

extern rtx gen_abssf2 ();

extern rtx gen_absdf2 ();

extern rtx gen_one_cmplsi2 ();

extern rtx gen_one_cmplhi2 ();

extern rtx gen_one_cmplqi2 ();

extern rtx gen_ashlsi3 ();

extern rtx gen_ashlhi3 ();

extern rtx gen_ashlqi3 ();

extern rtx gen_ashrsi3 ();

extern rtx gen_ashrhi3 ();

extern rtx gen_ashrqi3 ();

extern rtx gen_lshlsi3 ();

extern rtx gen_lshlhi3 ();

extern rtx gen_lshlqi3 ();

extern rtx gen_lshrsi3 ();

extern rtx gen_lshrhi3 ();

extern rtx gen_lshrqi3 ();

extern rtx gen_rotlsi3 ();

extern rtx gen_rotlhi3 ();

extern rtx gen_rotlqi3 ();

extern rtx gen_rotrsi3 ();

extern rtx gen_rotrhi3 ();

extern rtx gen_rotrqi3 ();

extern rtx gen_extv ();

extern rtx gen_extzv ();

extern rtx gen_insv ();

extern rtx gen_seq ();

extern rtx gen_sne ();

extern rtx gen_sgt ();

extern rtx gen_sgtu ();

extern rtx gen_slt ();

extern rtx gen_sltu ();

extern rtx gen_sge ();

extern rtx gen_sgeu ();

extern rtx gen_sle ();

extern rtx gen_sleu ();

extern rtx gen_beq ();

extern rtx gen_bne ();

extern rtx gen_bgt ();

extern rtx gen_bgtu ();

extern rtx gen_blt ();

extern rtx gen_bltu ();

extern rtx gen_bge ();

extern rtx gen_bgeu ();

extern rtx gen_ble ();

extern rtx gen_bleu ();

extern rtx gen_casesi_1 ();

extern rtx gen_casesi_2 ();

extern rtx gen_casesi ();

extern rtx gen_jump ();

extern rtx gen_call ();

extern rtx gen_call_value ();

extern rtx gen_return ();

enum insn_code {
  CODE_FOR_tstsi = 2,
  CODE_FOR_tsthi = 3,
  CODE_FOR_tstqi = 4,
  CODE_FOR_tstsf = 5,
  CODE_FOR_tstdf = 8,
  CODE_FOR_cmpsi = 11,
  CODE_FOR_cmphi = 12,
  CODE_FOR_cmpqi = 13,
  CODE_FOR_cmpdf = 14,
  CODE_FOR_cmpsf = 17,
  CODE_FOR_movsi = 31,
  CODE_FOR_movhi = 32,
  CODE_FOR_movstricthi = 33,
  CODE_FOR_movqi = 34,
  CODE_FOR_movstrictqi = 35,
  CODE_FOR_movsf = 36,
  CODE_FOR_movdf = 37,
  CODE_FOR_movdi = 38,
  CODE_FOR_pushasi = 39,
  CODE_FOR_truncsiqi2 = 40,
  CODE_FOR_trunchiqi2 = 41,
  CODE_FOR_truncsihi2 = 42,
  CODE_FOR_zero_extendhisi2 = 43,
  CODE_FOR_zero_extendqihi2 = 44,
  CODE_FOR_zero_extendqisi2 = 45,
  CODE_FOR_extendhisi2 = 49,
  CODE_FOR_extendqihi2 = 50,
  CODE_FOR_extendqisi2 = 51,
  CODE_FOR_extendsfdf2 = 52,
  CODE_FOR_truncdfsf2 = 55,
  CODE_FOR_floatsisf2 = 58,
  CODE_FOR_floatsidf2 = 61,
  CODE_FOR_floathisf2 = 64,
  CODE_FOR_floathidf2 = 65,
  CODE_FOR_floatqisf2 = 66,
  CODE_FOR_floatqidf2 = 67,
  CODE_FOR_ftruncdf2 = 68,
  CODE_FOR_ftruncsf2 = 69,
  CODE_FOR_fixsfqi2 = 70,
  CODE_FOR_fixsfhi2 = 71,
  CODE_FOR_fixsfsi2 = 72,
  CODE_FOR_fixdfqi2 = 73,
  CODE_FOR_fixdfhi2 = 74,
  CODE_FOR_fixdfsi2 = 75,
  CODE_FOR_fix_truncsfsi2 = 76,
  CODE_FOR_fix_truncdfsi2 = 77,
  CODE_FOR_addsi3 = 78,
  CODE_FOR_addhi3 = 80,
  CODE_FOR_addqi3 = 82,
  CODE_FOR_adddf3 = 84,
  CODE_FOR_addsf3 = 87,
  CODE_FOR_subsi3 = 90,
  CODE_FOR_subhi3 = 92,
  CODE_FOR_subqi3 = 94,
  CODE_FOR_subdf3 = 96,
  CODE_FOR_subsf3 = 99,
  CODE_FOR_mulhi3 = 102,
  CODE_FOR_mulhisi3 = 103,
  CODE_FOR_mulsi3 = 104,
  CODE_FOR_umulhi3 = 105,
  CODE_FOR_umulhisi3 = 106,
  CODE_FOR_umulsi3 = 107,
  CODE_FOR_muldf3 = 108,
  CODE_FOR_mulsf3 = 111,
  CODE_FOR_divhi3 = 114,
  CODE_FOR_divhisi3 = 115,
  CODE_FOR_divsi3 = 116,
  CODE_FOR_udivhi3 = 117,
  CODE_FOR_udivhisi3 = 118,
  CODE_FOR_udivsi3 = 119,
  CODE_FOR_divdf3 = 120,
  CODE_FOR_divsf3 = 123,
  CODE_FOR_modhi3 = 126,
  CODE_FOR_modhisi3 = 127,
  CODE_FOR_umodhi3 = 128,
  CODE_FOR_umodhisi3 = 129,
  CODE_FOR_divmodsi4 = 130,
  CODE_FOR_udivmodsi4 = 131,
  CODE_FOR_andsi3 = 132,
  CODE_FOR_andhi3 = 133,
  CODE_FOR_andqi3 = 134,
  CODE_FOR_iorsi3 = 137,
  CODE_FOR_iorhi3 = 138,
  CODE_FOR_iorqi3 = 139,
  CODE_FOR_xorsi3 = 140,
  CODE_FOR_xorhi3 = 141,
  CODE_FOR_xorqi3 = 142,
  CODE_FOR_negsi2 = 143,
  CODE_FOR_neghi2 = 144,
  CODE_FOR_negqi2 = 145,
  CODE_FOR_negsf2 = 146,
  CODE_FOR_negdf2 = 149,
  CODE_FOR_abssf2 = 152,
  CODE_FOR_absdf2 = 155,
  CODE_FOR_one_cmplsi2 = 158,
  CODE_FOR_one_cmplhi2 = 159,
  CODE_FOR_one_cmplqi2 = 160,
  CODE_FOR_ashlsi3 = 167,
  CODE_FOR_ashlhi3 = 168,
  CODE_FOR_ashlqi3 = 169,
  CODE_FOR_ashrsi3 = 170,
  CODE_FOR_ashrhi3 = 171,
  CODE_FOR_ashrqi3 = 172,
  CODE_FOR_lshlsi3 = 173,
  CODE_FOR_lshlhi3 = 174,
  CODE_FOR_lshlqi3 = 175,
  CODE_FOR_lshrsi3 = 176,
  CODE_FOR_lshrhi3 = 177,
  CODE_FOR_lshrqi3 = 178,
  CODE_FOR_rotlsi3 = 179,
  CODE_FOR_rotlhi3 = 180,
  CODE_FOR_rotlqi3 = 181,
  CODE_FOR_rotrsi3 = 182,
  CODE_FOR_rotrhi3 = 183,
  CODE_FOR_rotrqi3 = 184,
  CODE_FOR_extv = 188,
  CODE_FOR_extzv = 189,
  CODE_FOR_insv = 193,
  CODE_FOR_seq = 205,
  CODE_FOR_sne = 206,
  CODE_FOR_sgt = 207,
  CODE_FOR_sgtu = 208,
  CODE_FOR_slt = 209,
  CODE_FOR_sltu = 210,
  CODE_FOR_sge = 211,
  CODE_FOR_sgeu = 212,
  CODE_FOR_sle = 213,
  CODE_FOR_sleu = 214,
  CODE_FOR_beq = 215,
  CODE_FOR_bne = 216,
  CODE_FOR_bgt = 217,
  CODE_FOR_bgtu = 218,
  CODE_FOR_blt = 219,
  CODE_FOR_bltu = 220,
  CODE_FOR_bge = 221,
  CODE_FOR_bgeu = 222,
  CODE_FOR_ble = 223,
  CODE_FOR_bleu = 224,
  CODE_FOR_casesi_1 = 235,
  CODE_FOR_casesi_2 = 236,
  CODE_FOR_casesi = 237,
  CODE_FOR_jump = 239,
  CODE_FOR_call = 243,
  CODE_FOR_call_value = 244,
  CODE_FOR_return = 245,
  CODE_FOR_nothing };

enum expand_modifier {EXPAND_NORMAL, EXPAND_SUM, EXPAND_CONST_ADDRESS};

extern int cse_not_expected;

extern rtx save_expr_regs;

struct args_size
{
  int constant;
  tree var;
};

enum direction {none, upward, downward};   

typedef struct optab
{
  enum rtx_code code;
  struct {
    enum insn_code insn_code;
    char *lib_call;
  } handlers [(int) MAX_MACHINE_MODE ];
} * optab;

extern rtx (*insn_gen_function[]) ();

extern optab add_optab;
extern optab sub_optab;
extern optab smul_optab;	 
extern optab umul_optab;	 
extern optab smul_widen_optab;	 

extern optab umul_widen_optab;
extern optab sdiv_optab;	 
extern optab sdivmod_optab;	 
extern optab udiv_optab;
extern optab udivmod_optab;
extern optab smod_optab;	 
extern optab umod_optab;
extern optab flodiv_optab;	 
extern optab ftrunc_optab;	 
extern optab and_optab;		 
extern optab andcb_optab;	 
extern optab ior_optab;		 
extern optab xor_optab;		 
extern optab ashl_optab;	 
extern optab ashr_optab;	 
extern optab lshl_optab;	 
extern optab lshr_optab;	 
extern optab rotl_optab;	 
extern optab rotr_optab;	 

extern optab mov_optab;		 
extern optab movstrict_optab;	 

extern optab cmp_optab;		 
extern optab tst_optab;		 

extern optab neg_optab;		 
extern optab abs_optab;		 
extern optab one_cmpl_optab;	 
extern optab ffs_optab;		 

enum optab_methods
{
  OPTAB_DIRECT,
  OPTAB_LIB,
  OPTAB_WIDEN,
  OPTAB_LIB_WIDEN,
};
typedef rtx (*rtxfun) ();

extern rtxfun bcc_gen_fctn[((int)LAST_AND_UNUSED_RTX_CODE) ];

extern rtxfun setcc_gen_fctn[((int)LAST_AND_UNUSED_RTX_CODE) ];

rtx expand_binop ();

rtx sign_expand_binop ();

rtx expand_unop ();

rtx negate_rtx ();

void init_fixtab ();
void init_floattab ();

void expand_fix ();

void expand_float ();

rtx gen_add2_insn ();
rtx gen_sub2_insn ();
rtx gen_move_insn ();

void emit_clr_insn ();

void emit_0_to_1_insn ();

void emit_cmp_insn ();

void convert_move ();

rtx convert_to_mode ();

void emit_library_call ();

rtx force_operand ();

rtx expr_size ();

rtx plus_constant ();

rtx lookup_static_chain ();

rtx eliminate_constant_term ();

rtx memory_address ();

rtx memory_address_noforce ();

rtx change_address ();

int rtx_equal_p ();

rtx stabilize ();

rtx copy_all_regs ();

rtx copy_to_reg ();

rtx copy_addr_to_reg ();

rtx copy_to_mode_reg ();

rtx copy_to_suggested_reg ();

rtx force_reg ();

rtx force_not_mem ();

void adjust_stack ();

void anti_adjust_stack ();

rtx function_value ();

rtx hard_function_value ();

rtx hard_libcall_value ();

void copy_function_value ();

rtx round_push ();

rtx store_bit_field ();
rtx extract_bit_field ();
rtx expand_shift ();
rtx expand_bit_and ();
rtx expand_mult ();
rtx expand_divmod ();
rtx get_structure_value_addr ();
rtx expand_stmt_expr ();

void jumpifnot ();
void jumpif ();
void do_jump ();

rtx assemble_static_space ();

extern int recog_memoized ();

extern void insn_extract ();

extern rtx recog_operand[];

extern rtx *recog_operand_loc[];

extern rtx *recog_dup_loc[];

extern char recog_dup_num[];

extern char *insn_template[];

extern char *(*insn_outfun[]) ();

extern int insn_n_operands[];

extern int insn_n_dups[];

extern int insn_n_alternatives[];

extern char *insn_operand_constraint[][5 ];

extern char insn_operand_address_p[][5 ];

extern enum machine_mode insn_operand_mode[][5 ];

extern char insn_operand_strict_low[][5 ];

extern int (*insn_operand_predicate[][5 ]) ();

optab add_optab;
optab sub_optab;
optab smul_optab;
optab umul_optab;
optab smul_widen_optab;
optab umul_widen_optab;
optab sdiv_optab;
optab sdivmod_optab;
optab udiv_optab;
optab udivmod_optab;
optab smod_optab;
optab umod_optab;
optab flodiv_optab;
optab ftrunc_optab;
optab and_optab;
optab andcb_optab;
optab ior_optab;
optab xor_optab;
optab ashl_optab;
optab lshr_optab;
optab lshl_optab;
optab ashr_optab;
optab rotl_optab;
optab rotr_optab;

optab mov_optab;
optab movstrict_optab;

optab neg_optab;
optab abs_optab;
optab one_cmpl_optab;
optab ffs_optab;

optab cmp_optab;
optab ucmp_optab;   
optab tst_optab;

rtxfun bcc_gen_fctn[((int)LAST_AND_UNUSED_RTX_CODE) ];

rtxfun setcc_gen_fctn[((int)LAST_AND_UNUSED_RTX_CODE) ];

rtx
expand_binop (mode, binoptab, op0, op1, target, unsignedp, methods)
     enum machine_mode mode;
     optab binoptab;
     rtx op0, op1;
     rtx target;
     int unsignedp;
     enum optab_methods methods;
{
  enum mode_class class;
  enum machine_mode wider_mode;
  register rtx temp;
  rtx last = get_last_insn ();

  class = 	(mode_class[(int)(mode)]) ;

  op0 = protect_from_queue (op0, 0);
  op1 = protect_from_queue (op1, 0);
  if (target)
    target = protect_from_queue (target, 1);

  if (flag_force_mem)
    {
      op0 = force_not_mem (op0);
      op1 = force_not_mem (op1);
    }

  if (binoptab == add_optab
      || binoptab == and_optab
      || binoptab == ior_optab
      || binoptab == xor_optab
      || binoptab == smul_optab
      || binoptab == umul_optab
      || binoptab == smul_widen_optab
      || binoptab == umul_widen_optab)
    {
      if (((target == 0 || 	((target)->code)  == REG)
	   ? ((	((op1)->code)  == REG
	       && 	((op0)->code)  != REG)
	      || target == op1)
	   : rtx_equal_p (op1, target))
	  ||
	  	((op0)->code)  == CONST_INT)
	{
	  temp = op1;
	  op1 = op0;
	  op0 = temp;
	}
    }

  if (binoptab->handlers[(int) mode].insn_code != CODE_FOR_nothing)
    {
      int icode = (int) binoptab->handlers[(int) mode].insn_code;
      enum machine_mode mode0 = insn_operand_mode[icode][1];
      enum machine_mode mode1 = insn_operand_mode[icode][2];
      rtx pat;
      rtx xop0 = op0, xop1 = op1;

      if (target)
	temp = target;
      else
	temp = gen_reg_rtx (mode);

      if (	((op0)->mode)  != VOIDmode
	  && 	((op0)->mode)  != mode0)
	xop0 = convert_to_mode (mode0, xop0, unsignedp);

      if (	((xop1)->mode)  != VOIDmode
	  && 	((xop1)->mode)  != mode1)
	xop1 = convert_to_mode (mode1, xop1, unsignedp);

      if (! (*insn_operand_predicate[icode][1]) (xop0, mode0))
	xop0 = force_reg (mode0, xop0);

      if (! (*insn_operand_predicate[icode][2]) (xop1, mode1))
	xop1 = force_reg (mode1, xop1);

      if (! (*insn_operand_predicate[icode][0]) (temp, mode))
	temp = gen_reg_rtx (mode);

      pat = (*insn_gen_function[(int) (icode)])  (temp, xop0, xop1);
      if (pat)
	{
	  emit_insn (pat);
	  return temp;
	}
      else
	delete_insns_since (last);
    }

  if (binoptab->handlers[(int) mode].lib_call
      && (methods == OPTAB_LIB || methods == OPTAB_LIB_WIDEN))
    {
      rtx insn_before, insn_first, insn_last;
      rtx funexp = gen_rtx (SYMBOL_REF, SImode ,
			    binoptab->handlers[(int) mode].lib_call);

      if (! flag_no_function_cse)
	funexp = copy_to_mode_reg (SImode , funexp);

      insn_before = get_last_insn ();
      if (insn_before == 0)
	abort ();

      emit_library_call (gen_rtx (SYMBOL_REF, SImode ,
				  binoptab->handlers[(int) mode].lib_call),
			 mode, 2, op0, mode, op1, mode);
      target = hard_libcall_value (mode);
      temp = copy_to_reg (target);

      insn_first = ((insn_before)->fld[2].rtx) ;
      insn_last = get_last_insn ();

      ((insn_last)->fld[6].rtx) 
	= gen_rtx (EXPR_LIST, REG_EQUAL,
		   gen_rtx (binoptab->code, mode, op0, op1),
		   gen_rtx (INSN_LIST, REG_RETVAL, insn_first,
			    ((insn_last)->fld[6].rtx) ));
      ((insn_first)->fld[6].rtx) 
	= gen_rtx (INSN_LIST, REG_LIBCALL, insn_last,
		   ((insn_first)->fld[6].rtx) );
      return temp;
    }

  delete_insns_since (last);

  if (! (methods == OPTAB_WIDEN || methods == OPTAB_LIB_WIDEN))
    return 0;			 

  methods = (methods == OPTAB_LIB_WIDEN ? OPTAB_LIB : OPTAB_DIRECT);

  if (class == MODE_INT || class == MODE_FLOAT)
    {
      for (wider_mode = (enum machine_mode) ((int)(mode) + 1) ;
	   ((int) wider_mode < (int) MAX_MACHINE_MODE
	    && 	(mode_class[(int)(wider_mode)])  == class);
	   wider_mode = (enum machine_mode) ((int)(wider_mode) + 1) )
	{
	  if ((binoptab->handlers[(int) wider_mode].insn_code
	       != CODE_FOR_nothing)
	      || (methods == OPTAB_LIB
		  && binoptab->handlers[(int) wider_mode].lib_call))
	    {
	      rtx xop0 = op0, xop1 = op1;

	      if (	((xop0)->mode)  != VOIDmode)
		{
		  temp = gen_reg_rtx (wider_mode);
		  convert_move (temp, xop0, unsignedp);
		  xop0 = temp;
		}
	      if (	((xop1)->mode)  != VOIDmode)
		{
		  temp = gen_reg_rtx (wider_mode);
		  convert_move (temp, xop1, unsignedp);
		  xop1 = temp;
		}

	      temp = expand_binop (wider_mode, binoptab, xop0, xop1, 0,
				   unsignedp, methods);
	      if (temp)
		{
		  if (class == MODE_FLOAT)
		    {
		      if (target == 0)
			target = gen_reg_rtx (mode);
		      convert_move (target, temp, 0);
		      return target;
		    }
		  else
		    return gen_lowpart (mode, temp);
		}
	      else
		delete_insns_since (last);
	    }
	}
    }

  return 0;
}

rtx
sign_expand_binop (mode, uoptab, soptab, op0, op1, target, unsignedp, methods)
    enum machine_mode mode;
    optab uoptab, soptab;
    rtx op0, op1, target;
    int unsignedp;
    enum optab_methods methods;
{
  register rtx temp;
  optab direct_optab = unsignedp ? uoptab : soptab;
  struct optab wide_soptab;

  temp = expand_binop (mode, direct_optab, op0, op1, target,
		       unsignedp, OPTAB_DIRECT);
  if (temp || methods == OPTAB_DIRECT)
    return temp;

  wide_soptab = *soptab;
  wide_soptab.handlers[(int) mode].insn_code = CODE_FOR_nothing;
  wide_soptab.handlers[(int) mode].lib_call = 0;

  temp = expand_binop (mode, &wide_soptab, op0, op1, target,
		       unsignedp, OPTAB_WIDEN);

  if (temp == 0 && unsignedp)
    temp = expand_binop (mode, uoptab, op0, op1, target,
			 unsignedp, OPTAB_WIDEN);
  if (temp || methods == OPTAB_WIDEN)
    return temp;

  temp = expand_binop (mode, direct_optab, op0, op1, target, unsignedp, OPTAB_LIB);
  if (temp || methods == OPTAB_LIB)
    return temp;

  temp = expand_binop (mode, &wide_soptab, op0, op1, target,
		       unsignedp, methods);
  if (temp != 0)
    return temp;
  if (unsignedp)
    return expand_binop (mode, uoptab, op0, op1, target,
			 unsignedp, methods);
  return 0;
}

int
expand_twoval_binop (binoptab, op0, op1, targ0, targ1, unsignedp)
     optab binoptab;
     rtx op0, op1;
     rtx targ0, targ1;
     int unsignedp;
{
  enum machine_mode mode = 	((targ0 ? targ0 : targ1)->mode) ;
  enum mode_class class;
  enum machine_mode wider_mode;

  class = 	(mode_class[(int)(mode)]) ;

  op0 = protect_from_queue (op0, 0);
  op1 = protect_from_queue (op1, 0);

  if (flag_force_mem)
    {
      op0 = force_not_mem (op0);
      op1 = force_not_mem (op1);
    }

  if (targ0)
    targ0 = protect_from_queue (targ0, 1);
  else
    targ0 = gen_reg_rtx (mode);
  if (targ1)
    targ1 = protect_from_queue (targ1, 1);
  else
    targ1 = gen_reg_rtx (mode);

  if (binoptab->handlers[(int) mode].insn_code != CODE_FOR_nothing)
    {
      emit_insn ((*insn_gen_function[(int) (binoptab->handlers[(int) mode].insn_code)]) 
		 (targ0, op0, op1, targ1));
      return 1;
    }

  if (class == MODE_INT || class == MODE_FLOAT)
    {
      for (wider_mode = (enum machine_mode) ((int)(mode) + 1) ;
	   ((int) wider_mode < (int) MAX_MACHINE_MODE
	    && 	(mode_class[(int)(wider_mode)])  == class);
	   wider_mode = (enum machine_mode) ((int)(wider_mode) + 1) )
	{
	  if (binoptab->handlers[(int) wider_mode].insn_code
	      != CODE_FOR_nothing)
	    {
	      expand_twoval_binop_convert (binoptab, wider_mode, op0, op1,
					   targ0, targ1, unsignedp);
	      return 1;
	    }
	}
    }
  return 0;
}

int
expand_twoval_binop_convert (binoptab, mode, op0, op1, targ0, targ1, unsignedp)
     register optab binoptab;
     register rtx op0, op1, targ0, targ1;
     int unsignedp;
{
  register rtx t0 = gen_reg_rtx (SImode);
  register rtx t1 = gen_reg_rtx (SImode);
  register rtx temp;

  temp = gen_reg_rtx (SImode);
  convert_move (temp, op0, unsignedp);
  op0 = temp;
  temp = gen_reg_rtx (SImode);
  convert_move (temp, op1, unsignedp);
  op1 = temp;

  expand_twoval_binop (binoptab, op0, op1, t0, t1, unsignedp);
  convert_move (targ0, t0, unsignedp);
  convert_move (targ1, t1, unsignedp);
  return 1;
}

rtx
expand_unop (mode, unoptab, op0, target, unsignedp)
     enum machine_mode mode;
     optab unoptab;
     rtx op0;
     rtx target;
     int unsignedp;
{
  enum mode_class class;
  enum machine_mode wider_mode;
  register rtx temp;

  class = 	(mode_class[(int)(mode)]) ;

  op0 = protect_from_queue (op0, 0);

  if (flag_force_mem)
    {
      op0 = force_not_mem (op0);
    }

  if (target)
    target = protect_from_queue (target, 1);

  if (unoptab->handlers[(int) mode].insn_code != CODE_FOR_nothing)
    {
      int icode = (int) unoptab->handlers[(int) mode].insn_code;
      enum machine_mode mode0 = insn_operand_mode[icode][1];

      if (target)
	temp = target;
      else
	temp = gen_reg_rtx (mode);

      if (	((op0)->mode)  != VOIDmode
	  && 	((op0)->mode)  != mode0)
	op0 = convert_to_mode (mode0, op0, unsignedp);

      if (! (*insn_operand_predicate[icode][1]) (op0, mode0))
	op0 = force_reg (mode0, op0);

      if (! (*insn_operand_predicate[icode][0]) (temp, mode))
	temp = gen_reg_rtx (mode);

      emit_insn ((*insn_gen_function[(int) (icode)])  (temp, op0));
      return temp;
    }
  else if (unoptab->handlers[(int) mode].lib_call)
    {
      rtx insn_before, insn_last;
      rtx funexp = gen_rtx (SYMBOL_REF, SImode ,
			    unoptab->handlers[(int) mode].lib_call);

      if (! flag_no_function_cse)
	funexp = copy_to_mode_reg (SImode , funexp);

      insn_before = get_last_insn ();

      emit_library_call (gen_rtx (SYMBOL_REF, SImode ,
				  unoptab->handlers[(int) mode].lib_call),
			 mode, 1, op0, mode);
      target = hard_libcall_value (mode);
      temp = copy_to_reg (target);
      insn_last = get_last_insn ();
      ((insn_last)->fld[6].rtx) 
	= gen_rtx (EXPR_LIST, REG_EQUAL,
		   gen_rtx (unoptab->code, mode, op0),
		   gen_rtx (INSN_LIST, REG_RETVAL,
			    ((insn_before)->fld[2].rtx) ,
			    ((insn_last)->fld[6].rtx) ));
      ((((insn_before)->fld[2].rtx) )->fld[6].rtx) 
	= gen_rtx (INSN_LIST, REG_LIBCALL, insn_last,
		   ((((insn_before)->fld[2].rtx) )->fld[6].rtx) );
      return temp;
    }

  if (class == MODE_INT || class == MODE_FLOAT)
    {
      for (wider_mode = (enum machine_mode) ((int)(mode) + 1) ;
	   ((int) wider_mode < (int) MAX_MACHINE_MODE
	    && 	(mode_class[(int)(wider_mode)])  == class);
	   wider_mode = (enum machine_mode) ((int)(wider_mode) + 1) )
	{
	  if ((unoptab->handlers[(int) wider_mode].insn_code
	       != CODE_FOR_nothing)
	      || unoptab->handlers[(int) wider_mode].lib_call)
	    {
	      if (	((op0)->mode)  != VOIDmode)
		{
		  temp = gen_reg_rtx (wider_mode);
		  convert_move (temp, op0, unsignedp);
		  op0 = temp;
		}
	      target = expand_unop (wider_mode, unoptab, op0, 0, unsignedp);
	      if (class == MODE_FLOAT)
		{
		  if (target == 0)
		    target = gen_reg_rtx (mode);
		  convert_move (target, temp, 0);
		  return target;
		}
	      else
		return gen_lowpart (mode, target);
	    }
	}
    }

  return 0;
}

void
emit_unop_insn (icode, target, op0, code)
     int icode;
     rtx target;
     rtx op0;
     enum rtx_code code;
{
  register rtx temp;
  enum machine_mode mode0 = insn_operand_mode[icode][1];
  rtx insn;
  rtx prev_insn = get_last_insn ();

  temp = target = protect_from_queue (target, 1);

  op0 = protect_from_queue (op0, 0);

  if (flag_force_mem)
    op0 = force_not_mem (op0);

  if (! (*insn_operand_predicate[icode][1]) (op0, mode0))
    op0 = force_reg (mode0, op0);

  if (! (*insn_operand_predicate[icode][0]) (temp, 	((temp)->mode) )
      || (flag_force_mem && 	((temp)->code)  == MEM))
    temp = gen_reg_rtx (	((temp)->mode) );

  insn = emit_insn ((*insn_gen_function[(int) (icode)])  (temp, op0));

  if (code != UNKNOWN && ((insn)->fld[1].rtx)  != prev_insn)
    ((insn)->fld[6].rtx) 
      = gen_rtx (EXPR_LIST, REG_EQUAL,
		 gen_rtx (code, 	((temp)->mode) , op0),
		 ((insn)->fld[6].rtx) );
  if (temp != target)
    emit_move_insn (target, temp);
}

void
emit_clr_insn (x)
     rtx x;
{
  emit_move_insn (x, const0_rtx);
}

void
emit_0_to_1_insn (x)
     rtx x;
{
  emit_move_insn (x, const1_rtx);
}

void
emit_cmp_insn (x, y, size, unsignedp, align)
     rtx x, y;
     rtx size;
     int unsignedp;
     int align;
{
  enum machine_mode mode = 	((x)->mode) ;
  enum mode_class class;
  enum machine_mode wider_mode;

  class = 	(mode_class[(int)(mode)]) ;

  if (mode == VOIDmode) mode = 	((y)->mode) ;

  if (mode != BLKmode && flag_force_mem)
    {
      x = force_not_mem (x);
      y = force_not_mem (y);
    }

  if (mode == BLKmode)
    {
      emit_queue ();
      x = protect_from_queue (x, 0);
      y = protect_from_queue (y, 0);

      if (size == 0)
	abort ();

	{

	  emit_library_call (gen_rtx (SYMBOL_REF, SImode , "bcmp"),
			     SImode, 3, x, SImode , y, SImode , size, SImode );

	  emit_cmp_insn (hard_libcall_value (SImode), const0_rtx, 0, 0, 0);
	}
      return;
    }

  if ((y == const0_rtx || y == fconst0_rtx || y == dconst0_rtx)
      && tst_optab->handlers[(int) mode].insn_code != CODE_FOR_nothing)
    {
      int icode = (int) tst_optab->handlers[(int) mode].insn_code;

      emit_queue ();
      x = protect_from_queue (x, 0);
      y = protect_from_queue (y, 0);

      if (! (*insn_operand_predicate[icode][0])
	  (x, insn_operand_mode[icode][0]))
	x = force_reg (insn_operand_mode[icode][0], x);

      emit_insn ((*insn_gen_function[(int) (icode)])  (x));
      return;
    }

  if (cmp_optab->handlers[(int) mode].insn_code != CODE_FOR_nothing)
    {
      int icode = (int) cmp_optab->handlers[(int) mode].insn_code;

      emit_queue ();
      x = protect_from_queue (x, 0);
      y = protect_from_queue (y, 0);

      if (! (*insn_operand_predicate[icode][0])
	  (x, insn_operand_mode[icode][0]))
	x = force_reg (insn_operand_mode[icode][0], x);

      if (! (*insn_operand_predicate[icode][1])
	  (y, insn_operand_mode[icode][1]))
	y = force_reg (insn_operand_mode[icode][1], y);

      emit_insn ((*insn_gen_function[(int) (icode)])  (x, y));
      return;
    }

  if (class == MODE_INT || class == MODE_FLOAT)
    {
      for (wider_mode = (enum machine_mode) ((int)(mode) + 1) ;
	   ((int) wider_mode < (int) MAX_MACHINE_MODE
	    && 	(mode_class[(int)(wider_mode)])  == class);
	   wider_mode = (enum machine_mode) ((int)(wider_mode) + 1) )
	{
	  if (cmp_optab->handlers[(int) wider_mode].insn_code
	      != CODE_FOR_nothing)
	    {
	      x = convert_to_mode (wider_mode, x, unsignedp);
	      y = convert_to_mode (wider_mode, y, unsignedp);
	      emit_cmp_insn (x, y, 0, unsignedp);
	      return;
	    }
	}
    }

  if (cmp_optab->handlers[(int) mode].lib_call)
    {
      char *string = cmp_optab->handlers[(int) mode].lib_call;

      if (unsignedp && ucmp_optab->handlers[(int) mode].lib_call)
	string = ucmp_optab->handlers[(int) mode].lib_call;

      emit_library_call (gen_rtx (SYMBOL_REF, SImode , string),
			 SImode, 2, x, mode, y, mode);

      if (	(mode_class[(int)(mode)])  == MODE_INT)
	emit_cmp_insn (hard_libcall_value (SImode), const1_rtx, 0, unsignedp, 0);
      else
	emit_cmp_insn (hard_libcall_value (SImode), const0_rtx, 0, 0, 0);
      return;
    }

  if (class == MODE_FLOAT)
    {
      for (wider_mode = (enum machine_mode) ((int)(mode) + 1) ;
	   ((int) wider_mode < (int) MAX_MACHINE_MODE
	    && 	(mode_class[(int)(wider_mode)])  == class);
	   wider_mode = (enum machine_mode) ((int)(wider_mode) + 1) )
	{
	  if ((cmp_optab->handlers[(int) wider_mode].insn_code
	       != CODE_FOR_nothing)
	      || (cmp_optab->handlers[(int) wider_mode].lib_call != 0))
	    {
	      x = convert_to_mode (wider_mode, x, unsignedp);
	      y = convert_to_mode (wider_mode, y, unsignedp);
	      emit_cmp_insn (x, y, 0, unsignedp);
	    }
	}
      return;
    }

  abort ();
}

rtx
gen_add2_insn (x, y)
     rtx x, y;
{
  return ((*insn_gen_function[(int) (add_optab->handlers[(int) 	((x)->mode) ].insn_code)]) 
	  (x, x, y));
}

int
have_add2_insn (mode)
     enum machine_mode mode;
{
  return add_optab->handlers[(int) mode].insn_code != CODE_FOR_nothing;
}

rtx
gen_sub2_insn (x, y)
     rtx x, y;
{
  return ((*insn_gen_function[(int) (sub_optab->handlers[(int) 	((x)->mode) ].insn_code)]) 
	  (x, x, y));
}

int
have_sub2_insn (mode)
     enum machine_mode mode;
{
  return add_optab->handlers[(int) mode].insn_code != CODE_FOR_nothing;
}

rtx
gen_move_insn (x, y)
     rtx x, y;
{
  register enum machine_mode mode = 	((x)->mode) ;
  if (mode == VOIDmode)
    mode = 	((y)->mode) ;
  return ((*insn_gen_function[(int) (mov_optab->handlers[(int) mode].insn_code)])  (x, y));
}

static enum insn_code fixtab[2][2][2];
static enum insn_code fixtrunctab[2][2][2];
static enum insn_code floattab[2][2];

static enum insn_code
can_fix_p (fixmode, fltmode, unsignedp, truncp_ptr)
     enum machine_mode fltmode, fixmode;
     int unsignedp;
     int *truncp_ptr;
{
  *truncp_ptr = 0;
  if (fixtrunctab[fltmode != SFmode][fixmode == DImode][unsignedp]
      != CODE_FOR_nothing)
    return fixtrunctab[fltmode != SFmode][fixmode == DImode][unsignedp];
  if (ftrunc_optab->handlers[(int) fltmode].insn_code != CODE_FOR_nothing)
    {
      *truncp_ptr = 1;
      return fixtab[fltmode != SFmode][fixmode == DImode][unsignedp];
    }
  return CODE_FOR_nothing;
}

static enum insn_code
can_float_p (fltmode, fixmode)
     enum machine_mode fixmode, fltmode;
{
  return floattab[fltmode != SFmode][fixmode == DImode];
}

void
init_fixtab ()
{
  enum insn_code *p;
  for (p = fixtab[0][0];
       p < fixtab[0][0] + sizeof fixtab / sizeof (fixtab[0][0][0]); 
       p++)
    *p = CODE_FOR_nothing;
  for (p = fixtrunctab[0][0];
       p < fixtrunctab[0][0] + sizeof fixtrunctab / sizeof (fixtrunctab[0][0][0]); 
       p++)
    *p = CODE_FOR_nothing;

  if (((target_flags & 2) ) )
    fixtab[0][0][0] = CODE_FOR_fixsfsi2;

  if (((target_flags & 2) ) )
    fixtab[1][0][0] = CODE_FOR_fixdfsi2;

  if (((target_flags & 0100) ) )
    fixtrunctab[0][0][0] = CODE_FOR_fix_truncsfsi2;

  if (((target_flags & 0100) ) )
    fixtrunctab[1][0][0] = CODE_FOR_fix_truncdfsi2;

}

void
init_floattab ()
{
  enum insn_code *p;
  for (p = floattab[0];
       p < floattab[0] + sizeof floattab / sizeof (floattab[0][0]); 
       p++)
    *p = CODE_FOR_nothing;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    floattab[0][0] = CODE_FOR_floatsisf2;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    floattab[1][0] = CODE_FOR_floatsidf2;

}

void
expand_float (real_to, from, unsignedp)
     rtx real_to, from;
     int unsignedp;
{
  enum insn_code icode;
  register rtx to;

  to = real_to = protect_from_queue (real_to, 1);
  from = protect_from_queue (from, 0);

  if (flag_force_mem)
    {
      from = force_not_mem (from);
    }

  if (unsignedp && 	((to)->code)  != REG)
    to = gen_reg_rtx (	((to)->mode) );

  if ((icode = can_float_p (	((to)->mode) , 	((from)->mode) ))
      != CODE_FOR_nothing)
    {
      emit_unop_insn (icode, to, from, FLOAT);
    }
  else if (	((to)->mode)  == SFmode
	   && ((icode = can_float_p (	((from)->mode) , DFmode))
	       != CODE_FOR_nothing))
    {
      to = gen_reg_rtx (DFmode);
      emit_unop_insn (icode, to, from, FLOAT);
    }

  else if (	((from)->mode)  != DImode
	   && (can_float_p (	((to)->mode) , DImode) != CODE_FOR_nothing
	       || can_float_p (DFmode, DImode) != CODE_FOR_nothing))
    {
      register rtx tem = gen_reg_rtx (DImode);
      convert_move (tem, from, unsignedp);
      from = tem;

      unsignedp = 0;

      if ((icode = can_float_p (	((to)->mode) , 	((from)->mode) ))
	  != CODE_FOR_nothing)
	{
	  emit_unop_insn (icode, to, from, FLOAT);
	}
      else if ((icode = can_float_p (DFmode, DImode))
	        != CODE_FOR_nothing)
	{
	  to = gen_reg_rtx (DFmode);
	  emit_unop_insn (icode, to, from, FLOAT);
	}
    }

  else
    {
      if (	(mode_size[(int)(	((from)->mode) )])  < 	(mode_size[(int)(SImode)]) )
	{
	  from = convert_to_mode (SImode, from, unsignedp);
	  unsignedp = 0;
	}
      emit_library_call (gen_rtx (SYMBOL_REF, SImode ,
				  (	((from)->mode)  == SImode ? "__floatsidf"
				   : "__floatdidf")),
			 DFmode, 1, from, 	((from)->mode) );
      to = copy_to_reg (hard_libcall_value (DFmode));
    }

  if (unsignedp)
    {
      rtx label = gen_label_rtx ();
      rtx temp;
      double  offset;

      do_pending_stack_adjust ();
      emit_cmp_insn (to, 	((to)->mode)  == DFmode ? dconst0_rtx : fconst0_rtx,
		     0, 0, 0);
      emit_jump_insn (gen_bge (label));
      offset = ldexp (1.0,   (8  * mode_size[(int)(	((from)->mode) )]) ) ;
      temp = expand_binop (	((to)->mode) , add_optab, to,
			   immed_real_const_1 (offset, 	((to)->mode) ),
			   to, 0, OPTAB_LIB_WIDEN);
      if (temp != to)
	emit_move_insn (to, temp);
      do_pending_stack_adjust ();
      emit_label (label);
    }

  if (to != real_to)
    {
      if (	((real_to)->mode)  == 	((to)->mode) )
	emit_move_insn (real_to, to);
      else
	convert_move (real_to, to, 0);
    }
}

static rtx
ftruncify (x)
     rtx x;
{
  rtx temp = gen_reg_rtx (	((x)->mode) );
  return expand_unop (	((x)->mode) , ftrunc_optab, x, temp, 0);
}

void
expand_fix (to, from, unsignedp)
     register rtx to, from;
     int unsignedp;
{
  enum insn_code icode;
  register rtx target;
  int must_trunc = 0;

  while (1)
    {
      icode = can_fix_p (	((to)->mode) , 	((from)->mode) , unsignedp, &must_trunc);
      if (icode != CODE_FOR_nothing)
	{
	  if (must_trunc)
	    from = ftruncify (from);

	  emit_unop_insn (icode, to, from, FIX);
	  return;
	}

      icode = can_fix_p (DImode, 	((from)->mode) , unsignedp, &must_trunc);

      if (	((to)->mode)  != DImode && icode != CODE_FOR_nothing)
	{
	  register rtx temp = gen_reg_rtx (DImode);

	  if (must_trunc)
	    from = ftruncify (from);
	  emit_unop_insn (icode, temp, from, FIX);
	  convert_move (to, temp, unsignedp);
	  return;
	}

      if (	((from)->mode)  == DFmode)
	break;

      from = convert_to_mode (DFmode, from, 0);
    }

  to = protect_from_queue (to, 1);
  from = protect_from_queue (from, 0);

  if (flag_force_mem)
    from = force_not_mem (from);

  if (	((to)->mode)  != DImode)
    {
      emit_library_call (gen_rtx (SYMBOL_REF, SImode ,
				  unsignedp ? "__fixunsdfsi"
				  : "__fixdfsi"),
			 SImode, 1, from, DFmode);
      target = hard_libcall_value (SImode);
    }
  else
    {
      emit_library_call (gen_rtx (SYMBOL_REF, SImode ,
				  unsignedp ? "__fixunsdfdi"
				  : "__fixdfdi"),
			 DImode, 1, from, DFmode);
      target = hard_libcall_value (DImode);
    }

  if (	((to)->mode)  == 	((target)->mode) )
    emit_move_insn (to, target);
  else
    convert_move (to, target, 0);
}
static optab
init_optab (code)
     enum rtx_code code;
{
  int i;
  optab op = (optab) malloc (sizeof (struct optab));
  op->code = code;
  for (i = 0; i < (int) MAX_MACHINE_MODE ; i++)
    {
      op->handlers[i].insn_code = CODE_FOR_nothing;
      op->handlers[i].lib_call = 0;
    }
  return op;
}

void
init_optabs ()
{
  init_fixtab ();
  init_floattab ();
  init_comparisons ();

  add_optab = init_optab (PLUS);
  sub_optab = init_optab (MINUS);
  smul_optab = init_optab (MULT);
  umul_optab = init_optab (UMULT);
  smul_widen_optab = init_optab (MULT);
  umul_widen_optab = init_optab (UMULT);
  sdiv_optab = init_optab (DIV);
  sdivmod_optab = init_optab (UNKNOWN);
  udiv_optab = init_optab (UDIV);
  udivmod_optab = init_optab (UNKNOWN);
  smod_optab = init_optab (MOD);
  umod_optab = init_optab (UMOD);
  flodiv_optab = init_optab (DIV);
  ftrunc_optab = init_optab (UNKNOWN);
  and_optab = init_optab (AND);
  andcb_optab = init_optab (UNKNOWN);
  ior_optab = init_optab (IOR);
  xor_optab = init_optab (XOR);
  ashl_optab = init_optab (ASHIFT);
  ashr_optab = init_optab (ASHIFTRT);
  lshl_optab = init_optab (LSHIFT);
  lshr_optab = init_optab (LSHIFTRT);
  rotl_optab = init_optab (ROTATE);
  rotr_optab = init_optab (ROTATERT);
  mov_optab = init_optab (UNKNOWN);
  movstrict_optab = init_optab (UNKNOWN);
  cmp_optab = init_optab (UNKNOWN);
  ucmp_optab = init_optab (UNKNOWN);
  tst_optab = init_optab (UNKNOWN);
  neg_optab = init_optab (NEG);
  abs_optab = init_optab (ABS);
  one_cmpl_optab = init_optab (NOT);
  ffs_optab = init_optab (FFS);

  if ((1) )
    add_optab->handlers[(int) QImode].insn_code = CODE_FOR_addqi3;

  if ((1) )
    add_optab->handlers[(int) HImode].insn_code = CODE_FOR_addhi3;

  if ((1) )
    add_optab->handlers[(int) SImode].insn_code = CODE_FOR_addsi3;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    add_optab->handlers[(int) SFmode].insn_code = CODE_FOR_addsf3;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    add_optab->handlers[(int) DFmode].insn_code = CODE_FOR_adddf3;

  add_optab->handlers[(int) DImode].lib_call = "__adddi3";
  add_optab->handlers[(int) SFmode].lib_call = "__addsf3";
  add_optab->handlers[(int) DFmode].lib_call = "__adddf3";

  if ((1) )
    sub_optab->handlers[(int) QImode].insn_code = CODE_FOR_subqi3;

  if ((1) )
    sub_optab->handlers[(int) HImode].insn_code = CODE_FOR_subhi3;

  if ((1) )
    sub_optab->handlers[(int) SImode].insn_code = CODE_FOR_subsi3;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    sub_optab->handlers[(int) SFmode].insn_code = CODE_FOR_subsf3;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    sub_optab->handlers[(int) DFmode].insn_code = CODE_FOR_subdf3;

  sub_optab->handlers[(int) DImode].lib_call = "__subdi3";
  sub_optab->handlers[(int) SFmode].lib_call = "__subsf3";
  sub_optab->handlers[(int) DFmode].lib_call = "__subdf3";

  if ((1) )
    smul_optab->handlers[(int) HImode].insn_code = CODE_FOR_mulhi3;

  if (((target_flags & 1) ) )
    smul_optab->handlers[(int) SImode].insn_code = CODE_FOR_mulsi3;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    smul_optab->handlers[(int) SFmode].insn_code = CODE_FOR_mulsf3;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    smul_optab->handlers[(int) DFmode].insn_code = CODE_FOR_muldf3;

  smul_optab->handlers[(int) SImode].lib_call = "__mulsi3";

  smul_optab->handlers[(int) DImode].lib_call = "__muldi3";
  smul_optab->handlers[(int) SFmode].lib_call = "__mulsf3";
  smul_optab->handlers[(int) DFmode].lib_call = "__muldf3";

  if ((1) )
    smul_widen_optab->handlers[(int) SImode].insn_code = CODE_FOR_mulhisi3;

  if ((1) )
    umul_optab->handlers[(int) HImode].insn_code = CODE_FOR_umulhi3;

  if (((target_flags & 1) ) )
    umul_optab->handlers[(int) SImode].insn_code = CODE_FOR_umulsi3;

  umul_optab->handlers[(int) SImode].lib_call = "__umulsi3";

  umul_optab->handlers[(int) DImode].lib_call = "__umuldi3";
  umul_optab->handlers[(int) SFmode].lib_call = "__umulsf3";
  umul_optab->handlers[(int) DFmode].lib_call = "__umuldf3";

  if ((1) )
    umul_widen_optab->handlers[(int) SImode].insn_code = CODE_FOR_umulhisi3;

  if ((1) )
    sdiv_optab->handlers[(int) HImode].insn_code = CODE_FOR_divhi3;

  if (((target_flags & 1) ) )
    sdiv_optab->handlers[(int) SImode].insn_code = CODE_FOR_divsi3;

  sdiv_optab->handlers[(int) SImode].lib_call = "__divsi3";

  sdiv_optab->handlers[(int) DImode].lib_call = "__divdi3";

  if ((1) )
    udiv_optab->handlers[(int) HImode].insn_code = CODE_FOR_udivhi3;

  if (((target_flags & 1) ) )
    udiv_optab->handlers[(int) SImode].insn_code = CODE_FOR_udivsi3;

  udiv_optab->handlers[(int) SImode].lib_call = "__udivsi3";

  udiv_optab->handlers[(int) DImode].lib_call = "__udivdi3";

  if (((target_flags & 1) ) )
    sdivmod_optab->handlers[(int) SImode].insn_code = CODE_FOR_divmodsi4;

  if (((target_flags & 1) ) )
    udivmod_optab->handlers[(int) SImode].insn_code = CODE_FOR_udivmodsi4;

  if ((1) )
    smod_optab->handlers[(int) HImode].insn_code = CODE_FOR_modhi3;

  smod_optab->handlers[(int) SImode].lib_call = "__modsi3";

  smod_optab->handlers[(int) DImode].lib_call = "__moddi3";

  if ((1) )
    umod_optab->handlers[(int) HImode].insn_code = CODE_FOR_umodhi3;

  umod_optab->handlers[(int) SImode].lib_call = "__umodsi3";

  umod_optab->handlers[(int) DImode].lib_call = "__umoddi3";

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    flodiv_optab->handlers[(int) SFmode].insn_code = CODE_FOR_divsf3;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    flodiv_optab->handlers[(int) DFmode].insn_code = CODE_FOR_divdf3;

  flodiv_optab->handlers[(int) SFmode].lib_call = "__divsf3";
  flodiv_optab->handlers[(int) DFmode].lib_call = "__divdf3";

  if (((target_flags & 2) ) )
    ftrunc_optab->handlers[(int) SFmode].insn_code = CODE_FOR_ftruncsf2;

  if (((target_flags & 2) ) )
    ftrunc_optab->handlers[(int) DFmode].insn_code = CODE_FOR_ftruncdf2;

  if ((1) )
    and_optab->handlers[(int) QImode].insn_code = CODE_FOR_andqi3;

  if ((1) )
    and_optab->handlers[(int) HImode].insn_code = CODE_FOR_andhi3;

  if ((1) )
    and_optab->handlers[(int) SImode].insn_code = CODE_FOR_andsi3;

  and_optab->handlers[(int) DImode].lib_call = "__anddi3";

  andcb_optab->handlers[(int) DImode].lib_call = "__andcbdi3";

  if ((1) )
    ior_optab->handlers[(int) QImode].insn_code = CODE_FOR_iorqi3;

  if ((1) )
    ior_optab->handlers[(int) HImode].insn_code = CODE_FOR_iorhi3;

  if ((1) )
    ior_optab->handlers[(int) SImode].insn_code = CODE_FOR_iorsi3;

  ior_optab->handlers[(int) DImode].lib_call = "__iordi3";

  if ((1) )
    xor_optab->handlers[(int) QImode].insn_code = CODE_FOR_xorqi3;

  if ((1) )
    xor_optab->handlers[(int) HImode].insn_code = CODE_FOR_xorhi3;

  if ((1) )
    xor_optab->handlers[(int) SImode].insn_code = CODE_FOR_xorsi3;

  xor_optab->handlers[(int) DImode].lib_call = "__xordi3";

  if ((1) )
    ashl_optab->handlers[(int) QImode].insn_code = CODE_FOR_ashlqi3;

  if ((1) )
    ashl_optab->handlers[(int) HImode].insn_code = CODE_FOR_ashlhi3;

  if ((1) )
    ashl_optab->handlers[(int) SImode].insn_code = CODE_FOR_ashlsi3;

  ashl_optab->handlers[(int) SImode].lib_call = "__ashlsi3";
  ashl_optab->handlers[(int) DImode].lib_call = "__ashldi3";

  if ((1) )
    ashr_optab->handlers[(int) QImode].insn_code = CODE_FOR_ashrqi3;

  if ((1) )
    ashr_optab->handlers[(int) HImode].insn_code = CODE_FOR_ashrhi3;

  if ((1) )
    ashr_optab->handlers[(int) SImode].insn_code = CODE_FOR_ashrsi3;

  ashr_optab->handlers[(int) SImode].lib_call = "__ashrsi3";
  ashr_optab->handlers[(int) DImode].lib_call = "__ashrdi3";

  if ((1) )
    lshl_optab->handlers[(int) QImode].insn_code = CODE_FOR_lshlqi3;

  if ((1) )
    lshl_optab->handlers[(int) HImode].insn_code = CODE_FOR_lshlhi3;

  if ((1) )
    lshl_optab->handlers[(int) SImode].insn_code = CODE_FOR_lshlsi3;

  lshl_optab->handlers[(int) SImode].lib_call = "__lshlsi3";
  lshl_optab->handlers[(int) DImode].lib_call = "__lshldi3";

  if ((1) )
    lshr_optab->handlers[(int) QImode].insn_code = CODE_FOR_lshrqi3;

  if ((1) )
    lshr_optab->handlers[(int) HImode].insn_code = CODE_FOR_lshrhi3;

  if ((1) )
    lshr_optab->handlers[(int) SImode].insn_code = CODE_FOR_lshrsi3;

  lshr_optab->handlers[(int) SImode].lib_call = "__lshrsi3";
  lshr_optab->handlers[(int) DImode].lib_call = "__lshrdi3";

  if ((1) )
    rotl_optab->handlers[(int) QImode].insn_code = CODE_FOR_rotlqi3;

  if ((1) )
    rotl_optab->handlers[(int) HImode].insn_code = CODE_FOR_rotlhi3;

  if ((1) )
    rotl_optab->handlers[(int) SImode].insn_code = CODE_FOR_rotlsi3;

  rotl_optab->handlers[(int) SImode].lib_call = "__rotlsi3";
  rotl_optab->handlers[(int) DImode].lib_call = "__rotldi3";

  if ((1) )
    rotr_optab->handlers[(int) QImode].insn_code = CODE_FOR_rotrqi3;

  if ((1) )
    rotr_optab->handlers[(int) HImode].insn_code = CODE_FOR_rotrhi3;

  if ((1) )
    rotr_optab->handlers[(int) SImode].insn_code = CODE_FOR_rotrsi3;

  rotr_optab->handlers[(int) SImode].lib_call = "__rotrsi3";
  rotr_optab->handlers[(int) DImode].lib_call = "__rotrdi3";

  if ((1) )
    neg_optab->handlers[(int) QImode].insn_code = CODE_FOR_negqi2;

  if ((1) )
    neg_optab->handlers[(int) HImode].insn_code = CODE_FOR_neghi2;

  if ((1) )
    neg_optab->handlers[(int) SImode].insn_code = CODE_FOR_negsi2;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    neg_optab->handlers[(int) SFmode].insn_code = CODE_FOR_negsf2;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    neg_optab->handlers[(int) DFmode].insn_code = CODE_FOR_negdf2;

  neg_optab->handlers[(int) SImode].lib_call = "__negsi2"; 
  neg_optab->handlers[(int) DImode].lib_call = "__negdi2";
  neg_optab->handlers[(int) SFmode].lib_call = "__negsf2";
  neg_optab->handlers[(int) DFmode].lib_call = "__negdf2";

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    abs_optab->handlers[(int) SFmode].insn_code = CODE_FOR_abssf2;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    abs_optab->handlers[(int) DFmode].insn_code = CODE_FOR_absdf2;

  if ((1) )
    one_cmpl_optab->handlers[(int) QImode].insn_code = CODE_FOR_one_cmplqi2;

  if ((1) )
    one_cmpl_optab->handlers[(int) HImode].insn_code = CODE_FOR_one_cmplhi2;

  if ((1) )
    one_cmpl_optab->handlers[(int) SImode].insn_code = CODE_FOR_one_cmplsi2;

  one_cmpl_optab->handlers[(int) SImode].lib_call = "__one_cmplsi2"; 
  one_cmpl_optab->handlers[(int) DImode].lib_call = "__one_cmpldi2";

  ffs_optab->handlers[(int) SImode].lib_call = "ffs"; 

  if ((1) )
    mov_optab->handlers[(int) QImode].insn_code = CODE_FOR_movqi;

  if ((1) )
    mov_optab->handlers[(int) HImode].insn_code = CODE_FOR_movhi;

  if ((1) )
    mov_optab->handlers[(int) SImode].insn_code = CODE_FOR_movsi;

  if ((1) )
    mov_optab->handlers[(int) DImode].insn_code = CODE_FOR_movdi;

  if ((1) )
    mov_optab->handlers[(int) SFmode].insn_code = CODE_FOR_movsf;

  if ((1) )
    mov_optab->handlers[(int) DFmode].insn_code = CODE_FOR_movdf;

  if ((1) )
    movstrict_optab->handlers[(int) QImode].insn_code = CODE_FOR_movstrictqi;

  if ((1) )
    movstrict_optab->handlers[(int) HImode].insn_code = CODE_FOR_movstricthi;

  if ((1) )
    cmp_optab->handlers[(int) QImode].insn_code = CODE_FOR_cmpqi;

  if ((1) )
    cmp_optab->handlers[(int) HImode].insn_code = CODE_FOR_cmphi;

  if ((1) )
    cmp_optab->handlers[(int) SImode].insn_code = CODE_FOR_cmpsi;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    cmp_optab->handlers[(int) SFmode].insn_code = CODE_FOR_cmpsf;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    cmp_optab->handlers[(int) DFmode].insn_code = CODE_FOR_cmpdf;

  if ((1) )
    tst_optab->handlers[(int) QImode].insn_code = CODE_FOR_tstqi;

  if ((1) )
    tst_optab->handlers[(int) HImode].insn_code = CODE_FOR_tsthi;

  if ((1) )
    tst_optab->handlers[(int) SImode].insn_code = CODE_FOR_tstsi;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    tst_optab->handlers[(int) SFmode].insn_code = CODE_FOR_tstsf;

  if (((target_flags & 2)  || (target_flags & 0100) ) )
    tst_optab->handlers[(int) DFmode].insn_code = CODE_FOR_tstdf;

  cmp_optab->handlers[(int) DImode].lib_call = "__cmpdi2";
  ucmp_optab->handlers[(int) DImode].lib_call = "__ucmpdi2";
  cmp_optab->handlers[(int) SFmode].lib_call = "__cmpsf2";
  cmp_optab->handlers[(int) DFmode].lib_call = "__cmpdf2";

  if ((1) )
    bcc_gen_fctn[(int) EQ] = gen_beq;

  if ((1) )
    bcc_gen_fctn[(int) NE] = gen_bne;

  if ((1) )
    bcc_gen_fctn[(int) GT] = gen_bgt;

  if ((1) )
    bcc_gen_fctn[(int) GE] = gen_bge;

  if ((1) )
    bcc_gen_fctn[(int) GTU] = gen_bgtu;

  if ((1) )
    bcc_gen_fctn[(int) GEU] = gen_bgeu;

  if ((1) )
    bcc_gen_fctn[(int) LT] = gen_blt;

  if ((1) )
    bcc_gen_fctn[(int) LE] = gen_ble;

  if ((1) )
    bcc_gen_fctn[(int) LTU] = gen_bltu;

  if ((1) )
    bcc_gen_fctn[(int) LEU] = gen_bleu;

  if ((1) )
    setcc_gen_fctn[(int) EQ] = gen_seq;

  if ((1) )
    setcc_gen_fctn[(int) NE] = gen_sne;

  if ((1) )
    setcc_gen_fctn[(int) GT] = gen_sgt;

  if ((1) )
    setcc_gen_fctn[(int) GE] = gen_sge;

  if ((1) )
    setcc_gen_fctn[(int) GTU] = gen_sgtu;

  if ((1) )
    setcc_gen_fctn[(int) GEU] = gen_sgeu;

  if ((1) )
    setcc_gen_fctn[(int) LT] = gen_slt;

  if ((1) )
    setcc_gen_fctn[(int) LE] = gen_sle;

  if ((1) )
    setcc_gen_fctn[(int) LTU] = gen_sltu;

  if ((1) )
    setcc_gen_fctn[(int) LEU] = gen_sleu;

}

