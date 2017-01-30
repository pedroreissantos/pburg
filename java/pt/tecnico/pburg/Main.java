package pt.tecnico.pburg;

import java.io.*;
import java.util.*;
import org.antlr.v4.runtime.*;

public class Main {
    private static final String VERSION = "2.5";
    private static final String RCSID =
	"$Id: Main.java,v "+VERSION+" 2017/01/30 15:49:42 prs Exp $";
    private static String prefix = "Selector", treeClass = "Tree";
    private static String infile = "<stdin>", outfile;
    private static boolean Aflag, nflag, pflag;
    private static String _extends = "", _throws = "", _final = "";
    private static int Tflag, mflag, errcnt, nkids, maxkids, ntnumber, nrules;
    private static String startname = "", decl = "", code = "";
    private static Nonterm start = null;
    private static Term terms = null;
    private static Nonterm nts = null;
    private static Rule rules = null;
    private static Map<String, Name> ids = new HashMap<String, Name>();

    private static InputStream in;
    private static PrintWriter out;

    public static void main(String[] args) throws Exception {
	arguments(args);
	out.println("/*\ngenerated at "+new Date()+" "+new Date()
		    +"\nby "+RCSID+"\n*/");
	parse(in);
	System.err.println("*** PARSING ENDED ***");
	if (start != null) ckreach(start);
	for (Nonterm nt = nts; nt != null; nt = nt.link())
	    if (nt.rules() == null)
		error(nt.name()+": undefined nonterminal.");
	    else if (!nt.reached())
		error(nt.name()+": can't reach nonterminal.");

    	print(); //debug
	emit();
	out.print(code+"\n}\n");
	out.close(); // out.flush();
    }

    public static void warn(String msg) {
        System.err.println(msg);
    }

    public static void error(String msg) {
	warn(msg);
	errcnt++;
    }

    public static void arguments(String[] args) throws Exception {
        // for (String s: args)
	String s;
	for (int i = 0; i < args.length; i++)
	    if ((s = args[i]).equals("-T")) Tflag++;
	    else if (s.equals("-p") && i+1 < args.length)
	    	    prefix = args[++i];
	    else if (s.length() > 8 && s.substring(0,8).equals("-Jclass="))
		    prefix = s.substring(8);
	    else if (s.equals("-Jfinal"))
		    _final = "final";
	    else if (s.length() > 10 && s.substring(0,10).equals("-Jextends="))
		    _extends = s.substring(10);
	    else if (s.length() > 7 && s.substring(0,7).equals("-Jtree="))
		    treeClass = s.substring(7);
	    else if (s.length() > 9 && s.substring(0,9).equals("-Jthrows="))
		    _throws = s.substring(9);
	    else if (s.equals("-m")) mflag++;
	    else if (s.equals("-n")) nflag = true;
	    else if (s.equals("-A")) Aflag = true;
	    else if (s.equals("-v")) {
		    System.err.println(RCSID);
		    System.exit(1); }
	    else if (s.charAt(0) == '-' && s.length() > 1) {
		    error("usage: pburg [-T | -A | -n | -v | -m"
			  + " | -J[option] ]... [ [ input ] output ] \n"
			  + "\t-Jclass=name (of the class to generate, if other than '"+prefix+"')\n"
			  + "\t-Jfinal (make the generated class 'final')\n"
			  + "\t-Jextends=declaration (full declaration of extends and implements)\n"
			  + "\t-Jtree=name (tree class name, if other than '"+treeClass+"')\n"
			  + "\t-Jthrows=declaration (full declaration of reduce excpetions)\n");
		    System.exit(1); }
	    else if (in == null)
		if (s.equals("-"))
		    in = System.in;
		else
		    in = new FileInputStream(infile = s);
	    else if (out == null)
		if (s.equals("-"))
		    out = new PrintWriter(System.out);
		else
		    out = new PrintWriter(outfile = s, "UTF-8");
	if (out == null)
	    out = new PrintWriter(outfile = prefix + ".java", "UTF-8");
	if (in == null) in = System.in;
    }

