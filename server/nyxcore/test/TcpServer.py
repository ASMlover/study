#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2018 ASMlover. All rights reserved.
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

import _nyxcore

class TcpHandler(_nyxcore.rpc_handler):
    def __init__(self):
        super(TcpHandler, self).__init__(_nyxcore.service_type.gategame)

class TcpServer(_nyxcore.nyx_server):
    def __init__(self, ip, port, conn_handler=None,
            reuse_addr=False, service_type=_nyxcore.service_type.none):
        super(TcpServer, self).__init__()
        self.conn_handler = conn_handler
        self.ip = ip
        self.port = port
        self.started = False
        self.set_reuse_addr(reuse_addr)
        self.try_bind()
        self.listen(50)

    def try_bind(self):
        for x in xrange(100):
            try:
                self.bind(self.ip, self.port)
                break
            except:
                self.port += 1
        else:
            raise StandardError('failed to find a usable port to bind')
        self.started = True

    def get_endpoint(self):
        return (self.ip, self.port)

    def set_connection_handler(self, handler):
        self.conn_handler = handler

    def reset_connection(self):
        handler = TcpHandler()
        self.set_handler(handler)
        super(TcpServer, self).reset_connection()

    def on_close(self):
        print 'TcpServer.on_close'
