#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

class Socket {
    
    private:
        int sockfd_; 

    public:
        Socket();
        ~Socket();
        int prepareSocket();
        int getSocket();
        int unAssign();
            
};
#endif