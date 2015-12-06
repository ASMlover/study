%{
#include "calc.tab.h"

int yywrap(void) {
  return 1;
}
%}
%%
"+"         { return ADD; }
"-"         { return SUB; }
"*"         { return MUL; }
"/"         { return DIV; }
"|"         { return ABS; }
"("         { return LP; }
")"         { return RP; }
"//".*      {}
[-+]?[0-9]+ { yylval = atoi(yytext); return NUMBER; }
\n          { return EOL; }
[ \t]       {}
.           { printf("Mystery character %s\n", yytext); }
%%
