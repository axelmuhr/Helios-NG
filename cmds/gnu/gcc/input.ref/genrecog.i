
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

struct obstack obstack;
struct obstack *rtl_obstack = &obstack;

extern int xmalloc ();
extern void free ();

struct decision
{
  int number;
  char *position;
  enum rtx_code  code;
  char *exact;
  enum machine_mode mode;
  char *tests;
  int insn_code_number;
  struct decision *next;
  struct decision *success;
  int opno;
  int dupno;
  int dupcount;
  int test_elt_zero_int;
  int elt_zero_int;
  int test_elt_one_int;
  int elt_one_int;
  int ignmode;
  struct decision *afterward;
  int label_needed;
  char *c_test;
  char *reg_class;
  char enforce_mode;
  int veclen;
  int subroutine_number;
};

int next_subroutine_number;

int next_number;

int next_insn_code;

int dupcount;

struct decision *add_to_sequence ();
struct decision *try_merge_2 ();
void write_subroutine ();
void print_code ();
void clear_codes ();
void clear_modes ();
void change_state ();
void write_tree ();
char *copystr ();
char *concat ();
void fatal ();
void mybzero ();
struct decision *first;

struct decision *
make_insn_sequence (insn)
     rtx insn;
{
  rtx x;
  char *c_test = ((insn)->fld[ 2].rtstr) ;
  struct decision *last;

  dupcount = 0;

  if (((insn)->fld[ 1].rtvec->num_elem)  == 1)
    x = ((insn)->fld[ 1].rtvec->elem[ 0].rtx) ;
  else
    {
      x = rtx_alloc (PARALLEL);
      ((x)->fld[ 0].rtvec)  = ((insn)->fld[ 1].rtvec) ;
      ((x)->mode = ( VOIDmode)) ;
    }

  last = add_to_sequence (x, 0, "");

  if (c_test[0])
    last->c_test = c_test;
  last->insn_code_number = next_insn_code++;

  return first;
}

