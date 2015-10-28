import java.util.*;
class Nonterm extends Name {
  private int _number;		/* identifying number */
  private int _lhscount;	/* # times nt appears in a rule lhs */
  private boolean _reached;	/* if reached from start nonterminal */
  private Rule _rules;		/* rules w/nonterminal on lhs */
  private Rule _chain;		/* chain rules w/nonterminal on rhs */
  private Nonterm _link;	/* next terminal in number order */

  public Nonterm (String name, int number) {
    super(name);
    _number = number;
  }

  public int number() { return _number; }
  public int lhscount() { return _lhscount; }
	 void lhsincr() { _lhscount++; }
  public boolean reached() { return _reached; }
	 void reach() { _reached = true; }
  public Rule rules() { return _rules; }
  public void rules(Rule r) { _rules = r; }
  public Rule chain() { return _chain; }
  public void chain(Rule r) { _chain = r; }
  public Nonterm link() { return _link; }
  public void link(Nonterm nt) { _link = nt; }
  public boolean term() { return false; }

  public String print() {
    return name()+": n="+_number+" lhs="+_lhscount+" ("+_reached+")";
  }
}
