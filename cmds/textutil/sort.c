/* sort - sort a file of lines		Author: Michiel Huisjes */

/* SYNOPSIS:
 * 	sort [-funbirdcmt'x'] [+beg_pos[opts] [-end_pos]] [-o outfile] [file] ..
 * 
 * 	[opts] can be any of
 * 	-f : Fold upper case to lower.
 * 	-n : Sort to numeric value (optional decimal point) implies -b
 * 	-b : Skip leading blanks
 * 	-i : Ignore chars outside ASCII range (040 - 0176)
 * 	-r : Reverse the sense of comparisons.
 * 	-d : Sort to dictionary order. Only letters, digits, comma's and points
 * 	     are compared.
 * 	If any of these flags are used in [opts], then they override all global
 * 	ordering for this field.
 * 
 * 	I/O control flags are:
 * 	-u : Print uniq lines only once.
 * 	-c : Check if files are sorted in order.
 * 	-m : Merge already sorted files.
 * 	-o outfile : Name of output file. (Can be one of the input files).
 * 		     Default is stdout.
 * 	- : Take stdin as input.
 * 
 * 	Fields:
 * 	-t'x' : Field separating character is 'x'
 * 	+a.b : Start comparing at field 'a' with offset 'b'. A missing 'b' is
 * 	       taken to be 0.
 * 	-a.b : Stop comparing at field 'a' with offset 'b'. A missing 'b' is
 * 	       taken to be 0.
 * 	A missing -a.b means the rest of the line.
 */

#ifdef __HELIOS
#include <helios.h>
#include <stdlib.h>
#include <string.h>
#include <posix.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define atoi localatoi
#else
#include "stat.h"
#endif
#include <signal.h>

#define OPEN_FILES	16		/* Nr of open files per process */

#define MEMORY_SIZE	(20 * 1024)	/* Total mem_size */
#define LINE_SIZE	(1024 >> 1)	/* Max length of a line */
#define IO_SIZE		(2 * 1024)	/* Size of buffered output */
#define STD_OUT		 1		/* Fd of terminal */

/* Return status of functions */
#define OK		 0
#define ERROR		-1
#define NIL_PTR		((char *) 0)

/* Compare return values */
#define LOWER		-1
#define SAME		 0
#define HIGHER		 1

/*
 * Table definitions.
 */
#define DICT		0x001		/* Alpha, numeric, letters and . */
#define ASCII		0x002		/* All between ' ' and '~' */
#define BLANK		0x004		/* ' ' and '\t' */
#define DIGIT		0x008		/* 0-9 */
#define UPPER		0x010		/* A-Z */

typedef enum {				/* Boolean types */
  LFALSE = 0,
  LTRUE
} BOOL;

typedef struct {
  int fd;				/* Fd of file */
  char *buffer;				/* Buffer for reads */
  int read_chars;			/* Nr of chars actually read in buffer*/
  int cnt;				/* Nr of chars taken out of buffer */
  char *line;				/* Contains line currently used */
} MERGE	;

#define NIL_MERGE	((MERGE *) 0)
MERGE merge_f[OPEN_FILES];		/* Merge structs */
int buf_size;			/* Size of core available for each struct */

#define FIELDS_LIMIT	10		/* 1 global + 9 user */
#define GLOBAL		 0

typedef struct {
  int beg_field, beg_pos;		/* Begin field + offset */
  int end_field, end_pos;		/* End field + offset. ERROR == EOLN */
  BOOL reverse;				/* LTRUE if rev. flag set on this field*/
  BOOL blanks;
  BOOL dictionary;
  BOOL fold_case;
  BOOL ascii;
  BOOL numeric;
} FIELD;

/* Field declarations. A total of FILEDS_LIMIT is allowed */
FIELD fields[FIELDS_LIMIT];
int field_cnt;			/* Nr of field actually assigned */

/* Various output control flags */
BOOL check = LFALSE;
BOOL only_merge = LFALSE;
BOOL uniq = LFALSE;

char *mem_top;			/* Mem_top points to lowest pos of memory. */
char *cur_pos;			/* First free position in mem */
char **line_table;		/* Pointer to the internal line table */
BOOL in_core = LTRUE;		/* Set if input cannot all be sorted in core */

			/* Place where temp_files should be made */
#ifdef __HELIOS
char temp_files[] = "/helios/tmp/sortXXXXX.XX";
#else
char temp_files[] = "/tmp/sort.XXXXX.XX";
#endif
char *output_file;		/* Name of output file */
int out_fd;			/* Fd to output file (could be STD_OUT) */
char out_buffer[IO_SIZE];	/* For buffered output */

char **argptr;			/* Pointer to argv structure */
int args_offset;		/* Nr of args spilled on options */
int args_limit;			/* Nr of args given */

char separator;			/* Char that separates fields */
int nr_of_files = 0;		/* Nr_of_files to be merged */
int disabled;			/* Nr of files done */

char USAGE[] = "Usage: sort [-funbirdcmt'x'] [+beg_pos [-end_pos]] [-o outfile] [file] ..";

/*
 * Table of all chars. 0 means no special meaning.
 */
