%{
#include <stdio.h>
#include "y.tab.h"

int 
yywrap(void)
{
  return 1;
}
%}
%%
"+"   return ADD;
"-"   return SUB;
"*"   return MUL;
"/"   return DIV;
"("   return LP;
")"   return RP;
"\n"  return CR;
([1-9][0-9]*)|0|([0-9]+\.[0-9]) {
  double tmp;
  sscanf(yytext, "%lf", &tmp);
  yylval.double_value = tmp;
  return DOUBLE_LITERAL;
}
[ \t] ;
. {
  fprintf(stderr, "lexical error!\n");
  exit(1);
}
%%
