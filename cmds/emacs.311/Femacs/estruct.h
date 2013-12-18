/*      ESTRUCT:        Structure and preprocesser defined for
                        MicroEMACS 3.10

                        written by Daniel Lawrence
                        based on code by Dave G. Conroy,
                                Steve Wilhite and George Jones
*/

/*      Modifications
        11-Sep-89       Mike Burrow (INMOS)     Added folding.
*/

#ifdef  LATTICE
#undef  LATTICE          /* don't use their definitions...use ours       */
#endif
#ifdef  MSDOS
#undef  MSDOS
#endif
#ifdef  AMIGA
#undef  AMIGA
#endif
#ifdef  EGA
#undef  EGA
#endif
#ifdef  CTRLZ
#undef  CTRLZ
#endif

/*      Program Identification.....

        PROGNAME should always be MicroEMACS for a distribution
        unmodified version. People using MicroEMACS as a shell
        for other products should change this to reflect their
        product. Macros can query this via the $progname variable
*/

#define PROGNAME        "uEmacs"
#define VERSION         "3.10.21"


/*      Machine/OS definitions                  */
/*      [Set one of these!!]                    */

#define AMIGA   0                       /* AmigaDOS                     */
#define ST520   0                       /* ST520, TOS                   */
#define MSDOS   0                       /* MS-DOS                       */
#define OS2     0                       /* Microsoft or IBM OS/2        */
#define V7      0                       /* V7 UNIX or Coherent or BSD4.2*/ 
#define BSD     1                       /* UNIX BSD 4.2 and ULTRIX      */
#define USG     0                       /* UNIX system V                */
#define XENIX   0                       /* IBM-PC SCO XENIX             */
#define SUN     0                       /* SUN v4.0                     */
#define HPUX    0                       /* HPUX HP 9000 minicomputer    */
#define VMS     0                       /* VAX/VMS                      */
#define FINDER  0                       /* Macintosh OS                 */
#define WMCS    0                       /* Wicat's MCS                  */
#define AOSVS   0                       /* Data General AOS/VS          */

/*      Compiler definitions                    */
/*      [Set one of these!!]                    */
#define UNIX    1       /* a random UNIX compiler */
#define MWC     0       /* Mark Williams C */
#define ALCYON  0       /* ALCYON Atari ST compiler */ 
#define LATTICE 0       /* Lattice 2.14 through 3.0 compilers */
#define AZTEC   0       /* Aztec C 3.20e */
#define MSC     0       /* MicroSoft C compile version 3 & 4 & 5.1 */
#define TURBO   0       /* Turbo C/MSDOS */
#define DTL     0       /* DataLight C v3.12 */
#define DGC     0       /* Data General AOS/VS C... */

/*      Machine stack growth direction.                     */
/*  [Set this to 1 if your machine stack grows UP!!!]       */
/*      data general mv/eclipse series stack grows up.      */
/*      dec vax series stack grows down... got it???        */

#define STACK_GROWS_UP  0

/*      Debugging options       */
#define RAMSIZE 0       /* dynamic RAM memory usage tracking */
#define RAMSHOW 0       /* auto dynamic RAM reporting */
#define RAMTRCK 0       /* send debug info to MALLOC.DAT */

/*   Special keyboard/network definitions            */

#define WANGPC  0               /* WangPC - mostly escape sequences     */
#define VT100   0               /* Handle VT100 style keypad.   */
#define XONDATA 0               /* VMS only - set to force /NOTTSYNC/NOHOSTSY */
#define NOVELL  0               /* IBMPC Novell NetWare flag! */

/*      Terminal Output definitions             */
/*      [Set one of these!!]                    */

#define ANSI    0                       /* ANSI escape sequences        */
#define HP150   0                       /* HP150 screen driver          */
#define HP110   0                       /* HP110 screen driver          */
#define VMSVT   0                       /* various VMS terminal entries */
#define VT52    0                       /* VT52 terminal (Zenith).      */
#define RAINBOW 0                       /* Use Rainbow fast video.      */
#define TERMCAP 1                       /* Use TERMCAP                  */
#define IBMPC   0                       /* IBM-PC CGA/MONO/EGA/VGA drvr */
#define OS2NPM  0                       /* OS/2 non-Presentation Mgr.   */
#define DG10    0                       /* Data General system/10       */
#define TIPC    0                       /* TI Profesional PC driver     */
#define Z309    0                       /* Zenith 100 PC family driver  */
#define MAC     0                       /* Macintosh                    */
#define ATARI   0                       /* Atari 520/1040ST screen      */
#define DASHER  0                       /* DG Dasher 2xx/4xx crts       */