char table[256] = {
/* '^@' to space */
  0,	0,		0,	0,	0,	0,	0,	0,
  0,	BLANK | DICT,	0,	0,	0,	0,	0,	0,
  0,	0,		0,	0,	0,	0,	0,	0,
  0,	0,		0,	0,	0,	0,	0,	0,

/* space to '0' */
  BLANK | DICT | ASCII,	ASCII,	ASCII,	ASCII,	ASCII,	ASCII,	ASCII,
  ASCII,			ASCII,	ASCII,	ASCII,	ASCII,	ASCII,	ASCII,
  ASCII,			ASCII,

/* '0' until '9' */
  DIGIT | DICT | ASCII,	DIGIT | DICT | ASCII,	DIGIT | DICT | ASCII,
  DIGIT | DICT | ASCII,	DIGIT | DICT | ASCII,	DIGIT | DICT | ASCII,
  DIGIT | DICT | ASCII,	DIGIT | DICT | ASCII,	DIGIT | DICT | ASCII,
  DIGIT | DICT | ASCII,

/* ASCII from ':' to '@' */
  ASCII,	ASCII,	ASCII,	ASCII,	ASCII,	ASCII,	ASCII,

/* Upper case letters 'A' to 'Z' */
  UPPER | DICT | ASCII,	UPPER | DICT | ASCII,	UPPER | DICT | ASCII,
  UPPER | DICT | ASCII,	UPPER | DICT | ASCII,	UPPER | DICT | ASCII,
  UPPER | DICT | ASCII,	UPPER | DICT | ASCII,	UPPER | DICT | ASCII,
  UPPER | DICT | ASCII,	UPPER | DICT | ASCII,	UPPER | DICT | ASCII,
  UPPER | DICT | ASCII,	UPPER | DICT | ASCII,	UPPER | DICT | ASCII,
  UPPER | DICT | ASCII,	UPPER | DICT | ASCII,	UPPER | DICT | ASCII,
  UPPER | DICT | ASCII,	UPPER | DICT | ASCII,	UPPER | DICT | ASCII,
  UPPER | DICT | ASCII,	UPPER | DICT | ASCII,	UPPER | DICT | ASCII,
  UPPER | DICT | ASCII,	UPPER | DICT | ASCII,

/* ASCII from '[' to '`' */
  ASCII,	ASCII,	ASCII,	ASCII,	ASCII,	ASCII,

/* Lower case letters from 'a' to 'z' */
  DICT | ASCII,	DICT | ASCII,	DICT | ASCII,	DICT | ASCII,
  DICT | ASCII,	DICT | ASCII,	DICT | ASCII,	DICT | ASCII,
  DICT | ASCII,	DICT | ASCII,	DICT | ASCII,	DICT | ASCII,
  DICT | ASCII,	DICT | ASCII,	DICT | ASCII,	DICT | ASCII,
  DICT | ASCII,	DICT | ASCII,	DICT | ASCII,	DICT | ASCII,
  DICT | ASCII,	DICT | ASCII,	DICT | ASCII,	DICT | ASCII,
  DICT | ASCII,	DICT | ASCII,

/* ASCII from '{' to '~' */
  ASCII,	ASCII,	ASCII,	ASCII,

/* Stuff from -1 to -177 */
  0,	0,	0,	0,	0,	0,	0,	0,	0,
  0,	0,	0,	0,	0,	0,	0,	0,	0,
  0,	0,	0,	0,	0,	0,	0,	0,	0,
  0,	0,	0,	0,	0,	0,	0,	0,	0,
  0,	0,	0,	0,	0,	0,	0,	0,	0,
  0,	0,	0,	0,	0,	0,	0,	0,	0,
  0,	0,	0,	0,	0,	0,	0,	0,	0,
  0,	0,	0,	0,	0,	0,	0,	0,	0,
  0,	0,	0,	0,	0,	0,	0
};

/*
 * Error () prints the error message on stderr and exits if quit == LTRUE.
 */
void
error(
      register BOOL quit,
      register char *message,
      register char *arg )
{
  write(2, message, strlen(message));
  if (arg != NIL_PTR)
	write(2, arg, strlen(arg));
  write(2, ".\n", 2);
  if (quit)
	exit(1);
}

/*
 * Get_opts () assigns the options into the field structure as described in ptr.
 * This field structure could be the GLOBAL one.
 */
void
get_opts(
	 register char *ptr,
	 register FIELD *field )
{
  switch (*ptr) {
	case 'b' :		/* Skip leading blanks */
		field->blanks = LTRUE;
		break;
	case 'd' :		/* Dictionary order */
		field->dictionary = LTRUE;
		break;
	case 'f' :		/* Fold upper case to lower */
		field->fold_case = LTRUE;
		break;
	case 'i' :		/* Skip chars outside ' ' '~' */
		field->ascii = LTRUE;
		break;
	case 'n' :		/* Sort on numeric */
		field->numeric = LTRUE;
		field->blanks = LTRUE;
		break;
	case 'r' :		/* Reverse comparisons */
		field->reverse = LTRUE;
		break;
	default :		/* Illegal options */
		error(LTRUE, USAGE, NIL_PTR);
  }
}

