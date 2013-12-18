
	.file	"ctx.s"

	.toc
#T.ctx.c:	.tc	ctx.c[tc],ctx.c[rw]
	.globl	ctx[ds]
	.csect	ctx[ds]
#	.long	.getcontext[PR]
	.long	.makecontext[PR]
	.long	.swapcontext[PR]

	.long	TOC[tc0]
	.long	0

# ----------------------------------------------------------------------
# Structures...
#

# TOC function entry
	.set	toc_fn,		0	# function entry point
	.set	toc_toc,	4	# TOC pointer


# Context structure, keep in step with colib.c
	.set	ctx_pc,		0	# PC
	.set	ctx_sp,		4	# SP
	.set	ctx_toc,	8	# TOC
	.set	ctx_cr,		12	# CR
	.set	ctx_regs,	16	# General purpose regs

	.set	ctx_freg,	13	# first register to save in ctx
					# All regs from here to R31 will be
					# save to/restored from ctx_regs

# Stack Frame
	.set	sf_size,	24
	.set	sf_sp,		0	# back-chain stack pointer
	.set	sf_cr,		4	# CR save area
	.set	sf_lr,		8	# Link Register save area
	.set	sf_sj,		12	# Used by set/longjmp
	.set	sf_res,		16	# reserved
	.set	sf_TOC,		20	# TOC pointer for inter module calls

# ----------------------------------------------------------------------
# getcontext(context *ctx)
#
# Save current context - not used at present
#
# Argument:
# 		R3 = context ptr
#
# Returns:	R3 = 0 (== OK)
#

#	.toc
#T.getcontext:	.tc	.getcontext[tc],ctx[ds]

#	.globl	.getcontext[PR]
#	.csect	.getcontext[PR]

	
#	stm	ctx_freg,ctx_regs(3)	# store saveable registers
#	mflr	0			# get link register value
#	st	0,ctx_pc(3)		# store in context
#	st	1,ctx_sp(3)		# save SP in context
#	st	2,ctx_toc(3)		# save TOC
#	mfcr	0			# get condition register
#	st	0,ctx_cr(3)		# save in context
#	lil	3,1			# return 1
#	br
#
#	.tbtag	0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0

# ----------------------------------------------------------------------
# makecontext(context *ctx, void (*fn)(), char *stack)
#
# Make a context from which a function may be called
#
# Arguments:
#		R3 = context ptr
#		R4 = fn = TOC entry of [fn,toc]
#		R5 = sp
#
# Returns:	R3 = 0 (== OK)
#

	.toc
T.makecontext:	.tc	.makecontext[tc],ctx[ds]

	.globl	.makecontext[PR]
	.csect	.makecontext[PR]

	l	0,toc_fn(4)		# get function entry point
	st	0,ctx_pc(3)		# save function in new context
	l	0,toc_toc(4)		# get TOC pointer
	st	0,ctx_toc(3)		# save in new context
	si	5,5,sf_size		# make space on stack for link area
	st	5,ctx_sp(3)		# save SP to context
	mfcr	0			# get condition register
	st	0,ctx_cr(3)		# save in context
	stm	ctx_freg,ctx_regs(3)	# store saveable registers

	st	5,sf_sp(5)		# set backchain to SP
	st	4,sf_lr(5)		# set link reg to fn
	lil	0,0			# zero CR
	st	0,sf_cr(5)		

	lil	3,0			# return zero
	br

	.tbtag	0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0

# ----------------------------------------------------------------------
# swapcontext(context *from, context *to)
#
# Swap from one context to the next
#
# Arguments:
#		R3 = context to save to
#		R4 = context to load
#
# Returns:
#		R3 = 0 (== OK)

	.toc
T.swapcontext:	.tc	.swapcontext[tc],ctx[ds]

	.globl	.swapcontext[PR]
	.csect	.swapcontext[PR]

	mflr	0			# get return link
	st	0,ctx_pc(3)		# save in context
	st	1,ctx_sp(3)		# save current SP
	st	2,ctx_toc(3)		# save TOC ptr
	mfcr	0			# get condition register
	st	0,ctx_cr(3)		# save in context
	stm	ctx_freg,ctx_regs(3)	# save saveable regs

	l	0,ctx_pc(4)		# get pc value
	mtlr	0			# put in LR
	l	1,ctx_sp(4)		# load SP - now on new stack
	l	2,ctx_toc(4)		# load new TOC ptr
	l	0,ctx_cr(4)		# load up CR
	mtcr	0			# set CR
	lm	ctx_freg,ctx_regs(4)	# load saveable registers

	lil	3,0			# return 0
	br				# and exit into new context

	.tbtag	0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0
