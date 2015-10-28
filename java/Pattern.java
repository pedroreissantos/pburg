class Pattern {
  private Name _op;		/* a terminal or nonterminal */
  private Pattern _left, _right;	/* operands */
  private int _nterms;		/* number of terminal nodes in this tree */

  public Pattern(Name op) { this(op, null, null); }
  public Pattern(Name op, Pattern l) { this(op, l, null); }
  public Pattern(Name op, Pattern l, Pattern r) { _op = op; _left = l; _right = r; }

  public Name op() { return _op; }
  public Pattern left() { return _left; }
  public Pattern right() { return _right; }
  public int nterms() { return _nterms; }
  public void nterms(int nt) { _nterms = nt; }

  public String toString() {
    String s = _op.name();
    if (_left != null) {
      s += "(" + _left;
      if (_right != null)
        s += "," + _right;
      s += ")";
    }
    return s;
  }
}