struct decision *
add_to_sequence (pattern, last, position)
     rtx pattern;
     struct decision *last;
     char *position;
{
  register enum rtx_code  code;
  register struct decision *new
    = (struct decision *) xmalloc (sizeof (struct decision));
  struct decision *this;
  char *newpos;
  register char *fmt;
  register int i;
  int depth;
  int len;

  new->number = next_number++;
  new->position = copystr (position);
  new->exact = 0;
  new->next = 0;
  new->success = 0;
  new->insn_code_number = -1;
  new->tests = 0;
  new->opno = -1;
  new->dupno = -1;
  new->dupcount = -1;
  new->test_elt_zero_int = 0;
  new->test_elt_one_int = 0;
  new->elt_zero_int = 0;
  new->elt_one_int = 0;
  new->enforce_mode = 0;
  new->ignmode = 0;
  new->afterward = 0;
  new->label_needed = 0;
  new->c_test = 0;
  new->reg_class = 0;
  new->veclen = 0;
  new->subroutine_number = 0;

  this = new;

  if (last == 0)
    first = new;
  else
    last->success = new;

  depth = strlen (position);
  newpos = (char *) __builtin_alloca  (depth + 2);
  strcpy (newpos, position);
  newpos[depth + 1] = 0;

 restart:

  if (pattern == 0)
    {
      new->exact = "0";
      new->code = UNKNOWN;
      new->mode = VOIDmode;
      return new;
    }

  switch (	((pattern)->mode) )
    {
    case 0:
      new->mode = VOIDmode;
      break;

    default:
      new->mode = 	((pattern)->mode) ;
      break;
    }

  new->code = code = 	((pattern)->code) ;

  switch (code)
    {
    case MATCH_OPERAND:
      new->opno = ((pattern)->fld[ 0].rtint) ;
      new->code = UNKNOWN;
      new->tests = ((pattern)->fld[ 1].rtstr) ;
      if (*new->tests == 0)
	new->tests = 0;
      new->reg_class = ((pattern)->fld[ 2].rtstr) ;
      if (*new->reg_class == 0)
	new->reg_class = 0;
      return new;

    case MATCH_OPERATOR:
      new->opno = ((pattern)->fld[ 0].rtint) ;
      new->code = UNKNOWN;
      new->tests = ((pattern)->fld[ 1].rtstr) ;
      if (*new->tests == 0)
	new->tests = 0;
      for (i = 0; i < ((pattern)->fld[ 2].rtvec->num_elem) ; i++)
	{
	  newpos[depth] = i + '0';
	  new = add_to_sequence (((pattern)->fld[ 2].rtvec->elem[ i].rtx) , new, newpos);
	}
      this->success->enforce_mode = 0;
      return new;

    case MATCH_DUP:
      new->dupno = ((pattern)->fld[ 0].rtint) ;
      new->dupcount = dupcount++;
      new->code = UNKNOWN;
      return new;

    case ADDRESS:
      pattern = ((pattern)->fld[ 0].rtx) ;
      goto restart;

    case PC:
      new->exact = "pc_rtx";
      return new;

    case CC0:
      new->exact = "cc0_rtx";
      return new;

    case CONST_INT:
      if (((pattern)->fld[0].rtint)  == 0)
	{
	  new->exact = "const0_rtx";
	  return new;
	}
      if (((pattern)->fld[0].rtint)  == 1)
	{
	  new->exact = "const1_rtx";
	  return new;
	}
      break;

    case SET:
      newpos[depth] = '0';
      new = add_to_sequence (((pattern)->fld[0].rtx) , new, newpos);
      this->success->enforce_mode = 1;
      newpos[depth] = '1';
      new = add_to_sequence (((pattern)->fld[1].rtx) , new, newpos);
      return new;

    case STRICT_LOW_PART:
      newpos[depth] = '0';
      new = add_to_sequence (((pattern)->fld[ 0].rtx) , new, newpos);
      this->success->enforce_mode = 1;
      return new;

    case SUBREG:
      this->test_elt_one_int = 1;
      this->elt_one_int = ((pattern)->fld[ 1].rtint) ;
      newpos[depth] = '0';
      new = add_to_sequence (((pattern)->fld[ 0].rtx) , new, newpos);
      this->success->enforce_mode = 1;
      return new;

    case ZERO_EXTRACT:
    case SIGN_EXTRACT:
      newpos[depth] = '0';
      new = add_to_sequence (((pattern)->fld[ 0].rtx) , new, newpos);
      this->success->enforce_mode = 1;
      newpos[depth] = '1';
      new = add_to_sequence (((pattern)->fld[ 1].rtx) , new, newpos);
      newpos[depth] = '2';
      new = add_to_sequence (((pattern)->fld[ 2].rtx) , new, newpos);
      return new;
    }

  fmt = 	(rtx_format[(int)(code)]) ;
  len = 	(rtx_length[(int)(code)]) ;
  for (i = 0; i < len; i++)
    {
      newpos[depth] = '0' + i;
      if (fmt[i] == 'e' || fmt[i] == 'u')
	new = add_to_sequence (((pattern)->fld[ i].rtx) , new, newpos);
      else if (fmt[i] == 'i' && i == 0)
	{
	  this->test_elt_zero_int = 1;
	  this->elt_zero_int = ((pattern)->fld[ i].rtint) ;
	}
      else if (fmt[i] == 'i' && i == 1)
	{
	  this->test_elt_one_int = 1;
	  this->elt_one_int = ((pattern)->fld[ i].rtint) ;
	}
      else if (fmt[i] == 'E')
	{
	  register int j;

	  if (i != 0)
	    abort ();
	  this->veclen = ((pattern)->fld[ i].rtvec->num_elem) ;
	  for (j = 0; j < ((pattern)->fld[ i].rtvec->num_elem) ; j++)
	    {
	      newpos[depth] = 'a' + j;
	      new = add_to_sequence (((pattern)->fld[ i].rtvec->elem[ j].rtx) ,
				     new, newpos);
	    }
	}
      else if (fmt[i] != '0')
	abort ();
    }
  return new;
}

