local socket = require('socket')

function start_tcp_client()
	local client = assert(socket.tcp())
	if client:connect('localhost', 5555) then
		client:send('this is client test\n')
		local data, err = client:receive()
		if not err then
			print('-------------- client recv:', data)
		end
	end
	client:close()
end

function start_udp_client()
	local client = assert(socket.udp())
	local ip = socket.dns.toip('localhost')
	client:sendto('this is udp client testing', ip, 5555)
	local data, ip, port = client:receivefrom()
	if data then
		print('-------------- client recv(', ip, port, '):', data)
	end

	client:close()
end

start_udp_client()
