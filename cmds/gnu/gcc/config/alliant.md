;;- Machine description for GNU compiler
;;- Alliant FX Version (Based on Motorola 68000)
;;   Copyright (C) 1989 Free Software Foundation, Inc.

;; This file is part of GNU CC.

;; GNU CC is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 1, or (at your option)
;; any later version.

;; GNU CC is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GNU CC; see the file COPYING.  If not, write to
;; the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.


;;- instruction definitions

;;- @@The original PO technology requires these to be ordered by speed,
;;- @@    so that assigner will pick the fastest.

;;- See file "rtl.def" for documentation on define_insn, match_*, et. al.

;;- When naming insn's (operand 0 of define_insn) be careful about using
;;- names from other targets machine descriptions.

;;- cpp macro #define NOTICE_UPDATE_CC in file tm.h handles condition code
;;- updates for most instructions.

;;- Operand classes for the register allocator:
;;- 'a' one of the address registers can be used.
;;- 'd' one of the data registers can be used.
;;- 'f' one of the m68881 registers can be used
;;- 'r' either a data or an address register can be used.
;;- 'x' if one of the Sun FPA registers                    
;;- 'y' if one of the Low Sun FPA registers (fpa0-fpa15).

;;- Immediate Floating point operator constraints
;;- 'G' a floating point constant that is *NOT* one of the standard
;;   68881 constant values (to force calling output_move_const_double
;;   to get it from rom if it is a 68881 constant).
;;- 'H' one of the standard FPA constant values
;;
;;   See the functions standard_XXX_constant_p in output-m68k.c for more
;; info.

;;- Immedidate integer operands Constrains:
;;- 'I'  1 .. 8
;;- 'J'  -32768 .. 32767
;;- 'K'  -128 .. 127
;;- 'L'  -8 .. -1

;;- Some of these insn's are composites of several m68000 op codes.
;;- The assembler (or final @@??) insures that the appropriate one is
;;- selected.