struct decision *
merge_trees (old, add)
     register struct decision *old, *add;
{
  while (add)
    {
      register struct decision *next = add->next;
      add->next = 0;
      if (!try_merge_1 (old, add))
	old = try_merge_2 (old, add);
      add = next;
    }
  return old;
}

int
try_merge_1 (old, add)
     register struct decision *old, *add;
{
  while (old)
    {
      if ((old->position == add->position
	   || (old->position && add->position
	       && !strcmp (old->position, add->position)))
	  && (old->tests == add->tests
	      || (old->tests && add->tests && !strcmp (old->tests, add->tests)))
	  && (old->c_test == add->c_test
	      || (old->c_test && add->c_test && !strcmp (old->c_test, add->c_test)))
	  && old->test_elt_zero_int == add->test_elt_zero_int
	  && old->elt_zero_int == add->elt_zero_int
	  && old->test_elt_one_int == add->test_elt_one_int
	  && old->elt_one_int == add->elt_one_int
	  && old->veclen == add->veclen
	  && old->dupno == add->dupno
	  && old->opno == add->opno
	  && (old->tests == 0
	      || (add->enforce_mode ? no_same_mode (old) : old->next == 0))
	  && old->code == add->code
	  && old->mode == add->mode)
	{
	  old->success = merge_trees (old->success, add->success);
	  if (old->insn_code_number >= 0 && add->insn_code_number >= 0)
	    fatal ("Two actions at one point in tree.");
	  if (old->insn_code_number == -1)
	    old->insn_code_number = add->insn_code_number;
	  return 1;
	}
      old = old->next;
    }
  return 0;
}

struct decision *
try_merge_2 (old, add)
     struct decision *old, *add;
{
  register struct decision *p;
  struct decision *last = 0;
  struct decision *last_same_place = 0;

  int operand = 0 != add->tests;

  for (p = old; p; p = p->next)
    {
      if (p->position == add->position
	  || (p->position && add->position
	      && !strcmp (p->position, add->position)))
	{
	  last_same_place = p;
	  if (p->enforce_mode && (int) add->mode < (int) p->mode)
	    break;

	}

      else if (last_same_place)
	break;
      last = p;
    }

  if (last)
    {
      add->next = last->next;
      last->next = add;
      return old;
    }

  add->next = old;
  return add;
}

int
no_same_mode (node)
     struct decision *node;
{
  register struct decision *p;
  register enum machine_mode mode = node->mode;

  for (p = node->next; p; p = p->next)
    if (p->mode == mode)
      return 0;

  return 1;
}

int
break_out_subroutines (node)
     struct decision *node;
{
  int size = 0;
  struct decision *sub;
  for (sub = node; sub; sub = sub->next)
    size += 1 + break_out_subroutines (sub->success);
  if (size > 50 )
    {
      node->subroutine_number = ++next_subroutine_number;
      write_subroutine (node);
      size = 1;
    }
  return size;
}

void
write_subroutine (tree)
     struct decision *tree;
{
  printf ("int\nrecog_%d (x0, insn)\n     register rtx x0;\n     rtx insn;\n{\n",
	  tree->subroutine_number);
  printf ("  register rtx x1, x2, x3, x4, x5;\n  rtx x6, x7, x8, x9, x10, x11;\n");
  printf ("  int tem;\n");
  write_tree (tree, "", 0, "", 1);
  printf (" ret0: return -1;\n}\n\n");
}

