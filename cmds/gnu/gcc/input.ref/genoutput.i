
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

void fatal ();
void error ();
void mybcopy ();
void mybzero ();

int next_code_number;

struct data
{
  int code_number;
  char *name;
  char *template;		 
  int n_operands;		 
  int n_dups;			 
  int n_alternatives;		 
  struct data *next;
  char *constraints[40 ];
  int op_n_alternatives[40 ];
  char *predicates[40 ];
  char address_p[40 ];
  enum machine_mode modes[40 ];
  char strict_low[40 ];
  char outfun;			 
  char *machine_info;		 
};

struct data *insn_data;

struct data *end_of_insn_data;

int have_constraints;
void
output_prologue ()
{

  printf ("/* Generated automatically by the program `genoutput'\nfrom the machine description file `md'.  */\n\n");

  printf ("#include \"config.h\"\n");
  printf ("#include \"rtl.h\"\n");
  printf ("#include \"regs.h\"\n");
  printf ("#include \"hard-reg-set.h\"\n");
  printf ("#include \"real.h\"\n");
  printf ("#include \"conditions.h\"\n");
  printf ("#include \"insn-flags.h\"\n");
  printf ("#include \"insn-config.h\"\n\n");

  printf ("#ifndef __STDC__\n");
  printf ("#define const\n");
  printf ("#endif\n\n");

  printf ("#include \"output.h\"\n");
  printf ("#include \"aux-output.c\"\n\n");

  printf ("#ifndef INSN_MACHINE_INFO\n");
  printf ("#define INSN_MACHINE_INFO struct dummy1 {int i;}\n");
  printf ("#endif\n\n");
}

void
output_epilogue ()
{
  register struct data *d;

  printf ("\nchar * const insn_template[] =\n  {\n");
  for (d = insn_data; d; d = d->next)
    {
      if (d->template)
	printf ("    \"%s\",\n", d->template);
      else
	printf ("    0,\n");
    }
  printf ("  };\n");

  printf ("\nchar *(*const insn_outfun[])() =\n  {\n");
  for (d = insn_data; d; d = d->next)
    {
      if (d->outfun)
	printf ("    output_%d,\n", d->code_number);
      else
	printf ("    0,\n");
    }
  printf ("  };\n");

  printf ("\nrtx (*const insn_gen_function[]) () =\n  {\n");
  for (d = insn_data; d; d = d->next)
    {
      if (d->name)
	printf ("    gen_%s,\n", d->name);
      else
	printf ("    0,\n");
    }
  printf ("  };\n");

  printf ("\nconst int insn_n_operands[] =\n  {\n");
  for (d = insn_data; d; d = d->next)
    {
      printf ("    %d,\n", d->n_operands);
    }
  printf ("  };\n");

  printf ("\nconst int insn_n_dups[] =\n  {\n");
  for (d = insn_data; d; d = d->next)
    {
      printf ("    %d,\n", d->n_dups);
    }
  printf ("  };\n");

  if (have_constraints)
    {
      printf ("\nchar *const insn_operand_constraint[][MAX_RECOG_OPERANDS] =\n  {\n");
      for (d = insn_data; d; d = d->next)
	{
	  register int i, n = 0, start;
	  printf ("    {");

	  for (start = 0; start < d->n_operands; start++)
	    if (d->op_n_alternatives[start] > 0)
	      {
		if (n == 0)
		  n = d->op_n_alternatives[start];
		else if (n != d->op_n_alternatives[start])
		  error ("wrong number of alternatives in operand %d of insn number %d",
			 start, d->code_number);
	      }
	  d->n_alternatives = n;

	  for (i = 0; i < d->n_operands; i++)
	    {
	      if (d->constraints[i] == 0)
		printf (" \"\",");
	      else
		printf (" \"%s\",", d->constraints[i]);
	    }
	  if (d->n_operands == 0)
	    printf (" 0");
	  printf (" },\n");
	}
      printf ("  };\n");
    }
  else
    {
      printf ("\nconst char insn_operand_address_p[][MAX_RECOG_OPERANDS] =\n  {\n");
      for (d = insn_data; d; d = d->next)
	{
	  register int i;
	  printf ("    {");
	  for (i = 0; i < d->n_operands; i++)
	    printf (" %d,", d->address_p[i]);
	  if (d->n_operands == 0)
	    printf (" 0");
	  printf (" },\n");
	}
      printf ("  };\n");
    }

  printf ("\nconst enum machine_mode insn_operand_mode[][MAX_RECOG_OPERANDS] =\n  {\n");
  for (d = insn_data; d; d = d->next)
    {
      register int i;
      printf ("    {");
      for (i = 0; i < d->n_operands; i++)
	printf (" %smode,", 	(mode_name[(int)(d->modes[i])]) );
      if (d->n_operands == 0)
	printf (" VOIDmode");
      printf (" },\n");
    }
  printf ("  };\n");

  printf ("\nconst char insn_operand_strict_low[][MAX_RECOG_OPERANDS] =\n  {\n");
  for (d = insn_data; d; d = d->next)
    {
      register int i;
      printf ("    {");
      for (i = 0; i < d->n_operands; i++)
	printf (" %d,", d->strict_low[i]);
      if (d->n_operands == 0)
	printf (" 0");
      printf (" },\n");
    }
  printf ("  };\n");

  printf ("\nint (*const insn_operand_predicate[][MAX_RECOG_OPERANDS])() =\n  {\n");
  for (d = insn_data; d; d = d->next)
    {
      register int i;
      printf ("    {");
      for (i = 0; i < d->n_operands; i++)
	printf (" %s,", ((d->predicates[i] && d->predicates[i][0])
			 ? d->predicates[i] : "0"));
      if (d->n_operands == 0)
	printf (" 0");
      printf (" },\n");
    }
  printf ("  };\n");

  printf ("\nconst INSN_MACHINE_INFO insn_machine_info[] =\n  {\n");
  for (d = insn_data; d; d = d->next)
    {
      if (d->machine_info)
	printf ("    {%s},\n", d->machine_info);
      else
	printf("     {0},\n");
    }
  printf("  };\n");

  printf ("\nconst int insn_n_alternatives[] =\n  {\n");
  for (d = insn_data; d; d = d->next)
    {
      if (d->n_alternatives)
	printf ("    %d,\n", d->n_alternatives);
      else
	printf("     0,\n");
    }
  printf("  };\n");
}

