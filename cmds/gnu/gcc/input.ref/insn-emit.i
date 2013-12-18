
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

enum expand_modifier {EXPAND_NORMAL, EXPAND_SUM, EXPAND_CONST_ADDRESS};

extern int cse_not_expected;

extern rtx save_expr_regs;

enum direction {none, upward, downward};   

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

extern double ldexp ();

extern double atof ();

union real_extract 
{
  double  d;
  int i[sizeof (double ) / sizeof (int)];
};

extern char *insn_operand_constraint[][5 ];

extern rtx recog_operand[];

rtx
gen_tstsi (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, cc0_rtx, operand0);
}

rtx
gen_tsthi (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, cc0_rtx, operand0);
}

rtx
gen_tstqi (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, cc0_rtx, operand0);
}

rtx
gen_tstsf (operand0)
     rtx operand0;
{
  rtx recog_operand [1];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;

{
  if ((target_flags & 0100) )
    {
      emit_insn (gen_rtx (PARALLEL, VOIDmode,
		          gen_rtvec (2,
				     gen_rtx (SET, VOIDmode,
					      cc0_rtx, recog_operand [0]),
				     gen_rtx (CLOBBER, VOIDmode,
					      gen_reg_rtx (SImode)))));
      goto _done ;
    }
}
  operand0 = recog_operand [0];
  emit_insn (gen_rtx (SET, VOIDmode, cc0_rtx, operand0));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_tstdf (operand0)
     rtx operand0;
{
  rtx recog_operand [1];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;

{
  if ((target_flags & 0100) )
    {
      emit_insn (gen_rtx (PARALLEL, VOIDmode,
			  gen_rtvec (2, gen_rtx (SET, VOIDmode,
						 cc0_rtx, recog_operand [0]),
				     gen_rtx (CLOBBER, VOIDmode,
					      gen_reg_rtx (SImode)))));
      goto _done ;
    }
}
  operand0 = recog_operand [0];
  emit_insn (gen_rtx (SET, VOIDmode, cc0_rtx, operand0));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_cmpsi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, cc0_rtx, gen_rtx (COMPARE, VOIDmode, operand0, operand1));
}

rtx
gen_cmphi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, cc0_rtx, gen_rtx (COMPARE, VOIDmode, operand0, operand1));
}

rtx
gen_cmpqi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, cc0_rtx, gen_rtx (COMPARE, VOIDmode, operand0, operand1));
}

rtx
gen_cmpdf (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx recog_operand [2];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;

{
  if ((target_flags & 0100) )
    {
      rtx set = gen_rtx (SET, VOIDmode, cc0_rtx,
			 gen_rtx (COMPARE, VOIDmode, recog_operand [0], recog_operand [1]));
      emit_insn (gen_rtx (PARALLEL, VOIDmode,
		          gen_rtvec (2, set,
				     gen_rtx (CLOBBER, VOIDmode,
					      gen_reg_rtx (SImode)))));
      goto _done ;
    }
}
  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  emit_insn (gen_rtx (SET, VOIDmode, cc0_rtx, gen_rtx (COMPARE, VOIDmode, operand0, operand1)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_cmpsf (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx recog_operand [2];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;

{
  if ((target_flags & 0100) )
    {
      rtx set = gen_rtx (SET, VOIDmode, cc0_rtx,
			 gen_rtx (COMPARE, VOIDmode, recog_operand [0], recog_operand [1]));
      emit_insn (gen_rtx (PARALLEL, VOIDmode,
			  gen_rtvec (2, set,
				     gen_rtx (CLOBBER, VOIDmode,
					      gen_reg_rtx(SImode)))));
      goto _done ;
    }
}
  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  emit_insn (gen_rtx (SET, VOIDmode, cc0_rtx, gen_rtx (COMPARE, VOIDmode, operand0, operand1)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_movsi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, operand1);
}

rtx
gen_movhi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, operand1);
}

rtx
gen_movstricthi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, gen_rtx (STRICT_LOW_PART, VOIDmode, operand0), operand1);
}

rtx
gen_movqi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, operand1);
}

rtx
gen_movstrictqi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, gen_rtx (STRICT_LOW_PART, VOIDmode, operand0), operand1);
}

rtx
gen_movsf (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, operand1);
}

rtx
gen_movdf (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, operand1);
}

rtx
gen_movdi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, operand1);
}

rtx
gen_pushasi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, operand1);
}

rtx
gen_truncsiqi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (TRUNCATE, QImode, operand1));
}

