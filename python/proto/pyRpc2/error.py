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

import pyRpc2.proto.rpc_pb2 as rpc_pb

class ProtoError(Exception):
    def __init__(self, message, rpc_err_code):
        self.message = message
        self.rpc_err_code = rpc_err_code

class BadRequestDataError(ProtoError):
    def __init__(self, message):
        super(BadRequestDataError, self).__init__(
                message, rpc_pb.ET_BAD_REQUEST_DATA)

class BadRequestProtoError(ProtoError):
    def __init__(self, message):
        super(BadRequestProtoError, self).__init__(
                message, rpc_pb.ET_BAD_REQUEST_PROTO)

class ServiceNotFoundError(ProtoError):
    def __init__(self, message):
        super(ServiceNotFoundError, self).__init__(
                message, rpc_pb.ET_SERVICE_NOT_FOUND)

class MethodNotFoundError(ProtoError):
    def __init__(self, message):
        super(MethodNotFoundError, self).__init__(
                message, rpc_pb.ET_METHOD_NOT_FOUND)

class RpcError(ProtoError):
    def __init__(self, message):
        super(RpcError, self).__init__(message, rpc_pb.ET_RPC_ERROR)

class RpcFailed(ProtoError):
    def __init__(self, message):
        super(RpcFailed, self).__init__(message, rpc_pb.ET_RPC_FAILED)

class InvalidRequestProtoError(ProtoError):
    def __init__(self, message):
        super(InvalidRequestProtoError, self).__init__(
                message, rpc_pb.ET_INVALID_REQUEST_PROTO)

class BadReplyProtoError(ProtoError):
    def __init__(self, message):
        super(BadReplyProtoError, self).__init__(
            message, rpc_pb.ET_BAD_REPLY_PROTO)

class UnknownHostError(ProtoError):
    def __init__(self, message):
        super(UnknownHostError, self).__init__(
                message, rpc_pb.ET_UNKNOWN_HOST)

class RpcIOError(ProtoError):
    def __init__(self, message):
        super(RpcIOError, self).__init__(message, rpc_pb.ET_IO_ERROR)