    private static void parse(InputStream in)  throws Exception {
        scan l = new scan(new ANTLRInputStream(in));
        gram p = new gram(new CommonTokenStream(l));
        p.addErrorListener(new BaseErrorListener() {
            @Override
            public void syntaxError(Recognizer<?, ?> recognizer, Object offendingSymbol, int line, int charPositionInLine, String msg, RecognitionException e) {
                throw new IllegalStateException("failed to parse at line " + line + " due to " + msg, e);
            }
        });

        p.addParseListener(new gramBaseListener() {

            @Override
            public void exitPrule(gram.PruleContext ctx) {
// prule	: ID CL ptree pact? NL | ID CL ptree ID pact? NL | ID CL ptree INT pact? NL | NL ;

		if (ctx.ID(0) == null) return; // prule: NL;
		String cost = "0", act = "<no action>";
                if (ctx.ID(1) != null) cost = ctx.ID(1).getText();
                if (ctx.INT() != null) cost = ctx.INT().getText();
                if (ctx.pact() != null) act = ctx.pact().ACTION().getText();
                // System.out.println("nonterm( "+ctx.ID(0) +");");
                nonterm(ctx.ID(0).getText());
                rule(ctx.ID(0).getText(), tree(ctx.ptree()), act, cost, ctx.ID(0).getSymbol().getLine());
            }

	    private Pattern tree(gram.PtreeContext ctx) {
		if (ctx.ptree(0) == null)
		    return Main.tree(ctx.ID().getText(), null, null);
		if (ctx.ptree(1) == null)
		    return Main.tree(ctx.ID().getText(), tree(ctx.ptree(0)), null);
		return Main.tree(ctx.ID().getText(), tree(ctx.ptree(0)), tree(ctx.ptree(1)));
	    }

            @Override
            public void exitPlist(gram.PlistContext ctx) {
// plist	: IDENT EQ INTEG | IDENT EQ CHAR ;
		int i = 0;
		if (ctx.CHAR() != null)
		    i = ((int)ctx.CHAR().getText().charAt(1));
		else
		    i = Integer.parseInt(ctx.INTEG().getText());
		// System.out.println("term(" + ctx.IDENT().getText()+", "+ i);
		term(ctx.IDENT().getText(), i);
            }

	    private void include(String file) {
		int lineno = 0, i;
		String s = file.substring(1,file.length()-1);
		try {
		    BufferedReader in = new BufferedReader(new FileReader(s));
// BufferedReader in = new BufferedReader(new InputStreamReader(new FileInputStream(s)));
		    while ((s = in.readLine()) != null) {
			String line = new String(s.getBytes(),"UTF-8");
			lineno++;
			for (i = line.length()-1; i >= 0; i--)
			    if (line.charAt(i) == '=')
				break;
			term(line.substring(0, i), Integer.parseInt(line.substring(i+1)));
		    }
		} catch (FileNotFoundException e) {
		    System.out.println("Erro (file not found):"+file+": line "+lineno+": "+e);
		} catch (IOException e) {
		    System.out.println("Erro (IO):"+file+":"+lineno+": line "+e);
		}
	    }

            @Override
            public void exitPdecl(gram.PdeclContext ctx) {
// pdecl	: TOKEN plist* | START IDENT | INCLUDE STR | DECL ;
                // ctx.TERM() is processed in plist.
		if (ctx.START() != null)
		  startname = ctx.IDENT().getText();
		if (ctx.INCLUDE() != null)
		  include(ctx.STR().getText());
		if (ctx.DECL() != null) {
		  int len = ctx.DECL().getText().length();
		  // System.out.println("\tDECL: "+ctx.DECL().getText().substring(2, len-4));
		  decl += ctx.DECL().getText().substring(2, len-3);
		}
            }

            @Override
            public void exitPcode(gram.PcodeContext ctx) {
// pcode	: SEP2 END* ;
                code = ctx.getText().substring(2);
            }

        });
        p.pburg();
    }

    /* reach - mark all nonterminals in tree t as reachable */
    private static void reach(Pattern t) {
	    Name p = t.op();

	    if (!p.term())
		    if (!((Nonterm)p).reached())
			    ckreach((Nonterm)p);
	    if (t.left() != null)
		    reach(t.left());
	    if (t.right() != null)
		    reach(t.right());
    }

    /* ckreach - mark all nonterminals reachable from p */
    private static void ckreach(Nonterm p) {
	p.reach();
	for (Rule r = p.rules(); r != null; r = r.decode())
	    reach(r.pattern());
    }

    /* nonterm - create a new terminal id, if necessary */
    public static Nonterm nonterm(String id) {
	Name n = ids.get(id);
	Nonterm p, q, prev;
	int i;

	if (n != null) {
	    if (!n.term()) return (Nonterm)n;
	    error(id+": is a terminal");
	    return null;
	}
	p = new Nonterm(id, ++ntnumber);
	ids.put(id, p);
	if (p.number() == 1) start = p;
	for (prev = null, q = nts;
	     q != null && q.number() < p.number();
	     prev = q, q = q.link())
	    ;
	p.link(q);
	if (prev != null) prev.link(p); else nts = p;
	return p;
    }