rtx
gen_trunchiqi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (TRUNCATE, QImode, operand1));
}

rtx
gen_truncsihi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (TRUNCATE, HImode, operand1));
}

rtx
gen_zero_extendhisi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx recog_operand [2];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;
recog_operand [1] = make_safe_from (recog_operand [1], recog_operand [0]);
  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, const0_rtx));
  emit_insn (gen_rtx (SET, VOIDmode, gen_rtx (STRICT_LOW_PART, VOIDmode, gen_rtx (SUBREG, HImode, operand0, 0)), operand1));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_zero_extendqihi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx recog_operand [2];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;
recog_operand [1] = make_safe_from (recog_operand [1], recog_operand [0]);
  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, const0_rtx));
  emit_insn (gen_rtx (SET, VOIDmode, gen_rtx (STRICT_LOW_PART, VOIDmode, gen_rtx (SUBREG, QImode, operand0, 0)), operand1));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_zero_extendqisi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx recog_operand [2];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;
 recog_operand [1] = make_safe_from (recog_operand [1], recog_operand [0]); 
  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, const0_rtx));
  emit_insn (gen_rtx (SET, VOIDmode, gen_rtx (STRICT_LOW_PART, VOIDmode, gen_rtx (SUBREG, QImode, operand0, 0)), operand1));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_extendhisi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (SIGN_EXTEND, SImode, operand1));
}

rtx
gen_extendqihi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (SIGN_EXTEND, HImode, operand1));
}

rtx
gen_extendqisi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (SIGN_EXTEND, SImode, operand1));
}

rtx
gen_extendsfdf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx recog_operand [2];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (FLOAT_EXTEND, DFmode, operand1)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_truncdfsf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx recog_operand [2];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (FLOAT_TRUNCATE, SFmode, operand1)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_floatsisf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx recog_operand [2];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (FLOAT, SFmode, operand1)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_floatsidf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx recog_operand [2];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (FLOAT, DFmode, operand1)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_floathisf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (FLOAT, SFmode, operand1));
}

rtx
gen_floathidf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (FLOAT, DFmode, operand1));
}

rtx
gen_floatqisf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (FLOAT, SFmode, operand1));
}

rtx
gen_floatqidf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (FLOAT, DFmode, operand1));
}

rtx
gen_ftruncdf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (FIX, DFmode, operand1));
}

rtx
gen_ftruncsf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (FIX, SFmode, operand1));
}

rtx
gen_fixsfqi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (FIX, QImode, operand1));
}

rtx
gen_fixsfhi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (FIX, HImode, operand1));
}

rtx
gen_fixsfsi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (FIX, SImode, operand1));
}

rtx
gen_fixdfqi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (FIX, QImode, operand1));
}

rtx
gen_fixdfhi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (FIX, HImode, operand1));
}

rtx
gen_fixdfsi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (FIX, SImode, operand1));
}

rtx
gen_fix_truncsfsi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (FIX, SImode, gen_rtx (FIX, SFmode, operand1)));
}

rtx
gen_fix_truncdfsi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (FIX, SImode, gen_rtx (FIX, DFmode, operand1)));
}

rtx
gen_addsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (PLUS, SImode, operand1, operand2));
}

rtx
gen_addhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (PLUS, HImode, operand1, operand2));
}

rtx
gen_addqi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (PLUS, QImode, operand1, operand2));
}

rtx
gen_adddf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx recog_operand [3];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;
  recog_operand [2] = operand2;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  operand2 = recog_operand [2];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (PLUS, DFmode, operand1, operand2)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_addsf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx recog_operand [3];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;
  recog_operand [2] = operand2;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  operand2 = recog_operand [2];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (PLUS, SFmode, operand1, operand2)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_subsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (MINUS, SImode, operand1, operand2));
}

rtx
gen_subhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (MINUS, HImode, operand1, operand2));
}

rtx
gen_subqi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (MINUS, QImode, operand1, operand2));
}

rtx
gen_subdf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx recog_operand [3];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;
  recog_operand [2] = operand2;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  operand2 = recog_operand [2];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (MINUS, DFmode, operand1, operand2)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_subsf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx recog_operand [3];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;
  recog_operand [2] = operand2;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  operand2 = recog_operand [2];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (MINUS, SFmode, operand1, operand2)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_mulhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (MULT, HImode, operand1, operand2));
}

rtx
gen_mulhisi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (MULT, SImode, operand1, operand2));
}

rtx
gen_mulsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (MULT, SImode, operand1, operand2));
}

