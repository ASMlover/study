```
 _____         _             _
|_   _|_ _  __| |_ __   ___ | | ___
  | |/ _` |/ _` | '_ \ / _ \| |/ _ \
  | | (_| | (_| | |_) | (_) | |  __/
  |_|\__,_|\__,_| .__/ \___/|_|\___|
                |_|
```

## **Syntax Grammar**
```
program     -> declaration* EOF ;
declaration -> fn_decl | var_decl | statement ;
fn_decl     -> "fn" IDENTIFIER "(" parameters? ")" block_stmt ;
parameters  -> IDENTIFIER ( "," IDENTIFIER )* ;
var_decl    -> "var" IDENTIFIER ( "=" expression? ) ";" ;
statement   -> expr_stmt | block_stmt ;
expr_stmt   -> expression ";" ;
block_stmt  -> "{" declaration* "}" ;

expression  -> assignment ;
assignment  -> assignment | term ;
term        -> factor ( ( "+" | "-" ) factor )* ;
factor      -> call ( ( "*" | "/" ) call )* ;
call        -> primary ( "(" arguments? ")" )* ;
arguments   -> expression ( "," expression )* ;
primary     -> NUMERIC | STRING | IDENTIFIER | "nil" | "true" | "false" | "(" expression ")" ;
```

## **Keywords**
```
false fn nil true var
```

## **Operators**
| assignment | arithmetic |
| :--------: | :--------: |
|    a = b   |   a + b    |
|            |   a - b    |
|            |   a * b    |
|            |   a / b    |
