#include <stdio.h>
#include "typecheck.h"
#include "context.h"
#include "y.tab.h"

char *id_name(int); // from ada.l

void print_error(char *msg, tree t) {
	printf("Incorrect type: %s\n", msg);
	//printf("Tree:\n");
	//printTree(t);
	//printf("\n");
} 

void check(tree t) {
	// We can ignore the procedure and its ident
	check_stmt(t->second);
	check_stmt(t->third);
	
	end_scope();
}

/* Bitwise AND (&) is used throught. This is to prevent short circuting with logical AND (&&). */
int check_expr(tree t) {
	switch (t->kind) {
		case Obracket:
			if( t->first->kind == Ident & check_expr(t->second) == Integer & is_array(id_name(t->first->value))) {
				return get_type(id_name(t->first->value));
			}
			print_error("Invalid array usage.", t);
			return 0;
		case Plus:
		case Minus:
		case Mul:
		case Div:
			if( check_expr(t->first) == Integer & check_expr(t->second) == Integer ) {
				return Integer;
			} else {
				print_error("Arithmetic operators can only be used on two integers.", t);
				return 0;
			}
		case Uplus:
		case Uminus:
			if( check_expr(t->first) == Integer ) {
				return Integer;
			} else {
				print_error("Unary operators can only be used on integers.", t);
				return 0;
			}
		case Or:
		case And:
		case Xor:
			if( check_expr(t->first) == Boolean & check_expr(t->second) == Boolean ) {
				return Boolean;
			} else {
				print_error("Logical operators can only be used on two booleans.", t);
				return 0;
			}
		case Not:
			if( check_expr(t->first) == Boolean ) {
				return Boolean;
			} else {
				print_error("NOT can only be used on booleans.", t);
				return 0;
			}
		case Eq:
		case Neq:
			if( check_expr(t->first) == Boolean & check_expr(t->second) == Boolean ) {
				return Boolean;
			} // ELSE FALL THROUGH
		case Gth:
		case Geq:
		case Lth:
		case Leq:
			if( check_expr(t->first) == Integer & check_expr(t->second) == Integer ) {
				return Boolean;
			} else {
				print_error("Bad comparison.", t);
				return 0;
			}
		case True:
		case False:
			return Boolean;
		case Iconst:
			return Integer;
		case Ident:
			return get_type(id_name(t->value));
		default:
			print_error("Token of this type not checked by check_expr.", t);
			return 0;
	}
}

void check_stmt(tree t) {
	for( ; t != NULL; t = t->next ) {
		switch (t->kind) {
			case If:
			case Elseif:
				if( check_expr(t->first) != Boolean ) {
					print_error("Invalid IF statement.", t);
				}
				check_stmt(t->second);
				check_stmt(t->third);
				continue;
			case Else:
				check_stmt(t->first);
				continue;
			case Exit:
				if(check_expr(t->first) != Boolean) {
					print_error("exit when must produce a boolean.", t);
				}
				continue;
			case Assign:
				if(t->first->kind == Obracket | check_expr(t->first) != check_expr(t->second)) {
					print_error("Invalid assignment statement.", t);
				}
				continue;
			case For:
				new_scope();
				declare_var(id_name(t->first->value), Integer);
				if(check_expr(t->second->first) != Integer | check_expr(t->second->second) != Integer)//TODO or add a range check
					print_error("Invalid range.", t->second);
				
				check_stmt(t->third);
				end_scope();
				continue;
			case Declaration:;
				if(t->second->kind == Array) {
					for(tree cur = t->first; cur != NULL; cur = cur->next)
						declare_array(id_name(cur->value), t->second->second->kind);
				} else {
					for(tree cur = t->first; cur != NULL; cur = cur->next)
						declare_var(id_name(cur->value), t->second->kind);
				}
				continue;
			case Declare:
				new_scope();
				check_stmt(t->first);
				check_stmt(t->second);
				end_scope();
				continue;
			default:
				print_error("Token of this type not checked by check_stmt.", t);
				continue;
		}
	}
}