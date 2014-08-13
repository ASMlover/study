# -*- encoding: utf-8 -*- 
#
# Copyright (c) 2014 ASMlover. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list ofconditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materialsprovided with the
#    distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import ply.lex as lex 
import ply.yacc as yacc

tokens = (
    'NAME', 
    'NUMBER', 
    )

literals = ['=', '+', '-', '*', '/', '(', ')']

t_NAME = r'[a-zA-Z_][a-zA-Z0-9_]*'

def t_NUMBER(t):
  r'\d+'
  t.value = int(t.value)
  return t

t_ignore = ' \t'

def t_newline(t):
  r'\n+'
  t.lexer.lineno += t.value.count('\n')

def t_error(t):
  print('illegal character "%s"' % t.value[0])
  t.lexer.skip(1)


precedence = (
    ('left', '+', '-'), 
    ('left', '*', '/'), 
    ('right', 'UMINUS'), 
    )
names = {}

def p_statement_assign(p):
  'statment : NAME "=" expression'
  names[p[1]] = p[3]

def p_statement_expr(p):
  'statment : expression'
  print p[1]

def p_expression_binop(p):
  '''expression : expression '+' expression 
        | expression '-' expression 
        | expression '*' expression 
        | expression '/' expression '''
  if p[2] == '+':
    p[0] = p[1] + p[3]
  elif p[2] == '-':
    p[0] = p[1] - p[3]
  elif p[2] == '*':
    p[0] = p[1] * p[3]
  elif p[2] == '/':
    p[0] = p[1] / p[3]

def p_expression_uminus(p):
  'expression : "-" expression %prec UMINUS'
  p[0] = -p[2]

def p_expression_group(p):
  'expression : "(" expression ")"'
  p[0] = p[2]

def p_expression_number(p):
  'expression : NUMBER'
  p[0] = p[1]

def p_expression_name(p):
  'expression : NAME'
  try:
    p[0] = names[p[1]]
  except LookupError:
    print 'undefined name <%s>' % p[1]
    p[0] = 0

def p_error(p):
  if p:
    print 'syntax error at <%s>' % p.value
  else:
    print 'syntax error at EOF'


if __name__ == '__main__':
  lex.lex()
  yacc.yacc()

  while True:
    try:
      s = raw_input('calc > ')
    except EOFError:
      break
    if not s:
      continue
    yacc.parse(s)
