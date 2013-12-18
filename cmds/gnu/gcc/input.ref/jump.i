
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

rtx *jump_chain;

rtx delete_insn ();
void redirect_jump ();
void invert_jump ();
rtx next_real_insn ();
rtx prev_real_insn ();
rtx next_label ();

static void mark_jump_label ();
static void delete_jump ();
void invert_exp ();
static void redirect_exp ();
static rtx follow_jumps ();
static int tension_vector_labels ();
static void find_cross_jump ();
static void do_cross_jump ();
static enum rtx_code reverse_condition ();
static int jump_back_p ();
int condjump_p ();

void
jump_optimize (f, cross_jump, noop_moves)
     rtx f;
{
  register rtx insn;
  int changed;
  int first = 1;
  int max_uid = 0;
  rtx last_insn;

  for (insn = f; insn; insn = ((insn)->fld[2].rtx) )
    {
      if (	((insn)->code)  == CODE_LABEL)
	((insn)->fld[4].rtint)  = 0;
      if (	((insn)->code)  == JUMP_INSN)
	  ((insn)->fld[7].rtx)  = 0;
      if (((insn)->fld[0].rtint)  > max_uid)
	max_uid = ((insn)->fld[0].rtint) ;
    }

  max_uid++;

  jump_chain = (rtx *) __builtin_alloca  (max_uid * sizeof (rtx));
  memset (jump_chain,0, max_uid * sizeof (rtx)) ;

  for (insn = f; insn;)
    {
      if (	((insn)->code)  == BARRIER)
	{
	  insn = ((insn)->fld[2].rtx) ;
	  while (insn != 0 && 	((insn)->code)  != CODE_LABEL)
	    {
	      if (	((insn)->code)  == NOTE
		  && ((insn)->fld[4].rtint)  != -6 )
		insn = ((insn)->fld[2].rtx) ;
	      else
		insn = delete_insn (insn);
	    }
	}
      else
	insn = ((insn)->fld[2].rtx) ;
    }

  for (insn = f; insn; insn = ((insn)->fld[2].rtx) )
    if (	((insn)->code)  == JUMP_INSN && ! ((insn)->volatil) )
      {
	mark_jump_label (((insn)->fld[3].rtx) , insn, cross_jump);
	if (  ((insn)->fld[7].rtx)  != 0 && simplejump_p (insn))
	  {
	    jump_chain[((insn)->fld[0].rtint) ]
	      = jump_chain[((  ((insn)->fld[7].rtx) )->fld[0].rtint) ];
	    jump_chain[((  ((insn)->fld[7].rtx) )->fld[0].rtint) ] = insn;
	  }
	if (	((((insn)->fld[3].rtx) )->code)  == RETURN)
	  {
	    jump_chain[((insn)->fld[0].rtint) ] = jump_chain[0];
	    jump_chain[0] = insn;
	  }
      }

  last_insn = 0;
  for (insn = f; insn; )
    {
      if (	((insn)->code)  == CODE_LABEL && ((insn)->fld[4].rtint)  == 0)
	insn = delete_insn (insn);
      else
	{
	  last_insn = insn;
	  insn = ((insn)->fld[2].rtx) ;
	}
    }

  if (!optimize)
    {

      insn = last_insn;
      while (insn && (	((insn)->code)  == CODE_LABEL

		      || (	((insn)->code)  == JUMP_INSN
			  && 	((((insn)->fld[3].rtx) )->code)  == RETURN)
		      || (	((insn)->code)  == NOTE
			  && ((insn)->fld[4].rtint)  != -6 )))
	insn = ((insn)->fld[1].rtx) ;

      if (insn && 	((insn)->code)  == NOTE
	  && ((insn)->fld[4].rtint)  == -6 
	  && ! ((insn)->volatil) )
	{
	  extern int current_function_returns_null;
	  current_function_returns_null = 1;
	}
      for (insn = f; insn; insn = ((insn)->fld[2].rtx) )
	((insn)->volatil)  = 0;
      return;
    }

  if (noop_moves)
    for (insn = f; insn; )
      {
	register rtx next = ((insn)->fld[2].rtx) ;

	if (	((insn)->code)  == INSN)
	  {
	    register rtx body = ((insn)->fld[3].rtx) ;

	    if (	((body)->code)  == USE
		|| 	((body)->code)  == CLOBBER)
	      delete_insn (insn);
	    else

	      if (	((body)->code)  == SET
		     && (((body)->fld[0].rtx)  == ((body)->fld[1].rtx) 
			 || (	((((body)->fld[0].rtx) )->code)  == MEM
			     && 	((((body)->fld[1].rtx) )->code)  == MEM
			     && rtx_equal_p (((body)->fld[1].rtx) , ((body)->fld[0].rtx) )))
		     && ! (	((((body)->fld[0].rtx) )->code)  == MEM
			   && ((((body)->fld[0].rtx) )->volatil) )
		     && ! (	((((body)->fld[1].rtx) )->code)  == MEM
			   && ((((body)->fld[1].rtx) )->volatil) ))
	      delete_insn (insn);

	    else if (	((body)->code)  == SET)
	      {
		int sreg = true_regnum (((body)->fld[1].rtx) );
		int dreg = true_regnum (((body)->fld[0].rtx) );

		if (sreg == dreg && sreg >= 0)
		  delete_insn (insn);
		else if (sreg >= 0 && dreg >= 0)
		  {
		    rtx tem = find_equiv_reg (0, insn, 0,
					      sreg, 0, dreg,
					      	((((body)->fld[1].rtx) )->mode) );

		      if (tem != 0
			  && 	((tem)->mode)  == 	((((body)->fld[0].rtx) )->mode) )
			delete_insn (insn);
		  }
	      }
	  }
      insn = next;
    }

  changed = 1;
  while (changed)
    {
      register rtx next;
      changed = 0;

      for (insn = f; insn; insn = next)
	{

	  next = ((insn)->fld[2].rtx) ;

	  if (	((insn)->code)  == JUMP_INSN)
	    {
	      if (	((((insn)->fld[3].rtx) )->code)  == ADDR_VEC)
		changed |= tension_vector_labels (((insn)->fld[3].rtx) , 0, noop_moves);
	      if (	((((insn)->fld[3].rtx) )->code)  == ADDR_DIFF_VEC)
		changed |= tension_vector_labels (((insn)->fld[3].rtx) , 1, noop_moves);
	    }

	  if (	((insn)->code)  == JUMP_INSN &&   ((insn)->fld[7].rtx) )
	    {
	      register rtx reallabelprev = prev_real_insn (  ((insn)->fld[7].rtx) );
	      rtx temp;

	      if (reallabelprev == insn && condjump_p (insn))
		{
		  delete_jump (insn);
		  changed = 1;
		}
	      else if ((temp = next_real_insn (insn))
		       && 	((temp)->code)  == JUMP_INSN
		       && condjump_p (insn)
		       && simplejump_p (temp)
		       &&   ((insn)->fld[7].rtx)  ==   ((temp)->fld[7].rtx) )
		{
		  delete_jump (insn);
		  changed = 1;
		  next = ((insn)->fld[2].rtx) ;
		}
	      else if (simplejump_p (insn)
		       && (temp = next_real_insn (  ((insn)->fld[7].rtx) )) != 0
		       && 	((((temp)->fld[3].rtx) )->code)  == RETURN)
		{
		  ((insn)->fld[3].rtx)  = ((temp)->fld[3].rtx) ;
		  ((insn)->fld[4].rtint)  = -1;
		}
	      else if (reallabelprev != 0
		       && 	((reallabelprev)->code)  == JUMP_INSN
		       && prev_real_insn (reallabelprev) == insn
		       && no_labels_between_p (insn, reallabelprev)
		       && simplejump_p (reallabelprev)

		       && condjump_p (insn))
		{
		  ++((  ((reallabelprev)->fld[7].rtx) )->fld[4].rtint) ;
		  delete_insn (reallabelprev);

		  invert_jump (insn,   ((reallabelprev)->fld[7].rtx) );
		  --((  ((reallabelprev)->fld[7].rtx) )->fld[4].rtint) ;
		  next = insn;
		  changed = 1;
		}
	      else
		{
		  {
		    register rtx nlabel
		      = follow_jumps (  ((insn)->fld[7].rtx) , noop_moves);
		    if (nlabel !=   ((insn)->fld[7].rtx) )
		      {
			redirect_jump (insn, nlabel);
			changed = 1;
			next = insn;
		      }
		  }

		  {
		    rtx label1 = next_label (insn);
		    rtx range1end = label1 ? prev_real_insn (label1) : 0;

		    if (! first
			&& condjump_p (insn)
			&&   ((insn)->fld[7].rtx)  == label1
			&& ((label1)->fld[4].rtint)  == 1
			&& 	((range1end)->code)  == JUMP_INSN
			&& simplejump_p (range1end))
		      {
			rtx label2 = next_label (label1);
			rtx range2end = label2 ? prev_real_insn (label2) : 0;
			if (range1end != range2end
			    &&   ((range1end)->fld[7].rtx)  == label2
			    && 	((range2end)->code)  == JUMP_INSN
			    && 	((((range2end)->fld[2].rtx) )->code)  == BARRIER)
			  {
			    rtx range1beg = ((insn)->fld[2].rtx) ;
			    rtx range2beg = ((label1)->fld[2].rtx) ;
			    rtx range1after = ((range1end)->fld[2].rtx) ;
			    rtx range2after = ((range2end)->fld[2].rtx) ;
			    ((insn)->fld[2].rtx)  = range2beg;
			    ((range2beg)->fld[1].rtx)  = insn;
			    ((range2end)->fld[2].rtx)  = range1after;
			    ((range1after)->fld[1].rtx)  = range2end;
			    ((label1)->fld[2].rtx)  = range1beg;
			    ((range1beg)->fld[1].rtx)  = label1;
			    ((range1end)->fld[2].rtx)  = range2after;
			    ((range2after)->fld[1].rtx)  = range1end;

			    invert_jump (insn, label1);
			    changed = 1;
			    continue;
			  }
		      }
		  }

		  if (cross_jump && condjump_p (insn))
		    {
		      rtx newjpos, newlpos;
		      rtx x = prev_real_insn (  ((insn)->fld[7].rtx) );

		      if (x != 0 && ! jump_back_p (x, insn))

			x = 0;

		      newjpos = 0;

		      if (x != 0)
			find_cross_jump (insn, x, 2,
					 &newjpos, &newlpos);

		      if (newjpos != 0)
			{
			  do_cross_jump (insn, newjpos, newlpos);

			  ((((insn)->fld[3].rtx) )->fld[1].rtx) 
			    = gen_rtx (LABEL_REF, VOIDmode,   ((insn)->fld[7].rtx) );
			  emit_barrier_after (insn);
			  changed = 1;
			  next = insn;
			}
		    }

		  if (cross_jump && simplejump_p (insn))
		    {
		      rtx newjpos, newlpos;
		      rtx target;

		      newjpos = 0;

		      find_cross_jump (insn,   ((insn)->fld[7].rtx) , 1,
				       &newjpos, &newlpos);

		      if (((  ((insn)->fld[7].rtx) )->fld[0].rtint)  < max_uid)
			for (target = jump_chain[((  ((insn)->fld[7].rtx) )->fld[0].rtint) ];
			     target != 0 && newjpos == 0;
			     target = jump_chain[((target)->fld[0].rtint) ])
			  if (target != insn
			      &&   ((target)->fld[7].rtx)  ==   ((insn)->fld[7].rtx) 
			      && ! ((target)->volatil) )
			    find_cross_jump (insn, target, 2,
					     &newjpos, &newlpos);

		      if (newjpos != 0)
			{
			  do_cross_jump (insn, newjpos, newlpos);
			  changed = 1;
			  next = insn;
			}
		    }
		}
	    }
	  else if (	((insn)->code)  == JUMP_INSN
		   && 	((((insn)->fld[3].rtx) )->code)  == RETURN)
	    {

	      if (cross_jump)
		{
		  rtx newjpos, newlpos, target;

		  newjpos = 0;

		  for (target = jump_chain[0];
		       target != 0 && newjpos == 0;
		       target = jump_chain[((target)->fld[0].rtint) ])
		    if (target != insn
			&& ! ((target)->volatil) 
			&& 	((((target)->fld[3].rtx) )->code)  == RETURN)
		      find_cross_jump (insn, target, 2,
				       &newjpos, &newlpos);

		  if (newjpos != 0)
		    {
		      do_cross_jump (insn, newjpos, newlpos);
		      changed = 1;
		      next = insn;
		    }
		}
	    }

	}

      first = 0;
    }

  insn = last_insn;
  while (insn && (	((insn)->code)  == CODE_LABEL

		  || (	((insn)->code)  == JUMP_INSN
		      && 	((((insn)->fld[3].rtx) )->code)  == RETURN)
		  || (	((insn)->code)  == NOTE
		      && ((insn)->fld[4].rtint)  != -6 )))
    insn = ((insn)->fld[1].rtx) ;
  if (insn && 	((insn)->code)  == NOTE
      && ((insn)->fld[4].rtint)  == -6 )
    {
      extern int current_function_returns_null;
      current_function_returns_null = 1;
      delete_insn (insn);
    }
}

