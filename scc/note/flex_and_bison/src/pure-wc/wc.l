%option noyywrap nodefault reentrant
%{
struct pwc {
  int chars;
  int words;
  int lines;
};
%}
%option extra-type="struct pwc*"

%%
%{
struct pwc* pp = yyextra;
%}

[a-zA-Z]+ { ++pp->words; pp->chars += strlen(yytext); }
\n        { ++pp->chars; ++pp->lines; }
.         { ++pp->chars; }
%%

int main(int argc, char* argv[]) {
  struct pwc mypwc = {0, 0, 0};
  yyscan_t scanner;

  if (yylex_init_extra(&mypwc, &scanner)) {
    fprintf(stderr, "init alloc failed\n");
    return 1;
  }

  if (argc > 1) {
    FILE* f;

    if (!(f = fopen(argv[1], "r"))) {
      fprintf(stderr, "open '%s' failed\n", argv[1]);
      return 1;
    }
    yyset_in(f, scanner);
  }
  else {
    yyset_in(stdin, scanner);
  }

  yylex(scanner);
  fprintf(stdout, "chars=%d, words=%d, lines=%d\n",
    mypwc.chars, mypwc.words, mypwc.lines);

  if (argc > 1)
    fclose(yyget_in(scanner));

  yylex_destroy(scanner);

  return 0;
}
