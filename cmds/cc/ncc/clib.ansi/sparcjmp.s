!! !	.seg	"data"
!! !	.asciz	"%Z%%M% %I% %E% Copyr 1987 Sun Micro"
!! 	.seg	"text"
!! 
!! #include <sun4/asm_linkage.h>
!! #include <sun4/trap.h>
!! #include <syscall.h>
!! 
!! SC_ONSTACK	= (0*4)	! offsets in sigcontext structure
!! SC_MASK		= (1*4)
!! SC_SP		= (2*4)
!! SC_PC		= (3*4)
!! SC_NPC		= (4*4)
!! SC_PSR		= (5*4)
!! SC_G1		= (6*4)
!! SC_O0		= (7*4)
!! SC_WBCNT	= (8*4)
!! 
!! SS_SP		= (0*4)	! offset in sigstack structure
!! SS_ONSTACK	= (1*4)
!! SIGSTACKSIZE	= (2*4)
!! 
!! #ifndef S5EMUL
!! /*
!!  * setjmp(buf_ptr)
!!  * buf_ptr points to a nine word array (jmp_buf)
!!  * which is a sigcontext structure.
!!  */
!! 	ENTRY(setjmp)
!! 	b	setjmpsav
!! 	save	%sp, -SA(MINFRAME + SIGSTACKSIZE), %sp
!! #endif
!! 
!! /*
!!  * sigsetjmp(buf_ptr)
!!  * buf_ptr points to a ten word array (sigjmp_buf)
!!  * which is a sigcontext structure.
!!  */
!! 	ENTRY(sigsetjmp)
!! 	save	%sp, -SA(MINFRAME + SIGSTACKSIZE), %sp
!! 	st	%i1, [%i0]		! store "savemask"
!! 	tst	%i1			! should we save signal mask?
!! 	be	setjmpnosav		! if "savemask" 0, don't save
!! 	inc	4, %i0			! point to jmp_buf after savemask
!! 					! (delay slot)
!! 
!! setjmpsav:
!! 	clr	%o0
!! 	mov	SYS_sigblock,%g1
!! 	t	0			! sigblock(0)
!! 	st	%o0, [%i0 + SC_MASK]	! save old sigmask
!! 
!! setjmpnosav:
!! 	ld	[%sp], %g0		! paranoia, to ensure stack is resident
!! 	add	%sp, MINFRAME, %o1	! ptr to tmp sigstack struct
!! 	clr	%o0			! don't set sigstack information
!! 	mov	SYS_sigstack,%g1
!! 	t	0			! sigstack(0, sigstackptr)
!! 	ld	[%sp + MINFRAME + SS_ONSTACK], %g1 ! get onstack flag
!! 	st	%fp, [%i0 + SC_SP]	! interlock slot, my fp == his sp
!! 	st	%g1, [%i0 + SC_ONSTACK]
!! 	add	%i7, 8, %g1		! compute return pc
!! 	st	%g1, [%i0 + SC_PC]	! save pc
!! 	inc	4, %g1			! npc = pc + 4
!! 	st	%g1, [%i0 + SC_NPC]
!! 	clr	[%i0 + SC_PSR]		! psr (icc), g1 = 0 (paranoid)
!! 	clr	[%i0 + SC_G1]		!   o0 filled in by longjmp
!! 	clr	[%i0 + SC_WBCNT]	! no saved windows
!! 	ret
!! 	restore %g0, 0, %o0		! return (0)
!! 
!! #ifndef S5EMUL
!! /*
!!  * longjmp(buf_ptr, val)
!!  * buf_ptr points to a sigcontext which has been initialized by setjmp.
!!  * val is the value we wish to return to the setjmp caller
!!  *
!!  * We use sigcleanup (syscall 139) to atomically set the stack, the onstack
!!  * flag, and the mask. In addition we also flush the register file to the
!!  * stack and return.
!!  */
!! 	ENTRY(longjmp)
!! 	b	longjmpcmn
!! 	mov	1, %g1		! always restore signal mask (delay slot)
!! #endif
!! 
!! /*
!!  * siglongjmp(buf_ptr, val)
!!  * buf_ptr points to a sigjmp_buf which has been initialized by sigsetjmp.
!!  * val is the value we wish to return to the setjmp caller
!!  *
!!  * We use sigcleanup (syscall 139) to atomically set the stack, the onstack
!!  * flag, and the mask. In addition we also flush the register file to the
!!  * stack and return.
!!  */
!! 	ENTRY(siglongjmp)
!! 	ld	[%o0], %g1		! "savemask" flag
!! 	inc	4, %o0			! point to jmp_buf after savemask
!! 
!! longjmpcmn:
!! 	tst	%o1			! is return value 0?
!! 	bz,a	1f			! no - leave it alone
!! 	mov	1, %o1			! yes - set it to one
!! 1:
!! 	tst	%g1			! restore signal mask?
!! 	bnz,a	1f			! yes
!! 	mov	139, %g1		! sigcleanup (delay slot)
!! 
!! 	t	ST_FLUSH_WINDOWS	! flush all reg windows to the stack.
!! 	ld	[%o0 + SC_SP], %fp	! build new stack frame
!! 	sub	%fp, WINDOWSIZE, %sp	! establish new save area (paranoid)
!! 	ld	[%o0 + SC_PC], %g1	! get new return pc
!! 	sub	%g1, 8, %o7		! normalize return (for adb)
!! 	retl
!! 	restore	%o1, 0, %o0		! return (val)
!! 
!! 1:
!! 	st	%o1, [%o0 + SC_O0]	! setup return value
!! 	t	0
!! 	unimp	0			! just in case it returns
!! 
!! 
!! 

	.globl	_setjmp
	.globl	_longjmp
SIGBLOCK   = 109
SIGMASK    = 112
SIGCLEANUP = 139

_setjmp:
	save	%sp, -0x68, %sp
	clr	%o0		! Get current signal mask
	mov	SIGBLOCK, %g1	! Syscall sigblock
	ta	0
	st	%o0, [%i0 + 4]	! and store it.
	ld	[%sp], %g0	! No idea why this is done; other that memory
	add	%sp, 0x60, %o1	! 
	clr	%o0		! No sigstack information wanted
	mov	SIGMASK, %g1	! Syscall sigstack
	ta	0
	ld	[%sp + 96], %g1	! ONSTACK flag
	st	%fp, [%i0 + 8]	! Save stack pointer
	st	%g1, [%i0]	! ONSTACK flag
	add	%i7, 8, %g1	! the return address
	st	%g1, [%i0 + 12]	! Program counter
	add	%g1, 4, %g1	
	st	%g1, [%i0 + 16]	! Save next PC
	st	%g0, [%i0 + 20]	! Zero these words
	st	%g0, [%i0 + 24]
	st	%g0, [%i0 + 32]
	ret
	restore	
!
! 0x0:  onstack flag
! 0x4:  signal mask
! 0x8:  %fp (was %sp)
! 0xc:  return address (call adress + 8)
! 0x10: return address + 4 (call adress + 12)
! 0x14: zero
! 0x18: zero
! 0x1c: unset
! 0x20: zero
!
_longjmp:
	orcc	%g0, %o1, %g0
	be,a	L4
	mov	1, %o1		! Change 0 to 1
L4:
	mov	SIGCLEANUP, %g1	! The unused 139!
	st	%o1, [%o0 + 28]	! Fill o0
	ta	0		! Trap 139 is not in the lists !!
	unimp	0		! Should not get here
