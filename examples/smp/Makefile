LANG=simple
EXT=spl# file extension: .$(EXT)
LIB=lib# compiler library directory
UTIL=util# compiler library: lib$(LIB).a
RUN=run# runtime directory
RUNC=runc# runtime directory for gcc
WRAP=wrap# ABI wrapper directory
EXS=exs# examples directory
CC=gcc
CFLAGS=-g -DYYDEBUG
LIBRUN=$(RUN)/librun.a
LDLIBS=$(LIBRUN)
LEX=flex -dl
YACC=byacc -dv
BURG=pburg -T
#AS=nasm -felf64
AS=as
LD=ld
#ARCH=-DpfAMD64

.SUFFIXES: .asm $(EXT)

all: $(LANG)
	make -C $(RUN) install

lib::
	make -C $(LIB)

$(LANG): gram.y scan.l code.brg lib
	$(YACC) gram.y
	$(LEX) scan.l
	$(BURG) code.brg
	$(LINK.c) -o $@ $(ARCH) -I$(LIB) lex.yy.c y.tab.c yyselect.c -L$(LIB) -l$(UTIL)

d$(LANG): gram.y scan.l real.brg lib
	$(YACC) gram.y
	$(LEX) scan.l
	$(BURG) real.brg
	$(LINK.c) -o $@ $(ARCH) -I$(LIB) lex.yy.c y.tab.c yyselect.c -L$(LIB) -l$(UTIL)

examples:: $(LANG)
	make -C $(EXS)

run:: $(LANG)
	make -C $(EXS) run

%: %.asm
	$(AS) $*.asm -o $*.o
	$(LD) -o $@ $*.o $(LDLIBS)

clean::
	make -C $(LIB) clean
	make -C $(RUN) clean
	make -C $(RUNC) clean
	make -C $(WRAP) clean
	make -C $(EXS) clean
	rm -f *.o $(LANG) d$(LANG) lib$(LANG).a lex.yy.c y.tab.c y.tab.h y.output yyselect.c *.asm *~
