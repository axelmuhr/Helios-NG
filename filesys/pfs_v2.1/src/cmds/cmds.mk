C	=	c
PP	=	pp

PPINC	=	-i /ram/include
PPDEF	=	-d__HELIOS=1 -d__TRAN=1 -d__HELIOSTRAN=1
PPOPT	=	-v

# -i /j/pfs/fs,/j/util,/j/pfs/cmds,/helios/include

CMDS	=	access \
		chmod \
		deb \
		editvol \
		format \
		fsync \
		load \
		makefs \
		matrix \
		mksuper \
		refine \
		sync \
		termvol \
		unload

%	:	%.c
	$(PP) $(PPOPT) $(PPDEF) $(PPINC) $*.c -o /ram/c/$*.c
	$(C) /ram/c/$*.c -Lfault -o $*
	
all	:	$(CMDS)