static void
find_cross_jump (e1, e2, minimum, f1, f2)
     rtx e1, e2;
     int minimum;
     rtx *f1, *f2;
{
  register rtx i1 = e1, i2 = e2;
  register rtx p1, p2;

  rtx last1 = 0, last2 = 0;
  rtx afterlast1 = 0, afterlast2 = 0;

  *f1 = 0;
  *f2 = 0;

  while (1)
    {
      i1 = ((i1)->fld[1].rtx) ;
      while (i1 && 	((i1)->code)  == NOTE)
	i1 = ((i1)->fld[1].rtx) ;

      i2 = ((i2)->fld[1].rtx) ;
      while (i2 && (	((i2)->code)  == NOTE || 	((i2)->code)  == CODE_LABEL))
	i2 = ((i2)->fld[1].rtx) ;

      if (i1 == 0)
	break;

      if (i2 == e1 || i1 == e2)
	break;

      if (	((i1)->code)  == CODE_LABEL)
	{
	  --minimum;
	  break;
	}

      if (i2 == 0 || 	((i1)->code)  != 	((i2)->code) )
	break;

      p1 = ((i1)->fld[3].rtx) ;
      p2 = ((i2)->fld[3].rtx) ;
      if (	((p1)->code)  != 	((p2)->code) 
	  || !rtx_renumbered_equal_p (p1, p2))
	{

	  if (sets_cc0_p (p1) || sets_cc0_p (p2))
	    last1 = afterlast1, last2 = afterlast2, ++minimum;

	  if (	((i1)->code)  == JUMP_INSN
	      &&   ((i1)->fld[7].rtx) 
	      && prev_real_insn (  ((i1)->fld[7].rtx) ) == e1)
	    --minimum;
	  break;
	}

      if (	((p1)->code)  != USE && 	((p1)->code)  != CLOBBER)
	{
	  afterlast1 = last1, afterlast2 = last2;
	  last1 = i1, last2 = i2, --minimum;
	}
    }

  if (minimum <= 0 && last1 != 0)
    *f1 = last1, *f2 = last2;
}

