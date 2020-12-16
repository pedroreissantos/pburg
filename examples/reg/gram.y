%{
#include <stdio.h>
#include <stdlib.h>
#include "node.h"
int yylex(), yyparse(), yyerror(char*), yyselect(Node*);
%}
%union { int i; char *s; Node *n; }
%token<i> CONST
%token<s> ID
%type<n> expr
%nonassoc '='
%left '+'
%%
obj: expr ';'       { yyselect(uniNode(';', $1)); }
expr: ID             { $$ = strNode(ID, $1); }
    | CONST          { $$ = intNode(CONST, $1); }
    | ID '=' expr    { $$ = binNode('=', strNode(ID, $1), $3); }
    | expr '+' expr  { $$ = binNode('+', $1, $3); }
    ;
%%
char **yynames =
#if YYDEBUG > 0
(char**)yyname;
#else
 0;
#endif
