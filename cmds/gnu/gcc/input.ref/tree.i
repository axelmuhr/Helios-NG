
extern int target_flags;

enum reg_class { NO_REGS, LO_FPA_REGS, FPA_REGS, FP_REGS,
  FP_OR_FPA_REGS, DATA_REGS, DATA_OR_FPA_REGS, DATA_OR_FP_REGS,
  DATA_OR_FP_OR_FPA_REGS, ADDR_REGS, GENERAL_REGS,
  GENERAL_OR_FPA_REGS, GENERAL_OR_FP_REGS, ALL_REGS,
  LIM_REG_CLASSES };

extern enum reg_class regno_reg_class[];

extern	struct	_iobuf {
	int	_cnt;
	unsigned char *_ptr;
	unsigned char *_base;
	int	_bufsiz;
	short	_flag;
	char	_file;		 
} _iob[];

extern struct _iobuf 	*fopen();
extern struct _iobuf 	*fdopen();
extern struct _iobuf 	*freopen();
extern struct _iobuf 	*popen();
extern struct _iobuf 	*tmpfile();
extern long	ftell();
extern char	*fgets();
extern char	*gets();

extern char	*ctermid();
extern char	*cuserid();
extern char	*tempnam();
extern char	*tmpnam();

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

};

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

struct _obstack_chunk		 
{
  char  *limit;			 
  struct _obstack_chunk *prev;	 
  char	contents[4];		 
};

struct obstack		 
{
  long	chunk_size;		 
  struct _obstack_chunk* chunk;	 
  char	*object_base;		 
  char	*next_free;		 
  char	*chunk_limit;		 
  int	temp;			 
  int   alignment_mask;		 
  struct _obstack_chunk *(*chunkfun) ();  
  void (*freefun) ();		 
};

void obstack_init (struct obstack *obstack);

void * obstack_alloc (struct obstack *obstack, int size);

void * obstack_copy (struct obstack *obstack, void *address, int size);
void * obstack_copy0 (struct obstack *obstack, void *address, int size);

void obstack_free (struct obstack *obstack, void *block);

void obstack_blank (struct obstack *obstack, int size);

void obstack_grow (struct obstack *obstack, void *data, int size);
void obstack_grow0 (struct obstack *obstack, void *data, int size);

void obstack_1grow (struct obstack *obstack, int data_char);

void * obstack_finish (struct obstack *obstack);

int obstack_object_size (struct obstack *obstack);

int obstack_room (struct obstack *obstack);
void obstack_1grow_fast (struct obstack *obstack, int data_char);
void obstack_blank_fast (struct obstack *obstack, int size);

void * obstack_base (struct obstack *obstack);
void * obstack_next_free (struct obstack *obstack);
int obstack_alignment_mask (struct obstack *obstack);
int obstack_chunk_size (struct obstack *obstack);

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

extern int xmalloc ();
extern void free ();

struct obstack permanent_obstack;

struct obstack maybepermanent_obstack;

struct obstack temporary_obstack;

struct obstack momentary_obstack;

struct obstack *saveable_obstack;

struct obstack *rtl_obstack;

struct obstack *current_obstack;

struct obstack *expression_obstack;

char *maybepermanent_firstobj;
char *temporary_firstobj;
char *momentary_firstobj;

int all_types_permanent;

struct momentary_level
{
  struct momentary_level *prev;
  char *base;
  struct obstack *obstack;
};

struct momentary_level *momentary_stack;

char *tree_code_type[] = {

 "x", 

 "x", 

 "x", 

 "x", 

 "t", 	 

 "t", 

 "t", 

 "t", 

 "t", 

 "t", 

 "t", 

 "t", 

 "t", 

 "t", 

 "t", 

 "t", 

 "t", 

 "t", 

 "t", 

 "t", 

 "t", 	 

 "t", 

 "t", 

 "s", 

 "s", 

 "s", 

 "s", 

 "s", 

 "s", 

 "s", 

 "s", 

 "s", 

 "s", 

 "s", 

 "s", 

 "c", 

 "c", 

 "c", 

 "c", 

 "d", 
 "d", 
 "d", 
 "d", 
 "d", 
 "d", 
 "d", 
 "d", 
 "d", 

 "r", 

 "r", 

 "r", 

 "r", 

 "r", 

 "e", 

 "e", 

 "e", 

 "e", 

 "e", 

 "e", 

 "e", 

 "e", 

 "e", 

 "e", 

 "e", 
 "e", 
 "e", 

 "e", 

 "e", 

 "e", 

 "e", 

 "e", 
 "e", 
 "e", 
 "e", 

 "e", 

 "e", 
 "e", 
 "e", 
 "e", 

 "e", 

 "e", 

 "e", 

 "e", 
 "e", 
 "e", 
 "e", 

 "e", 
 "e", 
 "e", 
 "e", 

 "e", 
 "e", 
 "e", 
 "e", 
 "e", 

 "e", 
 "e", 
 "e", 
 "e", 
 "e", 

 "e", 
 "e", 
 "e", 
 "e", 
 "e", 
 "e", 

 "e", 
 "e", 
 "e", 
 "e", 

 "e", 

 "e", 

 "e", 

 "e", 

 "e", 

 "e", 

 "e", 
 "e", 

 "e", 

 "e", 

 "e", 

 "e", 
 "e", 

 "e", 
 "e", 
 "e", 
 "e", 

};

int tree_code_length[] = {

 0, 

 7, 

 2, 

 2, 

 0, 	 

 0, 

 0, 

 0, 

 0, 

 0, 

 0, 

 0, 

 0, 

 0, 

 0, 

 0, 

 0, 

 0, 

 0, 

 0, 

 0, 	 

 0, 

 0, 

 1, 

 1, 

 1, 

 1, 

 5, 

 5, 

 3, 

 1, 

 3, 

 1, 

 1, 

 1, 

 2, 

 3, 

 3, 

 3, 

 0, 
 0, 
 0, 
 0, 
 0, 
 0, 
 0, 
 0, 
 0, 

 2, 

 1, 

 2, 

 1, 

 2, 

 2, 

 2, 

 2, 

 2, 

 2, 

 2, 

 3, 

 3, 

 4, 

 3, 

 2, 
 2, 
 2, 

 2, 

 2, 

 2, 

 2, 

 2, 
 2, 
 2, 
 2, 

 2, 

 1, 
 1, 
 1, 
 1, 

 1, 

 2, 

 1, 

 2, 
 2, 
 1, 
 1, 

 2, 
 2, 
 2, 
 2, 

 2, 
 2, 
 2, 
 2, 
 1, 

 2, 
 2, 
 2, 
 2, 
 1, 

 2, 
 2, 
 2, 
 2, 
 2, 
 2, 

 2, 
 2, 
 1, 
 2, 

 1, 

 1, 

 2, 

 2, 

 1, 

 1, 

 2, 
 2, 

 1, 

 2, 

 1, 

 1, 
 1, 

 2, 
 2, 
 2, 
 2, 

};

