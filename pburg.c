#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pburg.h"

#define PREFIX(name) code##name		/* concatenate */
#define VER(maj,min) #maj"."#min	/* stringify */

#define VERSION "2.5"
static char rcsid[] = "$Id: pburg.c,v " VERSION " 2014/06/24 17:04:42 prs Exp $";
static char *prefix = "yy";
char *outfile = "select.c";
char *infile = "<stdin>";
int Aflag = 0, Tflag = 0, nflag = 0;
#ifndef NOJAVA
#define YYCLASS "Selector"
#define YYTREE "Tree"
int Jflag = 0;
static int pflag = 0; /* prefix has changed */
char *treeClass = YYTREE, *outClass = YYCLASS;
static char *extends = "", *throws = "", *final = "";
static void emitjava(void);
#endif
#ifndef NOMUNCH
static void emitmunch(Term terms, Nonterm start, int ntnumber);
int mflag = 0;
#endif
int nkids, maxkids;

extern int yylex();

static int ntnumber = 0;
static Nonterm start = 0;
static Term terms;
static Nonterm nts;
static Rule rules;
static int nrules;
static struct block {
	struct block *link;
} *memlist;			/* list of allocated blocks */

static char *stringf(char *fmt, ...);
static void print(char *fmt, ...);
static void ckreach(Nonterm p);
static void emitclosure(Nonterm nts);
static void emitcost(Tree t, char *v);
static void emitdefs(Nonterm nts, int ntnumber);
static void emitheader(void);
static void emitkids(Rule rules, int nrules);
static void emitnts(Rule rules, int nrules);
static void emitrecord(char *pre, Rule r, char *c, int cost);
static void emitrule(Nonterm nts);
static void emitlabel(Term terms, Nonterm start, int ntnumber);
static void emitselect();
static void emitstring(Rule rules);
static void emitstruct(Nonterm nts, int ntnumber);
static void emittrace();
static void emitreduce(Rule rules);
static void emittest(Tree t, char *v, char *suffix);
static void emitcontext(void);

int main(int argc, char *argv[]) {
	int i;
	Nonterm p;
	extern int blkline;
	time_t timer = time(NULL);
	
	for (i = 1; i < argc; i++)
		if (strcmp(argv[i], "-T") == 0)
			Tflag++;
		else if (strncmp(argv[i], "-J", 2) == 0) {
#ifndef NOJAVA
			Jflag = 1;
			if (argv[i][2] != 0) {
				if (strncmp(argv[i], "-Jclass=", 8) == 0)
					pflag++, prefix = &argv[i][8];
				if (strncmp(argv[i], "-Jfinal", 7) == 0)
					final = &argv[i][2];
				if (strncmp(argv[i], "-Jextends=", 10) == 0)
					extends = &argv[i][10];
				if (strncmp(argv[i], "-Jtree=", 7) == 0)
					treeClass = &argv[i][7];
				if (strncmp(argv[i], "-Jthrows=", 9) == 0)
					throws = &argv[i][9];
			}
			if (!pflag) prefix = YYCLASS;
			outfile = alloc(strlen(prefix)+6);
			sprintf(outfile, "%s.java", prefix);
			/*
			if (argv[i][2] != 0) treeClass = &argv[i][2];
			if (argv[i][2] != 0) {
			  char *sep = strchr(&argv[i][2], ',');
			  if (sep != 0) {
			    *sep++ = 0;
			    if (*sep != 0) outClass = sep;
			  }
			  if (argv[i][2] != 0) treeClass = &argv[i][2];
			}
			*/
#else
			fprintf(stderr, "%s not compiled with Java support.\n", argv[0]);
#endif
		}
		else if (strcmp(argv[i], "-m") == 0)
#ifndef NOMUNCH
      mflag++;
#else
			fprintf(stderr, "%s not compiled with maximal-munch support.\n", argv[0]);
#endif
		else if (strcmp(argv[i], "-n") == 0)
			nflag = 1;
		else if (strcmp(argv[i], "-A") == 0)
			Aflag = 1;
		else if (strcmp(argv[i], "-v") == 0)
			fprintf(stderr, "%s\n", rcsid), exit(1);
		else if (strncmp(argv[i], "-p", 2) == 0 && argv[i][2])
			prefix = &argv[i][2], ++pflag;
		else if (strncmp(argv[i], "-p", 2) == 0 && i + 1 < argc)
			prefix = argv[++i], ++pflag;
		else if (*argv[i] == '-' && argv[i][1]) {
			yyerror("usage: %s [-T | -A | -n | -v | -p prefix"
#ifndef NOMUNCH
			    " | -m",
#endif
#ifdef NOJAVA
			    " ]... [ [ input ] output ] \n",
#else
			    " | -J[option] ]... [ [ input ] output ] \n"
			    "\t-Jclass=name (of the class to generate, if other than '" YYCLASS "')\n"
			    "\t-Jfinal (make the generated class 'final')\n"
			    "\t-Jextends=declaration (full declaration of extends and implements)\n"
			    "\t-Jtree=name (tree class name, if other than '" YYTREE "')\n"
			    "\t-Jthrows=declaration (full declaration of reduce excpetions)\n",
#endif
				argv[0]);
			exit(1);
		} else if (yyin == NULL) {
			if (strcmp(argv[i], "-") == 0)
				yyin = stdin;
			else if ((yyin = fopen(infile = argv[i], "r")) == NULL) {
				yyerror("%s: can't read `%s'\n", argv[0], argv[i]);
				exit(1);
			}
		} else if (outfp == NULL) {
			if (strcmp(argv[i], "-") == 0)
				outfp = stdout;
			else if ((outfp = fopen(argv[i], "w")) == NULL) {
				yyerror("%s: can't write `%s'\n", argv[0], argv[i]);
				exit(1);
			}
			else outfile = argv[i];
		}
	if (outfp == NULL) {
		int off = 0;
		char *buf = malloc(strlen(prefix) + strlen(outfile) + 1);
		sprintf(buf, "%s%s", prefix, outfile);
#ifndef NOJAVA
		if (Jflag) off += strlen(prefix);
#endif
		if ((outfp = fopen(buf+off, "w")) == NULL) {
			yyerror("%s: can't write `%s'\n", argv[0], buf+off);
			exit(1);
		}
		free(buf);
	}
	if (yyin == NULL)
		yyin = stdin;
	if (outfp == NULL)
		outfp = stdout;

	print("/*\ngenerated at %sby %s\n*/\n", ctime(&timer), rcsid);
#ifndef NOJAVA
	if (!Jflag)
#endif
		emitcontext();

	yyparse();
	if (start)
		ckreach(start);
	for (p = nts; p; p = p->link) {
		if (p->rules == NULL)
			yyerror("undefined nonterminal `%s'\n", p->name);
		if (!p->reached)
			yyerror("can't reach nonterminal `%s'\n", p->name);
	}
	if (errcnt)
		return errcnt > 0;

#ifndef NOJAVA
	if (Jflag) emitjava(); else
#endif
	{
	emitheader();
	emitdefs(nts, ntnumber);
#ifndef NOMUNCH
	if (!mflag)
#endif
	emitstruct(nts, ntnumber);
	emitnts(rules, nrules);
	emitstring(rules);
#ifndef NOMUNCH
	if (mflag)
		emitmunch(terms, start, ntnumber);
  else
#endif
  {
	if (Tflag)
		emittrace();
	emitrule(nts);
	emitclosure(nts);
	if (start)
		emitlabel(terms, start, ntnumber);
  }
	if (errcnt)
		return errcnt > 0;
	emitkids(rules, nrules);
#ifndef NOMUNCH
	if (!mflag)
#endif
	emitreduce(rules);
	if (start)
		emitselect();
	if (nflag == 0) print("\n#line %d \"%s\"\n", blkline, infile);
	}
	while(yylex()) ; /* flush the code after the last %% */
#ifndef NOJAVA
	if (Jflag) print("\n}\n");
#endif
	while (memlist) {	/* for purify */
		struct block *q = memlist->link;
		free(memlist);
		memlist = q;
	}
	return errcnt > 0;
}

