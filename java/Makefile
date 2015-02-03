.SUFFIXES: .java .class .brg

SAMPLES=Sample4j.class Sample5j.class Sample6j.class

BURG=../pburg.exe

all: $(BURG) $(SAMPLES)

run: all
	for i in $(SAMPLES); do echo "*** $$i ***"; java `basename $$i .class`; done

.java.class:
	javac $<

.brg.java:
	$(BURG) -p $* -J $< $@

$(BURG):
	cd ..; make $(MFLAGS)

clean:
	rm -f $(SAMPLES) *.class *~
