#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "tree.h"
#include "generate.h"
#include "y.tab.h"
#include "context.h"
#include "out.h"
char *id_name(int); // from ada.l

struct for_node {
	int locations[100];
	int count;
	struct for_node *prev;
};
struct for_node *for_list_tail = NULL;


void gen_expr(tree root) {
	char s[40];
	switch(root->kind) {
		case Plus:
			gen_expr(root->first);
			gen_expr(root->second);
			put_code("ADDI ", 1);
			return;
		case Minus:
			gen_expr(root->first);
			gen_expr(root->second);
			put_code("SUBI ", 1);
			return;
		case Mul:
			gen_expr(root->first);
			gen_expr(root->second);
			put_code("MULI ", 1);
			return;
		case Div:
			gen_expr(root->first);
			gen_expr(root->second);
			put_code("DIVI ", 1);
			return;
		case Mod:
			gen_expr(root->first);
			gen_expr(root->second);
			gen_expr(root->first);
			gen_expr(root->second);
			put_code("DIVI ", 1);
			put_code("MULI ", 1);
			put_code("SUBI ", 1);
			return;
			
		case And:
			gen_expr(root->first);
			gen_expr(root->second);
			put_code("ANDI ", 1);
			return;
		case Or:
			gen_expr(root->first);
			gen_expr(root->second);
			put_code("ORW ", 1);
			return;
		case Xor:
			gen_expr(root->first);
			gen_expr(root->second);
			put_code("XORW ", 1);
			return;
		case Not:
		case Uminus:
			gen_expr(root->first);
			put_code("NOTW ", 1);
			return;
			
		case Eq:
			gen_expr(root->first);
			gen_expr(root->second);
			put_code("SUBI ", 1);
			put_code("TSTEQI ", 1);
			return;
		case Neq:
			gen_expr(root->first);
			gen_expr(root->second);
			put_code("SUBI ", 1);
			put_code("TSTEQI ", 1);
			put_code("NOTW ", 1);
			return;
		case Geq:
			gen_expr(root->first);
			gen_expr(root->second);
			put_code("SUBI ", 1);
			put_code("TSTLTI ", 1);
			put_code("NOTW ", 1);
			return;
		case Gth:
			gen_expr(root->first);
			put_code("PUSHW 1 ", 3); // Subtract 1 from the second term to transform to greater then or equal to
			put_code("SUBI ", 1);
			gen_expr(root->second);
			put_code("SUBI ", 1);
			put_code("TSTLTI ", 1);
			put_code("NOTW ", 1);
			return;
		case Leq:
			gen_expr(root->first);
			put_code("PUSHW 1 ", 3); // Subtract 1 from the second term to transform to strictly less than.
			put_code("SUBI ", 1);
			gen_expr(root->second);
			put_code("SUBI ", 1);
			put_code("TSTLTI ", 1);
			return;
		case Lth:
			gen_expr(root->first);
			gen_expr(root->second);
			put_code("SUBI ", 1);
			put_code("TSTLTI ", 1);
			return;
			
		case Obracket:
			// Calculate offset
			gen_expr(root->second);
			
			// Make sure it is above the lower bound
			put_code("DUPW ", 1);
			sprintf(s, "PUSHW %d ", get_lower_bound(root->first->value));
			put_code(s, 3);
			put_code("SUBI ", 1);
			put_code("TSTLTI ", 1);
			put_code("RGOZ 1 ", 3);
			put_code("HALT ", 1);
			
			// Make sure it is below the upper bound
			put_code("DUPW ", 1);
			sprintf(s, "PUSHW %d ", get_upper_bound(root->first->value));
			put_code(s, 3);
			put_code("SWAPW ", 1);
			put_code("SUBI ", 1);
			put_code("TSTLTI ", 1);
			put_code("RGOZ 1 ", 3);
			put_code("HALT ", 1);
			
			put_code("PUSHW 2 ", 3); // Size of a single element in the array
			put_code("MULI ", 1);
			
			// Find base address
			sprintf(s, "PUSHW %d ", get_addr(root->first->value));
			put_code(s, 3);
			
			// add base + offset
			put_code("ADDI ", 1);
			
			put_code("GETSW ", 1);
			return;
			
		case True:
			put_code("PUSHW -1 ", 3);
			return;
		case False:
			put_code("PUSHW 0 ", 3);
			return;
		case Iconst:
			sprintf(s, "PUSHW %d ", root->value);
			put_code(s, 3);
			return;
		case Ident:
			sprintf(s, "PUSHW %d GETSW ", get_addr(root->value));
			put_code(s, 4);
			return;
		default:
			printf("Not implemented in gen_expr.\n");
			printTree(root);
			printf("\n");
	}
}

