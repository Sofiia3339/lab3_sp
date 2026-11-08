%{
#include "ast.h"
#include <stdlib.h>
int yylex(void);
void yyerror(const char *);
extern int yylineno;
Node *root;
%}
%union { char *text; Node *node; }
%token INT MAIN IF ELSE WHILE RETURN
%token EQ NE LE GE AND OR INVALID
%token <text> NUMBER IDENT
%type <node> block statements statement expression declaration
%locations
%destructor { free($$); } IDENT NUMBER
%destructor { free_tree($$); } block statements statement declaration expression
%nonassoc IF_WITHOUT_ELSE
%nonassoc ELSE
%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%right UMINUS '!'
%start program
%%
program: INT MAIN '(' ')' block { root = node("Program", "main", @1.first_line, $5, 0, 0); };
block: '{' statements '}' { $$ = node("Block", "", @1.first_line, $2, 0, 0); };
statements: /* empty */ { $$ = 0; }
    | statements statement { $$ = append($1, $2); }
    | statements declaration { $$ = append($1, $2); };
declaration:
      INT IDENT '=' expression ';' { $$ = node("Declare", $2, @1.first_line, $4, 0, 0); free($2); }
    | INT IDENT ';' { $$ = node("Declare", $2, @1.first_line, 0, 0, 0); free($2); };
statement:
      IDENT '=' expression ';' { $$ = node("Assign", $1, @1.first_line, $3, 0, 0); free($1); }
    | RETURN expression ';' { $$ = node("Return", "", @1.first_line, $2, 0, 0); }
    | IF '(' expression ')' statement %prec IF_WITHOUT_ELSE { $$ = node("If", "", @1.first_line, $3, $5, 0); }
    | IF '(' expression ')' statement ELSE statement { $$ = node("If", "", @1.first_line, $3, $5, $7); }
    | WHILE '(' expression ')' statement { $$ = node("While", "", @1.first_line, $3, $5, 0); }
    | block { $$ = $1; }
    | ';' { $$ = node("Empty", "", @1.first_line, 0, 0, 0); };
expression:
      NUMBER { $$ = node("Number", $1, @1.first_line, 0, 0, 0); free($1); }
    | IDENT { $$ = node("Variable", $1, @1.first_line, 0, 0, 0); free($1); }
    | '(' expression ')' { $$ = $2; }
    | '-' expression %prec UMINUS { $$ = node("Unary", "-", @1.first_line, $2, 0, 0); }
    | '!' expression { $$ = node("Unary", "!", @1.first_line, $2, 0, 0); }
    | expression '+' expression { $$ = node("Binary", "+", @2.first_line, $1, $3, 0); }
    | expression '-' expression { $$ = node("Binary", "-", @2.first_line, $1, $3, 0); }
    | expression '*' expression { $$ = node("Binary", "*", @2.first_line, $1, $3, 0); }
    | expression '/' expression { $$ = node("Binary", "/", @2.first_line, $1, $3, 0); }
    | expression '%' expression { $$ = node("Binary", "%", @2.first_line, $1, $3, 0); }
    | expression '<' expression { $$ = node("Binary", "<", @2.first_line, $1, $3, 0); }
    | expression '>' expression { $$ = node("Binary", ">", @2.first_line, $1, $3, 0); }
    | expression LE expression { $$ = node("Binary", "<=", @2.first_line, $1, $3, 0); }
    | expression GE expression { $$ = node("Binary", ">=", @2.first_line, $1, $3, 0); }
    | expression EQ expression { $$ = node("Binary", "==", @2.first_line, $1, $3, 0); }
    | expression NE expression { $$ = node("Binary", "!=", @2.first_line, $1, $3, 0); }
    | expression AND expression { $$ = node("Binary", "&&", @2.first_line, $1, $3, 0); }
    | expression OR expression { $$ = node("Binary", "||", @2.first_line, $1, $3, 0); };
%%
void yyerror(const char *message) { fprintf(stderr, "Syntax error, line %d: %s\n", yylineno, message); }