/*      Language text options   (pick one)                              */

#define ENGLISH 1
#define FRENCH  0
#define SPANISH 0
#define GERMAN  0
#define DUTCH   0
#define PLATIN  0

/*      Configuration options   */

#define CVMVAS  1       /* arguments to page forward/back in pages      */
#define CLRMSG  0       /* space clears the message line with no insert */
#define CFENCE  1       /* fench matching in CMODE                      */
#define TYPEAH  1       /* type ahead causes update to be skipped       */
#define DEBUGM  1       /* $debug triggers macro debugging              */
#define LOGFLG  0       /* send all executed commands to EMACS.LOG      */
#define VISMAC  0       /* update display during keyboard macros        */
#define CTRLZ   0       /* add a ^Z at end of files under MSDOS only    */
#define NBRACE  1       /* new style brace matching command             */
#define COMPLET 1       /* new completion code (as of 3.10)             */
#define CLEAN   0       /* de-alloc memory on exit                      */
#define CALLED  0       /* is emacs a called subroutine? or stand alone */
#define ADDCR   0       /* ajout d'un CR en fin de chaque ligne (ST520) */
                        /* [= add a CR at the end of each line (ST520)] */
#define BINARY  1       /* using new binary searches?                   */

#define REVSTA  1       /* Status line appears in reverse video         */
#define COLOR   0       /* color commands and windows                   */

#define FILOCK  0       /* file locking under unix BSD 4.2              */
#define ISRCH   1       /* Incremental searches like ITS EMACS          */
#define WORDPRO 1       /* Advanced word processing features            */
#define FLABEL  0       /* function key label code [HP150]              */
#define APROP   1       /* Add code for Apropos command                 */
#define CRYPT   1       /* file encryption enabled?                     */
#define MAGIC   1       /* include regular expression matching?         */
#define AEDIT   1       /* advanced editing options: en/detabbing       */
#define PROC    1       /* named procedures                             */
#define MOUSE   1       /* Include routines for mouse actions           */
#define NOISY   1       /* Use a fancy BELL if it exists                */
#define DIACRIT 0       /* diacritical marks processed?                 */
#define SPEECH  0       /* spoken EMACS, for the sight impared [not ready] */

/*      Character set options           */
/*      [Set one of these!!]            */
#define ASCII   1       /* always using ASCII char sequences for now    */
#define EBCDIC  0       /* later IBM mainfraim versions will use EBCDIC */

/* handle constant and voids properly */

#if     VMS
#define CONST   readonly
#define VOID    void
#define NOSHARE noshare
#else
#if     AOSVS
#define CONST $shared $align(1)     /* fake a  const */
#define VOID
#define NOSHARE $low32k $align(1)   /* attempt to optimize read/write vars. */
#else
#ifdef  __STDC__
#define CONST   const
#define VOID    void
#define NOSHARE
#else
#define CONST
#define VOID
#define NOSHARE
#endif
#endif
#endif

/*      System dependant library redefinitions, structures and includes */

/*      the following define allows me to initialize unions...
        otherwise we make them structures (like the keybinding table)  */

#if     __STDC__        /* if ANSI C compatible */
#define ETYPE   union
#else
#define ETYPE   struct
#endif

#if MSDOS & (TURBO)        /* | MSC)     BG. 10-Oct-89 */
#define NEAR
#define DNEAR
#define PASCAL pascal
#define CDECL cdecl
#else
#define NEAR
#define DNEAR
#define PASCAL
#define CDECL
#endif

#if     TURBO
#include      <dos.h>
#include      <mem.h>
#undef peek
#undef poke
#define       peek(a,b,c,d)   movedata(a,b,FP_SEG(c),FP_OFF(c),d)
#define       poke(a,b,c,d)   movedata(FP_SEG(c),FP_OFF(c),a,b,d)
#endif

#if     LATTICE & MSDOS
/* you may have to remove this one definition with LATTICE version
   3.2 and above                                                  */
#define unsigned
#endif

