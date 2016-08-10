#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "context.h"
#include "out.h"

void print_current_context();

struct STEntry {
	int id;
	//char *name;
	int addr;
	int scope;
	int pos;
	int type;
	bool array;
	int lbound;
	int ubound;
};

const int var_len = 2; // all variables are 2 bytes.

struct STEntry* ST[50];
struct STEntry* STStack[300];
int size = 0; // Next free space on ST
int tos = 0; // Next free space on STStack
int currentScope = 0;
int DC = 0;

void push(struct STEntry *);
struct STEntry *pop();

void new_scope() {
	currentScope++;
	push(NULL);
}

void end_scope() {
	//print_current_context();
	while(STStack[tos-1] != NULL) {
		struct STEntry* entry = pop();
		free(ST[entry->pos]);
		ST[entry->pos] = entry;
	}
	
	// remove ST entries that are not defined outside of the scope that we are leaving.
	while(size > 0 && ST[size-1]->scope == currentScope) {
		size--;
	}
	// Pop null.
	pop();
	currentScope--;
}

int declare_array(/*char* name*/ int id, int type, int lower_bound, int upper_bound) {
	if(lower_bound > upper_bound) {
		printf("Lower array bound can not be greater then upper bound.\n");
	}
	//declare_var(name, type);
	declare_var(id, type);
	
	// Find just declared variable.
	struct STEntry* e = ST[0];
	for( int pos = 0; e != NULL && e->id != id; e = ST[++pos]){
		;
	}
	
	e->array = true;
	e->lbound = lower_bound;
	e->ubound = upper_bound;
	
	DC += var_len * (upper_bound - lower_bound);
	
	return e->addr;
}

int declare_var(/*char* name*/ int id, int type) { 
	struct STEntry* e = ST[0];
	int pos;
	for( pos = 0; e != NULL && e->id != id; e = ST[++pos]){
		;
	}
	/*if(e != NULL) {// If variable already exists in current scope, push it.
		if(e->scope == currentScope) {
			printf("Error: %s is declared twice in the same scope.\n", name);
			return 0;
		}
		push(e);
	}*/
	e = malloc(sizeof(struct STEntry));
	//e->name = name;
	e->id = id;
	e->addr = DC;
	DC += var_len;
	e->scope = currentScope;
	e->pos = size;
	e->type = type;
	e->array = false;
	if(pos == size) {// If variable doesn't exits, ST will be added to the end. 
		size++;
	}
	ST[pos] = e;
	
	char s[10];
	sprintf(s, "%d: ", e->addr);
	print(s);
	
	return e->addr;
}

/*int get_type(char *name) {
	struct STEntry *entry = ST[0];
	for(int pos = 0; entry != NULL && !(strcmp(entry->name, name) == 0); entry = ST[++pos])
		;
	
	if(entry == NULL || entry->scope > currentScope) {
		printf("Error: Variable %s is not declared.\n", name);
		return 0;
	}
	return entry->type;
}*/

int get_addr(int index) {
	struct STEntry* e = ST[0];
	for(int pos = 0; e != NULL && e->id != index; e = ST[++pos]){
		;
	}
	
	return e->addr;
}

int get_upper_bound(int index) {
	struct STEntry* e = ST[0];
	for(int pos = 0; e != NULL && e->id != index; e = ST[++pos]){
		;
	}
	
	return e->ubound;
}

int get_lower_bound(int index) {
	struct STEntry* e = ST[0];
	for(int pos = 0; e != NULL && e->id != index; e = ST[++pos]){
		;
	}
	
	return e->lbound;
}


void push(struct STEntry *entry) {
	STStack[tos++] = entry;
}

struct STEntry *pop() {
	return STStack[--tos];
}

void print_current_context() {
	printf("Printing context for scope level %d\n", currentScope);
	if(size == 0) {
		printf("No variables declared in this scope\n");
	}
	for(int i = 0; i < size; i++) {
		//printf("Symbol table entry #%d: '%s' declared in scope %d of type %s.\n", i, ST[i]->name, ST[i]->scope, ST[i]->type == 2 ? "Integer" : "Boolean");
		printf("Symbol table entry #%d: '%d' declared in scope %d of type %s.\n", i, ST[i]->id, ST[i]->scope, ST[i]->type == 2 ? "Integer" : "Boolean");
	}
	printf("End Context.\n\n");
}

/*bool is_array(char *name) {
	struct STEntry *entry = ST[0];
	for(int pos = 0; entry != NULL && !(strcmp(entry->name, name) == 0); entry = ST[++pos])
		;
	return entry != NULL ? entry->array : false;
}*/