rtx
gen_umulhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (UMULT, HImode, operand1, operand2));
}

rtx
gen_umulhisi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (UMULT, SImode, operand1, operand2));
}

rtx
gen_umulsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (UMULT, SImode, operand1, operand2));
}

rtx
gen_muldf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx recog_operand [3];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;
  recog_operand [2] = operand2;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  operand2 = recog_operand [2];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (MULT, DFmode, operand1, operand2)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_mulsf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx recog_operand [3];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;
  recog_operand [2] = operand2;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  operand2 = recog_operand [2];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (MULT, SFmode, operand1, operand2)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_divhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (DIV, HImode, operand1, operand2));
}

rtx
gen_divhisi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (DIV, HImode, operand1, operand2));
}

rtx
gen_divsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (DIV, SImode, operand1, operand2));
}

rtx
gen_udivhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (UDIV, HImode, operand1, operand2));
}

rtx
gen_udivhisi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (UDIV, HImode, operand1, operand2));
}

rtx
gen_udivsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (UDIV, SImode, operand1, operand2));
}

rtx
gen_divdf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx recog_operand [3];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;
  recog_operand [2] = operand2;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  operand2 = recog_operand [2];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (DIV, DFmode, operand1, operand2)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_divsf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx recog_operand [3];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;
  recog_operand [2] = operand2;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  operand2 = recog_operand [2];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (DIV, SFmode, operand1, operand2)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_modhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (MOD, HImode, operand1, operand2));
}

rtx
gen_modhisi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (MOD, HImode, operand1, operand2));
}

rtx
gen_umodhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (UMOD, HImode, operand1, operand2));
}

rtx
gen_umodhisi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (UMOD, HImode, operand1, operand2));
}

rtx
gen_divmodsi4 (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  return gen_rtx (PARALLEL, VOIDmode, gen_rtvec (2,
		gen_rtx (SET, VOIDmode, operand0, gen_rtx (DIV, SImode, operand1, operand2)),
		gen_rtx (SET, VOIDmode, operand3, gen_rtx (MOD, SImode, operand1, operand2))));
}

rtx
gen_udivmodsi4 (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  return gen_rtx (PARALLEL, VOIDmode, gen_rtvec (2,
		gen_rtx (SET, VOIDmode, operand0, gen_rtx (UDIV, SImode, operand1, operand2)),
		gen_rtx (SET, VOIDmode, operand3, gen_rtx (UMOD, SImode, operand1, operand2))));
}

rtx
gen_andsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (AND, SImode, operand1, operand2));
}

rtx
gen_andhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (AND, HImode, operand1, operand2));
}

rtx
gen_andqi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (AND, QImode, operand1, operand2));
}

rtx
gen_iorsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (IOR, SImode, operand1, operand2));
}

rtx
gen_iorhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (IOR, HImode, operand1, operand2));
}

rtx
gen_iorqi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (IOR, QImode, operand1, operand2));
}

rtx
gen_xorsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (XOR, SImode, operand1, operand2));
}

rtx
gen_xorhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (XOR, HImode, operand1, operand2));
}

rtx
gen_xorqi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (XOR, QImode, operand1, operand2));
}

rtx
gen_negsi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (NEG, SImode, operand1));
}

rtx
gen_neghi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (NEG, HImode, operand1));
}

rtx
gen_negqi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (NEG, QImode, operand1));
}

rtx
gen_negsf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx recog_operand [2];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (NEG, SFmode, operand1)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_negdf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx recog_operand [2];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (NEG, DFmode, operand1)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_abssf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx recog_operand [2];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (ABS, SFmode, operand1)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_absdf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx recog_operand [2];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (ABS, DFmode, operand1)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_one_cmplsi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (NOT, SImode, operand1));
}

rtx
gen_one_cmplhi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (NOT, HImode, operand1));
}

rtx
gen_one_cmplqi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (NOT, QImode, operand1));
}

rtx
gen_ashlsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (ASHIFT, SImode, operand1, operand2));
}

rtx
gen_ashlhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (ASHIFT, HImode, operand1, operand2));
}

rtx
gen_ashlqi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (ASHIFT, QImode, operand1, operand2));
}

rtx
gen_ashrsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (ASHIFTRT, SImode, operand1, operand2));
}

rtx
gen_ashrhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (ASHIFTRT, HImode, operand1, operand2));
}

rtx
gen_ashrqi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (ASHIFTRT, QImode, operand1, operand2));
}

rtx
gen_lshlsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (LSHIFT, SImode, operand1, operand2));
}

