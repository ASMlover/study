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

int yywrap(void) {
  return 1;
}

int main(int argc, char* argv[]) {
  yylex();
  printf("chars = %d, words = %d, lines = %d\n", chars, words, lines);
}