#if     AZTEC
#undef  putc
#undef  getc
#if     MSDOS
#define getc    a1getc
#define int86   sysint
#define intdos(a, b)    sysint(33, a, b)
#define inp     inportb
#define outp    outportb
#else
#define getc    agetc
#endif
#define putc    aputc

struct XREG {
        unsigned ax,bx,cx,dx,si,di,ds,es;
};

struct HREG {
        char al,ah,bl,bh,cl,ch,dl,dh,d1,d2,e1,e2;
};

union REGS {
        struct XREG x;
        struct HREG h;
};

struct SREGS {
        unsigned cs, ss, ds, es;
};
#endif

#if     MSDOS & DTL
#include        <dos.h>
#endif

#if     MSDOS & MWC
#include        <dos.h>
#define int86(a, b, c)  intcall(b, c, a)
#define intdos(a, b)    intcall(a, b, DOSINT)
#define inp(a)          in(a)
#define outp(a, b)      out(a, b)
#define movmem(a, b, c) memcpy(b, a, c)

struct XREG {
        unsigned int ax,bx,cx,dx,si,di,ds,es,flags;
};

struct HREG {
        char al,ah,bl,bh,cl,ch,dl,dh;
        unsigned int ds,es,flags;
};

union REGS {
        struct XREG x;
        struct HREG h;
};
#endif

#if     MSDOS & MSC
#include        <dos.h>
#include        <memory.h>
#define peek(a,b,c,d)   movedata(a,b,FP_SEG(c),FP_OFF(c),d)
#define poke(a,b,c,d)   movedata(FP_SEG(c),FP_OFF(c),a,b,d)
#define movmem(a, b, c)         memcpy(b, a, c)
#endif

#if     MSDOS & LATTICE
#undef  CPM
#undef  LATTICE
#include        <dos.h>
#undef  CPM
#endif

/* this keeps VMS happy */
#if     VMS
#define getname xgetname
#define unlink(a)       delete(a)
#endif

/* some options for AOS/VS */
#if     AOSVS
#define ORMDNI  1
#endif

/*      define some ability flags */

#if     IBMPC | Z309
#define MEMMAP  1
#else
#define MEMMAP  0
#endif

#if     MSDOS | OS2 | V7 | USG | HPUX | BSD | (ST520 & MWC) | WMCS
#define ENVFUNC 1
#else
#define ENVFUNC 0
#endif

#if     ATARI || MSDOS || OS2
#define DIRSEPSTR       "\\"
#define DIRSEPCHAR      '\\'
#else
#define DIRSEPSTR       "/"
#define DIRSEPCHAR      '/'
#endif

/*      Emacs global flag bit definitions (for gflags)  */

#define GFREAD  1

/*      internal constants      */

#define NBINDS  300                     /* max # of bound keys          */
#if     AOSVS == 0
#define NFILEN  80                      /* # of bytes, file name        */
#else
#define NFILEN  256
#endif
#define NBUFN   32                      /* # of bytes, buffer name      */
#define NLINE   256                     /* # of bytes, input line       */
#define NSTRING 128                     /* # of bytes, string buffers   */
#define NLONGS  1024			/* # of bytes, long strings     */
#define NKBDM   256                     /* # of strokes, keyboard macro */
#define NPAT    128                     /* # of bytes, pattern          */
#define HUGE    1000                    /* Huge number                  */
#define NLOCKS  100                     /* max # of file locks active   */
#define NCOLORS 16                      /* number of supported colors   */
#define KBLOCK  250                     /* sizeof kill buffer chunks    */
#define NBLOCK  16                      /* line block chunk size        */
#define NVSIZE  10                      /* max #chars in a var name     */
#define NMARKS  10                      /* number of marks              */
#define NFOLD   64                      /* # of bytes in fold markers   */ /* BG:  11-Oct-89 */
#define NFOLDS  32                      /* max depth of folding         */ /* MJB: 11-Sep-89 */

#define CTRL    0x0100          /* Control flag, or'ed in               */
#define META    0x0200          /* Meta flag, or'ed in                  */
#define CTLX    0x0400          /* ^X flag, or'ed in                    */
#define SPEC    0x0800          /* special key (function keys)          */
#define MOUS    0x1000          /* alternative input device (mouse)     */
#define SHFT    0x2000          /* shifted (for function keys)          */
#define ALTD    0x4000          /* ALT key...                           */