rtx
gen_lshlhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (LSHIFT, HImode, operand1, operand2));
}

rtx
gen_lshlqi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (LSHIFT, QImode, operand1, operand2));
}

rtx
gen_lshrsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (LSHIFTRT, SImode, operand1, operand2));
}

rtx
gen_lshrhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (LSHIFTRT, HImode, operand1, operand2));
}

rtx
gen_lshrqi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (LSHIFTRT, QImode, operand1, operand2));
}

rtx
gen_rotlsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (ROTATE, SImode, operand1, operand2));
}

rtx
gen_rotlhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (ROTATE, HImode, operand1, operand2));
}

rtx
gen_rotlqi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (ROTATE, QImode, operand1, operand2));
}

rtx
gen_rotrsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (ROTATERT, SImode, operand1, operand2));
}

rtx
gen_rotrhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (ROTATERT, HImode, operand1, operand2));
}

rtx
gen_rotrqi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (ROTATERT, QImode, operand1, operand2));
}

rtx
gen_extv (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (SIGN_EXTRACT, SImode, operand1, operand2, operand3));
}

rtx
gen_extzv (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (ZERO_EXTRACT, SImode, operand1, operand2, operand3));
}

rtx
gen_insv (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  return gen_rtx (SET, VOIDmode, gen_rtx (ZERO_EXTRACT, SImode, operand0, operand1, operand2), operand3);
}

rtx
gen_seq (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (EQ, VOIDmode, cc0_rtx, const0_rtx));
}

rtx
gen_sne (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (NE, VOIDmode, cc0_rtx, const0_rtx));
}

rtx
gen_sgt (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (GT, VOIDmode, cc0_rtx, const0_rtx));
}

rtx
gen_sgtu (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (GTU, VOIDmode, cc0_rtx, const0_rtx));
}

rtx
gen_slt (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (LT, VOIDmode, cc0_rtx, const0_rtx));
}

rtx
gen_sltu (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (LTU, VOIDmode, cc0_rtx, const0_rtx));
}

rtx
gen_sge (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (GE, VOIDmode, cc0_rtx, const0_rtx));
}

rtx
gen_sgeu (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (GEU, VOIDmode, cc0_rtx, const0_rtx));
}

rtx
gen_sle (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (LE, VOIDmode, cc0_rtx, const0_rtx));
}

rtx
gen_sleu (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (LEU, VOIDmode, cc0_rtx, const0_rtx));
}

rtx
gen_beq (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, pc_rtx, gen_rtx (IF_THEN_ELSE, VOIDmode, gen_rtx (EQ, VOIDmode, cc0_rtx, const0_rtx), gen_rtx (LABEL_REF, VOIDmode, operand0), pc_rtx));
}

rtx
gen_bne (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, pc_rtx, gen_rtx (IF_THEN_ELSE, VOIDmode, gen_rtx (NE, VOIDmode, cc0_rtx, const0_rtx), gen_rtx (LABEL_REF, VOIDmode, operand0), pc_rtx));
}

rtx
gen_bgt (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, pc_rtx, gen_rtx (IF_THEN_ELSE, VOIDmode, gen_rtx (GT, VOIDmode, cc0_rtx, const0_rtx), gen_rtx (LABEL_REF, VOIDmode, operand0), pc_rtx));
}

rtx
gen_bgtu (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, pc_rtx, gen_rtx (IF_THEN_ELSE, VOIDmode, gen_rtx (GTU, VOIDmode, cc0_rtx, const0_rtx), gen_rtx (LABEL_REF, VOIDmode, operand0), pc_rtx));
}

rtx
gen_blt (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, pc_rtx, gen_rtx (IF_THEN_ELSE, VOIDmode, gen_rtx (LT, VOIDmode, cc0_rtx, const0_rtx), gen_rtx (LABEL_REF, VOIDmode, operand0), pc_rtx));
}

rtx
gen_bltu (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, pc_rtx, gen_rtx (IF_THEN_ELSE, VOIDmode, gen_rtx (LTU, VOIDmode, cc0_rtx, const0_rtx), gen_rtx (LABEL_REF, VOIDmode, operand0), pc_rtx));
}

rtx
gen_bge (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, pc_rtx, gen_rtx (IF_THEN_ELSE, VOIDmode, gen_rtx (GE, VOIDmode, cc0_rtx, const0_rtx), gen_rtx (LABEL_REF, VOIDmode, operand0), pc_rtx));
}