/* alloc - allocate nbytes or issue fatal error */
void *alloc(int nbytes) {
	struct block *p = calloc(1, sizeof *p + nbytes);

	if (p == NULL) {
		yyerror("out of memory\n");
		exit(1);
	}
	p->link = memlist;
	memlist = p;
	return p + 1;
}

/* stringf - format and save a string */
static char *stringf(char *fmt, ...) {
	va_list ap;
	char buf[512];

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	return strcpy(alloc(strlen(buf) + 1), buf);
}	

struct entry {
	union {
		char *name;
		struct term t;
		struct nonterm nt;
	} sym;
	struct entry *link;
} *table[211];
#define HASHSIZE (sizeof table/sizeof table[0])

/* hash - return hash number for str */
static unsigned hash(char *str) {
	unsigned h = 0;

	while (*str)
		h = (h<<1) + *str++;
	return h;
}

/* lookup - lookup symbol name */
static void *lookup(char *name) {
	struct entry *p = table[hash(name)%HASHSIZE];

	for ( ; p; p = p->link)
		if (strcmp(name, p->sym.name) == 0)
			return &p->sym;
	return 0;
}

/* install - install symbol name */
static void *install(char *name) {
	struct entry *p = alloc(sizeof *p);
	int i = hash(name)%HASHSIZE;

	p->sym.name = name;
	p->link = table[i];
	table[i] = p;
	return &p->sym;
}

/* nonterm - create a new terminal id, if necessary */
Nonterm nonterm(char *id) {
	Nonterm p = lookup(id), *q = &nts;

	if (p && p->kind == NONTERM)
		return p;
	if (p && p->kind == TERM)
		yyerror("`%s' is a terminal\n", id);
	p = install(id);
	p->kind = NONTERM;
	p->number = ++ntnumber;
	if (p->number == 1)
		start = p;
	while (*q && (*q)->number < p->number)
		q = &(*q)->link;
	assert(*q == 0 || (*q)->number != p->number);
	p->link = *q;
	*q = p;
	return p;
}

/* term - create a new terminal id with external symbol number esn */
Term term(char *id, int esn) {
	Term p = lookup(id), *q = &terms;

	if (p) {
		yyerror("redefinition of terminal `%s' (ignored)\n", id);
		return p;
	}
	else
		p = install(id);
	p->kind = TERM;
	p->esn = esn;
	p->arity = -1;
	while (*q && (*q)->esn < p->esn)
		q = &(*q)->link;
	if (*q && (*q)->esn == p->esn && (*q)->esn != -1)
		yyerror("duplicate external symbol number `%s=%d'\n",
			p->name, p->esn);
	p->link = *q;
	*q = p;
	return p;
}

/* tree - create & initialize a tree node with the given fields */
Tree tree(char *id, Tree left, Tree right) {
	Tree t = alloc(sizeof *t);
	Term p = lookup(id);
	int arity = 0;

	if (left && right)
		arity = 2;
	else if (left)
		arity = 1;
	if (p == NULL && arity > 0) {
		yyerror("undefined terminal `%s'\n", id);
		p = term(id, -1);
	} else if (p == NULL && arity == 0)
		p = (Term)nonterm(id);
	else if (p && p->kind == NONTERM && arity > 0) {
		yyerror("`%s' is a nonterminal\n", id);
		p = term(id, -1);
	}
	if (p->kind == TERM && p->arity == -1)
		p->arity = arity;
	if (p->kind == TERM && arity != p->arity)
		yyerror("inconsistent arity for terminal `%s'\n", id);
	t->op = p;
	t->nterms = p->kind == TERM;
	if ((t->left = left) != NULL)
		t->nterms += left->nterms;
	if ((t->right = right) != NULL)
		t->nterms += right->nterms;
	return t;
}

#ifndef NOMUNCH
/* comparetreeterms - compare the location of terminals in two trees */
static int comparetreeterms(Tree a, Tree b) {
  Term ta = a->op, tb = b->op;

  if (ta->kind != tb->kind) return 0; /* different kind */
  if (ta->kind != TERM) return 1; /* both are non-terminals */
  if (ta->esn != tb->esn) return 0; /* different terminals */
  if (a->left && b->left) if (!comparetreeterms(a->left, b->left)) return 0;
  if (a->right && b->right) if (!comparetreeterms(a->right, b->right)) return 0;
  return 1;
}
#endif

/* rule - create & initialize a rule with the given fields */
Rule rule(char *id, Tree pattern, char *template, char *code, int line) {
	Rule r = alloc(sizeof *r), *q;
	Term p = pattern->op;
	char *end;

#ifndef NOMUNCH
   if (mflag == 1) {
    extern int yylineno;
    int lineno = yylineno;
    yylineno = line;
    if (pattern->nterms == 0)
      yyerror("maximal-munch requires a terminal in the rule\n");
    else {
      Term tm;
      Rule rl;
      for (tm = terms; tm; tm = tm->link)
	if (tm->esn == p->esn)
	  for (rl = tm->rules; rl; rl = rl->next)
	    if (comparetreeterms(pattern, rl->pattern))
	      yyerror("maximal-munch requires different terminal patterns from rule in line %d\n", rl->lineno);
    }
    yylineno = lineno;
  }
#endif
	r->lhs = nonterm(id);
	r->packed = ++r->lhs->lhscount;
	for (q = &r->lhs->rules; *q; q = &(*q)->decode)
		;
	*q = r;
	r->pattern = pattern;
	r->ern = ++nrules;
	r->template = template;
	r->lineno = line;
	r->code = code;
	r->cost = strtol(code, &end, 10);
	if (*end) {
		r->cost = -1;
		r->code = stringf("(%s(a))", code);
	}
	if (p->kind == TERM) {
#ifndef NOMUNCH
    if (mflag) {
	Rule rq, ant = 0;
	for (rq = p->rules;
	       rq && r->pattern->nterms <= rq->pattern->nterms ;
	       ant = rq, rq = rq->next) ;
		r->next = rq;
	if (ant) ant->next = r; else p->rules = r;
    } else
#endif
    {
	for (q = &p->rules; *q; q = &(*q)->next)
		;
	*q = r;
    }

	} else if (pattern->left == NULL && pattern->right == NULL) {
		Nonterm p = pattern->op;
		r->chain = p->chain;
	        p->chain = r;
		if (r->cost == -1)
			yyerror("illegal nonconstant cost `%s'\n", code);
	}
	for (q = &rules; *q; q = &(*q)->link)
		;
	r->link = *q;
	*q = r;
	return r;
}

