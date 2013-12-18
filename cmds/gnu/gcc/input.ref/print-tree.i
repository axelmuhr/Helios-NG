
extern int target_flags;

enum reg_class { NO_REGS, LO_FPA_REGS, FPA_REGS, FP_REGS,
  FP_OR_FPA_REGS, DATA_REGS, DATA_OR_FPA_REGS, DATA_OR_FP_REGS,
  DATA_OR_FP_OR_FPA_REGS, ADDR_REGS, GENERAL_REGS,
  GENERAL_OR_FPA_REGS, GENERAL_OR_FP_REGS, ALL_REGS,
  LIM_REG_CLASSES };

extern enum reg_class regno_reg_class[];

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

char *tree_code_name[] = {

 "error_mark", 

 "identifier_node", 

 "op_identifier", 

 "tree_list", 

 "void_type", 	 

 "integer_type", 

 "real_type", 

 "complex_type", 

 "enumeral_type", 

 "boolean_type", 

 "char_type", 

 "pointer_type", 

 "offset_type", 

 "reference_type", 

 "method_type", 

 "file_type", 

 "array_type", 

 "set_type", 

 "string_type", 

 "record_type", 

 "union_type", 	 

 "function_type", 

 "lang_type", 

 "label_stmt", 

 "goto_stmt", 

 "return_stmt", 

 "expr_stmt", 

 "with_stmt", 

 "let_stmt", 

 "if_stmt", 

 "exit_stmt", 

 "case_stmt", 

 "loop_stmt", 

 "compound_stmt", 

 "asm_stmt", 

 "integer_cst", 

 "real_cst", 

 "complex_cst", 

 "string_cst", 

 "function_decl", 
 "label_decl", 
 "const_decl", 
 "type_decl", 
 "var_decl", 
 "parm_decl", 
 "result_decl", 
 "field_decl", 
 "friend_decl", 

 "component_ref", 

 "indirect_ref", 

 "offset_ref", 

 "buffer_ref", 

 "array_ref", 

 "constructor", 

 "compound_expr", 

 "modify_expr", 

 "init_expr", 

 "new_expr", 

 "delete_expr", 

 "cond_expr", 

 "call_expr", 

 "method_call_expr", 

 "with_call_expr", 

 "plus_expr", 
 "minus_expr", 
 "mult_expr", 

 "trunc_div_expr", 

 "ceil_div_expr", 

 "floor_div_expr", 

 "round_div_expr", 

 "trunc_mod_expr", 
 "ceil_mod_expr", 
 "floor_mod_expr", 
 "round_mod_expr", 

 "rdiv_expr", 

 "fix_trunc_expr", 
 "fix_ceil_expr", 
 "fix_floor_expr", 
 "fix_round_expr", 

 "float_expr", 

 "expon_expr", 

 "negate_expr", 

 "min_expr", 
 "max_expr", 
 "abs_expr", 
 "ffs_expr", 

 "alshift_expr", 
 "arshift_expr", 
 "lrotate_expr", 
 "rrotate_expr", 

 "bit_ior_expr", 
 "bit_xor_expr", 
 "bit_and_expr", 
 "bit_andtc_expr", 
 "bit_not_expr", 

 "truth_andif_expr", 
 "truth_orif_expr", 
 "truth_and_expr", 
 "truth_or_expr", 
 "truth_not_expr", 

 "lt_expr", 
 "le_expr", 
 "gt_expr", 
 "ge_expr", 
 "eq_expr", 
 "ne_expr", 

 "in_expr", 
 "set_le_expr", 
 "card_expr", 
 "range_expr", 

 "convert_expr", 

 "nop_expr", 

 "save_expr", 

 "rtl_expr", 

 "addr_expr", 

 "reference_expr", 

 "wrapper_expr", 
 "anti_wrapper_expr", 

 "entry_value_expr", 

 "complex_expr", 

 "conj_expr", 

 "realpart_expr", 
 "imagpart_expr", 

 "predecrement_expr", 
 "preincrement_expr", 
 "postdecrement_expr", 
 "postincrement_expr", 

};

extern char *tree_code_type[];
extern int tree_code_length[];
extern char *mode_name[];

