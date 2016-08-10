#include <stdio.h>
#include "lex.yy.c"

int main() {
	int token;
	extern char *yytext;
	
	while((token = yylex()) != NULL) {
		printf("%d\t%s\n", token, yytext);
	}
	printf("%d\n", token);
	
	return 0;
}