/* Declarations for insn-output.c.  These functions are defined in recog.c.
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

/* Operand-predicate functions.  */
int general_operand ();
int push_operand ();
int memory_operand ();
int indirect_operand ();
int immediate_operand ();
int register_operand ();
int address_operand ();
int nonmemory_operand ();
int nonimmediate_operand ();

int offsetable_address_p ();
rtx adj_offsetable_operand ();

/* Output a string of assembler code.
   Defined in final.c.  */
void output_asm_insn();

/* When outputting assembler code, indicates which alternative
   of the constraints was actually satisfied.  */
int which_alternative;
