TCP	=	tcp

HELIOS	=	/helios
TMP	=	/ram
SYSINC	=	$(HELIOS)/include
TINC	=	$(TMP)/include

TINCS	=	$(TINC)/asm.h \
		$(TINC)/assert.h \
		$(TINC)/attrib.h \
		$(TINC)/codes.h \
		$(TINC)/config.h \
		$(TINC)/ctype.h \
		$(TINC)/device.h \
		$(TINC)/dirent.h \
		$(TINC)/environ.h \
		$(TINC)/errno.h \
		$(TINC)/event.h \
		$(TINC)/fault.h \
		$(TINC)/float.h \
		$(TINC)/gsp.h \
		$(TINC)/helios.h \
		$(TINC)/ioevents.h \
		$(TINC)/lb.h \
		$(TINC)/limits.h \
		$(TINC)/link.h \
		$(TINC)/locale.h \
		$(TINC)/math.h \
		$(TINC)/memory.h \
		$(TINC)/message.h \
		$(TINC)/module.h \
		$(TINC)/nonansi.h \
		$(TINC)/posix.h \
		$(TINC)/process.h \
		$(TINC)/protect.h \
		$(TINC)/pwd.h \
		$(TINC)/queue.h \
		$(TINC)/root.h \
		$(TINC)/sem.h \
		$(TINC)/servlib.h \
		$(TINC)/session.h \
		$(TINC)/setjmp.h \
		$(TINC)/signal.h \
		$(TINC)/stdarg.h \
		$(TINC)/stddef.h \
		$(TINC)/stdio.h \
		$(TINC)/stdlib.h \
		$(TINC)/string.h \
		$(TINC)/syslib.h \
		$(TINC)/task.h \
		$(TINC)/termios.h \
		$(TINC)/time.h \
		$(TINC)/trace.h \
		$(TINC)/unistd.h \
		$(TINC)/utime.h

$(TINC)/%.h	:	$(SYSINC)/%.h
	$(TCP) $< $@

all	: $(TINCS)