    /* term - create a new terminal id with external symbol number esn */
    public static Term term(String id, int esn) {
	Name n = ids.get(id);
	Term p = null, q, prev;
	int i, j;

	if (n != null) {
		error(id+": redefinition of terminal (ignored)");
		return p;
	}

	p = new Term(id, esn);
	ids.put(id, p);
	for (prev = null, q = terms;
	     q != null && q.esn() < p.esn();
	     prev = q, q = q.link())
	    ;
	if (q != null && q.esn() == p.esn() && q.esn() != -1)
		error("duplicate external symbol number `"+
			p.name()+"="+p.name()+"'\n");
	p.link(q);
	if (prev != null) prev.link(p); else terms = p;
	return p;
    }

    /* tree - create & initialize a tree node with the given fields */
    public static Pattern tree(String id, Pattern left, Pattern right) {
	Pattern t;
	Name p = ids.get(id);
	int arity = 0, nterms;

	if (left != null && right != null)
		arity = 2;
	else if (left != null)
		arity = 1;
	if (p == null && arity > 0) {
		error(id+":undefined terminal");
		p = term(id, -1);
	} else if (p == null && arity == 0)
		p = nonterm(id);
	else if (p != null && !p.term() && arity > 0) {
		error(id+": is a nonterminal");
		p = term(id, -1);
	}
	if (p.term() && ((Term)p).arity() == -1)
		((Term)p).arity(arity);
	if (p.term() && arity != ((Term)p).arity())
		error(id+": inconsistent arity for terminal");
	t = new Pattern(p, left, right);
	nterms = p.term() ? 1 : 0;
	if (t.left() != null) nterms += left.nterms();
	if (t.right() != null) nterms += right.nterms();
	t.nterms(nterms);
	return t;
    }

/* NOMUNCH comparetreeterms - compare the location of terminals in two trees */
    private static int comparetreeterms(Pattern a, Pattern b) {
	Name ta = a.op(), tb = b.op();

	if (ta.term() != tb.term()) return 0; /* different kind */
	if (!ta.term()) return 1; /* both are non-terminals */
	if (((Term)ta).esn() != ((Term)tb).esn()) return 0; /* different terminals */
	if (a.left() != null && b.left() != null)
	    if (comparetreeterms(a.left(), b.left()) == 0) return 0;
	if (a.right() != null && b.right() != null)
	    if (comparetreeterms(a.right(), b.right()) == 0) return 0;
	return 1;
    }


    /* rule - create & initialize a rule with the given fields */
    public static Rule rule(String id, Pattern pattern, String template, String code, int line) {
	Rule r, q, ant, rq;
	Name p = pattern.op();
	String end;

	if (mflag > 0) {
	    if (pattern.nterms() == 0)
		error("maximal-munch requires a terminal in the rule");
	    else
		for (Term tm = terms; tm != null; tm = tm.link())
		    if (tm.esn() == ((Term)p).esn())
			for (Rule rl = tm.rules(); rl != null; rl = rl.next())
			    if (comparetreeterms(pattern, rl.pattern()) != 0)
				error("maximal-munch requires different terminal patterns from rule in line "+ rl.line());
	}

	r = new Rule(id, pattern, template, code, line);
	r.lhs(nonterm(id));
	r.lhs().lhsincr();
	r.packed(r.lhs().lhscount());
	r.ern(++nrules);
	if (r.lhs().rules() == null) r.lhs().rules(r);
	else {
	    for (ant = null, q = r.lhs().rules(); q != null; ant = q, q = q.decode())
		;
	    if (ant != null) ant.decode(r); else r.lhs().rules(r);
	}
	if (p.term()) {
	    Term t = (Term)p;
	    if (mflag > 0) {
		for (ant = null, rq = t.rules();
		       rq != null && r.pattern().nterms() <= rq.pattern().nterms() ;
		       ant = rq, rq = rq.next()) ;
			r.next(rq);
		if (ant != null) ant.next(r); else t.rules(r);
	    } else {
		for (ant = null, q = t.rules(); q != null; ant = q, q = q.next())
			;
		if (ant != null) ant.next(r); else t.rules(r);
	    }
	} else if (pattern.left() == null && pattern.right() == null) {
	    Nonterm nt = (Nonterm)pattern.op();
	    r.chain(nt.chain());
	    nt.chain(r);
	    if (r.cost() == -1)
		    error(code +": illegal nonconstant cost");
	}
	for (ant = null, q = rules; q != null; ant = q, q = q.link())
		;
	r.link(q);
	if (ant != null) ant.link(r); else rules = r;
	return r;
    }