static void
do_cross_jump (insn, newjpos, newlpos)
     rtx insn, newjpos, newlpos;
{
  register rtx label;

  label = ((newlpos)->fld[1].rtx) ;
  if (	((label)->code)  != CODE_LABEL)
    {
      label = gen_label_rtx ();
      emit_label_after (label, ((newlpos)->fld[1].rtx) );
      ((label)->fld[4].rtint)  = 0;
    }
  if (	((((insn)->fld[3].rtx) )->code)  == RETURN)
    {
      extern rtx gen_jump ();
      ((insn)->fld[3].rtx)  = gen_jump (label);
      ((insn)->fld[4].rtint)  = -1;
        ((insn)->fld[7].rtx)  = label;
      ((label)->fld[4].rtint) ++;
    }
  else
    redirect_jump (insn, label);
  newjpos = ((newjpos)->fld[1].rtx) ;
  while (((newjpos)->fld[2].rtx)  != insn)
    if (	((((newjpos)->fld[2].rtx) )->code)  != NOTE)
      delete_insn (((newjpos)->fld[2].rtx) );
    else
      newjpos = ((newjpos)->fld[2].rtx) ;
}

static int
jump_back_p (insn, target)
     rtx insn, target;
{
  rtx cinsn, ctarget, prev;
  enum rtx_code codei, codet;

  if (simplejump_p (insn) || ! condjump_p (insn)
      || simplejump_p (target))
    return 0;
  if (target != prev_real_insn (  ((insn)->fld[7].rtx) ))
    return 0;

  prev = prev_real_insn (insn);
  if (! (	((prev)->code)  == INSN
	 && 	((((prev)->fld[3].rtx) )->code)  == SET
	 && ((((prev)->fld[3].rtx) )->fld[0].rtx)  == cc0_rtx
	 && (	(mode_class[(int)(	((((((prev)->fld[3].rtx) )->fld[1].rtx) )->mode) )])  == MODE_INT
	     || (	((((((prev)->fld[3].rtx) )->fld[1].rtx) )->code)  == COMPARE
		 && (	(mode_class[(int)(	((((((((prev)->fld[3].rtx) )->fld[1].rtx) )->fld[ 0].rtx) )->mode) )]) 
		     == MODE_INT)))))
    return 0;

  cinsn = ((((((insn)->fld[3].rtx) )->fld[1].rtx) )->fld[ 0].rtx) ;
  ctarget = ((((((target)->fld[3].rtx) )->fld[1].rtx) )->fld[ 0].rtx) ;

  codei = 	((cinsn)->code) ;
  codet = 	((ctarget)->code) ;
  if (((((((insn)->fld[3].rtx) )->fld[1].rtx) )->fld[ 1].rtx)  == pc_rtx)

    codei = reverse_condition (codei);
  if (((((((target)->fld[3].rtx) )->fld[1].rtx) )->fld[ 2].rtx)  == pc_rtx)
    codet = reverse_condition (codet);
  return (codei == codet
	  && rtx_renumbered_equal_p (((cinsn)->fld[ 0].rtx) , ((ctarget)->fld[ 0].rtx) )
	  && rtx_renumbered_equal_p (((cinsn)->fld[ 1].rtx) , ((ctarget)->fld[ 1].rtx) ));
}

