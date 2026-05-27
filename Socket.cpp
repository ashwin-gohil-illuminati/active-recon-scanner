#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
//#include <iostream>
#include "Socket.h"



Socket::Socket(){

}

Socket::~Socket(){

}

int Socket::getSocket(){
    return this->sockfd_;
}

int Socket::prepareSocket(){

    this->sockfd_= socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockfd_ < 0) {
        perror("Socket Creation Failed");
        return -1;
    }
    // Set socket options (example: 2-second timeout)
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;                
    setsockopt(this->sockfd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(this->sockfd_, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    return this->sockfd_;
}


int Socket::unAssign(){
    
    if(close(this->sockfd_) < 0) return -1;
    return 0;
}