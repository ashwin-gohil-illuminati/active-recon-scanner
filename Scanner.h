// Scanner.h
#ifndef SCANNER_H
#define SCANNER_H

#include "Socket.h"
#include <string>
#include <arpa/inet.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <mutex>
#include <ctime>
#include <iomanip>


class Scanner {

    public: 
        Scanner(const std::string& ip, int port); //if can be same but port will differ.
        //Even though two objects are having same varibles and same value are still different. 
        ~Scanner();
    
        static void* processWorker(void* arg);   //will be threaded and object would be passed which is itself only
        //struct sockaddr_in getHostConnection(); //done
        bool connectToHost(Socket* sockObj); //if connect happened, update the value of connected_ to true        
        int receiveData(Socket* sockObj);
        int sendData(const std::string &probe_string, Socket* sockObj);
        bool getConnectStatus(); //done
        //int getTimeoutSeconds(); //done
        void disconnect();  
        int getPortNumber();
        void setHostConnection(); 
        int loadPortProbes();
        static void initFile(const std::string &filename);
        static void writeToFile(const std::string &ip, int port, const std::string &banner);
        static void finalizeFile();
        
                
    private:
        std::string ip_;
        int port_;
        int sockfd_;
        bool connected_;
        //int timeoutSeconds_;
        sockaddr_in hostAddr;
        static std::map<int, std::string> portProbes;
        static std::ofstream outfile;
        static std::mutex fileMutex;
        static std::string fileName;

          
};
#endif



