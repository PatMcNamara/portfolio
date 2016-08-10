/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    Boolean = 1,
    Integer = 2,
    True = 3,
    False = 4,
    And = 5,
    Array = 6,
    Begin = 7,
    Declare = 8,
    Declaration = 50,
    Else = 9,
    Elseif = 10,
    End = 11,
    Exit = 12,
    For = 13,
    If = 14,
    In = 15,
    Is = 16,
    Loop = 17,
    Mod = 18,
    Not = 19,
    Of = 20,
    Or = 21,
    Procedure = 22,
    Then = 23,
    When = 24,
    While = 25,
    Xor = 26,
    Eq = 27,
    Neq = 28,
    Lth = 29,
    Leq = 30,
    Gth = 31,
    Geq = 32,
    Plus = 33,
    Minus = 34,
    Mul = 35,
    Div = 36,
    Oparen = 37,
    Cparen = 38,
    Obracket = 39,
    Cbracket = 40,
    Assign = 41,
    Range = 42,
    Semicolon = 43,
    Colon = 44,
    Comma = 45,
    Iconst = 46,
    Ident = 47,
    Uplus = 48,
    Uminus = 49
  };
#endif
/* Tokens.  */
#define Boolean 1
#define Integer 2
#define True 3
#define False 4
#define And 5
#define Array 6
#define Begin 7
#define Declare 8
#define Declaration 50
#define Else 9
#define Elseif 10
#define End 11
#define Exit 12
#define For 13
#define If 14
#define In 15
#define Is 16
#define Loop 17
#define Mod 18
#define Not 19
#define Of 20
#define Or 21
#define Procedure 22
#define Then 23
#define When 24
#define While 25
#define Xor 26
#define Eq 27
#define Neq 28
#define Lth 29
#define Leq 30
#define Gth 31
#define Geq 32
#define Plus 33
#define Minus 34
#define Mul 35
#define Div 36
#define Oparen 37
#define Cparen 38
#define Obracket 39
#define Cbracket 40
#define Assign 41
#define Range 42
#define Semicolon 43
#define Colon 44
#define Comma 45
#define Iconst 46
#define Ident 47
#define Uplus 48
#define Uminus 49

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 25 "ada.y" /* yacc.c:1909  */
 tree p; int i; int u; 

#line 161 "y.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