extern char spaces[];

static struct _iobuf  *outfile;

extern int tree_node_counter;

static char *markvec;

static void dump ();
void dump_tree ();

void
debug_dump_tree (root)
     tree root;
{
  dump_tree ((&_iob[2]) , root);
}

void
dump_tree (outf, root)
     struct _iobuf  *outf;
     tree root;
{
  markvec = (char *) __builtin_alloca  (tree_node_counter + 1);
  memset (markvec,0, tree_node_counter + 1) ;
  outfile = outf;
  dump (root, 0);
  fflush (outf);
}
static
void
wruid (node)
     tree node;
{
  if (node == 0 )
    fputs ("<>", outfile);
  else {
    fprintf (outfile, "%1d", ((node)->common.uid) );
  }
}

static 
void
part (title, node)
     char title[];
     tree node;
{
  fprintf (outfile, " %s = ", title);
  wruid (node);
  (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(';')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(';')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(';'),  outfile))) ;
}

static
void
cpart (title, ct, punct)
     char *title;
     tree ct;
     char punct;
{
  fprintf (outfile, " %s = ", title);
  if (ct == 0 )
    fputs ("<>", outfile);
  else
    {
      if (!((ct)->common.literal_attr) )
	{
	  (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)('@')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)('@')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)('@'),  outfile))) ;
	  wruid (ct);
	}
      else
	fprintf (outfile, "%ld", ((ct)->int_cst.int_cst_low) );
    }
  (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(punct)) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(punct)) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(punct),  outfile))) ;
}

static
void
walk (node, leaf, indent)
     tree node;
     tree leaf;
     int indent;
{
  if (node != 0 )
    dump (node, indent+1);
}

