# **Flex和Bison进阶**
***

## **纯词法分析器和纯语法分析器**
单一的分析可能需要调用若干次yylex(), 因为yylex需要返回记号给调用程序。可以通过某些静态变量在yylex之外设置和获取某些需要的值：
```c
yyscan_t scaninfo;                          // 指向词法分析器每个实例数据的指针
int yylex_init(&scaninfo);                  // 创建一个词法分析器
int yylex_init_extra(userstuff, &scaninfo); // 创建一词法分析器，带有指向用户数据的指针
yyset_in(stdin, scaninfo);                  // 设置输入文件和其他参数

while (...) {
  tok = yylex(scaninfo);                    // 直到分析结束
}
yylex_destroy(scaninfo);                    // 释放词法分析器的数据
```
一个纯词法分析器如下:
```c
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
      fprintf(stderr, "open %s failed\n", argv[1]);
      return 1;
    }
    yyset_in(f, scanner);
  }
  else {
    yyset_in(stdin, scanner);
  }

  yylex(scanner);
  fprintf(stdout, "lines=%d, words=%d, chars=%d\n",
    mypwc.lines, mypwc.words, mypwc.chars);

  if (argc > 1)
    fclose(yyget_in(scanner));

  yylex_destroy(scanner);
}
```
