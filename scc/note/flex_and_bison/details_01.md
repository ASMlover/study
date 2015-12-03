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

int main(int argc, char* argv[]) {
  yylex();
  printf("lines=%d, words=%d, chars=%d\n", lines, words, lines);

  return 0;
}
```