void generate(tree root) {
	for(tree t = root; t != NULL; t = t->next) {
		char s[20];
		switch(t->kind) {
			case Procedure:
				printTree(t);
				print(".DATA\n");
				generate(t->second);
				print("\n.CODE\n0: "); // Assumes there is only one procedure in the program
				generate(t->third);
				print("HALT .ENTRY 0\n"); // Assumes program starts at 0:
				
				end_scope();
				continue;
			case Declaration:
				if(t->second->kind == Array) {
					for(tree cur = t->first; cur != NULL; cur = cur->next) {
						declare_array(cur->value, t->second->second->kind, t->second->first->first->value, t->second->first->second->value);
					}
				} else {
					for(tree cur = t->first; cur != NULL; cur = cur->next) {
						declare_var(cur->value, t->second->kind);
					}
				}
				continue;
			case Declare:
				new_scope();
				generate(t->first);
				generate(t->second);
				end_scope();
				continue;
			case Assign:
				if(t->first->kind == Obracket) { // This is an array access.
					// Find base address
					sprintf(s, "PUSHW %d ", get_addr(t->first->first->value));
					put_code(s, 3);
					
					// Calculate offset
					gen_expr(t->first->second);
					
					// Make sure it is above the lower bound
					put_code("DUPW ", 1);
					sprintf(s, "PUSHW %d ", get_lower_bound(t->first->first->value));
					put_code(s, 3);
					put_code("SUBI ", 1);
					put_code("TSTLTI ", 1);
					put_code("RGOZ 1 ", 3);
					put_code("HALT ", 1);
					
					// Make sure it is below the upper bound
					put_code("DUPW ", 1);
					sprintf(s, "PUSHW %d ", get_upper_bound(t->first->first->value));
					put_code(s, 3);
					put_code("SWAPW ", 1);
					put_code("SUBI ", 1);
					put_code("TSTLTI ", 1);
					put_code("RGOZ 1 ", 3);
					put_code("HALT ", 1);
					
					put_code("PUSHW 2 ", 3);// Size of a single element in the array
					put_code("MULI ", 1);
					
					// add base + offset
					put_code("ADDI ", 1);
				} else {
					sprintf(s, "PUSHW %d ", get_addr(t->first->value));
					put_code(s, 3);
				}
				
				gen_expr(t->second);
				put_code("PUTSW ", 1);
				continue;
				
			case If:
			case Elseif:
				put_code("PUSHQ ", 1);
				int start = get_file_pos();
				put_code("      ", 8);
				gen_expr(t->first);
				put_code("GOZ ", 1);
				
				// if t->first is true
				generate(t->second);
				put_code("PUSHQ ", 1);
				int endif = get_file_pos();
				put_code("       ", 8);
				put_code("GOTO ", 1);
				// point start to here
				forward_ref(start);
				put_location();
				
				generate(t->third);
				forward_ref(endif);
				put_location();
				
				continue;
				
			case Else:
				generate(t->first);
				continue;
				
			case For:
				new_scope();
				if(for_list_tail == NULL) {
					for_list_tail = malloc(sizeof(struct for_node));
					for_list_tail->count = 0;
					for_list_tail->prev = NULL;
				} else {
					struct for_node *new = malloc(sizeof(struct for_node));
					new->count = 0;
					new->prev = for_list_tail;
					for_list_tail = new->prev;
				}
				
				// declare variable
				print("\n.DATA\n");
				declare_var(t->first->value, t->first->kind);
				print("\n.CODE");
				put_location();
				// Set variable to first value in range
				sprintf(s, "PUSHW %d ", get_addr(t->first->value));
				put_code(s, 3);
				gen_expr(t->second->first); // First value in range
				put_code("PUTSW ", 1);
				
				// Start of looping statements
				int loop_start = get_location();
				put_location();
				// Set up for conditional jump
				put_code("PUSHQ ", 1);
				int start_forward_ref = get_file_pos();
				put_code("      ", 8);
				
				// Check still in range
				gen_expr(t->first);
				put_code("PUSHW 1 ", 3); // Inclusive less than
				put_code("SUBI ", 1);
				gen_expr(t->second->second); // Second value in range
				put_code("SUBI ", 1);
				put_code("TSTEQI ", 1);
				put_code("NOTW ", 1);
				
				// Conditional jump to end of loop
				put_code("GOZ ", 1);
				
				// Do loop body
				generate(t->third);
				
				// Update loop variable
				sprintf(s, "PUSHW %d DUPW GETSW ", get_addr(t->first->value));
				put_code(s, 5);
				put_code("PUSHW 1 ", 3);
				put_code("ADDI ", 1);
				put_code("PUTSW ", 1);
				
				// Go back to top of loop
				sprintf(s, "PUSHQ %d ", loop_start);
				put_code(s, 9);
				put_code("GOTO ", 1);
				
				// Set end of loop location
				forward_ref(start_forward_ref);
				put_location();
				
				end_scope();
				
				for(int i = 0; i < for_list_tail->count; i++) {
					forward_ref(for_list_tail->locations[i]);
				}
				struct for_node *old_node = for_list_tail;
				for_list_tail = old_node->prev;
				free(old_node);
				continue;
			case Exit:
				put_code("PUSHQ ", 1);
				for_list_tail->locations[for_list_tail->count] = get_file_pos();
				for_list_tail->count++;
				put_code("      ", 8);
				if(t->first != NULL) {
					gen_expr(t->first);
					put_code("NOTW ", 1);
					put_code("GOZ ", 1);
				} else {
					put_code("GOTO ", 1);
				}
				continue;
			default:
				printf("Not implemented in generate.\n");
				printTree(t);
				printf("\n");
		}
	}
}