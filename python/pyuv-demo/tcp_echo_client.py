
import asyncio

async def on_echo():
    reader, writer = await asyncio.open_connection("127.0.0.1", 5555)

    wbuf = "aaaabbbbbcccc"
    writer.write(wbuf.encode())
    print(f"CLIENT: send - {wbuf}")

    rbuf = ""
    while True:
        data = await reader.read(1024)
        if not data:
            break

        rbuf += data.decode()
        break
    print(f"CLIENT: recv - {rbuf}")

    writer.close()
    print(f"CLIENT: disconnecting ...")

if __name__ == "__main__":
    try:
        asyncio.run(on_echo())
    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(e)