/* print - formatted output */
static void print(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	for ( ; *fmt; fmt++)
		if (*fmt == '%')
			switch (*++fmt) {
			case 'd': fprintf(outfp, "%d", va_arg(ap, int)); break;
			case 's': fputs(va_arg(ap, char *), outfp); break;
			case 'P': fprintf(outfp, "%s", prefix); break;
			case 'T': {
				Tree t = va_arg(ap, Tree);
				print("%S", t->op);
				if (t->left && t->right)
					print("(%T,%T)", t->left, t->right);
				else if (t->left)
					print("(%T)", t->left);
				break;
				}
			case 'R': {
				Rule r = va_arg(ap, Rule);
				print("%S: %T", r->lhs, r->pattern);
				break;
				}
			case 'S': {
				Term t = va_arg(ap, Term);
				fputs(t->name, outfp);
				break;
				}
			case '1': case '2': case '3': case '4': case '5': {
				int n = *fmt - '0';
				while (n-- > 0)
					putc('\t', outfp);
				break;
				}
			default: putc(*fmt, outfp); break;			
			}
		else
			putc(*fmt, outfp);
	va_end(ap);
}

/* reach - mark all nonterminals in tree t as reachable */
static void reach(Tree t) {
	Nonterm p = t->op;

	if (p->kind == NONTERM)
		if (!p->reached)
			ckreach(p);
	if (t->left)
		reach(t->left);
	if (t->right)
		reach(t->right);
}

/* ckreach - mark all nonterminals reachable from p */
static void ckreach(Nonterm p) {
	Rule r;

        p->reached = 1;
	for (r = p->rules; r; r = r->decode)
		reach(r->pattern);
}

/* emitcase - emit one case in function state */
static void emitcase(Term p, int ntnumber) {
	Rule r;

	print("%1case %d: /* %S */\n", p->esn, p);
	switch (p->arity) {
	case 0: case -1:
		break;
	case 1:
		print("%2%Plabel(LEFT_CHILD(a),a);\n");
		break;
	case 2:
		print("%2%Plabel(LEFT_CHILD(a),a);\n");
		print("%2%Plabel(RIGHT_CHILD(a),a);\n");
		break;
	default: assert(0);
	}
	for (r = p->rules; r; r = r->next) {
		char *indent = "\t\t\0";
		switch (p->arity) {
		case 0: case -1:
			print("%2/* %R */\n", r);
			if (r->cost == -1) {
				print("%2c = %s;\n", r->code);
				emitrecord("\t\t", r, "c", 0);
			} else
				emitrecord("\t\t", r, r->code, 0);
			break;
		case 1:
			if (r->pattern->nterms > 1) {
				print("%2if (%1/* %R */\n", r);
				emittest(r->pattern->left, "LEFT_CHILD(a)", " ");
				print("%2) {\n");
				indent = "\t\t\t";
			} else
				print("%2/* %R */\n", r);
			print("%sc = ", indent);
			emitcost(r->pattern->left, "LEFT_CHILD(a)");
			print("%s;\n", r->code);
			emitrecord(indent, r, "c", 0);
			if (indent[2])
				print("%2}\n");
			break;
		case 2:
			if (r->pattern->nterms > 1) {
				print("%2if (%1/* %R */\n", r);
				emittest(r->pattern->left,  "LEFT_CHILD(a)",
					r->pattern->right->nterms ? " && " : " ");
				emittest(r->pattern->right, "RIGHT_CHILD(a)", " ");
				print("%2) {\n");
				indent = "\t\t\t";
			} else
				print("%2/* %R */\n", r);
			print("%sc = ", indent);
			emitcost(r->pattern->left,  "LEFT_CHILD(a)");
			emitcost(r->pattern->right, "RIGHT_CHILD(a)");
			print("%s;\n", r->code);
			emitrecord(indent, r, "c", 0);
			if (indent[2])
				print("%2}\n");
			break;
		default: assert(0);
		}
	}
	if (p->rules) print("%2break;\n"); else print("%2return;\n"); /* 2.5 */
}

/* emitclosure - emit the closure functions */
static void emitclosure(Nonterm nts) {
	Nonterm p;

	for (p = nts; p; p = p->link)
		if (p->chain)
			print("static void %Pclosure_%S(NODEPTR_TYPE, int);\n", p);
	print("\n");
	for (p = nts; p; p = p->link)
		if (p->chain) {
			Rule r;
			print("static void %Pclosure_%S(NODEPTR_TYPE a, int c) {\n"
"%1struct %Pstate *p = (struct %Pstate *)STATE_LABEL(a);\n", p);
			for (r = p->chain; r; r = r->chain)
				emitrecord("\t", r, "c", r->cost);
			print("}\n\n");
		}
}

/* emitcost - emit cost computation for tree t */
static void emitcost(Tree t, char *v) {
	Nonterm p = t->op;

	if (p->kind == TERM) {
		if (t->left)
			emitcost(t->left,  stringf("LEFT_CHILD(%s)",  v));
		if (t->right)
			emitcost(t->right, stringf("RIGHT_CHILD(%s)", v));
	} else
		print("((struct %Pstate *)STATE_LABEL(%s))->cost[%P%S_NT] + ", v, p);
}

/* emitdefs - emit nonterminal defines and data structures */
static void emitdefs(Nonterm nts, int ntnumber) {
	Nonterm p;
	Term t;
	int i = 0, col = 0;

	for (p = nts; p; p = p->link)
		print("#define %P%S_NT %d\n", p, p->number);
	print("\n");

	if (!Tflag) return;
	print("static YYCONST char *%Pntname[] = {\n%10,\n");
	for (p = nts; p; p = p->link)
		print("%1\"%S\",\n", p);
	print("%10\n};\n\n");

	print("static YYCONST char *%Ptermname[] = {\n");
	for (t = terms; t; t = t->link) {
		while (t->esn > i++) {
			if ((col += 3) > 72) col = 3, print("\n");
			print(" \"\",");
		}
		if (col > 0) print("\n");
		print("%1/* %d */ \"%S\",\n", i, t);
		col = 0;
	}
	print("%1\"\"\n};\n\n");
}

/* emitcontext - emit context definitions */
static void emitcontext(void) {
	print("#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n");
	print("#define PBURG_PREFIX \"%s\"\n", prefix);
	print("#define PBURG_VERSION \"%s\"\n", VERSION);
	print("#define MAX_COST 0x7fff\n");
	print("#if defined(__STDC__) || defined(__cplusplus)\n"
		"#define YYCONST const\n#else\n#define YYCONST\n#endif\n");
	if (nflag == 0) print("\n#line 1 \"%s\"\n", infile);
	if (Tflag == 0) print("\n#define NOTRACE 1\n");
}

/* emitheader - emit initial definitions */
static void emitheader(void) {
	print("#ifndef PANIC\n#define PANIC %Ppanic\nstatic void "
	      "%Ppanic(YYCONST char *rot, YYCONST char *msg, int val) {\n"
	      "%1fprintf(stderr, \"Internal Error in %%s: %%s %%d\\n"
	      "exiting...\\n\",\n%2rot, msg, val);\n%1exit(2);\n}\n#endif\n");

	print("static void %Pkids(NODEPTR_TYPE, int, NODEPTR_TYPE[]);\n");
        /* prs 18mai13
	print("static void %Plabel(NODEPTR_TYPE,NODEPTR_TYPE);\n");
	print("static int %Prule(void*, int);\n\n");
        */
}

