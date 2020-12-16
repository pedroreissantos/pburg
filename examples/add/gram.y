%{
#include <stdio.h>
#include <stdlib.h>
#include "node.h"
#include "tabid.h"
extern int yylineno, trace;
extern FILE *yyin;
extern int yyselect(Node*);
FILE *out;
#ifndef YYDEBUG
#define yyname 0
#endif
%}
%union { char *s; Node *n; int i; }
%token PRINT
%token<i> INT
%token<s> STR ID
%type<n> decls decl instrs instr strs expr
%token NIL INSTRS PROG DECLS EXPR
%left '+'
%%
file: decls '{' instrs '}'    { Node *n = binNode(PROG, $1, $3); if (trace) printNode(n, 0, (char**)yyname); yyselect(n); }
    ;
decls:                  { $$ = nilNode(NIL); }
     | decls decl       { $$ = binNode(DECLS, $1, $2); }
     ;
decl: ID '=' STR ';'    { $$ = binNode('=', strNode(ID, $1), strNode(STR, $3)); IDnew(STR, $1, 0); }
    | ID '=' INT ';'    { $$ = binNode('=', strNode(ID, $1), intNode(INT, $3)); IDnew(INT, $1, 0); }
    ;
instrs:                 { $$ = nilNode(NIL); }
      | instrs instr    { $$ = binNode(INSTRS, $1, $2); }
      ;
instr: PRINT strs ';'   { $$ = uniNode(PRINT, $2); }
     ;
strs: expr
    | strs ',' expr     { $$ = binNode(EXPR, $1, $3); }
    ;
expr: ID                { $$ = strNode(ID, $1); $$->info = IDfind($1, 0); }
    | INT               { $$ = intNode(INT, $1); $$->info = INT; }
    | expr '+' expr     { $$ = binNode('+', $1, $3); if ($1->info == STR || $3->info == STR) yyerror("only integers can be added"); }
    | STR               { $$ = strNode(STR, $1); $$->info = STR; }
    ;