    public static void print() {
	System.out.print("IDS:\t");
	for (String s: ids.keySet()) System.out.print(" "+s);
	System.out.print("\nTERMS:");
	for (Term t = terms; t != null; t = t.link())
	  System.out.print("\t"+t);
	System.out.print("\nNTS:");
	for (Nonterm p = nts; p != null; p = p.link())
	  System.out.print("\t"+p);
	System.out.print("\nRULES:");
	for (Rule r = rules; r != null; r = r.link())
	  System.out.print("\n\t"+r);
	System.out.println("\nEND");
    }

    public static void emit() { // for emitjava
	Rule r;
	Term t;
	Nonterm p;
	int i, j, col, nt_s[];
	String str[], buf = "", pub = "", cl = prefix;

	/* emitcontext */
	if ((j = outfile.lastIndexOf('/')) == 0) j = -1;
	if ((i = outfile.lastIndexOf('.')) == 0) i = outfile.length();
	if (j < i && cl.equals(outfile.substring(j+1,i))) pub = "public ";
	out.print(decl +
	      "\n" + pub + " " + _final + " class "+ cl +" "+ _extends +" {\n\n"
	    + "public static final String PBURG_PREFIX=\""+ prefix +"\";\n"
	    + "public static final String PBURG_VERSION=\""+ VERSION +"\";\n"
	    + "public static final int MAX_COST=0x7fff;\n");

	/* emitheader */
	out.print("public void panic(String rot, String msg, int val) {\n"
	    + "\tSystem.err.println(\"Internal Error in \"+rot+\": \"+"
	    + "msg+\" \"+val+\"\\nexiting...\");\n\tSystem.exit(2);\n}\n");

	/* emitdefs(nts, ntnumber) */
	i = 0;
	col = 0;
	if (Tflag > 0) {
		out.print("private static final String[] ntname = {\n\t\"\",\n");
		for (p = nts; p != null; p = p.link())
			out.print("\t\""+p+"\",\n");
		out.print("\t\"\"\n};\n\n");
	}

	for (p = nts; p != null; p = p.link())
		out.print("private static final short "+p+"_NT="+p.number()+";\n");
	out.print("\n");

	out.print("private static final String termname[] = {\n");
	for (t = terms; t != null; t = t.link()) {
		while (t.esn() > i++) {
			if ((col += 4) > 72) { col = 4; out.print("\n"); }
			out.print(" \"\",");
		}
		if (col > 0) out.print("\n");
		out.print("\t/* "+i+" */ \""+t+"\",\n");
		col = 0;
	}
	out.print("\t\t\"\"\n};\n\n");

	if (mflag == 0) {
	    /* emitstruct(nts, ntnumber) */
	    out.print("private class state {\n\tint cost[];\n");
	    for (p = nts; p != null; p = p.link())
		    out.print("\tint "+p+";\n");
	    out.print("\tstate() { cost = new int["+(ntnumber+1)+"]; }\n");
	    out.print("};\n\n");
	}

	/* emitnts(rules, nrules) */
	nt_s = new int[nrules+2];
	str = new String[nrules+2];
	// for (i = 0; i < nrules+2; i++) str[i] = null;

	out.print("private static final short nts_[] = { 0 };\n");
	for (i = 0, r = rules; r != null; r = r.link()) {
                nkids = 0;
		buf = computents(r.pattern());
                if (maxkids < nkids) maxkids = nkids;
		for (j = 0; str[j] != null && !str[j].equals(buf); j++)
			;
		if (str[j] == null) {
			out.print("private static final short nts_"+j+"[] = { "+buf+"0 };\n");
			str[j] = buf;
		}
		nt_s[i++] = j;
	}
	out.print("\nprivate static final short nts[][] = {\n");
	for (i = j = 0, r = rules; r != null; r = r.link()) {
		for ( ; j < r.ern(); j++)
			out.print("\tnts_,\t/* "+j+" */\n");
		out.print("\tnts_"+nt_s[i++]+",\t/* "+j+++" */\n");
	}
	out.print("};\n\n");

	/* emitstring(rules) */
	if (Tflag > 0) {
		out.print("\nprivate static final String[] string = {\n");
		out.print("/* 0 */\t\"\",\n");
		for (r = rules; r != null; r = r.link())
			out.print("/* " + r.ern() + " */\t\""+r+"\",\n");
		out.print("};\n\n");
		/* emittrace */
		out.print("public void trace(" + treeClass + " p,"
		    + " int eruleno, int cost, int bestcost)\n{\n"
		    + "\tint op = p.label();\n\tString tname = "
		    + "termname[op] != null ? termname[op] : \"?\";\n"
		    + "\tSystem.err.println(p.hashCode()+\":\"+tname+"
		    + "\" matched \"+string[eruleno]+\" with cost "
		    + "\"+cost+\" vs. \"+bestcost);\n}\n\n");
	}

      if (mflag == 0) {
	/* emitrule(nts) */
	for (p = nts; p != null; p = p.link()) {
		out.print("private static final short decode_"+p+"[] = {\n\t0,\n");
		for (r = p.rules(); r != null; r = r.decode())
			out.print("\t"+r.ern()+",\n");
		out.print("};\n\n");
	}
	out.print("private int rule(state st, int goalnt) {\n"
		+ "\tif (goalnt < 1 || goalnt > "+ntnumber+")\n"
		+ "\t\tpanic(\"rule\", \"Bad goal nonterminal\", goalnt);\n"
		+ "\tif (st == null)\n\t\treturn 0;\n\tswitch (goalnt) {\n");
	for (p = nts; p != null; p = p.link())
		out.print("\tcase "+p+"_NT:"
			+ "\treturn decode_"+p+"[st."+p+"];\n");
	out.print("\tdefault:\n\t\tpanic(\"rule\", \"Bad goal nonterminal\", goalnt);\n\t\treturn 0;\n\t}\n}\n\n");

	/* emitclosure(nts) */
	for (p = nts; p != null; p = p.link())
		if (p.chain() != null) {
			out.print("private void closure_"+p+"(" + treeClass
			+ " a, int c) {\n\tstate p = (state)a.state();\n");
			for (r = p.chain(); r != null; r = r.chain())
				emitrecord("\t", r, "c", r.cost());
			out.print("}\n\n");
		}

	if (start != null) {
		/* emitlabel(terms, start, ntnumber) */
		out.print("public void label("+ treeClass +" a, "+ treeClass +" u) {\n"
		      + "\tint c;\n\tstate p;\n\n"
		      + "\tif (a == null)\n\t\tpanic(\"label\", \"Null tree in\", u.label());\n");
		out.print("\ta.state(p = new state());\n");
		for (i = 1; i <= ntnumber; i++)
			out.print("\tp.cost[" + i + "] =\n");
		out.print("\t\t0x7fff;\n\tswitch (a.label()) {\n");
		for (t = terms; t != null; t = t.link())
			emitcase(t, ntnumber);
		/* out.print("\tdefault:\n"
	"\t\tpanic(\"label\", \"Bad terminal\", a.label());\n\t}\n}\n\n"); */
	      out.print("\tdefault:\n\t\tpanic(\"label\", \"Bad terminal\", a.label());\n\t}\n");
	      if (Tflag == 2) out.print("\tfor (c = 1; c < "+ntnumber+"d; c++)\n\t\tif (p.cost[c] < 0x7fff) return;\n\tSystem.err.println(\"warning: can not match 0x\"+Integer.toHexString(System.identityHashCode(a)));\n"); /* 2.5 */
	      out.print("}\n\n");
	}
	// if (errcnt) return;
      }

      /* emitkids(rules, nrules) */
      Rule[] rc = new Rule[nrules+2];
      for (i = 0; i < nrules+2; i++) str[i] = null;

      for (i = 0, r = rules; r != null; r = r.link()) {
	  Ptr idx = new Ptr();
	  buf = computekids(r.pattern(), "p", idx);
	  for (j = 0; str[j] != null && !str[j].equals(buf); j++)
		  ;
	  if (str[j] == null)
		  str[j] = buf;
	  r.kids(rc[j]);
	  rc[j] = r;
      }
      out.print("private void kids("+ treeClass +" p, int eruleno, "+ treeClass +" kids[]) {\n\tif (p == null)\n\t\tpanic(\"kids\", \"Null tree in rule\", eruleno);\n\tif (kids == null)\n\t\tpanic(\"kids\", \"Null kids in\", p.label());\n\tswitch (eruleno) {\n");
      for (i = 0; (r = rc[i]) != null; i++) {
	  for ( ; r != null; r = r.kids())
	      out.print("\tcase "+r.ern()+": // "+r+"\n");
	  out.print(str[i]+"\t\tbreak;\n");
      }
      out.print("\tdefault:\n\t\tpanic(\"kids\", \"Bad rule number\", eruleno);\n\t}\n}\n\n");

      if (mflag == 0) {
	  /* emitreduce(rules) */
	  out.print("public void reduce("+treeClass+" p, int goalnt) "+_throws+"{\n");
	  out.print("  int eruleno = rule((state)p.state(), goalnt);\n");
	  out.print("  short nt[] = nts[eruleno];\n");
	  out.print("  "+treeClass+" kids[] = new "+treeClass+"["+maxkids+"];\n  int i;\n\n");
	  out.print("  for (kids(p, eruleno, kids), i = 0; nt[i] != 0; i++)\n");
	  out.print("    reduce(kids[i], nt[i]);\n\n");
	  out.print("  switch(eruleno) {\n");
	  for (r = rules; r != null; r = r.link()) {
	      String tmpl = r.template();
	      if (tmpl == null) tmpl = "";
	      out.print("\tcase "+r.ern()+": // "+r+"\n// line "+r.line()+" \""+infile+"\"\n"+tmpl+"\n\t\tbreak;\n");
	  }
	  out.print("\tdefault: break;\n  }\n}\n\n");
	  if (start != null) {
	      out.print("public boolean select("+treeClass+" p) "+_throws+" {\n\tlabel(p,p);\n\tif (((state)p.state())."+start.name()+" == 0) {\n");
	      if (Tflag > 0) out.print("\t\tSystem.err.println(\"No match for start symbol (\"+ntname["+start.number()+"]+\").\\n\");\n");
	      out.print("\t\treturn false;\n\t}\n\treduce(p, 1);\n\treturn true;\n}\n\n");
	  } else { warn("no rules in grammar.\n");
	      out.print("public boolean select("+treeClass+" p)\n{\n\treturn false;\n}\n"); }
      } else {
	  if (start != null) {
	      emitmunch(start, ntnumber);
	      out.print("public boolean select("+treeClass+" p) "+_throws+" {\n");
	      out.print("\tif (munch(p,1,0) == 0) {\n");
	      if (Tflag > 0) out.print("\t\tSystem.err.println(\"No match for start symbol (\"+ntname["+start.number()+"]+\").\\n\");\n");
	      out.print("\t\treturn false;\n\t}\n\treturn true;\n}\n\n");
	  } else { warn("no rules in grammar.\n");
	      out.print("public boolean select("+treeClass+" p)\n{\n\treturn false;\n}\n"); }
      }
    }

