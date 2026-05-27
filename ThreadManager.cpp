//ThreadManager.cpp

#include "Scanner.h"
#include "ThreadManager.h"
//#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <map>
#include <vector>
#include <unistd.h> 


using namespace std;
#define NUM_THREADS 50
bool ThreadManager::threadFree[50];


ThreadManager::ThreadManager(){
    setThreadMap();
    setThreadStatus();
}


ThreadManager::~ThreadManager(){
    //std::cout << "ThreadManager object destroyed." << std::endl;
}

void ThreadManager::setThreadMap(){

    for(int i = 0; i <NUM_THREADS; i++){
        threadMap.insert(std::make_pair((unsigned long)thread[i], i));
    }
}

void ThreadManager::setThreadStatus(){

    for(int i=0; i<NUM_THREADS; i++){
        ThreadManager::threadFree[i] = true;
    }
}

void ThreadManager::populate_ips(std::vector<string> ips){
    this->ipVector = ips;
    cout << "ThreadManager ipVector Size : " << ipVector.size() << endl;
}


//The threaded function. Single thread
void* ThreadManager::watchThreads(void* arg){
    ThreadManager* ctrl = static_cast<ThreadManager*>(arg); 
    //sem_wait(&connection_sem); //set it up later
    
    Scanner* sObject = nullptr;
        
    while(true){

        for(int i=0; i<NUM_THREADS; i++){
            usleep(100000);            
            
            if(ctrl->threadFree[i]){                
                if(ctrl->currentPort == MAXPORT){
                    ctrl->currentPort = MINPORT;                    
                    if(!ctrl->ipVector.empty()){
                        ctrl->ipVector.pop_back();
                        if(ctrl->ipVector.size() == 0) break;
                    }
                }
                if(ctrl->currentPort == MINPORT){
                    if(!ctrl->ipVector.empty()){
                        ctrl->currentIP = ctrl->ipVector.back();                        
                    }
                }   
                try{                    
                    sObject = new Scanner(ctrl->currentIP, ctrl->currentPort);                   
                    sObject->setHostConnection();
                    ctrl->threadFree[i] = false;
                    ctrl->currentPort = ctrl->currentPort + 1; 
                    
                    pthread_create(&(ctrl->thread[i]), nullptr, Scanner::processWorker, sObject); //& can be removed
                }catch (const std::runtime_error& e) {
                    std::cerr << "Socket creation problem."<< std::endl;
                    // Do not mark thread slot busy
                }                          
            }
        }
        if(ctrl->ipVector.empty()){
            break;
        }
        for (int i = 0; i < NUM_THREADS; i++) {
            int ret = pthread_join(ctrl->thread[i], nullptr); // Non-blocking check
            if (ret == 0) {
                ctrl->threadFree[i] = true;                
            }

        }
        
    }
    return nullptr;
}

