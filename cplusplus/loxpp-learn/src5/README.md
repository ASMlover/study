# **LOX**
***

## **Summary**
 * add statement (expression statement and print statement)

```
program     -> statement* EOF ;
statement   -> expr_stmt | print_stmt ;
expr_stmt   -> expression NEWLINE ;
print_stmt  -> "print" expression NEWLINE ;
```
