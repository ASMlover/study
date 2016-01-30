#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Copyright (c) 2016 ASMlover. All rights reserved.
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

from collections import namedtuple
import tyr_ast as ast
from tyr_error import TyrSyntaxError

MatchPattern = namedtuple('Match', ['pattern', 'body'])
ConditionElif = namedtuple('ConditionElif', ['cond', 'body'])

class ParserError(TyrSyntaxError):
    def __init__(self, message, token):
        super(ParserError, self).__init__(token.lineno, token.column, message)

def enter_scope(parser, name):
    class State(object):
        def __enter__(self):
            parser.scope.append(name)

        def __exit__(self, exc_type, exc_val, exc_tb):
            parser.scope.pop()

    return State()

class Subparser(object):
    PRECEDENCE = {
        'call': 10,
        'subscript': 10,

        'unary': 9,

        '*': 7,
        '/': 7,
        '%': 7,

        '+': 6,
        '-': 6,

        '>': 5,
        '>= ': 5,
        '<': 5,
        '<=': 5,

        '==': 4,
        '!=': 4,

        '&&': 3,

        '||': 2,

        '..': 1,
        '...': 1,
    }

    def get_subparser(self, token, subparsers, default=None):
        cls = subparsers.get(token.name, default)
        if cls:
            return cls()

class PrefixSubparser(Subparser):
    def parse(self, parser, tokens):
        raise NotImplementedError()

class InfixSubparser(Subparser):
    def parse(self, parser, tokens, left):
        raise NotImplementedError()

    def get_precedence(self, token):
        raise NotImplementedError()

class NumberExpression(PrefixSubparser):
    """number_expr: NUMBER"""
    def parse(self, parser, tokens):
        token = tokens.consume_expected('NUMBER')
        return ast.Number(token.value)

class StringExpression(PrefixSubparser):
    """str_expr: STRING"""
    def parse(self, parser, tokens):
        token = tokens.consume_expected('STRING')
        return ast.String(token.value)

class NameExpression(PrefixSubparser):
    """name_expr: NAME"""
    def parse(self, parser, tokens):
        token = tokens.consume_expected('NAME')
        return ast.Identifier(token.value)

class UnaryOperatorExpression(PrefixSubparser):
    """prefix_expr: OPERATOR expr"""
    SUPPORTED_OPERATORS = ['-', '!']
    def parse(self, parser, tokens):
        token = tokens.consume_expected('OPERATOR')
        if token.value not in self.SUPPORTED_OPERATORS:
            raise ParserError('Unary operator {} is not supported'.format(token.value), token)
        right = Expression().parse(parser, tokens, self.get_precedence(token))
        if not right:
            raise ParserError('Excepted expression'.format(token.value), tokens.consume())
        return ast.UnaryOperator(token.value, right)

    def get_precedence(self, token):
        return self.PRECEDENCE['unary']

class GroupExpression(PrefixSubparser):
    """group_expr: LPAREN expr RPAREN"""
    def parse(self, parser, tokens):
        tokens.consume_expected('LPAREN')
        right = Expression().parse(parser, tokens)
        tokens.consume_expected('RPAREN')
        return right

class ArrayExpression(PrefixSubparser):
    """array_epxr: LBRACK list_of_expr? RBRACK"""
    def parse(self, parser, tokens):
        tokens.consume_expected('LBRACK')
        items = ListOfExpressions().parse(parser, tokens)
        tokens.consume_expected('RBRACK')
        return ast.Array(items)

class DictionaryExpression(PrefixSubparser):
    """dict_expr: LCBRACK (expr COLON expr COMMA)* RCBRACK"""
    def parse_keyvalues(self, parser, tokens):
        items = []
        while not tokens.is_end():
            key = Expression().parse(parser, tokens)
            if key:
                tokens.consume_expected('COLON')
                value = Expression().parse(parser, tokens)
                if value is None:
                    raise ParserError('Dictionary value excepted', tokens.consume())
                items.append((key, value))
            else:
                break
            if tokens.current().name == 'COMMA':
                tokens.consume_expected('COMMA')
            else:
                break
        return items

    def parse(self, parser, tokens):
        tokens.consume_expected('LCBRACK')
        items = self.parse_keyvalues(parser, tokens)
        tokens.consume_expected('RCBRACK')
        return ast.Dictionary(items)

class BinaryOperatorExpression(InfixSubparser):
    """infix_expr: expr OPERATOR expr"""
    def parse(self, parser, tokens, left):
        token = tokens.consume_expected('OPERATOR')
        right = Expression().parse(parser, tokens, self.get_precedence(token))
        if right is None:
            raise ParserError('Excepted expression'.format(token.value), tokens.consume())
        return ast.BinaryOperator(token.value, left, right)

    def get_precedence(self, token):
        return self.PRECEDENCE[token.value]

class CallExpression(InfixSubparser):
    """call_expr: NAME LPAREN list_of_expr? RPAREN"""
    def parse(self, parser, tokens, left):
        tokens.consume_expected('LPAREN')
        arguments = ListOfExpressions().parse(parser, tokens)
        tokens.consume_expected('RPAREN')
        return ast.Call(left, arguments)

    def get_precedence(self, token):
        return self.PRECEDENCE['call']

class SubscriptOperatorExpression(InfixSubparser):
    """subscript_expr: NAME LBRACK expr RBRACK"""
    def parse(self, parser, tokens, left):
        tokens.consume_expected('LBRACK')
        key = Expression().parse(parser, tokens)
        if key is None:
            raise ParserError('Subscript operator key is required', tokens.current())
        tokens.consume_expected('RBRACK')
        return ast.SubscriptOperator(left, key)

    def get_precedence(self, token):
        return self.PRECEDENCE['subscript']

class Expression(Subparser):
    """expr:
        number_expr | str_expr | name_expr | group_expr |
        array_epxr | dict_expr | prefix_expr | infix_expr |
        call_expr | subscript_expr
    """
    def get_prefix_subparser(self, token):
        return self.get_subparser(token, {
            'NUMBER': NumberExpression,
            'STRING': StringExpression,
            'NAME': NameExpression,
            'LPAREN': GroupExpression,
            'LBRACK': ArrayExpression,
            'LCBRACK': DictionaryExpression,
            'OPERATOR': UnaryOperatorExpression,
        })

    def get_infix_subparser(self, token):
        return self.get_subparser(token, {
            'OPERATOR': BinaryOperatorExpression,
            'LPAREN': CallExpression,
            'LBRACK': SubscriptOperatorExpression,
        })

    def get_next_precedence(self, tokens):
        if not tokens.is_end():
            token = token.current()
            parser = self.get_infix_subparser(token)
            if parser != None:
                return parser.get_precedence(token)
        return 0

    def parse(self, parser, tokens, precedence=0):
        subparser = self.get_prefix_subparser(tokens.current())
        if subparser != None:
            left = subparser.parse(parser, tokens)
            if left != None:
                while precedence < self.get_next_precedence(tokens):
                    op = self.get_infix_subparser(token.current()).parse(parser, tokens, left)
                    if op != None:
                        left = op
                return left

class ListOfExpressions(Subparser):
    """list_of_expr: (expr COMMA)*"""
    def parse(self, parser, tokens):
        items = []
        while not tokens.is_end():
            exp = Expression().parse(parser, tokens)
            if exp != None:
                items.append(exp)
            else:
                break
            if tokens.current().name == 'COMMA':
                tokens.consume_expected('COMMA')
            else:
                break
        return items
