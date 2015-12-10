%{
#include <stdio.h>
#include <stdlib.h>
#include "calc.h"
%}

%union {
  struct ast* t;
  double d;
}

%token <d> NUMBER
%token EOL

%type <t> exp factor term

%%
calclist:
  | calclist exp EOL {
    fprintf(stdout, "= %lf\n", eval($2));
    treefree($2);
    fprintf(stdout, "> ");
  }
  | calclist EOL { fprintf(stdout, "> "); }
  ;

exp: factor
  | exp '+' factor { $$ = newast('+', $1, $3); }
  | exp '-' factor { $$ = newast('-', $1, $3); }
  ;

factor: term
  | factor '*' term { $$ = newast('*', $1, $3); }
  | factor '/' term { $$ = newast('/', $1, $3); }
  ;

term: NUMBER { $$ = newnum($1); }
  | '|' term '|' { $$ = newast('|', $2, NULL); }
  | '(' exp ')' { $$ = $2; }
  | '-' term { $$ = newast('M', $2, NULL); }
  ;
%%
