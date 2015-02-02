.SUFFIXES: .java .class .brg

SAMPLES=sample4j.class sample5j.class sample6j.class

BURG=../pburg.exe

all: $(BURG) $(SAMPLES)

.java.class:
	javac $<

.brg.java:
	$(BURG) -p $* -J $< $@

$(BURG):
	cd ..; make $(MFLAGS)

clean:
	rm $(SAMPLES)