#define BINDNUL 0               /* not bount to anything                */
#define BINDFNC 1               /* key bound to a function              */
#define BINDBUF 2               /* key bound to a buffer                */

#ifdef  FALSE
#undef  FALSE
#endif
#ifdef  TRUE
#undef  TRUE
#endif

#define FALSE   0                       /* False, no, bad, etc.         */
#define TRUE    1                       /* True, yes, good, etc.        */
#define ABORT   2                       /* Death, ^G, abort, etc.       */
#define FAILED  3                       /* not-quite fatal false return */

#define STOP    0                       /* keyboard macro not in use    */
#define PLAY    1                       /*                playing       */
#define RECORD  2                       /*                recording     */

/*      Competion types         */

#define CMP_BUFFER      0
#define CMP_COMMAND     1
#define CMP_FILENAME    2

/*      Directive definitions   */

#define DIF             0
#define DELSE           1
#define DENDIF          2
#define DGOTO           3
#define DRETURN         4
#define DENDM           5
#define DWHILE          6
#define DENDWHILE       7
#define DBREAK          8
#define DFORCE          9

#define NUMDIRS         10

/*
 * PTBEG, PTEND, FORWARD, and REVERSE are all toggle-able values for
 * the scan routines.
 */
#define PTBEG   0       /* Leave the point at the beginning on search   */
#define PTEND   1       /* Leave the point at the end on search         */
#define FORWARD 0                       /* forward direction            */
#define REVERSE 1                       /* backwards direction          */

#define FIOSUC  0                       /* File I/O, success.           */
#define FIOFNF  1                       /* File I/O, file not found.    */
#define FIOEOF  2                       /* File I/O, end of file.       */
#define FIOERR  3                       /* File I/O, error.             */
#define FIOMEM  4                       /* File I/O, out of memory      */
#define FIOFUN  5                       /* File I/O, eod of file/bad line*/
#define FIODEL  6                       /* Can't delete/rename file     */

#define CFCPCN  0x0001                  /* Last command was C-P, C-N    */
#define CFKILL  0x0002                  /* Last command was a kill      */

#define BELL    0x07                    /* a bell character             */
#define TAB     0x09                    /* a tab character              */

#if     V7 | USG | HPUX | BSD
#define PATHCHR ':'
#else
#if     WMCS
#define PATHCHR ','
#else
#define PATHCHR ';'
#endif
#endif

#define INTWIDTH        sizeof(int) * 3

/*      Macro argument token types                                      */

#define TKNUL   0                       /* end-of-string                */
#define TKARG   1                       /* interactive argument         */
#define TKBUF   2                       /* buffer argument              */
#define TKVAR   3                       /* user variables               */
#define TKENV   4                       /* environment variables        */
#define TKFUN   5                       /* function....                 */
#define TKDIR   6                       /* directive                    */
#define TKLBL   7                       /* line label                   */
#define TKLIT   8                       /* numeric literal              */
#define TKSTR   9                       /* quoted string literal        */
#define TKCMD   10                      /* command name                 */

/*      Internal defined functions                                      */

#define nextab(a)       (a - (a % tabsize)) + tabsize

/* DIFCASE represents the integer difference between upper
   and lower case letters.  It is an xor-able value, which is
   fortunate, since the relative positions of upper to lower
   case letters is the opposite of ascii in ebcdic.
*/

#ifdef  islower
#undef  islower
#endif
#ifdef  isupper
#undef  isupper
#endif

#if     ASCII
#define DIFCASE         0x20
#if     DIACRIT == 0
#define isletter(c)     (('a' <= c && 'z' >= c) || ('A' <= c && 'Z' >= c))
#define islower(c)      (('a' <= c && 'z' >= c))
#define isupper(c)      (('A' <= c && 'Z' >= c))
#endif
#endif

#if     EBCDIC
#define DIFCASE         0x40
#define isletter(c)     (('a' <= c && 'i' >= c) || ('j' <= c && 'r' >= c) || ('s' <= c && 'z' >= c) || ('A' <= c && 'I' >= c) || ('J' <= c && 'R' >= c) || ('S' <= c && 'Z' >= c))
#define islower(c)      (('a' <= c && 'i' >= c) || ('j' <= c && 'r' >= c) || ('s' <= c && 'z' >= c))
#define isupper(c)      (('A' <= c && 'I' >= c) || ('J' <= c && 'R' >= c) || ('S' <= c && 'Z' >= c))
#endif

