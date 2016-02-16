%option noyywrap nodefault yylineno case-insensitive
%{
#include <stdarg.h>
#include <string.h>
#include "sql.tab.h"

void yyerror(char* s, ...);
int oldstate;
%}

%x COMMENT
%s BTWMODE

%%
%%
