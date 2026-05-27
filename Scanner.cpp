//Scanner.cpp

#include "Scanner.h"
#include "Socket.h"
#include <asm-generic/socket.h>
#include <cstdlib>
//#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>      // for inet_pton / inet_ntop
#include <unistd.h>         // for close()
#include <cstring>          // for memset
#include <iostream>         // C++ I/O
#include <cerrno>
#include <pthread.h>

using namespace std;
map<int, std::string> Scanner::portProbes;

pthread_mutex_t the_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t the_mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t the_mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t the_mutex3 = PTHREAD_MUTEX_INITIALIZER;


std::ofstream Scanner::outfile;
std::mutex Scanner::fileMutex;
std::string Scanner::fileName;


Scanner::Scanner(const std::string& ip, int port){
    this->ip_ = ip;
    this->port_ = port;
    this->connected_ = false;

    if(Scanner::portProbes.empty()){
        loadPortProbes();
        std::cout << "portProbes map size : " << Scanner::portProbes.size() << std::endl;
    }
    //Scanner::initFile("PortScanResults.html");
}


Scanner::~Scanner(){
    //Scanner::finalizeFile();
    //std::cout << "Object - "<< ip_ << " destroyed." << std::endl;
}


bool Scanner::getConnectStatus(){
    return this->connected_;
}


int Scanner::getPortNumber(){
    return this->port_;
}



void Scanner::setHostConnection(){

     //Creating struct to connect host machine
     sockaddr_in addr;
     addr.sin_family = AF_INET;
     addr.sin_port = htons(this->port_);
     if (inet_pton(AF_INET, this->ip_.c_str(), &addr.sin_addr) <= 0) {
         std::cerr << "Invalid IP address: " << this->ip_ << std::endl;
         exit(EXIT_FAILURE); //AI reco
     }
     this->hostAddr = addr;    
}


int Scanner::loadPortProbes(){

    std::ifstream inputFile("PortProbe.txt"); // Replace "example.txt" with your file path

    if (!inputFile.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }
    int port;
    std::string probeString;
    std::string line;
    char target_char = ' ';
    while (std::getline(inputFile, line)) {
        port = std::stoi(line.substr(0, 4));
        probeString = line.substr(5,std::string::npos);
        Scanner::portProbes.insert(std::pair<int, std::string>(port, probeString));        
    }

    inputFile.close();    
   return 0;
}


bool Scanner::connectToHost(Socket* sockObj) {
    
    if(connect(sockObj->getSocket(), (struct sockaddr *)&(this->hostAddr), sizeof(this->hostAddr)) < 0) {
        if (errno == ECONNREFUSED) {
            std::cerr << "Connection refused by target." << std::endl;
        } else if (errno == ETIMEDOUT) {
            std::cerr << "Connection timed out." << std::endl;
        } else {
            perror("connect() failed");
        }
        this->connected_ = false;
        return false;
    }

    this->connected_ = true;
    return true;
}

int Scanner::receiveData(Socket* sockObj) {
    if (!this->connected_) {
        std::cerr << "Cannot receive: No connection established." << std::endl;
        return -1;
    }

    char buffer[1024];
    // Single recv() call with timeout
    int bytesReceived = recv(sockObj->getSocket(), buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            //std::cout << "No data received within timeout" << std::endl; //important
        } else {
            perror("recv() failed");
        }
        return -1;
    } else if (bytesReceived == 0) {
        //std::cout << "Peer closed the connection." << std::endl; //important
        return 0;
    } else {
        buffer[bytesReceived] = '\0';
        //pthread_mutex_lock (&the_mutex);
        //std::cout << this->ip_ << " " << this->port_ << " " <<"Received: " << buffer <<std::endl;
        //pthread_mutex_unlock (&the_mutex);
        Scanner::writeToFile(this->ip_, this->port_, buffer);
        return bytesReceived;
    }
}


