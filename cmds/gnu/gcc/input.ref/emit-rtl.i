
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

extern double ldexp ();

extern double atof ();

union real_extract 
{
  double  d;
  int i[sizeof (double ) / sizeof (int)];
};

int reg_rtx_no = 56 ;

static int label_num = 1;

static int first_label_num;

static int no_line_numbers;

rtx pc_rtx;			 
rtx cc0_rtx;			 
rtx cc1_rtx;			 
rtx const0_rtx;			 
rtx const1_rtx;			 
rtx fconst0_rtx;		 
rtx dconst0_rtx;		 

rtx stack_pointer_rtx;		 
rtx frame_pointer_rtx;		 
rtx arg_pointer_rtx;		 
rtx struct_value_rtx;		 
rtx struct_value_incoming_rtx;	 
rtx static_chain_rtx;		 
rtx static_chain_incoming_rtx;	 

static rtx first_insn = 0 ;
static rtx last_insn = 0 ;

rtx sequence_stack = 0;

static int cur_insn_uid = 1;

static int last_linenum = 0;
static char *last_filename = 0;

char *regno_pointer_flag;
int regno_pointer_flag_length;

rtx *regno_reg_rtx;

extern char *emit_filename;
extern int emit_lineno;

rtx change_address ();

rtx
gen_rtx ( __builtin_va_alist )
        int __builtin_va_alist; 
{
    char *  p;
  enum rtx_code code;
  enum machine_mode mode;
  register int i;		 
  register char *fmt;		 
  register rtx rt_val;		 

   p=(char *) &__builtin_va_alist ;
  code = (p += (((sizeof ( enum rtx_code) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( enum rtx_code *) (p - (((sizeof ( enum rtx_code) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;
  mode = (p += (((sizeof ( enum machine_mode) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( enum machine_mode *) (p - (((sizeof ( enum machine_mode) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;

  if (code == CONST_INT)
    {
      int arg = (p += (((sizeof ( int) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( int *) (p - (((sizeof ( int) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;
      if (arg == 0)
	return const0_rtx;
      if (arg == 1)
	return const1_rtx;
      rt_val = rtx_alloc (code);
      ((rt_val)->fld[0].rtint)  = arg;
    }
  else
    {
      rt_val = rtx_alloc (code);	 
      rt_val->mode = mode;		 

      fmt = 	(rtx_format[(int)(code)]) ;	 
      for (i = 0; i < 	(rtx_length[(int)(code)]) ; i++)
	{
	  switch (*fmt++)
	    {
	    case '0':		 
	      break;

	    case 'i':		 
	      ((rt_val)->fld[ i].rtint)  = (p += (((sizeof ( int) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( int *) (p - (((sizeof ( int) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;
	      break;

	    case 's':		 
	      ((rt_val)->fld[ i].rtstr)  = (p += (((sizeof ( char *) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( char * *) (p - (((sizeof ( char *) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;
	      break;

	    case 'e':		 
	    case 'u':		 
	      ((rt_val)->fld[ i].rtx)  = (p += (((sizeof ( rtx) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( rtx *) (p - (((sizeof ( rtx) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;
	      break;

	    case 'E':		 
	      ((rt_val)->fld[ i].rtvec)  = (p += (((sizeof ( rtvec) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( rtvec *) (p - (((sizeof ( rtvec) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;
	      break;

	    default:
	      abort();
	    }
	}
    }
   ;
  return rt_val;		 
}

rtvec
gen_rtvec ( __builtin_va_alist )
        int __builtin_va_alist; 
{
  int n, i;
    char *  p;
  rtx *vector;

   p=(char *) &__builtin_va_alist ;
  n = (p += (((sizeof ( int) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( int *) (p - (((sizeof ( int) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;

  if (n == 0)
    return (rtvec) 0  ;		 

  vector = (rtx *) __builtin_alloca  (n * sizeof (rtx));
  for (i = 0; i < n; i++)
    vector[i] = (p += (((sizeof ( rtx) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ,	*(( rtx *) (p - (((sizeof ( rtx) + sizeof (int) - 1) / sizeof (int)) * sizeof (int)) ))) ;
   ;

  return gen_rtvec_v (n, vector);
}

rtvec
gen_rtvec_v (n, argp)
     int n;
     rtx *argp;
{
  register int i;
  register rtvec rt_val;

  if (n == 0)
    return (rtvec) 0  ;		 

  rt_val = rtvec_alloc (n);	 

  for (i = 0; i < n; i++)
    rt_val->elem[i].rtx = *argp++;

  return rt_val;
}

rtx
gen_reg_rtx (mode)
     enum machine_mode mode;
{
  register rtx val;

  if (reg_rtx_no == regno_pointer_flag_length)
    {
      rtx *new1;
      char *new =
	(char *) oballoc (regno_pointer_flag_length * 2);
      memset (new,0, regno_pointer_flag_length * 2) ;
      memcpy ( new,regno_pointer_flag, regno_pointer_flag_length) ;
      regno_pointer_flag = new;

      new1 = (rtx *) oballoc (regno_pointer_flag_length * 2 * sizeof (rtx));
      memset (new1,0, regno_pointer_flag_length * 2 * sizeof (rtx)) ;
      memcpy ( new1,regno_reg_rtx, regno_pointer_flag_length * sizeof (rtx)) ;
      regno_reg_rtx = new1;

      regno_pointer_flag_length *= 2;
    }

  val = gen_rtx (REG, mode, reg_rtx_no);
  regno_reg_rtx[reg_rtx_no++] = val;
  return val;
}

void
mark_reg_pointer (reg)
     rtx reg;
{
  regno_pointer_flag[((reg)->fld[0].rtint) ]  = 1;
}

int
max_reg_num ()
{
  return reg_rtx_no;
}

int
max_label_num ()
{
  return label_num;
}

int
get_first_label_num ()
{
  return first_label_num;
}

rtx
gen_lowpart (mode, x)
     enum machine_mode mode;
     register rtx x;
{

  if (	(mode_size[(int)(mode)])  > 4 
      && 	(mode_size[(int)(mode)])  != 	(mode_size[(int)(	((x)->mode) )]) )
    abort ();
  if (	((x)->mode)  == mode)
    return x;
  if (	((x)->code)  == CONST_INT)
    return gen_rtx (CONST_INT, VOIDmode, ((x)->fld[0].rtint)  & (( (8  * mode_size[(int)(mode)])  >= 32 ) ? -1 : ((1 <<  (8  * mode_size[(int)(mode)]) ) - 1)) );
  if (	((x)->code)  == CONST_DOUBLE)
    return gen_rtx (CONST_INT, VOIDmode,
		    ((x)->fld[ 2].rtint)   & (( (8  * mode_size[(int)(mode)])  >= 32 ) ? -1 : ((1 <<  (8  * mode_size[(int)(mode)]) ) - 1)) );
  if (	((x)->code)  == MEM)
    {
      register int offset = 0;

      offset -= (((4 ) < ( 	(mode_size[(int)(mode)]) ) ? (4 ) : ( 	(mode_size[(int)(mode)]) )) 
		 - ((4 ) < ( 	(mode_size[(int)(	((x)->mode) )]) ) ? (4 ) : ( 	(mode_size[(int)(	((x)->mode) )]) )) );

      return change_address (x, mode, plus_constant (((x)->fld[ 0].rtx) , offset));
    }
  else if (	((x)->code)  == SUBREG)
    return (	((((x)->fld[0].rtx) )->mode)  == mode && ((x)->fld[1].rtint)  == 0
	    ? ((x)->fld[0].rtx) 
	    : gen_rtx (SUBREG, mode, ((x)->fld[0].rtx) , ((x)->fld[1].rtint) ));
  else if (	((x)->code)  == REG)
    {

      return gen_rtx (SUBREG, mode, x, 0);
    }
  else
    abort ();
}

rtx
gen_highpart (mode, x)
     enum machine_mode mode;
     register rtx x;
{
  if (	((x)->code)  == MEM)
    {
      register int offset = 0;

      offset = (((	(mode_size[(int)(	((x)->mode) )]) ) > ( 4 ) ? (	(mode_size[(int)(	((x)->mode) )]) ) : ( 4 )) 
		- ((	(mode_size[(int)(mode)]) ) > ( 4 ) ? (	(mode_size[(int)(mode)]) ) : ( 4 )) );

      return change_address (x, mode, plus_constant (((x)->fld[ 0].rtx) , offset));
    }
  else if (	((x)->code)  == REG)
    {

      if (	(mode_size[(int)(	((x)->mode) )])  > 4 )
	{
	  return gen_rtx (SUBREG, mode, x,
			  ((	(mode_size[(int)(	((x)->mode) )]) 
			    - ((	(mode_size[(int)(mode)]) ) > ( 4 ) ? (	(mode_size[(int)(mode)]) ) : ( 4 )) )
			   / 4 ));
	}

      return gen_rtx (SUBREG, mode, x, 0);
    }
  else
    abort ();
}

int
subreg_lowpart_p (x)
     rtx x;
{
  if (	((x)->code)  != SUBREG)
    return 1;

  return ((x)->fld[1].rtint)  == 0;
}

rtx
change_address (memref, mode, addr)
     rtx memref;
     enum machine_mode mode;
     rtx addr;
{
  rtx new;

  if (	((memref)->code)  != MEM)
    abort ();
  if (mode == VOIDmode)
    mode = 	((memref)->mode) ;
  if (addr == 0)
    addr = ((memref)->fld[ 0].rtx) ;

  new = gen_rtx (MEM, mode, memory_address (mode, addr));
  ((new)->volatil)  = ((memref)->volatil) ;
  ((new)->unchanging)  = ((memref)->unchanging) ;
  ((new)->in_struct)  = ((memref)->in_struct) ;
  return new;
}

rtx
gen_label_rtx ()
{
  register rtx label = gen_rtx (CODE_LABEL, VOIDmode, 0, 0, 0, label_num++);
  ((label)->fld[4].rtint)  = 0;
  return label;
}

rtx
gen_inline_header_rtx (insn, last_insn,
		       first_labelno, last_labelno,
		       max_parm_regnum, max_regnum, args_size)
     rtx insn, last_insn;
     int first_labelno, last_labelno, max_parm_regnum, max_regnum, args_size;
{
  rtx header = gen_rtx (INLINE_HEADER, VOIDmode,
			cur_insn_uid++, 0 ,
			insn, last_insn,
			first_labelno, last_labelno,
			max_parm_regnum, max_regnum, args_size);
  return header;
}

void
set_new_first_and_last_insn (first, last)
     rtx first, last;
{
  first_insn = first;
  last_insn = last;
}

static int unshare_copies = 0;	 

static rtx copy_rtx_if_shared ();

void
unshare_all_rtl (insn)
     register rtx insn;
{
  extern rtx stack_slot_list;

  for (; insn; insn = ((insn)->fld[2].rtx) )
    if (	((insn)->code)  == INSN || 	((insn)->code)  == JUMP_INSN
	|| 	((insn)->code)  == CALL_INSN)
      {
	((insn)->fld[3].rtx)  = copy_rtx_if_shared (((insn)->fld[3].rtx) );
	((insn)->fld[6].rtx)  = copy_rtx_if_shared (((insn)->fld[6].rtx) );
		((insn)->fld[5].rtx)  = copy_rtx_if_shared (	((insn)->fld[5].rtx) );
      }

  copy_rtx_if_shared (stack_slot_list);
}

static rtx
copy_rtx_if_shared (orig)
     rtx orig;
{
  register rtx x = orig;
  register int i;
  register enum rtx_code code;
  register char *format_ptr;
  int copied = 0;

  if (x == 0)
    return 0;

  code = 	((x)->code) ;

  switch (code)
    {
    case REG:
    case QUEUED:
    case CONST_INT:
    case CONST_DOUBLE:
    case SYMBOL_REF:
    case CODE_LABEL:
    case PC:
    case CC0:
      return x;

    case INSN:
    case JUMP_INSN:
    case CALL_INSN:
    case NOTE:
    case LABEL_REF:
    case BARRIER:
      return x;

    case MEM:

      if ( (	((((x)->fld[ 0].rtx) )->code)  == LABEL_REF || 	((((x)->fld[ 0].rtx) )->code)  == SYMBOL_REF	|| 	((((x)->fld[ 0].rtx) )->code)  == CONST_INT	|| 	((((x)->fld[ 0].rtx) )->code)  == CONST)  )
	return x;
      if (	((((x)->fld[ 0].rtx) )->code)  == PLUS
	  && (((((x)->fld[ 0].rtx) )->fld[ 0].rtx)  == frame_pointer_rtx
	      || ((((x)->fld[ 0].rtx) )->fld[ 0].rtx)  == arg_pointer_rtx)
	  &&  (	((((((x)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == LABEL_REF || 	((((((x)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == SYMBOL_REF	|| 	((((((x)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	|| 	((((((x)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST)  )
	{

	  if (! x->used)
	    ((x)->fld[ 0].rtx)  = copy_rtx_if_shared (((x)->fld[ 0].rtx) );
	  x->used = 1;
	  return x;
	}
      if (((x)->fld[ 0].rtx)  == frame_pointer_rtx
	  || ((x)->fld[ 0].rtx)  == arg_pointer_rtx)
	return x;
    }

  if (x->used)
    {
      register rtx copy;

      unshare_copies++;

      copy = rtx_alloc (code);
      memcpy ( copy,x, sizeof (int) * (	(rtx_length[(int)(code)])  + 1)) ;
      x = copy;
      copied = 1;
    }
  x->used = 1;

  format_ptr = 	(rtx_format[(int)(code)]) ;

  for (i = 0; i < 	(rtx_length[(int)(code)]) ; i++)
    {
      switch (*format_ptr++)
	{
	case 'e':
	  ((x)->fld[ i].rtx)  = copy_rtx_if_shared (((x)->fld[ i].rtx) );
	  break;

	case 'E':
	  if (((x)->fld[ i].rtvec)  != 0 )
	    {
	      register int j;

	      if (copied)
		((x)->fld[ i].rtvec)  = gen_rtvec_v (((x)->fld[ i].rtvec->num_elem) , &((x)->fld[ i].rtvec->elem[ 0].rtx) );
	      for (j = 0; j < ((x)->fld[ i].rtvec->num_elem) ; j++)
		((x)->fld[ i].rtvec->elem[ j].rtx) 
		  = copy_rtx_if_shared (((x)->fld[ i].rtvec->elem[ j].rtx) );
	    }
	  break;
	}
    }
  return x;
}

rtx
make_safe_from (x, other)
     rtx x, other;
{
  while (1)
    switch (	((other)->code) )
      {
      case SUBREG:
	other = ((other)->fld[0].rtx) ;
	break;
      case STRICT_LOW_PART:
      case SIGN_EXTEND:
      case ZERO_EXTEND:
	other = ((other)->fld[ 0].rtx) ;
	break;
      default:
	goto done;
      }
 done:
  if ((	((other)->code)  == MEM
       && ! (	((x)->code)  == LABEL_REF || 	((x)->code)  == SYMBOL_REF	|| 	((x)->code)  == CONST_INT	|| 	((x)->code)  == CONST) 
       && 	((x)->code)  != CONST_DOUBLE
       && 	((x)->code)  != REG
       && 	((x)->code)  != SUBREG)
      || (	((other)->code)  == REG
	  && (((other)->fld[0].rtint)  < 56 
	      || reg_mentioned_p (other, x))))
    {
      rtx temp = gen_reg_rtx (	((x)->mode) );
      emit_move_insn (temp, x);
      return temp;
    }
  return x;
}

rtx
get_insns ()
{
  return first_insn;
}

rtx
get_last_insn ()
{
  return last_insn;
}

int
get_max_uid ()
{
  return cur_insn_uid;
}

rtx
next_insn (insn)
     rtx insn;
{
  if (insn) return ((insn)->fld[2].rtx) ;
  return 0;
}

rtx
previous_insn (insn)
     rtx insn;
{
  if (insn) return ((insn)->fld[1].rtx) ;
  return 0;
}

static rtx
make_insn_raw (pattern, pat_formals)
     rtx pattern;
     rtvec pat_formals;
{
  register rtx insn;

  insn = rtx_alloc(INSN);
  ((insn)->fld[0].rtint)  = cur_insn_uid++;

  ((insn)->fld[3].rtx)  = pattern;
  ((insn)->fld[4].rtint)  = -1;
  	((insn)->fld[5].rtx)  = 0 ;
  ((insn)->fld[6].rtx)  = 0 ;

  return insn;
}

static rtx
make_jump_insn_raw (pattern, pat_formals)
     rtx pattern;
     rtvec pat_formals;
{
  register rtx insn;

  insn = rtx_alloc(JUMP_INSN);
  ((insn)->fld[0].rtint)  = cur_insn_uid++;

  ((insn)->fld[3].rtx)  = pattern;
  ((insn)->fld[4].rtint)  = -1;
  	((insn)->fld[5].rtx)  = 0 ;
  ((insn)->fld[6].rtx)  = 0 ;
    ((insn)->fld[7].rtx)  = 0 ;

  return insn;
}

static void
add_insn (insn)
     register rtx insn;
{
  ((insn)->fld[1].rtx)  = last_insn;
  ((insn)->fld[2].rtx)  = 0;

  if (0  != last_insn)
    ((last_insn)->fld[2].rtx)  = insn;

  if (0  == first_insn)
    first_insn = insn;

  last_insn = insn;
}

static void
add_insn_after (insn, after)
     rtx insn, after;
{
  ((insn)->fld[2].rtx)  = ((after)->fld[2].rtx) ;
  ((insn)->fld[1].rtx)  = after;

  if (((insn)->fld[2].rtx) )
    ((((insn)->fld[2].rtx) )->fld[1].rtx)  = insn;
  else
    last_insn = insn;
  ((after)->fld[2].rtx)  = insn;
}

void
delete_insns_since (from)
     rtx from;
{
  if (from == 0)
    first_insn = 0;
  else
    ((from)->fld[2].rtx)  = 0;
  last_insn = from;
}

void
reorder_insns (from, to, after)
     rtx from, to, after;
{
  if (((from)->fld[1].rtx) )
    ((((from)->fld[1].rtx) )->fld[2].rtx)  = ((to)->fld[2].rtx) ;
  if (((to)->fld[2].rtx) )
    ((((to)->fld[2].rtx) )->fld[1].rtx)  = ((from)->fld[1].rtx) ;
  if (last_insn == to)
    last_insn = ((from)->fld[1].rtx) ;
  if (first_insn == from)
    first_insn = ((to)->fld[2].rtx) ;

  if (((after)->fld[2].rtx) )
    {
      ((((after)->fld[2].rtx) )->fld[1].rtx)  = to;
      ((to)->fld[2].rtx)  = ((after)->fld[2].rtx) ;
    }
  ((from)->fld[1].rtx)  = after;
  ((after)->fld[2].rtx)  = from;
  if (after == last_insn)
    last_insn = to;
}

rtx
emit_insn_before (pattern, before)
     register rtx pattern, before;
{
  register rtx insn;

  if (	((pattern)->code)  == SEQUENCE)
    {
      register int i;
      if (((pattern)->fld[ 0].rtvec) )
	for (i = 0; i < ((pattern)->fld[ 0].rtvec->num_elem) ; i++)
	  add_insn_after (((pattern)->fld[ 0].rtvec->elem[ i].rtx) , ((before)->fld[1].rtx) );
      return ((before)->fld[1].rtx) ;
    }

  insn = make_insn_raw (pattern, 0);

  ((insn)->fld[1].rtx)  = ((before)->fld[1].rtx) ;
  ((insn)->fld[2].rtx)  = before;

  if (((insn)->fld[1].rtx) )
    ((((insn)->fld[1].rtx) )->fld[2].rtx)  = insn;
  else
    first_insn = insn;
  ((before)->fld[1].rtx)  = insn;

  return insn;
}

rtx
emit_jump_insn_before (pattern, before)
     register rtx pattern, before;
{
  register rtx insn = make_jump_insn_raw (pattern, 0);

  ((insn)->fld[1].rtx)  = ((before)->fld[1].rtx) ;
  ((insn)->fld[2].rtx)  = before;

  if (((insn)->fld[1].rtx) )
    ((((insn)->fld[1].rtx) )->fld[2].rtx)  = insn;
  else
    first_insn = insn;
  ((before)->fld[1].rtx)  = insn;

  return insn;
}

rtx
emit_call_insn_before (pattern, before)
     register rtx pattern, before;
{
  rtx insn = emit_insn_before (pattern, before);
  ((insn)->code = ( CALL_INSN)) ;
  return insn;
}

rtx
emit_insn_after (pattern, after)
     register rtx pattern, after;
{
  if (	((pattern)->code)  == SEQUENCE)
    {
      register int i;
      if (((pattern)->fld[ 0].rtvec) )
	for (i = 0; i < ((pattern)->fld[ 0].rtvec->num_elem) ; i++)
	  {
	    add_insn_after (((pattern)->fld[ 0].rtvec->elem[ i].rtx) , after);
	    after = ((after)->fld[2].rtx) ;
	  }
      return after;
    }
  else
    {
      register rtx insn = make_insn_raw (pattern, 0);
      add_insn_after (insn, after);
      return insn;
    }
}

rtx
emit_jump_insn_after (pattern, after)
     register rtx pattern, after;
{
  register rtx insn = make_jump_insn_raw (pattern, 0);

  add_insn_after (insn, after);
  return insn;
}

rtx
emit_barrier_after (after)
     register rtx after;
{
  register rtx insn = rtx_alloc (BARRIER);

  ((insn)->fld[0].rtint)  = cur_insn_uid++;

  add_insn_after (insn, after);
  return insn;
}

void
emit_label_after (label, after)
     rtx label, after;
{

  if (((label)->fld[0].rtint)  == 0)
    {
      ((label)->fld[0].rtint)  = cur_insn_uid++;
      add_insn_after (label, after);
    }
}

void
emit_note_after (subtype, after)
     int subtype;
     rtx after;
{
  register rtx note = rtx_alloc (NOTE);
  ((note)->fld[0].rtint)  = cur_insn_uid++;
  ((note)->fld[ 3].rtstr)  = 0;
  ((note)->fld[ 4].rtint)  = subtype;
  add_insn_after (note, after);
}

rtx
emit_insn (pattern)
     rtx pattern;
{
  rtx insn;

  if (	((pattern)->code)  == SEQUENCE)
    {
      register int i;
      if (((pattern)->fld[ 0].rtvec) )
	for (i = 0; i < ((pattern)->fld[ 0].rtvec->num_elem) ; i++)
	  add_insn (insn = ((pattern)->fld[ 0].rtvec->elem[ i].rtx) );
    }
  else
    {
      insn = make_insn_raw (pattern, 0 );
      add_insn (insn);
    }
  return insn;
}

rtx
emit_insns (insn)
     rtx insn;
{
  while (insn)
    {
      rtx next = ((insn)->fld[2].rtx) ;
      add_insn (insn);
      insn = next;
    }
}

rtx
emit_jump_insn (pattern)
     rtx pattern;
{
  if (	((pattern)->code)  == SEQUENCE)
    return emit_insn (pattern);
  else
    {
      register rtx insn = make_jump_insn_raw (pattern, 0 );
      add_insn (insn);
      return insn;
    }
}

rtx
emit_call_insn (pattern)
     rtx pattern;
{
  if (	((pattern)->code)  == SEQUENCE)
    return emit_insn (pattern);
  else
    {
      register rtx insn = make_insn_raw (pattern, 0 );
      add_insn (insn);
      ((insn)->code = ( CALL_INSN)) ;
      return insn;
    }
}

rtx
emit_label (label)
     rtx label;
{

  if (((label)->fld[0].rtint)  == 0)
    {
      ((label)->fld[0].rtint)  = cur_insn_uid++;
      add_insn (label);
    }
  return label;
}

rtx
emit_barrier ()
{
  register rtx barrier = rtx_alloc (BARRIER);
  ((barrier)->fld[0].rtint)  = cur_insn_uid++;
  add_insn (barrier);
  return barrier;
}

rtx
emit_line_note (file, line)
     char *file;
     int line;
{
  emit_filename = file;
  emit_lineno = line;

  if (no_line_numbers)
    return 0;

  return emit_note (file, line);
}

rtx
emit_note (file, line)
     char *file;
     int line;
{
  register rtx note;

  if (no_line_numbers && line > 0)
    return 0;

  if (line > 0)
    {
      if (file && last_filename && !strcmp (file, last_filename)
	  && line == last_linenum)
	return 0;
      last_filename = file;
      last_linenum = line;
    }

  note = rtx_alloc (NOTE);
  ((note)->fld[0].rtint)  = cur_insn_uid++;
  ((note)->fld[ 3].rtstr)  = file;
  ((note)->fld[ 4].rtint)  = line;
  add_insn (note);
  return note;
}

rtx
emit_line_note_force (file, line)
     char *file;
     int line;
{
  last_linenum = -1;
  return emit_line_note (file, line);
}

enum rtx_code
classify_insn (x)
     rtx x;
{
  if (	((x)->code)  == CODE_LABEL)
    return CODE_LABEL;
  if (	((x)->code)  == CALL)
    return CALL_INSN;
  if (	((x)->code)  == RETURN)
    return JUMP_INSN;
  if (	((x)->code)  == SET)
    {
      if (((x)->fld[0].rtx)  == pc_rtx)
	return JUMP_INSN;
      else if (	((((x)->fld[1].rtx) )->code)  == CALL)
	return CALL_INSN;
      else
	return INSN;
    }
  if (	((x)->code)  == PARALLEL)
    {
      register int j;
      for (j = ((x)->fld[ 0].rtvec->num_elem)  - 1; j >= 0; j--)
	if (	((((x)->fld[ 0].rtvec->elem[ j].rtx) )->code)  == CALL)
	  return CALL_INSN;
	else if (	((((x)->fld[ 0].rtvec->elem[ j].rtx) )->code)  == SET
		 && ((((x)->fld[ 0].rtvec->elem[ j].rtx) )->fld[0].rtx)  == pc_rtx)
	  return JUMP_INSN;
	else if (	((((x)->fld[ 0].rtvec->elem[ j].rtx) )->code)  == SET
		 && 	((((((x)->fld[ 0].rtvec->elem[ j].rtx) )->fld[1].rtx) )->code)  == CALL)
	  return CALL_INSN;
    }
  return INSN;
}

void
emit (x)
     rtx x;
{
  enum rtx_code code = classify_insn (x);

  if (code == CODE_LABEL)
    emit_label (x);
  else if (code == INSN)
    emit_insn (x);
  else if (code == JUMP_INSN)
    {
      register rtx insn = emit_jump_insn (x);
      if (simplejump_p (insn) || 	((x)->code)  == RETURN)
	emit_barrier ();
    }
  else if (code == CALL_INSN)
    emit_call_insn (x);
}

rtx
start_sequence ()
{
  sequence_stack
    = gen_rtx (INSN_LIST, VOIDmode,
	       first_insn, gen_rtx (INSN_LIST, VOIDmode,
				    last_insn, sequence_stack));
  first_insn = 0;
  last_insn = 0;
  return sequence_stack;
}

void
push_to_sequence (first)
     rtx first;
{
  rtx last;
  for (last = first; last && ((last)->fld[2].rtx) ; last = ((last)->fld[2].rtx) );
  sequence_stack
    = gen_rtx (INSN_LIST, VOIDmode,
	       first_insn, gen_rtx (INSN_LIST, VOIDmode,
				    last_insn, sequence_stack));
  first_insn = first;
  last_insn = last;
}

void
end_sequence (saved)
     rtx saved;
{
  first_insn = ((sequence_stack)->fld[ 0].rtx) ;
  last_insn = ((((sequence_stack)->fld[ 1].rtx) )->fld[ 0].rtx) ;
  sequence_stack = ((((sequence_stack)->fld[ 1].rtx) )->fld[ 1].rtx) ;
}

rtx
gen_sequence ()
{
  rtx tem;
  rtvec newvec;
  int i;
  int len;

  len = 0;
  for (tem = first_insn; tem; tem = ((tem)->fld[2].rtx) )
    len++;

  if (len == 0)
    return gen_rtx (SEQUENCE, VOIDmode, 0 );

  if (len == 1
      && (	((first_insn)->code)  == INSN
	  || 	((first_insn)->code)  == JUMP_INSN
	  || 	((first_insn)->code)  == CALL_INSN))
    return ((first_insn)->fld[3].rtx) ;

  newvec = rtvec_alloc (len);
  i = 0;
  for (tem = first_insn; tem; tem = ((tem)->fld[2].rtx) , i++)
    newvec->elem[i].rtx = tem;

  return gen_rtx (SEQUENCE, VOIDmode, newvec);
}

static void restore_reg_data_1 ();

void
restore_reg_data (first)
     rtx first;
{
  register rtx insn;
  int i;
  register int max_uid = 0;

  for (insn = first; insn; insn = ((insn)->fld[2].rtx) )
    {
      if (((insn)->fld[0].rtint)  >= max_uid)
	max_uid = ((insn)->fld[0].rtint) ;

      switch (	((insn)->code) )
	{
	case NOTE:
	case CODE_LABEL:
	case BARRIER:
	  break;

	case JUMP_INSN:
	case CALL_INSN:
	case INSN:
	  restore_reg_data_1 (((insn)->fld[3].rtx) );
	  break;
	}
    }

  cur_insn_uid = max_uid + 1;

  for (i = 56 ; i < reg_rtx_no; i++)
    if (regno_reg_rtx[i] == 0)
      regno_reg_rtx[i] = gen_rtx (REG, SImode, i);
}

static void
restore_reg_data_1 (orig)
     rtx orig;
{
  register rtx x = orig;
  register int i;
  register enum rtx_code code;
  register char *format_ptr;

  code = 	((x)->code) ;

  switch (code)
    {
    case QUEUED:
    case CONST_INT:
    case CONST_DOUBLE:
    case SYMBOL_REF:
    case CODE_LABEL:
    case PC:
    case CC0:
    case LABEL_REF:
      return;

    case REG:
      if (((x)->fld[0].rtint)  >= 56 )
	{

	  if (((x)->fld[0].rtint)  >= reg_rtx_no)
	    {
	      reg_rtx_no = ((x)->fld[0].rtint) ;

	      if (reg_rtx_no == regno_pointer_flag_length)
		{
		  rtx *new1;
		  char *new =
		    (char *) oballoc (regno_pointer_flag_length * 2);
		  memset (new,0, regno_pointer_flag_length * 2) ;
		  memcpy ( new,regno_pointer_flag, regno_pointer_flag_length) ;
		  regno_pointer_flag = new;

		  new1 = (rtx *) oballoc (regno_pointer_flag_length * 2 * sizeof (rtx));
		  memset (new1,0, regno_pointer_flag_length * 2 * sizeof (rtx)) ;
		  memcpy ( new1,regno_reg_rtx, regno_pointer_flag_length * sizeof (rtx)) ;
		  regno_reg_rtx = new1;

		  regno_pointer_flag_length *= 2;
		}
	      reg_rtx_no ++;
	    }
	  regno_reg_rtx[((x)->fld[0].rtint) ] = x;
	}
      return;

    case MEM:
      if (	((((x)->fld[ 0].rtx) )->code)  == REG)
	mark_reg_pointer (((x)->fld[ 0].rtx) );
      restore_reg_data_1 (((x)->fld[ 0].rtx) );
      return;
    }

  format_ptr = 	(rtx_format[(int)(code)]) ;

  for (i = 0; i < 	(rtx_length[(int)(code)]) ; i++)
    {
      switch (*format_ptr++)
	{
	case 'e':
	  restore_reg_data_1 (((x)->fld[ i].rtx) );
	  break;

	case 'E':
	  if (((x)->fld[ i].rtvec)  != 0 )
	    {
	      register int j;

	      for (j = 0; j < ((x)->fld[ i].rtvec->num_elem) ; j++)
		restore_reg_data_1 (((x)->fld[ i].rtvec->elem[ j].rtx) );
	    }
	  break;
	}
    }
}

void
init_emit (write_symbols)
     int write_symbols;
{
  first_insn = 0 ;
  last_insn = 0 ;
  sequence_stack = 0 ;
  cur_insn_uid = 1;
  reg_rtx_no = 56 ;
  last_linenum = 0;
  last_filename = 0;
  first_label_num = label_num;

  no_line_numbers = ! write_symbols;

  regno_pointer_flag_length = 56  + 100;

  regno_pointer_flag 
    = (char *) oballoc (regno_pointer_flag_length);
  memset (regno_pointer_flag,0, regno_pointer_flag_length) ;

  regno_reg_rtx 
    = (rtx *) oballoc (regno_pointer_flag_length * sizeof (rtx));
  memset (regno_reg_rtx,0, regno_pointer_flag_length * sizeof (rtx)) ;
}

void
init_emit_once ()
{

  pc_rtx = gen_rtx (PC, VOIDmode);
  cc0_rtx = gen_rtx (CC0, VOIDmode);

  const0_rtx = rtx_alloc (CONST_INT);
  ((const0_rtx)->fld[0].rtint)  = 0;
  const1_rtx = rtx_alloc (CONST_INT);
  ((const1_rtx)->fld[0].rtint)  = 1;

  fconst0_rtx = rtx_alloc (CONST_DOUBLE);
  dconst0_rtx = rtx_alloc (CONST_DOUBLE);
  {
    union real_extract u;

    u.d = 0;

    memcpy ( &((fconst0_rtx)->fld[ 2].rtint)  ,&u, sizeof u) ;
    ((fconst0_rtx)->fld[ 0].rtx)   = cc0_rtx;
    ((fconst0_rtx)->mode = ( SFmode)) ;

    memcpy ( &((dconst0_rtx)->fld[ 2].rtint)  ,&u, sizeof u) ;
    ((dconst0_rtx)->fld[ 0].rtx)   = cc0_rtx;
    ((dconst0_rtx)->mode = ( DFmode)) ;
  }

  stack_pointer_rtx = gen_rtx (REG, SImode , 15 );
  frame_pointer_rtx = gen_rtx (REG, SImode , 14 );

  struct_value_rtx = gen_rtx (REG, SImode , 9 );

  struct_value_incoming_rtx = struct_value_rtx;

  static_chain_rtx = gen_rtx (REG, SImode , 8 );

    static_chain_incoming_rtx = static_chain_rtx;

  if (14  == 14 )
    arg_pointer_rtx = frame_pointer_rtx;
  else
    arg_pointer_rtx = gen_rtx (REG, SImode , 14 );
}