void
write_tree (tree, prevpos, afterward, afterpos, initial)
     struct decision *tree;
     char *prevpos;
     int afterward;
     char *afterpos;
     int initial;
{
  register struct decision *p, *p1;
  char *pos;
  register int depth;
  int ignmode;
  enum anon1 { NO_SWITCH, CODE_SWITCH, MODE_SWITCH } in_switch = NO_SWITCH;
  char modemap[(int) MAX_MACHINE_MODE ];
  char codemap[((int)LAST_AND_UNUSED_RTX_CODE) ];

  pos = prevpos;

  if (tree->subroutine_number > 0 && ! initial)
    {
      printf (" L%d:\n", tree->number);

      if (afterward)
	{
	  printf ("  tem = recog_%d (x0, insn);\n",
		  tree->subroutine_number);
	  printf ("  if (tem >= 0) return tem;\n");
	  change_state (pos, afterpos);
	  printf ("  goto L%d;\n", afterward);
	}
      else
	printf ("  return recog_%d (x0, insn);\n",
		tree->subroutine_number);
      return;
    }

  tree->label_needed = 1;
  for (p = tree; p; p = p->next)
    {

      for (p1 = p->next; p1; p1 = p1->next)
	if (((p->code == UNKNOWN || p1->code == UNKNOWN || p->code == p1->code)
	     && (p->mode == VOIDmode || p1->mode == VOIDmode
		 || p->mode == p1->mode
		 || (in_switch != MODE_SWITCH && (p->tests || p1->tests))))
	    || strcmp (p1->position, p->position))
	  break;
      p->afterward = p1;
      if (p1) p1->label_needed = 1;

      if (in_switch == MODE_SWITCH
	  && (p->mode == VOIDmode || (! p->enforce_mode && p->tests != 0)))
	{
	  in_switch = NO_SWITCH;
	  printf ("  }\n");
	}
      if (in_switch == CODE_SWITCH && p->code == UNKNOWN)
	{
	  in_switch = NO_SWITCH;
	  printf ("  }\n");
	}

      if (p->label_needed)
	printf (" L%d:\n", p->number);

      if (p->success == 0 && p->insn_code_number < 0)
	abort ();

      change_state (pos, p->position);
      pos = p->position;
      depth = strlen (pos);

      ignmode = p->ignmode || pos[depth - 1] == '*' || p->tests;

      if (in_switch == NO_SWITCH)
	{

	  if (!ignmode && p->mode != VOIDmode && p->next && same_modes (p, p->mode))
	    {
	      printf ("  if (GET_MODE (x%d) != %smode)\n",
		      depth, 	(mode_name[(int)(p->mode)]) );
	      if (afterward)
		{
		  printf ("    {\n    ");
		  change_state (pos, afterpos);
		  printf ("      goto L%d;\n    }\n", afterward);
		}
	      else
		printf ("    goto ret0;\n");
	      clear_modes (p);
	      ignmode = 1;
	    }

	  if (p->code != UNKNOWN && p->next && same_codes (p, p->code))
	    {
	      printf ("  if (GET_CODE (x%d) != ", depth);
	      print_code (p->code);
	      printf (")\n");
	      if (afterward)
		{
		  printf ("    {");
		  change_state (pos, afterpos);
		  printf ("    goto L%d; }\n", afterward);
		}
	      else
		printf ("    goto ret0;\n");
	      clear_codes (p);
	    }
	}

      if (in_switch == NO_SWITCH && pos[depth-1] != '*')
	{
	  register int i;
	  int lose = 0;

	  mybzero (modemap, sizeof modemap);
	  for (p1 = p, i = 0;
	       (p1 && p1->mode != VOIDmode
		&& (p1->tests == 0 || p1->enforce_mode));
	       p1 = p1->next, i++)
	    {
	      if (! p->enforce_mode && modemap[(int) p1->mode])
		{
		  lose = 1;
		  break;
		}
	      modemap[(int) p1->mode] = 1;
	    }
	  if (!lose && i >= 4)
	    {
	      in_switch = MODE_SWITCH;
	      printf (" switch (GET_MODE (x%d))\n  {\n", depth);
	    }
	}

      if (in_switch == NO_SWITCH)
	{
	  register int i;
	  mybzero (codemap, sizeof codemap);
	  for (p1 = p, i = 0; p1 && p1->code != UNKNOWN; p1 = p1->next, i++)
	    {
	      if (codemap[(int) p1->code])
		break;
	      codemap[(int) p1->code] = 1;
	    }
	  if ((p1 == 0 || p1->code == UNKNOWN) && i >= 4)
	    {
	      in_switch = CODE_SWITCH;
	      printf (" switch (GET_CODE (x%d))\n  {\n", depth);
	    }
	}

      if (in_switch == MODE_SWITCH)
	{
	  if (modemap[(int) p->mode])
	    {
	      printf ("  case %smode:\n", 	(mode_name[(int)(p->mode)]) );
	      modemap[(int) p->mode] = 0;
	    }
	}
      if (in_switch == CODE_SWITCH)
	{
	  if (codemap[(int) p->code])
	    {
	      printf ("  case ");
	      print_code (p->code);
	      printf (":\n");
	      codemap[(int) p->code] = 0;
	    }
	}

      printf ("  if (");
      if (p->exact || (p->code != UNKNOWN && in_switch != CODE_SWITCH))
	{
	  if (p->exact)
	    printf ("x%d == %s", depth, p->exact);
	  else
	    {
	      printf ("GET_CODE (x%d) == ", depth);
	      print_code (p->code);
	    }
	  printf (" && ");
	}
      if (p->mode != VOIDmode && !ignmode && in_switch != MODE_SWITCH)
	printf ("GET_MODE (x%d) == %smode && ",
		depth, 	(mode_name[(int)(p->mode)]) );
      if (p->test_elt_zero_int)
	printf ("XINT (x%d, 0) == %d && ", depth, p->elt_zero_int);
      if (p->veclen)
	printf ("XVECLEN (x%d, 0) == %d && ", depth, p->veclen);
      if (p->test_elt_one_int)
	printf ("XINT (x%d, 1) == %d && ", depth, p->elt_one_int);
      if (p->dupno >= 0)
	printf ("rtx_equal_p (x%d, recog_operand[%d]) && ", depth, p->dupno);
      if (p->tests)
	printf ("%s (x%d, %smode)", p->tests, depth,
			(mode_name[(int)(p->mode)]) );
      else
	printf ("1");

      if (p->opno >= 0)
	printf (")\n    { recog_operand[%d] = x%d; ",
		p->opno, depth);
      else
	printf (")\n    ");

      if (p->c_test)
	printf ("if (%s) ", p->c_test);

      if (p->insn_code_number >= 0)
	printf ("return %d;", p->insn_code_number);
      else
	printf ("goto L%d;", p->success->number);

      if (p->opno >= 0)
	printf (" }\n");
      else
	printf ("\n");

      if (in_switch == CODE_SWITCH)
	{

	  for (p1 = p->next; p1; p1 = p1->next)
	    if (p1->code == UNKNOWN || p->code == p1->code)
	      break;
	  if (p1 == 0 || p1->code == UNKNOWN)
	    printf ("  break;\n");
	  else if (p1 != p->next)
	    {
	      printf (" goto L%d;\n", p1->number);
	      p1->label_needed = 1;
	    }
	}

      if (in_switch == MODE_SWITCH)
	{

	  for (p1 = p->next; p1; p1 = p1->next)
	    if (p1->mode == VOIDmode || p->mode == p1->mode)
	      break;
	  if (p1 == 0 || p1->mode == VOIDmode)
	    printf ("  break;\n");
	  else if (p1 != p->next)
	    {
	      printf (" goto L%d;\n", p1->number);
	      p1->label_needed = 1;
	    }
	}
    }

  if (in_switch != NO_SWITCH)
    printf ("  }\n");

  if (afterward)
    {
      change_state (pos, afterpos);
      printf ("  goto L%d;\n", afterward);
    }
  else
    printf ("  goto ret0;\n");

  for (p = tree; p; p = p->next)
    if (p->success)
      {
	  {
	    pos = p->position;
	    write_tree (p->success, pos,
			p->afterward ? p->afterward->number : afterward,
			p->afterward ? pos : afterpos,
			0);
	  }
      }
}

