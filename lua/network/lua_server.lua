local socket = require('socket')

function start_tcp_server()
	local server = assert(socket.tcp())
	local ip = socket.dns.toip('localhost')
	server:settimeout(2 / 1000)
	server:setsockname(ip, 5555)
	server:bind(ip, 5555)
	server:listen(32)

	print('----------- start server ---------')
	while true do
		local client = server:accept()
		if client then
			local data, err = client:receive()
			print('-------------------- data is:', data)
			if not err then
				client:send(data)
			end

			client:close()
		end
	end
end

function start_udp_server()
	local server = assert(socket.udp())
	local ip = socket.dns.toip('localhost')
	server:settimeout(2 / 1000)
	server:setsockname(ip, 5555)

	print('------ start server success ------')
	local counter = 0
	while true do
		local data, ip, port = server:receivefrom()
		if data then
			counter = counter + 1
			print('--------------------- receive from['..counter..']', ip, port, data)
			server:sendto(data, ip, port)
		end

		socket.sleep(0.01)
	end

	server:close()
end

-- start_tcp_server()
start_udp_server()
