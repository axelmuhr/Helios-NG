INSDIR = ../..
ALL = config
all:	$(ALL)

config:
	$(CC) $(CFLAGS) $(LDFLAGS) config.c -o config

install:	$(ALL)
	chmod 755 $(ALL)
	mv $(ALL) $(INSDIR)/bin

clean:
	rm -f *.o
clobber: clean
	rm -f $(ALL)
