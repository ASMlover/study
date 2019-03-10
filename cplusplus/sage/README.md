# **SAGE**
***

## **GRAMMER**
```
program         -> declaration* EOF ;
declaration     -> class_decl | func_decl | let_decl | statement ;
class_decl      -> "class" IDENTIFIER ( "(" IDENTIFIER ")" )? "{" NL func_decl* "}" NL ;
func_decl       -> "fn" IDENTIFIER "(" parameters? ")" block_stmt ;
parameters      -> IDENTIFIER ( "," IDENTIFIER )* ;
let_decl        -> "let" IDENTIFIER ( "=" expression )? NL ;
statement       -> if_stmt | while_stmt | for_stmt | return_stmt | break_stmt
                 | print_stmt | block_stmt | expr_stmt ;
if_stmt         -> "if" expression block_stmt ( "else" block_stmt )? ;
while_stmt      -> "while" expression block_stmt ;
for_stmt        -> "for" init_clause expression? ";" expression? block_stmt ;
init_clause     -> loop_let | loop_expr | ";" ;
loop_let        -> "let" IDENTIFIER ( "=" expression )? ";" ;
loop_expr       -> expression ";" ;
return_stmt     -> "return" expression? NL ;
break_stmt      -> "break" NL ;
print_stmt      -> "print" ( expression ( "," expression )* )? NL ;
block_stmt      -> "{" NL declaration* "}" NL ;
expr_stmt       -> expression NL ;

expression      -> assignment ;
assignment      -> ( call "." )? IDENTIFIER ( assign_oper ) assignment
                 | logic_or ;
assign_oper     -> "=" | "+=" | "-=" | "*=" | "/=" | "%=" ;
logic_or        -> logic_and ( "or" logic_and )* ;
logic_and       -> equality ( "and" equality )* ;
equality        -> comparison ( ( "is" | "!=" | "==" ) comparison )* ;
comparison      -> addition ( ( ">" | ">=" | "<" | "<=" ) addition )* ;
addition        -> multiplication ( ( "+" | "-" ) multiplication )* ;
multiplication  -> unary ( ( "*" | "/" | "%" ) unary )* ;
unary           -> ( "not" | "!" | "-" ) unary | call ;
call            -> primary ( "(" arguments? ")" | "." IDENTIFIER )* ;
arguments       -> expression ( "," expression )* ;
primary         -> INTEGER | DECIMAL | STRING
                 | "true" | "false" | "nil" | "self"
                 | "(" expression ")" | "super" "." IDENTIFIER ;
```
