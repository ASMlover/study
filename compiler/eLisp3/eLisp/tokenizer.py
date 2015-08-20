#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Copyright (c) 2015 ASMlover. All rights reserved.
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

import logging
import os
import re
import sys

logger = logging.getLogger('elisp.tokenizer')

TOKEN_LPAREN    = 'TOKEN_LPAREN'
TOKEN_RPAREN    = 'TOKEN_RPAREN'
TOKEN_QUOTE     = 'TOKEN_QUOTE'
TOKEN_STRING    = 'TOKEN_STRING'
TOKEN_NUMBER    = 'TOKEN_NUMBER'
TOKEN_SYMBOL    = 'TOKEN_SYMBOL'
TOKEN_CHAR      = 'TOKEN_CHAR'
TOKEN_UNKNOWN   = 'TOKEN_UNKNOWN'

REGEX_NUMBER = re.compile('^[+-]?(\d*\.?\d+|\d+\.?\d*)([eE][+-]?\d+)?$')
REGEX_SYMBOL = re.compile('^([a-zA-Z0-9+-<>=:$\*\?!]+)$')

class TokenizerException(RuntimeError):
    def __init__(self, message, state):
        self.message = message
        self.state = state

class State(object):
    STATE_NONE          = -1
    STATE_STRING        = 0
    STATE_SYMBOL        = 1
    STATE_CHAR          = 2
    STATE_STRING_CHAR   = 3

    states = {
        STATE_NONE:         'STATE_NONE',
        STATE_STRING:       'STATE_STRING',
        STATE_SYMBOL:       'STATE_SYMBOL',
        STATE_CHAR:         'STATE_CHAR',
        STATE_STRING_CHAR:  'STATE_STRING_CHAR',
    }

    def __init__(self, index=None, char=None):
        self.index = index
        self.char  = char
        self.state = self.STATE_NONE

    def state_name(self, state):
        return self.states.get(state, 'UNKNOWN')

    def set(self, state=STATE_NONE):
        logger.debug('STATE %s -> %s' % (
            self.state_name(self.state), self.state_name(state)))
        self.state = state

    @property
    def is_string(self):
        return self.state == self.STATE_STRING

    @property
    def is_symbol(self):
        return self.state == self.STATE_SYMBOL

    @property
    def is_char(self):
        return self.state == self.STATE_CHAR

    @property
    def is_string_char(self):
        return self.state == self.STATE_STRING_CHAR

    def __repr__(self):
        return 'TokenizerState state=%s {index=%d, char=%c}' % (
                self.state_name(self.state), self.index, self.char)
