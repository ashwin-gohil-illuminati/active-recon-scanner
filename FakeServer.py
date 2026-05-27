import socket
import select
import random
import threading
import time

# Dictionary of ports that require probe strings and their expected probes
PROBE_PORTS = {
    5003: "EHLO example.com",
    5014: "GET / HTTP/1.1",
    5024: "EHLO example.com",
    5027: "GET / HTTP/1.1",
    5028: "USER anonymous",
    5032: "HEAD / HTTP/1.1",
    5034: "version",
    5036: "HELP",
    5039: "HEAD / HTTP/1.1",
    5041: "EHLO example.com",
    5043: "EHLO example.com",
    5044: "EHLO example.com",
    5051: "EHLO example.com",
    5055: "version",
    5061: "version",
    5062: "HEAD / HTTP/1.1",
    5065: "PING",
    5067: "PING",
    5068: "GET / HTTP/1.1",
    5072: "HEAD / HTTP/1.1",
    5078: "version",
    5079: "HEAD / HTTP/1.1",
    5080: "EHLO example.com",
    5086: "HEAD / HTTP/1.1",
    5088: "HEAD / HTTP/1.1",
    5094: "HEAD / HTTP/1.1",
    5114: "GET / HTTP/1.1",
    5118: "PING",
    5123: "HEAD / HTTP/1.1",
    5128: "version",
    5129: "EHLO example.com",
    5131: "USER anonymous",
    5136: "HELP",
    5140: "HELP",
    5146: "HEAD / HTTP/1.1",
    5149: "HELP",
    5160: "EHLO example.com",
    5162: "version",
    5164: "GET / HTTP/1.1",
    5165: "GET / HTTP/1.1",
    5176: "HEAD / HTTP/1.1",
    5188: "HELP",
    5193: "HEAD / HTTP/1.1",
    5194: "EHLO example.com",
    5202: "HEAD / HTTP/1.1",
    5204: "USER anonymous",
    5205: "HELP",
    5223: "HELP",
    5231: "USER anonymous",
    5233: "EHLO example.com",
    5238: "HEAD / HTTP/1.1",
    5244: "USER anonymous",
    5245: "PING",
    5249: "PING",
    5250: "version",
    5256: "EHLO example.com",
    5261: "PING",
    5272: "PING",
    5273: "HEAD / HTTP/1.1",
    5275: "HELP",
    5276: "EHLO example.com",
    5290: "HEAD / HTTP/1.1",
    5292: "version",
    5307: "PING",
    5308: "HEAD / HTTP/1.1",
    5315: "EHLO example.com",
    5328: "USER anonymous",
    5333: "HEAD / HTTP/1.1",
    5339: "HELP",
    5356: "GET / HTTP/1.1",
    5363: "version",
    5365: "version",
    5373: "version",
    5392: "GET / HTTP/1.1",
    5418: "version",
    5423: "EHLO example.com",
    5424: "GET / HTTP/1.1",
    5428: "GET / HTTP/1.1",
    5436: "USER anonymous",
    5439: "EHLO example.com",
    5442: "HELP",
    5443: "GET / HTTP/1.1",
    5453: "EHLO example.com",
    5459: "version",
    5468: "version",
    5474: "GET / HTTP/1.1",
    5483: "USER anonymous",
    5486: "HEAD / HTTP/1.1",
    5492: "PING",
    5495: "HELP",
    5497: "USER anonymous",
    5505: "HELP",
    5507: "version",
    5511: "GET / HTTP/1.1",
    5512: "GET / HTTP/1.1",
    5515: "version",
    5516: "USER anonymous",
    5522: "PING",
    5527: "version",
    5528: "HELP",
    5533: "USER anonymous",
    5540: "USER anonymous",
    5541: "GET / HTTP/1.1",
    5544: "USER anonymous",
    5555: "HEAD / HTTP/1.1",
    5556: "HEAD / HTTP/1.1",
    5564: "HEAD / HTTP/1.1",
    5565: "HEAD / HTTP/1.1",
    5569: "HELP",
    5571: "EHLO example.com",
    5574: "EHLO example.com",
    5576: "USER anonymous",
    5584: "version",
    5590: "HELP",
    5599: "GET / HTTP/1.1",
    5602: "USER anonymous",
    5611: "HEAD / HTTP/1.1",
    5619: "EHLO example.com",
    5624: "GET / HTTP/1.1",
    5625: "EHLO example.com",
    5633: "GET / HTTP/1.1",
    5639: "HEAD / HTTP/1.1",
    5640: "HEAD / HTTP/1.1",
    5642: "GET / HTTP/1.1",
    5657: "PING",
    5658: "version",
    5666: "PING",
    5686: "GET / HTTP/1.1",
    5688: "HEAD / HTTP/1.1",
    5691: "version",
    5700: "PING",
    5704: "HELP",
    5705: "GET / HTTP/1.1",
    5708: "HEAD / HTTP/1.1",
    5729: "EHLO example.com",
    5731: "USER anonymous",
    5732: "HEAD / HTTP/1.1",
    5736: "USER anonymous",
    5737: "version",
    5739: "version",
    5741: "HEAD / HTTP/1.1",
    5742: "HEAD / HTTP/1.1",
    5759: "PING",
    5767: "version",
    5770: "GET / HTTP/1.1",
    5786: "GET / HTTP/1.1",
    5792: "version",
    5794: "USER anonymous",
    5798: "EHLO example.com",
    5799: "USER anonymous",
    5800: "version",
    5801: "HEAD / HTTP/1.1",
    5824: "USER anonymous",
    5825: "version",
    5829: "PING",
    5830: "PING",
    5837: "GET / HTTP/1.1",
    5839: "HEAD / HTTP/1.1",
    5840: "version",
    5846: "GET / HTTP/1.1",
    5870: "version",
    5873: "PING",
    5882: "HELP",
    5883: "EHLO example.com",
    5894: "PING",
    5906: "version",
    5914: "HELP",
    5920: "HELP",
    5921: "PING",
    5923: "HEAD / HTTP/1.1",
    5926: "EHLO example.com",
    5927: "GET / HTTP/1.1",
    5929: "USER anonymous",
    5930: "PING",
    5931: "PING",
    5935: "USER anonymous",
    5945: "EHLO example.com",
    5947: "GET / HTTP/1.1",
    5949: "EHLO example.com",
    5951: "PING",
    5958: "GET / HTTP/1.1",
    5961: "HEAD / HTTP/1.1",
    5966: "EHLO example.com",
    5967: "version",
    5969: "HEAD / HTTP/1.1",
    5977: "USER anonymous",
    5979: "GET / HTTP/1.1",
    5982: "EHLO example.com",
    5989: "USER anonymous",
    5990: "version",
    5997: "HELP",
    5998: "version",
    6000: "HEAD / HTTP/1.1"
}