void
print_code (code)
     enum rtx_code  code;
{
  register char *p1;
  for (p1 = 	(rtx_name[(int)(code)]) ; *p1; p1++)
    {
      if (*p1 >= 'a' && *p1 <= 'z')
	(--((&_iob[1]) )->_cnt >= 0 ?	(int)(*((&_iob[1]) )->_ptr++ = (unsigned char)((*p1 + 'A' - 'a'))) :	((((&_iob[1]) )->_flag & 0200 ) && -((&_iob[1]) )->_cnt < ((&_iob[1]) )->_bufsiz ?	((*((&_iob[1]) )->_ptr = (unsigned char)((*p1 + 'A' - 'a'))) != '\n' ?	(int)(*((&_iob[1]) )->_ptr++) :	_flsbuf(*(unsigned char *)((&_iob[1]) )->_ptr, (&_iob[1]) )) :	_flsbuf((unsigned char)((*p1 + 'A' - 'a')), (&_iob[1]) )))  ;
      else
	(--((&_iob[1]) )->_cnt >= 0 ?	(int)(*((&_iob[1]) )->_ptr++ = (unsigned char)((*p1))) :	((((&_iob[1]) )->_flag & 0200 ) && -((&_iob[1]) )->_cnt < ((&_iob[1]) )->_bufsiz ?	((*((&_iob[1]) )->_ptr = (unsigned char)((*p1))) != '\n' ?	(int)(*((&_iob[1]) )->_ptr++) :	_flsbuf(*(unsigned char *)((&_iob[1]) )->_ptr, (&_iob[1]) )) :	_flsbuf((unsigned char)((*p1)), (&_iob[1]) )))  ;
    }
}

