import java.util.*;
class Term extends Name {
  private int _esn;		/* external symbol number */
  private int _arity;		/* operator arity */
  private Term _link;		/* next terminal in esn order */
  private Rule _rules;		/* rules whose pattern starts with term */

  public Term(String name, int esn) {
    super(name);
    _esn = esn;
    _arity = -1;
  }

  public int esn() { return _esn; }
  public int arity() { return _arity; }
  public void arity(int n) { _arity = n; }
  public Term link() { return _link; }
  public void link(Term r) { _link = r; }
  public Rule rules() { return _rules; }
  public void rules(Rule r) { _rules = r; }
  public boolean term() { return true; }

  public String print() { return name() +": esn="+_esn+" #="+_arity; }
}