static enum rtx_code
reverse_condition (code)
     enum rtx_code code;
{
  switch (code)
    {
    case EQ:
      return NE;

    case NE:
      return EQ;

    case GT:
      return LE;

    case GE:
      return LT;

    case LT:
      return GE;

    case LE:
      return GT;

    case GTU:
      return LEU;

    case GEU:
      return LTU;

    case LTU:
      return GEU;

    case LEU:
      return GTU;

    default:
      abort ();
      return UNKNOWN;
    }
}

int
simplejump_p (insn)
     rtx insn;
{
  register rtx x = ((insn)->fld[3].rtx) ;
  if (	((x)->code)  != SET)
    return 0;
  if (	((((x)->fld[0].rtx) )->code)  != PC)
    return 0;
  if (	((((x)->fld[1].rtx) )->code)  != LABEL_REF)
    return 0;
  return 1;
}

int
condjump_p (insn)
     rtx insn;
{
  register rtx x = ((insn)->fld[3].rtx) ;
  if (	((x)->code)  != SET)
    return 0;
  if (	((((x)->fld[0].rtx) )->code)  != PC)
    return 0;
  if (	((((x)->fld[1].rtx) )->code)  == LABEL_REF)
    return 1;
  if (	((((x)->fld[1].rtx) )->code)  != IF_THEN_ELSE)
    return 0;
  if (((((x)->fld[1].rtx) )->fld[ 2].rtx)  == pc_rtx
      && 	((((((x)->fld[1].rtx) )->fld[ 1].rtx) )->code)  == LABEL_REF)
    return 1;
  if (((((x)->fld[1].rtx) )->fld[ 1].rtx)  == pc_rtx
      && 	((((((x)->fld[1].rtx) )->fld[ 2].rtx) )->code)  == LABEL_REF)
    return 1;
  return 0;
}