/* computekids - compute paths to kids in tree t */
static char *computekids(Tree t, char *v, char *bp, int *ip) {
	Term p = t->op;

	if (p->kind == NONTERM) {
		sprintf(bp, "\t\tkids[%d] = %s;\n", (*ip)++, v);
		bp += strlen(bp);
	} else if (p->arity > 0) {
		bp = computekids(t->left, stringf("LEFT_CHILD(%s)", v), bp, ip);
		if (p->arity == 2)
			bp = computekids(t->right, stringf("RIGHT_CHILD(%s)", v), bp, ip);
	}
	return bp;
}

/* emitkids - emit _kids */
static void emitkids(Rule rules, int nrules) {
	int i;
	Rule r, *rc = alloc((nrules + 1 + 1)*sizeof *rc);
	char **str  = alloc((nrules + 1 + 1)*sizeof *str);

	for (i = 0, r = rules; r; r = r->link) {
		int j = 0;
		char buf[1024], *bp = buf;
		*computekids(r->pattern, "p", bp, &j) = 0;
		for (j = 0; str[j] && strcmp(str[j], buf); j++)
			;
		if (str[j] == NULL)
			str[j] = strcpy(alloc(strlen(buf) + 1), buf);
		r->kids = rc[j];
		rc[j] = r;
	}
	print("static void %Pkids(NODEPTR_TYPE p, int eruleno, NODEPTR_TYPE kids[]) {\n"
"%1if (!p)\n%2PANIC(\"%Pkids\", \"Null tree in rule\", eruleno);\n"
"%1if (!kids)\n%2PANIC(\"%Pkids\", \"Null kids in\", OP_LABEL(p));\n"
"%1switch (eruleno) {\n");
	for (i = 0; (r = rc[i]) != NULL; i++) {
		for ( ; r; r = r->kids)
			print("%1case %d: /* %R */\n", r->ern, r);
		print("%s%2break;\n", str[i]);
	}
	print("%1default:\n%2PANIC(\"%Pkids\", \"Bad rule number\", eruleno);\n%1}\n}\n\n");
}

/* emitlabel - emit label function */
static void emitlabel(Term terms, Nonterm start, int ntnumber) {
	int i;
	Term p;

	print("static void %Plabel(NODEPTR_TYPE a, NODEPTR_TYPE u) {\n"
	      "%1int c;\n%1struct %Pstate *p;\n\n"
	      "%1if (!a)\n%2PANIC(\"%Plabel\", \"Null tree in\", OP_LABEL(u));\n");
	print("%1STATE_LABEL(a) = p = (struct yystate *)malloc(sizeof *p);\n"
	      "%1memset(p, 0, sizeof *p);\n");
	for (i = 1; i <= ntnumber; i++)
		print("%1p->cost[%d] =\n", i);
	print("%20x7fff;\n%1switch (OP_LABEL(a)) {\n");
	for (p = terms; p; p = p->link)
		emitcase(p, ntnumber);
	print("%1default:\n"
"%2PANIC(\"%Plabel\", \"Bad terminal\", OP_LABEL(a));\n%1}\n");
	if (Tflag == 2) print("%1for (c = 1; c < %d; c++)\n%2if (p->cost[c] < 0x7fff) return;\n%1fprintf(stderr,\"warning: can not match 0x%%x\\n\",(long)a);\n", ntnumber); /* 2.5 */
	print("}\n\n");
}

#ifndef NOMUNCH

/* emitcode - emit the user code actions */
static void emitcode(Rule rules) {
	Rule r;
	print("static int %Pmunch(NODEPTR_TYPE p, int goalnt, int loop);\n");
	print("static int %Pmaximal(NODEPTR_TYPE p, int eruleno)\n{\n");
	print("  short *nts = %Pnts[eruleno];\n");
	print("  NODEPTR_TYPE kids[%d];\n  int i;\n\n", maxkids);
	print("  for (%Pkids(p, eruleno, kids), i = 0; nts[i]; i++)\n");
	print("    if (%Pmunch(kids[i], nts[i], 0) == 0) return 0;\n\n");
        if (Tflag) print("  fprintf(stderr, \"0x%%lx: %%s\\n\", (long)p, %Pstring[eruleno]);\n");
	print("  switch(eruleno) {\n");
	for (r = rules; r; r = r->link) {
		char *tmpl = r->template;
		if (tmpl == 0) tmpl = "";
		print("%1case %d: /* %R */\n", r->ern, r);
		if (nflag == 0) print("#line %d \"%s\"\n", r->lineno, infile);
		print("%s\n%2break;\n", tmpl);
	}
	print("%1default: break;\n  }\n%1return 1;\n}\n\n");
}

/* emitmaximal - emit one case in function state */
static void emitmaximal(Term p, int ntnumber) {
	Rule r;

	print("%1case %d: /* %S */\n", p->esn, p);
	for (r = p->rules; r; r = r->next) {
		switch (p->arity) {
		case 0: case -1:
                        print("%2if (goalnt == %d%1/* %R */\n", r->lhs->number, r);
			if (r->cost == -1) print("%3&& %s < MAX_COST\n", r->code);
                        print("%2) {\n");
                        print("%3if (%Pmaximal(a, %d) == 1) return 1;\n", r->ern);
                        if (Tflag) print("%3fprintf(stderr, \"0x%%lx: %R\\n\", (long)a);\n", r);
			print("%2}\n");
			break;
		case 1:
                        print("%2if (goalnt == %d%1/* %R */\n", r->lhs->number, r);
			if (r->pattern->nterms > 1) {
				print("&& ");
				emittest(r->pattern->left, "LEFT_CHILD(a)", " ");
                        }
			if (r->cost == -1) print("%3&& %s < MAX_COST\n", r->code);
			print("%2) {\n");
                        print("%3if (%Pmaximal(a, %d) == 1) return 1;\n", r->ern);
                        if (Tflag) print("%3fprintf(stderr, \"0x%%lx: %R\\n\", (long)a);\n", r);
			print("%2}\n");
			break;
		case 2:
                        print("%2if (goalnt == %d%1/* %R */\n", r->lhs->number, r);
			if (r->pattern->nterms > 1) {
				print("&& ");
				emittest(r->pattern->left,  "LEFT_CHILD(a)",
					r->pattern->right->nterms ? " && " : " ");
				emittest(r->pattern->right, "RIGHT_CHILD(a)", " ");
			}
			if (r->cost == -1) print("%3&& %s < MAX_COST\n", r->code);
			print("%2) {\n");
                        print("%3if (%Pmaximal(a, %d) == 1) return 1;\n", r->ern);
                        if (Tflag) print("%3fprintf(stderr, \"0x%%lx: %R\\n\", (long)a);\n", r);
			print("%2}\n");
			break;
		default: assert(0);
		}
	}
	print("%2break;\n");
}

