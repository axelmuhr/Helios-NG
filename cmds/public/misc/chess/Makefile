# This file contains the Makefile for CHESS.
#   Copyright (C) 1986 Free Software Foundation, Inc.
#
# This file is part of CHESS.
#
# CHESS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY.  No author or distributor
# accepts responsibility to anyone for the consequences of using it
# or for whether it serves any particular purpose or works at all,
# unless he says so in writing.  Refer to the CHESS General Public
# License for full details.
#
# Everyone is granted permission to copy, modify and redistribute
# CHESS, but only under the conditions described in the
# CHESS General Public License.   A copy of this license is
# supposed to have been given to you along with CHESS so you
# can know your rights and responsibilities.  It should be in a
# file named COPYING.  Among other things, the copyright notice
# and this notice must be preserved on all copies.
#
#
# Flags for compiling GNU Chess
#    DISPLAY   - allow use of curses based display
#    CHESSTOOL - allow use of SUN-window front-end
#    MSDOS     - allow compiling for a MS-DOS environment
#CFLAGS = -DCHESSTOOL -O # for Suns
CFLAGS = -DDISPLAY -O -lcurses -ltermcap # for curses

gnuchess: gnuchess.c
	cc -o gnuchess gnuchess.c $(CFLAGS)

# src disted in two parts because it's so big
gnuchess.c: src-catted
src-catted: gnuchess.c1 gnuchess.c2
	cat gnuchess.c1 gnuchess.c2 >gnuchess.c
	touch src-catted
# rm gnuchess.c1 gnuchess.c2
