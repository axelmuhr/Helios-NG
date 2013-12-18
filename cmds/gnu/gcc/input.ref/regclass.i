
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

typedef long HARD_REG_SET[((56  + 32  - 1) / 32 ) ];

extern char fixed_regs[56 ];

extern HARD_REG_SET fixed_reg_set;

extern char call_used_regs[56 ];

extern HARD_REG_SET call_used_reg_set;

extern char call_fixed_regs[56 ];

extern HARD_REG_SET call_fixed_reg_set;

extern char global_regs[56 ];

extern int reg_alloc_order[56 ];

extern HARD_REG_SET reg_class_contents[];

extern int reg_class_size[(int) LIM_REG_CLASSES ];

extern enum reg_class reg_class_superclasses[(int) LIM_REG_CLASSES ][(int) LIM_REG_CLASSES ];

extern enum reg_class reg_class_subclasses[(int) LIM_REG_CLASSES ][(int) LIM_REG_CLASSES ];

extern enum reg_class reg_class_subunion[(int) LIM_REG_CLASSES ][(int) LIM_REG_CLASSES ];

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

typedef long *regset;

extern int regset_bytes;
extern int regset_size;

extern int n_basic_blocks;

extern rtx *basic_block_head;

extern rtx *basic_block_end;

extern regset *basic_block_live_at_start;

extern short *reg_basic_block;

extern int max_regno;

extern short *reg_n_refs;

extern short *reg_n_sets;

extern short *reg_n_deaths;

extern int *reg_n_calls_crossed;

extern int *reg_live_length;

extern short *reg_renumber;

extern char regs_ever_live[56 ];

extern char *reg_names[56 ];

extern short *regno_first_uid;

extern short *regno_last_uid;

extern char *regno_pointer_flag;

extern rtx *regno_reg_rtx;

extern int caller_save_needed;

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

char fixed_regs[56 ];

HARD_REG_SET fixed_reg_set;

static char initial_fixed_regs[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, } ;

char call_used_regs[56 ];

HARD_REG_SET call_used_reg_set;

static char initial_call_used_regs[] = {1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, } ;

char call_fixed_regs[56 ];

HARD_REG_SET call_fixed_reg_set;

char global_regs[56 ];

HARD_REG_SET reg_class_contents[] = {	{0, 0},	{0xff000000, 0x000000ff},	{0xff000000, 0x00ffffff},	{0x00ff0000, 0x00000000},	{0xffff0000, 0x00ffffff},	{0x000000ff, 0x00000000},	{0xff0000ff, 0x00ffffff},	{0x00ff00ff, 0x00000000},	{0xffff00ff, 0x00ffffff},	{0x0000ff00, 0x00000000},	{0x0000ffff, 0x00000000},	{0xff00ffff, 0x00ffffff},	{0x00ffffff, 0x00000000},	{0xffffffff, 0x00ffffff},	} ;

int reg_class_size[(int) LIM_REG_CLASSES ];

enum reg_class reg_class_superclasses[(int) LIM_REG_CLASSES ][(int) LIM_REG_CLASSES ];

enum reg_class reg_class_subclasses[(int) LIM_REG_CLASSES ][(int) LIM_REG_CLASSES ];

enum reg_class reg_class_subunion[(int) LIM_REG_CLASSES ][(int) LIM_REG_CLASSES ];

char *reg_names[] = {"d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",	"a0", "a1", "a2", "a3", "a4", "a5", "a6", "sp",	"fp0", "fp1", "fp2", "fp3", "fp4", "fp5", "fp6", "fp7", "fpa0", "fpa1", "fpa2", "fpa3", "fpa4", "fpa5", "fpa6", "fpa7", "fpa8", "fpa9", "fpa10", "fpa11", "fpa12", "fpa13", "fpa14", "fpa15", "fpa16", "fpa17", "fpa18", "fpa19", "fpa20", "fpa21", "fpa22", "fpa23", "fpa24", "fpa25", "fpa26", "fpa27", "fpa28", "fpa29", "fpa30", "fpa31", } ;

