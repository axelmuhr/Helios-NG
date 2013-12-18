	.text
___codeseg:
	.align	4
	.globl	___mcount
#
# This version for a PCRef16 address in the code
#
___mcount:
	noop
	noop
	savew12
	loadw	12(sp),r12	# Get return address
	loadw	(r12),r13	# Old count address
	addq	$8,r12		# Real return address
	loadw	(r13),r14	# The count
	storw	r12,12(sp)	# fiddle return address
	addq	$1,r14		# Increment
	storw	r14,(r13)	# Put back
	restw12                 # Recover registers
	ret	sp		# and return
