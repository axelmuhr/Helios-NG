/* 
 * C++ interface to <a.out.h>
 */

#ifndef aout_h
#define aout_h

#include "defs.h"
#include <a.out.h>
#include <bstring.h>

extern char zero[];		/* for zero-filled page aligning */

/* struct exec is defined <sys/exec.h> */
struct Exec : public exec {
    boolean BadMagic () {
	return a_magic != OMAGIC && a_magic != NMAGIC && a_magic != ZMAGIC;
    }
    int TextOffset () { return a_magic == ZMAGIC ? 1024 : sizeof(Exec); }
    int DataOffset () { return TextOffset() + (int)a_text; }
    int RelocOffset () { return TextOffset() + (int)a_text + (int)a_data; }
    int SymOffset () { return RelocOffset() + (int)a_trsize + (int)a_drsize; }
    int StrOffset () { return SymOffset() + (int)a_syms; }
    void ZeroMe () { bzero(this, sizeof(Exec)); }
    void Dump();
};

/*
 * Format of a symbol table entry; this file is included by <a.out.h>
 * and should be used if you aren't interested the a.out header
 * or relocation information.
 */
struct NList : public nlist {
    int Index () { return n_un.n_strx; }
    int Type () { return n_type & N_TYPE & ~N_EXT; }
    boolean Global () { return (n_type & N_EXT) != 0; }
    int Stab () { return n_type & N_STAB; }
    boolean Undefined () { return Type() == N_UNDF && n_value == 0; }
    boolean Defined () { return Type() != N_UNDF; }
    boolean MaybeBss () {
	return (Type() == N_BSS || (Type() == N_UNDF && n_value != 0));
    }
    boolean IsVar();
    void Dump ();
};

static const int N_ANON = 0x10;		/* a variable of some kind */

/*
 * Format of a relocation datum.
 */
struct RelocInfo : public relocation_info {
    void Dump();		
};

extern boolean IsObjectFile(void*);

#endif
