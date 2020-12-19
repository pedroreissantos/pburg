RUN=run
CFLAGS=-g -fno-stack-protector # when __stack_chk_fail in Ubunto
BOOT=$(RUN).o # bootstrap runtime for a C compiler
RUNTIME=lib$(RUN).a # stand-alone runtime
OBJ=linux64.o lib64.o sys64.o wrap.o dblwrap.o char.o cwrap.o
CC=gcc

all: $(RUNTIME) $(BOOT)

$(RUNTIME): dbl.o $(OBJ)
	ar crl $@ $?

$(BOOT): dbl.o $(OBJ)
	ld -r -o  $@ $^

dbl.o: dbl.c
	$(CC) $(CFLAGS) -DPOSTFIX -c $<

.SUFFIXES: .s .obj .wrp
.s.obj:
	$(AS) $< -o $@
.s.o:
	$(AS) $< -o $@
.wrp.s:
	$(WRAP) $< $@
install:: $(RUNTIME)
	cp $(RUNTIME) ..
clean::
	rm -f $(RUNTIME) $(BOOT) *.o a.out core
