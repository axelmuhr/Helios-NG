
PROGRAM		= hepc_driver

DRIVER		= ../installation/Driver.o
SOURCE		= $(PROGRAM).c 
OBJECT		= $(PROGRAM).o 

INCLUDE		= hepcio.h hepcdefs.h
LIBRARIES	= 

DEBUG_FLAG	=
CFLAGS		= 

LOPTS		= 
GREP		= /bin/egrep


all: $(DRIVER)

$(DRIVER):	$(OBJECT)
	cp $(OBJECT) $(DRIVER)
	cp hepcio.h ../installation

$(OBJECT):	$(SOURCE) $(INCLUDE)
	$(CC) $(CFLAGS) -c $(SOURCE) 

clean:
	rm -f $(OBJECT) core
	-sccs clean

lint:	
	lint $(LOPTS) $(SOURCE)

