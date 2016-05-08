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

import logging
import socket
import socketserver

from pyRpc2 import error
from pyRpc2 import logger
from pyRpc2.controller import RpcController
from pyRpc2.proto import rpc_pb2 as rpc_pb

_logger = logger.get_logger(__name__)

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

        read_data = self.rfile.read()

        rpc_reply = self.validate_and_execute_request(read_data)
        _logger.debug('SocketHandler.handle: to client: %s', rpc_reply)

        self.wfile.write(rpc_reply.SerializeToString())
        self.request.shutdown(socket.SHUT_RDWR)

    def validate_and_execute_request(self, request_data):
        pass

    def parse_service_request(self, client_byte_stream):
        pass

    def retrive_service(self, service_name):
        pass

    def retrive_method(self, service, method_name):
        pass

    def retrive_proto_request(self, service, method, request):
        pass

    def call_method(self, service, method, proto_request):
        pass

    def hanle_error(self, e):
        pass

class ThreadTcpServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    socketserver.allow_reuse_address = True

class RpcServer(object):
    service_map = {}

    def __init__(self, host='localhost', port=5555):
        self.host = host
        self.port = port

    def register_service(self, service):
        pass

    def run(self):
        pass
