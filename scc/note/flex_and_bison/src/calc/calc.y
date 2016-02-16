%{
#include <stdio.h>
%}

%token NUMBER
%token ADD SUB MUL DIV ABS LP RP
%token EOL

%%
calclist: /* empty regular */
  | calclist exp EOL { fprintf(stdout, "= %d\n", $2); }
  ;

exp: factor
  | exp ADD factor { $$ = $1 + $3; }
  | exp SUB factor { $$ = $1 - $3; }
  ;

factor: term
  | factor MUL term { $$ = $1 * $3; }
  | factor DIV term { $$ = $1 / $3; }
  ;

term: NUMBER
  | ABS exp ABS { $$ = $2 >= 0 ? $2 : -$2; }
  | LP exp RP   { $$ = $2; }
  ;
%%

int yyerror(char* s) {
  fprintf(stderr, "error: %s\n", s);
  return 0;
}

int main(int argc, char* argv[]) {
  yyparse();
  return 0;
}
