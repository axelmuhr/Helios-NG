CC=c -wA -T4 -D_BSD
CCOPTS=-DTYPE=int -DPTRS -DMEMZERO -DRANDINIT=409667 -UBRAINDAMAGED -DZEROFILLED

# These options would be best to compile ``large'' yabba and unyabba
# on a Sun or other generic 32-bit UNIX machine:
# CCOPTS=-O2 -DTYPE=int -DPTRS -DBZERO -DZEROFILLED -DRANDINIT="getpid()"
# On a System V machine, -DMEMZERO should be used instead of -DBZERO.
# If you want more speed, add -DMOD=131072.
# For large compressions, try -DNODEMAX=500000 -DMOD=1048576 -DNODENUM=65533.
# On a small machine, these options might be more appropriate:
# CCOPTS=-O2 -DTYPE=short -DOPTCANT5
# To see what the options mean and find out what other options are
# available, keep reading.

# You should already have run the sysconf script (at least on any UNIX box)
# to test for system features. sysconf may have modified the CCOPTS line
# on line 2 of this Makefile.

# To check your configuration decisions before compiling, type
# % make checkconf
# % ./checkconf
# or something equivalent on non-UNIX systems, and read through
# checkconf's output. You can give checkconf any -D or -U options to
# see what effect changes will have. On a UNIX machine, you can use
# the ``try'' shell script to test yabba and unyabba and see how they
# measure up against compress and uncompress in speed and effectiveness.

# TYPE and PTRS have the biggest effect on how yabba and unyabba operate.
#
# TYPE is the type that yabba and unyabba will use for indexing. It
# should almost certainly be either short or int. If PTRS is defined,
# yabba (and unyabba and whap, but not unwhap) will use pointers rather
# than integers wherever possible. On almost all machines yabba runs
# faster with -DPTRS. More precisely:
#
# -UPTRS -DTYPE=short: Use shorts. This is ``small'' yabba and unyabba.
# -UPTRS -DTYPE=int: Use ints everywhere. On large machines this is
# usually quite a bit faster than the small version, but also uses
# twice as much memory.
# -DPTRS -DTYPE=short: small unwhap, but use pointers in yabba
# wherever possible. This typically doubles yabba's memory use.
# -DPTRS -DTYPE=int: Use ints everywhere, with pointers wherever
# possible. This is ``large'' yabba and unyabba. On large machines it is
# the fastest configuration, but uses twice as much memory as ``small.''
#
# If you want to generate very large-block compressions (see below)
# on a small machine where int is 16 bits, you may want to try compiling
# with -DPTRS -DTYPE=long.

# NODEMAX and NODENUM control the compression block size.
#
# Any adaptive dictionary compressor has a block size. It can keep
# track of this much input, or this much output, or this many strings
# at once, and compress by looking for patterns in the input data.
# But eventually there's too much to keep track of, and the compressor
# has to stop adapting to the patterns.
#
# In the LZW-based UNIX ``compress'' program, for instance, the block
# size is stated as the number of bits that the coder can use to
# identify strings in its dictionary. Once it runs out of identifying
# numbers, it can't adapt any further.
#
# In this compressor, the natural block size is the size of input.
# NODENUM sets the number of bytes of input that yabba can adapt to at
# once. Past that it will stick to the current dictionary as long as the
# patterns seem useful, then clear the strings and start adapting all
# over again.
#
# The user can set NODENUM dynamically with -m. NODEMAX (minimum 512,
# default 65533---NODEMAX + 2 must fit into TYPE) sets the maximum
# possible value; several fixed arrays in yabba and unyabba are
# dimensioned with size NODEMAX. NODENUM defaults to NODEMAX.
#
# NODENUM and NODEMAX also control the size of input expected by unyabba.
# unyabba *must* be given the same -m parameter as yabba was given for
# compression. That means that if you compress on one machine where
# yabba has NODENUM set to 60000, you won't be able to decompress on
# another machine where unyabba has NODEMAX set to 20000. -m has the
# same effect on portability that -b does for compress (though -m is
# expressed in somewhat more tractable units).

# MOD (default 65536) is the size of a hash table kept by yabba. It must
# be a power of two. It should be on a similar order of magnitude as
# NODEMAX---if it is much too small, the hash table will be too crowded,
# and if it is much too big, the table will waste memory.

# BITBUFSIZE only affects yabba (at the moment). It is the size of two
# output buffers kept by the bit-packing coroutines. It defaults to 1000.

# HASHTYPE (default TYPE) is the type used for storing hash values in
# yabba. It has little effect on whap memory use, so on large machines
# you may want to set -DHASHTYPE=int even for small whap. However, it
# does affect memory use in yabba and unyabba, so -DHASHTYPE=short may
# be useful. In any case, MOD - 1 must fit into unsigned HASHTYPE.

