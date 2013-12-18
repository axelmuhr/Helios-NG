REM Simple brute force command file for building gawk under msdos
REM
REM *** This has only been using MSC 5.1 ***
REM
REM Written by Arnold Robbins, May 1991
REM Based on earlier makefile for dos
REM
REM Copyright (C) 1986, 1988, 1989, 1991 the Free Software Foundation, Inc.
REM 
REM This file is part of GAWK, the GNU implementation of the
REM AWK Progamming Language.
REM 
REM GAWK is free software; you can redistribute it and/or modify
REM it under the terms of the GNU General Public License as published by
REM the Free Software Foundation; either version 1, or (at your option)
REM any later version.
REM 
REM GAWK is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM GNU General Public License for more details.
REM 
REM You should have received a copy of the GNU General Public License
REM along with GAWK; see the file COPYING.  If not, write to
REM the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
REM
REM	debug flags:	DEBUG=#-DDEBUG #-DFUNC_TRACE -DMEMDEBUG
REM			DEBUGGER= #-Zi
REM
cl -c -AL -Oalt array.c
cl -c -AL -Oalt awktab.c
cl -c -AL -Oalt builtin.c
cl -c -AL -Oalt dfa.c
cl -c -AL -Oalt eval.c
cl -c -AL -Oalt field.c
cl -c -AL -Oalt io.c
cl -c -AL -Oalt iop.c
cl -c -AL -Oalt main.c
cl -c -AL -Oalt missing.c
cl -c -AL -Oalt msg.c
cl -c -AL -Oalt node.c
cl -c -AL -Oalt popen.c
cl -c -AL -Oalt re.c
cl -c -AL -Oalt version.c
REM
REM this kludge necessary because MSC 5.1 compiler bombs with -Oail (where
REM -Ox == "-Oailt -Gs")
REM
REM You can ignore the warnings you will get
cl -c -AL -Ot regex.c
REM
REM	I'm not sure what this is for.  It was commented out
REM 	LINKFLAGS= /CO /NOE /NOI /st:0x1800
REM
link @names.lnk,gawk.exe /E /FAR /PAC /NOE /NOI /st:0x1800;
