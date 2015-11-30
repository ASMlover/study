%{
#include <stdio.h>
#include <stdlib.h>
#define YYDEBUG 1
%}
%union {
  int int_value;
  double double_value;
}
%token <double_value> DOUBLE_LITERAL
%token CALC_ADD CALC_SUB CALC_MUL CALC_DIV CALC_CR
%type <double_value> expression term primary_expression
%%
line_list
  : line
  | line_list line
  ;
line
  : expression CALC_CR
  {
    printf(">> %lf\n", $1);
  }
  | error CALC_CR
  {
    yyclearin;
    yyerrok;
  }
  ;
expression
  : term
  | expression CALC_ADD term
  {
    $$ = $1 + $3;
  }
  | expression CALC_SUB term
  {
    $$ = $1 - $3;
  }
  ;
term
  : primary_expression
  | term CALC_MUL primary_expression
  {
    $$ = $1 * $3;
  }
  | term CALC_DIV primary_expression
  {
    $$ = $1 / $3;
  }
  ;
primary_expression
  : DOUBLE_LITERAL
  ;
%%
int yyerror(const char* str) {
  extern char* yytext;
  fprintf(stderr, "parser error near %s\n", yytext);
  return 0;
}

int main(int argc, char* argv[]) {
  extern int yyparse(void);
  extern FILE* yyin;

  yyin = stdin;
  if (yyparse()) {
    fprintf(stderr, "Error ! Error ! Error !\n");
    exit(1);
  }
}