/*
 * Atoi() converts a string to an int.
 */
int
atoi( register char *ptr )
{
  register int num = 0;			/* Accumulator */

  while (table[*ptr] & DIGIT)
	num = num * 10 + *ptr++ - '0';

  return num;
}

/*
 * New_field () assigns a new field as described by the arguments. 
 * A field description is of the form: +a.b[opts] -c.d, where b and d, as well
 * as -c.d and [opts] are optional. Nr before digit is field nr. Nr after digit 
 * is offset from field.
 */
void
new_field(
	  register FIELD *field,	/* Field to assign */
	  int *offset,			/* Offset in argv structure */
	  BOOL beg_fl )			/* Assign beg or end of field */
{
  register char *ptr;

  ptr = argptr[*offset];
  *offset += 1;				/* Incr offset to next arg */
  ptr++;

  if (beg_fl)
	field->beg_field = atoi(ptr);	/* Assign int of first field */
  else 
	field->end_field = atoi(ptr);

  while (table[*ptr] & DIGIT)		/* Skip all digits */
	ptr++;

  if (*ptr == '.') {			/* Check for offset */
	ptr++;
	if (beg_fl)
		field->beg_pos = atoi(ptr);
	else
		field->end_pos = atoi(ptr);
	while (table[*ptr] & DIGIT)	/* Skip digits */
		ptr++;
  }

  if (beg_fl) {
	while (*ptr != '\0')		/* Check options after field */
		get_opts(ptr++, field);
  }

  if (beg_fl) {			/* Check for end pos */
	ptr = argptr[*offset];
	if (*ptr == '-' && table[*(ptr + 1)] & DIGIT) {
		new_field(field, offset, LFALSE);
		if (field->beg_field > field->end_field)
			error(LTRUE, "End field is before start field!",NIL_PTR);
	}
	else				/* No end pos. */
		field->end_field = ERROR;
  }
}

/*
 * File_name () returns the nr argument from the argument list, or a uniq 
 * filename if the nr is too high, or the arguments were not merge files.
 */

char *
file_name(register int nr )
{
  if (only_merge) {
	if (args_offset + nr < args_limit)
		return argptr[args_offset + nr];
  }

#ifdef __HELIOS
  temp_files[22] = nr / 26 + 'a';
  temp_files[23] = nr % 26 + 'a';
#else
  temp_files[16] = nr / 26 + 'a';
  temp_files[17] = nr % 26 + 'a';
#endif

  return temp_files;
}

#ifdef __STDC__
void Catch(int dummy)
#else
Catch()
#endif
{
	register short i;

	signal (SIGINT, SIG_IGN);
	only_merge = LFALSE;
	for (i = 0; i < 26; i++)
		(void) unlink (file_name (i));
	exit (2);
}

/*
 * Adjust_options() assigns all global variables set also in the fields
 * assigned.
 */
void
adjust_options(register FIELD *field )
{
  register FIELD *gfield = &fields[GLOBAL];

  if (gfield->reverse)
	field->reverse = LTRUE;
  if (gfield->blanks)
	field->blanks = LTRUE;
  if (gfield->dictionary)
	field->dictionary = LTRUE;
  if (gfield->fold_case)
	field->fold_case = LTRUE;
  if (gfield->ascii)
	field->ascii = LTRUE;
  if (gfield->numeric)
	field->numeric = LTRUE;
}


/*
 * Open_outfile () assigns to out_fd the fd where the output must go when all
 * the sorting is done.
 */
void
open_outfile( void )
{
  if (output_file == NIL_PTR)
	out_fd = STD_OUT;
  else if ((out_fd = creat(output_file, 0644)) < 0)
	error(LTRUE, "Cannot creat ", output_file);
}

/*
 * Last_line () find the last line in core and retuns the offset from the top
 * of the memory.
 */
int
last_line( void )
{
  register int i;

  for (i = MEMORY_SIZE - 1; i > 0; i--)
	if (mem_top[i] == '\n')
		break;
  return i + 1;
}

/*
 * Mwrite () performs a normal write (), but checks the return value.
 */
void
mwrite(
       int fd,
       char *address,
       register int bytes )
{
  if (write(fd, address, bytes) != bytes && bytes != 0)
	error(LTRUE, "Write error", NIL_PTR);
}



/*
 * Skip_fields () skips nf fields of the line pointed to by str.
 */
char *
skip_fields(
	    register char *str,
	    int nf )
{
  while (nf-- > 0) {
	if (separator == '\0') {	/* Means ' ' or '\t' */
		while (*str != ' '  && *str != '\t' && *str != '\n')
			str++;
		while (table[*str] & BLANK)
			str++;
	}
	else {
		while (*str != separator && *str != '\n')
			str++;
		str++;
	}
  }
  return str;			/* Return pointer to indicated field */
}

/*
 * Copy () copies the src line into the dest line including linefeed.
 */
void
copy(
     register char *dest,
     register char *src )
{
  while ((*dest++ = *src++) != '\n')
	;
}

/*
 * Build_field builds a new line from the src as described by the field.
 * The result is put in dest.
 */
