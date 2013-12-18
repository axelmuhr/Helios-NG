#define HELIOS
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/textutil/RCS/diff.c,v 1.7 1994/05/12 13:40:24 nickc Exp $";
#endif

/*
 * Last Hacked by Robert A. Larson, blarson@ecla.usc.edu, Nov 29 86
 *   OSK support (except for error routines).
 *   Real context diffs, with a couple of minor problems:
 *	  If the first change is deleting leading lines, and the second
 *	such that the context overlaps the deleted lines, the deleted
 *	lines are output as context.  This is consistant with other
 *	cases of overlapping context, but patch doesn't like it.  It's
 *	not hard to manually fix the diff in this (rare?) case.
 *	  File modifacation times are not output.
 *	  At most 9 lines of context is output.
 *
 * Previously Hacked by John Gilmore, hoptoad!gnu, 26Sep86.
 * Compiles and runs under Unix.  Much faster since it doesn't reallocate
 * every data structure in the inner loop(!).  Compatible with Unix diff
 * output format, though it occasionally finds different sets of changed
 * lines (both are valid).  -c option needs work.  Also, ftell's in
 * <check> should be dumped when possible.
 *
From: EVERHART%ARISIA%rca.com@CSNET-RELAY.ARPA
Message-Id: <8608201845.AA15181@lll-crg.ARPA>
Date:     Wed, 20 Aug 86 10:34 EDT
To: hoptoad!gnu@LLL-CRG.ARPA
Subject:  Decus C DIFF (partially moved to Amiga) source.
 */

/*
 *  	  D I F F
 */

/* For VMS:
  )BUILD  $(TKBOPTIONS) = {
  	  TASK  = ...DIF
  	}
*/

/*
 *	OSK changes: since OSK doesn't have realloc, put the size 
 *	allocated in the head of each allocated region.  This code
 *	assumes int is a maximally alligned type.  There is AMIGA
  *	code to avoid realloc, but it is broken.
 */

#ifdef  DOCUMENTATION
title  diff  Differential File Comparison
index  	Differential File Comparison

synopsis

  diff [option] file1 file2

description

  Diff compares two files, showing what must be changed to make
  them identical. Either file1 or file2 (but not both) may refer
  to directories.  If that is the case, a file in the directory
  whose name is the same as the other file argument will be used.
  The standard input may be used for one of the files by replacing
  the argument by "-".  Except for the standard input, both files
  must be on disk devices.
  .s
  Options:
  .lm +8
  .s.i -8;-b  Remove trailing whitespace (blanks and tabs)
  and compress all other strings of whitespace to a single blank.
  .s.i -8;-c  Print some context -- matching lines before
  and after the non-match section.  Mark non-matched sections
  with "|".
  .s.i -8;-i  Ignore lower/upper case distinctions.
  .s.i -8;-e  Output is in an "editor script" format which
  is compatible with the Unix 'ed' editor.
  .s.lm -8
  All information needed to compare the files is maintained in main
  memory. This means that very large files (or fairly large files with
  many differences) will cause the program to abort with an "out of space"
  message.  Main memory requirements (in words) are approximately:
  .s
  	2 * (length of file1 + length of file2)
  .br
  	+ 3 * (number of changes)
  .s
  (Where "length" is the number of lines of data in each file.)
  .s
  The algorithm reads each file twice, once to build hash tables and once
  to check for fortuitous matches (two lines that are in fact different,
  but which have the same hash value).  CPU time requirements include
  sorting the hash tables and randomly searching memory tables for
  equivalence classes. For example, on a time-shared VAX-11/780,
  comparing two 1000 line files required about 30 seconds (elapsed
  clock time) and about 10,000 bytes of working storage.  About 90
  per-cent of the time was taken up by file I/O.

diagnostics

  .lm +8
  .s.i -8;Warning, bad option 'x'
  .s
  The option is ignored.
  .s.i -8;Usage ...
  .s
  Two input files were not specified.
  .s.i -8;Can't open input file "filename".  Can't continue.
  .s.i -8;Out of space
  .s
  The program ran out of memory while comparing the two files.
  .s.i -8;Cannot read line nnn at xxx in file[A/B]
  .s
  This indicates an I/O error when seeking to the specific line.
  It should not happen.
  .s.i -8;Spurious match, output is not optimal.
  .s
  Two lines that were different yielded the same hash value.  This is
  harmless except that the difference output is not the minimum set of
  differences between the two files.  For example, instead of the output:
  .s
  	lines 1 to 5 were changed to ...
  .s
  the program will print
  .s
  	lines 1 to 3 were changed to ...
  .br
  	lines 4 to 5 were changed to ...
  .s
  The program uses a CRC16 hash code.  The likelihood of this error is
  quite small.
  .lm -8

author

  The diff algorithm was developed by J. W. Hunt and M. D. McIlroy,
  using a central algorithm defined by H. S. Stone.
  It was published in:
  .s.lm +4.nf
  Hunt, J. W., and McIlroy, M. D.,
  An Algorithm for Differential File Comparison,
  Computing Science Technical Report #41,
  Bell Laboratories, Murray Hill, NJ  07974
  .s.lm -4.f

bugs

  On RSX and DECUS C on VMS systems, diff may fail if the both
  files are not "variable-length, implied carriage control"
  format.  The scopy program can be used to convert files
  to this format if problems arise.

  When compiled under VAX C, diff handles STREAM_LF files
  properly (in addition to the canonical variable-length implied
  carriage control files).  Other variations should work, but
  have not been tested.

  When compiled under VAX C, diff is quite slow for unknown reasons
  which ought to be investigated.  On the other hand, it has access to
  effectively unlimited memory.

  Output in a form suitable for ed - the -e option - seems rather
  pointless; the analogue on DEC systems is SLP (SUMSLP on VMS).
  It would be simple to provide SLP-compatible output.  The question
  is, why bother - since the various DEC file comparison utilities
  already produce it.

#endif

