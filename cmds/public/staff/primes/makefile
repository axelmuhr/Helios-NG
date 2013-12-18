#
# Makefile for the genprimes program
#
# Defines:
#
# FAST = 0	- no acceleration
# FAST = 1	- uses AccelerateCode() to put executable in fast RAM
# FAST = 2	- uses Accelerate() to put execution stack in fast RAM
# FAST = 3	- uses both of the above methods
# TIME		- times how long the prime generation process takes
# PRINT		- displays the results of the calculations
#

genprimes: gen.o primes.o
	c gen.o primes.o -o genprimes

gen.o: gen.c
	c -c -Fd -DFAST=3 -DTIME gen.c
#	c -c     -DFAST=0 -DTIME gen.c

primes.o: primes.c
	c -c -Fd primes.c	
#	c -c     primes.c	

clean:
	rm -f gen.o primes.o genprimes emacs.bak genprimes.*


genprimes.snake: primes.c gen.c
	cc -Aa -DFAST=0 -DTIME -D_INCLUDE_POSIX_SOURCE gen.c primes.c -o genprimes.snake
