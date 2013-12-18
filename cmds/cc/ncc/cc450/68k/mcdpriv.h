/*
 * C compiler file mcdpriv.h,  version 1
 * (Private interfaces within machine-dependent back end).
 * Copyright (C) Codemist Ltd., 1989
 */

#ifndef __mcdpriv_h
#define __mcdpriv_h 1

extern int32 	asm_padcol8(int32 n);
extern int32 	decode_instruction(int32 off, char *s);
extern Symstr *	decode_external(int32 p);
extern int	split_module_table;


#define annotations (feature & FEATURE_ANNOTATE)
/* #define tom68sex(w,type) (w) */

#ifdef TARGET_HAS_DEBUGGER

  extern int32 local_fpaddress(int32 p);
  /* p is the bindaddr field from a binder for a local object.  Returns the
   * offset of the object from the fp (assuming that fp and sp have not been
   * split in the frame containing the object ...).
   */

  extern void dbg_writedebug(void);
  /* Call from the object formatter to the debug table generator to
   * cause tables to be output
   */

  extern void obj_writedebug(void *, size_t);

#endif

#endif