int Scanner::sendData(const std::string &msg, Socket* sockObj) {

    if (!connected_) {
        std::cerr << "Cannot send: No connection established." << std::endl;
        return -1;
    }

    int bytesSent = send(sockObj->getSocket(), msg.c_str(), msg.size(), 0);
    if (bytesSent < 0) {
        perror("send() failed");
        return -1;
    }

    return bytesSent;
}


void* Scanner::processWorker(void* arg) {
    
    Scanner* ctrl = static_cast<Scanner*>(arg); 
    Socket* socObject =  new Socket(); 
    socObject->prepareSocket(); 
    ctrl->setHostConnection();
    ctrl->connectToHost(socObject);

    if (ctrl->getConnectStatus()) {
        // --- 2. Try to receive an immediate banner ---
        // Assumes receiveData() is non-blocking and returns -1 with EAGAIN/EWOULDBLOCK on timeout.
        int recvBytes = ctrl->receiveData(socObject); 
        if (recvBytes == 0 || (recvBytes < 0 && errno == EAGAIN)) {
            std::string probeString;
                      
            auto it = Scanner::portProbes.find(ctrl->port_);
            if (it != Scanner::portProbes.end()) {
                probeString = it->second; // Copy the string to a local variable
                probeString = probeString+"\n";
            }
            //There are 193 ports to probe in total
            if (!probeString.empty()) {                

                char buffer[1024];
                                
                int bytesSent = send(socObject->getSocket(), probeString.c_str(), probeString.size(), 0);
                int bytesReceived = recv(socObject->getSocket(), buffer, sizeof(buffer) - 1, 0);              
                if(bytesReceived > 0){
                    buffer[bytesReceived] = '\0';
                    Scanner::writeToFile(ctrl->ip_, ctrl->port_, buffer);
                }                
            }
        }

    }
    
    socObject->unAssign();
    delete socObject;
    delete ctrl;

    return nullptr;     
}
    

void Scanner::disconnect() {
    //cout << "disconnect called" <<endl;
    if (this->sockfd_ >= 0) {
        // Gracefully shut down both send and receive directions
        shutdown(this->sockfd_, SHUT_RDWR);
        close(this->sockfd_);
        this->sockfd_ = -1;
    }
    this->connected_ = false;
}


//File Write Methods
void Scanner::initFile(const std::string &filename) {
    std::lock_guard<std::mutex> lock(fileMutex);
    fileName = filename;
    outfile.open(fileName, std::ios::out | std::ios::trunc);
    if (!outfile.is_open()) {
        std::cerr << "Error opening file: " << fileName << "\n";
        return;
    }

    // Current time
    std::time_t now = std::time(nullptr);
    std::tm *ltm = std::localtime(&now);
    std::ostringstream ts;
    ts << std::put_time(ltm, "%Y-%m-%d %H:%M:%S");

    // Write HTML header
    outfile <<
R"(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <title>Port Scan Results</title>
  <style>
    body {
      background-color: #0a4b8c;
      color: white;
      font-family: monospace;
      padding: 40px;
      text-align: center;
    }
    h1 { font-size: 2.5rem; margin-bottom: 10px; }
    .timestamp { font-size: 1rem; margin-bottom: 40px; }
    .entry { margin: 4px 0; }
  </style>
</head>
<body>
  <h1>Port Scan Results</h1>
  <div class="timestamp">)" << ts.str() << R"(</div>
)";

    outfile.flush();
}


void Scanner::writeToFile(const std::string &ip, int port, const std::string &banner) {
    std::lock_guard<std::mutex> lock(fileMutex);

    if (!outfile.is_open()) return;

    // Write one entry [IP  Port  Banner]
    //outfile << "  <div class=\"entry\">[" 
    //        << ip << "  " << port << "  " << banner << "]</div>\n";
    outfile << "  <div class=\"entry\" style=\"text-align: left;\">[" 
    << ip << "  " << port << "  " << banner << "]</div>\n";
    outfile.flush(); // atomic + immediate
}

void Scanner::finalizeFile() {
    std::lock_guard<std::mutex> lock(fileMutex);

    if (!outfile.is_open()) return;

    // Write closing HTML
    outfile << "\n</body>\n</html>\n";
    outfile.close();
}