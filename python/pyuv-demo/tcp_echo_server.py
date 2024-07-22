import signal
import pyuv

def on_read(client, data, error):
    if data is None:
        client.close()
        clients.remove(client)
        return
    client.write(data)

def on_connection(server, error):
    client = pyuv.TCP(server.loop)
    server.accept(client)
    clients.append(client)
    client.start_read(on_read)

def signal_cb(handle, signum):
    for c in clients:
        c.close()

    signal_handle.close()
    server.close()


print(f"PyUV version {pyuv.__version__}")

loop = pyuv.Loop.default_loop()
clients = []

server = pyuv.TCP(loop)
server.bind(("0.0.0.0", 5555))
server.listen(on_connection)

signal_handle = pyuv.Signal(loop)
signal_handle.start(signal_cb, signal.SIGINT)

loop.run()