void
init_reg_sets ()
{
  register int i, j;

  memcpy ( fixed_regs,initial_fixed_regs, sizeof fixed_regs) ;
  memcpy ( call_used_regs,initial_call_used_regs, sizeof call_used_regs) ;
  memset (global_regs,0, sizeof global_regs) ;

  memset (reg_class_size,0, sizeof reg_class_size) ;
  for (i = 0; i < (int) LIM_REG_CLASSES ; i++)
    for (j = 0; j < 56 ; j++)
      if (((reg_class_contents[i])[( j) / 32 ] & (1 << (( j) % 32 ))) )
	reg_class_size[i]++;

  for (i = 0; i < (int) LIM_REG_CLASSES ; i++)
    {
      for (j = 0; j < (int) LIM_REG_CLASSES ; j++)
	{

	    HARD_REG_SET c;
	  register int k;

	  do { register long *scan_tp_ = (c), *scan_fp_ = ( reg_class_contents[i]);	register int i;	for (i = 0; i < ((56  + 32  - 1) / 32 ) ; i++)	*scan_tp_++ = *scan_fp_++; } while (0) ;
	  do { register long *scan_tp_ = (c), *scan_fp_ = ( reg_class_contents[j]);	register int i;	for (i = 0; i < ((56  + 32  - 1) / 32 ) ; i++)	*scan_tp_++ |= *scan_fp_++; } while (0) ;
	  for (k = 0; k < (int) LIM_REG_CLASSES ; k++)
	    {
	      do { register long *scan_xp_ = (reg_class_contents[k]), *scan_yp_ = ( c);	register int i;	for (i = 0; i < ((56  + 32  - 1) / 32 ) ; i++)	if (0 != (*scan_xp_++ & ~*scan_yp_++)) break;	if (i == ((56  + 32  - 1) / 32 ) ) goto 
				     subclass1; } while (0) ;
	      continue;

	    subclass1:
	      reg_class_subunion[i][j] = (enum reg_class) k;
	    }
	}
    }

  for (i = 0; i < (int) LIM_REG_CLASSES ; i++)
    {
      for (j = 0; j < (int) LIM_REG_CLASSES ; j++)
	{
	  reg_class_superclasses[i][j] = LIM_REG_CLASSES;
	  reg_class_subclasses[i][j] = LIM_REG_CLASSES;
	}
    }

  for (i = 0; i < (int) LIM_REG_CLASSES ; i++)
    {
      if (i == (int) NO_REGS)
	continue;

      for (j = i + 1; j < (int) LIM_REG_CLASSES ; j++)
	{
	  enum reg_class *p;

	  do { register long *scan_xp_ = (reg_class_contents[i]), *scan_yp_ = ( reg_class_contents[j]);	register int i;	for (i = 0; i < ((56  + 32  - 1) / 32 ) ; i++)	if (0 != (*scan_xp_++ & ~*scan_yp_++)) break;	if (i == ((56  + 32  - 1) / 32 ) ) goto 
				 subclass; } while (0) ;
	  continue;
	subclass:

	  p = &reg_class_superclasses[i][0];
	  while (*p != LIM_REG_CLASSES) p++;
	  *p = (enum reg_class) j;
	  p = &reg_class_subclasses[j][0];
	  while (*p != LIM_REG_CLASSES) p++;
	  *p = (enum reg_class) i;
	}
    }

}

