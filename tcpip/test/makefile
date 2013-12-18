
CFLAGS = -D_BSD

all: echod client

echod : tcp_echo.o
	c -o $@ $^ -lbsd

client : tcp_clie.o
	c -o $@ $^ -lbsd


