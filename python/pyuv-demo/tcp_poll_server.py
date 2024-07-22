#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import errno
import sys
import socket
import signal
import weakref
import pyuv

STOPSIGNALS = (signal.SIGINT, signal.SIGTERM)
NONBLOCKING = (errno.EAGAIN, errno.EWOULDBLOCK)

class Connection(object):
	def __init__(self, sock, addr, loop):
		self.sock = sock
		self.addr = addr
		self.sock.setblocking(False)
		self.buf = ""
		self.watcher = pyuv.Poll(loop, self.sock.fileno())
		self.watcher.start(pyuv.UV_READABLE, self.io_cb)

	def reset(self, events):
		self.watcher.start(events, self.io_cb)

	def close(self):
		self.watcher.stop()
		self.watcher = None
		self.sock.close()

	def handle_error(self, msg):
		print(f"Connection.handle_error: {msg}")
		self.close()

	def handle_read(self):
		try:
			buf = self.sock.recv(1024)
		except socket.error as err:
			if err.args[0] not in NONBLOCKING:
				self.handle_error(f"error reading from {self.sock}")
		if buf:
			self.buf += buf.decode()
			self.reset(pyuv.UV_READABLE | pyuv.UV_WRITABLE)
		else:
			self.handle_error("connection closed by peer")

	def handle_write(self):
		try:
			sent = self.sock.send(self.buf.encode())
		except socket.error as err:
			if err.args[0] not in NONBLOCKING:
				self.handle_error(f"error writing to {self.sock}")
		else:
			self.buf = self.buf[sent:]
			if not self.buf:
				self.reset(pyuv.UV_READABLE)

	def io_cb(self, watcher, revents, error):
		if error is not None:
			return

		if revents & pyuv.UV_READABLE:
			self.handle_read()
		elif revents & pyuv.UV_WRITABLE:
			self.handle_write()

class Server(object):
	def __init__(self, addr):
		self.sock = socket.socket()
		self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self.sock.bind(addr)
		self.sock.setblocking(False)
		self.addr = self.sock.getsockname()
		self.loop = pyuv.Loop.default_loop()
		self.poll_watcher = pyuv.Poll(self.loop, self.sock.fileno())
		self._async = pyuv.Async(self.loop, self.async_cb)
		self.conns = weakref.WeakValueDictionary()
		self.signal_watchers = set()

	def start(self):
		self.sock.listen(socket.SOMAXCONN)
		self.poll_watcher.start(pyuv.UV_READABLE, self.io_cb)
		for sig in STOPSIGNALS:
			handle = pyuv.Signal(self.loop)
			handle.start(self.signal_cb, sig)
			self.signal_watchers.add(handle)
		self.loop.run()

	def stop(self):
		self.poll_watcher.stop()
		for watcher in self.signal_watchers:
			watcher.stop()
		self.signal_watchers.clear()
		self.sock.close()
		for conn in self.conns.values():
			conn.close()

	def handle_error(self, msg):
		print(f"Server.handle_error: {msg}")
		self.stop()

	def signal_cb(self, handle, signum):
		self._async.send()

	def async_cb(self, handle):
		handle.close()
		self.stop()

	def io_cb(self, watcher, revents, error):
		try:
			while True:
				try:
					sock, addr = self.sock.accept()
					print(f"Server.io_cb accept connection: {sock} {addr}")
				except socket.error as err:
					if err.args[0] in NONBLOCKING:
						break
					else:
						raise Exception("{err}")
				else:
					self.conns[addr] = Connection(sock, addr, self.loop)
		except Exception:
			self.handle_error("error accepting a connection")

if __name__ == "__main__":
	server = Server(("127.0.0.1", 5555))
	server.start()
