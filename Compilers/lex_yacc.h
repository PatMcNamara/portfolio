	// lex.yy.c
extern char *yytext;		// most recently scanned token
int /*"C"*/ yylex (void);	// return token_number of next token
	// y.tab.c
int /*"C"*/ yyparse(void);	// parse 
	// your .l file
void /*"C"*/ yyerror(char *s);// print a parser error; expected in yyparse()
