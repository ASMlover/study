%option noyywrap
%{
int chars = 0;
int words = 0;
int lines = 0;

int totalchars = 0;
int totalwords = 0;
int totallines = 0;
%}
%%
[a-zA-Z]+ { ++words; chars += strlen(yytext); }
\n        { ++chars; ++lines; }
.         { ++chars; }
%%

int main(int argc, char* argv[]) {
  int i;

  if (argc < 2) {
    // read from stdin
    yylex();
    fprintf(stdout, "chars=%d, words=%d, lines=%d\n", chars, words, lines);
    return 0;
  }

  for (i = 1; i < argc; ++i) {
    FILE* f = fopen(argv[i], "r");

    if (!f) {
      fprintf(stderr, "open %s failed\n", argv[i]);
      return 1;
    }

    yyrestart(f);
    yylex();
    fclose(f);

    fprintf(stdout, "%s : chars=%d, words=%d, lines=%d\n",
      argv[i], chars, words, lines);

    totalchars += chars; chars = 0;
    totalwords += words; words = 0;
    totallines += lines; lines = 0;
  }

  if (argc > 2) {
    fprintf(stdout, "totalchars=%d, totalwords=%d, totallines=%d\n",
      totalchars, totalwords, totallines);
  }

  return 0;
}
