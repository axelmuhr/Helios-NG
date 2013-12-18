/* Communication between reload.c and reload1.c.
   Copyright (C) 1987 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */


/* See reload.c and reload1.c for comments on these variables.  */

extern rtx reload_in[FIRST_PSEUDO_REGISTER];
extern rtx reload_out[FIRST_PSEUDO_REGISTER];
extern rtx reload_in_reg[FIRST_PSEUDO_REGISTER];
extern enum reg_class reload_reg_class[FIRST_PSEUDO_REGISTER];
extern enum machine_mode reload_inmode[FIRST_PSEUDO_REGISTER];
extern enum machine_mode reload_outmode[FIRST_PSEUDO_REGISTER];
extern char reload_strict_low[FIRST_PSEUDO_REGISTER];
extern char reload_optional[FIRST_PSEUDO_REGISTER];
extern int reload_inc[FIRST_PSEUDO_REGISTER];
extern int n_reloads;

extern rtx reload_reg_rtx[FIRST_PSEUDO_REGISTER];

extern rtx *reg_equiv_constant;
extern rtx *reg_equiv_address;
extern rtx *reg_equiv_mem;

/* All the "earlyclobber" operands of the current insn
   are recorded here.  */
extern int n_earlyclobbers;
extern rtx reload_earlyclobbers[MAX_RECOG_OPERANDS];

/* First uid used by insns created by reload in this function.
   Used in find_equiv_reg.  */
extern int reload_first_uid;

void init_reload ();
void find_reloads ();
void subst_reloads ();