/* emitmunch - emit munch function */
static void emitmunch(Term terms, Nonterm start, int ntnumber) {
	int i;
	Term p;

        emitcode(rules);
	print("static int %Pmunch(NODEPTR_TYPE a, int goalnt, int loop) {\n"
	      "%1NODEPTR_TYPE kids[%d];\n"
	      "%1if (!a)\n"
              "%2PANIC(\"%Pmunch\", \"Null in tree for rule\", goalnt);\n", maxkids);
	print("%1switch (OP_LABEL(a)) {\n");
	for (p = terms; p; p = p->link)
		emitmaximal(p, ntnumber);
	print("%1default:\n"
		"%2PANIC(\"%Pmunch\", \"Bad terminal\", OP_LABEL(a));\n%1}\n");
        /* prs */
        if (mflag > 1) {
		Nonterm p;
        	Rule r;
               	for (r = nts->rules; r; r = r->link)
 			if (r->pattern->nterms == 0) {
				print("%1if (goalnt == %d && loop <= %d) if (%Pmunch(a, %d, loop+1)) { /* %R */\n", r->lhs->number, ntnumber, ((Nonterm)r->pattern->op)->number, r);
				if (Tflag) print("%2fprintf(stderr, \"0x%%lx: %R\\n\", (long)a);\n", r);
				print("%2return 1;\n%1}\n");
			}
        }
	if (Tflag) print("%1fprintf(stderr, \"0x%%lx: NO MATCH for %%s to %%s\\n\", (long)a, %Ptermname[OP_LABEL(a)], %Pntname[goalnt]);\n");
	print("%1return 0;\n}\n\n");
}

#endif

/* computents - fill in bp with _nts vector for tree t */
static char *computents(Tree t, char *bp) {
	if (t) {
		Nonterm p = t->op;
		if (p->kind == NONTERM) {
			sprintf(bp, "%s%s_NT, ", prefix, p->name);
			bp += strlen(bp);
                        nkids++;
		} else
			bp = computents(t->right, computents(t->left,  bp));
	}
	return bp;
}

/* emitnts - emit _nts ragged array */
static void emitnts(Rule rules, int nrules) {
	Rule r;
	int i, j, *nts = alloc((nrules + 1)*sizeof *nts);
	char **str = alloc((nrules + 1)*sizeof *str);

	for (i = 0, r = rules; r; r = r->link) {
		char buf[1024];
                nkids = 0;
		*computents(r->pattern, buf) = 0;
                if (maxkids < nkids) maxkids = nkids;
		for (j = 0; str[j] && strcmp(str[j], buf); j++)
			;
		if (str[j] == NULL) {
			print("static short %Pnts_%d[] = { %s0 };\n", j, buf);
			str[j] = strcpy(alloc(strlen(buf) + 1), buf);
		}
		nts[i++] = j;
	}
	print("\nstatic short *%Pnts[] = {\n");
	for (i = j = 0, r = rules; r; r = r->link) {
		for ( ; j < r->ern; j++)
			print("%10,%1/* %d */\n", j);
		print("%1%Pnts_%d,%1/* %d */\n", nts[i++], j++);
	}
	print("};\n\n");
}

/* emitrecord - emit code that tests for a winning match of rule r */
static void emitrecord(char *pre, Rule r, char *c, int cost) {
	if (Tflag)
		print("%s%Ptrace(a, %d, %s + %d, p->cost[%P%S_NT]);\n",
			pre, r->ern, c, cost, r->lhs);
	print("%sif (", pre);
	print("%s + %d < p->cost[%P%S_NT]) {\n"
"%s%1p->cost[%P%S_NT] = %s + %d;\n%s%1p->rule.%P%S = %d;\n",
		c, cost, r->lhs, pre, r->lhs, c, cost, pre, r->lhs,
		r->packed);
	if (r->lhs->chain)
		print("%s%1%Pclosure_%S(a, %s + %d);\n", pre, r->lhs, c, cost);
	print("%s}\n", pre);
}

/* emitrule - emit decoding vectors and _rule */
static void emitrule(Nonterm nts) {
	Nonterm p;

	for (p = nts; p; p = p->link) {
		Rule r;
		print("static short %Pdecode_%S[] = {\n%10,\n", p);
		for (r = p->rules; r; r = r->decode)
			print("%1%d,\n", r->ern);
		print("};\n\n");
	}
	print("static int %Prule(void *state, int goalnt) {\n"
"%1if (goalnt < 1 || goalnt > %d)\n%2PANIC(\"%Prule\", \"Bad goal nonterminal\", goalnt);\n"
"%1if (!state)\n%2return 0;\n%1switch (goalnt) {\n", ntnumber);
	for (p = nts; p; p = p->link)
		print("%1case %P%S_NT:"
"%1return %Pdecode_%S[((struct %Pstate *)state)->rule.%P%S];\n", p, p, p);
	print("%1default:\n%2PANIC(\"%Prule\", \"Bad goal nonterminal\", goalnt);\n%2return 0;\n%1}\n}\n\n");
}

/* emitstring - emit arrays of templates, instruction flags, and rules */
static void emitstring(Rule rules) {
	Rule r;

#if 0
	print("static char *%Ptemplates[] = {\n");
	print("/* 0 */%10,\n");
	for (r = rules; r; r = r->link)
		print("/* %d */%1\"%s\",%1/* %R */\n", r->ern, r->template, r);
	print("};\n");
	print("\nstatic char %Pisinstruction[] = {\n");
	print("/* 0 */%10,\n");
	for (r = rules; r; r = r->link) {
		int len = strlen(r->template);
		print("/* %d */%1%d,%1/* %s */\n", r->ern,
			len >= 2 && r->template[len-2] == '\\' && r->template[len-1] == 'n',
			r->template);
	}
	print("};\n");
#endif
	if (!Tflag) return;
	print("\nstatic YYCONST char *%Pstring[] = {\n");
	print("/* 0 */%10,\n");
	for (r = rules; r; r = r->link)
		print("/* %d */%1\"%R\",\n", r->ern, r);
	print("};\n\n");
}

/* emittrace - emit the trace rotine */
static void emittrace() {
	print("#ifndef TRACE\nstatic void %Ptrace(NODEPTR_TYPE p,"
	      " int eruleno, int cost, int bestcost)\n{\n"
	      "%1int op = OP_LABEL(p);\n%1YYCONST char *tname = "
	      "%Ptermname[op] ? %Ptermname[op] : \"?\";\n"
	      "%1fprintf(stderr, \"0x%%lx:%%s matched %%s with cost "
	      "%%d vs. %%d\\n\", (long)p, tname, "
	      "%Pstring[eruleno], cost, bestcost);\n}\n#endif\n\n");
}

/* emitreduce - emit the reduction actions */
static void emitreduce(Rule rules) {
	Rule r;
	print("static void %Preduce(NODEPTR_TYPE p, int goalnt)\n{\n");
	print("  int eruleno = %Prule(STATE_LABEL(p), goalnt);\n");
	print("  short *nts = %Pnts[eruleno];\n");
	print("  NODEPTR_TYPE kids[%d];\n  int i;\n\n", maxkids);
	print("  for (%Pkids(p, eruleno, kids), i = 0; nts[i]; i++)\n");
	print("    %Preduce(kids[i], nts[i]);\n\n");
	print("  switch(eruleno) {\n");
	for (r = rules; r; r = r->link) {
		char *tmpl = r->template;
		if (tmpl == 0) tmpl = "";
		print("%1case %d: /* %R */\n", r->ern, r);
		if (nflag == 0) print("#line %d \"%s\"\n", r->lineno, infile);
		print("%s\n%2break;\n", tmpl);
	}
	print("%1default: break;\n  }\n}\n\n");
}

