import asyncio
import websockets
import socket
import json

UDP_IP = "127.0.0.1"
UDP_PORT = 5006
WS_PORT = 8081

connected_clients = set()

async def ws_handler(websocket):
    connected_clients.add(websocket)
    try:
        await websocket.wait_closed()
    finally:
        connected_clients.remove(websocket)

async def udp_listener():
    loop = asyncio.get_running_loop()
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))
    sock.setblocking(False)

    print(f"Listening for UDP telemetry from C++ on port {UDP_PORT}...")
    while True:
        data, addr = await loop.sock_recvfrom(sock, 1024)
        if not connected_clients:
            continue
        
        # C++'tan gelen beklenen format: "time,alt,vel,qx,qy,qz,qw"
        data_str = data.decode('utf-8').strip()
        
        # Eğer fizik motoru patlayıp NaN ürettiyse (tarayıcıyı çökerteceği için) bu paketi atla
        if "nan" in data_str.lower():
            continue
            
        parts = data_str.split(',')
        if len(parts) >= 7:
            try:
                payload = json.dumps({
                    "time": float(parts[0]),
                    "alt": float(parts[1]),
                    "vel": float(parts[2]),
                    "thrust": float(parts[3]), 
                    "qx": float(parts[4]),
                    "qy": float(parts[5]),
                    "qz": float(parts[6]),
                    "qw": float(parts[7])
                })
                # Broadcast to all connected WebSockets
                websockets.broadcast(connected_clients, payload)
            except Exception as e:
                pass

async def main():
    async with websockets.serve(ws_handler, "localhost", WS_PORT):
        print(f"WebSocket server started on ws://localhost:{WS_PORT}")
        await udp_listener()

if __name__ == "__main__":
    asyncio.run(main())
