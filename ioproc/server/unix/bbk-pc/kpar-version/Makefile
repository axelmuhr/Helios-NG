# %W%	%E%

PROGRAM		= bbk_driver

DRIVER		= ../../installation/BBK/Driver.o
SOURCE		= $(PROGRAM).c 
OBJECT		= $(PROGRAM).o 

INCLUDE		= bbkio.h bbkdefs.h
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

