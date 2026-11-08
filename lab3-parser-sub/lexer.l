%option noyywrap yylineno nounput noinput never-interactive
%x COMMENT
%{
#include "ast.h"
#include "parser.tab.h"
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
static int comment_line;
%}
%%
[ \t\r\n]+          ;
"//"[^\n]*           ;
"/*"                 { comment_line = yylineno; BEGIN(COMMENT); }
<COMMENT>"*/"        { BEGIN(INITIAL); }
<COMMENT>\n          ;
<COMMENT>.           ;
<COMMENT><<EOF>>     { fail(comment_line, "Unterminated comment"); }
"int"                { return INT; }
"main"               { return MAIN; }
"if"                 { return IF; }
"else"               { return ELSE; }
"while"              { return WHILE; }
"return"             { return RETURN; }
"=="                 { return EQ; }
"!="                 { return NE; }
"<="                 { return LE; }
">="                 { return GE; }
"&&"                 { return AND; }
"||"                 { return OR; }
[0-9]+               { yylval.text = copy_text(yytext); return NUMBER; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.text = copy_text(yytext); return IDENT; }
[(){};+*/%<>=!\-]    { return yytext[0]; }
.                    { fprintf(stderr, "Lexical error, line %d: unexpected '%s'\n", yylineno, yytext); return INVALID; }
%%