    private static String computekids(Pattern t, String v, Ptr ip) {
	Name p = t.op();
	String bp = "";

	if (!p.term()) {
		bp = "\t\tkids["+ip.get()+"] = "+v+";\n";
		ip.incr();
	} else if (((Term)p).arity() > 0) {
		bp = computekids(t.left(), v+".left()", ip);
		if (((Term)p).arity() == 2)
			bp += computekids(t.right(), v+".right()", ip);
	}
	return bp;
    }

    private static String computents(Pattern t) {
	if (t != null) {
		if (!t.op().term()) {
			Nonterm p = (Nonterm)t.op();
                        nkids++;
			return p.name()+"_NT, ";
		} else {
			return computents(t.left()) + computents(t.right());
		}
	}
	return "";
    }

    private static void emitcost(Pattern t, String v) {
	Name p = t.op();

	if (p.term()) {
		if (t.left() != null)
			emitcost(t.left(),  v+".left()");
		if (t.right() != null)
			emitcost(t.right(), v+".right()");
	} else
		out.print("((state)"+v+".state()).cost["+p+"_NT] + ");
    }

    private static void emittest(Pattern t, String v, String suffix) {
	Name p = t.op();

	if (p.term()) {
		out.print("\t\t\t"+v+".label() == "+((Term)p).esn()
			+(t.nterms() > 1 ? " && " : suffix)+"// "+p+"\n");
		if (t.left() != null)
			emittest(t.left(), v+".left()",
			      t.right() != null && t.right().nterms() != 0 ? " && " : suffix);
		if (t.right() != null)
			emittest(t.right(), v+".right()", suffix);
	}
    }

