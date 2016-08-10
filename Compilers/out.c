#include <stdio.h>
#include "out.h"

extern FILE *output;
int LC = 0;
//bool in_data_section = false;

// Puts the designated string into the output file without updating LC
void print(char *str) {
	printf("putting %s into file.\n", str);
	fprintf(output, "%s", str);
}

// Puts the designated string into the output file and updates LC by size.
void put_code(char *opcode, int size) {
	print(opcode);
	LC += size;
}

/* This function will put the current LC into the file file at location file_index. */
void forward_ref(int file_index) {
	int cur = ftell(output);
	fseek(output, file_index, SEEK_SET);
	printf("Seeking to %d.\n", file_index);
	
	char s[30];
	sprintf(s, "%d ", LC);
	print(s);
	
	printf("Seeking back to %d\n", cur);
	fseek(output, cur, SEEK_SET);
}

int get_file_pos() {
	printf("current file position %d\n", ftell(output));
	return ftell(output);
}

int get_location() {
	return LC;
}

void put_location() {
	char s[20];
	sprintf(s, "\n%d: ", LC);
	print(s);
}