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

import asyncio

DEF_BUFLEN = 256

@asyncio.coroutine
def handle_echo_client(loop, addr, port, message):
    reader, writer = yield from asyncio.open_connection(
            addr, port, loop=loop)
    print('Send message: %s' % message)
    writer.write(message.encode())

    data = yield from reader.read(DEF_BUFLEN)
    print('Received from {%s: %d}: %s' % (addr, port, data.decode()))

    print('Close the socket')
    writer.close()

def echo_client():
    loop = asyncio.get_event_loop()
    addr, port = '127.0.0.1', 5555
    clients = []
    for i in range(500):
        message = '<%d> Hello, world!' % i
        clients.append(handle_echo_client(loop, addr, port, message))
    loop.run_until_complete(asyncio.wait(clients))
    loop.close()

@asyncio.coroutine
def handle_echo_server(reader, writer):
    data = yield from reader.read(DEF_BUFLEN)
    message = data.decode()
    addr = writer.get_extra_info('peername')
    print('Received %s from %r' % (message, addr))

    print('Send: %r' % message)
    writer.write(data)
    yield from writer.drain()

    print('Close the client socket')
    writer.close()

def echo_server():
    loop = asyncio.get_event_loop()
    coro = asyncio.start_server(
            handle_echo_server, '127.0.0.1', 5555, loop=loop)
    server = loop.run_until_complete(coro)

    print('server on {}'.format(server.sockets[0].getsockname()))
    try:
        loop.run_forever()
    except KeyboardInterrupt:
        pass

    server.close()
    loop.run_until_complete(server.wait_closed())
    loop.close()

if __name__ == '__main__':
    import sys
    if sys.argv[1] == 's':
        echo_server()
    elif sys.argv[1] == 'c':
        echo_client()