int max_opno;
int num_dups;
char *constraints[40 ];
int op_n_alternatives[40 ];
char *predicates[40 ];
char address_p[40 ];
enum machine_mode modes[40 ];
char strict_low[40 ];

void
scan_operands (part, this_address_p, this_strict_low)
     rtx part;
     int this_address_p;
     int this_strict_low;
{
  register int i, j;
  register enum rtx_code  code;
  register char *format_ptr;

  if (part == 0)
    return;

  code = 	((part)->code) ;

  if (code == MATCH_OPERAND)
    {
      int opno = ((part)->fld[ 0].rtint) ;
      if (opno > max_opno)
	max_opno = opno;
      if (max_opno >= 40 )
	error ("Too many operands (%d) in one instruction pattern.\n",
	       max_opno + 1);
      modes[opno] = 	((part)->mode) ;
      strict_low[opno] = this_strict_low;
      predicates[opno] = ((part)->fld[ 1].rtstr) ;
      constraints[opno] = ((part)->fld[ 2].rtstr) ;
      if (((part)->fld[ 2].rtstr)  != 0 && *((part)->fld[ 2].rtstr)  != 0)
	{
	  op_n_alternatives[opno] = n_occurrences (',', ((part)->fld[ 2].rtstr) ) + 1;
	  have_constraints = 1;
	}
      address_p[opno] = this_address_p;
      return;
    }

  if (code == MATCH_OPERATOR)
    {
      int opno = ((part)->fld[ 0].rtint) ;
      if (opno > max_opno)
	max_opno = opno;
      if (max_opno >= 40 )
	error ("Too many operands (%d) in one instruction pattern.\n",
	       max_opno + 1);
      modes[opno] = 	((part)->mode) ;
      strict_low[opno] = 0;
      predicates[opno] = ((part)->fld[ 1].rtstr) ;
      constraints[opno] = 0;
      address_p[opno] = 0;
      for (i = 0; i < ((part)->fld[ 2].rtvec->num_elem) ; i++)
	scan_operands (((part)->fld[ 2].rtvec->elem[ i].rtx) , 0, 0);
      return;
    }

  if (code == MATCH_DUP)
    {
      ++num_dups;
      return;
    }

  if (code == ADDRESS)
    {
      scan_operands (((part)->fld[ 0].rtx) , 1, 0);
      return;
    }

  if (code == STRICT_LOW_PART)
    {
      scan_operands (((part)->fld[ 0].rtx) , 0, 1);
      return;
    }

  format_ptr = 	(rtx_format[(int)(	((part)->code) )]) ;

  for (i = 0; i < 	(rtx_length[(int)(	((part)->code) )]) ; i++)
    switch (*format_ptr++)
      {
      case 'e':
	scan_operands (((part)->fld[ i].rtx) , 0, 0);
	break;
      case 'E':
	if (((part)->fld[ i].rtvec)  != 0 )
	  for (j = 0; j < ((part)->fld[ i].rtvec->num_elem) ; j++)
	    scan_operands (((part)->fld[ i].rtvec->elem[ j].rtx) , 0, 0);
	break;
      }
}

