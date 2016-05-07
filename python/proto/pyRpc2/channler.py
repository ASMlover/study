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
import google.protobuf.service as service

from pyRpc2 import logger
from pyRpc2.controller import RpcController
import pyRpc2.error as error
import pyRpc2.proto.rpc_pb2 as rpc_pb

_logger = logger.get_logger(__name__)

class SocketCreator(object):
    def create_socket(self):
        return socket.socket(socket.AF_INET, socket.SOCK_STREAM)

class RpcChannel(service.RpcChannel):
    def __init__(self, host='localhost', port=5555,
                 socket_creator=SocketCreator()):
        self.host = host
        self.port = port
        self.socket_creator = socket_creator

    def new_controller(self):
        return RpcController()

    def validate_request(self, request):
        if not request.IsInitialized():
            raise error.BadRequestProtoError('Request proto error')

    def open_socket(self, host, port):
        fd = self.socket_creator.create_socket()
        try:
            fd.connect((host, port))
        except socket.gaierror:
            self.close_socket(fd)
            msg = 'Could not find host %s' % host
            raise error.UnknownHostError(msg)
        except socket.error:
            self.close_socket(fd)
            msg = 'Could not open IO for %s:%d' % (host, port)
            raise error.RpcIOError(msg)
        return fd

    def close_socket(self, fd):
        try:
            fd and fd.close()
        except:
            pass

    def create_rpc_request(self, method, request):
        rpc_request = rpc_pb.Request()
        rpc_request.request_proto = request.SerializeToString()
        rpc_request.service_name = method.containing_service.full_name
        rpc_request.method_name = method.name

        return rpc_request

    def write_rpc_message(self, fd, rpc_request):
        try:
            wfile = fd.makefile('w')
            wfile.write(rpc_request.SerializeToString())
            wfile.flush()
            fd.shutdown(socket.SHUT_WR)
        except socket.error:
            self.close_socket(fd)
            raise error.RpcIOError('Error writing data to server')

    def read_rpc_message(self, fd):
        try:
            rfile = fd.makefile('r')
            byte_stream = rfile.read()
        except socket.error:
            raise error.RpcIOError('Error reading data from server')
        finally:
            self.close_socket(fd)

        return byte_stream

    def parse_reply(self, byte_stream, reply_class):
        pass

    def CallMethod(self, method, controller, request, reply_class, done):
        pass
