/************************************************************************
 *  Source file:      c40load.c
 * 
 * These functions implement a generalized C40 COFF loader and symbol extraction
 * utility.
 * 
 * SYNOPSIS:
 * 
 * u_short c40_load(dsp_id, filename, entry_address, nsym, symnames, stab)
 * short dsp_id;	
 * 	DSP selector.  Under UNIX this is the file descriptor for the
 * 	desired DSP.  If this value is -1, then the program is not
 * 	loaded to the DSP, only the symbols are extracted from the COFF
 * 	file.
 * 
 * char *filename;	
 * 	Name of the COFF file.  This file may exist either in the current
 *	directory, or a directory specified by an environment variable
 *	(see the definition for C40VAR).
 * 
 * u_long *entry_address;	
 * 	Returns the program entry address as specified in the COFF
 * 	file's optional file header.
 * 
 * short nsym;
 * 	Number of symbols to extract from the COFF file.  If 0, no
 * 	symbols are extracted and the "symnames" and "stab" parameters
 * 	may be null.
 * 
 * char *symnames;
 * 	List of symbol names to extract from the COFF file.  The size
 * 	of this list is given by the "nsym" parameter above.
 * 
 * struct symtab *stab;
 * 	Array of symbol table structures that will be filled in.  Each
 * 	element of the array corresponds to an element of the symnames
 * 	array above.
 * 
 * These routines can be ported to other C40 hardware by defining the
 * macro DSP_WRITE32() to an external function that will write a block of
 * 32-bit words to a specified address on the specified DSP.  This
 * function is called as follows:
 * 
 * DSP_WRITE32(dsp, addr, buf, len)
 * 	short	dsp;	DSP selector
 * 	u_long	addr;	DSP address 
 * 	u_long	*dbuf;	buffer of words to write to DSP
 * 	u_long	csize;	size of buffer in words
 * 
 */
#include <sys/types.h>
#include <sys/file.h>
#include "coff.h"

/*
 * the following declarations are needed only within this module
 */
#include "c40load.h"

/*
 * external function prototypes
 */
char *getenv();

/*
 * internal function prototypes
 */
static int load_program();
static void extract_symbols();
static int read_coff_header();
static void swap_SYMTAB();
static void swap_long_buf();
static void swap_SECHDR();
static void swap_OFILHDR();
static void swap_FILHDR();
static void swap_short();
static void read_str();
static int write_block();
static void swap_long();

/*
 * globals
 */
char	cofferr[COFFERRLEN];	/* error message string */
static char	*fname;

/*************************************************************************
 * coffload() - function to download COFF file to DSP
 * Returns TRUE if successful, FALSE if failed.
 */
unsigned short c40_load(dsp_id, filename, entry_address, nsym, symnames, stab)
short	dsp_id;
unsigned long	*entry_address;
short nsym;
char *symnames[];
struct symtab stab[];
char	*filename;
{
	char	path[200];
	short	swapb = 0;		/* swap bytes flag */
	int 	rcode, mode;
	int 	coff_fd;
	FILHDR	fhdr;			/* file header */
	OFILHDR	ofhdr;			/* optional file header */

#ifdef DOS
	mode = O_RDONLY | O_BINARY;
#else
	mode = O_RDONLY;
#endif
	/*
	 * open file
	 */
	fname = filename;
	if (!findfile(filename, C40VAR, path)) {
		sprintf(cofferr, "Cannot find file `%s'", filename);
		return (FALSE);
	}
	if ((coff_fd = open(path, mode)) <= 0) {
		sprintf(cofferr,"Cannot open file '%s'", filename);
		return (FALSE);
	}
	/*
	 * read the header
	 */
	if (!read_coff_header(coff_fd, &fhdr, &swapb)) {
		return FALSE;
	}

	/*
	 * read optional file header if it exists
	 */
	if (fhdr.ohdrlen) {
		if (read(coff_fd, &ofhdr, OFILHSZ) != OFILHSZ) {
			sprintf(cofferr, 
"Cannot read optional file header in `%s'", filename);
			close(coff_fd);
			return FALSE;
		}
		if (swapb) {
			swap_OFILHDR(&ofhdr);
		}
		if (entry_address) {
			*entry_address = ofhdr.entryaddr;
		}
	}

	/*
	 * load program only if a valid dsp_id is provided
	 */
	if (dsp_id != -1) {
		rcode = load_program(dsp_id, coff_fd, fhdr.nsec, swapb);
		if (!rcode) {
			close(coff_fd);
			return (0);
		}
	}

	/*
	 * extract symbols only if nsym > 0
	 */
	if (nsym > 0) {
		extract_symbols(coff_fd, &fhdr, swapb, nsym, symnames, stab);
	}


	close(coff_fd);
	return TRUE;
}

