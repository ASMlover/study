# **LOX**
***

## **Summary**
 * add declararion statement and assignment expression

```
program     -> declararion* EOF ;
declararion -> let_decl | statement ;
let_decl    -> "let" IDENTIFILER ( "=" expression ) NEWLINE ;
statement   -> expr_stmt | if_stmt | print_stmt | block_stmt ;
expr_stmt   -> expression NEWLINE ;
if_stmt     -> "if" "(" expression ")" statement ( "else" statement )? ;
print_stmt  -> "print" ( expression ( "," expression )* )? NEWLINE ;
block_stmt  -> "{" declararion* "}" ;

expression  -> assignment ;
assignment  -> IDENTIFILER ( assign_oper ) assignment | equality ;
assign_oper -> "=" | "+=" | "-=" | "*=" | "/=" | "%=" ;
...
equality    -> ... ;
```
