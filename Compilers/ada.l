%{
#include <stdio.h>
#include "tree.h"
#include "y.tab.h"
void yyerror (char*);
int getIdent(char[]);
int line=1;
%}
L		[A-Za-z]
D		[0-9]
%%
[ \t]+		;
and			{return And;}
array		{return Array;}
begin		{return Begin;}
declare		{return Declare;}
else		{return Else;}
elsif		{return Elseif;}
end			{return End;}
exit		{return Exit;}
for			{return For;}
if			{return If;}
in			{return In;}
is			{return Is;}
loop		{return Loop;}
mod			{return Mod;}
not			{return Not;}
of			{return Of;}
or			{return Or;}
procedure	{return Procedure;}
then		{return Then;}
when		{return When;}
while		{return While;}
xor			{return Xor;}
BOOLEAN		{return Boolean;}
INTEGER		{return Integer;}
TRUE		{return True;}
FALSE		{return False;}
{D}+		{ yylval.i = atoi (yytext); return Iconst; }
{L}({L}|{D}|_)*	{ yylval.i = getIdent(yytext); return Ident; }
"="			{return Eq;}
"/="		{return Neq;}
"<="		{return Leq;}
"<"			{return Lth;}
">="		{return Geq;}
">"			{return Gth;}
"+"			{return Plus;}
"-"			{return Minus;}
"*"			{return Mul;}
"/"			{return Div;}
"("			{return Oparen;}
")"			{return Cparen;}
"["			{return Obracket;}
"]"			{return Cbracket;}
":="		{return Assign;}
".."		{return Range;}
","			{return Comma;}
";"			{return Semicolon;}
":"			{return Colon;}
"\n"		{line++;}
.			{yyerror ("Bad Character");}
%%
int yywrap (void) { return 1; }

void yyerror (char s[])
{
	printf ("Error: %s at token %s in line %d\n", s, yytext, line);
}

typedef char name[15];
static char names[200][15] = {"<no name>"};
int top = 0;

int getIdent(char text[]) {
	int	i;

	for (i = 1; i <= top; i ++)
		if (strcmp (text, names[i]) == 0) return i;
	strcpy (names[++top], text);
	return top;
}

char *id_name (int i)
{
	return names[i];
}