rtx
gen_bgeu (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, pc_rtx, gen_rtx (IF_THEN_ELSE, VOIDmode, gen_rtx (GEU, VOIDmode, cc0_rtx, const0_rtx), gen_rtx (LABEL_REF, VOIDmode, operand0), pc_rtx));
}

rtx
gen_ble (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, pc_rtx, gen_rtx (IF_THEN_ELSE, VOIDmode, gen_rtx (LE, VOIDmode, cc0_rtx, const0_rtx), gen_rtx (LABEL_REF, VOIDmode, operand0), pc_rtx));
}

rtx
gen_bleu (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, pc_rtx, gen_rtx (IF_THEN_ELSE, VOIDmode, gen_rtx (LEU, VOIDmode, cc0_rtx, const0_rtx), gen_rtx (LABEL_REF, VOIDmode, operand0), pc_rtx));
}

rtx
gen_casesi_1 (operand0, operand1, operand2, operand3, operand4)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
     rtx operand4;
{
  rtx recog_operand [5];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;
  recog_operand [2] = operand2;
  recog_operand [3] = operand3;
  recog_operand [4] = operand4;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  operand2 = recog_operand [2];
  operand3 = recog_operand [3];
  operand4 = recog_operand [4];
  emit_insn (gen_rtx (SET, VOIDmode, operand3, gen_rtx (PLUS, SImode, operand0, operand1)));
  emit_insn (gen_rtx (SET, VOIDmode, cc0_rtx, gen_rtx (COMPARE, VOIDmode, operand2, operand3)));
  emit_jump_insn (gen_rtx (SET, VOIDmode, pc_rtx, gen_rtx (IF_THEN_ELSE, VOIDmode, gen_rtx (LTU, VOIDmode, cc0_rtx, const0_rtx), gen_rtx (LABEL_REF, VOIDmode, operand4), pc_rtx)));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_casesi_2 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx recog_operand [3];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;
  recog_operand [2] = operand2;

  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  operand2 = recog_operand [2];
  emit_insn (gen_rtx (SET, VOIDmode, operand0, gen_rtx (MEM, HImode, operand1)));
  emit_jump_insn (gen_rtx (PARALLEL, VOIDmode, gen_rtvec (2,
		gen_rtx (SET, VOIDmode, pc_rtx, gen_rtx (PLUS, SImode, pc_rtx, operand0)),
		gen_rtx (USE, VOIDmode, gen_rtx (LABEL_REF, VOIDmode, operand2)))));
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_casesi (operand0, operand1, operand2, operand3, operand4)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
     rtx operand4;
{
  rtx recog_operand [5];
  rtx _val;
  start_sequence ();
  recog_operand [0] = operand0;
  recog_operand [1] = operand1;
  recog_operand [2] = operand2;
  recog_operand [3] = operand3;
  recog_operand [4] = operand4;

{
  rtx table_elt_addr;
  rtx index_diff;

  recog_operand [1] = negate_rtx (SImode, recog_operand [1]);
  index_diff = gen_reg_rtx (SImode);
  emit_insn (gen_casesi_1 (recog_operand [0], recog_operand [1], recog_operand [2],
			   index_diff, recog_operand [4]));
  table_elt_addr
    = memory_address_noforce
        (HImode,
	 gen_rtx (PLUS, SImode ,
		  gen_rtx (MULT, SImode , index_diff,
			   gen_rtx (CONST_INT, VOIDmode, 2)),
		  gen_rtx (LABEL_REF, VOIDmode, recog_operand [3])));
  emit_insn (gen_casesi_2 (gen_reg_rtx (HImode), table_elt_addr, recog_operand [3]));
  goto _done ;
}
  operand0 = recog_operand [0];
  operand1 = recog_operand [1];
  operand2 = recog_operand [2];
  operand3 = recog_operand [3];
  operand4 = recog_operand [4];
  emit (operand0);
  emit (operand1);
  emit (operand2);
  emit (operand3);
  emit (operand4);
 _done:
  _val = gen_sequence ();
  end_sequence ();
  return _val;
}

rtx
gen_jump (operand0)
     rtx operand0;
{
  return gen_rtx (SET, VOIDmode, pc_rtx, gen_rtx (LABEL_REF, VOIDmode, operand0));
}

rtx
gen_call (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx (CALL, VOIDmode, operand0, operand1);
}

rtx
gen_call_value (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx (SET, VOIDmode, operand0, gen_rtx (CALL, VOIDmode, operand1, operand2));
}

rtx
gen_return ()
{
  return gen_rtx (RETURN, VOIDmode);
}