# Generate random banners for each port
def generate_banners():
    banners = {}
    
    # HTTP banners
    http_banners = [
        "HTTP/1.1 200 OK\r\nServer: Apache/2.4.41 (Ubuntu)\r\nContent-Type: text/html\r\n",
        "HTTP/1.1 404 Not Found\r\nServer: nginx/1.18.0\r\nContent-Type: text/html\r\n",
        "HTTP/1.1 301 Moved Permanently\r\nServer: Microsoft-IIS/10.0\r\nLocation: https://example.com\r\n",
        "HTTP/1.1 500 Internal Server Error\r\nServer: Caddy/2.4.6\r\nContent-Type: text/plain\r\n",
        "HTTP/1.1 403 Forbidden\r\nServer: lighttpd/1.4.55\r\nContent-Type: text/html\r\n"
    ]
    
    # FTP banners
    ftp_banners = [
        "220 (vsFTPd 3.0.3)\r\n",
        "220 Welcome to FileZilla Server 1.0.1\r\n",
        "220 ProFTPD 1.3.6a Server ready\r\n",
        "220 Microsoft FTP Service\r\n",
        "220 Pure-FTPd 1.0.49 Server\r\n"
    ]
    
    # SMTP banners
    smtp_banners = [
        "220 mail.example.com ESMTP Postfix\r\n",
        "220 smtp.gmail.com ESMTP\r\n",
        "220 Microsoft ESMTP MAIL Service ready\r\n",
        "220 example.com ESMTP Exim 4.94\r\n",
        "220 [127.0.0.1] ESMTP Service Ready\r\n"
    ]
    
    # SSH banners
    ssh_banners = [
        "SSH-2.0-OpenSSH_8.2p1 Ubuntu-4ubuntu0.2\r\n",
        "SSH-2.0-OpenSSH_7.4\r\n",
        "SSH-2.0-PuTTY_Release_0.74\r\n",
        "SSH-2.0-1.82_sshlib LibSSH\r\n",
        "SSH-2.0-OpenSSH_8.0\r\n"
    ]
    
    # Database banners
    db_banners = [
        "5.7.33-0ubuntu0.18.04.1\r\n",
        "PostgreSQL 13.3\r\n",
        "Redis server v=6.2.5\r\n",
        "MongoDB 4.4.6\r\n",
        "SQLite 3.31.1\r\n"
    ]
    
    # Custom banners for various services
    custom_banners = [
        "Welcome to Telnet Service\r\n",
        "IMAP4rev1 Server Ready\r\n",
        "* OK [CAPABILITY IMAP4rev1 STARTTLS] Dovecot ready\r\n",
        "+OK POP3 server ready\r\n",
        "220 NNTP Service Ready\r\n",
        "VNC Server: RFB 003.008\r\n",
        "RFB 003.889\r\n",
        "220 LDAP server ready\r\n",
        "SMB Server 2.1\r\n",
        "Rsync server version 3.1.3\r\n"
    ]
    
    # Assign banners to ports
    for port in range(5000, 6001):
        if port % 5 == 0:
            banners[port] = random.choice(http_banners)
        elif port % 5 == 1:
            banners[port] = random.choice(ftp_banners)
        elif port % 5 == 2:
            banners[port] = random.choice(smtp_banners)
        elif port % 5 == 3:
            banners[port] = random.choice(ssh_banners)
        elif port % 5 == 4:
            if random.random() < 0.5:
                banners[port] = random.choice(db_banners)
            else:
                banners[port] = random.choice(custom_banners)
    
    return banners

