parser grammar gram;

options { tokenVocab=scan; } // use tokens from scan.g4

pburg	: pdecl* SEP prule* pcode?
	;

pdecl	: TERM plist* CR
	| START IDENT CR
	| INCLUDE STR CR
	| DECL CR
	| CR
	;

plist	: IDENT EQ INTEG
	| IDENT EQ CHAR
	;

prule	: ID CL ptree pact? NL
	| ID CL ptree ID pact? NL
	| ID CL ptree INT pact? NL
	| NL
	;

pact	: ACTION
	;

ptree	: ID
	| ID LP ptree RP
	| ID LP ptree CM ptree RP
	;

pcode	: SEP2 END* ;
