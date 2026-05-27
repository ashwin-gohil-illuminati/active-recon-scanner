
#FakeServerAllBanners.py
import socket
import select
import random
import sys

# --- Configuration ---
START_PORT = 5000
END_PORT = 6000

# A pool of realistic-looking banners to choose from
BANNER_LIST = [
    "SSH-2.0-OpenSSH_8.9p1 Ubuntu-3ubuntu0.1",
    "SSH-2.0-OpenSSH_7.4p1 Debian-10+deb9u7",
    "220 (vsFTPd 3.0.3)",
    "220 (FileZilla Server 0.9.60 beta)",
    "HTTP/1.1 400 Bad Request\r\nServer: nginx/1.18.0 (Ubuntu)\r\nContent-Type: text/html\r\n",
    "HTTP/1.1 200 OK\r\nServer: Apache/2.4.41 (Ubuntu)\r\n",
    "220 mail.example.com ESMTP Postfix (Ubuntu)",
    "SMTP Ready",
    "Welcome to My Custom Service v1.2",
    "REDIS-001-RESPONSE",
    "*1\r\n$4\r\nPONG\r\n", # Redis-like response
    "IMAP4rev1 Service Ready",
    "+OK POP3 server ready",
    "MySQL-5.7.33-0ubuntu0.18.04.1",
    "Elasticsearch/7.10.2",
    "Git Protocol Version 2",
]

def main():
    """
    Main function to set up and run the server using select.
    All ports will provide a banner upon connection.
    """
    server_sockets = []
    
    print("Starting Fake Banner Server (All Ports Have Banners)...")
    print(f"Attempting to listen on ports {START_PORT} to {END_PORT}")

    # --- 1. Create, bind, and listen on all ports ---
    for port in range(START_PORT, END_PORT + 1):
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # This allows you to re-run the script immediately after closing it
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        try:
            # Bind the socket to the address and port
            server_socket.bind(('0.0.0.0', port))
            # Listen for incoming connections
            server_socket.listen(5)
            print(f"[+] Server successfully listening on port {port}")
            server_sockets.append(server_socket)
        except OSError as e:
            print(f"[-] Could not start server on port {port}: {e}")
            # Close the socket if binding failed
            server_socket.close()

    if not server_sockets:
        print("\n[!] No servers were started. Exiting.")
        sys.exit(1)

    print(f"\n[*] {len(server_sockets)} servers are now active. Press Ctrl+C to stop.")

    # --- 2. Main loop using select ---
    try:
        while True:
            # 'select' waits until at least one socket is ready to be read (i.e., a connection is incoming)
            # The 1.0 is a timeout in seconds, so the loop checks for a shutdown signal at least once per second.
            readable, _, _ = select.select(server_sockets, [], [], 1.0)

            for sock in readable:
                try:
                    # Accept the connection. This will not block because select told us it's ready.
                    conn, addr = sock.accept()
                    # print(f"[*] Connection from {addr[0]}:{addr[1]} on port {sock.getsockname()[1]}")

                    # --- MODIFICATION: Always send a banner ---
                    # The random check for silent ports has been removed.
                    banner = random.choice(BANNER_LIST)
                    try:
                        conn.sendall(banner.encode('utf-8'))
                        # print(f"[+] Sent banner on port {sock.getsockname()[1]}")
                    except BrokenPipeError:
                        # Client disconnected before we could send
                        pass
                    
                    # Close the client connection immediately
                    conn.close()

                except ConnectionResetError:
                    # This can happen if the client disconnects abruptly
                    continue

    except KeyboardInterrupt:
        print("\n[*] Shutting down server...")
    finally:
        # --- 3. Clean up all server sockets on exit ---
        print("[*] Closing all server sockets.")
        for s in server_sockets:
            s.close()

if __name__ == "__main__":
    main()