# RESETNUM (default 8192) and RESETFUZZ (default 30, can be negative)
# control how yabba decides when to clear its dictionary. After NODENUM or
# the -m limit is reached, yabba checks every RESETNUM input characters
# whether compression is still worth it. RESETFUZZ has some vaguely
# defined effect on the meaning of ``worth it'': the higher RESETFUZZ
# is, the longer yabba holds out before clearing the old patterns. The user
# can change RESETFUZZ and RESETNUM dynamically, so don't worry about them
# too much.

# Under -r, yabba needs some reasonably random value from the environment.
# If you define an integer RANDINIT, yabba will initialize its generator
# based on that value. Otherwise it has to stick to a default sequence,
# which does not add as much security. (If you can't think up a good
# definition for RANDINIT, open up your phone book and pick a number.)

# Many operating systems have brain-damaged putc(): if you put a
# (perfectly valid) 255 byte, putc() will return EOF, indicating an error.
# This is the case under Ultrix 3.1 and Convex UNIX 8.0, for instance.
# You MUST define -DBRAINDAMAGED if sysconf tells you to; otherwise yabba
# and unyabba will crash mysteriously. Also complain to your vendor.

# If your compiler blows up on yw.c, you may want to set -DOPTCANT5.
# This will change certain heavily unrolled loops to lightly unrolled.
# If that still doesn't help, try -DOPTCANT2; then -DOPTCANT1.

# If the internal representation of the NULL pointer on your machine is
# 0, you can make yabba run slightly faster with -DBZERO or -DMEMZERO.
# (checkconf will tell you if this is true.) If you have bzero(), use
# -DBZERO. If you have ANSI memset(), use -DMEMZERO. If you have
# neither, you're out of luck. MEMZERO is ignored under -DBZERO.

# If you have a machine with particularly fast memory access (perhaps
# certain microcomputers), you might try defining -DHASHPTRS. This will
# use some more memory but may run faster. I have not seen any machines
# where -DHASHPTRS helps.

# Finally, you should set -DZEROFILLED if your operating system
# guarantees that static arrays are initialized to zeros. Don't set it
# under -DPTRS if NULL pointers don't have internal representation 0.
# ZEROFILLED just makes yabba start up a bit faster. (checkconf warns you
# if it notices a non-zero-filled array, but its test isn't guaranteed.)
# UNIX systems generally guarantee -DZEROFILLED.

# All of the above comments apply equally to whap as to yabba except
# where otherwise noted.

default: all

AP: whap unwhap

all: yabba unyabba

checkconf: checkconf.c Makefile
	$(CC) $(CCOPTS) -o checkconf checkconf.c

yabba: yabba.o bitout.o percent.o ytexts.o Makefile getopt.o
	$(CC) $(CCOPTS) -o yabba yabba.o bitout.o percent.o ytexts.o getopt.o

unyabba: unyabba.o percent.o ytexts.o Makefile getopt.o
	$(CC) $(CCOPTS) -o unyabba unyabba.o percent.o ytexts.o getopt.o

whap: whap.o bitout.o percent.o wtexts.o Makefile
	@echo 'Warning! If you use AP coding except for instruction and amusement,'
	@echo 'you may be infringing on a patent.'
	@echo 'If you understand this, press return:'
	@read foo;
	$(CC) $(CCOPTS) -DWHAP -o whap whap.o bitout.o percent.o wtexts.o

unwhap: unwhap.o percent.o wtexts.o Makefile
	$(CC) $(CCOPTS) -DWHAP -o unwhap unwhap.o percent.o wtexts.o

bitout.o: bitout.c bitout.h Makefile
	$(CC) $(CCOPTS) -c bitout.c

percent.o: percent.c percent.h Makefile
	$(CC) $(CCOPTS) -c percent.c

ytexts.o: texts.c texts.h Makefile
	$(CC) $(CCOPTS) -c texts.c
	mv texts.o ytexts.o

wtexts.o: texts.c texts.h Makefile
	$(CC) $(CCOPTS) -DWHAP -c texts.c
	mv texts.o wtexts.o

yabba.o: yw.c bitout.h percent.h texts.h huptrie.h Makefile
	$(CC) $(CCOPTS) -c yw.c
	mv yw.o yabba.o

whap.o: yw.c bitout.h percent.h texts.h huptrie.h Makefile
	$(CC) $(CCOPTS) -DWHAP -c yw.c
	mv yw.o whap.o

unyabba.o: unyabba.c bitout.h percent.h texts.h huptrie.h Makefile
	$(CC) $(CCOPTS) -c unyabba.c

unwhap.o: unwhap.c percent.h texts.h Makefile
	$(CC) $(CCOPTS) -DWHAP -c unwhap.c

install:
	@echo 'Run INSTALL in a root shell.'
	exit 1

clean:
	@echo 'Why do you want to make clean, anyway?'
	@echo 'If you changed the Makefile, it knows it should remake.'
	@echo 'If you want to save space, do make shar and then remove everything.'
	rm -f unwhap.o unyabba.o whap.o yabba.o yw.o texts.o bitout.o percent.o wtexts.o ytexts.o checkconf whap unwhap yabba unyabba

shar:
	shar `cat FILES` > yw.shar
	chmod 400 yw.shar
