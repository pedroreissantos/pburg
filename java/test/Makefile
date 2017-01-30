.SUFFIXES: .class .java .brg
BURG=java -jar ../pburg-all.jar
JAVAC=javac -cp .:../pburg-all.jar
JAVA=java -cp .:../pburg-all.jar

SAMPLES=Sample4j.class Sample5j.class Sample6j.class

all: $(SAMPLES)
	cd ..; for i in $(SAMPLES); do echo "*** $$i ***"; java test.`basename $$i .class`; done


.java.class:
	$(JAVAC) $<

.brg.java:
	$(BURG) -Jclass=$* $< $@

clean::
	rm -rf *.class *.java
