%{
#include "pburg.h"
#include "y.tab.h"
#include <string.h>
static char rcsid[] = "$Id: scan.l,v 1.2 2010/10/25 11:32:18 prs Exp $";
char *blkadd(int), *strdup(const char*);
static int blkcnt;
int blkline;
%}

%x CODE END BLK BLKC
%s RULE
%%
^"%!".*		;
^"%{"		BEGIN(CODE);
<CODE>.|\n	fputs(yytext, outfp);
<CODE>^"%}"	BEGIN(INITIAL);
[ \f\t]+	;
\\\n		;
[\n(),;=]	return yytext[0];
[:]		blkline = yylineno; return yytext[0];
<RULE>^"%%"	BEGIN(END); blkline = yylineno; return 0;
<RULE>\{	BEGIN(BLK); blkadd(0); blkcnt = 1; blkline = yylineno;
^"%%"		BEGIN(RULE); return PPERCENT;
^"%term"	return TERMINAL;
^"%start"	return START;
^"%include"[ \t]+\"([^"\\]|\\.)*\"	yytext[yyleng-1] = 0; yylval.string = strdup(strchr(yytext,'"')+1); return INCLUDE;
[0-9]+		yylval.n = atoi(yytext); return INT;
\'[^'\\]\'	yylval.n = yytext[1]; return INT;
[a-zA-Z_][a-zA-Z_0-9]*		yylval.string = strdup(yytext); return ID;
<BLK>\"([^"\\]|\\.)*\"		blkadd(1);
<BLK>\'([^'\\]|\\..?.?)\'	blkadd(1);
<BLK>\}		blkadd(1); if (--blkcnt <= 0) { BEGIN(RULE); return BLOCK; }
<BLK>\{		blkadd(1); blkcnt++;
<BLK>"//".*	blkadd(1);
<BLK>"/*"	blkadd(1); BEGIN(BLKC);
<BLK>[^"'{}]	blkadd(1); /* was: .|\n */
<BLKC>"*/"	blkadd(1); BEGIN(BLK);
<BLKC>.|\n	blkadd(1);
<END>.*\n	fputs(yytext, outfp);
.|\n		yyerror("unrecognised character: %s\n", yytext);
%%
int yywrap() { return 1; }
#define INCR 10
char *blkadd(int i)
{
  static int cnt, max;
  if (i == 0) {
    yylval.string = (char*)malloc(max = INCR);
    if (yylval.string == 0) {
      yyerror("out of memory.");
      exit(2);
    }
    *yylval.string = 0;
    cnt = 0;
  }
  if ((cnt += yyleng) >= max -1) {
    yylval.string = (char*)realloc(yylval.string, max = max + yyleng + INCR);
    if (yylval.string == 0) {
      yyerror("out of memory.");
      exit(2);
    }
  }
  strcat(yylval.string, yytext);
  return yylval.string;
}