    private static void emitrecord(String pre, Rule r, String c, int cost) {
	if (Tflag > 0)
		out.print(pre+"trace(a, "+r.ern()+", "+c+" + "+cost+", p.cost["+r.lhs()+"_NT]);\n");
	out.print(pre+"if (");
	out.print(c+" + "+cost+" < p.cost["+r.lhs()+"_NT]) {\n"+pre+"\tp.cost["+r.lhs()+"_NT] = (short) "+c+" + "+cost+";\n"+pre+"\tp."+r.lhs()+" = "+r.packed()+";\n");
	if (r.lhs().chain() != null)
		out.print(pre+"\tclosure_"+r.lhs()+"(a, "+c+" + "+cost+");\n");
	out.print(pre+"}\n");
    }

    private static void emitcase(Term p, int ntnumber) { // for emitcase
	out.print("\tcase "+p.esn()+": // "+p+"\n");
	switch (p.arity()) {
	    case 0: case -1:
		break;
	    case 1:
		out.print("\t\tlabel(a.left(),a);\n");
		break;
	    case 2:
		out.print("\t\tlabel(a.left(),a);\n");
		out.print("\t\tlabel(a.right(),a);\n");
		break;
	    default: error("arity: emitcase: internal error");
	}
	for (Rule r = p.rules(); r != null; r = r.next()) {
	    String indent = "\t\t";
	    switch (p.arity()) {
		case 0: case -1:
		    out.print("\t\t// "+r+"\n");
		    if (r.cost() == -1) {
			    out.print("\t\tc = "+r.code()+";\n");
			    emitrecord("\t\t", r, "c", 0);
		    } else
			    emitrecord("\t\t", r, r.code(), 0);
		    break;
		case 1:
		    if (r.pattern().nterms() > 1) {
			    out.print("\t\tif (\t// "+r+"\n");
			    emittest(r.pattern().left(), "a.left()", " ");
			    out.print("\t\t) {\n");
			    indent = "\t\t\t";
		    } else
			    out.print("\t\t// "+r+"\n");
		    out.print(indent+"c = ");
		    emitcost(r.pattern().left(), "a.left()");
		    out.print(r.code()+";\n");
		    emitrecord(indent, r, "c", 0);
		    if (indent.length() > 2)
			    out.print("\t\t}\n");
		    break;
		case 2:
		    if (r.pattern().nterms() > 1) {
			    out.print("\t\tif (\t// "+r+"\n");
			    emittest(r.pattern().left(),  "a.left()",
				    r.pattern().right().nterms() != 0 ? " && " : " ");
			    emittest(r.pattern().right(), "a.right()", " ");
			    out.print("\t\t) {\n");
			    indent = "\t\t\t";
		    } else
			    out.print("\t\t// "+r+"\n");
		    out.print(indent+"c = ");
		    emitcost(r.pattern().left(),  "a.left()");
		    emitcost(r.pattern().right(), "a.right()");
		    out.print(r.code()+";\n");
		    emitrecord(indent, r, "c", 0);
		    if (indent.length() > 2)
			    out.print("\t\t}\n");
		    break;
		default: error("arity: emitcase: internal error");
	    }
	}
	if (p.rules() != null) out.print("\t\tbreak;\n"); else out.print("\t\treturn;\n"); /* 2.5 */
    }