int
sets_cc0_p (x)
     rtx x;
{
  if (	((x)->code)  == SET && ((x)->fld[0].rtx)  == cc0_rtx)
    return 1;
  if (	((x)->code)  == PARALLEL)
    {
      int i;
      int sets_cc0 = 0;
      int other_things = 0;
      for (i = ((x)->fld[ 0].rtvec->num_elem)  - 1; i >= 0; i--)
	{
	  if (	((((x)->fld[ 0].rtvec->elem[ i].rtx) )->code)  == SET
	      && ((((x)->fld[ 0].rtvec->elem[ i].rtx) )->fld[0].rtx)  == cc0_rtx)
	    sets_cc0 = 1;
	  else if (	((((x)->fld[ 0].rtvec->elem[ i].rtx) )->code)  == SET)
	    other_things = 1;
	}
      return ! sets_cc0 ? 0 : other_things ? -1 : 1;
    }
  return 0;
}

int
no_labels_between_p (beg, end)
     rtx beg, end;
{
  register rtx p;
  for (p = beg; p != end; p = ((p)->fld[2].rtx) )
    if (	((p)->code)  == CODE_LABEL)
      return 0;
  return 1;
}

rtx
prev_real_insn (label)
     rtx label;
{
  register rtx insn = ((label)->fld[1].rtx) ;
  register enum rtx_code  code;

  while (1)
    {
      if (insn == 0)
	return 0;
      code = 	((insn)->code) ;
      if (code == INSN || code == CALL_INSN || code == JUMP_INSN)
	break;
      insn = ((insn)->fld[1].rtx) ;
    }

  return insn;
}

rtx
next_real_insn (label)
     rtx label;
{
  register rtx insn = ((label)->fld[2].rtx) ;
  register enum rtx_code  code;

  while (1)
    {
      if (insn == 0)
	return insn;
      code = 	((insn)->code) ;
      if (code == INSN || code == CALL_INSN || code == JUMP_INSN)
	break;
      insn = ((insn)->fld[2].rtx) ;
    }

  return insn;
}

rtx
next_label (insn)
     rtx insn;
{
  do insn = ((insn)->fld[2].rtx) ;
  while (insn != 0 && 	((insn)->code)  != CODE_LABEL);
  return insn;
}

