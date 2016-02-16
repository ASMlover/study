%%
"colour"  { printf("color"); }
"flavour" { printf("flavor"); }
"clever"  { printf("smart"); }
"smart"   { printf("elegant"); }
.         { printf("%s", yytext); }
%%

int yywrap(void) {
  return 1;
}

int main(int argc, char* argv[]) {
  yylex();
  return 0;
}
