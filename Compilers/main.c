#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "lex_yacc.h"
#include "tree.h"
//#include "typecheck.h"
#include "generate.h"

extern FILE *yyin;
tree root;

FILE *output;

int main (int argc, char **argv) {
	if ((yyin = fopen (argv[1], "r")) == NULL) {
		fprintf (stderr, "%s: Can't open Input File %s\n", argv[0], argv[1]); 
		exit(1);
	}

	yyparse();

	if(root == NULL) {// If there was a syntax error, the tree will be null.
		printf("Parsing failed. Exiting.\n");
		exit(1);
	}
	
	printf("Code checking disabled, code is assumed to be valid.\n");
	//check(root);
	
	if((output = fopen(argv[2], "w")) == NULL) {
		printf("Failed to open output file %s.\n", argv[2]);
		exit(1);
	}
	
	generate(root);
}