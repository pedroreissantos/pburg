import java.util.*;
class Rule {
  private Nonterm _lhs;		/* lefthand side nonterminal */
  private Pattern _pattern;	/* rule pattern */
  private int _ern;		/* external rule number */
  private int _packed;		/* packed external rule number */
  private int _cost;		/* cost, if a constant */
  private String _code;		/* cost, if an expression */
  private String _template;	/* assembler template */
  private int _lineno;		/* first line of assembler template */
  private Rule _link;		/* next rule in ern order */
  private Rule _next;		/* next rule with same pattern root */
  private Rule _chain;		/* next chain rule with same rhs */
  private Rule _decode;		/* next rule with same lhs */
  private Rule _kids;		/* next rule with same _kids pattern */

  public Rule(String id, Pattern pat, String tmpl, String cod, int line) {
    _pattern = pat;
    _template = tmpl;
    _code = cod;
    _lineno = line;

    int i;
    for (i = 0; i < _code.length(); i++)
      if (_code.charAt(i) < '0' || _code.charAt(i) > '9')
        break;
    if (i == _code.length())
      _cost = Integer.parseInt(_code);
    else { _cost = -1; _code = "(" + _code + "(a))"; }
  }

  public Nonterm lhs() { return _lhs; }
	 void lhs(Nonterm nt) { _lhs = nt; }
  public Pattern pattern() { return _pattern; }
  public int ern() { return _ern; }
	 void ern(int n) { _ern = n; }
  public int packed() { return _packed; }
	 void packed(int i) { _packed = i; }
  public int cost() { return _cost; }
	 void cost(int c) { _cost = c; }
  public String code() { return _code; }
	 void code(String s) { _code = s; }
  public String template() { return _template; }
  public int line() { return _lineno; }
  public Rule link() { return _link; }
	 void link(Rule r) { _link = r; }
  public Rule next() { return _next; }
	 void next(Rule r) { _next = r; }
  public Rule chain() { return _chain; }
	 void chain(Rule r) { _chain = r; }
  public Rule decode() { return _decode; }
	 void decode(Rule r) { _decode = r; }
  public Rule kids() { return _kids; }
	 void kids(Rule r) { _kids = r; }

  public String print() {
    return "rule= "+_pattern+" "+_template+" #"+_lineno+" -> "+_code+"$"+_cost;
  }
  public String toString() {
    return ""+_lhs+": "+_pattern;
  }
}
