/*
 * m68k/mcerrs.h - prototype for machine-specific error messages file
 */

%O  /* Ordinary error messages - mapped onto numeric codes */

/*
 * The following message is always machine specific since it may include
 * information about the source of support for the product.
 */
#define misc_disaster_banner   "\n\
*************************************************************************\n\
* The compiler has detected an internal inconsistency.  This can occur  *\n\
* because it has run out of a vital resource such as memory or disk     *\n\
* space or because there is a fault in it.  If you cannot easily alter  *\n\
* your program to avoid causing this rare failure, please contact your  *\n\
* dealer.  The dealer may be able to help you immediately and will be   *\n\
* able to report a suspected compiler fault to the support centre.      *\n\
*************************************************************************\n\
\n"

%S  /* System failure messages - text not preserved */

#define syserr_branch_round_literals "branch round literals"
#define syserr_big_branch "branch offset too large %lx"
#define syserr_big_displacement "displacement out of range %ld"
#define syserr_labref_type "Unknown label reference type %.8lx"
#define syserr_local_base "local_base %lx"
#define syserr_local_address "local_address %lx"
#define syserr_local_addr "local_addr"
#define syserr_regnum "Invalid register number %ld"
#define syserr_outHW "outHW(%lx)"
#define syserr_ill_instr "Illegal bits set in instruction field (%lx,%lx)"
#define syserr_addr_disp "Address register displacement out of range"
#define syserr_invalid_index_mode "Invalid index mode extension field"
#define syserr_pc_disp "PC displacement out of range"
#define syserr_eff_addr "Bad effective address mode for extension field"
#define syserr_data_sym "Unable to find another data symbol at %ld"
#define syserr_enter "emit(J_ENTER %ld)"
#define syserr_ldrrk "m68kgen(unsigned LDRR/K"
#define syserr_remove_noops "remove_noops(MOVR r,r) failed"
#define syserr_silly_shift "Silly shift value %ld"
#define syserr_setsp_confused "SETSP confused %ld!=%ld %ld"
#define syserr_illegal_jopmode "Illegal JOP mode(%lx)"
#define syserr_remove_fpnoops "remove_noops(MOVF/DR r,r) failed"
#define syserr_unimp_jopmode "Non-implemented JOP mode(%lx)"
#define syserr_fp_reg "Attempt to use non fp reg for fp value"
#define syserr_pr_asmname "pr_asmname"
#define syserr_asmlab "odd asmlab(%lx)"
#define syserr_display_asm "display_asm(%lx)"
#define syserr_asm_trailer "asm_trailer(%ld)"
#define syserr_asm_datalen "asm_data len=%ld"
#define syserr_asm_trailer1 "asm_trailer(%ldF%ld)"

/* end of m68k/mcerrs.h */
