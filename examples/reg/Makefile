.SUFFIXES: .brg

COMPILER=reg
UTIL=../lib/libutil.a

CC=gcc
CFLAGS=-g -I../lib -DYYDEBUG
LEX=flex
LFLAGS=-l
YACC=byacc
YFLAGS=-dv
BURG=pburg
BFLAGS=-T
LDLIBS=-L ../lib -lutil

$(COMPILER): scan.o gram.o code.o $(UTIL)
	$(LINK.c) -o $@ $^
$(UTIL):
	make -C ../lib
.brg.c:
	$(BURG) $(BFLAGS) $< $@
code.c: gram.c
scan.c: gram.c
clean:
	rm -f y.tab.h y.output *.asm *.o scan.c gram.c code.c
