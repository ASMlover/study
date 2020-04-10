#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2020 ASMlover. All rights reserved.
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

WAIT_MSG = 1
READ_MSG = 2

async def on_serve(reader, writer):
	writer.write("*".encode())
	await writer.drain()

	status = WAIT_MSG
	while True:
		data = await reader.read(1024)
		if not data:
			break
		recv_data = data.decode()

		send_data = []
		for c in recv_data:
			if status == WAIT_MSG:
				if c == '^':
					status = READ_MSG
			elif status == READ_MSG:
				if c == '$':
					status = WAIT_MSG
				else:
					send_data.append(chr(ord(c) + 1))

		if send_data:
			writer.write(''.join(send_data).encode())
			await writer.drain()
	writer.close()

async def main():
	server = await asyncio.start_server(on_serve, '0.0.0.0', 5555)
	loop, addr = asyncio.get_event_loop(), server.sockets[0].getsockname()
	print(f"Serving on {addr} with {loop} ...")

	await server.serve_forever()

if __name__ == '__main__':
	try:
		asyncio.run(main())
	except KeyboardInterrupt:
		print("Serve finished")
	except Exception as e:
		print(e)
