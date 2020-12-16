%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "node.h"
#include "tabid.h"

int yylex();
void evaluate(Node *p);
void yyerror(char *s);
int lbl;
%}

%union {
	int i;			/* integer value */
	char *s;		/* symbol name or string literal */
	Node *n;		/* node pointer */
};

%token <i> INTEGER
%token <s> VARIABLE STRING
%token WHILE IF PRINT READ PROGRAM END

%nonassoc IFX
%nonassoc ELSE
%left GE LE EQ NE '>' '<'
%left '+' '-'
%left '*' '/' '%'
%nonassoc UMINUS

%type <n> stmt expr list

%token DO START

%%

program	: PROGRAM list END		  { evaluate($2); freeNode($2); }
	;

stmt	: ';'				  { $$ = 0; }
	| PRINT STRING ';'		  { $$ = strNode(STRING, $2); }
	| PRINT expr ';'		  { $$ = uniNode(PRINT, $2); }
	| READ VARIABLE ';'		  { if (IDfind($2, 0) < 0) $$ = 0; else $$ = strNode(READ, $2); }
	| VARIABLE '=' expr ';'		  { IDnew(0, $1, (void*)IDtest);
					    $$ = binNode('=', strNode(VARIABLE, $1), $3); }
	| WHILE '(' expr ')' stmt	  { $$ = binNode(WHILE, binNode(DO, nilNode(START), $3), $5); }
	| IF '(' expr ')' stmt %prec IFX  { $$ = binNode(IF, $3, $5); }
	| IF '(' expr ')' stmt ELSE stmt  { $$ = binNode(ELSE, binNode(IF, $3, $5), $7); }
	| '{' list '}'			  { $$ = $2; }
	;

list	: stmt				  { $$ = $1; }
	| list stmt			  { $$ = binNode(';', $1, $2); }
	;

expr	: INTEGER			  { $$ = intNode(INTEGER, $1); }
	| VARIABLE			  { if (IDfind($1, 0) < 0) $$ = 0; else $$ = strNode(VARIABLE, $1); }
	| '-' expr %prec UMINUS		  { $$ = uniNode(UMINUS, $2); }
	| expr '+' expr			  { $$ = binNode('+', $1, $3); }
	| expr '-' expr			  { $$ = binNode('-', $1, $3); }
	| expr '*' expr			  { $$ = binNode('*', $1, $3); }
	| expr '/' expr			  { $$ = binNode('/', $1, $3); }
	| expr '%' expr			  { $$ = binNode('%', $1, $3); }
	| expr '<' expr			  { $$ = binNode('<', $1, $3); }
	| expr '>' expr			  { $$ = binNode('>', $1, $3); }
	| expr GE expr			  { $$ = binNode(GE, $1, $3); }
	| expr LE expr			  { $$ = binNode(LE, $1, $3); }
	| expr NE expr			  { $$ = binNode(NE, $1, $3); }
	| expr EQ expr			  { $$ = binNode(EQ, $1, $3); }
	| '(' expr ')'			  { $$ = $2; }
	;

%%

char *mklbl(int n) {
  static char buf[20];
  sprintf(buf, "_i%d", n);
  return strdup(buf);
}

char **yynames =
#if YYDEBUG > 0
		 (char**)yyname;
#else
		 0;
#endif