/*
 * Diff maintains all information needed to compare the two files in main
 * memory.  This means that very large files (or fairly large files with
 * many differences) will cause the program to abort with an "out of space"
 * error.  Main memory requirements (in words) are approximately:
 *
 *  2 * (length of file1 + length of file2) + (3 * number of changes)
 *
 * The diff algorithm reads each file twice (once to build hash tables and
 * a second time to check for fortuitous matches), then reads the differences
 * by seeking randomly within the files.  CPU time requirements include
 * sorting the two hash vectors and randomly searching memory tables for
 * equivalence classes.  For example, running in Vax compatibility
 * mode, two 1000 line files with a fair number of differences took
 * about 25 seconds (elapsed wall clock time) for processing.  Most of this
 * time was spent in the file read routines.  This test required slightly
 * more than 6000 words of memory for internal tables.
 *
 * The diff algorithm was developed by J. W. Hunt and M. D. McIlroy,
 * using a central algorithm defined by H. S. Stone.  The algorithm
 * was described in:
 *
 *  Hunt, J. W., and McIlroy, M. D.,
 *  An Algorithm for Differential File Comparison,
 *  Computing Science Technical Report #41,
 *  Bell Laboratories, Murray Hill, NJ  07974
 *  
 * The following description is summarized from that document.  While
 * it has been slightly modified to correspond to the program source, the
 * algorithm is essentially identical.
 *
 * 1.  Read the input files, building two vectors containing the
 *  line number (serial) and hash value (hash) of each line.
 *  Data for fileA will be in a vector pointed to by fileA[],
 *  while data for fileB will be pointed to by fileB[]. The
 *  lengths (number of lines) of the files will be represented
 *  by lenA and lenB respectively.  [This is slightly different
 *  from the published algorithm.]
 *
 * 2.  Note initial and final sequences that have identical
 *  hash values to shorten subsequent processing.  Note that
 *  the "jackpot" phase (step 9.) will examine all lines in
 *  the file.  Next, sort each file using hash as the primary
 *  key and serial as the secondary key.
 *
 * 3.  Construct an array of equivalence classes (member[1..lenB])
 *  where each element contains the line number in fileB and a
 *  flag which is True if the indicated line is the first member
 *  of an equivalence class.  (An equivalence class is a set of
 *  lines which all hash to the same value.  The lines themselves
 *  are not necessarily identical.)
 *
 * 4.  Construct an array, class[1..lenA], where each element, I, is set to
 *  the index of a line, J, in fileB if member[J] is the first
 *   element in an equivalence class and the hash code of line[I] in
 *  fileA is the same as the hash code of line[J] in fileB.  Class[I]
 *  is set to zero if no such line exists.
 *
 *  If non-zero, class[I] now points in member[] to the beginning of
 *  the class of lines in fileB equivalent to line[I] in fileA.
 *
 * The next two steps implement the longest common subsequence algorithm.
 *
 * 5.  Structure CANDIDATE { a, b, previous }, where a and b are line
 *   numbers and previous a reference to a candidate, will store
 *  candidate lists as they are constructed.
 *
 *  Vector clist[] stores references to candidates.  It is dimensioned
 *  (0..min(lenA, lenB) + 1)
 *
 *  Initialize
 *  	clist[0] = CANDIDATE {   0,   0, -1 };
 *  	clist[1] = CANDIDATE { A+1, B+1, -1 };
 *  	ktop = 1;
 *
 *  clist[1] is a fence beyond the last usefully filled element
 *  and -1 is an out-of-range clist index. Ktop is the index of the
 *  fence.  Note, because of the memory allocation used, clist[]
 *  is actually composed of two vectors, clist[] containing the
 *  candidate reference, and klist[] containing pointers to clist.
 *
 * 6.  For (A = 1 to lenA) {
 *  	I = class[A];  -- the index in member[]:  beginning of
 *  	  	-- the class of lines in fileB equivalent
 *  	  	-- to this line in fileA.
 *  	if (I is non-zero) {
 *  	  Merge each member into the candidate list
 *  	  as discussed below.
 *  	}
 *
 * Unravel the chain of candidates, getting a vector of common subsequences:
 *
 * 7.  Set all elements of match[0..lenA] to zero.
 *
 * 8.  clist[ktop-1] points to the subsequence chain head.  For each element
 *  in the chain, let A and B be the line number entries.  Then, set
 *
 *  	match[A] = B;
 *
 *  The non-zero elements of match[]  now pick out a longest common
 *  subsequence chain, possibly including spurious matches due to
 *  hash coincidences.  The pairings between the two files are:
 *
 *  if (match[A] is non-zero) {
 *  	line A in fileA matches line match[A] in fileB;
 *  }
 *
 * Now, read each line of fileA and fileB to check for jackpots:
 *
 * 9.  for (A = 1 to lenA) {
 *  	if (match[A] is nonzero) {
 *  	  if (fileA[A] is not identical to fileB[match[A]])
 *  	  	match[A] = 0;  -- Hash congruence
 *  	}
 *  }
 *
 * Ignoring "squish" complications, the merge step may be defined as follows:
 *
 *  Entry:
 *  	clist[]  	Candidate pointer array
 *  	ktop  	Fence beyond last filled index
 *  	A  	Current index in fileA
 *  	member[]  Equivalence vector
 *  	I  	The index in member[] of the first element
 *  	  	  of the class of lines in fileB that are
 *  	  	  equivalent to line[A] in fileA.
 *
 * 1.  Let clist[R] be "an r-candidate" and C a reference to
 *  the last candidate found, which will always be an r-candidate.
 *  clist[R] will be updated with this reference once the previous
 *  value of clist[R] is no longer needed.  Initialize:
 *
 *  	R = 0;
 *  	C = clist[0];
 *
 * 2.  Do steps 3 through 6 repeatedly:
 *
 *   3.  set B to the line number in member[I];
 *  search clist[R..ktop] for an element, clist[S], such that
 *
 *  	clist[S-1].b < B and clist[S].b >= B
 *
 *  Note that clist[] is ordered on clist[].b so that binary
 *  search will work.  The search algorithm used requires the
 *  two "fence" entries described above.
 *
 *  If such an element is found, perform steps 4. and 5.
 *
 *  4. Extend the longest common subsequence chain:
 *
 *  	If (clist[S].b > B) {
 *  	  clist[R] = C;
 *  	  R = S;
 *  	  C = candidate(A, B, clist[S - 1]);
 *  	}
 *
 *  5. Extend the number of subsequences, moving the fence:
 *
 *  	If (S == ktop) {
 *  	  clist[ktop + 1] = clist[ktop]
 *  	  ktop = ktop + 1;
 *  	  break out of step 2's loop;
 *  	}
 *
 *   6.  I = I + 1;
 *  if (member[I] is the first element in another class) {
 *  	break out of step 2's loop;
 *  }
 *  else {
 *  	continue at step 2.
 *  }
 *
 * 7.  clist[R] = C;
 *  exit merge subroutine.
 *
 * To illustrate vector contents, here is a sample run:
 *
 * File A:
 *  line 1
 *  line 2
 *  line 3
 *  line 4
 *  line 5 gets deleted
 *  line 6
 *
 * File B:
 *  line 1
 *  line 1.5 inserted
 *  line 2
 *  line 3 changed
 *  line 4
 *  line 6
 *
 * (For clarity, the "squish" step is omitted from the following)
 *
 * On entry to equiv() (after readin and sorting), the file[] vector is
 * as follows (the first entry in each pair is the line number, the
 * second is the hash value.  Entries are sorted on hash value):
 *
 * FileA[] (1..lines in fileA):
 *   line   hash
 *  3 042400  6 043300  5 050026  1 102201  2 102701  4 103501
 * FileB[] (1..lines in fileB):
 *  6 043300  2 045600  1 102201  3 102701  5 103501  4 147166
 *
 *
 * After Equiv has processed file[]:
 *
 * FileA[] (1..lines in fileA):
 *   line value
 *  3 0  6 1  5 0  1 3  2 4  4 5
 * Member[] (0..lines in fileB)
 *  0  -6  -2  -1  -3  -5  -4
 *
 *
 * After unsort() has unwound fileB:
 *
 * Class[] (1 .. lines in fileA):
 *  3   4  0  5  0  1
 *
 * Within unravel(), match is built in the following order:
 *
 *  match[6] := 6
 *  match[4] := 5
 *  match[2] := 3
 *  match[1] := 1
 *
 * Match[] (0 .. lines in fileA):
 *
 *   0  1  3  0  5  0  6
 *
 * Output is as follows:
 *
 *  1a2
 *  > line 1.5 inserted
 *  3c4
 *  < line 3
 *  ---
 *  > line 3 changed
 *  5d5
 *  < line 5 gets deleted
 *
 *
 */
