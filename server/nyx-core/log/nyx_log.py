#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2017 ASMlover. All rights reserved.
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

import json
import logging
import logging.handlers as LH
import platform
import time
import traceback

def _nyxlog_compact_traceback(self):
    self.error(traceback.format_exc())

# [LOG-LEVEL] 定义log的级别
NYXLOG_CRITICAL = logging.CRITICAL
NYXLOG_ERROR = logging.ERROR
NYXLOG_WARNING = logging.WARN
NXYLOG_WARN = logging.WARN
NYXLOG_INFO = logging.INFO
NYXLOG_DEBUG = logging.DEBUG

NYXLOG_STREAM = 'stream'
NYXLOG_SYSLOG = 'syslog'
NYXLOG_FILE = 'file'

class GameLogger(object):
    def __init__(self, logger):
        self._logger = logger

    def log(self, operation, info_dict):
        logtime = time.strftime("%Y-%m-%d %H:%M:%S")
        jsonstr = json.dumps(info_dict, ensure_ascii=False)
        self._logger.info('[%s][%s] - %s' % (logtime, operation, jsonstr))

class LogManager(object):
    created_modules = set()
    log_level = NYXLOG_DEBUG
    log_handle = NYXLOG_STREAM
    log_tag = ''
    game_log_tag = ''
    sys_logger = None

    @staticmethod
    def set_log_level(level):
        LogManager.log_level = level

    @staticmethod
    def set_log_handle(handle):
        LogManager.log_handle = handle

    @staticmethod
    def set_log_tag(log_tag):
        LogManager.log_tag = log_tag

    @staticmethod
    def set_game_log_tag(log_tag):
        LogManager.game_log_tag = log_tag

    @staticmethod
    def get_logger(module_name):
        pass

    @staticmethod
    def get_game_logger():
        pass
