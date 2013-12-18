default:
ifeq ($(HPROC),ARM)
	@ cd .. ; unset MAKEFLAGS ; $(MAKE) ccarm
else
  ifeq ($(HPROC),C40)
	@ cd .. ; unset MAKEFLAGS ; $(MAKE) ccc40
  else
    ifeq ($(HPROC),M68K)
	@ cd .. ; unset MAKEFLAGS ; $(MAKE) ccm68k
    endif
  endif
endif

all:
	@ cd .. ; unset MAKEFLAGS ; $(MAKE) all

clean:
	@ cd .. ; unset MAKEFLAGS ; $(MAKE) clean-RS6000

install:
	@ cd .. ; unset MAKEFLAGS ; $(MAKE) install-RS6000

GHF=-s -b
AH=/hsrc/include/
PH=../mip/
LH=../cfe/

ifeq ($(HPROC),ARM)
  MH=../arm/
else
  ifeq ($(HPROC),C40)
    MH=../c40/
  else
    ifeq ($(HPROC),M68K)
      MH=../68k/
    endif
  endif
endif

ANSIHDRS = \
$(AH)assert.h $(AH)ctype.h  $(AH)errno.h  $(AH)float.h  $(AH)limits.h \
$(AH)locale.h $(AH)math.h   $(AH)setjmp.h $(AH)signal.h $(AH)stdarg.h \
$(AH)stddef.h $(AH)stdio.h  $(AH)stdlib.h $(AH)string.h $(AH)time.h

HDRFILES = \
assert.h ctype.h  errno.h  float.h limits.h locale.h math.h setjmp.h \
signal.h stdarg.h stddef.h stdio.h stdlib.h string.h time.h

headers.c:	$(ANSIHDRS) ${PH}miperrs.h ${MH}mcerrs.h ${LH}feerrs.h genhdrs
		genhdrs ${GHF} -o headers.c \
			-e errors.h -q ${PH}miperrs.h -q ${LH}feerrs.h \
			-q ${MH}mcerrs.h -I $(AH) $(HDRFILES)

errors.h:	headers.c always-null-target
	mv errors.h ${MH}


always-null-target:
	