void
gen_insn (insn)
     rtx insn;
{
  register struct data *d = (struct data *) xmalloc (sizeof (struct data));
  register int i;

  d->code_number = next_code_number++;
  if (((insn)->fld[ 0].rtstr) [0])
    d->name = ((insn)->fld[ 0].rtstr) ;
  else
    d->name = 0;

  d->next = 0;
  if (end_of_insn_data)
    end_of_insn_data->next = d;
  else
    insn_data = d;

  end_of_insn_data = d;

  max_opno = -1;
  num_dups = 0;

  mybzero (constraints, sizeof constraints);
  mybzero (op_n_alternatives, sizeof op_n_alternatives);
  mybzero (predicates, sizeof predicates);
  mybzero (address_p, sizeof address_p);
  mybzero (modes, sizeof modes);
  mybzero (strict_low, sizeof strict_low);
  for (i = 0; i < ((insn)->fld[ 1].rtvec->num_elem) ; i++)
    scan_operands (((insn)->fld[ 1].rtvec->elem[ i].rtx) , 0, 0);
  d->n_operands = max_opno + 1;
  d->n_dups = num_dups;
  mybcopy (constraints, d->constraints, sizeof constraints);
  mybcopy (op_n_alternatives, d->op_n_alternatives, sizeof op_n_alternatives);
  mybcopy (predicates, d->predicates, sizeof predicates);
  mybcopy (address_p, d->address_p, sizeof address_p);
  mybcopy (modes, d->modes, sizeof modes);
  mybcopy (strict_low, d->strict_low, sizeof strict_low);
  d->machine_info = ((insn)->fld[ 4].rtstr) ;

  if (((insn)->fld[ 3].rtstr) [0] != '*')
    {
      d->template = ((insn)->fld[ 3].rtstr) ;
      d->outfun = 0;
      return;
    }

  d->template = 0;
  d->outfun = 1;

  printf ("\nchar *\n");
  printf ("output_%d (operands, insn)\n", d->code_number);
  printf ("     rtx *operands;\n");
  printf ("     rtx insn;\n");
  printf ("{\n");

  {
    register char *cp = &(((insn)->fld[ 3].rtstr) [1]);
    while (*cp) (--((&_iob[1]) )->_cnt >= 0 ?	(int)(*((&_iob[1]) )->_ptr++ = (unsigned char)((*cp++))) :	((((&_iob[1]) )->_flag & 0200 ) && -((&_iob[1]) )->_cnt < ((&_iob[1]) )->_bufsiz ?	((*((&_iob[1]) )->_ptr = (unsigned char)((*cp++))) != '\n' ?	(int)(*((&_iob[1]) )->_ptr++) :	_flsbuf(*(unsigned char *)((&_iob[1]) )->_ptr, (&_iob[1]) )) :	_flsbuf((unsigned char)((*cp++)), (&_iob[1]) )))  ;
    (--((&_iob[1]) )->_cnt >= 0 ?	(int)(*((&_iob[1]) )->_ptr++ = (unsigned char)(('\n'))) :	((((&_iob[1]) )->_flag & 0200 ) && -((&_iob[1]) )->_cnt < ((&_iob[1]) )->_bufsiz ?	((*((&_iob[1]) )->_ptr = (unsigned char)(('\n'))) != '\n' ?	(int)(*((&_iob[1]) )->_ptr++) :	_flsbuf(*(unsigned char *)((&_iob[1]) )->_ptr, (&_iob[1]) )) :	_flsbuf((unsigned char)(('\n')), (&_iob[1]) )))  ;
  }
  printf ("}\n");
}