#ifdef HELIOS
#include <syslib.h>
#endif

#ifndef HELIOS
#define TRUE 1
#define FALSE 0
typedef void int;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/*
 * Note: IO_SUCCESS and IO_ERROR are defined in the Decus C stdio.h file
 */
#ifndef  IO_SUCCESS
#define  IO_SUCCESS  0
#endif
#ifndef  IO_ERROR
#define  IO_ERROR  1
#endif

#define  EOS  	0

typedef struct candidate
  {
    long  	b;  	  /* Line in fileB  	*/
    long  	a;  	  /* Line in fileA  	*/
    long  	link;  	  /* Previous candidate  	*/
  }
CANDIDATE;

typedef struct line
  {
    unsigned long	hash;  		/* Hash value etc.  	*/
    long  		serial;  	/* Line number  	  */
  }
LINE;

LINE *		file[ 2 ];  	  /* Hash/line for total file  */
#define  fileA  file[ 0 ]
#define  fileB  file[ 1 ]

LINE *		sfile[ 2 ];  	  /* Hash/line after prefix  */
#define  sfileA sfile[ 0 ]
#define  sfileB sfile[ 1 ]

long  		len[ 2 ];  	  	/* Actual lines in each file  */
#define  lenA   len[ 0 ]
#define  lenB   len[ 1 ]

long  		slen[ 2 ];  	  /* Squished lengths  	*/
#define  slenA  slen[ 0 ]
#define  slenB  slen[ 1 ]

long  		prefix;  	  	/* Identical lines at start  */
long  		suffix;  	  	/* Identical lenes at end  */

FILE *		infd[2] = { NULL, NULL };  /* Input file identifiers  */
FILE *		tempfd;  	  	/* Temp for input redirection  */


/*
 * The following vectors overlay the area defined by fileA
 */

long *		Class;  	/* Unsorted line numbers  */
long *		klist;  	/* Index of element in clist  */
CANDIDATE *	clist;  	/* Storage pool for candidates  */
long  		clength = 0;  	/* Number of active candidates  */
#define		CSIZE_INC 50	/* How many to allocate each time we have to */
long		csize = CSIZE_INC; /* Current size of storage pool */

long *		match;  	  /* Longest subsequence  	*/
long *		oldseek;  	/* Seek position in file A  */

/*
 * The following vectors overlay the area defined by fileB
 */

long *		member;  	/* Concatenated equiv. classes  */
long *		newseek;  	/* Seek position in file B  */
char *		textb;  	  /* Input from file2 for check  */

/*
 * Global variables
 */

long  		eflag  = FALSE;  /* Edit script requested  */
long  		bflag  = FALSE;  /* Blank supress requested  */
long  		cflag  = FALSE;  /* Context printout  	*/
long  		iflag  = FALSE;  /* Ignore case requested  */
char  		text[ 257 ];  	 /* Input line from file1  */

int 		foundadiff = 0 ; /* global flag to see if we had a diff */

#ifdef HELIOS
#ifdef  DEBUG
void rdump(long *,char *);
#endif
#ifdef  DEBUG
void dump(LINE *,long,long);
void dumpklist(long, char *);
#endif
#endif

#ifdef  DEBUG
#define  TIMING
#endif
#ifdef  TIMING
extern long  	time();
/* extern char *12226mend; */
long  		totaltime;
long  		sectiontime;
char *		mstart;
#endif


void
fputss(
       register char *	s,
       register FILE *	iop )
/*
 * Like fput() except that it puts a newline at the end of the line.
 */
{
  /*
   * Why wasn't this written like the OSK section?  What's the difference between
   * fputc and putc other than I've never heard of fputc?
   */
  register c;


  while ((c = *s++) != '\0')
    fputc(c, iop);

  fputc('\n', iop);

  return;
  
} /* fputss */


/*
 * Fgetss() is like fgets() except that the terminating newline
 * is removed.
 */

char *
fgetss(
       char *		s,
       long 		n,
       register FILE *	iop )
{
  register 		c;
  register char *	cs;

  
  cs = s;

  /*
   * The getc in the next line used to be an "fgetc".  Change it back if
   * getc doesn't work on your system, though that would be odd.
   */

  while ((c = getc(iop)) >= 0 && --n > 0)
    {
      if (c=='\n')
	break;

      *cs++ = c;
    }

  if (c < 0 && cs == s)
    return ((char *)NULL);

  *cs = '\0';  	  /* Overwrite newline as null  */

  return (s);
  
} /* fgetss */

/*
 * Input routine, read one line to buffer[], return TRUE on eof, else FALSE.
 * The terminating newline is always removed.  If "-b" was given, trailing
 * whitespace (blanks and tabs) are removed and strings of blanks and
 * tabs are replaced by a single blank.  Getline() does all hacking for
 * redirected input files.
 */