int
same_codes (p, code)
     register struct decision *p;
     register enum rtx_code  code;
{
  for (; p; p = p->next)
    if (p->code != code)
      return 0;

  return 1;
}

void
clear_codes (p)
     register struct decision *p;
{
  for (; p; p = p->next)
    p->code = UNKNOWN;
}

int
same_modes (p, mode)
     register struct decision *p;
     register enum machine_mode mode;
{
  for (; p; p = p->next)
    if (p->mode != mode || p->tests)
      return 0;

  return 1;
}

void
clear_modes (p)
     register struct decision *p;
{
  for (; p; p = p->next)
    p->ignmode = 1;
}
void
change_state (oldpos, newpos)
     char *oldpos;
     char *newpos;
{
  int odepth = strlen (oldpos);
  int depth = odepth;
  int ndepth = strlen (newpos);

  while (strncmp (oldpos, newpos, depth))
    --depth;

  while (depth < ndepth)
    {
      if (newpos[depth] == '*')
	printf ("  x%d = recog_addr_dummy;\n  XEXP (x%d, 0) = x%d;\n",
		depth + 1, depth + 1, depth);
      else if (newpos[depth] >= 'a' && newpos[depth] <= 'z')
	printf ("  x%d = XVECEXP (x%d, 0, %d);\n",
		depth + 1, depth, newpos[depth] - 'a');
      else
	printf ("  x%d = XEXP (x%d, %c);\n",
		depth + 1, depth, newpos[depth]);
      ++depth;
    }
}
char *
copystr (s1)
     char *s1;
{
  register char *tem;

  if (s1 == 0)
    return 0;

  tem = (char *) xmalloc (strlen (s1) + 1);
  strcpy (tem, s1);

  return tem;
}

void
mybzero (b, length)
     register char *b;
     register int length;
{
  while (length-- > 0)
    *b++ = 0;
}

