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
"|" |
"(" |
")" { return yytext[0]; }
[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]+{EXP}? { yylval.d = atof(yytext); return NUMBER; }

\n { return EOL; }
"//".*
[ \t] {}
. {yyerror("Mystery character %c\n", *yytext); }
%%