/* emitselect - emit the selection entry point */
static void emitselect() {
	if (start) {
                      print("int %Pselect(NODEPTR_TYPE p)\n{\n");
#ifndef NOMUNCH
                if (mflag) {
                      print("%1if (%Pmunch(p,1,0) == 0) {\n");
                      if (Tflag) print("%2fprintf(stderr, \"No match for start symbol (%%s).\\n\", %Pntname[%d]);\n", start->number);
                      print("%2return 1;\n%1}\n"
                            "%1return 0;\n}\n\n");
                } else
#endif
                {
                      print("%1%Plabel(p,p);\n"
                            "%1if (((struct %Pstate *)STATE_LABEL(p))->rule.%P%s == 0) {\n", start->name);
                      if (Tflag) print("%2fprintf(stderr, \"No match for start symbol (%%s).\\n\", %Pntname[%d]);\n", start->number);
                      print("%2return 1;\n%1}\n"
                            "%1%Preduce(p, 1);\n%1return 0;\n}\n\n");
                }
	} else { yywarn("no rules in grammar.\n");
		print("int %Pselect(NODEPTR_TYPE p)\n{\n%1return 1;\n}\n"); }
}

/* emitstruct - emit the definition of the state structure */
static void emitstruct(Nonterm nts, int ntnumber) {
	print("struct %Pstate {\n%1short cost[%d];\n%1struct {\n", ntnumber + 1);
	for ( ; nts; nts = nts->link) {
		int n = 1, m = nts->lhscount;
		while ((m >>= 1) != 0)
			n++;		
		print("%2unsigned int %P%S:%d;\n", nts, n);
	}
	print("%1} rule;\n};\n\n");
}

/* emittest - emit clause for testing a match */
static void emittest(Tree t, char *v, char *suffix) {
	Term p = t->op;

	if (p->kind == TERM) {
		print("%3OP_LABEL(%s) == %d%s/* %S */\n", v, p->esn,
			t->nterms > 1 ? " && " : suffix, p);
		if (t->left)
			emittest(t->left, stringf("LEFT_CHILD(%s)",  v),
				t->right && t->right->nterms ? " && " : suffix);
		if (t->right)
			emittest(t->right, stringf("RIGHT_CHILD(%s)", v), suffix);
	}
}

#ifndef NOJAVA
static char *computejavakids(Tree t, char *v, char *bp, int *ip) {
	Term p = t->op;

	if (p->kind == NONTERM) {
		sprintf(bp, "\t\tkids[%d] = %s;\n", (*ip)++, v);
		bp += strlen(bp);
	} else if (p->arity > 0) {
		bp = computejavakids(t->left, stringf("%s.left()", v), bp, ip);
		if (p->arity == 2)
			bp = computejavakids(t->right, stringf("%s.right()", v), bp, ip);
	}
	return bp;
}
static char *computejavants(Tree t, char *bp) {
	if (t) {
		Nonterm p = t->op;
		if (p->kind == NONTERM) {
			sprintf(bp, "%s_NT, ", p->name);
			bp += strlen(bp);
                        nkids++;
		} else
			bp = computejavants(t->right, computejavants(t->left,  bp));
	}
	return bp;
}

static void emitjavacost(Tree t, char *v) {
	Nonterm p = t->op;

	if (p->kind == TERM) {
		if (t->left)
			emitjavacost(t->left,  stringf("%s.left()",  v));
		if (t->right)
			emitjavacost(t->right, stringf("%s.right()", v));
	} else
		print("((state)%s.state()).cost[%S_NT] + ", v, p);
}
static void emitjavatest(Tree t, char *v, char *suffix) {
	Term p = t->op;

	if (p->kind == TERM) {
		print("%3%s.label() == %d%s// %S\n", v, p->esn,
			t->nterms > 1 ? " && " : suffix, p);
		if (t->left)
			emitjavatest(t->left, stringf("%s.left()",  v),
				t->right && t->right->nterms ? " && " : suffix);
		if (t->right)
			emitjavatest(t->right, stringf("%s.right()", v), suffix);
	}
}

static void emitjavarecord(char *pre, Rule r, char *c, int cost) {
	if (Tflag)
		print("%strace(a, %d, %s + %d, p.cost[%S_NT]);\n",
			pre, r->ern, c, cost, r->lhs);
	print("%sif (", pre);
	print("%s + %d < p.cost[%S_NT]) {\n"
"%s%1p.cost[%S_NT] = (short) %s + %d;\n%s%1p.%S = %d;\n",
		c, cost, r->lhs, pre, r->lhs, c, cost, pre, r->lhs,
		r->packed);
	if (r->lhs->chain)
		print("%s%1closure_%S(a, %s + %d);\n", pre, r->lhs, c, cost);
	print("%s}\n", pre);
}

static void emitjavacase(Term p, int ntnumber) {
	Rule r;

	print("%1case %d: // %S\n", p->esn, p);
	switch (p->arity) {
	case 0: case -1:
		break;
	case 1:
		print("%2label(a.left(),a);\n");
		break;
	case 2:
		print("%2label(a.left(),a);\n");
		print("%2label(a.right(),a);\n");
		break;
	default: assert(0);
	}
	for (r = p->rules; r; r = r->next) {
		char *indent = "\t\t\0";
		switch (p->arity) {
		case 0: case -1:
			print("%2// %R\n", r);
			if (r->cost == -1) {
				print("%2c = %s;\n", r->code);
				emitjavarecord("\t\t", r, "c", 0);
			} else
				emitjavarecord("\t\t", r, r->code, 0);
			break;
		case 1:
			if (r->pattern->nterms > 1) {
				print("%2if (%1// %R\n", r);
				emitjavatest(r->pattern->left, "a.left()", " ");
				print("%2) {\n");
				indent = "\t\t\t";
			} else
				print("%2// %R\n", r);
			print("%sc = ", indent);
			emitjavacost(r->pattern->left, "a.left()");
			print("%s;\n", r->code);
			emitjavarecord(indent, r, "c", 0);
			if (indent[2])
				print("%2}\n");
			break;
		case 2:
			if (r->pattern->nterms > 1) {
				print("%2if (%1// %R\n", r);
				emitjavatest(r->pattern->left,  "a.left()",
					r->pattern->right->nterms ? " && " : " ");
				emitjavatest(r->pattern->right, "a.right()", " ");
				print("%2) {\n");
				indent = "\t\t\t";
			} else
				print("%2// %R\n", r);
			print("%sc = ", indent);
			emitjavacost(r->pattern->left,  "a.left()");
			emitjavacost(r->pattern->right, "a.right()");
			print("%s;\n", r->code);
			emitjavarecord(indent, r, "c", 0);
			if (indent[2])
				print("%2}\n");
			break;
		default: assert(0);
		}
	}
	if (p->rules) print("%2break;\n"); else print("%2return;\n"); /* 2.5 */
	/* print("%2break;\n"); */
}

#ifndef NOMUNCH