char *
concat (s1, s2)
     char *s1, *s2;
{
  register char *tem;

  if (s1 == 0)
    return s2;
  if (s2 == 0)
    return s1;

  tem = (char *) xmalloc (strlen (s1) + strlen (s2) + 2);
  strcpy (tem, s1);
  strcat (tem, " ");
  strcat (tem, s2);

  return tem;
}

int
xrealloc (ptr, size)
     char *ptr;
     int size;
{
  int result = realloc (ptr, size);
  if (!result)
    fatal ("virtual memory exhausted");
  return result;
}

int
xmalloc (size)
{
  register int val = malloc (size);

  if (val == 0)
    fatal ("virtual memory exhausted");
  return val;
}

void
fatal (s, a1, a2)
{
  fprintf ((&_iob[2]) , "genrecog: ");
  fprintf ((&_iob[2]) , s, a1, a2);
  fprintf ((&_iob[2]) , "\n");
  fprintf ((&_iob[2]) , "after %d instruction definitions\n",
	   next_insn_code);
  exit (33 );
}
int
main (argc, argv)
     int argc;
     char **argv;
{
  rtx desc;
  struct decision *tree = 0;
  struct _iobuf  *infile;
  extern rtx read_rtx ();
  register int c;

  _obstack_begin ((rtl_obstack), 0, 0, xmalloc , free ) ;

  if (argc <= 1)
    fatal ("No input file name.");

  infile = fopen (argv[1], "r");
  if (infile == 0)
    {
      perror (argv[1]);
      exit (33 );
    }

  init_rtl ();
  next_insn_code = 0;

  printf ("/* Generated automatically by the program `genrecog'\nfrom the machine description file `md'.  */\n\n");

  while (1)
    {
      c = read_skip_spaces (infile);
      if (c == (-1) )
	break;
      ungetc (c, infile);

      desc = read_rtx (infile);
      if (	((desc)->code)  == DEFINE_INSN)
	tree = merge_trees (tree, make_insn_sequence (desc));
      if (	((desc)->code)  == DEFINE_PEEPHOLE
	  || 	((desc)->code)  == DEFINE_EXPAND)
	next_insn_code++;
    }

  printf ("#include \"config.h\"\n");
  printf ("#include \"rtl.h\"\n");
  printf ("#include \"insn-config.h\"\n");
  printf ("#include \"recog.h\"\n");
  printf ("\n/* `recog' contains a decision tree\n   that recognizes whether the rtx X0 is a valid instruction.\n\n   recog returns -1 if the rtx is not valid.\n   If the rtx is valid, recog returns a nonnegative number\n   which is the insn code number for the pattern that matched.\n");

  printf ("   This is the same as the order in the machine description of\n   the entry that matched.  This number can be used as an index into\n   insn_templates and insn_n_operands (found in insn-output.c)\n   or as an argument to output_insn_hairy (also in insn-output.c).  */\n\n");

  printf ("rtx recog_operand[MAX_RECOG_OPERANDS];\n\n");
  printf ("rtx *recog_operand_loc[MAX_RECOG_OPERANDS];\n\n");
  printf ("rtx *recog_dup_loc[MAX_DUP_OPERANDS];\n\n");
  printf ("char recog_dup_num[MAX_DUP_OPERANDS];\n\n");
  printf ("extern rtx recog_addr_dummy;\n\n");
  printf ("#define operands recog_operand\n\n");

  break_out_subroutines (tree);

  printf ("int\nrecog (x0, insn)\n     register rtx x0;\n     rtx insn;\n{\n");
  printf ("  register rtx x1, x2, x3, x4, x5;\n  rtx x6, x7, x8, x9, x10, x11;\n");
  printf ("  int tem;\n");

  write_tree (tree, "", 0, "", 1);
  printf (" ret0: return -1;\n}\n");

  fflush ((&_iob[1]) );
  exit (((((&_iob[1]) )->_flag&040 )!=0)  != 0 ? 33  : 0 );
}

