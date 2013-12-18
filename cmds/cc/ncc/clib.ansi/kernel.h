/*
 *      interface to host os.
 *      copyright (c) acorn computers ltd., 1988
 */

#ifndef __size_t
#  define __size_t 1
   typedef unsigned int size_t;   /* from <stddef.h> */
#endif

typedef struct {
   int r[10];            /* only r0 - r9 matter for swi's */
} _kernel_swi_regs;

typedef struct {
   int load, exec;       /* load, exec addresses */
   int start, end;       /* start address/length, end address/attributes */
} _kernel_osfile_block;

typedef struct {
   void * dataptr;       /* memory address of data */
   int nbytes, fileptr;
   int buf_len;          /* these fields for arthur gpbp extensions */
   char * wild_fld;      /* points to wildcarded filename to match */
} _kernel_osgbpb_block;

typedef struct {
   int errnum;           /* error number */
   char errmess[252];    /* error message (zero terminated) */
} _kernel_oserror;

typedef struct stack_chunk {
   unsigned long sc_mark;       /* == 0xf60690ff */
   struct stack_chunk *sc_next, *sc_prev;
   unsigned long sc_size;
   int (*sc_deallocate)();
} _kernel_stack_chunk;

extern _kernel_stack_chunk *_kernel_current_stack_chunk(void);

extern void _kernel_setreturncode(unsigned code);

extern void _kernel_exit(int);

extern void _kernel_exittraphandler(void);

#define _kernel_host_undefined    -1
#define _kernel_bbc_mos1_0         0
#define _kernel_bbc_mos1_2         1
#define _kernel_bbc_acw            2
#define _kernel_bbc_master         3
#define _kernel_bbc_master_et      4
#define _kernel_bbc_master_compact 5
#define _kernel_arthur             6
#define _kernel_springboard        7
#define _kernel_a_unix             8

extern int _kernel_hostos(void);
/*
 *  returns the identity of the host os
 */

extern int _kernel_fpavailable(void);
/*
 * returns 0 if floating point is not available (no emulator nor hardware)
 */

extern _kernel_oserror *_kernel_swi(int no, _kernel_swi_regs *in, _kernel_swi_regs *out);
/*
 *  generic swi interface.  returns null if there was no error.
 *  the swi number may have the x bit set (bit 17) or not; it makes no
 *  difference
 */

extern char *_kernel_command_string(void);
/*
 * returns the address of (maybe a copy of) the string used to run the program
 */

/*
 *  the int value returned by the following functions may have value:
 *    >= 0  if the call succeeds (significance then depends on the function)
 *     -1   if the call fails but causes no os error (eg escape for rdch).  not
 *          all functions are capable of generating this result.  this return
 *          value corresponds to the c flag being set by the swi.
 *     -2   if the call causes an os error (in which case, _kernel_oserror must
 *          be used to find which error it was)
 */

#define _kernel_error (-2)

extern int _kernel_osbyte(int op, int x, int y);
/*
 *  performs an osbyte operation.
 *  if there is no error, the result contains
 *     the return value of r1 (x) in its bottom byte
 *     the return value of r2 (y) in its second byte
 *     1 in the third byte if carry is set on return, otherwise 0
 *     0 in its top byte
 *  (not all of these values will be significant, depending on the
 *   particular osbyte operation).
 */

extern int _kernel_osrdch(void);
/*
 *  returns a character read from the currently selected os input stream
 */

extern int _kernel_oswrch(int ch);
/*
 *  writes a byte to all currently selected os output streams
 *  the return value just indicates success or failure.
 */

extern int _kernel_osbget(unsigned handle);
/*
 *  returns the next byte from the file identified by 'handle'.
 *  (-1 => eof).
 */

extern int _kernel_osbput(int ch, unsigned handle);
/*
 *  writes a byte to the file identified by 'handle'.
 *  the return value just indicates success or failure.
 */

extern int _kernel_osgbpb(int op, unsigned handle, _kernel_osgbpb_block *inout);/*
 *  reads or writes a number of bytes from a filing system.
 *  the return value just indicates success or failure.
 *  note that for some operations, the return value of c is significant,
 *  and for others it isn't.  in all cases, therefore, a return value of -1
 *  is possible, but for some operations it should be ignored.
 *  (to confuse matters further, some brazil filing systems don't set c when
 *   they should, so the best course of action may be to ignore the result
 *   unless it indicates an error).
 */

