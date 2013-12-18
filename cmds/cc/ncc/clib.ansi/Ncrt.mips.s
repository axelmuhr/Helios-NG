 # This is a disassembly of crt0.o until I understand it.
	.text
	.globl	_start
	.globl	_environ
	.globl	__init
	.ent	_start, 3
_start:
	la	$gp, _gp	# 0x10008000 perhaps
	lw      $a0, 0($sp)
	addiu   $a1, $sp, 0x4
	move    $a2, $a1
L1:
	lw      $a3, 0($a2)
	addiu   $a2, $a2, 0x4
	bne     $a3, $0, L1
	nop
	la	$at, _environ
	sw      $a2, ($at)
	addiu   $sp, $sp, 0xffffffc8
	addiu   $at, $0, 0xfffffff8
	and     $sp, $sp, $at
	sw      $0, 52($sp)
	jal     __init			# Go do the thing
	nop
	sw      $v0, 48($sp)
	move    $a0, $v0
	jal     exit			# If main returned then exit
	nop
	lw      $a0, 48($sp)		# and if exit returns just crash
	addiu   $v0, $0, 0x1
	syscall
	.end
