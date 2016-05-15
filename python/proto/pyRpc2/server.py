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

import socket
import socketserver
import threading

from pyRpc2 import error
from pyRpc2 import logger
from pyRpc2.controller import RpcController
from pyRpc2.rpc_pb2 import rpc_pb

_logger = logger.get_logger('pyRpc2.Server')

class Callback(object):
    def __init__(self):
        self.invoked = False
        self.reply = None

    def run(self, reply):
        self.reply = reply
        self.invoked = True

class SocketHandler(socketserver.StreamRequestHandler):
    def handle(self):
        _logger.debug('SocketHandler.handle: got a request')

        recv = self.rfile.read()
        # TODO:

    def validate_execute_request(self, input_data):
        try:
            # TODO:
            request = None

    def parse_service_request(self, byte_stream):
        pass

    def retrieve_service(self, service_name):
        pass

    def retrieve_method(self, service, method_name):
        pass

    def retrieve_proto_request(self, service, method, request):
        pass

    def call_method(self, service, method, proto_request):
        pass

    def handle_with_error(self, e):
        pass

class RpcServer(object):
    service_map = {}

    def __init__(self, host='localhost', port=5555):
        self.host = host
        self.port = port

    def register_service(self, service):
        RpcServer.service_map[service.GetDescriptor().full_name] = service

    def run(self):
        _logger.info('RpcServer.run: running server on port %d', self.port)
        pass