extern int _kernel_osword(int op, int *data);
/*
 *  performs an osword operation.
 *  the size and format of the block *data depends on the particular osword
 *  being used; it may be updated.
 */

extern int _kernel_osfind(int op, char *name);
/*
 *  opens or closes a file.
 *    open returns a file handle (0 => open failed without error)
 *    close the return value just indicates success or failure
 */

extern int _kernel_osfile(int op, const char *name, _kernel_osfile_block *inout);
/*  performs an osfile operation, with values of r2 - r5 taken from the osfile
 *  block.  the block is updated with the return values of these registers,
 *  and the result is the return value of r0 (or an error indication)
 */

extern int _kernel_osargs(int op, unsigned handle, int arg);
/*
 *  performs an osargs operation.
 *  the result is an error indication, or
 *    the current filing system number (if op = handle = 0)
 *    the value returned in r2 by the osargs operation otherwise
 */

extern int _kernel_oscli(const char *s);
/*
 *  hands the argument string to the os command line interpreter to execute
 *  as a command.  this should not be used to invoke other applications:
 *  _kernel_system exists for that.  even using it to run a replacement
 *  application is somewhat dubious (abort handlers are left as those of the
 *  current application).
 *  the return value just indicates error or no error
 */

extern _kernel_oserror *_kernel_last_oserror(void);
/*
 *  returns a pointer to an error block describing the last os error since
 *  _kernel_last_oserror was last called (or since there program started
 *  if there has been no such call).  if there has been no os error, returns
 *  a null pointer.  note that occurrence of a further error may overwrite the
 *  contents of the block.
 *  if _kernel_swi caused the last os error, the error already returned by that
 *  call gets returned by this too.
 */

extern _kernel_oserror *_kernel_getenv(const char *name, char *buffer, unsigned size);
/*
 *  reads the value of a system variable, placing the value string in the
 *  buffer (of size size).
 *  under arthur, this just gives access to  os_readvarval.
 *  under brazil, it accesses the file  $.environ
 *  (lines of which have the format  varname space value newline).
 */

extern _kernel_oserror *_kernel_setenv(const char *name, const char *value);
/*
 *  updates the value of a system variable to be string valued, with the
 *  given value (value = null deletes the variable)
 *  under brazil, this returns the error "not implemented"
 */

extern int _kernel_system(const char *string, int chain);
/*
 *  hands the argument string to the os command line interpreter to execute.
 *  if chain is 0, the calling application is copied to the top of memory first,
 *    then handlers are installed so that if the command string causes an
 *    application to be invoked, control returns to _kernel_system, which then
 *    copies the calling application back into its proper place - the command
 *    is executed as a sub-program.  of course, since the sub-program executes
 *    in the same address space, there is no protection against errant writes
 *    by it to the code or data of the caller.  and if there is insufficient
 *    space to load the sub-program, the manner of the subsequent death is
 *    unlikely to be pretty.
 *  if chain is 1, all handlers are removed before calling the cli, and if it
 *    returns (the command is built-in) _kernel_system exits.
 *  the return value just indicates error or no error
 */


extern unsigned _kernel_alloc(unsigned minwords, void **block);
/*
 *  tries to allocate a block of sensible size >= minwords.  failing that,
 *  it allocates the largest possible block (may be size zero).
 *  sensible size means at least 2k words.
 *  *block is returned a pointer to the start of the allocated block
 *  (null if 'a block of size zero' has been allocated).
 */

typedef void freeproc(void *);
typedef void * allocproc(unsigned);

extern void _kernel_register_allocs(allocproc *malloc, freeproc *free);
/*
 *  registers procedures to be used by the kernel when it requires to
 *  free or allocate storage.  the allocproc may be called during stack
 *  extension, so may not check for stack overflow (nor may any procedure
 *  called from it), and must guarantee to require no more than 41 words
 *  of stack.
 */

extern int _kernel_escape_seen(void);
/*
 * returns 1 if there has been an escape since the previous call of
 * _kernel_escape_seen (or since program start, if there has been no
 * previous call).  escapes are never ignored with this mechanism,
 * whereas they may be with the language eventproc mechanism since there
 * may be no stack to call it on.
 */