static rtx
follow_jumps (label, ignore_loops)
     rtx label;
     int ignore_loops;
{
  register rtx insn;
  register rtx next;
  register rtx value = label;
  register int depth;

  for (depth = 0;
       (depth < 10
	&& (insn = next_real_insn (value)) != 0
	&& 	((insn)->code)  == JUMP_INSN
	&&   ((insn)->fld[7].rtx)  != 0
	&& (next = ((insn)->fld[2].rtx) )
	&& 	((next)->code)  == BARRIER);
       depth++)
    {

      rtx tem;
      if (!ignore_loops)
	for (tem = value; tem != insn; tem = ((tem)->fld[2].rtx) )
	  if (	((tem)->code)  == NOTE
	      && ((tem)->fld[4].rtint)  == -4 )
	    return value;

      if (  ((insn)->fld[7].rtx)  == label)
	break;
      value =   ((insn)->fld[7].rtx) ;
    }
  return value;
}

static int
tension_vector_labels (x, idx, ignore_loops)
     register rtx x;
     register int idx;
     int ignore_loops;
{
  int changed = 0;
  register int i;
  for (i = ((x)->fld[ idx].rtvec->num_elem)  - 1; i >= 0; i--)
    {
      register rtx olabel = ((((x)->fld[ idx].rtvec->elem[ i].rtx) )->fld[ 0].rtx) ;
      register rtx nlabel = follow_jumps (olabel, ignore_loops);
      if (nlabel != olabel)
	{
	  ((((x)->fld[ idx].rtvec->elem[ i].rtx) )->fld[ 0].rtx)  = nlabel;
	  ++((nlabel)->fld[4].rtint) ;
	  if (--((olabel)->fld[4].rtint)  == 0)
	    delete_insn (olabel);
	  changed = 1;
	}
    }
  return changed;
}

static void
mark_jump_label (x, insn, cross_jump)
     register rtx x;
     rtx insn;
     int cross_jump;
{
  register enum rtx_code  code = 	((x)->code) ;
  register int i;
  register char *fmt;

  if (code == LABEL_REF)
    {
      register rtx label = ((x)->fld[ 0].rtx) ;
      register rtx next;
      if (	((label)->code)  != CODE_LABEL)
	return;

      for (next = ((label)->fld[2].rtx) ; next; next = ((next)->fld[2].rtx) )
	{
	  if (	((next)->code)  == CODE_LABEL)
	    label = next;
	  else if (	((next)->code)  != NOTE
		   || ((next)->fld[4].rtint)  == -4 
		   || ((next)->fld[4].rtint)  == -6 )
	    break;
	}
      ((x)->fld[ 0].rtx)  = label;
      ++((label)->fld[4].rtint) ;
      if (insn)
	  ((insn)->fld[7].rtx)  = label;
      return;
    }

  if (code == ADDR_VEC || code == ADDR_DIFF_VEC)
    insn = 0;

  fmt = 	(rtx_format[(int)(code)]) ;
  for (i = 	(rtx_length[(int)(code)]) ; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	mark_jump_label (((x)->fld[ i].rtx) , insn, cross_jump);
      else if (fmt[i] == 'E')
	{
	  register int j;
	  for (j = 0; j < ((x)->fld[ i].rtvec->num_elem) ; j++)
	    mark_jump_label (((x)->fld[ i].rtvec->elem[ j].rtx) , insn, cross_jump);
	}
    }
}

static void
delete_jump (insn)
     rtx insn;
{
  register rtx x = ((insn)->fld[3].rtx) ;
  register rtx prev;

  if (	((x)->code)  == SET
      && 	((((x)->fld[0].rtx) )->code)  == PC)
    {
      prev = ((insn)->fld[1].rtx) ;
      delete_insn (insn);

      while (prev && 	((prev)->code)  == NOTE)
	prev = ((prev)->fld[1].rtx) ;
      if (prev && 	((prev)->code)  == INSN
	  && sets_cc0_p (((prev)->fld[3].rtx) ) > 0
	  && !find_reg_note (prev, REG_INC, 0))
	delete_insn (prev);
    }
}