static void
cwalk (s, leaf, indent)
     tree s;
     tree leaf;
     int indent;
{
  if (s != 0 ) 
    if (!((s)->common.literal_attr) )
      walk (s, leaf, indent);
}
static void
prtypeinfo (node)
     register tree node;
{
  int first;
  part ("type", ((node)->common.type) );
  first = 1;
  fputs (" [", outfile);
  if (((node)->common.external_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("external", outfile);
      first = 0;
    }
  if (((node)->common.public_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("public", outfile);
      first = 0;
    }
  if (((node)->common.static_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("static", outfile);
      first = 0;
    }
  if (((node)->common.volatile_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("volatile", outfile);
      first = 0;
    }
  if (((node)->common.packed_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("packed", outfile);
      first = 0;
    }
  if (((node)->common.readonly_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("readonly", outfile);
      first = 0;
    }
  if (((node)->common.literal_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("literal", outfile);
      first = 0;
    }
  if (((node)->common.nonlocal_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("nonlocal", outfile);
      first = 0;
    }
  if (((node)->common.addressable_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("addressable", outfile);
      first = 0;
    }
  if (((node)->common.regdecl_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("regdecl", outfile);
      first = 0;
    }
  if (((node)->common.this_vol_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("this_vol", outfile);
      first = 0;
    }
  if (((node)->common.unsigned_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("unsigned", outfile);
      first = 0;
    }
  if (((node)->common.asm_written_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("asm_written", outfile);
      first = 0;
    }
  if (((node)->common.inline_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("inline", outfile);
      first = 0;
    }
  if (((node)->common.used_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("used", outfile);
      first = 0;
    }
  if (((node)->common.permanent_attr) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("permanent", outfile);
      first = 0;
    }
  if (((node)->common.lang_flag_1) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("lang_flag_1", outfile);
      first = 0;
    }
  if (((node)->common.lang_flag_2) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("lang_flag_2", outfile);
      first = 0;
    }
  if (((node)->common.lang_flag_3) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("lang_flag_3", outfile);
      first = 0;
    }
  if (((node)->common.lang_flag_4) )
    {
      if (!first) (--( outfile)->_cnt >= 0 ?	(int)(*( outfile)->_ptr++ = (unsigned char)(' ')) :	((( outfile)->_flag & 0200 ) && -( outfile)->_cnt < ( outfile)->_bufsiz ?	((*( outfile)->_ptr = (unsigned char)(' ')) != '\n' ?	(int)(*( outfile)->_ptr++) :	_flsbuf(*(unsigned char *)( outfile)->_ptr,  outfile)) :	_flsbuf((unsigned char)(' '),  outfile))) ;
      fputs ("lang_flag_4", outfile);
      first = 0;
    }
  fputs ("] ", outfile);
}

static void
prdeclmodeinfo (node)
     tree node;
{
  register enum machine_mode mode = ((node)->decl.mode) ;
  fprintf (outfile, " %s;", mode_name[(int) mode]);

  cpart ("size", ((node)->decl.size) , '*');
  fprintf (outfile, "%d;", ((node)->decl.size_unit) );

  fprintf (outfile, " alignment = %1d;", ((node)->decl.align) );
}

static
void
prtypemodeinfo (node)
     tree node;
{
  register enum machine_mode mode = ((node)->type.mode) ;
  fprintf (outfile, " %s;", mode_name[(int) mode]);

  cpart ("size", ((node)->type.size) , '*');
  fprintf (outfile, "%d;", ((node)->type.size_unit) );

  fprintf (outfile, " alignment = %1d;", ((node)->type.align) );
}

static
void
skip (indent)
     int indent;
{
  (--(outfile)->_cnt >= 0 ?	(int)(*(outfile)->_ptr++ = (unsigned char)('\n')) :	(((outfile)->_flag & 0200 ) && -(outfile)->_cnt < (outfile)->_bufsiz ?	((*(outfile)->_ptr = (unsigned char)('\n')) != '\n' ?	(int)(*(outfile)->_ptr++) :	_flsbuf(*(unsigned char *)(outfile)->_ptr, outfile)) :	_flsbuf((unsigned char)('\n'), outfile))) ;
  fputs (spaces + (strlen (spaces) - (12 + ((40 < (indent+1)*2) ? 40 : (indent+1)*2) )), outfile);
}

static 
void
dump (node, indent)
     tree node;
     int indent;
{
  register enum tree_code code = ((node)->common.code) ;
  register int i;
  register int len, first_rtl;
  int nochain = 0;

  if (markvec[((node)->common.uid) ])
    return;
  markvec[((node)->common.uid) ] = 1;

  fputs ("   ", outfile);
  fprintf (outfile, "%5d", ((node)->common.uid) );
  fputs (spaces + (strlen (spaces) - ((40 <  (indent+1)*2) ? 40 :  (indent+1)*2) ), outfile);
  fputs (tree_code_name[(int) code], outfile);

  switch (*tree_code_type[(int) code])
    {
    case 'd':
      fputs (" name = ", outfile);
      if (((node)->decl.name)  == 0 )
	fputs ("<>;", outfile);
      else
	fprintf (outfile, "%s;",
		 ((((node)->decl.name) )->identifier.pointer) );
      fprintf (outfile, " at %s line %d;",
	       ((node)->decl.filename) , ((node)->decl.linenum) );
      skip (indent);
      prdeclmodeinfo (node);
      prtypeinfo (node);

      skip (indent);
      fprintf (outfile, " offset = %1d;", ((node)->decl.offset) );
      if (((node)->decl.voffset)   != 0 )
	{
	  fputs ("voffset = ", outfile);
	  wruid (((node)->decl.voffset)  );
	  fprintf (outfile, "*%1d;", ((node)->decl.voffset_unit) );
	}
      part ("context", ((node)->decl.context) );
      if (((node)->decl.arguments)   || ((node)->decl.result) 
	  || ((node)->decl.initial) )
	{
	  skip (indent);
	  part ("arguments", ((node)->decl.arguments)  );
	  part ("result", ((node)->decl.result) );
	  if ((int) (((node)->decl.initial) ) == 1)
	    fprintf (outfile, " initial = const 1;");
	  else
	    part ("initial", ((node)->decl.initial) );
	}

      part ("chain", ((node)->common.chain) );
      nochain = 1;
      fputc ('\n', outfile);
      cwalk (((node)->decl.size) , node, indent);
      walk (((node)->common.type) , node, indent);
      walk (((node)->decl.voffset)  , node, indent);
      walk (((node)->decl.context) , node, indent);
      walk (((node)->decl.arguments)  , node, indent);
      walk (((node)->decl.result) , node, indent);
      if ((int) (((node)->decl.initial) ) != 1)
	walk (((node)->decl.initial) , node, indent);
      break;

    case 't':
      prtypemodeinfo (node);
      prtypeinfo (node);

      skip (indent);
      part ("pointers_to_this", ((node)->type.pointer_to) );
      if (code == ARRAY_TYPE || code == SET_TYPE)
	{
	  part ("domain", ((node)->type.values) );
	  cpart ("separation", ((node)->type.sep) , '*');
	  fprintf (outfile, "%d;", ((node)->type.sep_unit) );
	}
      else if (code == INTEGER_TYPE)
	{
	  cpart ("min", ((node)->type.sep) , ';');
	  cpart ("max", ((node)->type.max) , ';');
	  fprintf (outfile, "precision = %d;", ((node)->type.sep_unit) );
	}
      else if (code == ENUMERAL_TYPE)
	{
	  cpart ("min", ((node)->type.sep) , ';');
	  cpart ("max", ((node)->type.max) , ';');
	  part ("values", ((node)->type.values) );
	  fprintf (outfile, "precision = %d;", ((node)->type.sep_unit) );
	}
      else if (code == REAL_TYPE)
	{
	  fprintf (outfile, "precision = %d;", ((node)->type.sep_unit) );
	}
      else if (code == RECORD_TYPE
	       || code == UNION_TYPE)
	{
	  part ("fields", ((node)->type.values) );
	}
      else if (code == FUNCTION_TYPE)
	{
	  part ("arg_types", ((node)->type.values) );
	}
      else if (code == METHOD_TYPE)
	{
	  part ("arg_types", ((node)->type.values) );
	}

      part ("chain", ((node)->common.chain) );

      nochain = 1;
      fputc ('\n', outfile);
      cwalk (((node)->type.size) , node, indent);
      walk (((node)->common.type) , node, indent);
      walk (((node)->type.values) , node, indent);
      walk (((node)->type.sep) , node, indent);
      walk (((node)->type.pointer_to) , node, indent);
      break;

    case 'e':
    case 'r':
      prtypeinfo (node);
      fputs (" ops =", outfile);
      first_rtl = len = tree_code_length[(int) code];

      switch (code)
	{
	case SAVE_EXPR:
	  first_rtl = 1;
	  break;
	case CALL_EXPR:
	  first_rtl = 2;
	  break;
	case METHOD_CALL_EXPR:
	  first_rtl = 3;
	  break;
	case WITH_CLEANUP_EXPR:
	  first_rtl = 1;
	  break;
	case RTL_EXPR:
	  first_rtl = 0;
	}
      for (i = 0; i < len; i++)
	{
	  if (i >= first_rtl)
	    {
	      skip (indent);
	      print_rtl (outfile, ((node)->exp.operands[ i]) );
	      fprintf (outfile, "\n");
	    }
	  else
	    {
	      fputs (" ", outfile);
	      wruid (((node)->exp.operands[ i]) );
	      fputs (";", outfile);
	    }
	}
      part ("chain", ((node)->common.chain) );
      fputc ('\n', outfile);
      walk (((node)->common.type) , node, indent);
      for (i = 0; i < len && i < first_rtl; i++)
	walk (((node)->exp.operands[ i]) , node, indent);
      break;

    case 's':
      prtypeinfo (node);
      fprintf (outfile, " at %s line %d;",
	       ((node)->stmt.filename) , ((node)->stmt.linenum) );
      switch (((node)->common.code) )
	{
	case IF_STMT:
	  part ("cond", ((node)->if_stmt.cond) );
	  part ("then", ((node)->if_stmt.thenpart) );
	  part ("else", ((node)->if_stmt.elsepart) );
	  break;

	case LET_STMT:
	case WITH_STMT:
	  part ("vars", ((node)->bind_stmt.vars) );
	  part ("tags", ((node)->bind_stmt.type_tags) );
	  part ("supercontext", ((node)->bind_stmt.supercontext) );
	  part ("bind_size", ((node)->bind_stmt.bind_size) );
	  part ("body", ((node)->stmt.body) );
	  break;

	case CASE_STMT:
	  part ("case_index", ((node)->case_stmt.index) );
	  part ("case_list", ((node)->case_stmt.case_list) );
	  break;

	default:
	  part ("body", ((node)->stmt.body) );
	  break;
	}
      part ("chain", ((node)->common.chain) );
      fputc ('\n', outfile);
      walk (((node)->common.type) , node, indent);
      switch (((node)->common.code) )
	{
	case IF_STMT:
	  walk (((node)->if_stmt.cond) , node, indent);
	  walk (((node)->if_stmt.thenpart) , node, indent);
	  walk (((node)->if_stmt.elsepart) , node, indent);
	  break;

	case LET_STMT:
	case WITH_STMT:
	  walk (((node)->bind_stmt.vars) , node, indent);
	  walk (((node)->bind_stmt.type_tags) , node, indent);
	  walk (((node)->bind_stmt.supercontext) , node, indent);
	  walk (((node)->bind_stmt.bind_size) , node, indent);
	  walk (((node)->stmt.body) , node, indent);
	  break;

	case CASE_STMT:
	  walk (((node)->case_stmt.index) , node, indent);
	  walk (((node)->case_stmt.case_list) , node, indent);
	  break;

	default:
	  walk (((node)->stmt.body) , node, indent);
	  break;
	}
      break;

    case 'c':
      switch (code)
	{
	case INTEGER_CST:
	  if (((node)->int_cst.int_cst_high)  == 0)
	    fprintf (outfile, " = %1u;", ((node)->int_cst.int_cst_low) );
	  else if (((node)->int_cst.int_cst_high)  == -1
		   && ((node)->int_cst.int_cst_low)  != 0)
	    fprintf (outfile, " = -%1u;", -((node)->int_cst.int_cst_low) );
	  else
	    fprintf (outfile, " = 0x%x%08x;",
		     ((node)->int_cst.int_cst_high) ,
		     ((node)->int_cst.int_cst_low) );
	  break;

	case REAL_CST:

	  fprintf (outfile, " = %e;", ((node)->real_cst.real_cst) );

	  break;

	case COMPLEX_CST:
	  part ("realpart", ((node)->complex.real) );
	  part ("imagpart", ((node)->complex.imag) );
	  walk (((node)->complex.real) , node, indent);
	  walk (((node)->complex.imag) , node, indent);
	  break;

	case STRING_CST:
	  fprintf (outfile, " = \"%s\";", ((node)->string.pointer) );
	}
      prtypeinfo (node);
      part ("chain", ((node)->common.chain) );
      fputc ('\n', outfile);
      walk (((node)->common.type) , node, indent);
      break;

    case 'x':
      if (code == IDENTIFIER_NODE)
	{
	  fprintf (outfile, " = %s;\n", ((node)->identifier.pointer) );
	  nochain = 1;
	}
      else if (code == TREE_LIST)
	{
	  prtypeinfo (node);
	  part ("purpose", ((node)->list.purpose) );
	  part ("value", ((node)->list.value) );
	  part ("chain", ((node)->common.chain) );
	  fputc ('\n', outfile);
	  walk (((node)->common.type) , node, indent);
	  walk (((node)->list.purpose) , node, indent);
	  walk (((node)->list.value) , node, indent);
	}
      else if (code == OP_IDENTIFIER)
	{
	  prtypeinfo (node);
	  part ("op1", ((node)->list.purpose) );
	  part ("op2", ((node)->list.value) );
	  part ("chain", ((node)->common.chain) );
	  fputc ('\n', outfile);
	  walk (((node)->common.type) , node, indent);
	  walk (((node)->list.purpose) , node, indent);
	  walk (((node)->list.value) , node, indent);
	}
      else if (code == ERROR_MARK)
	fputc ('\n', outfile);
      else abort ();

      break;

    default:
      abort ();
    }  

  if (((node)->common.chain)  != 0  && ! nochain)
    dump (((node)->common.chain) , indent);
}