(define_insn ""
  [(set (match_operand:DF 0 "push_operand" "=m")
	(match_operand:DF 1 "general_operand" "ro<>fyF"))]
  ""
  "*
{
  if (FP_REG_P (operands[1]))
    return \"fmoved %1,%0\";
  return output_move_double (operands);
}")

(define_insn ""
  [(set (match_operand:DI 0 "push_operand" "=m")
	(match_operand:DI 1 "general_operand" "ro<>Fy"))]
  ""
  "*
{
  return output_move_double (operands);
}")

(define_insn "tstsi"
  [(set (cc0)
	(match_operand:SI 0 "general_operand" "rm"))]
  ""
  "*
{
  if (TARGET_68020 || ! ADDRESS_REG_P (operands[0]))
    return \"tstl %0\";
  /* If you think that the 68020 does not support tstl a0,
     reread page B-167 of the 68020 manual more carefully.  */
  /* On an address reg, cmpw may replace cmpl.  */
  return \"cmpw %#0,%0\";
}")

(define_insn "tsthi"
  [(set (cc0)
	(match_operand:HI 0 "general_operand" "rm"))]
  ""
  "*
{
  if (TARGET_68020 || ! ADDRESS_REG_P (operands[0]))
    return \"tstw %0\";
  return \"cmpw %#0,%0\";
}")

(define_insn "tstqi"
  [(set (cc0)
	(match_operand:QI 0 "general_operand" "dm"))]
  ""
  "tstb %0")
  
(define_expand "tstsf"
  [(set (cc0)
	(match_operand:SF 0 "general_operand" ""))]
  "TARGET_68881"
  "")


(define_insn ""
  [(set (cc0)
	(match_operand:SF 0 "general_operand" "fdm"))]
  "TARGET_68881"
  "*
{
  cc_status.flags = CC_IN_68881;
  if (FP_REG_P (operands[0]))
    return \"ftests %0\";
  return \"ftests %0\";
}")

(define_expand "tstdf"
  [(set (cc0)
	(match_operand:DF 0 "general_operand" ""))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (cc0)
	(match_operand:DF 0 "general_operand" "fm"))]
  "TARGET_68881"
  "*
{
  cc_status.flags = CC_IN_68881;
  if (FP_REG_P (operands[0]))
    return \"ftestd %0\";
  return \"ftestd %0\";
}")

;; compare instructions.

;; A composite of the cmp, cmpa, & cmpi m68000 op codes.
(define_insn "cmpsi"
  [(set (cc0)
	(compare (match_operand:SI 0 "general_operand" "rKs,mr")
		 (match_operand:SI 1 "general_operand" "mr,Ksr")))]
  ""
  "*
{
  if (REG_P (operands[1])
      || (!REG_P (operands[0]) && GET_CODE (operands[0]) != MEM))
    {
      cc_status.flags |= CC_REVERSED;
      return \"cmpl %0,%1\"; 
    }
  return \"cmpl %1,%0\";
}")

(define_insn "cmphi"
  [(set (cc0)
	(compare (match_operand:HI 0 "general_operand" "rn,mr")
		 (match_operand:HI 1 "general_operand" "mr,nr")))]
  ""
  "*
{
  if (REG_P (operands[1])
      || (!REG_P (operands[0]) && GET_CODE (operands[0]) != MEM))
    {
      cc_status.flags |= CC_REVERSED;
      return \"cmpw %0,%1\"; 
    }
  return \"cmpw %1,%0\";
}")

;;
;;	This seems to cause problems -- PMP
;;
;;(define_insn ""
;;  [(set (cc0)
;;	(compare (match_operand:QI 0 "memory_operand" ">")
;;	         (match_operand:QI 1 "memory_operand" ">")))]
;;  "GET_CODE (XEXP (operands[0], 0)) == POST_INC
;;   && GET_CODE (XEXP (operands[1], 0)) == POST_INC"
;;  "cmpmb %1,%0")

(define_insn "cmpqi"
  [(set (cc0)
	(compare (match_operand:QI 0 "general_operand" "dn,md")
		 (match_operand:QI 1 "general_operand" "dm,nd")))]
  ""
  "*
{
  if (REG_P (operands[1])
      || (!REG_P (operands[0]) && GET_CODE (operands[0]) != MEM))
    {
      cc_status.flags |= CC_REVERSED;
      return \"cmpb %0,%1\";
    }
  return \"cmpb %1,%0\";
}")

(define_expand "cmpdf"
  [(set (cc0)
	(compare (match_operand:DF 0 "general_operand" "")
		 (match_operand:DF 1 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (cc0)
	(compare (match_operand:DF 0 "general_operand" "f,m")
		 (match_operand:DF 1 "general_operand" "fm,f")))]
  "TARGET_68881"
  "*
{
  cc_status.flags = CC_IN_68881;
  if (REG_P (operands[0]))
    {
      if (REG_P (operands[1]))
	return \"fcmpd %1,%0\";
      else
        return \"fcmpd %1,%0\";
    }
  cc_status.flags |= CC_REVERSED;
  return \"fcmpd %0,%1\";
}")

(define_expand "cmpsf"
 [(set (cc0)
       (compare (match_operand:SF 0 "general_operand" "")
		(match_operand:SF 1 "general_operand" "")))]
 "TARGET_68881"
 "")

(define_insn ""
  [(set (cc0)
	(compare (match_operand:SF 0 "general_operand" "f,md")
		 (match_operand:SF 1 "general_operand" "fmd,f")))]
  "TARGET_68881"
  "*
{
  cc_status.flags = CC_IN_68881;
  if (FP_REG_P (operands[0]))
    {
      if (FP_REG_P (operands[1]))
	return \"fcmps %1,%0\";
      else
        return \"fcmps %1,%0\";
    }
  cc_status.flags |= CC_REVERSED;
  return \"fcmps %0,%1\";
}")

;; Recognizers for btst instructions.

(define_insn ""
  [(set (cc0) (zero_extract (match_operand:QI 0 "nonimmediate_operand" "do")
			    (const_int 1)
			    (minus:SI (const_int 7)
				      (match_operand:SI 1 "general_operand" "di"))))]
  ""
  "* { return output_btst (operands, operands[1], operands[0], insn, 7); }")

(define_insn ""
  [(set (cc0) (zero_extract (match_operand:SI 0 "nonimmediate_operand" "d")
			    (const_int 1)
			    (minus:SI (const_int 31)
				      (match_operand:SI 1 "general_operand" "di"))))]
  ""
  "* { return output_btst (operands, operands[1], operands[0], insn, 31); }")

;; The following two patterns are like the previous two
;; except that they use the fact that bit-number operands
;; are automatically masked to 3 or 5 bits.

(define_insn ""
  [(set (cc0) (zero_extract (match_operand:QI 0 "nonimmediate_operand" "do")
			    (const_int 1)
			    (minus:SI (const_int 7)
				      (and:SI
				       (match_operand:SI 1 "general_operand" "d")
				       (const_int 7)))))]
  ""
  "* { return output_btst (operands, operands[1], operands[0], insn, 7); }")

(define_insn ""
  [(set (cc0) (zero_extract (match_operand:SI 0 "nonimmediate_operand" "d")
			    (const_int 1)
			    (minus:SI (const_int 31)
				      (and:SI
				       (match_operand:SI 1 "general_operand" "d")
				       (const_int 31)))))]
  ""
  "* { return output_btst (operands, operands[1], operands[0], insn, 31); }")

(define_insn ""
  ;; The constraint "o,d" here means that a nonoffsetable memref
  ;; will match the first alternative, and its address will be reloaded.
  ;; Copying the memory contents into a reg would be incorrect if the
  ;; bit position is over 7.
  [(set (cc0) (zero_extract (match_operand:QI 0 "nonimmediate_operand" "o,d")
			    (const_int 1)
			    (match_operand:SI 1 "general_operand" "i,i")))]
  "GET_CODE (operands[1]) == CONST_INT"
  "*
{ operands[1] = gen_rtx (CONST_INT, VOIDmode, 7 - INTVAL (operands[1]));
  return output_btst (operands, operands[1], operands[0], insn, 7); }")

(define_insn ""
  [(set (cc0) (zero_extract (match_operand:HI 0 "nonimmediate_operand" "o,d")
			    (const_int 1)
			    (match_operand:SI 1 "general_operand" "i,i")))]
  "GET_CODE (operands[1]) == CONST_INT"
  "*
{
  if (GET_CODE (operands[0]) == MEM)
    {
      operands[0] = adj_offsetable_operand (operands[0],
					    INTVAL (operands[1]) / 8);
      operands[1] = gen_rtx (CONST_INT, VOIDmode, 
			     7 - INTVAL (operands[1]) % 8);
      return output_btst (operands, operands[1], operands[0], insn, 7);
    }
  operands[1] = gen_rtx (CONST_INT, VOIDmode,
			 15 - INTVAL (operands[1]));
  return output_btst (operands, operands[1], operands[0], insn, 15);
}")

(define_insn ""
  [(set (cc0) (zero_extract (match_operand:SI 0 "nonimmediate_operand" "do")
			    (const_int 1)
			    (match_operand:SI 1 "general_operand" "i")))]
  "GET_CODE (operands[1]) == CONST_INT"
  "*
{
  if (GET_CODE (operands[0]) == MEM)
    {
      operands[0] = adj_offsetable_operand (operands[0],
					    INTVAL (operands[1]) / 8);
      operands[1] = gen_rtx (CONST_INT, VOIDmode, 
			     7 - INTVAL (operands[1]) % 8);
      return output_btst (operands, operands[1], operands[0], insn, 7);
    }
  operands[1] = gen_rtx (CONST_INT, VOIDmode,
			 31 - INTVAL (operands[1]));
  return output_btst (operands, operands[1], operands[0], insn, 31);
}")

(define_insn ""
  [(set (cc0) (subreg:SI (lshiftrt:QI (match_operand:QI 0 "nonimmediate_operand" "dm")
				      (const_int 7))
			 0))]
  ""
  "*
{
  cc_status.flags = CC_Z_IN_NOT_N | CC_NOT_NEGATIVE;
  return \"tstb %0\";
}")

(define_insn ""
  [(set (cc0) (and:SI (sign_extend:SI (sign_extend:HI (match_operand:QI 0 "nonimmediate_operand" "dm")))
		      (match_operand:SI 1 "general_operand" "i")))]
  "(GET_CODE (operands[1]) == CONST_INT
    && (unsigned) INTVAL (operands[1]) < 0x100
    && exact_log2 (INTVAL (operands[1])) >= 0)"
  "*
{ register int log = exact_log2 (INTVAL (operands[1]));
  operands[1] = gen_rtx (CONST_INT, VOIDmode, log);
  return output_btst (operands, operands[1], operands[0], insn, 7);
}")

;; move instructions
(define_insn "swapsi"
  [(set (match_operand:SI 0 "general_operand" "r")
	(match_operand:SI 1 "general_operand" "r"))
   (set (match_dup 1) (match_dup 0))]
  ""
  "exg %1,%0")

;; Special case of fullword move when source is zero.
;; The reason this is special is to avoid loading a zero
;; into a data reg with moveq in order to store it elsewhere.
   
(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=g")
	(const_int 0))]
  ""
  "*
{
  if (ADDRESS_REG_P (operands[0]))
    return \"subl %0,%0\";
  return \"clrl %0\";
}")

;; Another special case in which it is not desirable
;; to reload the constant into a data register.
(define_insn ""
  [(set (match_operand:SI 0 "push_operand" "=m")
	(match_operand:SI 1 "general_operand" "J"))]
  "GET_CODE (operands[1]) == CONST_INT
   && INTVAL (operands[1]) >= -0x8000
   && INTVAL (operands[1]) < 0x8000"
  "pea %a1")

;; General case of fullword move.  The register constraints
;; force integer constants in range for a moveq to be reloaded
;; if they are headed for memory.
(define_insn "movsi"
  ;; Notes: make sure no alternative allows g vs g.
  ;; We don't allow f-regs since fixed point cannot go in them.
  ;; We do allow y and x regs since fixed point is allowed in them.
  [(set (match_operand:SI 0 "general_operand" "=g,da,y,!*x*r*m")
	(match_operand:SI 1 "general_operand" "daymKs,i,g,*x*r*m"))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == CONST_INT)
    {
      if (operands[1] == const0_rtx
	  && (DATA_REG_P (operands[0])
	      || GET_CODE (operands[0]) == MEM))
	return \"clrl %0\";
      else if (DATA_REG_P (operands[0])
	       && INTVAL (operands[1]) < 128
	       && INTVAL (operands[1]) >= -128)
	return \"moveq %1,%0\";
      else if (ADDRESS_REG_P (operands[0])
	       && INTVAL (operands[1]) < 0x8000
	       && INTVAL (operands[1]) >= -0x8000)
	return \"movw %1,%0\";
      else if (push_operand (operands[0], SImode)
	       && INTVAL (operands[1]) < 0x8000
	       && INTVAL (operands[1]) >= -0x8000)
        return \"pea %a1\";
    }
  else if ((GET_CODE (operands[1]) == SYMBOL_REF
	    || GET_CODE (operands[1]) == CONST)
	   && push_operand (operands[0], SImode))
    return \"pea %a1\";
  else if ((GET_CODE (operands[1]) == SYMBOL_REF
	    || GET_CODE (operands[1]) == CONST)
	   && ADDRESS_REG_P (operands[0]))
    return \"lea %a1,%0\";
  return \"movl %1,%0\";
}")

(define_insn "movhi"
  [(set (match_operand:HI 0 "general_operand" "=g")
	(match_operand:HI 1 "general_operand" "g"))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == CONST_INT)
    {
      if (operands[1] == const0_rtx
	  && (DATA_REG_P (operands[0])
	      || GET_CODE (operands[0]) == MEM))
	return \"clrw %0\";
      else if (DATA_REG_P (operands[0])
	       && INTVAL (operands[1]) < 128
	       && INTVAL (operands[1]) >= -128)
        {
	  return \"moveq %1,%0\";
	}
      else if (INTVAL (operands[1]) < 0x8000
	       && INTVAL (operands[1]) >= -0x8000)
	return \"movw %1,%0\";
    }
  else if (CONSTANT_P (operands[1]))
    return \"movl %1,%0\";
  /* Recognize the insn before a tablejump, one that refers
     to a table of offsets.  Such an insn will need to refer
     to a label on the insn.  So output one.  Use the label-number
     of the table of offsets to generate this label.  */
  if (GET_CODE (operands[1]) == MEM
      && GET_CODE (XEXP (operands[1], 0)) == PLUS
      && (GET_CODE (XEXP (XEXP (operands[1], 0), 0)) == LABEL_REF
	  || GET_CODE (XEXP (XEXP (operands[1], 0), 1)) == LABEL_REF)
      && GET_CODE (XEXP (XEXP (operands[1], 0), 0)) != PLUS
      && GET_CODE (XEXP (XEXP (operands[1], 0), 1)) != PLUS)
    {
      rtx labelref;
      if (GET_CODE (XEXP (XEXP (operands[1], 0), 0)) == LABEL_REF)
	labelref = XEXP (XEXP (operands[1], 0), 0);
      else
	labelref = XEXP (XEXP (operands[1], 0), 1);
      ASM_OUTPUT_INTERNAL_LABEL (asm_out_file, \"LI\",
				 CODE_LABEL_NUMBER (XEXP (labelref, 0)));
    }
  return \"movw %1,%0\";
}")

(define_insn "movstricthi"
  [(set (strict_low_part (match_operand:HI 0 "general_operand" "+dm"))
	(match_operand:HI 1 "general_operand" "rmn"))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == CONST_INT)
    {
      if (operands[1] == const0_rtx
	  && (DATA_REG_P (operands[0])
	      || GET_CODE (operands[0]) == MEM))
	return \"clrw %0\";
    }
  return \"movw %1,%0\";
}")

(define_insn "movqi"
  [(set (match_operand:QI 0 "general_operand" "=d,*a,m")
	(match_operand:QI 1 "general_operand" "dmi*a,d*a,dmi"))]
  ""
  "*
{
  if (operands[1] == const0_rtx)
    return \"clrb %0\";
  if (GET_CODE (operands[1]) == CONST_INT
      && INTVAL (operands[1]) == -1)
    return \"st %0\";
  if (GET_CODE (operands[1]) != CONST_INT && CONSTANT_P (operands[1]))
    return \"movl %1,%0\";
  if (ADDRESS_REG_P (operands[0]) || ADDRESS_REG_P (operands[1]))
    return \"movw %1,%0\";
  return \"movb %1,%0\";
}")

(define_insn "movstrictqi"
  [(set (strict_low_part (match_operand:QI 0 "general_operand" "+dm"))
	(match_operand:QI 1 "general_operand" "dmn"))]
  ""
  "*
{
  if (operands[1] == const0_rtx)
    return \"clrb %0\";
  return \"movb %1,%0\";
}")

(define_insn "movsf"
  [(set (match_operand:SF 0 "general_operand" "=rmf,x,y,rm,!x,!rm")
	(match_operand:SF 1 "general_operand" "rmfF,x,rmF,y,rm,x"))]
  ""
  "*
{
  if (FP_REG_P (operands[0]))
    {
      if (FP_REG_P (operands[1]))
	return \"fmoves %1,%0\";
      else if (ADDRESS_REG_P (operands[1]) || DATA_REG_P (operands[1]))
	return \"movl %1,%-\;fmoves %+,%0\";
      else if (GET_CODE (operands[1]) == CONST_DOUBLE)
	{
	  rtx xoperands[2];
	  xoperands[1] = gen_rtx (CONST_INT, VOIDmode,
				  CONST_DOUBLE_HIGH (operands[1]));
	  output_asm_insn (\"movl %1,%-\", xoperands);
	  xoperands[1] = gen_rtx (CONST_INT, VOIDmode,
				  CONST_DOUBLE_LOW (operands[1]));
	  output_asm_insn (\"movl %1,%-\", xoperands);
	  return \"fmoveds %+,%0\";
	}
      return \"fmoves %1,%0\";
    }
  if (FP_REG_P (operands[1]))
    {
      if (ADDRESS_REG_P (operands[0]) || DATA_REG_P (operands[0]))
	return \"fmoves %1,%-\;movl %+,%0\";
      return \"fmoves %1,%0\";
    }
  return \"movl %1,%0\";
}")

;; This pattern forces (set (reg:DF ...) (const_double ...))
;; to be reloaded by putting the constant into memory.
;; It must come before the more general movdf pattern.

(define_insn ""
  [(set (match_operand:DF 0 "general_operand" "=f,f,f")
        (match_operand:DF 1 "" "mF,m,F"))]
  "GET_CODE (operands[1]) == CONST_DOUBLE"
  "*
{
  rtx xoperands[2];
  xoperands[1] = gen_rtx (CONST_INT, VOIDmode,
			  CONST_DOUBLE_HIGH (operands[1]));
  output_asm_insn (\"movl %1,%-\", xoperands);
  xoperands[1] = gen_rtx (CONST_INT, VOIDmode,
			  CONST_DOUBLE_LOW (operands[1]));
  output_asm_insn (\"movl %1,%-\", xoperands);
  return \"fmoved %+,%0\";
}
")

(define_insn "movdf"
  [(set (match_operand:DF 0 "general_operand" "=rm,&rf,&rof<>,y,rm,x,!x,!rm")
	(match_operand:DF 1 "general_operand" "rf,m,rof<>,rm,y,x,rm,x"))]
  ""
  "*
{
  if (FP_REG_P (operands[0]))
    {
      if (FP_REG_P (operands[1]))
	return \"fmoved %1,%0\";
      if (REG_P (operands[1]))
	{
	  rtx xoperands[2];
	  xoperands[1] = gen_rtx (REG, SImode, REGNO (operands[1]) + 1);
	  output_asm_insn (\"movl %1,%-\", xoperands);
	  output_asm_insn (\"movl %1,%-\", operands);
	  return \"fmoved %+,%0\";
	}
      if (GET_CODE (operands[1]) == CONST_DOUBLE)
	{
	  rtx xoperands[2];
	  xoperands[1] = gen_rtx (CONST_INT, VOIDmode,
				  CONST_DOUBLE_HIGH (operands[1]));
	  output_asm_insn (\"movl %1,%-\", xoperands);
	  xoperands[1] = gen_rtx (CONST_INT, VOIDmode,
				  CONST_DOUBLE_LOW (operands[1]));
	  output_asm_insn (\"movl %1,%-\", xoperands);
	  return \"fmoved %+,%0\";
	}
      return \"fmoved %1,%0\";
    }
  else if (FP_REG_P (operands[1]))
    {
      if (REG_P (operands[0]))
	{
	  output_asm_insn (\"fmoved %1,%-\;movl %+,%0\", operands);
	  operands[0] = gen_rtx (REG, SImode, REGNO (operands[0]) + 1);
	  return \"movl %+,%0\";
	}
      else
        return \"fmoved %1,%0\";
    }
  return output_move_double (operands);
}
")

;; movdi can apply to fp regs in some cases
(define_insn "movdi"
  ;; Let's see if it really still needs to handle fp regs, and, if so, why.
  [(set (match_operand:DI 0 "general_operand" "=rm,&r,&ro<>,y,rm,!*x,!rm")
	(match_operand:DI 1 "general_operand" "rF,m,roi<>F,rmiF,y,rmF,*x"))]
  ""
  "*
{
  if (FP_REG_P (operands[0]))
    {
      if (FP_REG_P (operands[1]))
	return \"fmoved %1,%0\";
      if (REG_P (operands[1]))
	{
	  rtx xoperands[2];
	  xoperands[1] = gen_rtx (REG, SImode, REGNO (operands[1]) + 1);
	  output_asm_insn (\"movl %1,%-\", xoperands);
	  output_asm_insn (\"movl %1,%-\", operands);
	  return \"fmoved %+,%0\";
	}
      if (GET_CODE (operands[1]) == CONST_DOUBLE)
	return output_move_const_double (operands);
      return \"fmoved %1,%0\";
    }
  else if (FP_REG_P (operands[1]))
    {
      if (REG_P (operands[0]))
	{
	  output_asm_insn (\"fmoved %1,%-\;movl %+,%0\", operands);
	  operands[0] = gen_rtx (REG, SImode, REGNO (operands[0]) + 1);
	  return \"movl %+,%0\";
	}
      else
        return \"fmoved %1,%0\";
    }
  return output_move_double (operands);
}
")

;; These go after the move instructions
;; because the move instructions are better (require no spilling)
;; when they can apply.  But these go before the add and subtract insns
;; because it is often shorter to use these when both apply.
(define_insn "pushasi"
  [(set (match_operand:SI 0 "push_operand" "=m")
	(match_operand:SI 1 "address_operand" "p"))]
  ""
  "pea %a1")


(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=a")
	(match_operand:QI 1 "address_operand" "p"))]
  ""
  "lea %a1,%0")

;; truncation instructions
(define_insn "truncsiqi2"
  [(set (match_operand:QI 0 "general_operand" "=dm,d")
	(truncate:QI
	 (match_operand:SI 1 "general_operand" "doJ,i")))]
  ""
  "*
{
  if (GET_CODE (operands[0]) == REG)
    return \"movl %1,%0\";
  if (GET_CODE (operands[1]) == MEM)
    operands[1] = adj_offsetable_operand (operands[1], 3);
  return \"movb %1,%0\";
}")

(define_insn "trunchiqi2"
  [(set (match_operand:QI 0 "general_operand" "=dm,d")
	(truncate:QI
	 (match_operand:HI 1 "general_operand" "doJ,i")))]
  ""
  "*
{
  if (GET_CODE (operands[0]) == REG)
    return \"movl %1,%0\";
  if (GET_CODE (operands[1]) == MEM)
    operands[1] = adj_offsetable_operand (operands[1], 1);
  return \"movb %1,%0\";
}")

(define_insn "truncsihi2"
  [(set (match_operand:HI 0 "general_operand" "=dm,d")
	(truncate:HI
	 (match_operand:SI 1 "general_operand" "roJ,i")))]
  ""
  "*
{
  if (GET_CODE (operands[0]) == REG)
    return \"movl %1,%0\";
  if (GET_CODE (operands[1]) == MEM)
    operands[1] = adj_offsetable_operand (operands[1], 2);
  return \"movw %1,%0\";
}")

;; zero extension instructions

(define_expand "zero_extendhisi2"
  [(set (match_operand:SI 0 "register_operand" "")
	(const_int 0))
   (set (strict_low_part (subreg:HI (match_dup 0) 0))
	(match_operand:HI 1 "general_operand" ""))]
  ""
  "operands[1] = make_safe_from (operands[1], operands[0]);")

(define_expand "zero_extendqihi2"
  [(set (match_operand:HI 0 "register_operand" "")
	(const_int 0))
   (set (strict_low_part (subreg:QI (match_dup 0) 0))
	(match_operand:QI 1 "general_operand" ""))]
  ""
  "operands[1] = make_safe_from (operands[1], operands[0]);")

(define_expand "zero_extendqisi2"
  [(set (match_operand:SI 0 "register_operand" "")
	(const_int 0))
   (set (strict_low_part (subreg:QI (match_dup 0) 0))
	(match_operand:QI 1 "general_operand" ""))]
  ""
  " operands[1] = make_safe_from (operands[1], operands[0]); ")

;; Patterns to recognize zero-extend insns produced by the combiner.

;; Note that the one starting from HImode comes before those for QImode
;; so that a constant operand will match HImode, not QImode.
(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=do<>")
	(zero_extend:SI
	 (match_operand:HI 1 "general_operand" "rmn")))]
  ""
  "*
{
  if (DATA_REG_P (operands[0]))
    {
      if (GET_CODE (operands[1]) == REG
	  && REGNO (operands[0]) == REGNO (operands[1]))
	return \"andl %#0xFFFF,%0\";
      if (reg_mentioned_p (operands[0], operands[1]))
        return \"movw %1,%0\;andl %#0xFFFF,%0\";
      return \"clrl %0\;movw %1,%0\";
    }
  else if (GET_CODE (operands[0]) == MEM
	   && GET_CODE (XEXP (operands[0], 0)) == PRE_DEC)
    return \"movw %1,%0\;clrw %0\";
  else if (GET_CODE (operands[0]) == MEM
	   && GET_CODE (XEXP (operands[0], 0)) == POST_INC)
    return \"clrw %0\;movw %1,%0\";
  else
    {
      output_asm_insn (\"clrw %0\", operands);
      operands[0] = adj_offsetable_operand (operands[0], 2);
      return \"movw %1,%0\";
    }
}")

(define_insn ""
  [(set (match_operand:HI 0 "general_operand" "=do<>")
	(zero_extend:HI
	 (match_operand:QI 1 "general_operand" "dmn")))]
  ""
  "*
{
  if (DATA_REG_P (operands[0]))
    {
      if (GET_CODE (operands[1]) == REG
	  && REGNO (operands[0]) == REGNO (operands[1]))
	return \"andw %#0xFF,%0\";
      if (reg_mentioned_p (operands[0], operands[1]))
        return \"movb %1,%0\;andw %#0xFF,%0\";
      return \"clrw %0\;movb %1,%0\";
    }
  else if (GET_CODE (operands[0]) == MEM
	   && GET_CODE (XEXP (operands[0], 0)) == PRE_DEC)
    {
      if (REGNO (XEXP (XEXP (operands[0], 0), 0))
	  == STACK_POINTER_REGNUM)
	return \"clrw %-\;movb %1,%0\";
      else
	return \"movb %1,%0\;clrb %0\";
    }
  else if (GET_CODE (operands[0]) == MEM
	   && GET_CODE (XEXP (operands[0], 0)) == POST_INC)
    return \"clrb %0\;movb %1,%0\";
  else
    {
      output_asm_insn (\"clrb %0\", operands);
      operands[0] = adj_offsetable_operand (operands[0], 1);
      return \"movb %1,%0\";
    }
}")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=do<>")
	(zero_extend:SI
	 (match_operand:QI 1 "general_operand" "dmn")))]
  ""
  "*
{
  if (DATA_REG_P (operands[0]))
    {
      if (GET_CODE (operands[1]) == REG
	  && REGNO (operands[0]) == REGNO (operands[1]))
	return \"andl %#0xFF,%0\";
      if (reg_mentioned_p (operands[0], operands[1]))
        return \"movb %1,%0\;andl %#0xFF,%0\";
      return \"clrl %0\;movb %1,%0\";
    }
  else if (GET_CODE (operands[0]) == MEM
	   && GET_CODE (XEXP (operands[0], 0)) == PRE_DEC)
    {
      operands[0] = XEXP (XEXP (operands[0], 0), 0);
      return \"clrl %0@-\;movb %1,%0@(3)\";
    }
  else if (GET_CODE (operands[0]) == MEM
	   && GET_CODE (XEXP (operands[0], 0)) == POST_INC)
    {
      operands[0] = XEXP (XEXP (operands[0], 0), 0);
      return \"clrl %0@+\;movb %1,%0@(-1)\";
    }
  else
    {
      output_asm_insn (\"clrl %0\", operands);
      operands[0] = adj_offsetable_operand (operands[0], 3);
      return \"movb %1,%0\";
    }
}")

;; sign extension instructions
;; Note that the one starting from HImode comes before those for QImode
;; so that a constant operand will match HImode, not QImode.

(define_insn "extendhisi2"
  [(set (match_operand:SI 0 "general_operand" "=*d,a")
	(sign_extend:SI
	 (match_operand:HI 1 "general_operand" "0,rmn")))]
  ""
  "*
{
  if (ADDRESS_REG_P (operands[0]))
    return \"movw %1,%0\";
  return \"extl %0\";
}")

(define_insn "extendqihi2"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(sign_extend:HI
	 (match_operand:QI 1 "general_operand" "0")))]
  ""
  "extw %0")

(define_insn "extendqisi2"
  [(set (match_operand:SI 0 "general_operand" "=d")
	(sign_extend:SI
	 (match_operand:QI 1 "general_operand" "0")))]
  "TARGET_68020"
  "extbl %0")

;; Conversions between float and double.

(define_expand "extendsfdf2"
  [(set (match_operand:DF 0 "general_operand" "")
	(float_extend:DF
	 (match_operand:SF 1 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:DF 0 "general_operand" "=f")
	(float_extend:DF
	  (match_operand:SF 1 "general_operand" "fm")))]
  "TARGET_68881"
  "*
{
  if (FP_REG_P (operands[0]) && FP_REG_P (operands[1]))
    return \"fmovesd %1,%0\";
  if (FP_REG_P (operands[0]))
    return \"fmovesd %1,%0\";
  if (DATA_REG_P (operands[0]) && FP_REG_P (operands[1]))
    {
      output_asm_insn (\"fmovesd %1,%-\;movl %+,%0\", operands);
      operands[0] = gen_rtx (REG, SImode, REGNO (operands[0]) + 1);
      return \"movl %+,%0\";
    }
  return \"fmovesd %1,%0\";
}")

;; This cannot output into an f-reg because there is no way to be
;; sure of truncating in that case.

(define_expand "truncdfsf2"
  [(set (match_operand:SF 0 "general_operand" "")
	(float_truncate:SF
	  (match_operand:DF 1 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:SF 0 "general_operand" "=fm")
	(float_truncate:SF
	  (match_operand:DF 1 "general_operand" "f")))]
  "TARGET_68881"
  "*
{
  if (FP_REG_P (operands[1]))
    {
      if (FP_REG_P (operands[0])) 
	return \"fmoveds %1,%0\";
      else
	{
	  output_asm_insn (\"fmoveds %1,%-\", operands);
	  return \"movl %+,%0\";
	}
    }
  if (FP_REG_P (operands[0])) 
    return \"fmoveds %1,%0\";
  return \"fmoveds %1,%0\";
}")


;; Conversion between fixed point and floating point.
;; Note that among the fix-to-float insns
;; the ones that start with SImode come first.
;; That is so that an operand that is a CONST_INT
;; (and therefore lacks a specific machine mode).
;; will be recognized as SImode (which is always valid)
;; rather than as QImode or HImode.

(define_expand "floatsisf2"
  [(set (match_operand:SF 0 "general_operand" "")
	(float:SF (match_operand:SI 1 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:SF 0 "general_operand" "=f")
	(float:SF (match_operand:SI 1 "general_operand" "dm")))]
  "TARGET_68881"
  "fmovels %1,%0")

(define_expand "floatsidf2"
  [(set (match_operand:DF 0 "general_operand" "")
	(float:DF (match_operand:SI 1 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:DF 0 "general_operand" "=f")
	(float:DF (match_operand:SI 1 "general_operand" "dm")))]
  "TARGET_68881"
  "fmoveld %1,%0")

(define_insn "floathisf2"
  [(set (match_operand:SF 0 "general_operand" "=f")
	(float:SF (match_operand:HI 1 "general_operand" "dm")))]
  "TARGET_68881"
  "fmovews %1,%0")

(define_insn "floathidf2"
  [(set (match_operand:DF 0 "general_operand" "=f")
	(float:DF (match_operand:HI 1 "general_operand" "dm")))]
  "TARGET_68881"
  "fmovewd %1,%0")

(define_insn "floatqisf2"
  [(set (match_operand:SF 0 "general_operand" "=f")
	(float:SF (match_operand:QI 1 "general_operand" "dm")))]
  "TARGET_68881"
  "fmovebs %1,%0")

(define_insn "floatqidf2"
  [(set (match_operand:DF 0 "general_operand" "=f")
	(float:DF (match_operand:QI 1 "general_operand" "dm")))]
  "TARGET_68881"
  "fmovebd %1,%0")

;; Convert a float to a float whose value is an integer.
;; This is the first stage of converting it to an integer type.

(define_insn "ftruncdf2"
  [(set (match_operand:DF 0 "general_operand" "=fm")
	(fix:DF (match_operand:DF 1 "general_operand" "fm")))]
  "TARGET_68881"
  "*
{
  if (FP_REG_P (operands[1]))
    {
      if (FP_REG_P (operands[0]))
	{
	  output_asm_insn (\"fmovedl %1,%-\", operands);
	  return \"fmoveld %+,%0\";
        }
      return \"fmovedl %1,%0\";
    }
  if (FP_REG_P (operands[0]))
    {
      output_asm_insn (\"fmoved %1,%0\", operands);
      output_asm_insn (\"fmovedl %0,%-\", operands);
      return \"fmoveld %+,%0\";
    }
  return \"fmovedl %1,%0\";
}")

(define_insn "ftruncsf2"
  [(set (match_operand:SF 0 "general_operand" "=fm")
	(fix:SF (match_operand:SF 1 "general_operand" "fm")))]
  "TARGET_68881"
  "*
{
  if (FP_REG_P (operands[1]))
    {
      if (FP_REG_P (operands[0]))
        {
	  output_asm_insn (\"fmovesl %1,%-\", operands);
	  return \"fmovels %+,%0\";
	}
      return \"fmovesl %1,%0\";
    }
  if (FP_REG_P (operands[0]))
    {
      output_asm_insn (\"fmovesl %1,%-\", operands);
      return \"fmovels %+,%0\";
    }
  return \"fmovesl %1,%0\";
}")

;; Convert a float whose value is an integer
;; to an actual integer.  Second stage of converting float to integer type.
(define_insn "fixsfqi2"
  [(set (match_operand:QI 0 "general_operand" "=m")
	(fix:QI (match_operand:SF 1 "general_operand" "f")))]
  "TARGET_68881"
  "*
{
  if (FP_REG_P (operands[0]))
    {
      output_asm_insn (\"fmovesb %1,%-\", operands);
      return \"fmovebs %+,%0\";
    }
  if (DATA_REG_P (operands[0]))
    {
      output_asm_insn (\"fmovesb %1,%-\", operands);
      return \"movb% %+,%0\";
    }
  return \"fmovesb %1,%0\";
}")

(define_insn "fixsfhi2"
  [(set (match_operand:HI 0 "general_operand" "=m")
	(fix:HI (match_operand:SF 1 "general_operand" "f")))]
  "TARGET_68881"
  "*
{
  if (FP_REG_P (operands[0]))
    {
      output_asm_insn (\"fmovesw %1,%-\", operands);
      return \"fmovews %+,%0\";
    }
  if (DATA_REG_P (operands[0]))
    {
      output_asm_insn (\"fmovesw %1,%-\", operands);
      return \"movw% %+,%0\";
    }
  return \"fmovesw %1,%0\";
}")

(define_insn "fixsfsi2"
  [(set (match_operand:SI 0 "general_operand" "=fdm")
	(fix:SI (match_operand:SF 1 "general_operand" "f")))]
  "TARGET_68881"
  "*
{
  if (FP_REG_P (operands[0]))
    {
      output_asm_insn (\"fmovesl %1,%-\", operands);
      return \"fmovels %+,%0\";
    }
  if (DATA_REG_P (operands[0]))
    {
      output_asm_insn (\"fmovesl %1,%-\", operands);
      return \"movl %+,%0\";
    }
  return \"fmovesl %1,%0\";
}")

(define_insn "fixdfqi2"
  [(set (match_operand:QI 0 "general_operand" "=fdm")
	(fix:QI (match_operand:DF 1 "general_operand" "f")))]
  "TARGET_68881"
  "*
{
  if (FP_REG_P (operands[0]))
    {
      output_asm_insn (\"fmovedb %1,%-\", operands);
      return \"fmovebd %+,%0\";
    }
  if (DATA_REG_P (operands[0]))
    {
      output_asm_insn (\"fmovedb %1,%-\", operands);
      return \"movb% %+,%0\";
    }
  return \"fmovedb %1,%0\";
}")

(define_insn "fixdfhi2"
  [(set (match_operand:HI 0 "general_operand" "=fdm")
	(fix:HI (match_operand:DF 1 "general_operand" "f")))]
  "TARGET_68881"
  "*
{
  if (FP_REG_P (operands[0]))
    {
      output_asm_insn (\"fmovedw %1,%-\", operands);
      return \"fmovewd %+,%0\";
    }
  if (DATA_REG_P (operands[0]))
    {
      output_asm_insn (\"fmovedw %1,%-\", operands);
      return \"movw% %+,%0\";
    }
  return \"fmovedw %1,%0\";
}")

(define_insn "fixdfsi2"
  [(set (match_operand:SI 0 "general_operand" "=fdm")
	(fix:SI (match_operand:DF 1 "general_operand" "f")))]
  "TARGET_68881"
  "*
{
  if (FP_REG_P (operands[0]))
    {
      output_asm_insn (\"fmovedl %1,%-\", operands);
      return \"fmoveld %+,%0\";
    }
  if (DATA_REG_P (operands[0]))
    {
      output_asm_insn (\"fmovedl %1,%-\", operands);
      return \"movl %+,%0\";
    }
  return \"fmovedl %1,%0\";
}")


;; add instructions

(define_insn "addsi3"
  [(set (match_operand:SI 0 "general_operand" "=m,r,!a,!a")
	(plus:SI (match_operand:SI 1 "general_operand" "%0,0,a,rJK")
		 (match_operand:SI 2 "general_operand" "dIKLs,mrIKLs,rJK,a")))]
  ""
  "*
{
  if (! operands_match_p (operands[0], operands[1]))
    {
      if (!ADDRESS_REG_P (operands[1]))
	{
	  rtx tmp = operands[1];

	  operands[1] = operands[2];
	  operands[2] = tmp;
	}

      /* These insns can result from reloads to access
	 stack slots over 64k from the frame pointer.  */
      if (GET_CODE (operands[2]) == CONST_INT
	  && INTVAL (operands[2]) + 0x8000 >= (unsigned) 0x10000)
        return \"movl %2,%0\;addl %1,%0\";
      if (GET_CODE (operands[2]) == REG)
	return \"lea %1@[%2:L:B],%0\";
      else
	return \"lea %1@(%c2),%0\";
    }
  if (GET_CODE (operands[2]) == CONST_INT)
    {
      if (INTVAL (operands[2]) > 0
	  && INTVAL (operands[2]) <= 8)
	return (ADDRESS_REG_P (operands[0])
		? \"addqw %2,%0\"
		: \"addql %2,%0\");
      if (INTVAL (operands[2]) < 0
	  && INTVAL (operands[2]) >= -8)
        {
	  operands[2] = gen_rtx (CONST_INT, VOIDmode,
			         - INTVAL (operands[2]));
	  return (ADDRESS_REG_P (operands[0])
		  ? \"subqw %2,%0\"
		  : \"subql %2,%0\");
	}
      if (ADDRESS_REG_P (operands[0])
	  && INTVAL (operands[2]) >= -0x8000
	  && INTVAL (operands[2]) < 0x8000)
	return \"addw %2,%0\";
    }
  return \"addl %2,%0\";
}")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=a")
	(plus:SI (match_operand:SI 1 "general_operand" "0")
		 (sign_extend:SI (match_operand:HI 2 "general_operand" "rmn"))))]
  ""
  "addw %2,%0")

(define_insn "addhi3"
  [(set (match_operand:HI 0 "general_operand" "=m,r")
	(plus:HI (match_operand:HI 1 "general_operand" "%0,0")
		 (match_operand:HI 2 "general_operand" "dn,rmn")))]
  ""
  "*
{
  if (GET_CODE (operands[2]) == CONST_INT)
    {
      if (INTVAL (operands[2]) > 0
	  && INTVAL (operands[2]) <= 8)
	return \"addqw %2,%0\";
    }
  if (GET_CODE (operands[2]) == CONST_INT)
    {
      if (INTVAL (operands[2]) < 0
	  && INTVAL (operands[2]) >= -8)
	{
	  operands[2] = gen_rtx (CONST_INT, VOIDmode,
			         - INTVAL (operands[2]));
	  return \"subqw %2,%0\";
	}
    }
  return \"addw %2,%0\";
}")

(define_insn ""
  [(set (strict_low_part (match_operand:HI 0 "general_operand" "+m,d"))
	(plus:HI (match_dup 0)
		 (match_operand:HI 1 "general_operand" "dn,rmn")))]
  ""
  "addw %1,%0")

(define_insn "addqi3"
  [(set (match_operand:QI 0 "general_operand" "=m,d")
	(plus:QI (match_operand:QI 1 "general_operand" "%0,0")
		 (match_operand:QI 2 "general_operand" "dn,dmn")))]
  ""
  "*
{
  if (GET_CODE (operands[2]) == CONST_INT)
    {
      if (INTVAL (operands[2]) > 0
	  && INTVAL (operands[2]) <= 8)
	return \"addqb %2,%0\";
    }
  if (GET_CODE (operands[2]) == CONST_INT)
    {
      if (INTVAL (operands[2]) < 0 && INTVAL (operands[2]) >= -8)
       {
	 operands[2] = gen_rtx (CONST_INT, VOIDmode, - INTVAL (operands[2]));
	 return \"subqb %2,%0\";
       }
    }
  return \"addb %2,%0\";
}")

(define_insn ""
  [(set (strict_low_part (match_operand:QI 0 "general_operand" "+m,d"))
	(plus:QI (match_dup 0)
		 (match_operand:QI 1 "general_operand" "dn,dmn")))]
  ""
  "addb %1,%0")

(define_expand "adddf3"
  [(set (match_operand:DF 0 "general_operand" "")
	(plus:DF (match_operand:DF 1 "general_operand" "")
		 (match_operand:DF 2 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:DF 0 "general_operand" "=f")
	(plus:DF (match_operand:DF 1 "general_operand" "%f")
		 (match_operand:DF 2 "general_operand" "fm")))]
  "TARGET_68881"
  "faddd %2,%1,%0")

(define_expand "addsf3"
  [(set (match_operand:SF 0 "general_operand" "")
	(plus:SF (match_operand:SF 1 "general_operand" "")
		 (match_operand:SF 2 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:SF 0 "general_operand" "=f")
	(plus:SF (match_operand:SF 1 "general_operand" "%f")
		 (match_operand:SF 2 "general_operand" "fm")))]
  "TARGET_68881"
  "fadds %2,%1,%0")

;; subtract instructions

(define_insn "subsi3"
  [(set (match_operand:SI 0 "general_operand" "=m,r,!a,?d")
	(minus:SI (match_operand:SI 1 "general_operand" "0,0,a,mrIKs")
		  (match_operand:SI 2 "general_operand" "dIKs,mrIKs,J,0")))]
  ""
  "*
{
  if (! operands_match_p (operands[0], operands[1]))
    {
      if (operands_match_p (operands[0], operands[2]))
	{
	  if (GET_CODE (operands[1]) == CONST_INT)
	    {
	      if (INTVAL (operands[1]) > 0
		  && INTVAL (operands[1]) <= 8)
		return \"subql %1,%0\;negl %0\";
	    }
	  return \"subl %1,%0\;negl %0\";
	}
      /* This case is matched by J, but negating -0x8000
         in an lea would give an invalid displacement.
	 So do this specially.  */
      if (INTVAL (operands[2]) == -0x8000)
	return \"movl %1,%0\;subl %2,%0\";
      return \"lea %1@(%n2),%0\";
    }
  if (GET_CODE (operands[2]) == CONST_INT)
    {
      if (INTVAL (operands[2]) > 0
	  && INTVAL (operands[2]) <= 8)
	return \"subql %2,%0\";
      if (ADDRESS_REG_P (operands[0])
	  && INTVAL (operands[2]) >= -0x8000
	  && INTVAL (operands[2]) < 0x8000)
	return \"subw %2,%0\";
    }
  return \"subl %2,%0\";
}")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=a")
	(minus:SI (match_operand:SI 1 "general_operand" "0")
		  (sign_extend:SI (match_operand:HI 2 "general_operand" "rmn"))))]
  ""
  "subw %2,%0")

(define_insn "subhi3"
  [(set (match_operand:HI 0 "general_operand" "=m,r")
	(minus:HI (match_operand:HI 1 "general_operand" "0,0")
		  (match_operand:HI 2 "general_operand" "dn,rmn")))]
  ""
  "subw %2,%0")

(define_insn ""
  [(set (strict_low_part (match_operand:HI 0 "general_operand" "+m,d"))
	(minus:HI (match_dup 0)
		  (match_operand:HI 1 "general_operand" "dn,rmn")))]
  ""
  "subw %1,%0")

(define_insn "subqi3"
  [(set (match_operand:QI 0 "general_operand" "=m,d")
	(minus:QI (match_operand:QI 1 "general_operand" "0,0")
		  (match_operand:QI 2 "general_operand" "dn,dmn")))]
  ""
  "subb %2,%0")

(define_insn ""
  [(set (strict_low_part (match_operand:QI 0 "general_operand" "+m,d"))
	(minus:QI (match_dup 0)
		  (match_operand:QI 1 "general_operand" "dn,dmn")))]
  ""
  "subb %1,%0")

(define_expand "subdf3"
  [(set (match_operand:DF 0 "general_operand" "")
	(minus:DF (match_operand:DF 1 "general_operand" "")
		  (match_operand:DF 2 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:DF 0 "general_operand" "=f")
	(minus:DF (match_operand:DF 1 "general_operand" "f")
		  (match_operand:DF 2 "general_operand" "fm")))]
  "TARGET_68881"
  "fsubd %2,%1,%0")

(define_expand "subsf3"
  [(set (match_operand:SF 0 "general_operand" "")
	(minus:SF (match_operand:SF 1 "general_operand" "")
		  (match_operand:SF 2 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:SF 0 "general_operand" "=f")
	(minus:SF (match_operand:SF 1 "general_operand" "f")
		  (match_operand:SF 2 "general_operand" "fm")))]
  "TARGET_68881"
  "fsubs %2,%1,%0")

;; multiply instructions

(define_insn "mulhi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(mult:HI (match_operand:HI 1 "general_operand" "%0")
		 (match_operand:HI 2 "general_operand" "dmn")))]
  ""
  "muls %2,%0")

(define_insn "mulhisi3"
  [(set (match_operand:SI 0 "general_operand" "=d")
	(mult:SI (match_operand:HI 1 "general_operand" "%0")
		 (match_operand:HI 2 "general_operand" "dmn")))]
  ""
  "muls %2,%0")

(define_insn "mulsi3"
  [(set (match_operand:SI 0 "general_operand" "=d")
	(mult:SI (match_operand:SI 1 "general_operand" "%0")
		 (match_operand:SI 2 "general_operand" "dmsK")))]
  "TARGET_68020"
  "mulsl %2,%0")

(define_insn "umulhi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(umult:HI (match_operand:HI 1 "general_operand" "%0")
		  (match_operand:HI 2 "general_operand" "dmn")))]
  ""
  "mulu %2,%0")

(define_insn "umulhisi3"
  [(set (match_operand:SI 0 "general_operand" "=d")
	(umult:SI (match_operand:HI 1 "general_operand" "%0")
		  (match_operand:HI 2 "general_operand" "dmn")))]
  ""
  "mulu %2,%0")

(define_insn "umulsi3"
  [(set (match_operand:SI 0 "general_operand" "=d")
	(umult:SI (match_operand:SI 1 "general_operand" "%0")
		  (match_operand:SI 2 "general_operand" "dmsK")))]
  "TARGET_68020"
  "mulul %2,%0")

(define_expand "muldf3"
  [(set (match_operand:DF 0 "general_operand" "")
	(mult:DF (match_operand:DF 1 "general_operand" "")
		 (match_operand:DF 2 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:DF 0 "general_operand" "=f")
	(mult:DF (match_operand:DF 1 "general_operand" "%f")
		 (match_operand:DF 2 "general_operand" "fm")))]
  "TARGET_68881"
  "fmuld %2,%1,%0")

(define_expand "mulsf3"
  [(set (match_operand:SF 0 "general_operand" "")
	(mult:SF (match_operand:SF 1 "general_operand" "")
		 (match_operand:SF 2 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:SF 0 "general_operand" "=f")
	(mult:SF (match_operand:SF 1 "general_operand" "%f")
		 (match_operand:SF 2 "general_operand" "fm")))]
  "TARGET_68881"
  "fmuls %2,%1,%0")

;; divide instructions

(define_insn "divhi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(div:HI (match_operand:HI 1 "general_operand" "0")
		(match_operand:HI 2 "general_operand" "dmn")))]
  ""
  "extl %0\;divs %2,%0")

(define_insn "divhisi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(div:HI (match_operand:SI 1 "general_operand" "0")
		(match_operand:HI 2 "general_operand" "dmn")))]
  ""
  "divs %2,%0")

(define_insn "divsi3"
  [(set (match_operand:SI 0 "general_operand" "=d")
	(div:SI (match_operand:SI 1 "general_operand" "0")
		(match_operand:SI 2 "general_operand" "dmsK")))]
  "TARGET_68020"
  "divsl %2,%0,%0")

(define_insn "udivhi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(udiv:HI (match_operand:HI 1 "general_operand" "0")
		 (match_operand:HI 2 "general_operand" "dmn")))]
  ""
  "andl %#0xFFFF,%0\;divu %2,%0")

(define_insn "udivhisi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(udiv:HI (match_operand:SI 1 "general_operand" "0")
		 (match_operand:HI 2 "general_operand" "dmn")))]
  ""
  "divu %2,%0")

(define_insn "udivsi3"
  [(set (match_operand:SI 0 "general_operand" "=d")
	(udiv:SI (match_operand:SI 1 "general_operand" "0")
		 (match_operand:SI 2 "general_operand" "dmsK")))]
  "TARGET_68020"
  "divul %2,%0,%0")

(define_expand "divdf3"
  [(set (match_operand:DF 0 "general_operand" "")
	(div:DF (match_operand:DF 1 "general_operand" "")
		(match_operand:DF 2 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:DF 0 "general_operand" "=f")
	(div:DF (match_operand:DF 1 "general_operand" "f")
		(match_operand:DF 2 "general_operand" "fm")))]
  "TARGET_68881"
  "fdivd %2,%1,%0")

(define_expand "divsf3"
  [(set (match_operand:SF 0 "general_operand" "")
	(div:SF (match_operand:SF 1 "general_operand" "")
		(match_operand:SF 2 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:SF 0 "general_operand" "=f")
	(div:SF (match_operand:SF 1 "general_operand" "0")
		(match_operand:SF 2 "general_operand" "fm")))]
  "TARGET_68881"
  "fdivs %2,%0,%0")

;; Remainder instructions.

(define_insn "modhi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(mod:HI (match_operand:HI 1 "general_operand" "0")
		(match_operand:HI 2 "general_operand" "dmn")))]
  ""
  "*
{
  /* The swap insn produces cc's that don't correspond to the result.  */
  CC_STATUS_INIT;
  return \"extl %0\;divs %2,%0\;swap %0\";
}")

(define_insn "modhisi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(mod:HI (match_operand:SI 1 "general_operand" "0")
		(match_operand:HI 2 "general_operand" "dmn")))]
  ""
  "*
{
  /* The swap insn produces cc's that don't correspond to the result.  */
  CC_STATUS_INIT;
  return \"divs %2,%0\;swap %0\";
}")

(define_insn "umodhi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(umod:HI (match_operand:HI 1 "general_operand" "0")
		 (match_operand:HI 2 "general_operand" "dmn")))]
  ""
  "*
{
  /* The swap insn produces cc's that don't correspond to the result.  */
  CC_STATUS_INIT;
  return \"andl %#0xFFFF,%0\;divu %2,%0\;swap %0\";
}")

(define_insn "umodhisi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(umod:HI (match_operand:SI 1 "general_operand" "0")
		 (match_operand:HI 2 "general_operand" "dmn")))]
  ""
  "*
{
  /* The swap insn produces cc's that don't correspond to the result.  */
  CC_STATUS_INIT;
  return \"divu %2,%0\;swap %0\";
}")

(define_insn "divmodsi4"
  [(set (match_operand:SI 0 "general_operand" "=d")
	(div:SI (match_operand:SI 1 "general_operand" "0")
		(match_operand:SI 2 "general_operand" "dmsK")))
   (set (match_operand:SI 3 "general_operand" "=d")
	(mod:SI (match_dup 1) (match_dup 2)))]
  "TARGET_68020"
  "divsl %2,%0,%3")

(define_insn "udivmodsi4"
  [(set (match_operand:SI 0 "general_operand" "=d")
	(udiv:SI (match_operand:SI 1 "general_operand" "0")
		 (match_operand:SI 2 "general_operand" "dmsK")))
   (set (match_operand:SI 3 "general_operand" "=d")
	(umod:SI (match_dup 1) (match_dup 2)))]
  "TARGET_68020"
  "divul %2,%0,%3")

;; logical-and instructions

(define_insn "andsi3"
  [(set (match_operand:SI 0 "general_operand" "=m,d")
	(and:SI (match_operand:SI 1 "general_operand" "%0,0")
		(match_operand:SI 2 "general_operand" "dKs,dmKs")))]
  ""
  "*
{
  if (GET_CODE (operands[2]) == CONST_INT
      && (INTVAL (operands[2]) | 0xffff) == 0xffffffff
      && (DATA_REG_P (operands[0])
	  || offsetable_memref_p (operands[0])))
    { 
      if (GET_CODE (operands[0]) != REG)
        operands[0] = adj_offsetable_operand (operands[0], 2);
      operands[2] = gen_rtx (CONST_INT, VOIDmode,
			     INTVAL (operands[2]) & 0xffff);
      /* Do not delete a following tstl %0 insn; that would be incorrect.  */
      CC_STATUS_INIT;
      if (operands[2] == const0_rtx)
        return \"clrw %0\";
      return \"andw %2,%0\";
    }
  return \"andl %2,%0\";
}")

(define_insn "andhi3"
  [(set (match_operand:HI 0 "general_operand" "=m,d")
	(and:HI (match_operand:HI 1 "general_operand" "%0,0")
		(match_operand:HI 2 "general_operand" "dn,dmn")))]
  ""
  "andw %2,%0")

(define_insn "andqi3"
  [(set (match_operand:QI 0 "general_operand" "=m,d")
	(and:QI (match_operand:QI 1 "general_operand" "%0,0")
		(match_operand:QI 2 "general_operand" "dn,dmn")))]
  ""
  "andb %2,%0")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=d")
	(and:SI (zero_extend:SI (match_operand:HI 1 "general_operand" "dm"))
		(match_operand:SI 2 "general_operand" "0")))]
  "GET_CODE (operands[2]) == CONST_INT
   && (unsigned int) INTVAL (operands[2]) < (1 << GET_MODE_BITSIZE (HImode))"
  "andw %1,%0")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=d")
	(and:SI (zero_extend:SI (match_operand:QI 1 "general_operand" "dm"))
		(match_operand:SI 2 "general_operand" "0")))]
  "GET_CODE (operands[2]) == CONST_INT
   && (unsigned int) INTVAL (operands[2]) < (1 << GET_MODE_BITSIZE (QImode))"
  "andb %1,%0")

;; inclusive-or instructions

(define_insn "iorsi3"
  [(set (match_operand:SI 0 "general_operand" "=m,d")
	(ior:SI (match_operand:SI 1 "general_operand" "%0,0")
		(match_operand:SI 2 "general_operand" "dKs,dmKs")))]
  ""
  "*
{
  register int logval;
  if (GET_CODE (operands[2]) == CONST_INT
      && INTVAL (operands[2]) >> 16 == 0
      && (DATA_REG_P (operands[0])
	  || offsetable_memref_p (operands[0])))
    { 
      if (GET_CODE (operands[0]) != REG)
        operands[0] = adj_offsetable_operand (operands[0], 2);
      /* Do not delete a following tstl %0 insn; that would be incorrect.  */
      CC_STATUS_INIT;
      return \"orw %2,%0\";
    }
  if (GET_CODE (operands[2]) == CONST_INT
      && (logval = exact_log2 (INTVAL (operands[2]))) >= 0
      && (DATA_REG_P (operands[0])
	  || offsetable_memref_p (operands[0])))
    { 
      if (DATA_REG_P (operands[0]))
	operands[1] = gen_rtx (CONST_INT, VOIDmode, logval);
      else
        {
	  operands[0] = adj_offsetable_operand (operands[0], 3 - (logval / 8));
	  operands[1] = gen_rtx (CONST_INT, VOIDmode, logval % 8);
	}
      return \"bset %1,%0\";
    }
  return \"orl %2,%0\";
}")

(define_insn "iorhi3"
  [(set (match_operand:HI 0 "general_operand" "=m,d")
	(ior:HI (match_operand:HI 1 "general_operand" "%0,0")
		(match_operand:HI 2 "general_operand" "dn,dmn")))]
  ""
  "orw %2,%0")

(define_insn "iorqi3"
  [(set (match_operand:QI 0 "general_operand" "=m,d")
	(ior:QI (match_operand:QI 1 "general_operand" "%0,0")
		(match_operand:QI 2 "general_operand" "dn,dmn")))]
  ""
  "orb %2,%0")

;; xor instructions

(define_insn "xorsi3"
  [(set (match_operand:SI 0 "general_operand" "=do,m")
	(xor:SI (match_operand:SI 1 "general_operand" "%0,0")
		(match_operand:SI 2 "general_operand" "di,dKs")))]
  ""
  "*
{
  if (GET_CODE (operands[2]) == CONST_INT
      && INTVAL (operands[2]) >> 16 == 0
      && (offsetable_memref_p (operands[0]) || DATA_REG_P (operands[0])))
    { 
      if (! DATA_REG_P (operands[0]))
	operands[0] = adj_offsetable_operand (operands[0], 2);
      /* Do not delete a following tstl %0 insn; that would be incorrect.  */
      CC_STATUS_INIT;
      return \"eorw %2,%0\";
    }
  return \"eorl %2,%0\";
}")

(define_insn "xorhi3"
  [(set (match_operand:HI 0 "general_operand" "=dm")
	(xor:HI (match_operand:HI 1 "general_operand" "%0")
		(match_operand:HI 2 "general_operand" "dn")))]
  ""
  "eorw %2,%0")

(define_insn "xorqi3"
  [(set (match_operand:QI 0 "general_operand" "=dm")
	(xor:QI (match_operand:QI 1 "general_operand" "%0")
		(match_operand:QI 2 "general_operand" "dn")))]
  ""
  "eorb %2,%0")

;; negation instructions

(define_insn "negsi2"
  [(set (match_operand:SI 0 "general_operand" "=dm")
	(neg:SI (match_operand:SI 1 "general_operand" "0")))]
  ""
  "negl %0")

(define_insn "neghi2"
  [(set (match_operand:HI 0 "general_operand" "=dm")
	(neg:HI (match_operand:HI 1 "general_operand" "0")))]
  ""
  "negw %0")

(define_insn "negqi2"
  [(set (match_operand:QI 0 "general_operand" "=dm")
	(neg:QI (match_operand:QI 1 "general_operand" "0")))]
  ""
  "negb %0")

(define_expand "negsf2"
  [(set (match_operand:SF 0 "general_operand" "")
	(neg:SF (match_operand:SF 1 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:SF 0 "general_operand" "=f")
	(neg:SF (match_operand:SF 1 "general_operand" "fdm")))]
  "TARGET_68881"
  "*
{
  if (REG_P (operands[1]) && ! DATA_REG_P (operands[1]))
    return \"fnegs %1,%0\";
  return \"fnegs %1,%0\";
}")

(define_expand "negdf2"
  [(set (match_operand:DF 0 "general_operand" "")
	(neg:DF (match_operand:DF 1 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:DF 0 "general_operand" "=f")
	(neg:DF (match_operand:DF 1 "general_operand" "fm")))]
  "TARGET_68881"
  "*
{
  if (REG_P (operands[1]) && ! DATA_REG_P (operands[1]))
    return \"fnegd %1,%0\";
  return \"fnegd %1,%0\";
}")

;; Absolute value instructions

(define_expand "abssf2"
  [(set (match_operand:SF 0 "general_operand" "")
	(abs:SF (match_operand:SF 1 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:SF 0 "general_operand" "=f")
	(abs:SF (match_operand:SF 1 "general_operand" "fdm")))]
  "TARGET_68881"
  "*
{
  if (REG_P (operands[1]) && ! DATA_REG_P (operands[1]))
    return \"fabss %1,%0\";
  return \"fabss %1,%0\";
}")

(define_expand "absdf2"
  [(set (match_operand:DF 0 "general_operand" "")
	(abs:DF (match_operand:DF 1 "general_operand" "")))]
  "TARGET_68881"
  "")

(define_insn ""
  [(set (match_operand:DF 0 "general_operand" "=f")
	(abs:DF (match_operand:DF 1 "general_operand" "fm")))]
  "TARGET_68881"
  "*
{
  if (REG_P (operands[1]) && ! DATA_REG_P (operands[1]))
    return \"fabsd %1,%0\";
  return \"fabsd %1,%0\";
}")

;; one complement instructions

(define_insn "one_cmplsi2"
  [(set (match_operand:SI 0 "general_operand" "=dm")
	(not:SI (match_operand:SI 1 "general_operand" "0")))]
  ""
  "notl %0")

(define_insn "one_cmplhi2"
  [(set (match_operand:HI 0 "general_operand" "=dm")
	(not:HI (match_operand:HI 1 "general_operand" "0")))]
  ""
  "notw %0")

(define_insn "one_cmplqi2"
  [(set (match_operand:QI 0 "general_operand" "=dm")
	(not:QI (match_operand:QI 1 "general_operand" "0")))]
  ""
  "notb %0")

;; Optimized special case of shifting.
;; Must precede the general case.

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=d")
	(ashiftrt:SI (match_operand:SI 1 "memory_operand" "m")
		     (const_int 24)))]
  "GET_CODE (XEXP (operands[1], 0)) != POST_INC
   && GET_CODE (XEXP (operands[1], 0)) != PRE_DEC"
  "*
{
  if (TARGET_68020)
    return \"movb %1,%0\;extbl %0\";
  return \"movb %1,%0\;extw %0\;extl %0\";
}")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=d")
	(lshiftrt:SI (match_operand:SI 1 "memory_operand" "m")
		     (const_int 24)))]
  "GET_CODE (XEXP (operands[1], 0)) != POST_INC
   && GET_CODE (XEXP (operands[1], 0)) != PRE_DEC"
  "*
{
  if (reg_mentioned_p (operands[0], operands[1]))
    return \"movb %1,%0\;andl %#0xFF,%0\";
  return \"clrl %0\;movb %1,%0\";
}")

(define_insn ""
  [(set (cc0) (compare (match_operand:QI 0 "general_operand" "i")
		       (lshiftrt:SI (match_operand:SI 1 "memory_operand" "m")
				    (const_int 24))))]
  "(GET_CODE (operands[0]) == CONST_INT
    && (INTVAL (operands[0]) & ~0xff) == 0)"
  "* cc_status.flags |= CC_REVERSED;
  return \"cmpb %0,%1\";
")

(define_insn ""
  [(set (cc0) (compare (lshiftrt:SI (match_operand:SI 0 "memory_operand" "m")
				    (const_int 24))
		       (match_operand:QI 1 "general_operand" "i")))]
  "(GET_CODE (operands[1]) == CONST_INT
    && (INTVAL (operands[1]) & ~0xff) == 0)"
  "*
  return \"cmpb %1,%0\";
")

(define_insn ""
  [(set (cc0) (compare (match_operand:QI 0 "general_operand" "i")
		       (ashiftrt:SI (match_operand:SI 1 "memory_operand" "m")
				    (const_int 24))))]
  "(GET_CODE (operands[0]) == CONST_INT
    && ((INTVAL (operands[0]) + 0x80) & ~0xff) == 0)"
  "* cc_status.flags |= CC_REVERSED;
  return \"cmpb %0,%1\";
")

(define_insn ""
  [(set (cc0) (compare (ashiftrt:SI (match_operand:SI 0 "memory_operand" "m")
				    (const_int 24))
		       (match_operand:QI 1 "general_operand" "i")))]
  "(GET_CODE (operands[1]) == CONST_INT
    && ((INTVAL (operands[1]) + 0x80) & ~0xff) == 0)"
  "*
  return \"cmpb %1,%0\";
")

;; arithmetic shift instructions
;; We don't need the shift memory by 1 bit instruction

(define_insn "ashlsi3"
  [(set (match_operand:SI 0 "general_operand" "=d")
	(ashift:SI (match_operand:SI 1 "general_operand" "0")
		   (match_operand:SI 2 "general_operand" "dI")))]
  ""
  "asll %2,%0")

(define_insn "ashlhi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(ashift:HI (match_operand:HI 1 "general_operand" "0")
		   (match_operand:HI 2 "general_operand" "dI")))]
  ""
  "aslw %2,%0")

(define_insn "ashlqi3"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(ashift:QI (match_operand:QI 1 "general_operand" "0")
		   (match_operand:QI 2 "general_operand" "dI")))]
  ""
  "aslb %2,%0")

(define_insn "ashrsi3"
  [(set (match_operand:SI 0 "general_operand" "=d")
	(ashiftrt:SI (match_operand:SI 1 "general_operand" "0")
		     (match_operand:SI 2 "general_operand" "dI")))]
  ""
  "asrl %2,%0")

(define_insn "ashrhi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(ashiftrt:HI (match_operand:HI 1 "general_operand" "0")
		     (match_operand:HI 2 "general_operand" "dI")))]
  ""
  "asrw %2,%0")

(define_insn "ashrqi3"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(ashiftrt:QI (match_operand:QI 1 "general_operand" "0")
		     (match_operand:QI 2 "general_operand" "dI")))]
  ""
  "asrb %2,%0")

;; logical shift instructions

(define_insn "lshlsi3"
  [(set (match_operand:SI 0 "general_operand" "=d")
	(lshift:SI (match_operand:SI 1 "general_operand" "0")
		   (match_operand:SI 2 "general_operand" "dI")))]
  ""
  "lsll %2,%0")

(define_insn "lshlhi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(lshift:HI (match_operand:HI 1 "general_operand" "0")
		   (match_operand:HI 2 "general_operand" "dI")))]
  ""
  "lslw %2,%0")

(define_insn "lshlqi3"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(lshift:QI (match_operand:QI 1 "general_operand" "0")
		   (match_operand:QI 2 "general_operand" "dI")))]
  ""
  "lslb %2,%0")

(define_insn "lshrsi3"
  [(set (match_operand:SI 0 "general_operand" "=d")
	(lshiftrt:SI (match_operand:SI 1 "general_operand" "0")
		     (match_operand:SI 2 "general_operand" "dI")))]
  ""
  "lsrl %2,%0")

(define_insn "lshrhi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(lshiftrt:HI (match_operand:HI 1 "general_operand" "0")
		     (match_operand:HI 2 "general_operand" "dI")))]
  ""
  "lsrw %2,%0")

(define_insn "lshrqi3"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(lshiftrt:QI (match_operand:QI 1 "general_operand" "0")
		     (match_operand:QI 2 "general_operand" "dI")))]
  ""
  "lsrb %2,%0")

;; rotate instructions

(define_insn "rotlsi3"
  [(set (match_operand:SI 0 "general_operand" "=d")
	(rotate:SI (match_operand:SI 1 "general_operand" "0")
		   (match_operand:SI 2 "general_operand" "dI")))]
  ""
  "roll %2,%0")

(define_insn "rotlhi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(rotate:HI (match_operand:HI 1 "general_operand" "0")
		   (match_operand:HI 2 "general_operand" "dI")))]
  ""
  "rolw %2,%0")

(define_insn "rotlqi3"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(rotate:QI (match_operand:QI 1 "general_operand" "0")
		   (match_operand:QI 2 "general_operand" "dI")))]
  ""
  "rolb %2,%0")

(define_insn "rotrsi3"
  [(set (match_operand:SI 0 "general_operand" "=d")
	(rotatert:SI (match_operand:SI 1 "general_operand" "0")
		     (match_operand:SI 2 "general_operand" "dI")))]
  ""
  "rorl %2,%0")

(define_insn "rotrhi3"
  [(set (match_operand:HI 0 "general_operand" "=d")
	(rotatert:HI (match_operand:HI 1 "general_operand" "0")
		     (match_operand:HI 2 "general_operand" "dI")))]
  ""
  "rorw %2,%0")

(define_insn "rotrqi3"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(rotatert:QI (match_operand:QI 1 "general_operand" "0")
		     (match_operand:QI 2 "general_operand" "dI")))]
  ""
  "rorb %2,%0")

;; Special cases of bit-field insns which we should
;; recognize in preference to the general case.
;; These handle aligned 8-bit and 16-bit fields,
;; which can usually be done with move instructions.

(define_insn ""
  [(set (zero_extract:SI (match_operand:SI 0 "nonimmediate_operand" "+do")
			 (match_operand:SI 1 "immediate_operand" "i")
			 (match_operand:SI 2 "immediate_operand" "i"))
	(match_operand:SI 3 "general_operand" "d"))]
  "TARGET_68020 && TARGET_BITFIELD
   && GET_CODE (operands[1]) == CONST_INT
   && (INTVAL (operands[1]) == 8 || INTVAL (operands[1]) == 16)
   && GET_CODE (operands[2]) == CONST_INT
   && INTVAL (operands[2]) % INTVAL (operands[1]) == 0
   && (GET_CODE (operands[0]) == REG
       || ! mode_dependent_address_p (XEXP (operands[0], 0)))"
  "*
{
  if (REG_P (operands[0]))
    {
      if (INTVAL (operands[1]) + INTVAL (operands[2]) != 32)
        return \"bfins %3,[%c2,%c1]%0\";
    }
  else
    operands[0]
      = adj_offsetable_operand (operands[0], INTVAL (operands[2]) / 8);

  if (GET_CODE (operands[3]) == MEM)
    operands[3] = adj_offsetable_operand (operands[3],
					  (32 - INTVAL (operands[1])) / 8);
  if (INTVAL (operands[1]) == 8)
    return \"movb %3,%0\";
  return \"movw %3,%0\";
}")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=&d")
	(zero_extract:SI (match_operand:SI 1 "nonimmediate_operand" "do")
			 (match_operand:SI 2 "immediate_operand" "i")
			 (match_operand:SI 3 "immediate_operand" "i")))]
  "TARGET_68020 && TARGET_BITFIELD
   && GET_CODE (operands[2]) == CONST_INT
   && (INTVAL (operands[2]) == 8 || INTVAL (operands[2]) == 16)
   && GET_CODE (operands[3]) == CONST_INT
   && INTVAL (operands[3]) % INTVAL (operands[2]) == 0
   && (GET_CODE (operands[1]) == REG
       || ! mode_dependent_address_p (XEXP (operands[1], 0)))"
  "*
{
  if (REG_P (operands[1]))
    {
      if (INTVAL (operands[2]) + INTVAL (operands[3]) != 32)
	return \"bfextu [%c3,%c2]%1,%0\";
    }
  else
    operands[1]
      = adj_offsetable_operand (operands[1], INTVAL (operands[3]) / 8);

  output_asm_insn (\"clrl %0\", operands);
  if (GET_CODE (operands[0]) == MEM)
    operands[0] = adj_offsetable_operand (operands[0],
					  (32 - INTVAL (operands[1])) / 8);
  if (INTVAL (operands[2]) == 8)
    return \"movb %1,%0\";
  return \"movw %1,%0\";
}")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=d")
	(sign_extract:SI (match_operand:SI 1 "nonimmediate_operand" "do")
			 (match_operand:SI 2 "immediate_operand" "i")
			 (match_operand:SI 3 "immediate_operand" "i")))]
  "TARGET_68020 && TARGET_BITFIELD
   && GET_CODE (operands[2]) == CONST_INT
   && (INTVAL (operands[2]) == 8 || INTVAL (operands[2]) == 16)
   && GET_CODE (operands[3]) == CONST_INT
   && INTVAL (operands[3]) % INTVAL (operands[2]) == 0
   && (GET_CODE (operands[1]) == REG
       || ! mode_dependent_address_p (XEXP (operands[1], 0)))"
  "*
{
  if (REG_P (operands[1]))
    {
      if (INTVAL (operands[2]) + INTVAL (operands[3]) != 32)
	return \"bfexts [%c3,%c2]%1,%0\";
    }
  else
    operands[1]
      = adj_offsetable_operand (operands[1], INTVAL (operands[3]) / 8);

  if (INTVAL (operands[2]) == 8)
    return \"movb %1,%0\;extbl %0\";
  return \"movw %1,%0\;extl %0\";
}")

;; Bit field instructions, general cases.
;; "o,d" constraint causes a nonoffsetable memref to match the "o"
;; so that its address is reloaded.

(define_insn "extv"
  [(set (match_operand:SI 0 "general_operand" "=d,d")
	(sign_extract:SI (match_operand:QI 1 "nonimmediate_operand" "o,d")
			 (match_operand:SI 2 "general_operand" "di,di")
			 (match_operand:SI 3 "general_operand" "di,di")))]
  "TARGET_68020 && TARGET_BITFIELD"
  "bfexts [%c3,%c2]%1,%0")

(define_insn "extzv"
  [(set (match_operand:SI 0 "general_operand" "=d,d")
	(zero_extract:SI (match_operand:QI 1 "nonimmediate_operand" "o,d")
			 (match_operand:SI 2 "general_operand" "di,di")
			 (match_operand:SI 3 "general_operand" "di,di")))]
  "TARGET_68020 && TARGET_BITFIELD"
  "bfextu [%c3,%c2]%1,%0")

(define_insn ""
  [(set (zero_extract:SI (match_operand:QI 0 "nonimmediate_operand" "+o,d")
			 (match_operand:SI 1 "general_operand" "di,di")
			 (match_operand:SI 2 "general_operand" "di,di"))
        (xor:SI (zero_extract:SI (match_dup 0) (match_dup 1) (match_dup 2))
		(match_operand 3 "immediate_operand" "i,i")))]
  "TARGET_68020 && TARGET_BITFIELD
   && GET_CODE (operands[3]) == CONST_INT
   && (INTVAL (operands[3]) == -1
       || (GET_CODE (operands[1]) == CONST_INT
           && (~ INTVAL (operands[3]) & ((1 << INTVAL (operands[1]))- 1)) == 0))"
  "bfchg [%c2,%c1]%0")

(define_insn ""
  [(set (zero_extract:SI (match_operand:QI 0 "nonimmediate_operand" "+o,d")
			 (match_operand:SI 1 "general_operand" "di,di")
			 (match_operand:SI 2 "general_operand" "di,di"))
	(const_int 0))]
  "TARGET_68020 && TARGET_BITFIELD"
  "bfclr [%c2,%c1]%0")

(define_insn ""
  [(set (zero_extract:SI (match_operand:QI 0 "nonimmediate_operand" "+o,d")
			 (match_operand:SI 1 "general_operand" "di,di")
			 (match_operand:SI 2 "general_operand" "di,di"))
	(const_int -1))]
  "TARGET_68020 && TARGET_BITFIELD"
  "bfset [%c2,%c1]%0")

(define_insn "insv"
  [(set (zero_extract:SI (match_operand:QI 0 "nonimmediate_operand" "+o,d")
			 (match_operand:SI 1 "general_operand" "di,di")
			 (match_operand:SI 2 "general_operand" "di,di"))
	(match_operand:SI 3 "general_operand" "d,d"))]
  "TARGET_68020 && TARGET_BITFIELD"
  "bfins %3,[%c2,%c1]%0")

;; Now recognize bit field insns that operate on registers
;; (or at least were intended to do so).

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=d")
	(sign_extract:SI (match_operand:SI 1 "nonimmediate_operand" "d")
			 (match_operand:SI 2 "general_operand" "di")
			 (match_operand:SI 3 "general_operand" "di")))]
  "TARGET_68020 && TARGET_BITFIELD"
  "bfexts [%c3,%c2]%1,%0")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=d")
	(zero_extract:SI (match_operand:SI 1 "nonimmediate_operand" "d")
			 (match_operand:SI 2 "general_operand" "di")
			 (match_operand:SI 3 "general_operand" "di")))]
  "TARGET_68020 && TARGET_BITFIELD"
  "bfextu [%c3,%c2]%1,%0")

(define_insn ""
  [(set (zero_extract:SI (match_operand:SI 0 "nonimmediate_operand" "+d")
			 (match_operand:SI 1 "general_operand" "di")
			 (match_operand:SI 2 "general_operand" "di"))
	(const_int 0))]
  "TARGET_68020 && TARGET_BITFIELD"
  "bfclr [%c2,%c1]%0")

(define_insn ""
  [(set (zero_extract:SI (match_operand:SI 0 "nonimmediate_operand" "+d")
			 (match_operand:SI 1 "general_operand" "di")
			 (match_operand:SI 2 "general_operand" "di"))
	(const_int -1))]
  "TARGET_68020 && TARGET_BITFIELD"
  "bfset [%c2,%c1]%0")

(define_insn ""
  [(set (zero_extract:SI (match_operand:SI 0 "nonimmediate_operand" "+d")
			 (match_operand:SI 1 "general_operand" "di")
			 (match_operand:SI 2 "general_operand" "di"))
	(match_operand:SI 3 "general_operand" "d"))]
  "TARGET_68020 && TARGET_BITFIELD"
  "*
{
  return \"bfins %3,[%c2,%c1]%0\";
}")

;; Special patterns for optimizing bit-field instructions.

(define_insn ""
  [(set (cc0)
	(zero_extract:SI (match_operand:QI 0 "memory_operand" "o")
			 (match_operand:SI 1 "general_operand" "di")
			 (match_operand:SI 2 "general_operand" "di")))]
  "TARGET_68020 && TARGET_BITFIELD
   && GET_CODE (operands[1]) == CONST_INT"
  "*
{
  if (operands[1] == const1_rtx
      && GET_CODE (operands[2]) == CONST_INT)
    {    
      int width = GET_CODE (operands[0]) == REG ? 31 : 7;
      return output_btst (operands,
			  gen_rtx (CONST_INT, VOIDmode,
				   width - INTVAL (operands[2])),
			  operands[0],
			  insn, 1000);
      /* Pass 1000 as SIGNPOS argument so that btst will
         not think we are testing the sign bit for an `and'
	 and assume that nonzero implies a negative result.  */
    }
  if (INTVAL (operands[1]) != 32)
    cc_status.flags = CC_NOT_NEGATIVE;
  return \"bftst [%c2,%c1]%0\";
}")

(define_insn ""
  [(set (cc0)
	(subreg:QI
	 (zero_extract:SI (match_operand:QI 0 "memory_operand" "o")
			  (match_operand:SI 1 "general_operand" "di")
			  (match_operand:SI 2 "general_operand" "di"))
	 0))]
  "TARGET_68020 && TARGET_BITFIELD
   && GET_CODE (operands[1]) == CONST_INT"
  "*
{
  if (operands[1] == const1_rtx
      && GET_CODE (operands[2]) == CONST_INT)
    {    
      int width = GET_CODE (operands[0]) == REG ? 31 : 7;
      return output_btst (operands,
			  gen_rtx (CONST_INT, VOIDmode,
				   width - INTVAL (operands[2])),
			  operands[0],
			  insn, 1000);
      /* Pass 1000 as SIGNPOS argument so that btst will
         not think we are testing the sign bit for an `and'
	 and assume that nonzero implies a negative result.  */
    }
  if (INTVAL (operands[1]) != 32)
    cc_status.flags = CC_NOT_NEGATIVE;
  return \"bftst [%c2,%c1]%0\";
}")

(define_insn ""
  [(set (cc0)
	(subreg:HI
	 (zero_extract:SI (match_operand:QI 0 "memory_operand" "o")
			  (match_operand:SI 1 "general_operand" "di")
			  (match_operand:SI 2 "general_operand" "di"))
	 0))]
  "TARGET_68020 && TARGET_BITFIELD
   && GET_CODE (operands[1]) == CONST_INT"
  "*
{
  if (operands[1] == const1_rtx
      && GET_CODE (operands[2]) == CONST_INT)
    {    
      int width = GET_CODE (operands[0]) == REG ? 31 : 7;
      return output_btst (operands,
			  gen_rtx (CONST_INT, VOIDmode,
				   width - INTVAL (operands[2])),
			  operands[0],
			  insn, 1000);
      /* Pass 1000 as SIGNPOS argument so that btst will
         not think we are testing the sign bit for an `and'
	 and assume that nonzero implies a negative result.  */
    }
  if (INTVAL (operands[1]) != 32)
    cc_status.flags = CC_NOT_NEGATIVE;
  return \"bftst [%c2,%c1]%0\";
}")
  
;;; now handle the register cases
(define_insn ""
  [(set (cc0)
	(zero_extract:SI (match_operand:SI 0 "nonimmediate_operand" "d")
			 (match_operand:SI 1 "general_operand" "di")
			 (match_operand:SI 2 "general_operand" "di")))]
  "TARGET_68020 && TARGET_BITFIELD
   && GET_CODE (operands[1]) == CONST_INT"
  "*
{
  if (operands[1] == const1_rtx
      && GET_CODE (operands[2]) == CONST_INT)
    {    
      int width = GET_CODE (operands[0]) == REG ? 31 : 7;
      return output_btst (operands,
			  gen_rtx (CONST_INT, VOIDmode,
				   width - INTVAL (operands[2])),
			  operands[0],
			  insn, 1000);
      /* Pass 1000 as SIGNPOS argument so that btst will
         not think we are testing the sign bit for an `and'
	 and assume that nonzero implies a negative result.  */
    }
  if (INTVAL (operands[1]) != 32)
    cc_status.flags = CC_NOT_NEGATIVE;
  return \"bftst [%c2,%c1]%0\";
}")

(define_insn ""
  [(set (cc0)
	(subreg:QI
	 (zero_extract:SI (match_operand:SI 0 "nonimmediate_operand" "d")
			  (match_operand:SI 1 "general_operand" "di")
			  (match_operand:SI 2 "general_operand" "di"))
	 0))]
  "TARGET_68020 && TARGET_BITFIELD
   && GET_CODE (operands[1]) == CONST_INT"
  "*
{
  if (operands[1] == const1_rtx
      && GET_CODE (operands[2]) == CONST_INT)
    {    
      int width = GET_CODE (operands[0]) == REG ? 31 : 7;
      return output_btst (operands,
			  gen_rtx (CONST_INT, VOIDmode,
				   width - INTVAL (operands[2])),
			  operands[0],
			  insn, 1000);
      /* Pass 1000 as SIGNPOS argument so that btst will
         not think we are testing the sign bit for an `and'
	 and assume that nonzero implies a negative result.  */
    }
  if (INTVAL (operands[1]) != 32)
    cc_status.flags = CC_NOT_NEGATIVE;
  return \"bftst [%c2,%c1]%0\";
}")

(define_insn ""
  [(set (cc0)
	(subreg:HI
	 (zero_extract:SI (match_operand:SI 0 "nonimmediate_operand" "d")
			  (match_operand:SI 1 "general_operand" "di")
			  (match_operand:SI 2 "general_operand" "di"))
	 0))]
  "TARGET_68020 && TARGET_BITFIELD
   && GET_CODE (operands[1]) == CONST_INT"
  "*
{
  if (operands[1] == const1_rtx
      && GET_CODE (operands[2]) == CONST_INT)
    {    
      int width = GET_CODE (operands[0]) == REG ? 31 : 7;
      return output_btst (operands,
			  gen_rtx (CONST_INT, VOIDmode,
				   width - INTVAL (operands[2])),
			  operands[0],
			  insn, 1000);
      /* Pass 1000 as SIGNPOS argument so that btst will
         not think we are testing the sign bit for an `and'
	 and assume that nonzero implies a negative result.  */
    }
  if (INTVAL (operands[1]) != 32)
    cc_status.flags = CC_NOT_NEGATIVE;
  return \"bftst [%c2,%c1]%0\";
}")

(define_insn "seq"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(eq (cc0) (const_int 0)))]
  ""
  "*
  cc_status = cc_prev_status;
  OUTPUT_JUMP (\"seq %0\", \"fseq %0\", \"seq %0\");
")

(define_insn "sne"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(ne (cc0) (const_int 0)))]
  ""
  "*
  cc_status = cc_prev_status;
  OUTPUT_JUMP (\"sne %0\", \"fsne %0\", \"sne %0\");
")

(define_insn "sgt"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(gt (cc0) (const_int 0)))]
  ""
  "*
  cc_status = cc_prev_status;
  OUTPUT_JUMP (\"sgt %0\", \"fsgt %0\", \"andb %#0xc,%!\;sgt %0\");
")

(define_insn "sgtu"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(gtu (cc0) (const_int 0)))]
  ""
  "* cc_status = cc_prev_status;
     return \"shi %0\"; ")

(define_insn "slt"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(lt (cc0) (const_int 0)))]
  ""
  "* cc_status = cc_prev_status;
     OUTPUT_JUMP (\"slt %0\", \"fslt %0\", \"smi %0\"); ")

(define_insn "sltu"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(ltu (cc0) (const_int 0)))]
  ""
  "* cc_status = cc_prev_status;
     return \"scs %0\"; ")

(define_insn "sge"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(ge (cc0) (const_int 0)))]
  ""
  "* cc_status = cc_prev_status;
     OUTPUT_JUMP (\"sge %0\", \"fsge %0\", \"spl %0\"); ")

(define_insn "sgeu"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(geu (cc0) (const_int 0)))]
  ""
  "* cc_status = cc_prev_status;
     return \"scc %0\"; ")

(define_insn "sle"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(le (cc0) (const_int 0)))]
  ""
  "*
  cc_status = cc_prev_status;
  OUTPUT_JUMP (\"sle %0\", \"fsle %0\", \"andb %#0xc,%!\;sle %0\");
")

(define_insn "sleu"
  [(set (match_operand:QI 0 "general_operand" "=d")
	(leu (cc0) (const_int 0)))]
  ""
  "* cc_status = cc_prev_status;
     return \"sls %0\"; ")

;; Basic conditional jump instructions.

(define_insn "beq"
  [(set (pc)
	(if_then_else (eq (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "*
{
  OUTPUT_JUMP (\"jeq %l0\", \"fbeq %l0\", \"jeq %l0\");
}")

(define_insn "bne"
  [(set (pc)
	(if_then_else (ne (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "*
{
  OUTPUT_JUMP (\"jne %l0\", \"fbneq %l0\", \"jne %l0\");
}")

(define_insn "bgt"
  [(set (pc)
	(if_then_else (gt (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "*
  OUTPUT_JUMP (\"jgt %l0\", \"fbgt %l0\", \"andb %#0xc,%!\;jgt %l0\");
")

(define_insn "bgtu"
  [(set (pc)
	(if_then_else (gtu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "*
  return \"jhi %l0\";
")

(define_insn "blt"
  [(set (pc)
	(if_then_else (lt (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "*
  OUTPUT_JUMP (\"jlt %l0\", \"fblt %l0\", \"jmi %l0\");
")

(define_insn "bltu"
  [(set (pc)
	(if_then_else (ltu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "*
  return \"jcs %l0\";
")

(define_insn "bge"
  [(set (pc)
	(if_then_else (ge (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "*
  OUTPUT_JUMP (\"jge %l0\", \"fbge %l0\", \"jpl %l0\");
")

(define_insn "bgeu"
  [(set (pc)
	(if_then_else (geu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "*
  return \"jcc %l0\";
")

(define_insn "ble"
  [(set (pc)
	(if_then_else (le (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "*
  OUTPUT_JUMP (\"jle %l0\", \"fble %l0\", \"andb %#0xc,%!\;jle %l0\");
")

(define_insn "bleu"
  [(set (pc)
	(if_then_else (leu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "*
  return \"jls %l0\";
")

;; Negated conditional jump instructions.

(define_insn ""
  [(set (pc)
	(if_then_else (eq (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "*
{
  OUTPUT_JUMP (\"jne %l0\", \"fbneq %l0\", \"jne %l0\");
}")

(define_insn ""
  [(set (pc)
	(if_then_else (ne (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "*
{
  OUTPUT_JUMP (\"jeq %l0\", \"fbeq %l0\", \"jeq %l0\");
}")

(define_insn ""
  [(set (pc)
	(if_then_else (gt (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "*
  OUTPUT_JUMP (\"jle %l0\", \"fbngt %l0\", \"andb %#0xc,%!\;jle %l0\");
")

(define_insn ""
  [(set (pc)
	(if_then_else (gtu (cc0)
			   (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "*
  return \"jls %l0\";
")

(define_insn ""
  [(set (pc)
	(if_then_else (lt (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "*
  OUTPUT_JUMP (\"jge %l0\", \"fbnlt %l0\", \"jpl %l0\");
")

(define_insn ""
  [(set (pc)
	(if_then_else (ltu (cc0)
			   (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "*
  return \"jcc %l0\";
")

(define_insn ""
  [(set (pc)
	(if_then_else (ge (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "*
  OUTPUT_JUMP (\"jlt %l0\", \"fbnge %l0\", \"jmi %l0\");
")

(define_insn ""
  [(set (pc)
	(if_then_else (geu (cc0)
			   (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "*
  return \"jcs %l0\";
")

(define_insn ""
  [(set (pc)
	(if_then_else (le (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "*
  OUTPUT_JUMP (\"jgt %l0\", \"fbnle %l0\", \"andb %#0xc,%!\;jgt %l0\");
")

(define_insn ""
  [(set (pc)
	(if_then_else (leu (cc0)
			   (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "*
  return \"jhi %l0\";
")

;; Subroutines of "casesi".

(define_expand "casesi_1"
  [(set (match_operand:SI 3 "general_operand" "")
	(plus:SI (match_operand:SI 0 "general_operand" "")
		 ;; Note operand 1 has been negated!
		 (match_operand:SI 1 "immediate_operand" "")))
   (set (cc0) (compare (match_operand:SI 2 "general_operand" "")
		       (match_dup 3)))
   (set (pc) (if_then_else (ltu (cc0) (const_int 0))
			   (label_ref (match_operand 4 "" "")) (pc)))]
  ""
  "")

(define_expand "casesi_2"
  [(set (match_operand:SI 0 "" "") (mem:HI (match_operand:SI 1 "" "")))
   ;; The USE here is so that at least one jump-insn will refer to the label,
   ;; to keep it alive in jump_optimize.
   (parallel [(set (pc)
		   (plus:SI (pc) (match_dup 0)))
	      (use (label_ref (match_operand 2 "" "")))])]
  ""
  "")

;; Operand 0 is index (in bytes); operand 1 is minimum, operand 2 the maximum;
;; operand 3 is CODE_LABEL for the table;
;; operand 4 is the CODE_LABEL to go to if index out of range.
(define_expand "casesi"
  ;; We don't use these for generating the RTL, but we must describe
  ;; the operands here.
  [(match_operand:SI 0 "general_operand" "")
   (match_operand:SI 1 "immediate_operand" "")
   (match_operand:SI 2 "general_operand" "")
   (match_operand 3 "" "")
   (match_operand 4 "" "")]
  ""
  "
{
  rtx table_elt_addr;
  rtx index_diff;

  operands[1] = negate_rtx (SImode, operands[1]);
  index_diff = gen_reg_rtx (SImode);
  /* Emit the first few insns.  */
  emit_insn (gen_casesi_1 (operands[0], operands[1], operands[2],
			   index_diff, operands[4]));
  /* Construct a memory address.  This may emit some insns.  */
  table_elt_addr
    = memory_address_noforce
        (HImode,
	 gen_rtx (PLUS, Pmode,
		  gen_rtx (MULT, Pmode, index_diff,
			   gen_rtx (CONST_INT, VOIDmode, 2)),
		  gen_rtx (LABEL_REF, VOIDmode, operands[3])));
  /* Emit the last few insns.  */
  emit_insn (gen_casesi_2 (gen_reg_rtx (HImode), table_elt_addr, operands[3]));
  DONE;
}")

;; Recognize one of the insns resulting from casesi_2.
(define_insn ""
  [(set (pc)
	(plus:SI (pc) (match_operand:HI 0 "general_operand" "r")))
   (use (label_ref (match_operand 1 "" "")))]
  ""
  "*
  return \"jmp pc@(2:B)[%0:W:B]\";
")

;; Unconditional and other jump instructions
(define_insn "jump"
  [(set (pc)
	(label_ref (match_operand 0 "" "")))]
  ""
  "*
  return \"jra %l0\";
")

(define_insn ""
  [(set (pc)
	(if_then_else
	 (ne (compare (plus:HI (match_operand:HI 0 "general_operand" "g")
			       (const_int -1))
		      (const_int -1))
	     (const_int 0))
	 (label_ref (match_operand 1 "" ""))
	 (pc)))
   (set (match_dup 0)
	(plus:HI (match_dup 0)
		 (const_int -1)))]
  ""
  "*
{
  if (DATA_REG_P (operands[0]))
    return \"dbra %0,%l1\";
  if (GET_CODE (operands[0]) == MEM)
    {
      return \"subqw %#1,%0\;jcc %l1\";
    }
  return \"subqw %#1,%0\;cmpw %#-1,%0\;jne %l1\";
}")

(define_insn ""
  [(set (pc)
	(if_then_else
	 (ne (compare (plus:SI (match_operand:SI 0 "general_operand" "g")
			       (const_int -1))
		      (const_int -1))
	     (const_int 0))
	 (label_ref (match_operand 1 "" ""))
	 (pc)))
   (set (match_dup 0)
	(plus:SI (match_dup 0)
		 (const_int -1)))]
  ""
  "*
{
  if (DATA_REG_P (operands[0]))
    return \"dbra %0,%l1\;clrw %0\;subql %#1,%0\;jcc %l1\";
  if (GET_CODE (operands[0]) == MEM)
    return \"subql %#1,%0\;jcc %l1\";
  return \"subql %#1,%0\;cmpl %#-1,%0\;jne %l1\";
}")

;; dbra patterns that use REG_NOTES info generated by strength_reduce.

(define_insn ""
  [(set (pc)
	(if_then_else
	  (ge (plus:SI (match_operand:SI 0 "general_operand" "g")
			(const_int -1))
	      (const_int 0))
	  (label_ref (match_operand 1 "" ""))
	  (pc)))
   (set (match_dup 0)
	(plus:SI (match_dup 0)
		 (const_int -1)))]
  "find_reg_note (insn, REG_NONNEG, 0)"
  "*
{
  if (DATA_REG_P (operands[0]))
    return \"dbra %0,%l1\;clrw %0\;subql %#1,%0\;jcc %l1\";
  if (GET_CODE (operands[0]) == MEM)
    return \"subql %#1,%0\;jcc %l1\";
  return \"subql %#1,%0\;cmpl %#-1,%0\;jne %l1\";
}")

;; Call subroutine with no return value.
(define_insn "call"
  [(call (match_operand:QI 0 "general_operand" "o")
	 (match_operand:SI 1 "general_operand" "g"))]
  ;; Operand 1 not really used on the m68000.

  ""
  "*
{
  rtx xoperands[2];
  int size = XINT(operands[1],0);

  if (size == 0) 
    return \"subl a0,a0\;jsr %0\;movl a6@(-4),a0\";
  else
    {
      xoperands[1] = gen_rtx (CONST_INT, VOIDmode, size/4 );
      output_asm_insn (\"movl sp,a0\;movl %1,%-\", xoperands );
      return \"jsr %0\;movl a6@(-4),a0\;addqw #4,sp\";
    }
}")

;; Call subroutine, returning value in operand 0
;; (which must be a hard register).
(define_insn "call_value"
  [(set (match_operand 0 "" "rf")
	(call (match_operand:QI 1 "general_operand" "o")
	      (match_operand:SI 2 "general_operand" "g")))]
  ;; Operand 2 not really used on the m68000.
  ""
  "*
{
  rtx xoperands[3];
  int size = XINT(operands[2],0);

  if (size == 0)
    return \"subl a0,a0\;jsr %1\;movl a6@(-4),a0\";
  else
    {
      xoperands[2] = gen_rtx (CONST_INT, VOIDmode, size/4 );
      output_asm_insn (\"movl sp,a0\;movl %2,%-\", xoperands );
      return \"jsr %1\;movl a6@(-4),a0\;addqw #4,sp\";
    }
}")

(define_insn "return"
  [(return)]
  "0"
  "ret 0")

;; This is the first machine-dependent peephole optimization.
;; It is useful when a floating value is returned from a function call
;; and then is moved into an FP register.
;; But it is mainly intended to test the support for these optimizations.

(define_peephole
  [(set (reg:SI 15) (plus:SI (reg:SI 15) (const_int 4)))
   (set (match_operand:DF 0 "register_operand" "f")
	(match_operand:DF 1 "register_operand" "ad"))]
  "FP_REG_P (operands[0]) && ! FP_REG_P (operands[1])"
  "*
{
  rtx xoperands[2];
  xoperands[1] = gen_rtx (REG, SImode, REGNO (operands[1]) + 1);
  output_asm_insn (\"movl %1,%@\", xoperands);
  output_asm_insn (\"movl %1,%-\", operands);
  return \"fmoved %+,%0\";
}
")


;;- Local variables:
;;- mode:emacs-lisp
;;- comment-start: ";;- "
;;- comment-start-skip: ";+- *"
;;- eval: (set-syntax-table (copy-sequence (syntax-table)))
;;- eval: (modify-syntax-entry ?[ "(]")
;;- eval: (modify-syntax-entry ?] ")[")
;;- eval: (modify-syntax-entry ?{ "(}")
;;- eval: (modify-syntax-entry ?} "){")
;;- End:
