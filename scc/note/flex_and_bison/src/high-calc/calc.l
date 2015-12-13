%option noyywrap nodefault yylineno
%{
#include "calc.h"
#include "calc.tab.h"
%}

EXP ([Ee][-+]?[0-9]+)

%%
"+" |
"-" |
"*" |
"/" |
"=" |
"|" |
"," |
";" |
"(" |
")" { return yytext[0]; }

">" { yylval.fn = 1; return CMP; }
"<" { yylval.fn = 2; return CMP; }
"<>" { yylval.fn = 3; return CMP; }
"==" { yylval.fn = 4; return CMP; }
">=" { yylval.fn = 5; return CMP; }
"<=" { yylval.fn = 6; return CMP; }

"if"    { return IF; }
"then"  { return THEN; }
"else"  { return ELSE; }
"while" { return WHILE; }
"do"    { return DO; }
"let"   { return LET; }

"sqrt"  { yylval.fn = B_SQRT; return FUNC; }
"exp"   { yylval.fn = B_EXP; return FUNC; }
"log"   { yylval.fn = B_EXP; return FUNC; }
"print" { yylval.fn = B_PRINT; return FUNC; }
"exit"  { yylval.fn = B_EXIT; return FUNC; }

[a-zA-Z_][a-zA-Z_0-9]*  { yylval.s = lookup(yytext); return NAME; }

[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]+{EXP}?  { yylval.d = atof(yytext); return NUMBER; }

"//".*
[ \t]

\\\n  { fprintf(stdout, "c> "); }
\n    { return EOL; }
.     { yyerror("Mystery character %c\n", *yytext); }
%%