void
init_reg_sets_1 ()
{
  register int i;

  { int i; HARD_REG_SET x; if (!(target_flags & 0100) )	{ do { register long *scan_tp_ = (x), *scan_fp_ = ( reg_class_contents[(int)FPA_REGS]);	register int i;	for (i = 0; i < ((56  + 32  - 1) / 32 ) ; i++)	*scan_tp_++ = *scan_fp_++; } while (0) ; for (i = 0; i < 56 ; i++ ) if (((x)[( i) / 32 ] & (1 << (( i) % 32 ))) ) fixed_regs[i] = call_used_regs[i] = 1; } if ((target_flags & 0100) )	{ do { register long *scan_tp_ = (x), *scan_fp_ = ( reg_class_contents[(int)FP_REGS]);	register int i;	for (i = 0; i < ((56  + 32  - 1) / 32 ) ; i++)	*scan_tp_++ = *scan_fp_++; } while (0) ; for (i = 0; i < 56 ; i++ ) if (((x)[( i) / 32 ] & (1 << (( i) % 32 ))) ) fixed_regs[i] = call_used_regs[i] = 1; } } ;

  for (i = 0; i < 56 ; i++)
    if (global_regs[i])
      {
	if (call_used_regs[i] && ! fixed_regs[i])
	  warning ("call-clobbered register used for global register variable");
	fixed_regs[i] = 1;
	call_used_regs[i] = 1;
      }

  do { register long *scan_tp_ = (fixed_reg_set);	register int i;	for (i = 0; i < ((56  + 32  - 1) / 32 ) ; i++)	*scan_tp_++ = 0; } while (0) ;
  do { register long *scan_tp_ = (call_used_reg_set);	register int i;	for (i = 0; i < ((56  + 32  - 1) / 32 ) ; i++)	*scan_tp_++ = 0; } while (0) ;
  do { register long *scan_tp_ = (call_fixed_reg_set);	register int i;	for (i = 0; i < ((56  + 32  - 1) / 32 ) ; i++)	*scan_tp_++ = 0; } while (0) ;

  memcpy ( call_fixed_regs,fixed_regs, sizeof call_fixed_regs) ;

  call_fixed_regs[9 ] = 1;

  call_fixed_regs[8 ] = 1;

  for (i = 0; i < 56 ; i++)
    {
      if (((i) == 0) )
	call_fixed_regs[i] = 1;
      if (fixed_regs[i])
	((fixed_reg_set)[( i) / 32 ] |= 1 << (( i) % 32 )) ;
      if (call_used_regs[i])
	((call_used_reg_set)[( i) / 32 ] |= 1 << (( i) % 32 )) ;
      if (call_fixed_regs[i])
	((call_fixed_reg_set)[( i) / 32 ] |= 1 << (( i) % 32 )) ;
    }
}

void
fix_register (name, fixed, call_used)
     char *name;
     int fixed, call_used;
{
  int i;

  for (i = 0; i < 56 ; i++)
    if (!strcmp (reg_names[i], name))
      {
	fixed_regs[i] = fixed;
	call_used_regs[i] = call_used;
	break;
      }

  if (i == 56 )
    {
      warning ("unknown register name: %s", name);
      return;
    }
}

struct savings
{
  short savings[(int) LIM_REG_CLASSES ];
  short memcost;
  short nrefs;
};

static struct savings *savings;

static char *prefclass;

static char *preferred_or_nothing;

void reg_class_record ();
void record_address_regs ();

enum reg_class
reg_preferred_class (regno)
     int regno;
{
  if (prefclass == 0)
    return GENERAL_REGS;
  return (enum reg_class) prefclass[regno];
}

int
reg_preferred_or_nothing (regno)
{
  if (prefclass == 0)
    return 0;
  return preferred_or_nothing[regno];
}

int
regclass_init ()
{
  prefclass = 0;
}