#if     DIACRIT
#define CHCASE(c)       chcase(c)       /* Toggle extended letter case.*/
#else
#define CHCASE(c)       ((c) ^ DIFCASE) /* Toggle the case of a letter.*/
#endif

/*      Dynamic RAM tracking and reporting redefinitions        */

#if     RAMSIZE
#define malloc  allocate
#define free    release
#endif


/*
 * There is a window structure allocated for every active display window. The
 * windows are kept in a big list, in top to bottom screen order, with the
 * listhead at "wheadp". Each window contains its own values of dot and mark.
 * The flag field contains some bits that are set by commands to guide
 * redisplay. Although this is a bit of a compromise in terms of decoupling,
 * the full blown redisplay is just too expensive to run for every input
 * character.
 */
typedef struct  WINDOW {
        struct  WINDOW *w_wndp;         /* Next window                  */
        struct  BUFFER *w_bufp;         /* Buffer displayed in window   */
        struct  LINE *w_linep;          /* Top line in the window       */
        struct  LINE *w_dotp;           /* Line containing "."          */
        short   w_doto;                 /* Byte offset for "."          */
        struct  LINE *w_markp[NMARKS];  /* Line containing "mark"       */
        short   w_marko[NMARKS];        /* Byte offset for "mark"       */
        char    w_toprow;               /* Origin 0 top row of window   */
        char    w_ntrows;               /* # of rows of text in window  */
        char    w_force;                /* If NZ, forcing row.          */
        char    w_flag;                 /* Flags.                       */
#if     COLOR
        char   w_fcolor;               /* current forground color      */
        char    w_bcolor;               /* current background color     */
#endif
        int     w_fcol;                 /* first column displayed       */
}       WINDOW;

#define WFFORCE 0x01                    /* Window needs forced reframe  */
#define WFMOVE  0x02                    /* Movement from line to line   */
#define WFEDIT  0x04                    /* Editing within a line        */
#define WFHARD  0x08                    /* Better to a full display     */
#define WFMODE  0x10                    /* Update mode line.            */
#define WFCOLR  0x20                    /* Needs a color change         */

/* Define a fold marker lookup table for different
 * file types. MJB: 13-Sep-89.
 */

typedef struct  FM {
        char      *fm_extension;        /* File name extension          */
        char      *fm_startopen;        /* Opening of start-fold marker */
        char      *fm_startclose;       /* Close of start-fold marker   */
        char      *fm_end;              /* end-fold marker              */
        struct FM *fm_next;             /* next fold-mark entry         */
}       FOLDMARKENT;

/* In order to enter nested folds, we require to stack some of the buffer
 * information prior to the entry, and also the number of leading spaces
 * removed to left justify the buffer. MJB: 11-Sep-89
 */
typedef struct  FOLDS {
        struct  LINE *f_topline;  /* Link to top of fold    */   /* MJB: 11-Sep-89 */
        struct  LINE *f_botline;  /* Link to bottom of fold */   /* MJB: 11-Sep-89 */
}       FOLDS;

/*
 * Text is kept in buffers. A buffer header, described below, exists for every
 * buffer in the system. The buffers are kept in a big list, so that commands
 * that search for a buffer by name can find the buffer header. There is a
 * safe store for the dot and mark in the header, but this is only valid if
 * the buffer is not being displayed (that is, if "b_nwnd" is 0). The text for
 * the buffer is kept in a circularly linked list of lines, with a pointer to
 * the header line in "b_linep" Buffers may be "Inactive" which means the files associated with them
 * have not been read in yet. These get read in at "use buffer" time.
 *
 * Folding requirements: A stack of previous fold information, the amount
 *                       of removed indentation and the depth of the 
 *                       current fold. MJB: 11-Sep-89
 */
