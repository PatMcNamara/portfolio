%{
#include <stddef.h>

#include "tree.h"
#include "lex_yacc.h"

extern tree root;
%}

%start program

%token Boolean 1 Integer 2 True 3 False 4 And 5 Array 6 Begin 7 Declare 8 Declaration 50
%token Else 9 Elseif 10 End 11 Exit 12 For 13 If 14 In 15 Is 16 Loop 17
%token Mod 18 Not 19 Of 20 Or 21 Procedure 22 Then 23 When 24 While 25 Xor 26
%token Eq 27 Neq 28 Lth 29 Leq 30 Gth 31 Geq 32
%token Plus 33 Minus 34 Mul 35 Div 36
%token Oparen 37 Cparen 38 Obracket 39 Cbracket 40
%token Assign 41 Range 42 Semicolon 43 Colon 44 Comma 45
%token <i> Iconst 46 Ident 47
%token <u> Uplus 48 Uminus 49

%type <p>	decls declaration id_list type const_range stmts statement range ref end_if expr relation sum prod factor basic
%type <u>	sign

%union { tree p; int i; int u; }

%%
program
	: Procedure Ident Is decls Begin stmts End Semicolon
		{ root = buildTree (Procedure, buildIntTree(Ident, $2), $4, $6 ); }
	;
decls
	: declaration Semicolon decls
		{ $$ = $1; $$->next = $3; }
	| 
		{ $$ = NULL; } 
	;
declaration
	: id_list Colon type 
		{ $$ = buildTree(Declaration, $1, $3, NULL); }
	;
id_list
	: Ident Comma id_list
		{ $$ = buildIntTree(Ident, $1); $$->next = $3; }
	| Ident
		{ $$ = buildIntTree(Ident, $1); }
	;
type
	: Integer
		{ $$ = buildTree(Integer, NULL, NULL, NULL); }
	| Boolean
		{ $$ = buildTree(Boolean, NULL, NULL, NULL); }
	| Array Obracket const_range Cbracket Of type
		{ $$ = buildTree(Array, $3, $6, NULL); }
	;
const_range
	: Iconst Range Iconst
		{ $$ = buildTree(Range, buildIntTree(Iconst, $1), buildIntTree(Iconst, $3), NULL); }
	;
stmts
	: statement Semicolon stmts
		{ $$ = $1; $$->next = $3; }
	| 
		{ $$ = NULL; }
	;
statement
	: ref Assign expr
		{ $$ = buildTree(Assign, $1, $3, NULL); }
	| Declare decls Begin stmts End
		{ $$ = buildTree(Declare, $2, $4, NULL); }
	| For Ident In range Loop stmts End Loop
		{ $$ = buildTree(For, buildIntTree(Ident, $2), $4, $6); }
	| Exit
		{ $$ = buildTree(Exit, NULL, NULL, NULL); }
	| Exit When expr 
		{ $$ = buildTree(Exit, $3, NULL, NULL); }
	| If expr Then stmts end_if
		{ $$ = buildTree(If, $2, $4, $5); }
	;
range
	: sum Range sum
		{ $$ = buildTree(Range, $1, $3, NULL); }
	;
ref
	: Ident Obracket expr Cbracket
		{ $$ = buildTree(Obracket, buildIntTree(Ident, $1), $3, NULL); }
	| Ident
		{ $$ = buildIntTree(Ident, $1); }
	;
end_if
	: End If
		{ $$ = NULL; }
	| Else stmts End If
		{ $$ = buildTree(Else, $2, NULL, NULL); }
	| Elseif expr Then stmts end_if
		{ $$ = buildTree(Elseif, $2, $4, NULL);  $$->next = $5; }
	;
expr
	: relation Or relation
		{ $$ = buildTree(Or, $1, $3, NULL); }
	| relation And relation
		{ $$ = buildTree(And, $1, $3, NULL); }
	| relation Xor relation
		{ $$ = buildTree(Xor, $1, $3, NULL); }
	| relation
		{ $$ = $1; }
	;
relation
	: sum Eq sum 
		{ $$ = buildTree(Eq, $1, $3, NULL); }
	| sum Neq sum
		{ $$ = buildTree(Neq, $1, $3, NULL); }
	| sum Gth sum
		{ $$ = buildTree(Gth, $1, $3, NULL); }
	| sum Geq sum
		{ $$ = buildTree(Geq, $1, $3, NULL); }
	| sum Lth sum
		{ $$ = buildTree(Lth, $1, $3, NULL); }
	| sum Leq sum
		{ $$ = buildTree(Leq, $1, $3, NULL); }
	| sum 
		{ $$ = $1; }
	;
sum
	: sign prod
		{ $$ = buildTree($1, $2, NULL, NULL);
		  if($$->kind == 0) // If there is no sign just put in the prod.
		     $$ = $2;
		}
	| sum Plus prod
		{ $$ = buildTree(Plus, $1, $3, NULL); }
	| sum Minus prod
		{ $$ = buildTree(Minus, $1, $3, NULL); }
	;
sign
	: Plus
		{ $$ = Uplus; /* This isn't really needed but whatever. */ }
	| Minus
		{ $$ = Uminus; }
	|
		{ $$ = 0; }
	;
prod
	: factor
		{ $$ = $1; }
	| prod Mul factor
		{ $$ = buildTree(Mul, $1, $3, NULL); }
	| prod Div factor
		{ $$ = buildTree(Div, $1, $3, NULL); }
	| prod Mod factor
		{ $$ = buildTree(Mod, $1, $3, NULL); }
	;
factor
	: Not basic
		{ $$ = buildTree(Not, $2, NULL, NULL); }
	| basic
		{ $$ = $1; }
	;
basic
	: ref
		{ $$ = $1; }
	| Oparen expr Cparen
		{ $$ = $2; }
	| Iconst
		{ $$ = buildIntTree(Iconst, $1); }
	| True
		{ $$ = buildTree(True, NULL, NULL, NULL); }
	| False
		{ $$ = buildTree(False, NULL, NULL, NULL); }
	;
%%