# Generate the banners
BANNERS = generate_banners()

# Function to handle a client connection
def handle_client(conn, port):
    try:
        # Check if this port requires a probe
        if port in PROBE_PORTS:
            expected_probe = PROBE_PORTS[port]
            
            # Wait for the probe
            data = conn.recv(1024).decode('utf-8', errors='ignore').strip()
            
            # Check if the probe matches
            if expected_probe in data:
                # Send the banner
                conn.send(BANNERS[port].encode('utf-8'))
            else:
                # Send a generic error response
                conn.send(b"Invalid request\r\n")
        else:
            # Send the banner immediately
            conn.send(BANNERS[port].encode('utf-8'))
    except Exception as e:
        print(f"Error handling connection on port {port}: {e}")
    finally:
        conn.close()

# Function to start a server on a specific port
def start_server(port):
    try:
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(('0.0.0.0', port))
        server_socket.listen(5)
        
        print(f"Server started on port {port}")
        
        # Use select to handle connections
        inputs = [server_socket]
        outputs = []
        
        while True:
            readable, writable, exceptional = select.select(inputs, outputs, inputs, 1)
            
            for s in readable:
                if s is server_socket:
                    # New connection
                    conn, addr = s.accept()
                    print(f"New connection on port {port} from {addr[0]}:{addr[1]}")
                    
                    # Handle the client in a new thread
                    client_thread = threading.Thread(target=handle_client, args=(conn, port))
                    client_thread.daemon = True
                    client_thread.start()
            
            for s in exceptional:
                print(f"Exceptional condition on {s}")
                inputs.remove(s)
                s.close()
                
    except Exception as e:
        print(f"Error starting server on port {port}: {e}")

# Main function to start all servers
def main():
    print("Starting robust fake server with multiple banners...")
    print(f"Listening on ports 5000-6000")
    print(f"Ports requiring probe: {len(PROBE_PORTS)}")
    
    # Start a server for each port in a separate thread
    for port in range(5000, 6001):
        server_thread = threading.Thread(target=start_server, args=(port,))
        server_thread.daemon = True
        server_thread.start()
        # Small delay to prevent overwhelming the system
        time.sleep(0.01)
    
    print("All servers started. Press Ctrl+C to stop.")
    
    try:
        # Keep the main thread alive
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nShutting down servers...")

if __name__ == "__main__":
    main()