typedef struct  BUFFER {
        struct  BUFFER *b_bufp;         /* Link to next BUFFER          */
        struct  LINE *b_dotp;           /* Link to "." LINE structure   */
        short   b_doto;                 /* Offset of "." in above LINE  */
        struct  LINE *b_markp[NMARKS];  /* The same as the above two,   */
        short   b_marko[NMARKS];        /* but for the "mark"           */
        int     b_fcol;                 /* first col to display         */
        struct  LINE *b_linep;          /* Link to the header LINE      */
        struct  LINE *b_topline;        /* Link to narrowed top text    */
        struct  LINE *b_botline;        /* Link to narrowed bottom text */
        char    b_active;               /* window activated flag        */
        char    b_nwnd;                 /* Count of windows on buffer   */
        char    b_flag;                 /* Flags                        */
        int     b_mode;                 /* editor mode of this buffer   */
        char    b_fname[NFILEN];        /* File name                    */
        char    b_bname[NBUFN];         /* Buffer name                  */
#if     CRYPT
        char    b_key[NPAT];            /* current encrypted key        */
#endif
        short   b_nfolds;               /* Number of entered folds      */ /* MJB: 11-Sep-89 */
        struct  FOLDS b_folds[NFOLDS];  /* Entered fold information     */ /* MJB: 11-Sep-89 */
}       BUFFER;

#define BFINVS  0x01                    /* Internal invisable buffer    */
#define BFCHG   0x02                    /* Changed since last write     */
#define BFTRUNC 0x04                    /* buffer was truncated when read */
#define BFNAROW 0x08                    /* buffer has been narrowed     */

/*      mode flags      */
#define NUMMODES        9              /* # of defined modes           */

#define MDWRAP  0x0001                  /* word wrap                    */
#define MDCMOD  0x0002                  /* C indentation and fence match*/
#define MDSPELL 0x0004                  /* spell error parsing          */
#define MDEXACT 0x0008                  /* Exact matching for searches  */
#define MDVIEW  0x0010                  /* read-only buffer             */
#define MDOVER  0x0020                  /* overwrite mode               */
#define MDMAGIC 0x0040                  /* regular expresions in search */
#define MDCRYPT 0x0080                  /* encrytion mode active        */
#define MDASAVE 0x0100                  /* auto-save mode               */

/*
 * The starting position of a region, and the size of the region in
 * characters, is kept in a region structure.  Used by the region commands.
 */
typedef struct  {
        struct  LINE *r_linep;          /* Origin LINE address.         */
        short   r_offset;               /* Origin LINE offset.          */
        long    r_size;                 /* Length in characters.        */
}       REGION;

/*
 * All text is kept in circularly linked lists of "LINE" structures. These
 * begin at the header line (which is the blank line beyond the end of the
 * buffer). This line is pointed to by the "BUFFER". Each line contains a the
 * number of bytes in the line (the "used" size), the size of the text array,
 * and the text. The end of line is not stored as a byte; it's implied.
 *
 * Folding requirements: A line type field and a pointer around 
 *                       any folded text. MJB: 11-Sep-89
 */

#define LNORMAL         0       /* Normal line            */ /* MJB: 11-Sep-89 */
#define LSOFOLD         1       /* Start of fold          */ /* MJB: 11-Sep-89 */
#define LEOFOLD         2       /* End of fold            */ /* MJB: 11-Sep-89 */
#define LSOEFOLD        3       /* Start of expanded fold */ /* MJB: 11-Sep-89 */
#define LEOEFOLD        4       /* End of expanded fold   */ /* MJB: 11-Sep-89 */

typedef struct  LINE {
        struct  LINE *l_fp;     /* Link to the next line        */
        struct  LINE *l_bp;     /* Link to the previous line    */
        short   l_size;         /* Allocated size               */
        short   l_used;         /* Used size                    */
        short   l_type;         /* Type of line                 */ /* MJB: 11-Sep-89 */
        struct  LINE *l_foldp;  /* Pointer around a fold        */ /* MJB: 11-Sep-89 */
	short	l_lmargin;	/* Left margin of line 		*/ /* MJB: 16-Oct-89 */
	short   l_omargin;      /* Outer margin of entered fold */ /* MJB: 14-Dec-89 */
        char    l_text[1];      /* A bunch of characters        */
}       LINE;

#define lgetc(lp, n)    ((lp)->l_text[(n)]&0xFF)
#define lputc(lp, n, c) ((lp)->l_text[(n)]=(c))
#define llength(lp)     ((lp)->l_used)

/* In order to restore folds, we need to be able to store the start
 * of a fold, until we find its corresponding end. This is just a 
 * circular list of lines. MJB: 15-Sep-89.
 */