    /* emitcode - emit the user code actions */
    private static void emitcode(Rule rules) { // for emitcode
	out.print("private int maximal("+treeClass+" p, int eruleno)\n{\n");
	out.print("  short[] ntsr = nts[eruleno];\n");
	out.print("  "+treeClass+" kids[] = new "+treeClass+"["+maxkids+"];\n  int i;\n\n");
	out.print("  for (kids(p, eruleno, kids), i = 0; ntsr[i] != 0; i++)\n");
	out.print("    if (munch(kids[i], ntsr[i], 0) == 0) return 0;\n\n");
	/* ObjectUtils.identityToString(obj) or Integer.toHexString(System.identityHashCode(p)) */
        if (Tflag > 0) out.print("  System.err.println(\"0x\"+Integer.toHexString(System.identityHashCode(p))+\": \"+string[eruleno]);\n");
	out.print("  switch(eruleno) {\n");
	for (Rule r = rules; r != null; r = r.link()) {
		String tmpl = r.template();
		if (tmpl == null) tmpl = "";
		out.print("\tcase "+r.ern()+": /* "+r+" */\n");
		out.print(tmpl+"\n\t\tbreak;\n");
	}
	out.print("\tdefault: break;\n  }\n\treturn 1;\n}\n\n");
    }

    /* emitmaximal - emit one case in function state */
    private static void emitmaximal(Term p, int ntnumber) { // for emitmaximal
	out.print("\tcase "+p.esn()+": /* "+p+" */\n");
	for (Rule r = p.rules(); r != null; r = r.next()) {
	    String indent = "\t\t";
	    switch (p.arity()) {
	      case 0: case -1:
		out.print("\t\tif (goalnt == "+r.lhs().number()+"\t/* "+r+" */\n");
		if (r.cost() == -1) out.print("\t\t\t&& "+r.code()+" < MAX_COST\n");
		out.print("\t\t) {\n");
		out.print("\t\t\tif (maximal(a, "+r.ern()+") == 1) return 1;\n");
		if (Tflag > 0) out.print("\t\t\tSystem.err.println(\"0x\"+Integer.toHexString(System.identityHashCode(a))+\": "+r+"\");\n");
		out.print("\t\t}\n");
		break;
	      case 1:
		out.print("\t\tif (goalnt == "+r.lhs().number()+"\t/* "+r+" */\n");
		if (r.pattern().nterms() > 1) {
		    out.print("&& ");
		    emittest(r.pattern().left(), "a.left()", " ");
		}
		if (r.cost() == -1) out.print("\t\t\t&& "+r.code()+" < MAX_COST\n");
		out.print("\t\t) {\n");
		out.print("\t\t\tif (maximal(a, "+r.ern()+") == 1) return 1;\n");
		if (Tflag > 0) out.print("\t\t\tSystem.err.println(\"0x\"+Integer.toHexString(System.identityHashCode(a))+\": "+r+"\");\n");
		out.print("\t\t}\n");
		break;
	      case 2:
		out.print("\t\tif (goalnt == "+r.lhs().number()+"\t/* "+r+
			" */\n");
		if (r.pattern().nterms() > 1) {
		    out.print("&& ");
		    emittest(r.pattern().left(),  "a.left()",
			r.pattern().right().nterms() != 0 ? " && " : " ");
		    emittest(r.pattern().right(), "a.right()", " ");
		}
		if (r.cost() == -1) out.print("\t\t\t&& "+r.code()+" < MAX_COST\n");
		out.print("\t\t) {\n");
		out.print("\t\t\tif (maximal(a, "+r.ern()+
		    ") == 1) return 1;\n");
		if (Tflag > 0) out.print("\t\t\tSystem.err.println(\"0x\""+
		    "+Integer.toHexString(System.identityHashCode(a))+\": "
		    +r+"\");\n");
		out.print("\t\t}\n");
		break;
	      default: error("arity: emitcase: internal error");
	    }
	}
	out.print("\t\tbreak;\n");
    }

