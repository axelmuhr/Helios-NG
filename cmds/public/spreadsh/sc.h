/*	SC	A Table Calculator
 *		Common definitions
 *
 *		original by James Gosling, September 1982
 *		modified by Mark Weiser and Bruce Israel,
 *			University of Maryland
 *		R. Bond  12/86
 *		More mods by Alan Silverstein, 3-4/88, see list of changes.
 *		$Revision: 1.5 $
 *
 */



#define MAXROWS 200
#define MAXCOLS 40
#define RESCOL 4  /* columns reserved for row numbers */
#define RESROW 3  /* rows reserved for prompt, error, and column numbers */
#define DEFWIDTH 10 /* Default column width and precision */
#define DEFPREC   2
#define error move(1,0), clrtoeol(), (void) printw

struct ent_ptr {
    int vf;
    struct ent *vp;
};

struct range_s {
	struct ent_ptr left, right;
};

/*
 * If you want to save room, make row and col below into unsigned
 * chars and make sure MAXROWS and MAXCOLS above are both less
 * than 256.  (128 if your compiler doesn't support unsigned char).
 *
 * Some not too obvious things about the flags:
 *    is_valid means there is a valid number in v.
 *    label set means it points to a valid constant string.
 *    is_strexpr set means expr yields a string expression.
 *    If is_strexpr is not set, and expr points to an expression tree, the
 *        expression yields a numeric expression.
 *    So, either v or label can be set to a constant. 
 *        Either (but not both at the same time) can be set from an expression.
 */

#define VALID_CELL(p, r, c) ((p = tbl[r][c]) != 0 && ((p->flags & is_valid) || p->label))

struct ent {
    double v;
    char *label;
    struct enode *expr;
    short flags;
    short row, col;
    struct ent *next;
};

struct range {
    struct ent_ptr r_left, r_right;
    char *r_name;
    struct range *r_next, *r_prev;
    int r_is_range;
};

#define FIX_ROW 1
#define FIX_COL 2

struct enode {
    int op;
    union {
	double k;
	struct ent_ptr v;
	struct range_s r;
	char *s;
	struct {
	    struct enode *left, *right;
	} o;
    } e;
};

/* op values */
#define O_VAR 'v'
#define O_CONST 'k'
#define O_SCONST '$'
#define REDUCE 0200	/* Or'ed into OP if operand is a range */

#define OP_BASE 256
#define ACOS OP_BASE + 0
#define ASIN OP_BASE + 1
#define ATAN OP_BASE + 2
#define CEIL OP_BASE + 3
#define COS OP_BASE + 4
#define EXP OP_BASE + 5 
#define FABS OP_BASE + 6 
#define FLOOR OP_BASE + 7
#define HYPOT OP_BASE + 8
#define LOG OP_BASE + 9
#define LOG10 OP_BASE + 10
#define POW OP_BASE + 11
#define SIN OP_BASE + 12
#define SQRT OP_BASE + 13
#define TAN OP_BASE + 14
#define DTR OP_BASE + 15
#define RTD OP_BASE + 16
#define MIN OP_BASE + 17
#define MAX OP_BASE + 18
#define RND OP_BASE + 19
#define HOUR OP_BASE + 20
#define MINUTE OP_BASE + 21
#define SECOND OP_BASE + 22
#define MONTH OP_BASE + 23
#define DAY OP_BASE + 24
#define YEAR OP_BASE + 25
#define NOW OP_BASE + 26
#define DATE OP_BASE + 27
#define FMT OP_BASE + 28
#define SUBSTR OP_BASE + 29
#define STON OP_BASE + 30
#define EQS OP_BASE + 31
#define EXT OP_BASE + 32
#define ELIST OP_BASE + 33	/* List of expressions */
#define LMAX  OP_BASE + 34
#define LMIN  OP_BASE + 35
#define NVAL OP_BASE + 36
#define SVAL OP_BASE + 37
#define PV OP_BASE + 38
#define FV OP_BASE + 39
#define PMT OP_BASE + 40
#define STINDEX OP_BASE + 41
#define LOOKUP OP_BASE + 42
#define ATAN2 OP_BASE + 43
#define INDEX OP_BASE + 44

/* flag values */
#define is_valid     0001
#define is_changed   0002
#define is_strexpr   0004
#define is_leftflush 0010
#define is_deleted   0020

#define ctl(c) (c&037)
#define ESC 033
#ifdef __HELIOS
#define CSI 155
#endif
#define DEL 0177

#define BYCOLS 1
#define BYROWS 2
#define BYGRAPH 4		/* Future */

#define	TBL	1		/* tblprint style output for 'tbl' */
#define	LATEX	2		/* tblprint style output for 'LaTeX' */
#define	TEX	3		/* tblprint style output for 'TeX' */

/* Types for etype() */

#define NUM	1
#define STR	2

extern struct ent *tbl[MAXROWS][MAXCOLS];