typedef struct  FOLDLINELIST {
        struct  FOLDLINELIST    *fll_fp;        /* link to next         */
        struct  FOLDLINELIST    *fll_bp;        /* link to previous     */
        struct  LINE            *fll_line;      /* the line             */
}       FOLDLINELIST;
 
/*
 * The editor communicates with the display using a high level interface. A
 * "TERM" structure holds useful variables, and indirect pointers to routines
 * that do useful operations. The low level get and put routines are here too.
 * This lets a terminal, in addition to having non standard commands, have
 * funny get and put character code too. The calls might get changed to
 * "termp->t_field" style in the future, to make it possible to run more than
 * one terminal type.
 */
typedef struct  {
        short   t_mrow;                  /* max number of rows allowable */
        short   t_nrow;                 /* current number of rows used  */
        short   t_mcol;                 /* max Number of columns.       */
        short   t_ncol;                 /* current Number of columns.   */
        short   t_margin;               /* min margin for extended lines*/
        short   t_scrsiz;               /* size of scroll region "      */
        int     t_pause;                /* # times thru update to pause */
        int (PASCAL NEAR *t_open)();    /* Open terminal at the start.  */
        int (PASCAL NEAR *t_close)();   /* Close terminal at end.       */
        int (PASCAL NEAR *t_kopen)();   /* Open keyboard                */
        int (PASCAL NEAR *t_kclose)();  /* close keyboard               */
        int (PASCAL NEAR *t_getchar)(); /* Get character from keyboard. */
        int (PASCAL NEAR *t_putchar)(); /* Put character to display.    */
        int (PASCAL NEAR *t_flush)();   /* Flush output buffers.        */
        int (PASCAL NEAR *t_move)();    /* Move the cursor, origin 0.   */
        int (PASCAL NEAR *t_eeol)();    /* Erase to end of line.        */
        int (PASCAL NEAR *t_eeop)();    /* Erase to end of page.        */
        int (PASCAL NEAR *t_beep)();    /* Beep.                        */
        int (PASCAL NEAR *t_rev)();     /* set reverse video state      */
        int (PASCAL NEAR *t_rez)();     /* change screen resolution     */
#if     COLOR
        int (PASCAL NEAR *t_setfor)(); /* set forground color          */
        int (PASCAL NEAR *t_setback)(); /* set background color         */
#endif
}       TERM;

/*      TEMPORARY macros for terminal I/O  (to be placed in a machine
                                            dependant place later) */

#define TTopen          (*term.t_open)
#define TTclose         (*term.t_close)
#define TTkopen         (*term.t_kopen)
#define TTkclose        (*term.t_kclose)
#define TTgetc          (*term.t_getchar)
#define TTputc          (*term.t_putchar)
#define TTflush         (*term.t_flush)
#define TTmove          (*term.t_move)
#define TTeeol          (*term.t_eeol)
#define TTeeop          (*term.t_eeop)
#define TTbeep          (*term.t_beep)
#define TTrev           (*term.t_rev)
#define TTrez           (*term.t_rez)
#if     COLOR
#define TTforg          (*term.t_setfor)
#define TTbacg          (*term.t_setback)
#endif

/*      Structure for the table of current key bindings         */

ETYPE EPOINTER {
        int (PASCAL NEAR *fp)();        /* C routine to invoke */
        BUFFER *buf;                    /* buffer to execute */
};

typedef struct  {
        short k_code;           /* Key code                     */
        short k_type;           /* binding type (C function or EMACS buffer) */
        ETYPE EPOINTER k_ptr;   /* ptr to thing to execute */
}       KEYTAB;

/*      structure for the name binding table            */

typedef struct {
        char *n_name;                   /* name of function key */
        int (PASCAL NEAR *n_func)();    /* function name is bound to */
}       NBIND;

/*      The editor holds deleted text chunks in the KILL buffer. The
        kill buffer is logically a stream of ascii characters, however
        due to its unpredicatable size, it gets implemented as a linked
        list of chunks. (The d_ prefix is for "deleted" text, as k_
        was taken up by the keycode structure)
*/

typedef struct KILL {
        struct KILL *d_next;   /* link to next chunk, NULL if last */
        char d_chunk[KBLOCK];   /* deleted text */
} KILL;

/*      When emacs' command interpetor needs to get a variable's name,
        rather than it's value, it is passed back as a VDESC variable
        description structure. The v_num field is a index into the
        appropriate variable table.
*/

