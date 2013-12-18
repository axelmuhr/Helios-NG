/* c40load.h
 *
 * Definitions for the generalized C40 loader.  The only thing that may need
 * to be changed for porting to other hardware are the DSP_WRITE32,
 * BLOCKSIZE, and C40VAR macros.
 *
 * Structure declarations (with bugs fixed) taken from:
 * TMS320 Floating-Point DSP Assembly Language Tools manual, Appendix A.
 */

#include <stdio.h>
#include <string.h>

#ifdef DOS
#	include <io.h>
#else
#	include <unistd.h>
#	include	<fcntl.h>
#	include <sys/types.h>
#endif

/*
 * This macro defines a function that will write a block of data to a DSP
 */
#define	DSP_WRITE32(dsp, addr, buf, len)	\
	c40_write_long(dsp, addr, buf, len)

#define	BLOCKSIZE	0x800	/* block size for downloading data */

/*
 * C40VAR is the name of an environment variable set in the style of "path"
 * that lists directories for finding loadable programs and the like
 */
#define	C40VAR	"VC40PATH"

/*
 * COFF magic numbers
 */
#define C40MAGIC	0x93
#define	C30MAGIC	0x93
#define	C40OPTMAGIC	0x108

/*
 * string lengths
 */
#define SYMNAMLEN 8	/* symbol name length */
#define	MAX_SNAME 80	/* maximum symbol name length */
#define	COFFERRLEN	80	/* coff error message length */

/*
 * structure sizes
 */
#define	FILHSZ	sizeof(FILHDR)	/* size of file header structure */
#define OFILHSZ	sizeof(OFILHDR)	/* size of optional file header */
#define	SECHSZ	sizeof(SECHDR)	/* size of section header */
#define	SYMTABSZ	sizeof(SYMTAB)	/* size of symbol table entry */

/* WARNING: SPARC structures are always rounded up to a word boundary! */
#if defined(sun4) || defined(sun4c)
#	define	SYMSIZE	(sizeof(SYMTAB)-2)
#endif
#if defined(DOS) || defined(sun3) || defined(LYNXOS)
#	define	SYMSIZE	(sizeof(SYMTAB))
#endif


/*
 * file header flags
 */
#define	F_RELFLG 0x0001	/* relocation information stripped from file */
#define	F_EXEC	 0x0002	/* File is executable (no unresolved references) */
#define	F_LNNO	 0x0004	/* Line numbers stripped from file */
#define	F_LSYMS	 0x0008	/* Local symbols stripped from file */
#define	F_VERS	 0x0010	/* TMS32C40 object code */
#define	F_LITTLE 0x0100	/* Object data LSB first */

/*
 * section header flags
 * note: the term LOADED means that the raw data for this section appears
 * in the object file.
 */
#define	STYP_REG 0x0000	/* regular section (allocated, relocated, loaded) */
#define	STYP_DSECT	0x0001	/* dummy section (relocated, not allocated, not loaded) */
#define	STYP_NOLOAD	0x0002	/* noload section (allocated, relocated, not loaded) */
#define	STYP_COPY	0x0010	/* copy secton (not allocated, relocated, loaded) */
#define	STYP_TEXT	0x0020	/* section contains executable code */
#define	STYPE_DATA	0x0040	/* section contains initialized data */
#define	STYPE_BSS	0x0080	/* section contians unitialized data */
#define	STYP_ALIGN	0x0f00	/* align section by 2^n */
#define	STYP_BLOCK	0x1000	/* use alignment blocking factor */

/*
 * File header structure
 */
typedef	struct {
	unsigned short	magic;		/* magic number = 0x93 for C30/C40 */
	unsigned short	nsec;		/* number of sections */
	long		time;		/* time and date stamp */
	long		symtabptr;	/* ptr to symbol table in COFF file */
	long		nsym;		/* number of entries in symbol table */
	unsigned short	ohdrlen;	/* length of optional header */
	unsigned short	flags;		/* flags defined above */
} FILHDR;

/*
 * Optional file header structure - only used for relocation at download time
 */
typedef struct {
	unsigned short	magic;	/* magic number = 0x0108 */
	unsigned short	stamp;	/* version stamp - gives version of tools */
	long		textlen;	/* size (in words) of .text section */
	long		datalen;	/* size (in words) of .data section */
	long		bsslen;		/* size (in BITS) of .bss section */
	unsigned long	entryaddr;	/* entry point address */
	long		textaddr;	/* beginning address of .text */
	long		dataaddr;	/* beginning address of .data */
} OFILHDR;

/* 
 * section header structures
 */
typedef	struct {
	char		name[8];	/* section name */
	long		paddr;		/* section's physical (run) address */
	long		vaddr;		/* section's virtual (load) address */
	long		size;		/* section size (in words) */
	long		secptr;		/* ptr to raw data in COFF file */
	long		relptr;		/* ptr to relocation entries in COFF file */
	long		lnumptr;	/* ptr to line number entries in COFF file */
	unsigned short	nreloc;		/* number of relocation entries */
	unsigned short	nlnum;		/* number of line number entries */
	unsigned short	flags;		/* flags defined above */
	char		_reserved;	/* reserved */
	char		mempag;		/* memory page number */
} SECHDR;

/*
 * symbol table entry structure
 */
typedef struct
{
	union
	{
		char		symnam[SYMNAMLEN];	/* if name <= 8 chars */
		struct
		{
			long	zeros;		/* 0 if name > 8 chars */
			long	symnamptr;	/* ptr into string table */
		} lname;
	} symnam;
	long			value;		/* value of symbol */
	short			secnum;		/* section number */
	unsigned short		type;		/* type and derived type */
	char			sclass;		/* storage class */
	char			numaux;		/* number of aux. entries */
} SYMTAB;

