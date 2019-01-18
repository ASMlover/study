# **LOX**
***

## **Summary**
 * add declararion statement and assignment expression

```
program     -> declararion* EOF ;
declararion -> class_decl | func_decl | let_decl | statement ;
class_decl  -> "class" IDENTIFILER ( ":" IDENTIFILER )? "{" function* "}" ;
func_decl   -> "fn" function ;
function    -> IDENTIFILER "(" parameters? ")" block_stmt ;
parameters  -> IDENTIFILER ( "," IDENTIFILER )* ;
let_decl    -> "let" IDENTIFILER ( "=" expression ) NEWLINE ;
statement   -> expr_stmt | for_stmt | if_stmt | print_stmt
            | return_stmt | while_stmt | block_stmt ;
expr_stmt   -> expression NEWLINE ;
for_stmt    -> "for" "(" init_clause expression? ";" expression? ")" statement ;
init_clause -> loop_let | loop_expr | ";" ;
loop_let    -> "let" IDENTIFILER ( "=" expression ) ";" ;
loop_expr   -> expression ";" ;
if_stmt     -> "if" "(" expression ")" statement ( "else" statement )? ;
print_stmt  -> "print" ( expression ( "," expression )* )? NEWLINE ;
return_stmt -> "return" expression? NEWLINE ;
while_stmt  -> "while" "(" expression ")" statement ;
block_stmt  -> "{" declararion* "}" ;

expression  -> assignment ;
assignment  -> ( call "." )? IDENTIFILER ( assign_oper ) assignment | logical_or ;
assign_oper -> "=" | "+=" | "-=" | "*=" | "/=" | "%=" ;
logical_or  -> logical_and ( "or" logical_and )* ;
logical_and -> equality ( "and" equality )* ;
...
equality    -> ... ;
...
unary       -> ( "-" | "!" | "not" ) unary | call ;
call        -> primary ( "(" arguments? ")" | "." IDENTIFILER )* ;
arguments   -> expression ( "," expression )* ;
primary     -> INTEGER | DECIMAL | STRING | "true" | "false" | "nil" | "self"
            | "(" expression ")" | "super" "." IDENTIFILER ;
```
