
import socket

def start_echo_server(ip, port):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((ip, port))
    server_socket.listen(1)
    print(f"[*] Listening on {ip}:{port}")

    client_socket, address = server_socket.accept()
    print(f"[+] Accepted connection from {address[0]}:{address[1]}")

    while True:
        data = client_socket.recv(1024)
        if not data:
            break
        client_socket.sendall(data)

    client_socket.close()

start_echo_server('0.0.0.0', 80)