void
regclass (f, nregs)
     rtx f;
     int nregs;
{

  register rtx insn;
  register int i;

  init_recog ();

  savings = (struct savings *) __builtin_alloca  (nregs * sizeof (struct savings));
  memset (savings,0, nregs * sizeof (struct savings)) ;

  for (insn = f; insn; insn = ((insn)->fld[2].rtx) )
    if ((	((insn)->code)  == INSN
	 && 	((((insn)->fld[3].rtx) )->code)  != USE
	 && 	((((insn)->fld[3].rtx) )->code)  != CLOBBER
	 && 	((((insn)->fld[3].rtx) )->code)  != ASM_INPUT)
	|| (	((insn)->code)  == JUMP_INSN
	    && 	((((insn)->fld[3].rtx) )->code)  != ADDR_VEC
	    && 	((((insn)->fld[3].rtx) )->code)  != ADDR_DIFF_VEC)
	|| 	((insn)->code)  == CALL_INSN)
      {
	if (	((insn)->code)  == INSN && asm_noperands (((insn)->fld[3].rtx) ) >= 0)
	  {
	    int noperands = asm_noperands (((insn)->fld[3].rtx) );

	    rtx *operands = (rtx *) oballoc (noperands * sizeof (rtx));
	    char **constraints
	      = (char **) oballoc (noperands * sizeof (char *));

	    decode_asm_operands (((insn)->fld[3].rtx) , operands, 0, constraints, 0);

	    for (i = noperands - 1; i >= 0; i--)
	      reg_class_record (operands[i], i, constraints);

	    obfree (operands);
	  }
	else
	  {
	    int insn_code_number = recog_memoized (insn);

	    insn_extract (insn);

	    for (i = insn_n_operands[insn_code_number] - 1; i >= 0; i--)
	      reg_class_record (recog_operand[i], i,
				insn_operand_constraint[insn_code_number]);

	    if (optimize
		&& insn_n_operands[insn_code_number] >= 3
		&& insn_operand_constraint[insn_code_number][1][0] == '0'
		&& insn_operand_constraint[insn_code_number][1][1] == 0
		&& (	((recog_operand[1])->code)  == LABEL_REF || 	((recog_operand[1])->code)  == SYMBOL_REF	|| 	((recog_operand[1])->code)  == CONST_INT	|| 	((recog_operand[1])->code)  == CONST) 
		&& ! rtx_equal_p (recog_operand[0], recog_operand[1])
		&& ! rtx_equal_p (recog_operand[0], recog_operand[2])
		&& 	((recog_operand[0])->code)  == REG)
	      {
		rtx previnsn = prev_real_insn (insn);
		rtx newinsn
		  = emit_insn_before (gen_move_insn (recog_operand[0],
						     recog_operand[1]),
				      insn);

		if (previnsn == 0 || 	((previnsn)->code)  == JUMP_INSN)
		  {
		    int b;
		    for (b = 0; b < n_basic_blocks; b++)
		      if (insn == basic_block_head[b])
			basic_block_head[b] = newinsn;
		  }

		reg_n_sets[((recog_operand[0])->fld[0].rtint) ]++;

		*recog_operand_loc[1] = recog_operand[0];
		for (i = insn_n_dups[insn_code_number] - 1; i >= 0; i--)
		  if (recog_dup_num[i] == 1)
		    *recog_dup_loc[i] = recog_operand[0];

	      }
	  }
      }

  prefclass = (char *) oballoc (nregs);
  preferred_or_nothing = (char *) oballoc (nregs);

  for (i = 56 ; i < nregs; i++)
    {
      register int best_savings = 0;
      enum reg_class best = ALL_REGS;

      register int class;
      register struct savings *p = &savings[i];

      for (class = (int) ALL_REGS - 1; class > 0; class--)
	{
	  if (p->savings[class] > best_savings)
	    {
	      best_savings = p->savings[class];
	      best = (enum reg_class) class;
	    }
	  else if (p->savings[class] == best_savings)
	    {
	      best = reg_class_subunion[(int)best][class];
	    }
	}

      prefclass[i] = (int) best;

      if (reg_n_refs != 0)
	preferred_or_nothing[i]
	  = ((best_savings - p->savings[(int) GENERAL_REGS])
	     >= p->nrefs + p->memcost);
    }

}