void
build_field(
	    char *dest,				/* Holds result */
	    register FIELD *field,		/* Field description */
	    register char *src )		/* Source line */
{
  char *begin = src;				/* Remember start location */
  char *last;					/* Pointer to end location */
  int i;

/* Skip begin fields */
  src = skip_fields(src, field->beg_field);

/* Skip begin positions */
  for (i = 0; i < field->beg_pos && *src != '\n'; i++)
	src++;

/* Copy whatever is left */
  copy(dest, src);

/* If end field is assigned truncate (perhaps) the part copied */
  if (field->end_field != ERROR) {		/* Find last field */
	last = skip_fields(begin, field->end_field);
/* Skip positions as given by end fields description */
	for (i = 0; i < field->end_pos && *last != '\n'; i++)
		last++;
	dest[last - src] = '\n';		/* Truncate line */
  }
}

/*
 * Digits compares () the two strings that point to a number of digits followed 
 * by an optional decimal point.
 */
int
digits(
       register char *str1,
       register char *str2,
       BOOL check_sign )			/* LTRUE if sign must be checked */
{
  BOOL negative = LFALSE;		/* LTRUE if negative numbers */
  int diff, pow, ret;

/* Check for optional minus or plus sign */
  if (check_sign) {
	if (*str1 == '-') {
		negative = LTRUE;
		str1++;
	}
	else if (*str1 == '+')
		str1++;

	if (*str2 == '-') {
		if (negative == LFALSE)
			return HIGHER;
		str2++;
	}
	else if (negative)
		return LOWER;
	else if (*str2 == '+')
		str2++;
  }

/* Keep incrementing as long as digits are available and equal */
  while ((table[*str1] & DIGIT) && table[*str2] & DIGIT) {
	if (*str1 != *str2)
		break;
	str1++;
	str2++;
  }

/* First check for the decimal point. */
  if (*str1 == '.' || *str2 == '.') {
	if (*str1 == '.') {
		if (*str2 == '.')	/* Both. Check decimal part */
			ret = digits(str1 + 1, str2 + 1, LFALSE);
		else 
			ret = (table[*str2] & DIGIT) ? LOWER : HIGHER;
	}
	else 
		ret = (table[*str1] & DIGIT) ? HIGHER : LOWER;
  }

/* Now either two digits differ, or unknown char is seen (e.g. end of string) */
  else if ((table[*str1] & DIGIT) && (table[*str2] & DIGIT)) {
	diff = *str1 - *str2;		/* Basic difference */
	pow = 0;			/* Check power of numbers */
	while (table[*str1++] & DIGIT)
		pow++;
	while (table[*str2++] & DIGIT)
		pow--;
	ret = (pow == 0) ? diff : pow;
  }

/* Unknown char. Check on which string it occurred */
  else {
	if ((table[*str1] & DIGIT) == 0)
		ret = (table[*str2] & DIGIT) ? LOWER : SAME;
	else
		ret = HIGHER;
  }

/* Reverse sense of comparisons if negative is LTRUE. (-1000 < -1) */
  return (negative) ? -ret : ret;
}

/*
 * Cmp () is the actual compare routine. It compares according to the
 * description given in the field pointer.
 */
int
cmp(
    register char * el1,
    register char * el2,
    FIELD *field )
{
  int c1, c2;

  if (field->blanks) {		/* Skip leading blanks */
	while (table[*el1] & BLANK)
		el1++;
	while (table[*el2] & BLANK)
		el2++;
  }

  if (field->numeric)		/* Compare numeric */
	return digits(el1, el2, LTRUE);

  for (; ;) {
	while (*el1 == *el2) {
		if (*el1++ == '\n')	/* EOLN on both strings */
			return SAME;
		el2++;
	}
	if (*el1 == '\n')		/* EOLN on string one */
		return LOWER;
	if (*el2 == '\n')
		return HIGHER;
	if (field->ascii) {/* Skip chars outside 040 - 0177 */
		if ((table[*el1] & ASCII) == 0) {
			do {
				el1++;
			} while ((table[*el1] & ASCII) == 0);
			continue;
		}
		if ((table[*el2] & ASCII) == 0) {
			do {
				el2++;
			} while ((table[*el2] & ASCII) == 0);
			continue;
		}
	}
	if (field->dictionary) {/* Skip non-dict chars */
		if ((table[*el1] & DICT) == 0) {
			do {
				el1++;
			} while ((table[*el1] & DICT) == 0);
			continue;
		}
		if ((table[*el2] & DICT) == 0) {
			do {
				el2++;
			} while ((table[*el2] & DICT) == 0);
			continue;
		}
	}
	if (field->fold_case) {	/* Fold upper case to lower */
		if (table[c1 = *el1++] & UPPER)
			c1 += 'a' - 'A';
		if (table[c2 = *el2++] & UPPER)
			c2 += 'a' - 'A';
		if (c1 == c2)
			continue;
		return c1 - c2;
	}
	return *el1 - *el2;
  }
  /* NOTREACHED */
}

/*
 * Cmp_fields builds new lines out of the lines pointed to by el1 and el2 and 
 * puts it into the line1 and line2 arrays. It then calls the cmp () routine 
 * with the field describing the arguments.
 */
