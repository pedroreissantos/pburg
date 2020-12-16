%{
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "y.tab.h"
%}
%%
[a-zA-Z][a-zA-Z0-9]+	yylval.s = strdup(yytext); return ID;
[0-9]+			yylval.i = atoi(yytext); return CONST;
.			return *yytext;
%%
int yywrap() { return 1; }
char *getyytext() { return yytext; }
