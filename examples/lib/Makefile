LIB=util
CFLAGS=-g
CC=gcc

all:: lib$(LIB).a

lib$(LIB).a: node.o tabid.o main.o
	ar crl $@ $?

clean::
	rm -f lib$(LIB).a *.o a.out core
