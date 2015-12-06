%option noyywrap
%{
int chars = 0;
int words = 0;
int lines = 0;
%}
%%
[a-zA-Z]+ { ++words; chars += strlen(yytext); }
\n        { ++chars; ++lines; }
.         { ++chars; }
%%

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    fprintf(stderr, "Usage: wc [filename]\n");
    return 1;
  }
  if (argc > 1) {
    if (!(yyin = fopen(argv[1], "r"))) {
      fprintf(stderr, argv[1]);
      return 1;
    }
  }

  yylex();
  fprintf(stdout, "lines=%d, words=%d, chars=%d\n", lines, words, chars);

  return 0;
}
