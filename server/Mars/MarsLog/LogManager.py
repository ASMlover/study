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

import json
import logging
import logging.handlers as LH
import platform
import sys
import traceback
import time
import types

CRITICAL = logging.CRITICAL
ERROR = logging.ERROR
WARNING = logging.WARN
WARN = logging.WARN
INFO = logging.INFO
DEBUG = logging.DEBUG

STREAM = 'stream'
SYSLOG = 'syslog'
FILE = 'file'

def logCompactTraceback(self):
    self.error(traceback.format_exc())

class LogManager(object):
    createdModules = set()
    logLevel = DEBUG
    logHandle = STREAM
    logTag = ''
    saLogTag = ''
    sysLogger = None

    @staticmethod
    def getLogger(moduleName):
        if LogManager.logHandle == SYSLOG and platform.system() == 'Linux' and LogManager.sysLogger != None:
            return logging.LoggerAdapter(LogManager.sysLogger, {'modulename': moduleName})

        if moduleName in LogManager.createdModules:
            return logging.getLogger(moduleName)
        logger = logging.getLogger(moduleName)
        logger.logLastExcept = types.MethodType(logCompactTraceback, logger)
        logger.setLevel(LogManager.logLevel)
        formatList = ['%(asctime)s', 'MarsEngine', LogManager.logTag, '%(name)s', '%(levelname)s', '%(message)s']
        if LogManager.logHandle == SYSLOG:
            if platform.system() == 'Linux':
                ch = LH.SysLogHandler('/dev/log', facility=LH.SysLogHandler.LOG_LOCAL1)
                LogManager.sysLogger = logger
                formatList = ['%(asctime)s', 'MarsEngine', LogManager.logTag, '%(modulename)s', '%(levelname)s', '%(message)s']
            else:
                ch = logging.FileHandler('%s_%s.log' % (LogManager.logTag, time.strftime('%Y%m%d%H%M%S')), encoding='utf8')
        elif LogManager.logHandle == FILE:
            ch = logging.FileHandler('%s_%s.log' % (LogManager.logTag, time.strftime('%Y%m%d%H%M%S')), encoding='utf8')
        else:
            ch = logging.StreamHandler()

        ch.setLevel(LogManager.logLevel)
        formatter = logging.Formatter(' - '.join(formatList))
        ch.setFormatter(formatter)
        logger.addHandler(ch)
        LogManager.createdModules.add(moduleName)

        if LogManager.logHandle == SYSLOG and platform.system() == 'Linux' and LogManager.sysLogger != None:
            return logging.LoggerAdapter(LogManager.sysLogger, {'modulename': moduleName})

        return logger

    @staticmethod
    def setLogLevel(level):
        LogManager.logLevel = level

    @staticmethod
    def setLogHandle(handle):
        LogManager.logHandle = handle

    @staticmethod
    def setLogTag(logTag):
        LogManager.logTag = logTag

    @staticmethod
    def getSALogger():
        if ('SALogger' in LogManager.createdModules):
            return SALogger(logging.getLogger('SALogger'))
        logger = logging.getLogger('SALogger')
        logger.setLevel(logging.INFO)
        if sys.platform.startswith('linux'):
            ch = LH.SysLogHandler('/dev/log', facility=LH.SysLogHandler.LOG_LOCAL0)
            ch.setLevel(logging.INFO)
            formatter = logging.Formatter(LogManager.saLogTag + ': %(message)s')
            ch.setFormatter(formatter)
            logger.addHandler(ch)
        else:
            fileHandler = logging.FileHandler('%s_%s.log' % (LogManager.saLogTag, time.strftime('%Y%d%m%H%M%S')), encoding='utf8')
            formatter = logging.Formatter('%(message)s')
            fileHandler.setFormatter(formatter)
            logger.addHandler(fileHandler)
        LogManager.createdModules.add('SALogger')
        return SALogger(logger)

    @staticmethod
    def setSALogTag(logTag):
        LogManager.saLogTag = logTag

class SALogger(object):
    def __init__(self, logger):
        self.logger = logger

    def log(self, operation, infoDict):
        logTime = time.strftime('%Y-%m-%d %H:%M:%S')
        jsonStr = json.dumps(infoDict, ensure_ascii=False)
        self.logger.info('[%s][%s]:%s' % (logTime, operation, jsonStr))


if __name__ == '__main__':
    logger = LogManager.getLogger('LogManager.Test')
    logger.info('info message')
    logger.debug('debug message')
    logger.error('error message')
    logger.warn('warning message')
    logger.critical('critical message')

    LogManager.setSALogTag('SAGameLog')
    saLogger = LogManager.getSALogger()
    saLogger.log('Test', {'testTime': 123456})

    try:
        raise Exception('LogManager')
    except:
        logger.logLastExcept()
