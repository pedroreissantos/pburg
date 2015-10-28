public abstract class Name {
  private String _name;		/* nonterminal name */

  public Name(String name) { _name = name; }
  public String name() { return _name; }
  public String toString() { return _name; }

  public abstract boolean term();
}
