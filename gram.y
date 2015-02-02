%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pburg.h"
static char rcsid[] = "$Id: gram.y,v 2.5 1997/11/21 18:59:34 drh Exp $";
/*lint -e616 -e527 -e652 -esym(552,yynerrs) -esym(563,yynewstate,yyerrlab) */
#define YYDEBUG 1
extern int yylineno, blkline;
extern char *infile;
%}
%union {
	int n;
	char *string;
	Tree tree;
}
%term TERMINAL
%term START
%term PPERCENT
%token  <string>        ID INCLUDE BLOCK
%token  <n>             INT
%type	<string>	nonterm block
%type   <tree>          tree
%%
spec	: decls PPERCENT rules		{ }
	| decls				{ yydebug = 1; }
	;

decls	: /* lambda */
	| decls decl
	;

decl	: TERMINAL blist '\n'
	| START nonterm '\n'		{
		if (nonterm($2)->number != 1)
			yyerror("redeclaration of the start symbol\n");
		}
	| INCLUDE { include($1); }
	| '\n'
	| error '\n'			{ yyerrok; }
	;

blist	: /* lambda */
	| blist ID '=' INT      	{ term($2, $4); }
	;

rules	: /* lambda */
	| rules nonterm ':' tree block '\n'	{ rule($2, $4, $5, "0", blkline); }
	| rules nonterm ':' tree ID block '\n'	{ rule($2, $4, $6, $5, blkline); }
	| rules nonterm ':' tree INT block '\n'	{ char cost[20];
						  sprintf(cost, "%d", $5);
						  rule($2, $4, $6, strdup(cost), blkline); }
	| rules '\n'
	| rules error '\n'		{ yyerrok; }
	;

block	: /* lambda */				{ $$ = 0; }
	| BLOCK					{ $$ = $1; }
	;

nonterm	: ID				{ nonterm($$ = $1); }
	;

tree	: ID                            { $$ = tree($1,  0,  0); }
	| ID '(' tree ')'               { $$ = tree($1, $3,  0); }
	| ID '(' tree ',' tree ')'      { $$ = tree($1, $3, $5); }
	;
%%
#include <stdarg.h>
#include <ctype.h>

int errcnt = 0;
FILE *outfp = NULL;

void yyerror(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	if (yylineno > 0)
		fprintf(stderr, "%s: line %d: ", infile, yylineno);
	vfprintf(stderr, fmt, ap);
	if (fmt[strlen(fmt)-1] != '\n')
		 fprintf(stderr, "\n");
	errcnt++;
	va_end(ap);
}

void yywarn(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	if (yylineno > 0)
		fprintf(stderr, "%s: line %d: ", infile, yylineno);
	fprintf(stderr, "warning: ");
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

void yyinfo(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	if (yylineno > 0)
		fprintf(stderr, "%s: line %d: ", infile, yylineno);
	fprintf(stderr, "information: ");
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

#define MAXSIZ 120
static void include(const char* filename) {
	  extern int Aflag, Tflag, Jflag;
	  FILE *fp = fopen(filename, "r");
	  char buf[MAXSIZ], name[MAXSIZ];
	  int val, num = 0;
	  if (fp != 0) {
#ifndef NOJAVA
	    if (Jflag) {
	      while (fgets(buf, MAXSIZ-1, fp))
		if (strncmp("public final static short ", buf, 26) == 0 &&
		    sscanf(buf+26, "%[^=]=%d;", name, &val) == 2)
		  term(strdup(name), val), num++;
	    } else
#endif
	    if (Aflag) {
	      while (fgets(buf, MAXSIZ-1, fp))
		if (strncmp("#define", buf, 7) == 0 &&
		    sscanf(buf, "%*s %s %d", name, &val) == 2)
		  term(strdup(name), val), num++;
	    } else {
	      while (fgets(buf, MAXSIZ-1, fp)
	             && strncmp("#define", buf, 7) == 0)
		if (sscanf(buf, "%*s %s %d", name, &val) == 2)
		  term(strdup(name), val), num++;
	    }
	    fclose(fp);
	  }
	  if (Tflag) yyinfo("included %d tokens from \"%s\"\n", num, filename);
	}