long
getline(
  FILE *	fd,
  char *	buffer )
{
  register char *	top;
  register char *	fromp;
  register char  	c;

  
  if (fgetss(buffer, sizeof text, fd) == NULL)
    {
      *buffer = EOS;

      return(TRUE);
    }
  
  if (fd == stdin)
    fputss(buffer, tempfd);

  if (bflag || iflag)
    {
      top   = buffer;
      fromp = buffer;

      while ((c = *fromp++) != EOS)
	{
  	  if (bflag && (c == ' ' || c == '\t'))
	    {
	      c = ' ';

	      while (*fromp == ' ' || *fromp == '\t')
		fromp++;
	    }

  	  if (iflag)
	    c = tolower(c);

	  *top++ = c;
  	}

      if (bflag && top[-1] == ' ')
	top--;

      *top = EOS;
    }
  
  return(FALSE);

} /* getline */


long
streq(
  register char  *s1,
  register char  *s2 )
/*
 * TRUE if strings are identical
 */
{
  while (*s1++ == *s2)
    {
      if (*s2++ == EOS)
  	return (TRUE);
    }

  return (FALSE);

} /* streq */


static unsigned long crc16a[] =
  {
  0000000,  0140301,  0140601,  0000500,
  0141401,  0001700,  0001200,  0141101,
  0143001,  0003300,  0003600,  0143501,
  0002400,  0142701,  0142201,  0002100,  
};

static unsigned long crc16b[] =
  {
  0000000,  0146001,  0154001,  0012000,
  0170001,  0036000,  0024000,  0162001,
  0120001,  0066000,  0074000,  0132001,
  0050000,  0116001,  0104001,  0043000,
};


unsigned long
hash( char * buffer )
/*
 * Return the CRC16 hash code for the buffer
 * Algorithm from Stu Wecker (Digital memo 130-959-002-00).
 */
{
  register unsigned long  crc;
  register char  	*tp;
  register long   	temp;


  crc = 0;

  for (tp = buffer; *tp != EOS;)
    {
      temp = *tp++ ^ crc;  /* XOR crc with new char  */

      crc = (crc >> 8)
	^ crc16a[ (temp & 0017) ]
  	  ^ crc16b[ (temp & 0360) >> 4 ];
    }

#ifdef  DEBUG_ALL
  printf("%06o: %s\n", (crc == 0) ? 1 : crc, buffer);
#endif

  return((crc == 0) ? (unsigned long) 1 : crc);

} /* hash */


void
noroom( char * why )
{
  fprintf(stderr, "?DIFF-F-out of room when %s\n", why);

  exit(IO_ERROR+1);	/* ACC make error > 1 */
}


char *
myalloc(
	long	amount,
	char *	why )
/*
 * Allocate or crash.
 */
{
  register char  *pointer;

  if ((pointer = (char *)Malloc( (unsigned long) amount) ) == NULL)
    noroom( why );

  return (pointer);

} /* myalloc */


/*
 * Reallocate pointer, compacting storage
 *
 * The "compacting storage" part is probably not relevant any more.
 * There used to be horrid code here that malloc'd one byte and freed
 * it at magic times, to cause garbage collection of the freespace
 * or something.  It's safely gone now, you didn't have to see it.
 *	-- John Gilmore, Nebula Consultants, Sept 26, 1986
 */

char *
compact(
	char *	pointer,
	long  	new_amount,
	char *	why )
{
  register char *	new_pointer;
  extern void *		realloc( void *, size_t );


  /* XXX - NC - 9/4/92 - should this be a call to Realloc() ???? */
  
  if ((new_pointer = (char *) realloc( (void *)pointer, (size_t) new_amount)) == NULL)
    {
      noroom(why);
    }

#ifdef  DEBUG
  if (new_pointer != pointer)
    {
      fprintf( stderr, "moved from %06o to %06o\n",
	       pointer, new_pointer );
    }

  /*  rdump(new_pointer, why);
   */

#endif

  return (new_pointer);

} /* compact */


void
input(
      long	which )  	  /* 0 or 1 to redefine infd[]  */
/*
 * Read the file, building hash table
 */
{
  register LINE *	lentry;
  register long  	linect = 0;
  FILE *		fd;
#define	LSIZE_INC 200	/* # of line entries to alloc at once */
  long			lsize = LSIZE_INC;

  
  lentry = (LINE *)myalloc(sizeof(LINE) * (lsize+3), "line");
  
  fd = infd[which];
  
  while (!getline(fd, text))
    {
      if (++linect >= lsize)
	{
	  lsize += 200;

	  lentry = (LINE *)compact( (char *)lentry,
				    (lsize + 3) * sizeof(LINE),
				   "extending line vector");
	}

      lentry[linect].hash = hash(text);
    }

  /*
   * If input was from stdin ("-" command), finish off the temp file.
   */

  if (fd == stdin)
    {
#ifdef OLDCODE
      fclose(tempfd);
      tempfd = infd[which] = tmpfile();
#endif
      infd[which] = tempfd;		/* switch from stdin to temp file */
    }

  /* If we wanted to be stingy with memory, we could realloc lentry down
   * to its exact size (+3 for some odd reason) here.  No need?  */

  len[which]  = linect;
  file[which] = lentry;

  return;
  
}


void
squish( void )
/*
 * Look for initial and trailing sequences that have identical hash values.
 * Don't bother building them into the candidate vector.
 */
{
  register long  	i;
  register LINE *	ap;
  register LINE *	bp;
  long  		j;
  long  		k;


  /*
   * prefix -> first line (from start) that doesn't hash identically
   */

  i  = 0;
  ap = &fileA[ 1 ];
  bp = &fileB[ 1 ];
  
  while (i < lenA && i < lenB && ap->hash == bp->hash)
    {
      i++; ap++; bp++;
    }
  
  prefix = i;
  
  /*
   * suffix -> first line (from end) that doesn't hash identically
   */
  
  j  = lenA - i;
  k  = lenB - i;  
  ap = &fileA[ lenA ];
  bp = &fileB[ lenB ];
  i  = 0;
  
  while (i < j && i < k && ap->hash == bp->hash)
    {
      i++; ap--; bp--;
    }
  
  suffix = i;
  
  /*
   * Tuck the counts away
   */
  
  for (k = 0; k <= 1; k++)
    {
      sfile[k] = file[k] + prefix;
      j        = slen[k] = len[k] - prefix - suffix;

      for (i = 0, ap = sfile[k]; i <= slen[k]; i++, ap++)
	{
  	  ap->serial = i;
  	}
    }

  return;
  
} /* squish */


void
sort(
     LINE *	vector,  	/* What to sort  	  */
     long  	vecsize )  	/* How many to sort  	*/