int
cmp_fields(
	   register char * el1,
	   register char * el2 )
{
  int i, ret = 0;
  char line1[LINE_SIZE], line2[LINE_SIZE];

  for (i = 0; i < field_cnt; i++) {	/* Setup line parts */
	build_field(line1, &fields[i + 1], el1);
	build_field(line2, &fields[i + 1], el2);
	if ((ret = cmp(line1, line2, &fields[i + 1])) != SAME)
		break;			/* If equal, try next field */
  }

/* Check for reverse flag */
  if (i != field_cnt && fields[i + 1].reverse)
	return -ret;

/* Else return the last return value of cmp () */
  return ret;
}

/*
 * Compare is called by all sorting routines. It checks if fields assignments 
 * has been made. if so, it calls cmp_fields (). If not, it calls cmp () and
 * reversed the return value if the (global) reverse flag is set.
 */
int
compare(
	register char * el1,
	register char * el2 )
{
  int ret;

  if (field_cnt > GLOBAL)
	return cmp_fields(el1, el2);

  ret = cmp(el1, el2, &fields[GLOBAL]);
  return (fields[GLOBAL].reverse) ? -ret : ret;
}


/*
 * Print_table prints the line table in the given file_descriptor. If the fd
 * equals ERROR, it opens a temp_file itself.
 */
void
print_table(int fd )
{
  register char **line_ptr;	/* Ptr in line_table */
  register char *ptr;		/* Ptr to line */
  int index = 0;			/* Index in output buffer */

  if (fd == ERROR) {
	if ((fd = creat(file_name (nr_of_files), 0644)) < 0)
		error(LTRUE, "Cannot creat ", file_name (nr_of_files));
  }

  for (line_ptr = line_table; *line_ptr != NIL_PTR; line_ptr++) {
	ptr = *line_ptr;
				/* Skip all same lines if uniq is set */
	if (uniq && *(line_ptr + 1) != NIL_PTR) {
		if (compare(ptr, *(line_ptr + 1)) == SAME)
			continue;
	}
	do {			/* Print line in a buffered way */
		out_buffer[index++] = *ptr;
		if (index == IO_SIZE) {
			mwrite(fd, out_buffer, IO_SIZE);
			index = 0;
		}
	} while (*ptr++ != '\n');
  }
  mwrite(fd, out_buffer, index);	/* Flush buffer */
  (void) close(fd);			/* Close file */
  nr_of_files++;			/* Increment nr_of_files to merge */
}

/*
 * Mread () performs a normal read (), but checks the return value.
 */
void
mread(
      int fd,
      char *address,
      register int bytes )
{
  if (read(fd, address, bytes) < 0 && bytes != 0)
	error(LTRUE, "Read error", NIL_PTR);
}

#ifndef __HELIOS
/*
 * Msbrk() does a sbrk() and checks the return value.
 */
char *msbrk(size)
register unsigned size;
{
  extern char *sbrk();
  register char *address;

  if ((address = sbrk(size)) < 0)
	error(LTRUE, "Not enough memory. Use chmem to allocate more", NIL_PTR);
  return address;
}

/*
 * Mbrk() does a brk() and checks the return value.
 */
char *mbrk(size)
register unsigned size;
{
  extern char *brk();
  register char *address;

  if ((address = brk(size)) < 0)
	error(LTRUE, "Cannot reset memory", NIL_PTR);
  return address;
}
#else
/*
 * Msbrk() does a sbrk() and checks the return value.
 */
char *msbrk(register unsigned size )
{
  register char *address;

  if ((address = (char *)malloc(size)) == NULL)
	error(LTRUE, "Not enough memory. ", NIL_PTR);
  return address;
}

/*
 * Mbrk() does a brk() and checks the return value.
 */
char *mbrk(register void * ptr )
{
  free( (char *) ptr );
  return NULL;
}
#endif

/*
 * Incr () increments the heap.
 */

void
incr(
     register int si,
     register int ei )
{
  char *tmp;

  while (si <= (ei >> 1)) {
	si <<= 1;
	if (si + 1 <= ei && compare(line_table[si - 1], line_table[si]) <= 0)
		si++;
	if (compare(line_table[(si >> 1) - 1],line_table[si - 1]) >= 0)
		return;
	tmp = line_table[(si >> 1) - 1];
	line_table[(si >> 1) - 1] = line_table[si - 1];
	line_table[si - 1] = tmp;
  }
}

/*
 * Sort_table () sorts the line table consisting of nel elements.
 */
void
sort_table(register int nel )
{
  char *tmp;
  register int i;

  /* Make heap */
  for (i = (nel >> 1); i >= 1; i--)
	incr(i, nel);
  
  /* Sort from heap */
  for (i = nel; i > 1; i--) {
	tmp = line_table[0];
	line_table[0] = line_table[i - 1];
	line_table[i - 1] = tmp;
	incr(1, i - 1);
  }
}

/*
 * Sort () sorts the input in memory starting at mem_top.
 */
