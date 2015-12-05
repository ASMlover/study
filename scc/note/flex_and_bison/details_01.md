# **Flex和Bison简介**
***

词法分析把输入分割成一个个有意义的词块(token)；而语法分析则确定这些token是如何彼此关联的。词法分析通过所做的就是在输入中寻找字符的模式。

<h2 id="1">1. 第一个Flex程序</h2>
```c
/* 类Unix的wc程序 */
%{
int chars = 0;
int words = 0;
int lines = 0;
%}
%%
[a-zA-Z]+ { words++; chars += strlen(yytext); }
\n        { chars++; lines++; }
.         { chars++; }
%%

int yywrap(void) {
  return 1;
}

int main(int argc, char* argv[]) {
  yylex();
  printf("lines=%d, words=%d, chars=%d\n", lines, words, lines);

  return 0;
}
```

<h2 id="2">纯Flex的程序</h2>
简单的程序可以将所有的内容都放到Flex里面完成, 或只需要一点C代码。如从英式英语到美式英语的转换：
```c
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
```