/***************************************************************************
 * This function actually loads the program
 */
static int load_program(dsp_id, coff_fd, nsecs, swapb)
int dsp_id;
int coff_fd;
int nsecs;
int swapb;
{
	long	i;
	u_long	dsize, dspaddr;
	SECHDR	sechdr;			/* COFF section header */
#ifdef DOS
	long	floc;
#else
	off_t	floc;
#endif

	/*
	 * process each section header
	 */
	for (i=0; i<nsecs; i++) {
		if (read(coff_fd, (char *) &sechdr, SECHSZ) != SECHSZ) {
			sprintf(cofferr, 
"Failed to read section header %d from COFF file", i);
			return (FALSE);
		}

		/*
		 * swap bytes if necessary 
		 */
		if (swapb) {
			swap_SECHDR(&sechdr);
		}
		/*
		 * read and download section data
		 */
		if (sechdr.name[0] == 0) continue;
		if (sechdr.size == 0L) continue;
		if (sechdr.secptr == 0) continue;

		/*
		 * remember current file location and seek to section data
		 */
		floc = tell(coff_fd);
		if (lseek(coff_fd, sechdr.secptr, 0) == -1L) {
			sprintf(cofferr,
"failed to lseek() to section data for section %d in COFF file", i);
			return (FALSE);
		}

		/*
		 * check for an initialization table
		 */
		if (sechdr.flags & STYP_COPY) {	/* initialization table */
			read(coff_fd, &dsize, sizeof(long));
			if (swapb) {
				swap_long(&dsize);
			}
			while (dsize) {
				read(coff_fd, &dspaddr, sizeof(long));
				if (swapb) {
					swap_long(&dspaddr);
				}
				write_block(dsp_id, coff_fd, swapb, dspaddr,
					dsize);

				read(coff_fd, &dsize, sizeof(long));
				if (swapb) {
					swap_long(&dsize);
				}
			}
		}
		else {	/* normal section */
			if (!write_block(dsp_id, coff_fd, swapb, sechdr.paddr, 
		    		         sechdr.size)) {
				sprintf(cofferr,
"Failed to read section %d in COFF file", i);
				return (FALSE);
			}
		}

		/*
		 * reset file pointer to section data
		 */
		lseek(coff_fd, floc, SEEK_SET);
	}
	return (TRUE);
}


/***************************************************************************
 * this function extracts symbols from the COFF file
 */
static void extract_symbols(coff_fd, fhdrp, swapb, nsym, symnames, stab)
int coff_fd;
FILHDR *fhdrp;
int swapb;
int nsym;
char *symnames[];
struct symtab stab[];
{
	char	sname[MAX_SNAME];	/* long symbol names */
	long	i, j;
	long	symcnt, strtab;
	SYMTAB	syms;
#ifdef DOS
	long	floc;
#else
	off_t	floc;
#endif

	/*
	 * mark all requested symbols as "undefined" - they will be defined
	 * as they are found in the COFF file
	 */
	for (i=0; i<nsym; i++) {
		stab[i].type = T_UNDEF;
	}
	symcnt = nsym;	/* this many symbols to find */

	/* file ptr to string tab */
	strtab = fhdrp->symtabptr + fhdrp->nsym * SYMSIZE;