void
reg_class_record (op, opno, constraints)
     rtx op;
     int opno;
     char **constraints;
{
  char *constraint = constraints[opno];
  register char *p;
  register enum reg_class class = NO_REGS;
  char *next = 0;
  int memok = 0;
  int double_cost = 0;

  while (1)
    {
      if (	((op)->code)  == SUBREG)
	op = ((op)->fld[0].rtx) ;
      else break;
    }

  if (	((op)->code)  == MEM)
    record_address_regs (((op)->fld[ 0].rtx) , 2, 0);

  if (	((op)->code)  != REG)
    {

      if (constraint != 0 && constraint[0] == 'p')
	record_address_regs (op, 2, 0);
      return;
    }

  for (p = constraint; *p || next; p++)
    {
      if (*p == 0)
	{
	  p = next;
	  next = 0;
	}
      switch (*p)
	{
	case '=':
	case '?':
	case '#':
	case '&':
	case '!':
	case '%':
	case 'F':
	case 'G':
	case 'H':
	case 'i':
	case 'n':
	case 's':
	case 'p':
	case ',':
	  break;

	case '+':
	  double_cost = 1;
	  break;

	case 'm':
	case 'o':
	  memok = 1;
	  break;

	case '*':
	  p++;
	  break;

	case 'g':
	case 'r':
	  class
	    = reg_class_subunion[(int) class][(int) GENERAL_REGS];
	  break;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':

	  next = constraints[*p - '0'];
	  break;

	default:
	  class
	    = reg_class_subunion[(int) class][(int) ((*p) == 'a' ? ADDR_REGS :	((*p) == 'd' ? DATA_REGS :	((*p) == 'f' ? ((target_flags & 2)  ? FP_REGS :	NO_REGS) :	((*p) == 'x' ? ((target_flags & 0100)  ? FPA_REGS :	NO_REGS) :	((*p) == 'y' ? ((target_flags & 0100)  ? LO_FPA_REGS :	NO_REGS) :	NO_REGS))))) ];
	}
    }

  {
    register int i;
    register struct savings *pp;
    register enum reg_class class1;
    int cost = 2 * (1 + double_cost);
    pp = &savings[((op)->fld[0].rtint) ];

    if (class != NO_REGS && class != ALL_REGS)
      {
	pp->savings[(int) class] += cost;
	for (i = 0; ; i++)
	  {
	    class1 = reg_class_subclasses[(int)class][i];
	    if (class1 == LIM_REG_CLASSES)
	      break;
	    pp->savings[(int) class1] += cost;
	  }
      }

    if (! memok)
      pp->memcost += 1 + 2 * double_cost;
    pp->nrefs++;
  }
}

void
record_address_regs (x, bcost, icost)
     rtx x;
     int bcost, icost;
{
  register enum rtx_code code = 	((x)->code) ;

  switch (code)
    {
    case CONST_INT:
    case CONST:
    case CC0:
    case PC:
    case SYMBOL_REF:
    case LABEL_REF:
      return;

    case PLUS:

      {
	rtx arg0 = ((x)->fld[ 0].rtx) ;
	rtx arg1 = ((x)->fld[ 1].rtx) ;
	register enum rtx_code code0 = 	((arg0)->code) ;
	register enum rtx_code code1 = 	((arg1)->code) ;
	int icost0 = 0;
	int icost1 = 0;
	int suppress1 = 0;
	int suppress0 = 0;

	while (code0 == SUBREG)
	  arg0 = ((arg0)->fld[0].rtx) , code0 = 	((arg0)->code) ;
	while (code1 == SUBREG)
	  arg1 = ((arg1)->fld[0].rtx) , code1 = 	((arg1)->code) ;

	if (code0 == MULT || code1 == MEM)
	  icost0 = 2;
	else if (code1 == MULT || code0 == MEM)
	  icost1 = 2;
	else if (code0 == CONST_INT)
	  suppress0 = 1;
	else if (code1 == CONST_INT)
	  suppress1 = 1;
	else if (code0 == REG && code1 == REG)
	  {
	    if (regno_pointer_flag[((arg0)->fld[0].rtint) ] )
	      icost1 = 2;
	    else if (regno_pointer_flag[((arg1)->fld[0].rtint) ] )
	      icost0 = 2;
	    else
	      icost0 = icost1 = 1;
	  }
	else if (code0 == REG)
	  {
	    if (code1 == PLUS
		&& ! regno_pointer_flag[((arg0)->fld[0].rtint) ] )
	      icost0 = 2;
	    else
	      regno_pointer_flag[((arg0)->fld[0].rtint) ]  = 1;
	  }
	else if (code1 == REG)
	  {
	    if (code0 == PLUS
		&& ! regno_pointer_flag[((arg1)->fld[0].rtint) ] )
	      icost1 = 2;
	    else
	      regno_pointer_flag[((arg1)->fld[0].rtint) ]  = 1;
	  }

	if (! suppress0)
	  record_address_regs (arg0, 2 - icost0, icost0);
	if (! suppress1)
	  record_address_regs (arg1, 2 - icost1, icost1);
      }
      break;

    case POST_INC:
    case PRE_INC:
    case POST_DEC:
    case PRE_DEC:

      record_address_regs (((x)->fld[ 0].rtx) , 2 * bcost, 2 * icost);
      break;

    case REG:
      {
	register struct savings *pp;
	register enum reg_class class, class1;
	pp = &savings[((x)->fld[0].rtint) ];
	pp->nrefs++;

	class = ADDR_REGS ;
	if (class != NO_REGS && class != ALL_REGS)
	  {
	    register int i;
	    pp->savings[(int) class] += bcost;
	    for (i = 0; ; i++)
	      {
		class1 = reg_class_subclasses[(int)class][i];
		if (class1 == LIM_REG_CLASSES)
		  break;
		pp->savings[(int) class1] += bcost;
	      }
	  }

	class = GENERAL_REGS ;
	if (icost != 0 && class != NO_REGS && class != ALL_REGS)
	  {
	    register int i;
	    pp->savings[(int) class] += icost;
	    for (i = 0; ; i++)
	      {
		class1 = reg_class_subclasses[(int)class][i];
		if (class1 == LIM_REG_CLASSES)
		  break;
		pp->savings[(int) class1] += icost;
	      }
	  }
      }
      break;

    default:
      {
	register char *fmt = 	(rtx_format[(int)(code)]) ;
	register int i;
	for (i = 	(rtx_length[(int)(code)])  - 1; i >= 0; i--)
	  if (fmt[i] == 'e')
	    record_address_regs (((x)->fld[ i].rtx) , bcost, icost);
      }
    }
}