/*
 * Sort hash entries
 */
{
  register long  	j;
  register LINE *	aim;
  register LINE *	ai;
  long  		mid;  
  long  		k;
  LINE  		work;


  for (j = 1; j <= vecsize; j *= 2)
    ;
  
  mid = (j - 1);
  
  while ((mid /= 2) != 0)
    {
      k = vecsize - mid;

      for (j = 1; j <= k; j++)
	{
  	  for (ai = &vector[ j ]; ai > vector; ai -= mid)
	    {
	      aim = &ai[ mid ];

	      if (aim < ai)
		break;  /* ?? Why ??  	*/

	      if (aim->hash > ai->hash ||
		  aim->hash == ai->hash &&
		  aim->serial > ai->serial)
		break;

	      work.hash   = ai->hash;
	      ai->hash    = aim->hash;
	      aim->hash   = work.hash;
	      work.serial = ai->serial;
	      ai->serial  = aim->serial;
	      aim->serial = work.serial;
	    }
  	}
    }

  return;

} /* sort */


void
equiv( void )
/*
 * Build equivalence class vector
 */
{
  register LINE *	ap;
  register union
    {
      LINE *		bp;
      long *		mp;
    } 			r;
  register long  	j;
  LINE *		atop;


#ifdef  DEBUG
  printf("equiv entry\n");

  for (j = 1; j <= slenA; j++)
    printf("sfileA[%d] = %6d %06o\n",
	   j, sfileA[j].serial, sfileA[j].hash);

  for (j = 1; j <= slenB; j++)
    printf("sfileB[%d] = %6d %06o\n",
	   j, sfileB[j].serial, sfileB[j].hash);
#endif

  j    = 1;
  ap   = &sfileA[1];
  r.bp = &sfileB[1];
  atop = &sfileA[slenA];
  
  while (ap <= atop && j <= slenB)
    {
      if (ap->hash < r.bp->hash)
	{
  	  ap->hash = 0;
  	  ap++;
  	}
      else if (ap->hash == r.bp->hash)
	{
  	  ap->hash = j;
  	  ap++;
  	}
      else
	{
  	  r.bp++;
  	  j++;
  	}
    }

  while (ap <= atop)
    {
      ap->hash = 0;
      ap++;
    }
  
  sfileB[slenB + 1].hash = 0;
  
#ifdef  DEBUG
  printf("equiv exit\n");

  for (j = 1; j <= slenA; j++)
    printf("sfileA[%d] = %6d %06o\n",
	   j, sfileA[j].serial, sfileA[j].hash);

  for (j = 1; j <= slenB; j++)
    printf("sfileB[%d] = %6d %06o\n",
	   j, sfileB[j].serial, sfileB[j].hash);
#endif

  ap   = &sfileB[0];
  atop = &sfileB[slenB];
  r.mp = &member[0];
  
  while (++ap <= atop)
    {
      r.mp++;
      *r.mp = -(ap->serial);

      while (ap[1].hash == ap->hash)
	{
  	  ap++;
  	  r.mp++;
  	  *r.mp = ap->serial;
  	}
    }
  
  r.mp[1] = -1;

#ifdef  DEBUG
  for (j = 0; j <= slenB; j++)
    printf("member[%d] = %d\n", j, member[j]);
#endif

  return;
  
} /* equiv */

void
unsort( void )
/*
 * Build class vector
 */
{
  register long *	temp;
  register long *	tp;
  register union
    {
      LINE *		ap;
      long *		cp;
    } 			u;
  LINE *		evec;
  long *		eclass;
#ifdef  DEBUG
  long  		i;
#endif

  
  temp = (long *)myalloc((slenA + 1) * sizeof(long), "unsort scratch");

  u.ap = &sfileA[1];
  evec = &sfileA[slenA];
  
  while (u.ap <= evec)
    {
#ifdef  DEBUG
      printf("temp[%2d] := %06o\n", u.ap->serial, u.ap->hash);
#endif

      temp[u.ap->serial] = u.ap->hash;
      u.ap++;
    }

  /*
   * Copy into class vector and free work space
   */

  u.cp   = &Class[1];
  eclass = &Class[slenA];
  tp     = &temp[1];
  
  while (u.cp <= eclass)
    *u.cp++ = *tp++;

  Free((char *) temp);

#ifdef  DEBUG
  printf("unsort exit\n");

  for (i = 1; i <= slenA; i++)
    printf("class[%d] = %d %06o\n", i, Class[i], Class[i]);
#endif

  return;
  
} /* unsort */


long
search(
       register unsigned long 	low,
       register unsigned long 	high,
       register long  		b )
/*
 * Search klist[low..top] (inclusive) for b.  If klist[low]->b >= b,
 * return zero.  Else return s such that klist[s-1]->b < b and
 * klist[s]->b >= b.  Note that the algorithm presupposes the two
 * preset "fence" elements, (0, 0) and (slenA, slenB).
 */
{
  register long  		temp;
  register unsigned long  	mid;


  if (clist[ klist[ low ] ].b >= b)
    return (0);
  
  while ((mid = (low + high) / 2) > low)
    {
      if ((temp = clist[ klist[ mid ] ].b) > b)
	high = mid;
      else if (temp < b)
	low = mid;
      else 
	return (mid);
    }

  return (mid + 1);

} /* search */


long
newcand(
	long  	a,	  	/* Line in fileA  	  */
	long  	b,  		/* Line in fileB  	  */
	long  	pred )  	/* Link to predecessor, index in cand[]  */
{
  register CANDIDATE  *New;


  clength++;

  if (++clength >= csize)
    {
      csize += CSIZE_INC;

      clist = (CANDIDATE *)compact((char *)clist,
				   csize * sizeof (CANDIDATE),
				   "extending clist");
    }

  New = &clist[clength - 1];

  New->a    = a;
  New->b    = b;
  New->link = pred;
  
  return(clength - 1);

} /* newcand */


