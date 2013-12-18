# makefile for staff contributed public domain code

OPTS = -ansi \
	-pedantic \
	-Wall \
	-Wwrite-strings \
	-O \
	-fstrength-reduce \
	-fcombine-regs \
	-fomit-frame-pointer \
	-finline-functions \
	-funsigned-char


all: tran  ox bsplit

tran: TRAN/ox TRAN/bsplit


# ox is an improved version of 'od'

ox: ox.c
	gcc ox.c -o ox -ltermcap $(OPTS)

TRAN/ox: ox.c
	c ox.c -D_BSD -o TRAN/ox -ltermcap

 
# bsplit is a version of 'split' that works for binary files

bsplit: bsplit.c
	gcc $(OPTS) bsplit.c -o bsplit

TRAN/bsplit: bsplit.c
	c bsplit.c -o TRAN/bsplit

