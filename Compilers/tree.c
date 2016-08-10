#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "y.tab.h"

char *id_name (int); /* Defined in the .l file. */

tree buildTree (int kind, tree one, tree two, tree three)
{
	tree p = (tree)malloc(sizeof (node));
	p->kind = kind;
	p->first = one;
	p->second = two;
	p->third = three;
	p->next = NULL;
	return p;
}

tree buildIntTree (int kind, int val)
{
	tree p = (tree)malloc(sizeof (node));
	p->kind = kind;
	p->value = val;
	p->first = p->second = p->third = NULL;
	p->next = NULL;
	return p;
}

char TokName[][12] = 
	{"NoTok", 
	"Boolean", "Integer", "True", "False", "And", "Array", "Begin", "Declare", "Else", "Elseif",
	"End", "Exit", "For", "If", "In", "Is", "Loop", "%", "Not", "Of",
	"Or", "Procedure", "Then", "When", "While" "Xor", "REMOVE THIS", "=", "!=", "<", "<=",
	">", ">=", "+", "-", "*", "/", "(", ")", "[", "]",
	":=", "..", ";", ":", ",", "Iconst", "Ident", "U+", "-", "Declaration"};
static int indent = 0;
void printTree (tree p)
{
	if (p == NULL) return;
	for (; p != NULL; p = p->next) {
		printf ("%*s%s", indent, "", TokName[p->kind]);
		switch (p->kind) {
			case Ident: 
				//printf (" %s\n", id_name (p->value));
				printf (" %d\n", p->value);
				break;
			case Iconst:
				printf (" %d\n", p->value);
				break;
			default:
				printf ("\n");
				indent += 2;
				printTree (p->first);
				printTree (p->second);
				printTree (p->third);
				indent -= 2;
		}
	}
}
