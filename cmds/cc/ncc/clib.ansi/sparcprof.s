	.text
___codeseg:
	.globl	___mcount
!
!
___mcount:
	ld	[%o7+8],%o0	! Address of count word
	ld	[%o0],%o1	! Old count
	add	%o1, 1, %o1	! Increment
	jmpl	%o7+16, %g0	! Return two word late
	st	%o1,[%o0]	! Put back

