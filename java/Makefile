.SUFFIXES: .class .java .g4 .brg
SCAN=scan
GRAM=gram
MAIN=pburg
TREE=Tree

CLASS=$(MAIN).class $(SCAN).class $(GRAM).class $(TREE).class
SAMPLES=Sample4j.class Sample5j.class Sample6j.class

#BURG=../pburg.exe -J # C version
BURG=java $(MAIN)

all: $(CLASS) $(SAMPLES) # $(BURG)

$(MAIN).class: $(SCAN).java $(GRAM).java
$(GRAM).class: $(SCAN).java

.g4.java:
	antlr4 $<

.java.class:
	javac $<

.brg.java:
	$(BURG) -p $* $< $@

#$(BURG): ; cd ..; make $(MFLAGS)

run: all
	for i in $(SAMPLES); do echo "*** $$i ***"; java `basename $$i .class`; done

clean:
	rm -f *.class $(GRAM)*.java $(SCAN)*.java *.tokens *~