void
gen_peephole (peep)
     rtx peep;
{
  register struct data *d = (struct data *) xmalloc (sizeof (struct data));
  register int i;

  d->code_number = next_code_number++;
  d->name = 0;

  d->next = 0;
  if (end_of_insn_data)
    end_of_insn_data->next = d;
  else
    insn_data = d;

  end_of_insn_data = d;

  max_opno = -1;
  mybzero (constraints, sizeof constraints);
  mybzero (op_n_alternatives, sizeof op_n_alternatives);

  for (i = 0; i < ((peep)->fld[ 0].rtvec->num_elem) ; i++)
    scan_operands (((peep)->fld[ 0].rtvec->elem[ i].rtx) , 0, 0);

  d->n_operands = max_opno + 1;
  d->n_dups = 0;
  mybcopy (constraints, d->constraints, sizeof constraints);
  mybcopy (op_n_alternatives, d->op_n_alternatives, sizeof op_n_alternatives);
  mybzero (d->predicates, sizeof predicates);
  mybzero (d->address_p, sizeof address_p);
  mybzero (d->modes, sizeof modes);
  mybzero (d->strict_low, sizeof strict_low);
  d->machine_info = ((peep)->fld[ 3].rtstr) ;

  if (((peep)->fld[ 2].rtstr) [0] != '*')
    {
      d->template = ((peep)->fld[ 2].rtstr) ;
      d->outfun = 0;
      return;
    }

  d->template = 0;
  d->outfun = 1;

  printf ("\nchar *\n");
  printf ("output_%d (operands, insn)\n", d->code_number);
  printf ("     rtx *operands;\n");
  printf ("     rtx insn;\n");
  printf ("{\n");
  printf ("%s\n", &(((peep)->fld[ 2].rtstr) [1]));
  printf ("}\n");
}

void
gen_expand (insn)
     rtx insn;
{
  register struct data *d = (struct data *) xmalloc (sizeof (struct data));
  register int i;

  d->code_number = next_code_number++;
  if (((insn)->fld[ 0].rtstr) [0])
    d->name = ((insn)->fld[ 0].rtstr) ;
  else
    d->name = 0;

  d->next = 0;
  if (end_of_insn_data)
    end_of_insn_data->next = d;
  else
    insn_data = d;

  end_of_insn_data = d;

  max_opno = -1;
  num_dups = 0;

  mybzero (predicates, sizeof predicates);
  mybzero (modes, sizeof modes);
  if (((insn)->fld[ 1].rtvec) )
    for (i = 0; i < ((insn)->fld[ 1].rtvec->num_elem) ; i++)
      scan_operands (((insn)->fld[ 1].rtvec->elem[ i].rtx) , 0, 0);
  d->n_operands = max_opno + 1;
  mybcopy (predicates, d->predicates, sizeof predicates);
  mybcopy (modes, d->modes, sizeof modes);

  mybzero (d->constraints, sizeof constraints);
  mybzero (d->op_n_alternatives, sizeof op_n_alternatives);
  mybzero (d->address_p, sizeof address_p);
  mybzero (d->strict_low, sizeof strict_low);

  d->n_dups = 0;
  d->template = 0;
  d->outfun = 0;
  d->machine_info = 0;
}
int
xmalloc (size)
{
  register int val = malloc (size);

  if (val == 0)
    fatal ("virtual memory exhausted");
  return val;
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

void
mybzero (b, length)
     register char *b;
     register int length;
{
  while (length-- > 0)
    *b++ = 0;
}

void
mybcopy (b1, b2, length)
     register char *b1;
     register char *b2;
     register int length;
{
  while (length-- > 0)
    *b2++ = *b1++;
}

void
fatal (s, a1, a2)
{
  fprintf ((&_iob[2]) , "genoutput: ");
  fprintf ((&_iob[2]) , s, a1, a2);
  fprintf ((&_iob[2]) , "\n");
  exit (33 );
}

void
error (s, a1, a2)
{
  fprintf ((&_iob[2]) , "genoutput: ");
  fprintf ((&_iob[2]) , s, a1, a2);
  fprintf ((&_iob[2]) , "\n");
}
int
main (argc, argv)
     int argc;
     char **argv;
{
  rtx desc;
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

  output_prologue ();
  next_code_number = 0;
  have_constraints = 0;

  while (1)
    {
      c = read_skip_spaces (infile);
      if (c == (-1) )
	break;
      ungetc (c, infile);

      desc = read_rtx (infile);
      if (	((desc)->code)  == DEFINE_INSN)
	gen_insn (desc);
      if (	((desc)->code)  == DEFINE_PEEPHOLE)
	gen_peephole (desc);
      if (	((desc)->code)  == DEFINE_EXPAND)
	gen_expand (desc);
    }

  output_epilogue ();

  fflush ((&_iob[1]) );
  exit (((((&_iob[1]) )->_flag&040 )!=0)  != 0 ? 33  : 0 );
}

int
n_occurrences (c, s)
     char c;
     char *s;
{
  int n = 0;
  while (*s)
    n += (*s++ == c);
  return n;
}