void
sort()
{
  register char *ptr = mem_top;
  register int count = 0;

/* Count number of lines in memory */
  while (*ptr) {
	if (*ptr++ == '\n')
		count++;
  }
  
/* Set up the line table */
  line_table = (char **) msbrk(count * sizeof (char *)+sizeof (char *));

  count = 1;
  ptr = line_table[0] = mem_top;
  while (*ptr) {
	if (*ptr++ == '\n')
		line_table[count++] = ptr;
  }
  
  line_table[count - 1] = NIL_PTR;

/* Sort the line table */
  sort_table(count - 1);

/* Stash output somewhere */
  if (in_core) {
	open_outfile();
	print_table(out_fd);
  }
  else
	print_table(ERROR);

/* Free line table */
  mbrk(line_table);
}

/*
 * Get_file reads the whole file of filedescriptor fd. If the file is too big
 * to keep in core, a partial sort is done, and the output is stashed somewhere.
 */
void
get_file(
	 int fd,			/* Fd of file to read */
	 register long size )		/* Size of file */
{
  register int i;
  long rest;		/* Rest in memory */
  char save_ch;		/* Used in stdin readings */

  rest = MEMORY_SIZE - (long)(cur_pos - mem_top);
  if (fd == 0) {		/* We're reding stdin */
	while ((i = read(0, cur_pos, (int)rest)) > 0) {
		if ((cur_pos - mem_top) + i == MEMORY_SIZE) {
			in_core = LFALSE;
			i = last_line();	/* End of last line */
			save_ch = mem_top[i];
			mem_top[i] = '\0';
			sort ();		/* Sort core */
			mem_top[i] = save_ch;	/* Restore erased char */
				/* Restore last (half read) line */
			for (size = 0; i + size != MEMORY_SIZE; size++)
				mem_top[size] = mem_top[i + size];
				/* Assign current pos. in memory */
			cur_pos = &mem_top[size];
		}
		else {	/* Fits, just assign position in mem. */
			cur_pos = cur_pos + i;
			*cur_pos = '\0';
		}
				/* Calculate rest of mem */
		rest = MEMORY_SIZE - (long)(cur_pos - mem_top);
	}
  }					/* Reading file. Check size */
  else if (size > rest) {			/* Won't fit */
	mread(fd, cur_pos, (int)rest);
	in_core = LFALSE;
	i = last_line();		/* Get pos. of last line */
	mem_top[i] = '\0';		/* Truncate */
	(void) lseek(fd, (off_t)i - MEMORY_SIZE, 1);	/* Do this next time */
	rest = size - rest - i + MEMORY_SIZE;/* Calculate rest */
	cur_pos = mem_top;		/* Reset mem */
	sort();				/* Sort core */
	get_file(fd, rest);		/* Get rest of file */
  }
  else {					/* Fits. Just read in */
	mread(fd, cur_pos, (int)size);
	cur_pos = cur_pos + size;	/* Reassign cur_pos */
	*cur_pos = '\0';
	(void)close (fd);		/* File completed */
  }
}

/*
 * Put_line () prints the line into the out_fd filedescriptor. If line equals
 * NIL_PTR, the out_fd is flushed and closed.
 */
void
put_line(register char *line )
{
  static int index = 0;		/* Index in out_buffer */

  if (line == NIL_PTR) {		/* Flush and close */
	mwrite(out_fd, out_buffer, index);
	index = 0;
	(void) close(out_fd);
	return;
  }

  do {				/* Fill out_buffer with line */
	out_buffer[index++] = *line;
	if (index == IO_SIZE) {
		mwrite(out_fd, out_buffer, IO_SIZE);
		index = 0;
	}
  } while (*line++ != '\n');
}



/*
 * Read_line () reads a line from the fd from the merg struct. If the read
 * failed, disabled is incremented and the file is closed. Readings are
 * done in buf_size bytes.
 * Lines longer than LINE_SIZE are silently truncated.
 */
int
read_line(register MERGE *merg )
{
  register char *ptr = merg->line - 1;	/* Ptr buf that will hold line*/

  do {
	ptr++;
	if (merg->cnt == merg->read_chars) {/* Read new buffer */
		if ((merg->read_chars =
				 read(merg->fd, merg->buffer, buf_size)) <= 0) {
			(void) close(merg->fd);/* OOPS */
			merg->fd = ERROR;
			disabled++;
			return ERROR;
		}
		merg->cnt = 0;
	}
	*ptr = merg->buffer[merg->cnt++];/* Assign next char of line */
	if (ptr - merg->line == LINE_SIZE - 1)
		*ptr = '\n';		/* Truncate very long lines */
  } while (*ptr != '\n' && *ptr != '\0');
  
  if (*ptr == '\0')			/* Add '\n' to last line */
	*ptr = '\n';
  *++ptr = '\0';				/* Add '\0' */
  return OK;
}

/*
 * Print () prints the line of the merg structure and tries to read another one.
 * If this fails, it returns the next merg structure which file_descriptor is
 * still open. If none could be found, a NIL structure is returned.
 */
