lexer grammar scan;

DECL:		'%{' .*? '%}' ;
COM:	( '%!' (~[\n\r])* ) -> skip;
CCOM:		'/*' .*? '*/' -> skip ;
IDENT:		[a-zA-Z_] [a-zA-Z_0-9]* ;
INTEG:		[0-9]+ ;
SPACE:		[ \t\f] -> skip;
CHAR:		'\'' ~[\'\\] '\'' ;
STR:		'"' ( '\\"' | . )*? '"' ;
/*
STRING:		'"' (ESC | ~["\\])* '"' ;
fragment ESC:   '\\' (["\\/bfnrt] | UNICODE) ;
fragment UNICODE: 'u' HEX HEX HEX HEX ;
fragment HEX: [0-9a-fA-F] ;
*/
EQ:		'=' ;
TERM:		'%term' ;
START:		'%start' ;
INCLUDE:	'%include' ;
SEP:		'%%' -> mode(X);
CR:		'\r'? '\n' ;

mode X;
ID:		[a-zA-Z_] [a-zA-Z_0-9]* ;
INT:		[0-9]+ ;
STRING:		'"' ( '\\"' | . )*? '"' ;
COMMENT:	( '%!' (~[\n\r])* ) -> skip;
ACTION:		'{' ( ACTION | ~[{}] )* '}' ;
LP:		'(' ;
RP:		')' ;
CM:		',' ;
CL:		':' ;
NL:		'\r'? '\n' ;
SEP2:		'%%' -> mode(Y);
SPC:		[ \t\f] -> skip;

mode Y;
END:	. ;