long
subseq( void )
/*
 * Generate maximum common subsequence chain in clist[]
 */
{
  long  	  		a;
  register unsigned long	ktop;
  register long  		b;
  register long  		s;
  unsigned long			r;
  long  	  		i;
  long  	  		cand;


  klist[0] = newcand(0, 0, -1);
  klist[1] = newcand(slenA + 1, slenB + 1, -1);
  ktop = 1;  	  	/* -> guard entry  */

  for (a = 1; a <= slenA; a++)
    {
      /*
       * For each non-zero element in fileA ...
       */

      if ((i = Class[a]) == 0)
	continue;

      cand = klist[0];  	/* No candidate now  */
      r = 0;  	  	/* Current r-candidate  */

      do
	{
#ifdef  DEBUG
  	  printf("a = %d, i = %d, b = %d\n", a, i, member[i]);
#endif
  	  /*
  	   * Perform the merge algorithm
  	   */

  	  if ((b = member[i]) < 0)
	    b = -b;

#ifdef  DEBUG
  	  printf("search(%d, %d, %d) -> %d\n",
		 r, ktop, b, search(r, ktop, b));
#endif
  	  if ((s = search(r, ktop, b)) != 0)
	    {
	      if (clist[klist[s]].b > b)
		{
  	  	  klist[r] = cand;
  	  	  r        = s;
  	  	  cand     = newcand(a, b, klist[s-1]);
#ifdef  DEBUG
  	  	  dumpklist(ktop, "klist[s-1]->b > b");
#endif
  	  	}

	      if (s >= ktop)
		{
  	  	  klist[ktop + 1] = klist[ktop];
  	  	  ktop++;
#ifdef  DEBUG
  	  	  klist[r] = cand;
  	  	  dumpklist(ktop, "extend");
#endif
  	  	  break;
  	  	}
	    }
  	}
      while (member[++i] > 0);

      klist[r] = cand;
    }

#ifdef  DEBUG
  printf("last entry = %d\n", ktop - 1);
#endif

  return(ktop - 1);  	  /* Last entry found  */

} /* subseq */



void
_error( void )
{
  exit( 2 );	/* ACC make error > 1 */
}

/* VARARGS */
void
error(
  char *	format,
  int		arg )
/*
 * Error message before retiring.
 */
{
  fprintf( stderr, format, arg );
  putc( '\n', stderr );
  _error();
}


void
unravel( register long k )
{
  register long  	i;
  register CANDIDATE *	cp;
  long  	  	first_trailer;
  long  	  	difference;

  
  first_trailer = lenA - suffix;
  difference    = lenB - lenA;

#ifdef  DEBUG
  printf("first trailer = %d, difference = %d\n",
	 first_trailer, difference);
#endif

  for (i = 0; i <= lenA; i++)
    {
      match[i] = (i <= prefix) ? i
	: (i > first_trailer) ? i + difference
  	  : 0;
    }

#ifdef  DEBUG
  printf("unravel\n");
#endif

  while (k != -1)
    {
      cp = &clist[k];

#ifdef  DEBUG
      if (k < 0 || k >= clength)
	error("Illegal link -> %d", k);

      printf("match[%d] := %d\n", cp->a + prefix, cp->b + prefix);
#endif

      match[cp->a + prefix] = cp->b + prefix;
      k = cp->link;
    }

  return;
  
} /* unravel */


/*
 * Check for hash matches (jackpots) and collect random access indices to
 * the two files.
 *
 * It should be possible to avoid doing most of the ftell's by noticing
 * that we are not doing a context diff and noticing that if a line
 * compares equal to the other file, we will not ever need to know its
 * file position.  FIXME.
 */

long
check(
      char *	fileAname,
      char *	fileBname )
{
  register long  a;  	/* Current line in file A  */
  register long  b;  	/* Current line in file B  */
  long  	 jackpot;

  
  /*
   * The VAX C ftell() returns the address of the CURRENT record, not the
   * next one (as in DECUS C or, effectively, other C's).  Hence, the values
   * are "off by one" in the array.  OFFSET compensates for this.
   */
  
#define OFFSET 0

  b = 1;
  
  rewind(infd[0]);
  rewind(infd[1]);
  
  /*
   * See above; these would be over-written on VMS anyway.
   */
  
  oldseek[0] = ftell(infd[0]);
  newseek[0] = ftell(infd[1]);

  jackpot = 0;
  
#ifdef  DEBUG
  printf("match vector\n");

  for (a = 0; a <= lenA; a++)
    printf("match[%d] = %d\n", a, match[a]);
#endif

  for (a = 1; a <= lenA; a++)
    {
      if (match[a] == 0)
	{
	  /* Unique line in A */

	  oldseek[a+OFFSET] = ftell(infd[0]);

	  getline(infd[0], text);

	  continue;  
  	}

      while (b < match[a])
	{
	  /* Skip over unique lines in B */

	  newseek[b+OFFSET] = ftell(infd[1]);

	  getline(infd[1], textb);

	  b++;
  	}

      /*
       * Compare the two, supposedly matching, lines.
       * Unless we are going to print these lines, don't bother to
       * remember where they are.  We only print matching lines
       * if a context diff is happening, or if a jackpot occurs.
       */

      if (cflag)
	{
	  oldseek[a+OFFSET] = ftell(infd[0]);
	  newseek[b+OFFSET] = ftell(infd[1]);
	}

      getline(infd[0], text);

      getline(infd[1], textb);

      if (!streq(text, textb))
	{
  	  fprintf(stderr,  "Spurious match:\n");
  	  fprintf(stderr, "line %ld in %s, \"%s\"\n",
		  a, fileAname, text);
  	  fprintf(stderr, "line %ld in %s, \"%s\"\n",
		  b, fileBname, textb);
  	  match[a] = 0;
  	  jackpot++;
  	}

      b++;
    }

  for (; b <= lenB; b++)
    {
      newseek[b+OFFSET] = ftell(infd[1]);
      getline(infd[1], textb);
    }
  
  /*
   * The logical converse to the code up above, for NON-VMS systems, to
   * store away an fseek() pointer at the beginning of the file.  For VMS,
   * we need one at EOF...
   */

  return(jackpot);

} /* check */


void
range(
      long  	from,
      long  	to,
      long	w )
/*
 * Print a range
 */
{
  if (cflag)
    {
      if ((from -= cflag) <= 0)
	from = 1;

      if ((to += cflag) > len[w])
	to = len[ w ];
    }
  
  if (to > from)
    {
      printf( "%ld, %ld", from, to );
    }
  else if (to < from)
    {
      printf( "%ld, %ld", to, from );
    }
  else
    {
      printf( "%ld", from );
    }

  return;
  
} /* range */


void
fetch(
      long *		seekvec,
      register long	start,
      register long  	end,
      long  		trueend,
      FILE *		fd,
      char *		pfx )
/*
 * Print the appropriate text
 */
{
  register long		i;
  register long		first;
  register long		last;

  
  if (cflag)
    {
      if ((first = start - cflag) <= 0)
	first = 1;

      if ((last = end + cflag) > trueend)
	last = trueend;
    }
  else
    {
      first = start;
      last  = end;
    }

  /*dbug*/

#ifdef DEBUG
  printf("\nDBUG seek to seekvec[%d] = %ld\n",first,seekvec[first]);
#endif

  if (fseek(fd, seekvec[first], 0) != 0)
    {
      printf( "?Can't read line %ld at %08lx (hex) in file%c\n",
	     start, seekvec[first],
	     (fd == infd[0]) ? 'A' : 'B');
    }
  else
    {
      for (i = first; i <= last; i++)
	{
	  if (fgetss(text, sizeof text, fd) == NULL)
	    {
	      printf("** Unexpected end of file\n");

	      break;
	    }

#ifdef DEBUG
  	  printf("%5d: %s%s\n", i, pfx, text);
#else
  	  fputs((cflag && (i<start || i>end)) ? "  " : pfx, stdout);
  	  fputs(text, stdout);
	  putchar('\n');
#endif
  	}
    }

  return;
  
} /* fetch */