typedef union {
    struct {int s:1, u:16, x: 15; unsigned mhi, mlo; } i;
    int w[3]; } _extended_fp_number;

typedef struct {
   int r4, r5, r6, r7, r8, r9;
   int fp, sp, pc, sl;
   _extended_fp_number f4, f5, f6, f7; } _kernel_unwindblock;

extern int _kernel_unwind(_kernel_unwindblock *inout, char **language);
/*
 *  unwinds the call stack one level.
 *  returns >0 if it succeeds
 *          0 if it fails because it has reached the stack end
 *          <0 if it fails for any other reason (eg stack corrupt)
 *  input values for fp, sl and pc  must be correct.
 *  r4-r9 and f4-f7 are updated if the frame addressed by the input value
 *  of fp contains saved values for the corresponding registers.
 *  fp, sp, sl and pc are always updated
 *  *language is returned a pointer to a string naming the language
 *  corresponding to the returned value of pc.
 */

extern char *_kernel_procname(int pc);
/*
 *  returns a string naming the procedure containing the address pc.
 *  (or 0 if no name for it can be found).
 */

extern char *_kernel_language(int pc);
/*
 *  returns a string naming the language in whose code the address pc lies.
 *  (or 0 if it is in no known language).
 */

/*  divide and remainder functions.
 *  the signed functions round towards zero.
 *
 *  the div functions actually also return the remainder in a2, and use of
 *  this by a code-generator will be more efficient than a call to the rem
 *  function.
 *
 *  language rtss are encouraged to use these functions rather than providing
 *  their own, since considerable effort has been expended to make these fast.
 */

#ifndef __clipper
extern unsigned _kernel_udiv(unsigned divisor, unsigned dividend);
extern unsigned _kernel_urem(unsigned divisor, unsigned dividend);
extern unsigned _kernel_udiv10(unsigned dividend);

extern int _kernel_sdiv(int divisor, int dividend);
extern int _kernel_srem(int divisor, int dividend);
extern int _kernel_sdiv10(int dividend);
#endif

/*
 * Description of a 'Language description block'
 */

typedef enum { NotHandled, Handled } _kernel_HandledOrNot;

typedef struct {
   int regs [16];
} _kernel_registerset;

typedef struct {
   int regs [10];
} _kernel_eventregisters;

typedef void (*PROC) (void);
typedef _kernel_HandledOrNot (*_kernel_trapproc) (int code, _kernel_registerset *regs);
typedef _kernel_HandledOrNot (*_kernel_eventproc) (int code, _kernel_registerset *regs);

typedef struct {
   int size;
   int codestart, codeend;
   char *name;
   PROC (*InitProc)(void);
   PROC FinaliseProc;
   _kernel_trapproc TrapProc;
   _kernel_trapproc UncaughtTrapProc;
   _kernel_eventproc EventProc;
   _kernel_eventproc UnhandledEventProc;
   void (*FastEventProc) (_kernel_eventregisters *);
   int (*UnwindProc) (_kernel_unwindblock *inout, char **language);
   char * (*NameProc) (int pc);
} _kernel_languagedescription;

typedef int _kernel_ccproc(int, int, int);

extern int _kernel_call_client(int a1, int a2, int a3, _kernel_ccproc callee);
/* This is for shared library use only, and is not exported to shared library
 * clients. It is provided to allow library functions which call arbitrary
 * client code (C library signal, exit, _main) to do so correctly if the
 * client uses the old calling standard.
 */

extern int _kernel_client_is_module(void);
/* For shared library use only, not exported to clients.  Returns a
 * non-zero value if the library's client is a module executing in user
 * mode (ie module run code).
 */

#ifndef __clipper
extern int _kernel_processor_mode(void);
#endif

extern void _kernel_irqs_on(void);

extern void _kernel_irqs_off(void);

extern int _kernel_irqs_enabled(void);
/* returns 0 if interrupts are disabled; some non-zero value if enabled. */

extern void *_kernel_RMAalloc(size_t size);

extern void *_kernel_RMAextend(void *p, size_t size);

extern void _kernel_RMAfree(void *p);