extern char curfile[];
extern int strow, stcol;
extern int currow, curcol;
extern int savedrow, savedcol;
extern int FullUpdate;
extern int maxrow, maxcol;
extern int fwidth[MAXCOLS];
extern int precision[MAXCOLS];
extern char col_hidden[MAXCOLS];
extern char row_hidden[MAXROWS];
extern char line[1000];
extern int linelim;
extern int changed;
extern struct ent *to_fix;
extern int showsc, showsr;
extern struct enode *new_type( int, struct enode *, struct enode * );
extern struct enode *new_const( int, double );
extern struct enode *new_var( int, struct ent_ptr );
extern struct enode *new_str( char * );
extern struct enode *new_range( int, struct range_s );
extern struct ent *lookat( int, int);
extern struct enode *copye( struct enode *, int, int );
extern char *coltoa(int);
extern FILE *openout( char *, int * );
extern struct range *find_range( char *, int, struct ent *, struct ent * );
extern char *v_name( int, int );
extern char *r_name( int, int, int, int );
extern double eval( struct enode * );
extern char *seval( struct enode * );
extern int modflg;
extern int Crypt;
extern char *mdir;
extern char *xmalloc(unsigned);
extern int xfree(char *);
extern char *strtof( register char *p, double *res );
extern int atocol (char	*strng, int len );
extern void showstring (char *string,int leftflush, int hasvalue, int row, int col, int *nextcolp, int mxcol, int *fieldlenp, int r, int c );
extern void editexp( int row, int col );
extern void clearent( struct ent * );
extern void help( void );
extern void edits( int row, int col );
extern void editv( int row, int col );
extern void initkbd( void );
extern void kbd_again( void );
extern void resetkbd( void );
extern int nmgetch( void );
extern int writefile (char *fname,int r0, int c0, int rn, int cn );
extern int are_ranges( void );
extern char * r_name( int r1, int c1, int r2, int c2 );
extern char * v_name( int row, int col );
extern void list_range( FILE * f );
extern void write_range( FILE *f );
extern void sync_ranges( void );
extern void colshow_op( void );
extern void rowshow_op( void );
extern void sync_refs( void );
extern void forwrow( int arg );
extern void backrow( int arg );
extern void forwcol( int arg );
extern void backcol( int arg );
extern void erasedb( void );
extern void duprow( void );
extern void dupcol( void );
extern void insertrow( int arg );
extern void deleterow( int arg );
extern void insertcol( int arg );
extern void deletecol( int arg );
extern void rowvalueize( int arg );
extern void colvalueize( int arg );
extern void pullcells( int to_insert );
extern void hiderow( int arg );
extern void hidecol( int arg );
extern void closeout( FILE * f, int pid );
extern int get_rcqual ( int ch );
extern void doend( int rowinc, int colinc );
extern void readfile( char * fname, int eraseflg );
extern void copyent( register struct ent *n, register struct ent *p, int dr, int dc );
extern void EvalAll( void );
extern int yyparse( void );
extern void flush_saved( void );
extern void erase_area( int sr, int sc, int er, int ec );
extern int RealEvalAll ( void );
extern int etype( struct enode * e );
extern void openrow (int rs );
extern void closerow (register r );
extern void opencol (int cs );
extern void closecol (int cs );
extern void valueize_area(int sr, int sc, int er, int ec );
extern void free_ent(register struct ent *p);
extern void deraw( void );
extern void goraw( void );
extern int cwritefile( char * fname, int r0, int c0, int rn, int cn );
extern void creadfile( char * save, int  eraseflg );
extern void efree (register struct enode *e);
extern void syncref(register struct enode *e );
extern void clean_range( void );
extern int modcheck( char * endstr );
extern void del_range( struct ent * left, struct ent * right );
extern void add_range( char * name, struct ent_ptr left, struct ent_ptr right, int is_range );
extern void write_fd( FILE * f, int r0, int c0, int rn, int cn );
extern void showcol( int c1, int c2 );
extern void showrow( int r1, int r2 );
extern void hide_row( int arg );
extern void hide_col( int arg );
extern void slet ( struct ent *v, struct enode *se, int flushdir );
extern void let ( struct ent *v, struct enode *e );
extern void copy( struct ent *dv1, struct ent *dv2, struct ent *v1, struct ent *v2 );
extern void eraser( struct ent *v1, struct ent *v2 );
extern void moveto( int row, int col );
extern void num_search( double n );
extern void str_search( char * s );
extern void fill ( struct ent *v1, struct ent *v2, double start,double inc );
extern void go_last( void );
extern void setauto( int i );
extern void setorder( int i );
extern void setiterations(int i);
extern void printfile( char *fname, int r0, int c0, int rn, int cn );
extern void tblprintfile ( char * fname, int r0, int c0, int rn, int cn );
extern void doformat( int c1, int c2, int w, int p );
extern void yyerror( char * err );
extern int  yylex( void );

extern double prescale;
extern int extfunc;
extern int propagation;
extern int calc_order;
extern int autocalc;
extern int numeric;
extern int showcell;
extern int showtop;
extern int loading;
extern int tbl_style;
extern char *progname;


#if defined BSD42 || defined SYSIII

#ifndef cbreak
#define	cbreak		crmode
#define	nocbreak	nocrmode
#endif

#endif

#ifdef __HELIOS
#define	SIGBUS	SIGSEGV
#ifndef fork
#define	fork	vfork
#endif

#endif

