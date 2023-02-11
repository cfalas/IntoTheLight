import asyncio
import websockets
from logging import getLogger, INFO, StreamHandler

logger = getLogger('websockets')
logger.setLevel(INFO)
logger.addHandler(StreamHandler())

clients = set()

async def handler(websocket, path):
    global clients
    clients.add(websocket)
    try:
        async for message in websocket:
            for client in clients:
                if client != websocket:
                    await client.send(message)
            print(message)
    finally:
        clients.remove(websocket)

start_server = websockets.serve(handler, host='127.0.0.1', port=8765)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()