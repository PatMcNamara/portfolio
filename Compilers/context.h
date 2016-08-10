#include <stdbool.h>

void new_scope();
void end_scope();
int declare_array(int, int, int, int);
int declare_var(int, int);
int get_addr(int);
int get_lower_bound(int);
int get_upper_bound(int);
//int declare_array(char*, int, int, int);
//int declare_var(char*, int);
//int get_type(char*);
//bool is_array(char*);