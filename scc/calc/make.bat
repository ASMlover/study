bison --yacc -dv calc.y
flex calc.l
gcc -o calc y.tab.c lex.yy.c