	/* seek to symbol table */
	lseek(coff_fd, fhdrp->symtabptr, SEEK_SET);
	for (i=0; i<fhdrp->nsym; i++) {
		read(coff_fd, &syms, SYMSIZE);
		if (swapb) {
			swap_SYMTAB(&syms);
		}
		if (syms.symnam.lname.zeros) {
			strncpy(sname, syms.symnam.symnam, 8);
			sname[8] = '\0';	/* insure null terminated */
		}
		else {
			floc = tell(coff_fd);
			lseek(coff_fd, strtab + syms.symnam.lname.symnamptr, 
				SEEK_SET);
			read_str(coff_fd, sname, MAX_SNAME);
			lseek(coff_fd, floc, SEEK_SET);
		}
		/*
		 * look for external symbols only
		 */
		if (syms.sclass != C_EXT) continue;
		/*
		 * check this symbol against the list of desired symbols
		 */
		for (j=0; j<nsym; j++) {
			if (stab[j].type != T_UNDEF) continue;
			if (strcmp(symnames[j], sname) != 0) continue;
			/*
			 * found a symbol, define it.
			 */
			stab[j].sname = symnames[j];
			stab[j].type = syms.type;
			stab[j].class = syms.sclass;
			stab[j].val.l = syms.value;
			symcnt--;
			break;	/* goto next symbol in COFF file */
		}
		if (symcnt == 0) return;
	}
}

/***************************************************************************
 * function to read a block of data from the COFF file and write it to the
 * DSP.
 */
static int write_block(dsp_id, coff_fd, swapb, dspaddr, size)
int dsp_id;
int coff_fd;
int swapb;
u_long dspaddr;
u_long size;
{
	int	csize;
	u_long	dbuf[BLOCKSIZE];

	while (size > 0) {
		csize = (size > BLOCKSIZE) ? BLOCKSIZE : size;
		if (read(coff_fd, dbuf, sizeof(long)*csize) !=
			sizeof(long)*csize) {
				return FALSE;
		}
		if (swapb) {
			swap_long_buf(csize, dbuf);
		}
		DSP_WRITE32(dsp_id, dspaddr, dbuf, csize);
		size -= csize;
		dspaddr += csize;
	}
	return TRUE;
}

/***************************************************************************
 * function to read a null-terminated string from a file
 */
static void read_str(fd, str, maxstr)
int fd;
char *str;
int maxstr;
{
	while (maxstr-- > 0) {
		if (read(fd, str, 1) != 1) break;
		if (*str == '\0') break;
		str++;
	}
}

/***************************************************************************
 * function to read the COFF header
 */
static int read_coff_header(coff_fd, fhdr, swapb)
int	coff_fd;
FILHDR	*fhdr;
short	*swapb;
{
	unsigned short	C40MAGIC_swp;

	/*
	 * read file header
	 */
	if (read(coff_fd, fhdr, FILHSZ) != FILHSZ) {
		sprintf(cofferr,"Cannot read file header in '%s'", fname);
		close(coff_fd);
		return FALSE;
	}

	/*
	 * check for byte swapped order to stay compatible with DOS
	 */
	C40MAGIC_swp = C40MAGIC;
	swap_short(&C40MAGIC_swp);

	if (fhdr->magic == C40MAGIC_swp) {
		*swapb = TRUE;
		swap_FILHDR(fhdr);
	}
	else if (fhdr->magic == C40MAGIC) {
		*swapb = FALSE;
	}
	else {
		sprintf(cofferr, 
"Unknown magic number 0x%x in file header in file %s", fhdr->magic, fname);
		close(coff_fd);
		return FALSE;
	}
	return TRUE;
}
	

/*
 * Functions to perform byte swapping in shorts and longs - to keep COFF
 * files compatible from DOS to Sparc
 */

static void swap_short(a)
unsigned short *a;
{
	union {
		unsigned short 	val;
		char	bytes[2];
	} swapper;
	char	tmp;

	swapper.val = *a;
	tmp = swapper.bytes[0];
	swapper.bytes[0] = swapper.bytes[1];
	swapper.bytes[1] = tmp;
	*a = swapper.val;
}


static void swap_long(a)
long *a;
{
	union {
		long	val;
		unsigned short	wrds[2];
	} swapper;
	unsigned short	tmp;

	/*
	swap 16-bit words first
	*/
	swapper.val = *a;
	tmp = swapper.wrds[0];
	swapper.wrds[0] = swapper.wrds[1];
	swapper.wrds[1] = tmp;

	/*
	swap bytes within 16-bit words
	*/
	swap_short(&swapper.wrds[0]);
	swap_short(&swapper.wrds[1]);

	*a = swapper.val;
}