rtx
delete_insn (insn)
     register rtx insn;
{
  register rtx next = ((insn)->fld[2].rtx) ;
  register rtx prev = ((insn)->fld[1].rtx) ;

  if (((insn)->volatil) )
    {
      while (next && ((next)->volatil) )
	next = ((next)->fld[2].rtx) ;
      return next;
    }

  ((insn)->volatil)  = 1;

  if (next != 0 && 	((next)->code)  == BARRIER)
    {
      ((next)->volatil)  = 1;
      next = ((next)->fld[2].rtx) ;
    }

  if (optimize)
    {
      if (prev)
	((prev)->fld[2].rtx)  = next;

      if (next)
	((next)->fld[1].rtx) = prev;
    }

  if (	((insn)->code)  == JUMP_INSN &&   ((insn)->fld[7].rtx) )
    if (--((  ((insn)->fld[7].rtx) )->fld[4].rtint)  == 0)
      {

	delete_insn (  ((insn)->fld[7].rtx) );

	while (next && ((next)->volatil) )
	  next = ((next)->fld[2].rtx) ;
	return next;
      }

  while (prev && (((prev)->volatil)  || 	((prev)->code)  == NOTE))
    prev = ((prev)->fld[1].rtx) ;

  if (	((insn)->code)  == CODE_LABEL && prev
      && 	((prev)->code)  == BARRIER)
    {
      register enum rtx_code  code;
      while (next != 0
	     && ((code = 	((next)->code) ) == INSN
		 || code == JUMP_INSN || code == CALL_INSN
		 || code == NOTE))
	{
	  if (code == NOTE
	      && ((next)->fld[4].rtint)  != -6 )
	    next = ((next)->fld[2].rtx) ;
	  else

	    next = delete_insn (next);
	}
    }

  return next;
}

rtx
next_nondeleted_insn (insn)
     rtx insn;
{
  while (((insn)->volatil) )
    insn = ((insn)->fld[2].rtx) ;
  return insn;
}

void
delete_for_peephole (from, to)
     register rtx from, to;
{
  register rtx insn = from;

  while (1)
    {
      register rtx next = ((insn)->fld[2].rtx) ;
      register rtx prev = ((insn)->fld[1].rtx) ;

      if (	((insn)->code)  != NOTE)
	{
	  ((insn)->volatil)  = 1;

	  if (prev)
	    ((prev)->fld[2].rtx)  = next;

	  if (next)
	    ((next)->fld[1].rtx)  = prev;
	}

      if (insn == to)
	break;
      insn = next;
    }

}

void
invert_jump (jump, nlabel)
     rtx jump, nlabel;
{
  register rtx olabel =   ((jump)->fld[7].rtx) ;
  invert_exp (((jump)->fld[3].rtx) , olabel, nlabel);
    ((jump)->fld[7].rtx)  = nlabel;
  ++((nlabel)->fld[4].rtint) ;
  ((jump)->fld[4].rtint)  = -1;

  if (--((olabel)->fld[4].rtint)  == 0)
    delete_insn (olabel);
}

void
invert_exp (x, olabel, nlabel)
     rtx x;
     rtx olabel, nlabel;
{
  register enum rtx_code  code;
  register int i;
  register char *fmt;

  if (x == 0)
    return;

  code = 	((x)->code) ;
  if (code == IF_THEN_ELSE)
    {

      register rtx tem = ((x)->fld[ 1].rtx) ;
      ((x)->fld[ 1].rtx)  = ((x)->fld[ 2].rtx) ;
      ((x)->fld[ 2].rtx)  = tem;
    }

  if (code == LABEL_REF)
    {
      if (((x)->fld[ 0].rtx)  == olabel)
	((x)->fld[ 0].rtx)  = nlabel;
      return;
    }

  fmt = 	(rtx_format[(int)(code)]) ;
  for (i = 	(rtx_length[(int)(code)])  - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	invert_exp (((x)->fld[ i].rtx) , olabel, nlabel);
      if (fmt[i] == 'E')
	{
	  register int j;
	  for (j = 0; j < ((x)->fld[ i].rtvec->num_elem) ; j++)
	    invert_exp (((x)->fld[ i].rtvec->elem[ j].rtx) , olabel, nlabel);
	}
    }
}

void
redirect_jump (jump, nlabel)
     rtx jump, nlabel;
{
  register rtx olabel =   ((jump)->fld[7].rtx) ;

  if (nlabel == olabel)
    return;

  redirect_exp (((jump)->fld[3].rtx) , olabel, nlabel);
    ((jump)->fld[7].rtx)  = nlabel;
  ++((nlabel)->fld[4].rtint) ;
  ((jump)->fld[4].rtint)  = -1;

  if (--((olabel)->fld[4].rtint)  == 0)
    delete_insn (olabel);
}

