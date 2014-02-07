# **README for klang**
***


 * **Overview**
  - Just for learning creating an easy script language.
  - It's just for fun.



 * **Syntax**
    program 
      : definition-or-stmt
      | program definition-or-stmt
      ;
    
    definition-or-stmt
      : function-definition
      | stmt 
      ;

    function-definition
      : func identifier '(' param-list ')' block 
      | func identifier '(' ')' block
      ;

    param-list 
      : identifier 
      | param-list ',' identifier
      ;

    argument-list 
      : expr 
      | argument-list ',' expr 
      ; 

    stmt-list 
      : stmt 
      | stmt-list stmt 
      ;

    expr 
      : logical-or-expr 
      | identifier '=' expr 
      ;

    logical-or-expr 
      : logical-and-expr 
      | logical-or-expr '||' logical-and-expr 
      ;

    logical-and-expr 
      : equal-expr 
      | logical-and-expr '&&' equal-expr 
      ;

    equal-expr 
      : relational-expr 
      | equal-expr '==' relational-expr 
      | equal-expr '<>' relational-expr 
      ;

    relational-expr 
      : add-expr 
      | relational-expr '>' add-expr 
      | relational-expr '>=' add-expr 
      | relational-expr '<' add-expr 
      | relational-expr '<=' add-expr
      ;

    add-expr 
      : mul-exr 
      | add-expr '+' mul-expr
      | add-expr '-' mul-expr 
      ;

    mul-expr 
      : unary-expr 
      | mul-expr '*' unary-expr 
      | mul-expr '/' unary-expr 
      | mul-expr '%' unary-expr 
      ;

    unary-expr 
      : primary-expr 
      | '-' unary-expr 
      ; 

    primary-expr 
      : identifier '(' argument-list ')' 
      | identifier '(' ')'
      | '(' expr ')'
      | identifier 
      | int-literal 
      | real-literal 
      | str-literal 
      | true 
      | false 
      | nil 
      ;

    stmt 
      : expr ';'
      | global-stmt 
      | if-stmt 
      | while-stmt 
      | return-stmt 
      | break-stmt 
      ;

    global-stmt 
      : global identifier-list ';'
      ;

    identifier-list 
      : identifier 
      | identifier-list ',' identifier
      ;

    if-stmt 
      : if expr block 
      | if expr block else block 
      ;

    while-stmt 
      : while expr block 
      ;

    return-stmt 
      : return expr ';'
      | return ';'
      ;

    break-stmt 
      : break ';'
      ;

    block 
      : '{' stmt-list '}'
      | '{' '}'
      ;

