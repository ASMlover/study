# **SAGE**
***

## **GRAMMER**
```
program         -> declaration* EOF ;
declaration     -> let_decl | statement ;
let_decl        -> "let" IDENTIFIER ( "=" expression )? NL ;
statement       -> print_stmt | expr_stmt ;
print_stmt      -> "print" ( expression ( "," expression )* )? NL ;
expr_stmt       -> expression NL ;

expression      -> assignment ;
assignment      -> IDENTIFIER ( assign_oper ) assignment | equality ;
assign_oper     -> "=" | "+=" | "-=" | "*=" | "/=" | "%=" ;
equality        -> comparison ( ( "is" | "!=" | "==" ) comparison )* ;
comparison      -> addition ( ( ">" | ">=" | "<" | "<=" ) addition )* ;
addition        -> multiplication ( ( "+" | "-" ) multiplication )* ;
multiplication  -> unary ( ( "*" | "/" | "%" ) unary )* ;
unary           -> ( "not" | "!" | "-" ) unary | primary ;
primary         -> INTEGER | DECIMAL | STRING | "true" | "false" | "nil"
                | "(" expression ")" | IDENTIFIER ;
```