static void
redirect_exp (x, olabel, nlabel)
     rtx x;
     rtx olabel, nlabel;
{
  register enum rtx_code  code = 	((x)->code) ;
  register int i;
  register char *fmt;

  if (code == LABEL_REF)
    {
      if (((x)->fld[ 0].rtx)  == olabel)
	((x)->fld[ 0].rtx)  = nlabel;
      return;
    }

  fmt = 	(rtx_format[(int)(code)]) ;
  for (i = 	(rtx_length[(int)(code)])  - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	redirect_exp (((x)->fld[ i].rtx) , olabel, nlabel);
      if (fmt[i] == 'E')
	{
	  register int j;
	  for (j = 0; j < ((x)->fld[ i].rtvec->num_elem) ; j++)
	    redirect_exp (((x)->fld[ i].rtvec->elem[ j].rtx) , olabel, nlabel);
	}
    }
}

int
rtx_renumbered_equal_p (x, y)
     rtx x, y;
{
  register int i;
  register enum rtx_code  code = 	((x)->code) ;
  register char *fmt;
  if (x == y)
    return 1;
  if ((code == REG || (code == SUBREG && 	((((x)->fld[0].rtx) )->code)  == REG))
      && (	((y)->code)  == REG || (	((y)->code)  == SUBREG
				  && 	((((y)->fld[0].rtx) )->code)  == REG)))
    {
      register int j;

      if (	((x)->mode)  != 	((y)->mode) )
	return 0;

      if (code == SUBREG)
	{
	  i = ((((x)->fld[0].rtx) )->fld[0].rtint) ;
	  if (reg_renumber[i] >= 0)
	    i = reg_renumber[i];
	  i += ((x)->fld[1].rtint) ;
	}
      else
	{
	  i = ((x)->fld[0].rtint) ;
	  if (reg_renumber[i] >= 0)
	    i = reg_renumber[i];
	}
      if (	((y)->code)  == SUBREG)
	{
	  j = ((((y)->fld[0].rtx) )->fld[0].rtint) ;
	  if (reg_renumber[j] >= 0)
	    j = reg_renumber[j];
	  j += ((y)->fld[1].rtint) ;
	}
      else
	{
	  j = ((y)->fld[0].rtint) ;
	  if (reg_renumber[j] >= 0)
	    j = reg_renumber[j];
	}
      return i == j;
    }

  if (code != 	((y)->code) )
    return 0;
  switch (code)
    {
    case PC:
    case CC0:
    case ADDR_VEC:
    case ADDR_DIFF_VEC:
      return 0;

    case CONST_INT:
      return ((x)->fld[ 0].rtint)  == ((y)->fld[ 0].rtint) ;

    case LABEL_REF:

      return (next_real_insn (((x)->fld[ 0].rtx) )
	      == next_real_insn (((y)->fld[ 0].rtx) ));

    case SYMBOL_REF:
      return ((x)->fld[ 0].rtstr)  == ((y)->fld[ 0].rtstr) ;
    }

  if (	((x)->mode)  != 	((y)->mode) )
    return 0;

  fmt = 	(rtx_format[(int)(code)]) ;
  for (i = 	(rtx_length[(int)(code)])  - 1; i >= 0; i--)
    {
      register int j;
      switch (fmt[i])
	{
	case 'i':
	  if (((x)->fld[ i].rtint)  != ((y)->fld[ i].rtint) )
	    return 0;
	  break;

	case 's':
	  if (strcmp (((x)->fld[ i].rtstr) , ((y)->fld[ i].rtstr) ))
	    return 0;
	  break;

	case 'e':
	  if (! rtx_renumbered_equal_p (((x)->fld[ i].rtx) , ((y)->fld[ i].rtx) ))
	    return 0;
	  break;

	case '0':
	  break;

	case 'E':
	  if (((x)->fld[ i].rtvec->num_elem)  != ((y)->fld[ i].rtvec->num_elem) )
	    return 0;
	  for (j = ((x)->fld[ i].rtvec->num_elem)  - 1; j >= 0; j--)
	    if (!rtx_renumbered_equal_p (((x)->fld[ i].rtvec->elem[ j].rtx) , ((y)->fld[ i].rtvec->elem[ j].rtx) ))
	      return 0;
	  break;

	default:
	  abort ();
	}
    }
  return 1;
}

int
true_regnum (x)
     rtx x;
{
  if (	((x)->code)  == REG)
    {
      if (((x)->fld[0].rtint)  >= 56 )
	return reg_renumber[((x)->fld[0].rtint) ];
      return ((x)->fld[0].rtint) ;
    }
  if (	((x)->code)  == SUBREG)
    {
      int base = true_regnum (((x)->fld[0].rtx) );
      if (base >= 0 && base < 56 )
	return ((x)->fld[1].rtint)  + base;
    }
  return -1;
}