/* emitjavacode - emit the user code actions */
static void emitjavacode(Rule rules) {
	Rule r;
	print("private int maximal(%s p, int eruleno)\n{\n", treeClass);
	print("  short[] ntsr = nts[eruleno];\n");
	print("  %s kids[] = new %s[%d];\n  int i;\n\n",
            treeClass, treeClass, maxkids);
	print("  for (kids(p, eruleno, kids), i = 0; ntsr[i] != 0; i++)\n");
	print("    if (munch(kids[i], ntsr[i], 0) == 0) return 0;\n\n");
	/* ObjectUtils.identityToString(obj) or Integer.toHexString(System.identityHashCode(p)) */
        if (Tflag) print("  System.err.println(\"0x\"+Integer.toHexString(System.identityHashCode(p))+\": \"+string[eruleno]);\n");
	print("  switch(eruleno) {\n");
	for (r = rules; r; r = r->link) {
		char *tmpl = r->template;
		if (tmpl == 0) tmpl = "";
		print("%1case %d: /* %R */\n", r->ern, r);
		print("%s\n%2break;\n", tmpl);
	}
	print("%1default: break;\n  }\n%1return 1;\n}\n\n");
}

/* emitjavamaximal - emit one case in function state */
static void emitjavamaximal(Term p, int ntnumber) {
	Rule r;

	print("%1case %d: /* %S */\n", p->esn, p);
	for (r = p->rules; r; r = r->next) {
		char *indent = "\t\t\0";
		switch (p->arity) {
		case 0: case -1:
                        print("%2if (goalnt == %d%1/* %R */\n", r->lhs->number, r);
			if (r->cost == -1) print("%3&& %s < MAX_COST\n", r->code);
                        print("%2) {\n");
                        print("%3if (maximal(a, %d) == 1) return 1;\n", r->ern);
                        if (Tflag) print("%3System.err.println(\"0x\"+Integer.toHexString(System.identityHashCode(a))+\": %R\");\n", r);
			print("%2}\n");
			break;
		case 1:
                        print("%2if (goalnt == %d%1/* %R */\n", r->lhs->number, r);
			if (r->pattern->nterms > 1) {
				print("&& ");
				emitjavatest(r->pattern->left, "a.left()", " ");
                        }
			if (r->cost == -1) print("%3&& %s < MAX_COST\n", r->code);
			print("%2) {\n");
                        print("%3if (maximal(a, %d) == 1) return 1;\n", r->ern);
                        if (Tflag) print("%3System.err.println(\"0x\"+Integer.toHexString(System.identityHashCode(a))+\": %R\");\n", r);
			print("%2}\n");
			break;
		case 2:
                        print("%2if (goalnt == %d%1/* %R */\n", r->lhs->number, r);
			if (r->pattern->nterms > 1) {
				print("&& ");
				emitjavatest(r->pattern->left,  "a.left()",
					r->pattern->right->nterms ? " && " : " ");
				emitjavatest(r->pattern->right, "a.right()", " ");
			}
			if (r->cost == -1) print("%3&& %s < MAX_COST\n", r->code);
			print("%2) {\n");
                        print("%3if (maximal(a, %d) == 1) return 1;\n", r->ern);
                        if (Tflag) print("%3System.err.println(\"0x\"+Integer.toHexString(System.identityHashCode(a))+\": %R\");\n", r);
			print("%2}\n");
			break;
		default: assert(0);
		}
	}
	print("%2break;\n");
}

/* emitjavamunch - emit munch function */
static void emitjavamunch(Term terms, Nonterm start, int ntnumber) {
	int i;
	Term p;

        emitjavacode(rules);
	print("private int munch(%s a, int goalnt, int loop) {\n"
	      "%1%s kids[] = new %s[%d];\n"
	      "%1if (a == null)\n"
              "%2panic(\"munch\", \"Null in tree for rule\", goalnt);\n",
              treeClass, treeClass, treeClass, maxkids);
	print("%1switch (a.label()) {\n");
	for (p = terms; p; p = p->link)
		emitjavamaximal(p, ntnumber);
	print("%1default:\n"
"%2panic(\"munch\", \"Bad terminal\", a.label());\n%1}\n");
        if (mflag > 1) {
		Nonterm p;
        	Rule r;
               	for (r = nts->rules; r; r = r->link)
 			if (r->pattern->nterms == 0) {
				print("%1if (goalnt == %d && loop <= %d) if (munch(a, %d, loop+1) > 0) { /* %R */\n", r->lhs->number, ntnumber, ((Nonterm)r->pattern->op)->number, r);
                        	if (Tflag) print("%2System.err.println(\"0x\"+Integer.toHexString(System.identityHashCode(a))+\": %R\");\n", r);
				print("%2return 1;\n%1}\n");
			}
        }
	if (Tflag) print("%1System.err.println(\"0x\"+Integer.toHexString(System.identityHashCode(a))+\": NO MATCH for \"+termname[a.label()]+\" to \"+ntname[goalnt]);\n");
	print("%1return 0;\n}\n\n");
}

#endif

