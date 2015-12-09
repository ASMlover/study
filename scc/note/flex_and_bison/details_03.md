# **使用Bison**
***

## **1. Bison语法分析器如何匹配输入**
Bison基于你所给定的语法来生成一个可识别这个语法中有效“语句”的语法分析器。Bison只处理语法，需要保证其他部分的正确性。
```c
statement: NAME '=' expression

expression: NUMBER '+' NUMBER
          | NUMBER '?' NUMBER
```
竖线(|)意味着同一语法符号有两种可能性。Bison能有效地分析递归规则。

## **2. Bison的LALR(1)语法分析器无法分析的语法**
Bison语法分析器可以使用两种分析方法，一种是LALR(1)(自左向右向前查看一个记号)，另一种是GLR(通用的自左向右)。LALR不能处理有歧义的语法，也不能处理需要向前查看多个记号才能确定是否匹配规则的语法。
