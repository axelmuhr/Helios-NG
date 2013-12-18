
PROGRAM		= nidio_driver

DRIVER		= ../installation/Driver.o
SOURCE		= $(PROGRAM).c 
OBJECT		= $(PROGRAM).o 

INCLUDE		= nidio.h nidiodefs.h
LIBRARIES	= 

DEBUG_FLAG	=
CFLAGS		= 

LOPTS		= 
GREP		= /bin/egrep


all: $(DRIVER)

$(DRIVER):	$(OBJECT)
	cp $(OBJECT) $(DRIVER)
	cp nidio.h ../installation

$(OBJECT):	$(SOURCE) $(INCLUDE)
	$(CC) $(CFLAGS) -c $(SOURCE) 

clean:
	rm -f $(OBJECT) core
	-sccs clean

lint:	
	lint $(LOPTS) $(SOURCE)

