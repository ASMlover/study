#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import socket

address = ('localhost', 5555)

def start_tcp_client():
	client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	if client is None:
		print('create client socket failed ...')
		exit()
	
	client.connect(address)
	try:
		print('client connect server success ...')
		client.send(b'this is python client test\n')
		data = client.recv(512)
		if data is not None:
			print('recv from server:', repr(data))
	except KeyboardInterrupt:
		client.close()
		import sys
		sys.exit()
	client.close()

def start_udp_client():
	client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	try:
		client.sendto(str.encode('this is python udp client test'), address)
		data, addr = client.recvfrom(1024)
		if data is not None:
			print('----------- recv from server', addr, ':', bytes.decode(data))
	except KeyboardInterrupt:
		client.close()
		import sys
		sys.exit()
	client.close()

if __name__ == '__main__':
	start_udp_client()
