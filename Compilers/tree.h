typedef struct Node {
	int	kind, value;
	struct	Node *first, *second, *third, *next;
} node;
typedef node	*tree;

extern char TokName[][12];

tree buildTree (int kind, tree first, tree second, tree third);
tree buildIntTree (int kind, int val);
void printTree (tree);