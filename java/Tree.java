public class Tree {
	private Object state;
	int label;
	int place; // for register allocation
	Tree left, right;
	int value;
	String text;

	Tree() { label = -1; }
	Tree(int lbl) { label = lbl; }
	Tree(int lbl, int val) { this(lbl); value = val; }
	Tree(int lbl, String txt) { this(lbl); text = txt; }
	Tree(int lbl, Tree chld) { this(lbl); left = chld; }
	Tree(int lbl, Tree chld1, Tree chld2) { this(lbl); left = chld1; right = chld2; }

	Object state() { return state; }
	void state(Object st) { state = st; }
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