MERGE *
Print(
      register MERGE *merg,
      int file_cnt )				/* Nr of files that are being merged */
{
  register int i;

  put_line(merg->line);		/* Print the line */

  if (read_line(merg) == ERROR) {/* Read next line */
	for (i = 0; i < file_cnt; i++) {
		if (merge_f[i].fd != ERROR) {
			merg = &merge_f[i];
			break;
		}
	}
	if (i == file_cnt)	/* No more files left */
		return NIL_MERGE;
  }
  return merg;
}

/*
 * Uniq_lines () prints only the uniq lines out of the fd of the merg struct.
 */

void
uniq_lines(register MERGE *merg )
{
  char lastline[LINE_SIZE];		/* Buffer to hold last line */

  for (; ;) {
	put_line(merg->line);		/* Print this line */
	copy(lastline, merg->line);	/* and save it */
	if (read_line(merg) == ERROR)	/* Read the next */
		return;
				/* Keep reading until lines duffer */
	while (compare(lastline, merg->line) == SAME)
		if (read_line(merg) == ERROR)
			return;
  }
  /* NOTREACHED */
}

/*
 * Skip_lines () skips all same lines in all the files currently being merged.
 * It returns a pointer to the merge struct containing the smallest line.
 */
MERGE *
skip_lines(
	   register MERGE *smallest,
	   int file_cnt )
{
  register int i;
  int ret;

  if (disabled == file_cnt - 1)		/* We've had all */
	return smallest;

  for (i = 0; i < file_cnt; i++) {
	if (merge_f[i].fd == ERROR || smallest == &merge_f[i])
		continue;		/* Don't check same file */
	while ((ret = compare(merge_f[i].line, smallest->line)) == 0) {
		if (read_line(&merge_f[i]) == ERROR)
			break;		/* EOF */
	}
	if (ret < 0)		/* Line wasn't smallest. Try again */
		return skip_lines(&merge_f[i], file_cnt);
  }
  return smallest;
}

/*
 * Merge () merges the files between start_file and limit_file.
 */
void
merge(
      int start_file,
      int limit_file )
{
  register MERGE *smallest = NULL;	/* Keeps track of smallest line */
  register int i;
  int file_cnt = limit_file - start_file;/* Nr of files to merge */

/* Calculate size in core available for file_cnt merge structs */
  buf_size = MEMORY_SIZE / file_cnt - LINE_SIZE;

  mbrk(mem_top);		/* First reset mem to lowest loc. */
  disabled = 0;			/* All files not done yet */

/* Set up merge structures. */
  for (i = start_file; i < limit_file; i++) {
	smallest = &merge_f[i - start_file];
	if (!strcmp(file_name(i), "-"))	/* File is stdin */
		smallest->fd = 0;
	else if ((smallest->fd = open(file_name(i), O_RDONLY)) < 0) {
		smallest->fd = ERROR;
		error(LFALSE, "Cannot open ", file_name(i));
		disabled++;			/* Done this file */
		continue;
	}
	smallest->buffer = msbrk(buf_size);
	smallest->line = msbrk(LINE_SIZE);
	smallest->cnt = smallest->read_chars = 0;
	(void) read_line(smallest);		/* Read first line */
  }

  if (disabled == file_cnt) {			/* Couldn't open files */
	(void) close(out_fd);
	return;
  }

/* Find a merg struct to assign smallest. */
  for (i = 0; i < file_cnt; i++) {
	if (merge_f[i].fd != ERROR) {
		smallest = &merge_f[i];
		break;
	}
  }

/* Loop until all files minus one are done */
  while (disabled < file_cnt - 1) {
	if (uniq)		/* Skip all same lines */
		smallest = skip_lines(smallest, file_cnt);
	else {				/* Find smallest line */
		for (i = 0; i < file_cnt; i++) {
			if (merge_f[i].fd == ERROR)
				continue;/* We've had this one */
			if (compare(merge_f[i].line,smallest->line) < 0)
				smallest = &merge_f[i];
		}
	}				/* Print line and read next */
	smallest = Print(smallest, file_cnt);
  }

  if (only_merge && uniq)
	uniq_lines(smallest);		/* Print only uniq lines */
  else					/* Print rest of file */
	while (Print(smallest, file_cnt) != NIL_MERGE)
		;

  put_line(NIL_PTR);			/* Flush output buffer */
}

/*
 * Files_merge () merges all files as indicated by nr_of_files. Merging goes
 * in numbers of files that can be opened at the same time. (OPEN_FILES)
 */
void
files_merge(register int file_cnt )		/* Nr_of_files to merge */
{
  register int i;
  int limit;

  for (i = 0; i < file_cnt; i += OPEN_FILES) {
			/* Merge last files and store in output file */
	if ((limit = i + OPEN_FILES) >= file_cnt) {
		open_outfile();
		limit = file_cnt;
	}
	else {	/* Merge OPEN_FILES files and store in temp file */
#ifdef __HELIOS
		temp_files[22] = file_cnt / 26 + 'a';
		temp_files[23] = file_cnt % 26 + 'a';
#else
		temp_files[16] = file_cnt / 26 + 'a';
		temp_files[17] = file_cnt % 26 + 'a';
#endif
		if ((out_fd = creat(temp_files, 0644)) < 0)
			error(LTRUE, "Cannot creat ", temp_files);
		file_cnt++;
	}
	merge(i, limit);
  }

/* Cleanup mess */
  i = (only_merge) ? args_limit - args_offset : 0;
  while (i < file_cnt)
	(void) unlink(file_name(i++));
}


