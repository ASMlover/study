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

<h2 id="3">文法与语法分析</h2>
语法分析器的任务是找出输入记号之间的关系，一种常见的关系表达方式就是语法分析树。

在计算机分析程序里最常用的语言是上下文无关文法(Context-Free Grammer, CFG)，书写上下文无关文法的标准格式就是BackusNaur范式(BackusNaur Form, BNF)。例如:
```c
// 1 * 2 + 3 * 4 + 5的文法如下
<expr> ::= <factor>
    | <expr> + <factor>
<factor> ::= NUMBER
    | <factor> * NUMBER
```
在BNF中，::=被读作“是”或“变成”，|是“或者”，创建同类分支的另一种方式。规则左边的名称是语法符号(symbol)。有效的BNF总是带有递归性，规则会直接或间接指向自身。
