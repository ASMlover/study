# **PLY for Python**
***


## **1. 词法分析**
    1) 词法单元类型列表(token list)
        * 一个词法单元的元组, 必须以tokens作为元组的变量名;
          tokens = (
            'NUMBER', 
            'ADD', 
            'SUB', 
            'MUL', 
            'DIV', 
            'LPAREN', 
            'RPAREN', 
          )
    2) 词法单元模式定义
        * 可以采用正则表达式字符串或者函数来定义, 必须采用t_TOKENNAME的模式
          来命名(NUMBER类型的词法单元对应的变量名或函数名是t_NUMBER)
          t_ADD = r'+'
          t_SUB = r'-'
          t_MUL = r'*'
          t_DIV = r'/'
        * 识别出词法单元的时候, 需要执行动作的就可以使用函数来定义:
          def t_NUMBER(t):
            r'\d+(\.\d+)*'
            try:
              t.value = int(t.value)
            except ValueError:
              t.value = float(t.value)
            return t
        * 参数t是LexToken类型, 表示识别出的词法单元, 具有属性:
          value: 默认就是识别出的字符串序列
          type: 词法单元的类型, 就是在tokens元组中的定义
          line: 词法单元在源代码中的行号
          lexpos: 词法单元在该行的列号
        * 词法单元规则的添加顺序:
          所有用过函数定义的token的添加顺序与在词法定义文件中出现顺序相同
          所有通过正则表达式定义的token, 按正则表达式字符串长度由大至小排列
    3) 词法单元的丢弃(discarded tokens)
       可以添加t_ignore_前缀表示将丢弃匹配的token
        t_ignore_COMMENT = r'#.*'
    4) 行号和位置信息
        lex.py不知如何识别一行, 可以通过t_newline()规则来更新行号信息:
            def t_newline(t):
              r'\n+'
              t.lexer.lineno += len(t.value)
    5) 在词法分析器中定义literals变量可声明一些字符, 然后在yacc中直接使用;
        literals = ['+', '-', '*', '/'] 或 literals = '+-*/'
        literals只能是字符不能是字符串;
    6) 错误处理
        t_error()用于处理词法错误, 此时t.value包含了所有没有处理的输入字符串
        可以通过t.lexer.skip(1)来跳过1个字符, 然后继续解析;
    7) 词法分析器的生成和使用
        通过调用ply.lex.lex()可以构建词法分析器; 词法分析器生成之后, 可以调
        用下面两个函数实现词法分析:
          * lexer.input(data) 重置lexer并保存输入字符串
          * lexer.token() 返回识别出的下一个token, 解析完毕返回None
            lexer = lex.lex()
            lexer.input(data)
            for token in lexer:
              # processing ...
              pass


## **2. 语法分析**
    1) 文法规则的描述
       每个文法规则被描述为一个函数, 该函数的文档字符串描述了对应的上下文无
       关文法的规则; 函数体实现规则的语义动作; 每个函数都会接受一个参数p, 这
       个参数是一个序列(sequence), 包含了组成这个规则的所有语法符号, p[i]就
       是规则中第i个文法符号:
          def p_expression_add(p):
            'expression : expression ADD expression'
            #  |      |   |   |
            # p[0]   p[1]p[2]p[3]
            p[0] = p[1] + p[3]
    2) 起始规则
        yacc开始解析的规则, 可通过关键字参数传递给yacc以指定起始文法规则
            yacc.yacc(start = 'rule_name')
    3) 解析错误处理: 规则p_error(p)原来捕捉语法错误
    4) parser的构建
       通过yacc.yacc()来构建parser, 该函数可以构建LR parsing tablea;
    5) 有歧义的文法
       遇到有歧义的文法, 会报"shift/reduce"冲突或"reduce/reduce"冲突; 
        * "shift/reduce"是遇到一个词法单元时不值该执行规约动作还是执行词法单
          元移动;
          遇到该冲突是, yacc会采用shift动作; 为解决该歧义, yacc允许定义词法
          单元的优先级和结核性; 通过定义precedence元组可以实现:
              precedence = (
                ('left', 'ADD', 'SUB'), 
                ('left', 'MUL', 'DIV'),
              )
          当一个操作符有两个语义时, yacc通过虚词法单元解决:
              precedence = (
                ('left', 'ADD', 'SUB'), 
                ('left', 'MUL', 'DIV'),
                ('right', 'USUB'), # unary sub operator
              )
          USUB是虚词法单元, 还需为其定义规则:
              def p_expr_usub(p):
                'expression : "-" expression %prec USUB'
                p[0] = -p[2]
          nonassoc指定操作符不具备结合性; 如a < b < c是不合法的
              precedence = (
                ('nonassoc', 'LT', 'GT', 'LE', 'GE', 'EQ'), # </>/<=/>=/==
                ('left', 'ADD', 'SUB'), 
                ('left', 'MUL', 'DIV'), 
                ('right', 'USUB')
              )
        * 'reduce/reduce'是解析栈中可以应用多个规则进行规约, 解决方法是选择
          第一个出现的规则进行规约;