short *regno_first_uid;

short *regno_last_uid;

int max_parallel;

void reg_scan_mark_refs ();

void
reg_scan (f, nregs, repeat)
     rtx f;
     int nregs;
     int repeat;
{
  register rtx insn;

  if (!repeat)
    regno_first_uid = (short *) oballoc (nregs * sizeof (short));
  memset (regno_first_uid,0, nregs * sizeof (short)) ;

  if (!repeat)
    regno_last_uid = (short *) oballoc (nregs * sizeof (short));
  memset (regno_last_uid,0, nregs * sizeof (short)) ;

  max_parallel = 3;

  for (insn = f; insn; insn = ((insn)->fld[2].rtx) )
    if (	((insn)->code)  == INSN
	|| 	((insn)->code)  == CALL_INSN
	|| 	((insn)->code)  == JUMP_INSN)
      {
	if (	((((insn)->fld[3].rtx) )->code)  == PARALLEL
	    && ((((insn)->fld[3].rtx) )->fld[ 0].rtvec->num_elem)  > max_parallel)
	  max_parallel = ((((insn)->fld[3].rtx) )->fld[ 0].rtvec->num_elem) ;
	reg_scan_mark_refs (((insn)->fld[3].rtx) , ((insn)->fld[0].rtint) );
      }
}

void
reg_scan_mark_refs (x, uid)
     rtx x;
     int uid;
{
  register enum rtx_code code = 	((x)->code) ;

  switch (code)
    {
    case CONST_INT:
    case CONST:
    case CONST_DOUBLE:
    case CC0:
    case PC:
    case SYMBOL_REF:
    case LABEL_REF:
    case ADDR_VEC:
    case ADDR_DIFF_VEC:
      return;

    case REG:
      {
	register int regno = ((x)->fld[0].rtint) ;

	regno_last_uid[regno] = uid;
	if (regno_first_uid[regno] == 0)
	  regno_first_uid[regno] = uid;
      }
      break;

    default:
      {
	register char *fmt = 	(rtx_format[(int)(code)]) ;
	register int i;
	for (i = 	(rtx_length[(int)(code)])  - 1; i >= 0; i--)
	  {
	    if (fmt[i] == 'e')
	      reg_scan_mark_refs (((x)->fld[ i].rtx) , uid);
	    else if (fmt[i] == 'E' && ((x)->fld[ i].rtvec)  != 0)
	      {
		register int j;
		for (j = ((x)->fld[ i].rtvec->num_elem)  - 1; j >= 0; j--)
		  reg_scan_mark_refs (((x)->fld[ i].rtvec->elem[ j].rtx) , uid);		  
	      }
	  }
      }
    }
}

