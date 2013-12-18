/* Save and restore call-clobbered registers which are live across a call.
   Copyright (C) 1989 Free Software Foundation, Inc.

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

#include "config.h"
#include "rtl.h"
#include "insn-config.h"
#include "flags.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "reload.h"
#include "recog.h"
#include "basic-block.h"

/* Set of hard regs currently live (during scan of all insns).  */

static HARD_REG_SET hard_regs_live;

/* The block of storage on the stack where regs are saved */

static rtx save_block_addr;
static int save_block_size;

/* A REG rtx for each hard register that has been saved.  */

static rtx save_reg_rtx[FIRST_PSEUDO_REGISTER];

static void set_reg_live ();
static void clear_reg_live ();
static void insert_call_saves ();
static void emit_mult_save ();
static void emit_mult_restore ();
static rtx grow_save_block ();
static enum machine_mode choose_hard_reg_mode ();

/* Find the places where hard regs are live across calls and save them.  */

save_call_clobbered_regs ()
{
  rtx insn;
  int b;

  if (obey_regdecls)
    return;
  
  save_block_size = 0;
  save_block_addr = 0;
  bzero (save_reg_rtx, sizeof save_reg_rtx);

  for (b = 0; b < n_basic_blocks; b++)
    {
      regset regs_live = basic_block_live_at_start[b];
      int offset, bit, i;

      /* Compute hard regs live at start of block -- this is the
	 real hard regs marked live, plus live pseudo regs that
	 have been renumbered to hard regs.  */

#ifdef HARD_REG_SET
      hard_regs_live = *regs_live;
#else
      COPY_HARD_REG_SET (hard_regs_live, regs_live);
#endif

      for (offset = 0, i = 0; offset < regset_size; offset++)
	{
	  if (regs_live[offset] == 0)
	    i += HOST_BITS_PER_INT;
	  else
	    for (bit = 1; bit && i < max_regno; bit <<= 1, i++)
	      if ((regs_live[offset] & bit) && reg_renumber[i] >= 0)
		SET_HARD_REG_BIT (hard_regs_live, reg_renumber[i]);
	}

      /* Now scan the insns in the block, keeping track of what hard
	 regs are live as we go.  When we see a call, save the live
	 call-clobbered hard regs.  */

      for (insn = basic_block_head[b]; TRUE; insn = NEXT_INSN (insn))
	{
	  RTX_CODE code = GET_CODE (insn);

	  if (code == CALL_INSN)
	    insert_call_saves (insn);

	  if (code == INSN || code == CALL_INSN || code == JUMP_INSN)
	    {
	      rtx link;

	      /* NB: the normal procedure is to first enliven any
		 registers set by insn, then deaden any registers that
		 had their last use at insn.  This is incorrect now,
		 since multiple pseudos may have been mapped to the
		 same hard reg, and the death notes are ambiguous.  So
		 it must be done in the other, safe, order.  */

	      for (link = REG_NOTES (insn); link; link = XEXP (link, 1))
		if (REG_NOTE_KIND (link) == REG_DEAD)
		  clear_reg_live (XEXP (link, 0));

	      note_stores (PATTERN (insn), set_reg_live);
	    }

	  if (insn == basic_block_end[b])
	    break;
	}
    }
}

/* Here from note_stores when an insn stores a value in a register.
   Set the proper bit or bits in hard_regs_live.  */

static void
set_reg_live (reg, setter)
     rtx reg, setter;
{
  register int regno;

  /* WORD is which word of a multi-register group is being stored.
     For the case where the store is actually into a SUBREG of REG.
     Except we don't use it; I believe the entire REG needs to be
     live.  */
  int word = 0;

  if (GET_CODE (reg) == SUBREG)
    {
      word = SUBREG_WORD (reg);
      reg = SUBREG_REG (reg);
    }

  if (GET_CODE (reg) != REG)
    return;

  regno = REGNO (reg);

  /* For pseudo reg, see if it has been assigned a hardware reg.  */
  if (reg_renumber[regno] >= 0)
    regno = reg_renumber[regno] /* + word */;

  /* Handle hardware regs (and pseudos allocated to hard regs).  */
  if (regno < FIRST_PSEUDO_REGISTER && ! call_fixed_regs[regno])
    {
      register int last = regno + HARD_REGNO_NREGS (regno, GET_MODE (reg));
      while (regno < last)
	{
	  SET_HARD_REG_BIT (hard_regs_live, regno);
	  regno++;
	}
    }
}

/* Here when a REG_DEAD note records the last use of a reg.  Clear
   the appropriate bit or bits in hard_regs_live.  */

static void
clear_reg_live (reg)
     rtx reg;
{
  register int regno = REGNO (reg);

  /* For pseudo reg, see if it has been assigned a hardware reg.  */
  if (reg_renumber[regno] >= 0)
    regno = reg_renumber[regno];

  /* Handle hardware regs (and pseudos allocated to hard regs).  */
  if (regno < FIRST_PSEUDO_REGISTER && ! call_fixed_regs[regno])
    {
      /* Pseudo regs already assigned hardware regs are treated
	 almost the same as explicit hardware regs.  */
      register int last = regno + HARD_REGNO_NREGS (regno, GET_MODE (reg));
      while (regno < last)
	{
	  CLEAR_HARD_REG_BIT (hard_regs_live, regno);
	  regno++;
	}
    }
}      

/* Insert insns to save and restore live call-clobbered regs around
   call insn INSN.  */

