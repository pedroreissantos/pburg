package pt.tecnico.pburg;

public class Tree {
	private Object state;
	private int label;
	private int place; // for register allocation
	private Tree left, right;
	private int value;
	private String text;

	public Tree() { label = -1; }
	public Tree(int lbl) { label = lbl; }
	public Tree(int lbl, int val) { this(lbl); value = val; }
	public Tree(int lbl, String txt) { this(lbl); text = txt; }
	public Tree(int lbl, Tree chld) { this(lbl); left = chld; }
	public Tree(int lbl, Tree chld1, Tree chld2) { this(lbl); left = chld1; right = chld2; }

	public Object state() { return state; }
	public void state(Object st) { state = st; }
	public int label() { return label; }
	public Tree left() { return left; }
	public Tree right() { return right; }
	public int place() { return place; }
	public int place(int plc) { return place = plc; }
	public int value() { return value; }
	public String text() { return text; }

	public String toString() {
		if (text != null) return  "("+label()+" '"+text()+"')";
		if (right != null) return  "("+label()+" "+left()+" "+right()+")";
		if (left != null) return  "("+label()+" "+left()+")";
		return  "("+label()+" "+value()+")";
	}
}