static void emitjava(void) {
	Nonterm nt;
	Nonterm p;
	Term t;
	Rule r, *rc;
	int i, j, col, *nt_s;
	char **str, *pub = "", *out, *ptr, *cl = YYCLASS;

	/* emitcontext */
	if (pflag) cl = prefix;
	if((out = strrchr(outfile, '/')) == 0) out = outfile;
	if ((ptr = strchr(out, '.')) != 0) *ptr = 0;
	if (strcmp(cl, out) == 0) pub = "public ";
	print("\n%s %s class %s %s {\n\n", pub, final, cl, extends);
	print("public static final String PBURG_PREFIX=\"%s\";\n", prefix);
	print("public static final String PBURG_VERSION=\"%s\";\n", VERSION);
	print("public static final int MAX_COST=0x7fff;\n");

	/* emitheader */
	print("public void panic(String rot, String msg, int val) {\n"
	      "\tSystem.err.println(\"Internal Error in \"+rot+\": \"+"
	      "msg+\" \"+val+\"\\nexiting...\");\n\tSystem.exit(2);\n}\n");

	/* emitdefs(nts, ntnumber) */
	i = 0;
	col = 0;


	if (Tflag) {
		print("private static final String ntname[] = {\n%1\"\",\n");
		for (p = nts; p; p = p->link)
			print("%1\"%S\",\n", p);
		print("%1\"\"\n};\n\n");
	}

	for (p = nts; p; p = p->link)
		print("private static final short %S_NT=%d;\n", p, p->number);
	print("\n");

	print("private static final String termname[] = {\n");
	for (t = terms; t; t = t->link) {
		while (t->esn > i++) {
			if ((col += 4) > 72) col = 4, print("\n");
			print(" \"\",");
		}
		if (col > 0) print("\n");
		print("%1/* %d */ \"%S\",\n", i, t);
		col = 0;
	}
	print("%2\"\"\n};\n\n");

#ifndef NOMUNCH
    if (!mflag)
#endif
    {
	/* emitstruct(nts, ntnumber) */
	print("private class state {\n%1int cost[];\n");
	for (nt = nts ; nt; nt = nt->link)
		print("%1int %S;\n", nt);
	print("%1state() { cost = new int[%d]; }\n", ntnumber + 1);
	print("};\n\n");
    }

	/* emitnts(rules, nrules) */
	nt_s = alloc((nrules + 1)*sizeof *nt_s);
	str = alloc((nrules + 1)*sizeof *str);

	print("private static final short nts_[] = { 0 };\n");
	for (i = 0, r = rules; r; r = r->link) {
		char buf[1024];
                nkids = 0;
		*computejavants(r->pattern, buf) = 0;
                if (maxkids < nkids) maxkids = nkids;
		for (j = 0; str[j] && strcmp(str[j], buf); j++)
			;
		if (str[j] == NULL) {
			print("private static final short nts_%d[] = { %s0 };\n", j, buf);
			str[j] = strcpy(alloc(strlen(buf) + 1), buf);
		}
		nt_s[i++] = j;
	}
	print("\nprivate static final short nts[][] = {\n");
	for (i = j = 0, r = rules; r; r = r->link) {
		for ( ; j < r->ern; j++)
			print("%1nts_,%1/* %d */\n", j);
		print("%1nts_%d,%1/* %d */\n", nt_s[i++], j++);
	}
	print("};\n\n");

	/* emitstring(rules) */
	if (Tflag) {
		print("\nprivate static final String string[] = {\n");
		print("/* 0 */%1\"\",\n");
		for (r = rules; r; r = r->link)
			print("/* %d */%1\"%R\",\n", r->ern, r);
		print("};\n\n");
		/* emittrace */
		print("public void trace(%s p,"
		      " int eruleno, int cost, int bestcost)\n{\n"
		      "%1int op = p.label();\n%1String tname = "
		      "termname[op] != null ? termname[op] : \"?\";\n"
		      "%1System.err.println(p.hashCode()+\":\"+tname+"
		      "\" matched \"+string[eruleno]+\" with cost "
		      "\"+cost+\" vs. \"+bestcost);\n}\n\n", treeClass);
	}

#ifndef NOMUNCH
    if (!mflag)
#endif
    {
	/* emitrule(nts) */
	for (p = nts; p; p = p->link) {
		Rule r;
		print("private static final short decode_%S[] = {\n%10,\n", p);
		for (r = p->rules; r; r = r->decode)
			print("%1%d,\n", r->ern);
		print("};\n\n");
	}
	print("private int rule(state st, int goalnt) {\n"
"%1if (goalnt < 1 || goalnt > %d)\n%2panic(\"rule\", \"Bad goal nonterminal\", goalnt);\n"
"%1if (st == null)\n%2return 0;\n%1switch (goalnt) {\n", ntnumber);
	for (p = nts; p; p = p->link)
		print("%1case %S_NT:"
"%1return decode_%S[st.%S];\n", p, p, p);
	print("%1default:\n%2panic(\"rule\", \"Bad goal nonterminal\", goalnt);\n%2return 0;\n%1}\n}\n\n");

	/* emitclosure(nts) */
	for (p = nts; p; p = p->link)
		if (p->chain) {
			Rule r;
			print("private void closure_%S(%s a, int c) {\n"
"%1state p = (state)a.state();\n", p, treeClass);
			for (r = p->chain; r; r = r->chain)
				emitjavarecord("\t", r, "c", r->cost);
			print("}\n\n");
		}

	if (start) {
		/* emitlabel(terms, start, ntnumber) */
		print("public void label(%s a, %s u) {\n"
		      "%1int c;\n%1state p;\n\n"
		      "%1if (a == null)\n%2panic(\"label\", \"Null tree in\", u.label());\n",
		      treeClass, treeClass);
		print("%1a.state(p = new state());\n");
		for (i = 1; i <= ntnumber; i++)
			print("%1p.cost[%d] =\n", i);
		print("%20x7fff;\n%1switch (a.label()) {\n");
		for (t = terms; t; t = t->link)
			emitjavacase(t, ntnumber);
		/* print("%1default:\n"
	"%2panic(\"label\", \"Bad terminal\", a.label());\n%1}\n}\n\n"); */
	      print("%1default:\n"
"%2PANIC(\"%Plabel\", \"Bad terminal\", a.label(a));\n%1}\n");
	      if (Tflag == 2) print("%1for (c = 1; c < %d; c++)\n%2if (p.cost[c] < 0x7fff) return;\n%1System.err.println(\"warning: can not match 0x\"+Integer.toHexString(System.identityHashCode(a)));\n", ntnumber); /* 2.5 */
	      print("}\n\n");
	}
	if (errcnt) return;
    }
	/* emitkids(rules, nrules) */
	rc = alloc((nrules + 1 + 1)*sizeof *rc);
	str  = alloc((nrules + 1 + 1)*sizeof *str);

	for (i = 0, r = rules; r; r = r->link) {
		int j = 0;
		char buf[1024], *bp = buf;
		*computejavakids(r->pattern, "p", bp, &j) = 0;
		for (j = 0; str[j] && strcmp(str[j], buf); j++)
			;
		if (str[j] == NULL)
			str[j] = strcpy(alloc(strlen(buf) + 1), buf);
		r->kids = rc[j];
		rc[j] = r;
	}
	print("private void kids(%s p, int eruleno, %s kids[]) {\n"
"%1if (p == null)\n%2panic(\"kids\", \"Null tree in rule\", eruleno);\n"
"%1if (kids == null)\n%2panic(\"kids\", \"Null kids in\", p.label());\n"
"%1switch (eruleno) {\n", treeClass, treeClass);
	for (i = 0; (r = rc[i]) != NULL; i++) {
		for ( ; r; r = r->kids)
			print("%1case %d: // %R\n", r->ern, r);
		print("%s%2break;\n", str[i]);
	}
	print("%1default:\n%2panic(\"kids\", \"Bad rule number\", eruleno);\n%1}\n}\n\n");

#ifndef NOMUNCH
    if (!mflag)
#endif
    {
	/* emitreduce(rules) */
	print("public void reduce(%s p, int goalnt) %s{\n", treeClass, throws);
	print("  int eruleno = rule((state)p.state(), goalnt);\n");
	print("  short nt[] = nts[eruleno];\n");
	print("  %s kids[] = new %s[%d];\n  int i;\n\n", treeClass, treeClass, maxkids);
	print("  for (kids(p, eruleno, kids), i = 0; nt[i] != 0; i++)\n");
	print("    reduce(kids[i], nt[i]);\n\n");
	print("  switch(eruleno) {\n");
	for (r = rules; r; r = r->link) {
		char *tmpl = r->template;
		if (tmpl == 0) tmpl = "";
		print("%1case %d: // %R\n// line %d \"%s\"\n%s\n%2break;\n", r->ern, r, r->lineno, infile, tmpl);
	}
	print("%1default: break;\n  }\n}\n\n");
	if (start) {
		print("public boolean select(%s p) %s {\n%1label(p,p);\n"
		      "%1if (((state)p.state()).%s == 0) {\n",
		      treeClass, throws, start->name);
		if (Tflag) print("%2System.err.println(\"No match for start symbol (\"+ntname[%d]+\").\\n\");\n", start->number);
		print("%2return false;\n%1}\n"
		      "%1reduce(p, 1);\n%1return true;\n}\n\n");
	} else { yywarn("no rules in grammar.\n");
		print("public boolean select(%s p)\n{\n%1return false;\n}\n", treeClass); }
    }
#ifndef NOMUNCH
    else {
	if (start) {
		emitjavamunch(terms, start, ntnumber);
		print("public boolean select(%s p) %s {\n", treeClass, throws);
                print("%1if (munch(p,1,0) == 0) {\n");
		if (Tflag) print("%2System.err.println(\"No match for start symbol (\"+ntname[%d]+\").\\n\");\n", start->number);
		print("%2return false;\n%1}\n%1return true;\n}\n\n");
	} else { yywarn("no rules in grammar.\n");
		print("public boolean select(%s p)\n{\n%1return false;\n}\n", treeClass); }
    }
#endif
}
#endif