static void
insert_call_saves (insn)
     rtx insn;
{
  int regno;
  int save_block_size_needed;
  int save_block_offset[FIRST_PSEUDO_REGISTER];

  save_block_size_needed = 0;
  
  for (regno = 0; regno < FIRST_PSEUDO_REGISTER; ++regno)
    {
      save_block_offset[regno] = -1;
      if (call_used_regs[regno] && ! call_fixed_regs[regno]
	  && TEST_HARD_REG_BIT (hard_regs_live, regno))
	{
	  enum machine_mode mode = choose_hard_reg_mode (regno);
	  int align = GET_MODE_UNIT_SIZE (mode);
	  if (align > BIGGEST_ALIGNMENT / BITS_PER_UNIT)
	    align = BIGGEST_ALIGNMENT / BITS_PER_UNIT;
	  save_block_size_needed =
	    ((save_block_size_needed + align - 1) / align) * align;
	  save_block_offset[regno] = save_block_size_needed;
	  save_block_size_needed += GET_MODE_SIZE (mode);
	  if (! save_reg_rtx[regno])
	    save_reg_rtx[regno] = gen_rtx (REG, mode, regno);
	}
    }

  if (save_block_size < save_block_size_needed)
    save_block_addr = grow_save_block (save_block_addr,
				       save_block_size_needed);
  emit_mult_save (insn, save_block_addr, save_block_offset);
  emit_mult_restore (insn, save_block_addr, save_block_offset);
}

/* Emit a string of stores to save the hard regs listed in
   OFFSET[] at address ADDR; insert the loads after INSN.
   OFFSET[reg] is -1 if reg should not be saved, or a
   suitably-aligned offset from ADDR.  
   The offsets actually used do not have to be those listed
   in OFFSET, but should fit in a block of the same size.  */

static void
emit_mult_save (insn, addr, offset)
     rtx insn, addr;
     int offset[];
{
  int regno;

  for (regno = 0; regno < FIRST_PSEUDO_REGISTER; ++regno)
    if (offset[regno] >= 0)
      {
	rtx reg = save_reg_rtx[regno];
	rtx temp =
	  gen_rtx (MEM, GET_MODE (reg), plus_constant (addr, offset[regno]));
	emit_insn_before (gen_move_insn (temp, reg), insn);
      }
}

/* Emit a string of loads to restore the hard regs listed in
   OFFSET[] from address ADDR; insert the loads before INSN.
   OFFSET[reg] is -1 if reg should not be loaded, or a
   suitably-aligned offset from ADDR.  
   The offsets actually used do not need to be those provided in
   OFFSET, but should agree with whatever emit_mult_save does.  */

static void
emit_mult_restore (insn, addr, offset)
     rtx insn, addr;
     int offset[];     
{
  int regno;

  for (regno = FIRST_PSEUDO_REGISTER; --regno >= 0; )
    if (offset[regno] >= 0)
      {
	rtx reg = save_reg_rtx[regno];
	rtx temp =
	  gen_rtx (MEM, GET_MODE (reg), plus_constant (addr, offset[regno]));
	emit_insn_after (gen_move_insn (reg, temp), insn);
      }
}

/* Return the address of a new block of size SIZE on the stack.
   The old save block is at ADDR; ADDR is 0 if no block exists yet.  */

static rtx
grow_save_block (addr, size)
     rtx addr;
     int size;
{
  rtx newaddr;

  /* Keep the size a multiple of the main allocation unit.  */
  size = (((size + (BIGGEST_ALIGNMENT / BITS_PER_UNIT) - 1)
	   / (BIGGEST_ALIGNMENT / BITS_PER_UNIT))
	  * (BIGGEST_ALIGNMENT / BITS_PER_UNIT));

  /* If no save block exists yet, create one and return it.  */
  if (! addr)
    {
      save_block_size = size;
      return XEXP (assign_stack_local (BLKmode, size), 0);
    }

  /* Get a new block and coalesce it with the old one.  */
  newaddr = XEXP (assign_stack_local (BLKmode, size - save_block_size), 0);
  if (GET_CODE (newaddr) == PLUS
      && XEXP (newaddr, 0) == frame_pointer_rtx
      && GET_CODE (XEXP (newaddr, 1)) == CONST_INT
      && GET_CODE (addr) == PLUS
      && XEXP (addr, 0) == frame_pointer_rtx
      && GET_CODE (XEXP (addr, 1)) == CONST_INT
      && ((INTVAL (XEXP (newaddr, 1)) - INTVAL (XEXP (addr, 1))
	   == size - save_block_size)
	  || (INTVAL (XEXP (addr, 1)) - INTVAL (XEXP (newaddr, 1))
	      == size - save_block_size)))
    {
      save_block_size = size;
      if (INTVAL (XEXP (newaddr, 1)) < INTVAL (XEXP (addr, 1)))
	return newaddr;
      else
	return addr;
    }

  /* They didn't coalesce, find out why */
  abort ();			

  save_block_size = size;
  return XEXP (assign_stack_local (BLKmode, size), 0);
}

/* Return a machine mode that is legitimate for hard reg REGNO
   and large enough to save the whole register.  */

static enum machine_mode
choose_hard_reg_mode (regno)
     int regno;
{
  enum reg_class class = REGNO_REG_CLASS (regno);

  if (CLASS_MAX_NREGS (class, DImode) == 1
      && HARD_REGNO_MODE_OK (regno, DImode))
    return DImode;
  else if (CLASS_MAX_NREGS (class, DFmode) == 1
	   && HARD_REGNO_MODE_OK (regno, DFmode))
    return DFmode;
  else if (CLASS_MAX_NREGS (class, SImode) == 1
	   && HARD_REGNO_MODE_OK (regno, SImode))
    return SImode;
  else if (CLASS_MAX_NREGS (class, SFmode) == 1
	   && HARD_REGNO_MODE_OK (regno, SFmode))
    return SFmode;
  else if (CLASS_MAX_NREGS (class, HImode) == 1
	   && HARD_REGNO_MODE_OK (regno, HImode))
    return HImode;
  else
    abort ();
}