/*
 * Output a change entry: fileA[astart..aend] changed to fileB[bstart..bend]
 */

void
change(
       long  	astart,
       long  	aend,
       long  	bstart,
       long  	bend )
{
  char 		c;


  /*
   * This catches a "dummy" last entry
   */

  if (astart > aend && bstart > bend)
    return;

  foundadiff = 1;	/* ACC remeber we have had a difference */

  c = (astart > aend) ? 'a' : (bstart > bend) ? 'd' : 'c';

  if (cflag)
    fputs("**************\n*** ", stdout);

  if (c == 'a' && !cflag)
    range(astart-1, astart-1, 0L);	/* Addition: just print one odd # */
  else
    range(astart, aend, 0L);		/* Print both, if different */

  if (!cflag)
    {
      putchar(c);

      if (!eflag)
	{
	  if (c == 'd')
	    range(bstart-1, bstart-1, 1L); /* Deletion: just print one odd # */
	  else
	    range(bstart, bend, 1L);	/* Print both, if different */
	}
    }

  putchar('\n');
  
  if (!eflag)
    {
      fetch(oldseek, astart, aend, lenA, infd[0], 
	    cflag ? (c=='d' ? "- " : "! ") : "< ");

      if (cflag)
	{
	  fputs("--- ", stdout);
	  
	  range(bstart, bend, 1L);
	  
	  fputs(" -----\n", stdout);
	}
      else if (astart <= aend && bstart <= bend)
	printf("---\n");
    }

  fetch(newseek, bstart, bend, lenB, infd[1], 
	cflag ? (c=='a' ? "+ " : "! ") : (eflag ? "" : "> "));

  if (eflag && bstart <= bend)
    printf(".\n");

  return;
  
} /* change */


void
output(
       char *	fileAname,
       char *	fileBname )
{
  register long	astart;
  register long	aend = 0;
  long		bstart;
  register long	bend;

  
  rewind( infd[ 0 ] );
  rewind( infd[ 1 ] );
  
  match[ 0 ] = 0;
  
  match[ lenA + 1 ] = lenB + 1;
  
  if (!eflag)
    {
      if (cflag)
	{
	  /*
	   * Should include ctime style dates after the file names, but
	   * this would be non-trivial on OSK.  Perhaps there should be
	   * a special case for stdin.
	   */

	  printf("*** %s\n--- %s\n", fileAname, fileBname);
	}

      /*
       * Normal printout
       */

      for (astart = 1; astart <= lenA; astart = aend + 1)
	{
  	  /*
  	   * New subsequence, skip over matching stuff
  	   */

  	  while (astart <= lenA
		 && match[astart] == (match[astart - 1] + 1))
	    astart++;

  	  /*
  	   * Found a difference, setup range and print it
  	   */

  	  bstart = match[astart - 1] + 1;
  	  aend   = astart - 1;
	  
  	  while (aend < lenA && match[aend + 1] == 0)
	    aend++;

  	  bend = match[aend + 1] - 1;
	  
  	  match[aend] = bend;
	  
  	  change( astart, aend, bstart, bend );
  	}
    }
  else
    {
      /*
       * Edit script output -- differences are output "backwards"
       * for the benefit of a line-oriented editor.
       */

      for (aend = lenA; aend >= 1; aend = astart - 1)
	{
  	  while (aend >= 1
		 && match[aend] == (match[aend + 1] - 1)
		 && match[aend] != 0)
	    aend--;

	  bend   = match[aend + 1] - 1;
  	  astart = aend + 1;

 	  while (astart > 1 && match[astart - 1] == 0)
	    astart--;

  	  bstart          = match[astart - 1] + 1;
	  match[ astart ] = bstart;

	  change( astart, aend, bstart, bend );
  	}
    }

  if (lenA == 0)
    change(1, 0, 1, lenB);

  return;
  
} /* output */


#ifdef  DEBUG
void rdump(pointer, why)
long  	*pointer;
char  	*why;
/*
 * Dump memory block
 */
{
  long  *last;
  long  count;

  last = ((long **)pointer)[-1];
  fprintf(stderr, "dump %s of %06o -> %06o, %d words",
  	  why, pointer, last, last - pointer);
  last = (long *)(((long) last) & ~1);
  for (count = 0; pointer < last; ++count) {
  	if ((count & 07) == 0) {
  	  fprintf(stderr, "\n%06o", pointer);
  	}
  	fprintf(stderr, "\t%06o", *pointer);
  	pointer++;
  }
  fprintf(stderr, "\n");
}
#endif

void
cant(
     char *	filename,
     char *	what,
     int 	fatalflag )
/*
 * Can't open file message
 */
{
  fprintf( stderr, "Can't open %s file \"%s\": ", what, filename );

  perror( "" );

  if (fatalflag)
    {
      exit( fatalflag + 1 );	/* ACC make error > 1 */
    }

  return;
  
} /* cant */

#ifdef  DEBUG
void dump(d_linep, d_len, d_which)
LINE  *d_linep;
{
  register long i;
  
  printf("Dump of file%c, %d elements\n", "AB"[d_which], d_len);
  printf("linep @ %06o\n", d_linep);
  for (i = 0; i <= d_len; i++) {
  	printf("%3d  %6d  %06o\n", i,
  	  	d_linep[i].serial, d_linep[i].hash);
  }
}

void dumpklist(kmax, why)
long  kmax;
char  *why;
/*
 * Dump klist
 */
{
  register long  	i;
  register CANDIDATE  *cp;
  register long  	count;

  printf("\nklist[0..%d] %s, clength = %d\n", kmax, why, clength);
  for (i = 0; i <= kmax; i++) {
  	cp = &clist[klist[i]];
  	printf("%2d %2d", i, klist[i]);
  	if (cp >= &clist[0] && cp < &clist[clength])
  	  printf(" (%2d %2d -> %2d)\n", cp->a, cp->b, cp->link);
  	else if (klist[i] == -1)
  	  printf(" End of chain\n");
  	else  printf(" illegal klist element\n");
  }
  for (i = 0; i <= kmax; i++) {
  	count = -1;
  	for (cp = (CANDIDATE *)klist[i]; cp > &clist[0]; 
		cp = (CANDIDATE *)&cp->link) {
  	  if (++count >= 6) {
  	  	printf("\n    ");
  	  	count = 0;
  	  }
  	  printf(" (%2d: %2d,%2d -> %d)",
  	  	cp-clist, cp->a, cp->b, cp->link);
  	}
  	printf("\n");
  }
  printf("*\n");
}
#endif

