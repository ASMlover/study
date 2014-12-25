#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import socket
import time
from threading import Thread

address = ('', 5555)

def start_tcp_server():
	server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	server.bind(address)
	server.listen(32)
	
	print('----- start server success -----')
	running = True
	def handle_server():
		while running:
			client, addr = server.accept()
			if client is not None:
				data = client.recv(1024)
				if not data:
					client.close()
					break
				print('----- recevice from client:', data)
				client.send(data)
				client.close()
	
	trd = Thread(target=handle_server)
	trd.start()
	
	try:
		while True:
			time.sleep(1)
	except KeyboardInterrupt:
		running = False
		# server.shutdown(socket.SHUT_RDWR)
		server.close()

def start_udp_server():
	server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	server.bind(address)

	print('----- start udp server success -----')
	running = True
	def handle_server():
		while running:
			data, addr = server.recvfrom(1024)
			if data is not None:
				print('---------- recv from:', addr, data)
				server.sendto(data, addr)
			else:
				break
	trd = Thread(target=handle_server)
	trd.start()

	try:
		while True:
			time.sleep(1)
	except KeyboardInterrupt:
		running = False
		server.close()


if __name__ == '__main__':
	start_udp_server()