int tree_node_counter = 0;

static tree hash_table[1009 ];	 

void
init_tree ()
{
  _obstack_begin ((&permanent_obstack), 0, 0, xmalloc , free ) ;

  _obstack_begin ((&temporary_obstack), 0, 0, xmalloc , free ) ;
  temporary_firstobj = (char *) ({ struct obstack *__h = (&temporary_obstack);	({ struct obstack *__o = (__h);	int __len = ( ( 0));	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	__o->next_free += __len;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;
  _obstack_begin ((&momentary_obstack), 0, 0, xmalloc , free ) ;
  momentary_firstobj = (char *) ({ struct obstack *__h = (&momentary_obstack);	({ struct obstack *__o = (__h);	int __len = ( ( 0));	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	__o->next_free += __len;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;
  _obstack_begin ((&maybepermanent_obstack), 0, 0, xmalloc , free ) ;
  maybepermanent_firstobj
    = (char *) ({ struct obstack *__h = (&maybepermanent_obstack);	({ struct obstack *__o = (__h);	int __len = ( ( 0));	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	__o->next_free += __len;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;

  current_obstack = &permanent_obstack;
  expression_obstack = &permanent_obstack;
  rtl_obstack = saveable_obstack = &permanent_obstack;
  tree_node_counter = 1;
  memset (hash_table,0, sizeof hash_table) ;
}

void
temporary_allocation ()
{
  current_obstack = &temporary_obstack;
  expression_obstack = &temporary_obstack;
  rtl_obstack = saveable_obstack = &maybepermanent_obstack;
  momentary_stack = 0;
}

void
end_temporary_allocation ()
{
  current_obstack = &permanent_obstack;
  expression_obstack = &permanent_obstack;
  rtl_obstack = saveable_obstack = &permanent_obstack;
}

void
resume_temporary_allocation ()
{
  current_obstack = &temporary_obstack;
  expression_obstack = &temporary_obstack;
  rtl_obstack = saveable_obstack = &maybepermanent_obstack;
}

int
allocation_temporary_p ()
{
  return current_obstack == &temporary_obstack;
}

void
permanent_allocation ()
{
  ({ struct obstack *__o = (&temporary_obstack);	void *__obj = ( temporary_firstobj);	if (__obj >= (void *)__o->chunk && __obj < (void *)__o->chunk_limit) __o->next_free = __o->object_base = __obj;	else (obstack_free) (__o, __obj); }) ;
  ({ struct obstack *__o = (&momentary_obstack);	void *__obj = ( momentary_firstobj);	if (__obj >= (void *)__o->chunk && __obj < (void *)__o->chunk_limit) __o->next_free = __o->object_base = __obj;	else (obstack_free) (__o, __obj); }) ;
  ({ struct obstack *__o = (&maybepermanent_obstack);	void *__obj = ( maybepermanent_firstobj);	if (__obj >= (void *)__o->chunk && __obj < (void *)__o->chunk_limit) __o->next_free = __o->object_base = __obj;	else (obstack_free) (__o, __obj); }) ;

  current_obstack = &permanent_obstack;
  expression_obstack = &permanent_obstack;
  rtl_obstack = saveable_obstack = &permanent_obstack;
}

void
preserve_data ()
{
  maybepermanent_firstobj
    = (char *) ({ struct obstack *__h = (&maybepermanent_obstack);	({ struct obstack *__o = (__h);	int __len = ( ( 0));	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	__o->next_free += __len;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;
}

char *
oballoc (size)
     int size;
{
  return (char *) ({ struct obstack *__h = (current_obstack);	({ struct obstack *__o = (__h);	int __len = ( ( size));	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	__o->next_free += __len;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;
}

void
obfree (ptr)
     char *ptr;
{
  ({ struct obstack *__o = (current_obstack);	void *__obj = ( ptr);	if (__obj >= (void *)__o->chunk && __obj < (void *)__o->chunk_limit) __o->next_free = __o->object_base = __obj;	else (obstack_free) (__o, __obj); }) ;
}

char *
permalloc (size)
     long size;
{
  return (char *) ({ struct obstack *__h = (&permanent_obstack);	({ struct obstack *__o = (__h);	int __len = ( ( size));	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	__o->next_free += __len;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;
}

void
push_momentary ()
{
  struct momentary_level *tem
    = (struct momentary_level *) ({ struct obstack *__h = (&momentary_obstack);	({ struct obstack *__o = (__h);	int __len = ( (
						sizeof (struct momentary_level)));	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	__o->next_free += __len;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;
  tem->prev = momentary_stack;
  tem->base = (char *) ((&momentary_obstack)->object_base) ;
  tem->obstack = expression_obstack;
  momentary_stack = tem;
  expression_obstack = &momentary_obstack;
}

void
clear_momentary ()
{
  ({ struct obstack *__o = (&momentary_obstack);	void *__obj = ( momentary_stack->base);	if (__obj >= (void *)__o->chunk && __obj < (void *)__o->chunk_limit) __o->next_free = __o->object_base = __obj;	else (obstack_free) (__o, __obj); }) ;
}

void
pop_momentary ()
{
  struct momentary_level *tem = momentary_stack;
  momentary_stack = tem->prev;
  ({ struct obstack *__o = (&momentary_obstack);	void *__obj = ( tem);	if (__obj >= (void *)__o->chunk && __obj < (void *)__o->chunk_limit) __o->next_free = __o->object_base = __obj;	else (obstack_free) (__o, __obj); }) ;
  expression_obstack = tem->obstack;
}

int
suspend_momentary ()
{
  register int tem = expression_obstack == &momentary_obstack;
  expression_obstack = saveable_obstack;
  return tem;
}

void
resume_momentary (yes)
     int yes;
{
  if (yes)
    expression_obstack = &momentary_obstack;
}

tree
make_node (code)
     enum tree_code code;
{
  register tree t;
  register int type = *tree_code_type[(int) code];
  register int length;
  register struct obstack *obstack = current_obstack;
  register int i;

  switch (type)
    {
    case 'd':   
      length = sizeof (struct tree_decl);
      if (obstack != &permanent_obstack)
	obstack = saveable_obstack;
      break;

    case 't':   
      length = sizeof (struct tree_type);
      if (obstack != &permanent_obstack)
	obstack = all_types_permanent ? &permanent_obstack : saveable_obstack;
      break;

    case 's':   
      length = sizeof (struct tree_common)
	+ 2 * sizeof (int)
	  + tree_code_length[(int) code] * sizeof (char *);
      if (obstack != &permanent_obstack)
	obstack = saveable_obstack;
      break;

    case 'r':   
    case 'e':   
      obstack = expression_obstack;
      length = sizeof (struct tree_exp)
	+ (tree_code_length[(int) code] - 1) * sizeof (char *);
      break;

    case 'c':   
      obstack = expression_obstack;

      if (code == REAL_CST)
	{
	  length = sizeof (struct tree_real_cst);
	  break;
	}

    case 'x':   
      length = sizeof (struct tree_common)
	+ tree_code_length[(int) code] * sizeof (char *);

      if (code == IDENTIFIER_NODE) obstack = &permanent_obstack;
    }

  t = (tree) ({ struct obstack *__h = (obstack);	({ struct obstack *__o = (__h);	int __len = ( ( length));	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	__o->next_free += __len;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;

  ((t)->common.uid)  = tree_node_counter++;
  ((t)->common.type)  = 0;
  ((t)->common.chain)  = 0;
  for (i = (length / sizeof (int)) - 1;
       i >= sizeof (struct tree_common) / sizeof (int) - 1;
       i--)
    ((int *) t)[i] = 0;

  ((t)->common.code = ( code)) ;
  if (obstack == &permanent_obstack)
    ((t)->common.permanent_attr)  = 1;

  if (type == 'd')
    {
      extern int lineno;

      ((t)->decl.align)  = 1;
      ((t)->decl.size_unit)  = 1;
      ((t)->decl.voffset_unit)  = 1;
      ((t)->decl.linenum)  = lineno;
      ((t)->decl.filename)  = input_filename;
    }

  if (type == 't')
    {
      ((t)->type.align)  = 1;
      ((t)->type.size_unit)  = 1;
      ((t)->type.main_variant)  = t;
    }

  if (type == 'c')
    {
      ((t)->common.literal_attr)  = 1;
    }

  return t;
}

tree
copy_node (node)
     tree node;
{
  register tree t;
  register enum tree_code code = ((node)->common.code) ;
  register int length;
  register int i;

  switch (*tree_code_type[(int) code])
    {
    case 'd':   
      length = sizeof (struct tree_decl);
      break;

    case 't':   
      length = sizeof (struct tree_type);
      break;

    case 's':
      length = sizeof (struct tree_common)
	+ 2 * sizeof (int)
	  + tree_code_length[(int) code] * sizeof (char *);
      break;

    case 'r':   
    case 'e':   
      length = sizeof (struct tree_exp)
	+ (tree_code_length[(int) code] - 1) * sizeof (char *);
      break;

    case 'c':   

      if (code == REAL_CST)
	{
	  length = sizeof (struct tree_real_cst);
	  break;
	}

    case 'x':   
      length = sizeof (struct tree_common)
	+ tree_code_length[(int) code] * sizeof (char *);
    }

  t = (tree) ({ struct obstack *__h = (current_obstack);	({ struct obstack *__o = (__h);	int __len = ( ( length));	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	__o->next_free += __len;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;

  for (i = ((length + sizeof (int) - 1) / sizeof (int)) - 1;
       i >= 0;
       i--)
    ((int *) t)[i] = ((int *) node)[i];

  ((t)->common.uid)  = tree_node_counter++;
  ((t)->common.chain)  = 0;

  ((t)->common.permanent_attr)  = (current_obstack == &permanent_obstack);

  return t;
}

tree
get_identifier (text)
     register char *text;
{
  register int hi;
  register int i;
  register tree idp;
  register int len, hash_len;

  for (len = 0; text[len]; len++);

  hash_len = len;
  if (warn_id_clash && len > id_clash_len)
    hash_len = id_clash_len;

  hi = hash_len;
  for (i = 0; i < hash_len; i++)
    hi = ((hi * 613) + (unsigned)(text[i]));

  hi &= (1 << 30 ) - 1;
  hi %= 1009 ;
  for (idp = hash_table[hi]; idp; idp = ((idp)->common.chain) )
    if (((idp)->identifier.length)  == len
	&& !strcmp (((idp)->identifier.pointer) , text))
      return idp;		 
  if (warn_id_clash && len > id_clash_len)
    for (idp = hash_table[hi]; idp; idp = ((idp)->common.chain) )
      if (!strncmp (((idp)->identifier.pointer) , text, id_clash_len))
	{
	  warning ("`%s' and `%s' identical in first n characters",
		   ((idp)->identifier.pointer) , text);
	  break;
	}

  idp = make_node (IDENTIFIER_NODE);
  ((idp)->identifier.length)  = len;

  ((idp)->identifier.pointer)  = ({ struct obstack *__h = (&permanent_obstack);	({ struct obstack *__o = (__h);	int __len = ( ( len));	((__o->next_free + __len + 1 > __o->chunk_limit)	? _obstack_newchunk (__o, __len + 1) : 0),	memcpy ( __o->next_free, ( text), __len) ,	__o->next_free += __len,	*(__o->next_free)++ = 0;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;

  ((idp)->common.chain)  = hash_table[hi];
  hash_table[hi] = idp;
  return idp;			 
}

tree
build_int_2 (low, hi)
     int low, hi;
{
  register tree t = make_node (INTEGER_CST);
  ((t)->int_cst.int_cst_low)  = low;
  ((t)->int_cst.int_cst_high)  = hi;
  ((t)->common.type)  = integer_type_node;
  return t;
}

tree
build_real (type, d)
     tree type;
     double  d;
{
  tree v;

  v = make_node (REAL_CST);
  ((v)->common.type)  = type;
  ((v)->real_cst.real_cst)  = d;
  return v;
}

tree
build_real_from_int_cst (type, i)
     tree type;
     tree i;
{
  tree v;
  double d;

  v = make_node (REAL_CST);
  ((v)->common.type)  = type;

  if (((i)->int_cst.int_cst_high)  < 0)
    {
      d = (double) (~ ((i)->int_cst.int_cst_high) );
      d *= ((double) (1 << (32  / 2))
	    * (double) (1 << (32  / 2)));
      d += (double) (unsigned) (~ ((i)->int_cst.int_cst_low) );
      d = (- d - 1.0);
    }
  else
    {
      d = (double) ((i)->int_cst.int_cst_high) ;
      d *= ((double) (1 << (32  / 2))
	    * (double) (1 << (32  / 2)));
      d += (double) (unsigned) ((i)->int_cst.int_cst_low) ;
    }

  ((v)->real_cst.real_cst)  = d;
  return v;
}

tree
build_string (len, str)
     int len;
     char *str;
{
  register tree s = make_node (STRING_CST);
  ((s)->string.length)  = len;
  ((s)->string.pointer)  = ({ struct obstack *__h = (saveable_obstack);	({ struct obstack *__o = (__h);	int __len = ( ( len));	((__o->next_free + __len + 1 > __o->chunk_limit)	? _obstack_newchunk (__o, __len + 1) : 0),	memcpy ( __o->next_free, ( str), __len) ,	__o->next_free += __len,	*(__o->next_free)++ = 0;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;
  return s;
}

tree
build_complex (real, imag)
     tree real, imag;
{
  register tree t = make_node (COMPLEX_CST);
  ((t)->complex.real)  = real;
  ((t)->complex.imag)  = imag;
  return t;
}

int
integer_zerop (expr)
     tree expr;
{
  return (((expr)->common.code)  == INTEGER_CST
	  && ((expr)->int_cst.int_cst_low)  == 0
	  && ((expr)->int_cst.int_cst_high)  == 0);
}

int
integer_onep (expr)
     tree expr;
{
  return (((expr)->common.code)  == INTEGER_CST
	  && ((expr)->int_cst.int_cst_low)  == 1
	  && ((expr)->int_cst.int_cst_high)  == 0);
}

int
integer_all_onesp (expr)
     tree expr;
{
  register int prec;
  register int uns;

  if (((expr)->common.code)  != INTEGER_CST)
    return 0;

  uns = ((((expr)->common.type) )->common.unsigned_attr) ;
  if (!uns)
    return ((expr)->int_cst.int_cst_low)  == -1 && ((expr)->int_cst.int_cst_high)  == -1;

  prec = ((((expr)->common.type) )->type.sep_unit) ;
  if (prec >= 32 )
    return ((expr)->int_cst.int_cst_low)  == -1
      && ((expr)->int_cst.int_cst_high)  == (1 << (prec - 32 )) - 1;
  else
    return ((expr)->int_cst.int_cst_low)  == (1 << prec) - 1;
}

int
list_length (t)
     tree t;
{
  register tree tail;
  register int len = 0;

  for (tail = t; tail; tail = ((tail)->common.chain) )
    len++;

  return len;
}

tree
chainon (op1, op2)
     tree op1, op2;
{
  tree t;

  if (op1)
    {
      for (t = op1; ((t)->common.chain) ; t = ((t)->common.chain) )
	if (t == op2) abort ();	 
      ((t)->common.chain)  = op2;
      return op1;
    }
  else return op2;
}

tree
build_tree_list (parm, value)
     tree parm, value;
{
  register tree t = make_node (TREE_LIST);
  ((t)->list.purpose)  = parm;
  ((t)->list.value)  = value;
  return t;
}

tree
tree_cons (purpose, value, chain)
     tree purpose, value, chain;
{
  register tree node = make_node (TREE_LIST);
  ((node)->common.chain)  = chain;
  ((node)->list.purpose)  = purpose;
  ((node)->list.value)  = value;
  return node;
}

tree
perm_tree_cons (purpose, value, chain)
     tree purpose, value, chain;
{
  register tree node;
  register struct obstack *ambient_obstack = current_obstack;
  current_obstack = &permanent_obstack;

  node = make_node (TREE_LIST);
  ((node)->common.chain)  = chain;
  ((node)->list.purpose)  = purpose;
  ((node)->list.value)  = value;

  current_obstack = ambient_obstack;
  return node;
}

tree
temp_tree_cons (purpose, value, chain)
     tree purpose, value, chain;
{
  register tree node;
  register struct obstack *ambient_obstack = current_obstack;
  current_obstack = &temporary_obstack;

  node = make_node (TREE_LIST);
  ((node)->common.chain)  = chain;
  ((node)->list.purpose)  = purpose;
  ((node)->list.value)  = value;

  current_obstack = ambient_obstack;
  return node;
}

tree
saveable_tree_cons (purpose, value, chain)
     tree purpose, value, chain;
{
  register tree node;
  register struct obstack *ambient_obstack = current_obstack;
  current_obstack = saveable_obstack;

  node = make_node (TREE_LIST);
  ((node)->common.chain)  = chain;
  ((node)->list.purpose)  = purpose;
  ((node)->list.value)  = value;

  current_obstack = ambient_obstack;
  return node;
}

tree
tree_last (chain)
     register tree chain;
{
  register tree next;
  if (chain)
    while (next = ((chain)->common.chain) )
      chain = next;
  return chain;
}

tree
nreverse (t)
     tree t;
{
  register tree prev = 0, decl, next;
  for (decl = t; decl; decl = next)
    {
      next = ((decl)->common.chain) ;
      ((decl)->common.chain)  = prev;
      prev = decl;
    }
  return prev;
}

tree
size_in_bytes (type)
     tree type;
{
  if (type == error_mark_node)
    return integer_zero_node;
  type = ((type)->type.main_variant) ;
  if (((type)->type.size)  == 0)
    {
      incomplete_type_error (0, type);
      return integer_zero_node;
    }
  return convert_units (((type)->type.size) , ((type)->type.size_unit) ,
			8 );
}

int
int_size_in_bytes (type)
     tree type;
{
  int size;
  if (type == error_mark_node)
    return 0;
  type = ((type)->type.main_variant) ;
  if (((type)->type.size)  == 0)
    return -1;
  if (((((type)->type.size) )->common.code)  != INTEGER_CST)
    return -1;
  size = ((((type)->type.size) )->int_cst.int_cst_low)  * ((type)->type.size_unit) ;
  return (size + 8  - 1) / 8 ;
}

tree
array_type_nelts (type)
     tree type;
{
  tree index_type = ((type)->type.values) ;
  return (tree_int_cst_equal (((index_type)->type.sep) , integer_zero_node)
	  ? ((index_type)->type.max) 
	  : fold (build (MINUS_EXPR, integer_type_node,
			 ((index_type)->type.max) ,
			 ((index_type)->type.sep) )));
}

int
staticp (arg)
     tree arg;
{
  register enum tree_code code = ((arg)->common.code) ;

  if ((code == VAR_DECL || code == FUNCTION_DECL || code == CONSTRUCTOR)
      && (((arg)->common.static_attr)  || ((arg)->common.external_attr) ))
    return 1;

  if (code == STRING_CST)
    return 1;

  if (code == COMPONENT_REF)
    return (((((arg)->exp.operands[ 1]) )->decl.voffset)   == 0
	    && staticp (((arg)->exp.operands[ 0]) ));

  if (code == INDIRECT_REF)
    return ((((arg)->exp.operands[ 0]) )->common.literal_attr) ;

  if (code == ARRAY_REF)
    {
      if (((((((arg)->common.type) )->type.size) )->common.code)  == INTEGER_CST
	  && ((((arg)->exp.operands[ 1]) )->common.code)  == INTEGER_CST)
	return staticp (((arg)->exp.operands[ 0]) );
    }

  return 0;
}

int
lvalue_p (ref)
     tree ref;
{
  register enum tree_code code = ((ref)->common.code) ;

  if (language_lvalue_valid (ref))
    switch (code)
      {
      case COMPONENT_REF:
	return lvalue_p (((ref)->exp.operands[ 0]) );

      case STRING_CST:
	return 1;

      case INDIRECT_REF:
      case ARRAY_REF:
      case VAR_DECL:
      case PARM_DECL:
      case RESULT_DECL:
      case ERROR_MARK:
	if (((((ref)->common.type) )->common.code)  != FUNCTION_TYPE)
	  return 1;
	break;

      case CALL_EXPR:
	if (((((ref)->common.type) )->common.code)  == REFERENCE_TYPE)
	  return 1;
      }
  return 0;
}

int
lvalue_or_else (ref, string)
     tree ref;
     char *string;
{
  int win = lvalue_p (ref);
  if (! win)
    error ("invalid lvalue in %s", string);
  return win;
}

tree
save_expr (expr)
     tree expr;
{
  register tree t = fold (expr);

  if (((t)->common.literal_attr)  || ((t)->common.readonly_attr)  || ((t)->common.code)  == SAVE_EXPR)
    return t;

  return build (SAVE_EXPR, ((expr)->common.type) , t, 0 );
}

tree
stabilize_reference (ref)
     tree ref;
{
  register tree result;
  register enum tree_code code = ((ref)->common.code) ;

  switch (code)
    {
    case VAR_DECL:
    case PARM_DECL:
    case RESULT_DECL:
      result = ref;
      break;

    case NOP_EXPR:
    case CONVERT_EXPR:
    case FLOAT_EXPR:
    case FIX_TRUNC_EXPR:
    case FIX_FLOOR_EXPR:
    case FIX_ROUND_EXPR:
    case FIX_CEIL_EXPR:
      result = build_nt (code, stabilize_reference (((ref)->exp.operands[ 0]) ));
      break;

    case INDIRECT_REF:
      result = build_nt (INDIRECT_REF, save_expr (((ref)->exp.operands[ 0]) ));
      break;

    case COMPONENT_REF:
      result = build_nt (COMPONENT_REF,
			 stabilize_reference (((ref)->exp.operands[ 0]) ),
			 ((ref)->exp.operands[ 1]) );
      break;

    case ARRAY_REF:
      result = build_nt (ARRAY_REF, stabilize_reference (((ref)->exp.operands[ 0]) ),
			 save_expr (((ref)->exp.operands[ 1]) ));
      break;

    default:
      return ref;

    case ERROR_MARK:
      return error_mark_node;
    }

  ((result)->common.type)  = ((ref)->common.type) ;
  ((result)->common.readonly_attr)  = ((ref)->common.readonly_attr) ;
  ((result)->common.volatile_attr)  = ((ref)->common.volatile_attr) ;
  ((result)->common.this_vol_attr)  = ((ref)->common.this_vol_attr) ;

  return result;
}

tree
build ( __builtin_va_alist )
        int __builtin_va_alist; 
{
  register   char *  p;
  enum tree_code code;
  register tree t;
  register int length;
  register int i;

   p=(char *) &__builtin_va_alist ;

  code = (p += (((sizeof ( enum tree_code) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( enum tree_code *) (p - (((sizeof ( enum tree_code) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;
  t = make_node (code);
  length = tree_code_length[(int) code];
  ((t)->common.type)  = (p += (((sizeof ( tree) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( tree *) (p - (((sizeof ( tree) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;

  if (length == 2)
    {
      register tree arg0 = (p += (((sizeof ( tree) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( tree *) (p - (((sizeof ( tree) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;
      register tree arg1 = (p += (((sizeof ( tree) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( tree *) (p - (((sizeof ( tree) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;
      ((t)->exp.operands[ 0])  = arg0;
      ((t)->exp.operands[ 1])  = arg1;
      ((t)->common.volatile_attr) 
	= (arg0 && ((arg0)->common.volatile_attr) ) || (arg1 && ((arg1)->common.volatile_attr) );
    }
  else
    {
      for (i = 0; i < length; i++)
	{
	  register tree operand = (p += (((sizeof ( tree) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( tree *) (p - (((sizeof ( tree) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;
	  ((t)->exp.operands[ i])  = operand;
	  if (operand && ((operand)->common.volatile_attr) )
	    ((t)->common.volatile_attr)  = 1;
	}
    }
   ;
  return t;
}

tree
build_nt ( __builtin_va_alist )
        int __builtin_va_alist; 
{
  register   char *  p;
  register enum tree_code code;
  register tree t;
  register int length;
  register int i;

   p=(char *) &__builtin_va_alist ;

  code = (p += (((sizeof ( enum tree_code) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( enum tree_code *) (p - (((sizeof ( enum tree_code) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;
  t = make_node (code);
  length = tree_code_length[(int) code];

  for (i = 0; i < length; i++)
    ((t)->exp.operands[ i])  = (p += (((sizeof ( tree) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( tree *) (p - (((sizeof ( tree) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;

   ;
  return t;
}

tree
build_op_identifier (op1, op2)
     tree op1, op2;
{
  register tree t = make_node (OP_IDENTIFIER);
  ((t)->list.purpose)  = op1;
  ((t)->list.value)  = op2;
  return t;
}

tree
build_decl (code, name, type)
     enum tree_code code;
     tree name, type;
{
  register tree t;

  t = make_node (code);

  ((t)->decl.name)  = name;
  ((t)->common.type)  = type;
  ((t)->decl.arguments)   = (tree) 0  ;
  ((t)->decl.initial)  = (tree) 0  ;

  if (code == VAR_DECL || code == PARM_DECL || code == RESULT_DECL)
    layout_decl (t, 0);
  else if (code == FUNCTION_DECL)
    ((t)->decl.mode)  = QImode ;

  return t;
}

tree
build_let (filename, line, vars, body, supercontext, tags)
     char *filename;
     int line;
     tree vars, body, supercontext, tags;
{
  register tree t = make_node (LET_STMT);
  ((t)->stmt.filename)  = filename;
  ((t)->stmt.linenum)  = line;
  ((t)->bind_stmt.vars)  = vars;
  ((t)->stmt.body)  = body;
  ((t)->bind_stmt.supercontext)  = supercontext;
  ((t)->bind_stmt.bind_size)  = 0;
  ((t)->bind_stmt.type_tags)  = tags;
  return t;
}

tree
build_type_variant (type, constp, volatilep)
     tree type;
     int constp, volatilep;
{
  register tree t, m = ((type)->type.main_variant) ;
  register struct obstack *ambient_obstack = current_obstack;

  constp = !!constp;
  volatilep = !!volatilep;

  for (t = m; t; t = ((t)->type.next_variant) )
    if (constp == ((t)->common.readonly_attr) 
	&& volatilep == ((t)->common.volatile_attr) )
      return t;

  current_obstack
    = ((type)->common.permanent_attr)  ? &permanent_obstack : saveable_obstack;

  t = copy_node (type);
  ((t)->common.readonly_attr)  = constp;
  ((t)->common.volatile_attr)  = volatilep;
  ((t)->type.pointer_to)  = 0;
  ((t)->type.reference_to)  = 0;

  ((t)->type.next_variant)  = ((m)->type.next_variant) ;
  ((m)->type.next_variant)  = t;

  current_obstack = ambient_obstack;
  return t;
}

struct type_hash
{
  struct type_hash *next;	 
  int hashcode;			 
  tree type;			 
};

struct type_hash *type_hash_table[59 ];

int
type_hash_list (list)
     tree list;
{
  register int hashcode;
  register tree tail;
  for (hashcode = 0, tail = list; tail; tail = ((tail)->common.chain) )
    hashcode += ((((tail)->list.value) )->common.uid)  ;
  return hashcode;
}

tree
type_hash_lookup (hashcode, type)
     int hashcode;
     tree type;
{
  register struct type_hash *h;
  for (h = type_hash_table[hashcode % 59 ]; h; h = h->next)
    if (h->hashcode == hashcode
	&& ((h->type)->common.code)  == ((type)->common.code) 
	&& ((h->type)->common.type)  == ((type)->common.type) 
	&& (((h->type)->type.max)  == ((type)->type.max) 
	    || tree_int_cst_equal (((h->type)->type.max) ,
				   ((type)->type.max) ))
	&& (((h->type)->type.sep)  == ((type)->type.sep) 
	    || tree_int_cst_equal (((h->type)->type.sep) ,
				   ((type)->type.sep) ))
	&& (((h->type)->type.values)  == ((type)->type.values) 
	    || (((((h->type)->type.values) )->common.code)  == TREE_LIST
		&& ((((type)->type.values) )->common.code)  == TREE_LIST
		&& type_list_equal (((h->type)->type.values) , ((type)->type.values) ))))
      return h->type;
  return 0;
}

void
type_hash_add (hashcode, type)
     int hashcode;
     tree type;
{
  register struct type_hash *h;

  h = (struct type_hash *) oballoc (sizeof (struct type_hash));
  h->hashcode = hashcode;
  h->type = type;
  h->next = type_hash_table[hashcode % 59 ];
  type_hash_table[hashcode % 59 ] = h;
}

int debug_no_type_hash = 0;

tree
type_hash_canon (hashcode, type)
     int hashcode;
     tree type;
{
  tree t1;

  if (debug_no_type_hash)
    return type;

  t1 = type_hash_lookup (hashcode, type);
  if (t1 != 0)
    {
      struct obstack *o
	= ((type)->common.permanent_attr)  ? &permanent_obstack : saveable_obstack;
      ({ struct obstack *__o = (o);	void *__obj = ( type);	if (__obj >= (void *)__o->chunk && __obj < (void *)__o->chunk_limit) __o->next_free = __o->object_base = __obj;	else (obstack_free) (__o, __obj); }) ;
      return t1;
    }

  if (current_obstack == &permanent_obstack)
    type_hash_add (hashcode, type);

  return type;
}

int
type_list_equal (l1, l2)
     tree l1, l2;
{
  register tree t1, t2;
  for (t1 = l1, t2 = l2; t1 && t2; t1 = ((t1)->common.chain) , t2 = ((t2)->common.chain) )
    {
      if (((t1)->list.value)  != ((t2)->list.value) )
	return 0;
      if (((t1)->list.purpose)  != ((t2)->list.purpose) 
	  && !simple_cst_equal (((t1)->list.purpose) , ((t2)->list.purpose) ))
	return 0;
    }

  return t1 == t2;
}

int
tree_int_cst_equal (t1, t2)
     tree t1, t2;
{
  if (t1 == t2)
    return 1;
  if (t1 == 0 || t2 == 0)
    return 0;
  if (((t1)->common.code)  == INTEGER_CST
      && ((t2)->common.code)  == INTEGER_CST
      && ((t1)->int_cst.int_cst_low)  == ((t2)->int_cst.int_cst_low) 
      && ((t1)->int_cst.int_cst_high)  == ((t2)->int_cst.int_cst_high) )
    return 1;
  return 0;
}

int
tree_int_cst_lt (t1, t2)
     tree t1, t2;
{
  if (t1 == t2)
    return 0;

  if (!((((t1)->common.type) )->common.unsigned_attr) )
    return (((t1)->int_cst.int_cst_high)  < (( t2)->int_cst.int_cst_high) 	|| (((t1)->int_cst.int_cst_high)  == (( t2)->int_cst.int_cst_high) 	&& ((unsigned) ((t1)->int_cst.int_cst_low)  < (unsigned) (( t2)->int_cst.int_cst_low) ))) ;
  return ((unsigned) ((t1)->int_cst.int_cst_high)  < (unsigned) (( t2)->int_cst.int_cst_high) 	|| ((unsigned) ((t1)->int_cst.int_cst_high)  == (unsigned) (( t2)->int_cst.int_cst_high)  && ((unsigned) ((t1)->int_cst.int_cst_low)  < (unsigned) (( t2)->int_cst.int_cst_low) ))) ;
}

int
simple_cst_equal (t1, t2)
     tree t1, t2;
{
  register enum tree_code code1, code2;

  if (t1 == t2)
    return 1;
  if (t1 == 0 || t2 == 0)
    return 0;

  code1 = ((t1)->common.code) ;
  code2 = ((t2)->common.code) ;

  if (code1 == NOP_EXPR || code1 == CONVERT_EXPR)
    if (code2 == NOP_EXPR || code2 == CONVERT_EXPR)
      return simple_cst_equal (((t1)->exp.operands[ 0]) , ((t2)->exp.operands[ 0]) );
    else
      return simple_cst_equal (((t1)->exp.operands[ 0]) , t2);
  else if (code2 == NOP_EXPR || code2 == CONVERT_EXPR)
    return simple_cst_equal (t1, ((t2)->exp.operands[ 0]) );

  if (code1 != code2)
    return 0;

  switch (code1)
    {
    case INTEGER_CST:
      return ((t1)->int_cst.int_cst_low)  == ((t2)->int_cst.int_cst_low) 
	&& ((t1)->int_cst.int_cst_high)  == ((t2)->int_cst.int_cst_high) ;

    case REAL_CST:
      return ((((t1)->real_cst.real_cst) ) == ( ((t2)->real_cst.real_cst) )) ;

    case STRING_CST:
      return ((t1)->string.length)  == ((t2)->string.length) 
	&& !strcmp (((t1)->string.pointer) , ((t2)->string.pointer) );

    case CONSTRUCTOR:
      abort ();

    case VAR_DECL:
    case PARM_DECL:
    case CONST_DECL:
      return 0;

    case PLUS_EXPR:
    case MINUS_EXPR:
    case MULT_EXPR:
    case TRUNC_DIV_EXPR:
    case TRUNC_MOD_EXPR:
    case LSHIFT_EXPR:
    case RSHIFT_EXPR:
      return (simple_cst_equal (((t1)->exp.operands[ 0]) , ((t2)->exp.operands[ 0]) )
	      && simple_cst_equal (((t1)->exp.operands[ 1]) , ((t2)->exp.operands[ 1]) ));

    case NEGATE_EXPR:
    case ADDR_EXPR:
    case REFERENCE_EXPR:
      return simple_cst_equal (((t1)->exp.operands[ 0]) , ((t2)->exp.operands[ 0]) );

    default:
      abort ();
    }
}

tree
build_pointer_type (to_type)
     tree to_type;
{
  register tree t = ((to_type)->type.pointer_to) ;
  register struct obstack *ambient_obstack = current_obstack;
  register struct obstack *ambient_saveable_obstack = saveable_obstack;

  if (t)
    return t;

  if (((to_type)->common.permanent_attr) )
    {
      current_obstack = &permanent_obstack;
      saveable_obstack = &permanent_obstack;
    }

  t = make_node (POINTER_TYPE);
  ((t)->common.type)  = to_type;

  ((to_type)->type.pointer_to)  = t;

  layout_type (t);

  current_obstack = ambient_obstack;
  saveable_obstack = ambient_saveable_obstack;
  return t;
}

tree
build_index_type (maxval)
     tree maxval;
{
  register tree itype = make_node (INTEGER_TYPE);
  int maxint = ((maxval)->int_cst.int_cst_low) ;
  ((itype)->type.sep_unit)  = 32 ;
  ((itype)->type.sep)  = build_int_2 (0, 0);
  ((((itype)->type.sep) )->common.type)  = itype;
  ((itype)->type.max)  = maxval;
  ((maxval)->common.type)  = itype;
  ((itype)->type.mode)  = SImode;
  ((itype)->type.size)  = ((sizetype)->type.size) ;
  ((itype)->type.size_unit)  = ((sizetype)->type.size_unit) ;
  ((itype)->type.align)  = ((sizetype)->type.align) ;
  return type_hash_canon (maxint > 0 ? maxint : - maxint, itype);
}

tree
build_array_type (elt_type, index_type)
     tree elt_type, index_type;
{
  register tree t = make_node (ARRAY_TYPE);
  int hashcode;

  if (((elt_type)->common.code)  == FUNCTION_TYPE)
    {
      error ("arrays of functions are not meaningful");
      elt_type = integer_type_node;
    }

  ((t)->common.type)  = elt_type;
  ((t)->type.values)  = index_type;

  build_pointer_type (elt_type);

  if (index_type == 0)
    return t;

  hashcode = ((elt_type)->common.uid)   + ((index_type)->common.uid)  ;
  t = type_hash_canon (hashcode, t);

  if (((t)->type.size)  == 0)
    layout_type (t);
  return t;
}

tree
build_function_type (value_type, arg_types)
     tree value_type, arg_types;
{
  register tree t;
  int hashcode;

  if (((value_type)->common.code)  == FUNCTION_TYPE
      || ((value_type)->common.code)  == ARRAY_TYPE)
    {
      error ("function return type cannot be function or array");
      value_type = integer_type_node;
    }

  t = make_node (FUNCTION_TYPE);
  ((t)->common.type)  = value_type;
  ((t)->type.values)  = arg_types;

  hashcode = ((value_type)->common.uid)   + type_hash_list (arg_types);
  t = type_hash_canon (hashcode, t);

  if (((t)->type.size)  == 0)
    layout_type (t);
  return t;
}

tree
build_reference_type (to_type)
     tree to_type;
{
  register tree t = ((to_type)->type.reference_to) ;
  register struct obstack *ambient_obstack = current_obstack;
  register struct obstack *ambient_saveable_obstack = saveable_obstack;

  if (t)
    return t;

  if (((to_type)->common.permanent_attr) )
    {
      current_obstack = &permanent_obstack;
      saveable_obstack = &permanent_obstack;
    }

  t = make_node (REFERENCE_TYPE);
  ((t)->common.type)  = to_type;

  ((to_type)->type.reference_to)  = t;

  layout_type (t);

  current_obstack = ambient_obstack;
  saveable_obstack = ambient_saveable_obstack;
  return t;
}

tree
build_method_type (basetype, type)
     tree basetype, type;
{
  register tree t;
  int hashcode;

  t = make_node (METHOD_TYPE);

  if (((type)->common.code)  != FUNCTION_TYPE)
    abort ();

  ((t)->type.max)  = basetype;
  ((t)->common.type)  = ((type)->common.type) ;

  ((t)->type.values) 
    = tree_cons (0 , build_pointer_type (basetype), ((type)->type.values) );

  hashcode = ((basetype)->common.uid)   + ((type)->common.uid)  ;
  t = type_hash_canon (hashcode, t);

  if (((t)->type.size)  == 0)
    layout_type (t);

  return t;
}

tree
build_offset_type (basetype, type)
     tree basetype, type;
{
  register tree t;
  int hashcode;

  t = make_node (OFFSET_TYPE);

  ((t)->type.max)  = basetype;
  ((t)->common.type)  = type;

  hashcode = ((basetype)->common.uid)   + ((type)->common.uid)  ;
  t = type_hash_canon (hashcode, t);

  if (((t)->type.size)  == 0)
    layout_type (t);

  return t;
}

tree
get_unwidened (op, for_type)
     register tree op;
     tree for_type;
{

  register tree type = ((op)->common.type) ;
  register int final_prec = ((for_type != 0 ? for_type : type)->type.sep_unit) ;
  register int uns
    = (for_type != 0 && for_type != type
       && final_prec > ((type)->type.sep_unit) 
       && ((type)->common.unsigned_attr) );
  register tree win = op;

  while (((op)->common.code)  == NOP_EXPR)
    {
      register int bitschange
	= ((((op)->common.type) )->type.sep_unit) 
	  - ((((((op)->exp.operands[ 0]) )->common.type) )->type.sep_unit) ;

      if (bitschange < 0
	  && final_prec > ((((op)->common.type) )->type.sep_unit) )
	break;

      op = ((op)->exp.operands[ 0]) ;

      if (bitschange > 0)
	{
	  if (! uns || final_prec <= ((((op)->common.type) )->type.sep_unit) )
	    win = op;

	  if ((uns || ((op)->common.code)  == NOP_EXPR)
	      && ((((op)->common.type) )->common.unsigned_attr) )
	    {
	      uns = 1;
	      win = op;
	    }
	}
    }

  if (((op)->common.code)  == COMPONENT_REF
      && ((type)->common.code)  != REAL_TYPE)
    {
      int innerprec = (((((((op)->exp.operands[ 1]) )->decl.size) )->int_cst.int_cst_low) 
		       * ((((op)->exp.operands[ 1]) )->decl.size_unit) );
      type = type_for_size (innerprec, ((((op)->exp.operands[ 1]) )->common.unsigned_attr) );

      if (innerprec < ((((op)->common.type) )->type.sep_unit) 
	  && (for_type || ((((op)->exp.operands[ 1]) )->decl.mode)  != BImode)
	  && (! uns || final_prec <= innerprec
	      || ((((op)->exp.operands[ 1]) )->common.unsigned_attr) )
	  && type != 0)
	{
	  win = build (COMPONENT_REF, type, ((op)->exp.operands[ 0]) ,
		       ((op)->exp.operands[ 1]) );
	  ((win)->common.volatile_attr)  = ((op)->common.volatile_attr) ;
	  ((win)->common.this_vol_attr)  = ((op)->common.this_vol_attr) ;
	}
    }
  return win;
}

tree
get_narrower (op, unsignedp_ptr)
     register tree op;
     int *unsignedp_ptr;
{
  register int uns = 0;
  int first = 1;
  register tree win = op;

  while (((op)->common.code)  == NOP_EXPR)
    {
      register int bitschange
	= ((((op)->common.type) )->type.sep_unit) 
	  - ((((((op)->exp.operands[ 0]) )->common.type) )->type.sep_unit) ;

      if (bitschange < 0)
	break;

      op = ((op)->exp.operands[ 0]) ;

      if (bitschange > 0)
	{

	  if (first)
	    uns = ((((op)->common.type) )->common.unsigned_attr) ;

	  else if (uns != ((((op)->common.type) )->common.unsigned_attr) )
	    break;
	  first = 0;
	}

      win = op;
    }

  if (((op)->common.code)  == COMPONENT_REF
      && ((((op)->common.type) )->common.code)  != REAL_TYPE)
    {
      int innerprec = (((((((op)->exp.operands[ 1]) )->decl.size) )->int_cst.int_cst_low) 
		       * ((((op)->exp.operands[ 1]) )->decl.size_unit) );
      tree type = type_for_size (innerprec, ((op)->common.unsigned_attr) );

      if (innerprec < ((((op)->common.type) )->type.sep_unit) 
	  && ((((op)->exp.operands[ 1]) )->decl.mode)  != BImode
	  && (first || uns == ((((op)->exp.operands[ 1]) )->common.unsigned_attr) )
	  && type != 0)
	{
	  if (first)
	    uns = ((((op)->exp.operands[ 1]) )->common.unsigned_attr) ;
	  win = build (COMPONENT_REF, type, ((op)->exp.operands[ 0]) ,
		       ((op)->exp.operands[ 1]) );
	  ((win)->common.volatile_attr)  = ((op)->common.volatile_attr) ;
	  ((win)->common.this_vol_attr)  = ((op)->common.this_vol_attr) ;
	}
    }
  *unsignedp_ptr = uns;
  return win;
}

int
type_precision (type)
     register tree type;
{
  return ((((type)->common.code)  == INTEGER_TYPE
	   || ((type)->common.code)  == ENUMERAL_TYPE
	   || ((type)->common.code)  == REAL_TYPE)
	  ? ((type)->type.sep_unit)  : 32 );
}

int
int_fits_type_p (c, type)
     tree c, type;
{
  if (((type)->common.unsigned_attr) )
    return (!((unsigned) ((((type)->type.max) )->int_cst.int_cst_high)  < (unsigned) (( c)->int_cst.int_cst_high) 	|| ((unsigned) ((((type)->type.max) )->int_cst.int_cst_high)  == (unsigned) (( c)->int_cst.int_cst_high)  && ((unsigned) ((((type)->type.max) )->int_cst.int_cst_low)  < (unsigned) (( c)->int_cst.int_cst_low) ))) 
	    && !((unsigned) ((c)->int_cst.int_cst_high)  < (unsigned) (( ((type)->type.sep) )->int_cst.int_cst_high) 	|| ((unsigned) ((c)->int_cst.int_cst_high)  == (unsigned) (( ((type)->type.sep) )->int_cst.int_cst_high)  && ((unsigned) ((c)->int_cst.int_cst_low)  < (unsigned) (( ((type)->type.sep) )->int_cst.int_cst_low) ))) );
  else
    return (!(((((type)->type.max) )->int_cst.int_cst_high)  < (( c)->int_cst.int_cst_high) 	|| (((((type)->type.max) )->int_cst.int_cst_high)  == (( c)->int_cst.int_cst_high) 	&& ((unsigned) ((((type)->type.max) )->int_cst.int_cst_low)  < (unsigned) (( c)->int_cst.int_cst_low) ))) 
	    && !(((c)->int_cst.int_cst_high)  < (( ((type)->type.sep) )->int_cst.int_cst_high) 	|| (((c)->int_cst.int_cst_high)  == (( ((type)->type.sep) )->int_cst.int_cst_high) 	&& ((unsigned) ((c)->int_cst.int_cst_low)  < (unsigned) (( ((type)->type.sep) )->int_cst.int_cst_low) ))) );
}