#ifdef  TIMING
void ptime(why)
char  	*why;
/*
 * Dump time buffer
 */
{
  long  ttemp;

  ttemp = time(NULL);
  printf("%ld seconds for %s\n",
  	ttemp - sectiontime, why);
  sectiontime = ttemp;
}
#endif

/*
 *  	  s t r e q . c
 */
 
/*)LIBRARY
*/

#ifdef  DOCUMENTATION

title  streq  String Equality Test
index  	String equality test

synopsis
  .s.nf
  streq(a, b);
  char  	*a;
  char  	*b;
  .s.f
Description

  Return TRUE if the strings are equal.

Bugs

#endif

/* #define  EOS  0
#define  FALSE  0
#define  TRUE  1
*/
/*
 *  	  e r r o r . c
 */

/*)LIBRARY
*/

#ifdef  DOCUMENTATION

title  error  Fatal Error Exit
index  	Fatal error exit

synopsis
  .s.nf
  _error()

  error(format, args)
  char  	*format;
  .s.f
documentation

  Fatal error exits.  _error() halts, error() prints something
  on stderr and then halts.

bugs

  THIS DOES NOT WORK ON MANY SYSTEMS DUE TO EXTREMLY NON-PORTABLE CODE.
  Why oh why cannot people learn to use varargs properly?  This code will
  blow up on OSK.  Fortunatly, it is not used often...

#endif

/* #include  <stdio.h> */
  

int
main(
     long 	argc,
     char **	argv )
/*
 * Diff main program
 */
{
  register long  	i;
  register char *	ap;


#ifdef  TIMING
  sectiontime = time(&totaltime);
#endif

  while (argc > 1 && *(ap = argv[1]) == '-' && *++ap != EOS)
    {
      while (*ap != EOS)
	{
  	  switch ((*ap++))
	    {
	    case 'b':
	      bflag++;
	      break;

	    case 'c':
	      if (*ap > '0' && *ap <= '9')
		cflag = (long)(int)(*ap++ - '0');
	      else
		cflag = 3;
	      break;

	    case 'e':
	      eflag++;
	      break;

	    case 'i':
	      iflag++;
	      break;

	    default:
	      fprintf( stderr, "Warning, bad option '%c'\n", ap[ -1 ] );
	      break;
	    }
  	}
      
      argc--;
      argv++;
    }

  if (argc != 3)
    error( "Usage: diff [-options] file1 file2", 0 );
  
  if (cflag && eflag)
    {
      fprintf( stderr, "Warning, -c and -e are incompatible, -c supressed.\n" );
      
      cflag = FALSE;
    }
  
  argv++;
  
  for (i = 0; i <= 1; i++)
    {
      if (argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == EOS)
	{
  	  infd[ i ] = stdin;

	  if ((tempfd = tmpfile()) == NULL)
	    cant("temporary file", "work", 1);
  	}
      else
	{
  	  infd[i] = fopen(argv[i], "r");
	  
	  if (!infd[i])
	    cant(argv[i], "input", 2);	/* Fatal error */
  	}
    }

  if (infd[0] == stdin && infd[1] == stdin)
    error("Can't diff two things both on standard input.",0);

  if (infd[0] == NULL && infd[1] == NULL)
    {
      cant(argv[0], "input", 0);
      cant(argv[1], "input", 1);
    }

  /*
   * Read input, building hash tables.
   */
  
  input( 0L );
  
  input( 1L );
  
  squish();
  
#ifdef  DEBUG
  printf("before sort\n");
  
  for (i = 1; i <= slenA; i++)
    printf("sfileA[%d] = %6d %06o\n",
	   i, sfileA[i].serial, sfileA[i].hash);

  for (i = 1; i <= slenB; i++)
    printf("sfileB[%d] = %6d %06o\n",
	   i, sfileB[i].serial, sfileB[i].hash);
#endif

  sort( sfileA, slenA );

  sort( sfileB, slenB );

#ifdef  TIMING
  ptime("input");
#endif

#ifdef  DEBUG
  printf("after sort\n");

  for (i = 1; i <= slenA; i++)
    printf("sfileA[%d] = %6d %06o\n",
	   i, sfileA[i].serial, sfileB[i].hash);

  for (i = 1; i <= slenB; i++)
    printf("sfileB[%d] = %6d %06o\n",
	   i, sfileB[i].serial, sfileB[i].hash);
#endif

  /*
   * Build equivalence classes.
   */

  member = (long *)fileB;
  
  equiv();
  
  member = (long *)compact((char *)member, (long)((slenB + 2) * sizeof (long)),
			  "squeezing member vector");

  /*
   * Reorder equivalence classes into array Class[]
   */

  Class = (long *)fileA;

  unsort();

  Class = (long *)compact((char *)Class, (long)((slenA + 2) * sizeof (long)),
			 "compacting class vector");

#ifdef  TIMING
  ptime("equiv/unsort");
#endif

  /*
   * Find longest subsequences
   */

  klist = (long *)myalloc((long)((slenA + 2) * sizeof (long)), "klist");
  clist = (CANDIDATE *)myalloc((long)(csize * sizeof (CANDIDATE)), "clist");
  
  i = subseq();
  
  Free((char *)member);
  Free((char *)Class);
  
  match = (long *)myalloc((long)((lenA + 2) * sizeof (long)), "match");
  
  unravel(klist[i]);
  
  Free((char *)clist);
  Free((char *)klist);
  
#ifdef  TIMING
  ptime("subsequence/unravel");
#endif

  /*
   * Check for fortuitous matches and output differences
   */

  oldseek = (long *)myalloc((long)((lenA + 2) * sizeof (* oldseek)), "oldseek");
  newseek = (long *)myalloc((long)((lenB + 2) * sizeof (* newseek)), "newseek");
  
  textb = myalloc((long)(sizeof text), "textbuffer");
  
  if (check(argv[0], argv[1]))
    fprintf(stderr, "Spurious match, output is not optimal\n");
  
#ifdef  TIMING
  ptime("check");
#endif

  output( argv[0], argv[1] );

#ifdef  TIMING
  ptime("output");
  printf("%ld seconds required\n", sectiontime - totaltime);
#endif

  if (tempfd != NULL)
    {
      fclose(tempfd);
    }

  return foundadiff;

} /* main */
