.SUFFIXES: .class .java .g4 .brg
ANTLR4LIB=$$HOME/Java/antlr-4.6-complete.jar
ANTLR4=java -jar $(ANTLR4LIB)
JAVAC=javac -d . -cp .:$(ANTLR4LIB)
JAR=jar
#BURG=../pburg.exe -J # C version
BURG=java -jar $(PROG)-all.jar

PROG=pburg
D=pt/tecnico/pburg/
SCAN=scan
GRAM=gram
SRCS=$DMain.java $(SCAN).g4 $(GRAM).g4 $DName.java $DNonterm.java $DPattern.java $DRule.java $DTerm.java $DTree.java

all: $(PROG)-all.jar test

$(PROG).jar: $(SRCS)
	$(ANTLR4) *.g4 -o $D
	$(JAVAC) $D*.java
	$(JAR) -cfm $@ MANIFEST $D*.class

$(PROG)-all.jar: $(PROG).jar
	$(JAR) -xf $(ANTLR4LIB) org/antlr/v4/runtime
	$(JAR) -cfm $@ MANIFEST $D*.class org

test::
	cd test; make

clean::
	rm -rf $D*.class $D$(GRAM)*.java $D$(SCAN)*.java $D*.tokens $D*~ pburg*.jar org test/*.class
