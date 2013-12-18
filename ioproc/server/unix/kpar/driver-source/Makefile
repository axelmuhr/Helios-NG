# %W%	%E%
################################################################################
#	Copyright (C) 1989, 1990 K-par Systems Ltd.  All rights reserved
#
# Program/Library:	Interactive 386/ix imb Device Driver
#
# Purpose: 		Makefile for B008 Driver
#
# Author:		Chris Farey - 15-Oct-1990
#
################################################################################

PROGRAM		= imb_driver

DRIVER		= ../installation/Driver.o
SOURCE		= $(PROGRAM).c 
OBJECT		= $(PROGRAM).o 

INCLUDE		= imbio.h imbdefs.h
LIBRARIES	= 

DEBUG_FLAG	=
CFLAGS		= 

LOPTS		= 
GREP		= /bin/egrep


all: $(DRIVER)

$(DRIVER):	$(OBJECT)
	cp $(OBJECT) $(DRIVER)

$(OBJECT):	$(SOURCE) $(INCLUDE)
	$(CC) $(CFLAGS) -c $(SOURCE) 

clean:
	rm -f $(OBJECT) core
	-sccs clean

lint:	
	lint $(LOPTS) $(SOURCE)

