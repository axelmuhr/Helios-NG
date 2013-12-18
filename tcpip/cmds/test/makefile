LIB	= /helios/lib
MACRO	= /helios/ampp

BSDL	= /helios/lib/bsd.lib
HELIOSL	= /helios/lib/helios.lib
CLIB	= /helios/lib/c.lib

LINK	= $(HELIOSL) $(BSDL) $(CLIB)

C0	= /helios/lib/c0.o
S0	= /helios/lib/s0.o

CFLAGS  = -d_BSD

X	= /ram/x

default : rmerror client echod

rmerror :
	-rm -f errors

client : client.o
		asm -v -s4000 -h8000 -o $@ $(C0) $^ -l$(LINK)

client.o : tcp_client.c 
		cc $(CFLAGS) $< -s $(X) >> & errors && \
		asm -p $(X) -o $@ && \
		rm $(X)

echod : echod.o
		asm -v -s4000 -h8000 -o $@ $(C0) $^ -l$(LINK)

echod.o : tcp_echo.c 
		cc $(CFLAGS) $< -s $(X) >> & errors && \
		asm -p $(X) -o $@ && \
		rm $(X)


