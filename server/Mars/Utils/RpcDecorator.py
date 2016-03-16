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

import PathHelper as PH
PH.addPathes('../')

from MarsLog.LogManager import LogManager
from RpcArgument import ConvertError, RpcArgument, Avatar, MailBox
from EntityManager import EntityManager

MARS_RPC_CLIENT = 0 # client => server
MARS_RPC_SERVER = 1 # server => server
MARS_RPC_CS_ALL = 2 # client/server => server
MARS_RPC_CLIENT_STUB = 3 # server => client

_logger = LogManager.getLogger('Utils.RpcDecorator')

class RpcMethod(object):
    def __init__(self, func, rpcType, argTypes, pub):
        super(RpcMethod, self).__init__()
        self.func = func
        self.rpcType = rpcType
        self.argTypes = argTypes
        self.pub = pub

    def call(self, entity, placeHolder, paramters):
        if not isinstance(paramters, dict):
            _logger.warn('call: paramter decode failed in RPC %s(%s)', self.func.__name__, str(self.argTypes))
            return
        args = []
        first = True

        autoParameters = paramters.get('_')
        if autoParameters:
            paramters = autoParameters
            if self.argTypes:
                paramters = autoParameters[0]
            else:
                return self.func(entity, *autoParameters)

        for argType in self.argTypes:
            if first:
                first = False
                if isinstance(argType, Avatar):
                    avatar = EntityManager.getEntity(placeHolder)
                    args.append(avatar)
                    continue
                elif isinstance(argType, MailBox):
                    args.append(placeHolder)
                    continue

            try:
                arg = paramters[argType.getName()]
            except KeyError:
                _logger.warn('call: paramter %s not found in RPC %s, use default value', argType.getName(), self.func.__name__)
                arg = argType.defaultValue()

            try:
                if arg == None and argType.getType() == 'Uuid':
                    arg = None
                else:
                    arg = argType.convert(arg)
            except ConvertError as e:
                _logger.error('call: paramter %s cannot convert input %s for RPC %s exception %s', argType.getName(), str(arg), self.func.__name__, str(e))
                return
            args.append(arg)

        return self.func(entity, *args)

def rpcMethod(rpcType, argTypes=(), pub=True):
    assert rpcType in (MARS_RPC_CLIENT, MARS_RPC_SERVER, MARS_RPC_CS_ALL, MARS_RPC_CLIENT_STUB), 'type must be one of (MARS_RPC_CLIENT, MARS_RPC_SERVER, MARS_RPC_CS_ALL, MARS_RPC_CLIENT_STUB)'
    assert pub in (True, False), 'pub must be True or False'

    for argType in argTypes:
        assert isinstance(argType, RpcArgument), '[%s] args type error' % str(argType)

    def rpcMethodWrapper(func):
        method = RpcMethod(func, rpcType, argTypes, pub)
        def callRpcMethod(self, *args):
            funcForReload = func
            if rpcType == MARS_RPC_CLIENT_STUB:
                return method.call(self, None, *args)
            else:
                return method.call(self, *args)
        callRpcMethod.rpcMethod = method
        return callRpcMethod
    return rpcMethodWrapper

def exposeToClient(method):
    try:
        rpcType = method.rpcMethod.rpcType
        if rpcType == MARS_RPC_CLIENT or rpcType == MARS_RPC_CS_ALL:
            return True
        return False
    except AttributeError:
        return False

def exposeToServer(method):
    try:
        rpcType = method.rpcMethod.rpcType
        if rpcType == MARS_RPC_SERVER or rpcType == MARS_RPC_CS_ALL:
            return True
        return False
    except AttributeError:
        return False