/*
 * Length () returns the length of the argument line including the linefeed.
 */
int
length(register char *line )
{
  register int i = 1;		/* Add linefeed */

  while (*line++ != '\n')
	i++;
  return i;
}


/*
 * Check_file () checks if a file is sorted in order according to the arguments
 * given in main ().
 */

void
check_file(
	   int fd,
	   char *file )
{
  register MERGE *merg;			/* 1 file only */
  char lastline[LINE_SIZE];		/* Save last line */
  register int ret;			/* ret status of compare */

  if (fd == 0)
	file = "stdin";
  merg = (MERGE *) mem_top;		/* Assign MERGE structure */
  merg->buffer = mem_top + sizeof(MERGE);
  merg->line = msbrk(LINE_SIZE);
  merg->cnt = merg->read_chars = 0;
  merg->fd = fd;
  buf_size = MEMORY_SIZE - sizeof(MERGE);

  if (read_line(merg) == ERROR)		/* Read first line */
	return;
  copy (lastline, merg->line);		/* and save it */
  
  for (; ;) {
	if (read_line(merg) == ERROR)	/* EOF reached */
		break;
	if ((ret = compare(lastline, merg->line)) > 0) {
		error(LFALSE, "Disorder in file ", file);
		write(2, merg->line, length(merg->line));
		break;
	}
	else if (ret < 0)		/* Copy if lines not equal */
		copy(lastline, merg->line);
	else if (uniq) {
		error(LFALSE, "Non uniq line in file ", file);
		write(2, merg->line, length(merg->line));
		break;
	}
  }

  mbrk(mem_top);			/* Reset mem */
}

int
main(
     int argc,
     char *argv[] )
{
  int arg_count = 1;			/* Offset in argv */
  struct stat st;
  register char *ptr;			/* Ptr to *argv in use */
  register int fd;
  int pid, pow;

  argptr = argv;
  cur_pos = mem_top = msbrk(MEMORY_SIZE);	/* Find lowest mem. location */

  while (argc > 1 && ((ptr = argv[arg_count])[0] == '-' || *ptr == '+')) {
	if (*ptr == '-' && *(ptr + 1) == '\0')	/* "-" means stdin */
		break;
	if (*ptr == '+') {		/* Assign field. */
		if (++field_cnt == FIELDS_LIMIT)
			error(LTRUE, "Too many fields", NIL_PTR);
		new_field (&fields[field_cnt], &arg_count, LTRUE);
	}
	else {				/* Get output options */
		while (*++ptr) { switch (*ptr) {
			case 'c' :	/* Only check file */
				check = LTRUE;
				break;
			case 'm' :	/* Merge (sorted) files */
				only_merge = LTRUE;
				break;
			case 'u' :	/* Only give uniq lines */
				uniq = LTRUE;
				break;
			case 'o' :	/* Name of output file */
				output_file = argv[++arg_count];
				break;
			case 't' :	/* Field separator */
				ptr++;
				separator = *ptr;
				break;
			default :	/* Sort options */
				get_opts(ptr, &fields[GLOBAL]);
			}
		}
		arg_count++;
	}
  }

  for (fd = 1; fd <= field_cnt; fd++)
	adjust_options (&fields[fd]);

/* Create name of tem_files 'sort.pid.aa' */
#ifdef __HELIOS
  ptr = &temp_files[16];
#else
  ptr = &temp_files[10];
#endif
  pid = getpid();
  pow = 10000;
  while (pow != 0) {
	*ptr++ = pid / pow + '0';
	pid %= pow;
	pow /= 10;
  }

  signal (SIGINT, Catch);

/* Only merge files. Set up */
  if (only_merge) {
	args_limit = args_offset = arg_count;
	while (argv[args_limit] != NIL_PTR)
		args_limit++;		/* Find nr of args */
	files_merge(args_limit - arg_count);
	exit (0);
  }

  if (arg_count == argc) {		/* No args left. Use stdin */
	if (check)
		check_file(0, NIL_PTR);
	else
		get_file(0, 0L);
  }
  else while (arg_count < argc) {	/* Sort or check args */
	if (strcmp(argv[arg_count], "-") == 0)
		fd = 0;
	else if (stat(argv[arg_count], &st) < 0) {
		error(LFALSE, "Cannot find ", argv[arg_count++]);
		continue;
	}				/* Open files */
	else if ((fd = open(argv[arg_count], O_RDONLY)) < 0) {
		error(LFALSE, "Cannot open ", argv[arg_count++]);
		continue;
	}
	if (check)
		check_file(fd, argv[arg_count]);
	else				/* Get_file reads whole file */
		get_file(fd, st.st_size);
	arg_count++;
  }

  if (check)
	exit(0);

  sort();			/* Sort whatever is left */

  if (nr_of_files == 1)		/* Only one file sorted -> don't merge*/
	exit(0);

  files_merge(nr_of_files);
  exit(0);
}
