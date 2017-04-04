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
import logging.handlers as _LH
import new
import platform
import time
import traceback

# 定义Logging的级别
CRITICAL = logging.CRITICAL
ERROR = logging.ERROR
WARNING = logging.WARN
WARN = logging.WARN
INFO = logging.INFO
DEBUG = logging.DEBUG

_STREAM = 'stream'
_SYSLOG = 'syslog'
_FILE = 'file'

class GameLogger(object):
    def __init__(self, logger):
        self.logger = logger

    def log(self, operation, info_dict):
        logtime = time.strftime('%Y-%m-%d %H:%M:%S')
        jsonstr = json.dumps(info_dict, ensure_ascii=False)
        self.logger.info('[%s][%s] - %s', logtime, operation, jsonstr)

def _log_compact_traceback(logger):
    logger.error(traceback.format_exc())

class LogManager(object):
    _exists_modules = set()
    _level = DEBUG
    _handler = _STREAM
    _tag = 'NyxEngine.Core'
    _game_tag = 'NyxEngine.Game'
    _sys_logger = None

    @staticmethod
    def set_level(level):
        LogManager._level = level

    @staticmethod
    def set_handler(handler):
        LogManager._handler = handler

    @staticmethod
    def set_tag(tag):
        LogManager._tag = tag

    @staticmethod
    def set_game_tag(tag):
        LogManager._game_tag = tag

    @staticmethod
    def get_logger(modulename):
        ptname = platform.system()
        if (LogManager._handler == _SYSLOG
                and ptname == 'Linux'
                and LogManager._sys_logger is not None):
            return logging.LoggerAdapter(
                    LogManager._sys_logger, {'modulename': modulename})

        if modulename in LogManager._exists_modules:
            return logging.getLogger(modulename)

        logger = logging.getLogger(modulename)
        logger._log_exception = new.instancemethod(
                _log_compact_traceback, logger, logger.__class__)
        logger.setLevel(LogManager._level)
        formatlist = [
            '%(asctime)s',
            'NyxEngine',
            LogManager._tag,
            '%(name)s',
            '%(levelname)s',
            '%(message)s'
        ]
        if LogManager._handler == _SYSLOG:
            if ptname == 'Linux':
                ch = _LH.SysLogHandler(
                        '/dev/log', facility=_LH.SysLogHandler.LOG_LOCAL1)
                LogManager._sys_logger = logger
                formatlist = [
                    '%(asctime)s',
                    'NyxEngine',
                    LogManager._tag,
                    '%(modulename)s',
                    '%(levelname)s',
                    '%(message)s'
                ]
            else:
                ch = logging.FileHandler('%s.%s.log' % (LogManager._tag,
                    time.strftime('%Y%m%d%H%M%S')), encoding='utf-8')
        elif LogManager._handler == _FILE:
            ch = logging.FileHandler('%s.%s.log' % (LogManager._tag,
                time.strftime('%Y%m%d%H%M%S')), encoding='utf-8')
        else:
            ch = logging.StreamHandler()
        ch.setLevel(LogManager._level)
        formatter = logging.Formatter(' - '.join(formatlist))
        ch.setFormatter(formatter)
        logger.addHandler(ch)
        LogManager._exists_modules.add(modulename)

        if (LogManager._handler == _SYSLOG
                and ptname == 'Linux'
                and LogManager._sys_logger is not None):
            return logging.LoggerAdapter(
                    LogManager._sys_logger, {'modulename': modulename})
        return logger

    @staticmethod
    def get_game_logger():
        if 'GameLogger' in LogManager._exists_modules:
            return GameLogger(logging.getLogger('GameLogger'))

        logger = logging.getLogger('GameLogger')
        logger.setLevel(INFO)
        if platform.system() == 'Linux':
            ch = _LH.SysLogHandler(
                    '/dev/log', facility=_LH.SysLogHandler.LOG_LOCAL0)
            ch.setLevel(INFO)
            formatter = logging.Formatter(
                    '%s: %(message)s' % LogManager._game_tag)
        else:
            ch = logging.FileHandler('%s.%s.log' % (LogManager._game_tag,
                time.strftime('%Y%m%d%H%M%S')), encoding='utf-8')
            formatter = logging.Formatter('%(message)s')
        ch.setFormatter(formatter)
        logger.addHandler(ch)

        LogManager._exists_modules.add('GameLogger')
        return GameLogger(logger)
