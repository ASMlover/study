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

import threading
import time

from pyRpc2 import logger
from pyRpc2.channel import RpcChannel
from pyRpc2.error import RpcError

class RpcThread(threading.Thread):
    def __init__(self, method, service, controller, request, callback):
        super(RpcThread, self).__init__()
        self.method = method
        self.service = service
        self.controller = controller
        self.request = request
        self.callback = callback
        self.setDaemon(True)

    def run(self):
        if callable(self.callback):
            self.method(self.service, self.controller, self.request,
                    type('', (), {'run': lambda *args: self.callback(self.request, args[1])})())
        else:
            self.method(self.service,
                    self.controller, self.request, self.callback)

class RpcService(object):
    def __init__(self, service_stub_class, host, port):
        self.service_stub_class = service_stub_class
        self.host = host
        self.port = port

        self.channel = RpcChannel(host=self.host, port=self.port)
        self.service = self.service_stub_class(self.channel)

        for method in service_stub_class.GetDescriptor().methods:
            rpc = lambda request, timeout=None, callback=None: self.call(
                    service_stub_class.__dict__[method.name],
                    request, timeout, callback)
            self.__dict__[method.name] = rpc

    def call(self, rpc, request, timeout=None, callback=None):
        pass