    /* emitmunch - emit munch function */
    private static void emitmunch(Nonterm start, int ntnumber) { // for munch
	int i;

        emitcode(rules);
	out.print("private int munch("+treeClass+" a, int goalnt, int loop) {\n"
	    + "\t"+treeClass+" kids[] = new "+treeClass+"["+maxkids+"];\n"
	    + "\tif (a == null)\n"
            + "\t\tpanic(\"munch\", \"Null in tree for rule\", goalnt);\n");
	out.print("\tswitch (a.label()) {\n");
	for (Term t = terms; t != null; t = t.link())
	    emitmaximal(t, ntnumber);
	out.print("\tdefault:\n\t\tpanic(\"munch\", \"Bad terminal\", "+
		"a.label());\n\t}\n");
        if (mflag > 1) {
	    for (Rule r = nts.rules(); r != null; r = r.link())
		if (r.pattern().nterms() == 0) {
		    out.print("\tif (goalnt == "+r.lhs().number()+
		    	" && loop <= "+ntnumber+") if (munch(a, "+
			((Nonterm)r.pattern().op()).number()+
			", loop+1) > 0) { /* "+r+" */\n");
		    if (Tflag > 0) out.print("\t\tSystem.err.println(\"0x\"+"+
		    	"Integer.toHexString(System.identityHashCode(a))+\": "+
			r+"\");\n");
		    out.print("\t\treturn 1;\n\t}\n");
		}
        }
	if (Tflag > 0) out.print("\tSystem.err.println(\"0x\"+Integer."+
		"toHexString(System.identityHashCode(a))+\": NO MATCH for"+
		"\"+termname[a.label()]+\" to \"+ntname[goalnt]);\n");
	out.print("\treturn 0;\n}\n\n");
    }
}

class Ptr { int i; void incr() { i++; } int get() { return i; }}