typedef struct VDESC {
        int v_type;    /* type of variable */
        int v_num;      /* ordinal pointer to variable in list */
} VDESC;

/*      The !WHILE directive in the execution language needs to
        stack references to pending whiles. These are stored linked
        to each currently open procedure via a linked list of
        the following structure
*/

typedef struct WHBLOCK {
        LINE *w_begin;          /* ptr to !while statement */
        LINE *w_end;            /* ptr to the !endwhile statement*/
        int w_type;             /* block type */
        struct WHBLOCK *w_next; /* next while */
} WHBLOCK;

#define BTWHILE         1
#define BTBREAK         2

/*
 * Incremental search defines.
 */
#if     ISRCH

#define CMDBUFLEN       256     /* Length of our command buffer */

#define IS_ABORT        0x07    /* Abort the isearch */
#define IS_BACKSP       0x08    /* Delete previous char */
#define IS_TAB          0x09    /* Tab character (allowed search char) */
#define IS_NEWLINE      0x0D    /* New line from keyboard (Carriage return) */
#define IS_QUOTE        0x11    /* Quote next character */
#define IS_REVERSE      0x12    /* Search backward */
#define IS_FORWARD      0x13    /* Search forward */
#define IS_VMSQUOTE     0x16    /* VMS quote character */
#define IS_VMSFORW      0x18    /* Search forward for VMS */
#define IS_QUIT         0x1B    /* Exit the search */
#define IS_RUBOUT       0x7F    /* Delete previous character */

/* IS_QUIT is no longer used, the variable metac is used instead */

#endif

/* HICHAR - 1 is the largest character we will deal with.
 * HIBYTE represents the number of bytes in the bitmap.
 */
#define HICHAR          256
#define HIBYTE          HICHAR >> 3

#if     MAGIC
/*
 * Defines for the metacharacters in the regular expression
 * search routines.
 */
#define MCNIL           0       /* Like the '\0' for strings.*/
#define LITCHAR         1       /* Literal character, or string.*/
#define ANY             2
#define CCL             3
#define NCCL            4
#define BOL             5
#define EOL             6
#define DITTO           7
#define CLOSURE         256    /* An or-able value.*/
#define MASKCL          CLOSURE - 1

#define MC_ANY          '.'     /* 'Any' character (except newline).*/
#define MC_CCL          '['     /* Character class.*/
#define MC_NCCL         '^'     /* Negate character class.*/
#define MC_RCCL         '-'     /* Range in character class.*/
#define MC_ECCL         ']'     /* End of character class.*/
#define MC_BOL          '^'     /* Beginning of line.*/
#define MC_EOL          '$'     /* End of line.*/
#define MC_CLOSURE      '*'     /* Closure - does not extend past newline.*/
#define MC_DITTO        '&'     /* Use matched string in replacement.*/
#define MC_ESC          '\\'    /* Escape - suppress meta-meaning.*/

#define BIT(n)          (1 << (n))      /* An integer with one bit set.*/

/* Typedefs that define the bitmap type for searching (BITMAP),
 * the meta-character structure for MAGIC mode searching (MC),
 * and the meta-character structure for MAGIC mode replacment (RMC).
 */
typedef char    *BITMAP;

typedef struct {
        short int      mc_type;
        union {
               int     lchar;
                BITMAP  cclmap;
        } u;
} MC;

typedef struct {
        short int      mc_type;
        char    *rstr;
} RMC;
#endif

/* keep all these symbols the same length */
#define FOLDSYMBOL      "...  "         /* MJB: 13-Sep-89 */
#define BEGINFOLD       "{{{  "         /* MJB: 28-Sep-89 */
#define ENDFOLD         "}}}  "         /* MJB: 28-Sep-89 */


/*
        This is the message which should be added to any "About MicroEMACS"
        boxes on any of the machines with window managers.


        ------------------------------------------
        |                                        |
        |        MicroEMACS v3.xx                |
        |               for the ............     |
        |                                        |
        |    Text Editor and Corrector           |
        |                                        |
        |    written by Daniel M. Lawrence       |
        |    [based on code by Dave Conroy]      |
        |                                        |
        |    Send inquiries and donations to:    |
        |    617 New York St                     |
        |    Lafayette, IN 47901                 |
        |                                        |
        ------------------------------------------
*/