/*
 * Functions to perform byte swapping on various structures involved
 */

static void swap_FILHDR(fhdr)
FILHDR *fhdr;
{
	swap_short(&(fhdr->magic));
	swap_short(&(fhdr->nsec));
	swap_long(&(fhdr->time));
	swap_long(&(fhdr->symtabptr));
	swap_long(&(fhdr->nsym));
	swap_short(&(fhdr->ohdrlen));
	swap_short(&(fhdr->flags));
}

static void swap_OFILHDR(ofhdr)
OFILHDR *ofhdr;
{
	swap_short(&(ofhdr->magic));
	swap_short(&(ofhdr->stamp));
	swap_long(&(ofhdr->textlen));
	swap_long(&(ofhdr->datalen));
	swap_long(&(ofhdr->bsslen));
	swap_long(&(ofhdr->textaddr));
	swap_long(&(ofhdr->entryaddr));
	swap_long(&(ofhdr->dataaddr));
}

static void swap_SECHDR(shdr)
SECHDR *shdr;
{
	swap_long(&(shdr->paddr));
	swap_long(&(shdr->vaddr));
	swap_long(&(shdr->size));
	swap_long(&(shdr->secptr));
	swap_long(&(shdr->relptr));
	swap_long(&(shdr->lnumptr));
	swap_short(&(shdr->nreloc));
	swap_short(&(shdr->nlnum));
	swap_short(&(shdr->flags));
}

static void swap_long_buf(N, buf)
int N;
long *buf;
{
	int	i;

	for (i=0; i<N; i++) {
		swap_long(buf++);
	}
}

static void swap_SYMTAB(tptr)
SYMTAB *tptr;
{
	swap_long(&(tptr->value));
	swap_short((unsigned short *) &(tptr->secnum));
	swap_short(&(tptr->type));
	if (tptr->symnam.lname.zeros == 0L) {
		swap_long(&(tptr->symnam.lname.symnamptr));
	}
}

/***************************************************************************
 * findfile - function to search a list of paths for a file.  
 */
int findfile(name, env_var, path)
char *name, *env_var, *path;
{
	int	lstr;
	char	*envstr;
	char	*sptr, *eptr;
	
	*path = '\0';	/* initialize path to null */
	
	/*
	 * try the file name as given first
	 */
	if (testpath(path, name)) return (1);	/* done if file found */
	
	/*
	 * try current directory first
	 */
	getcwd(path, 80);
	if (testpath(path, name)) return (1);	/* done if file found */
	 
	/*
	 * get environment string
	 */
	envstr = getenv(env_var);
	if (envstr == NULL || envstr[0] == '\0') return (0);
	/*
	 * parse the environment string at ';', concatenate parsed string
	 * with file name and see if the file exists.
	 */
	sptr = envstr;		/* initialize string begin pointer */

	while ((eptr = strchr(sptr, ':')) != NULL) {
		strncpy(path, sptr, eptr - sptr);
		path[eptr - sptr] = '\0';	/* strncpy() will not null-
						   terminate */
		if (testpath(path, name)) return (1);	/* done if file found */

		sptr = eptr + 1;	/* search rest of env. var. */
	}
	
	/*
	 * if environment variable does not end with a ';', make sure to
	 * test the final component
	 */
	if ( *sptr != '\0') {	/* env. var did not end with a ';' */
		strcpy(path, sptr);
		if (testpath(path, name)) return (1);	/* done if file found */
	}	

	/*
	 * file not found, give up!
	 */
	*path = '\0';
	return (0);
}


static int testpath(path, name)
char *path, *name;
{
	int	lstr;
	int	testit;
	
	/*
	 * if not null, make sure path component ends with a '\'
	 */
	lstr = strlen(path);
	if (lstr > 0 && path[lstr-1] != '/')
		strcat(path, "/");
	
	/*
	 * concatenate the file name and see if file exists
	 */	
	strcat(path, name);
	testit = access(path, F_OK);
	if (testit == 0) {		/* file found! */
		return(1);
	}
	return (0);
}

