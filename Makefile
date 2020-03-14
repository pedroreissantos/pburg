# $Id: makefile,v 2.2 2012/05/23 19:15:04 prs Exp $
.SUFFIXES: .brg .1 .ps .pdf
BURG=pburg
VER=2.4
CC=gcc
YACC=byacc
O=.o
E=
PFLAGS=-T
CFLAGS=-g # -DNOJAVA
LDFLAGS=
YFLAGS=-d
LFLAGS=-l
LEX=flex
OBJS=$(BURG)$O gram$O scan$O
DIR=/usr/local/bin

.brg.c:
	./$(BURG)$E $(PFLAGS) $< $@

$(BURG)$E:	$(OBJS);	$(CC) -o $@ $(LDFLAGS) $(OBJS)

$(OBJS):	$(BURG).h

test:		$(BURG)$E sample4.o sample5.o sample6.o
		$(CC) -o sample4$E sample4.o; ./sample4$E
		$(CC) -o sample5$E sample5.o; ./sample5$E
		$(CC) -o sample6$E sample6.o; ./sample6$E

install:: $(BURG)$E
	sudo cp $< $(DIR)

.ps.pdf:
	ps2pdf $< $@

.1.ps:
	groff -man -Tps $< > $@

clean::
		rm -f *$O *~ scan.c gram.c y.tab.h core sample*.c a.out sample4$E sample5$E sample6$E yyselect.c

clobber::	clean
		rm -f y.tab.c gram.tab.c $(BURG)$E
