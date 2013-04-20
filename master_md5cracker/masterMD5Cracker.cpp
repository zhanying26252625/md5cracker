#include "../configure.h"
#include "masterMD5Cracker.h"
#include "logManager.h"
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

using namespace std;

MasterMD5Cracker::MasterMD5Cracker(){
    isCracking = false;
    isExisting = false;
}

MasterMD5Cracker::~MasterMD5Cracker(){
    
}

bool MasterMD5Cracker::createListeningThread(){

    LogManager& logMgr = LogManager::getInstance();
    
    int listeningSocket = 0;  

    struct sockaddr_in server_addr;    
        
    if((listeningSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        logMgr << "[ERROR]Can't create Socket" << endl;
        return false;
    }

    int option = 1;
    if(setsockopt(listeningSocket,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(int)) == -1) {
        logMgr << "[ERROR]Can't setsocket" << endl;
        return false;
    }
        
    server_addr.sin_family = AF_INET;         
    server_addr.sin_port = htons( MASTER_PORT );     
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    bzero(&(server_addr.sin_zero),8); 

    if(bind(listeningSocket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))== -1) {
        logMgr << "[ERROR]Unable to bind" <<endl;
        return false;
    }

    if (listen(listeningSocket, 5) == -1) {
        logMgr << "[ERROR]Listen error" << endl;
        return false;
    }

    this->listeningSocket = listeningSocket;

    //create listening thread
    pthread_t thread;

    int rc = pthread_create(&thread,NULL,MasterMD5Cracker::listeningThreadFunc,(void*)this);

    if(-1 == rc){
        logMgr << "[ERROR] can't create listening thread" << endl;
        return false;
    }

    return true;
}

void* MasterMD5Cracker::listeningThreadFunc(void* arg){

    LogManager& logMgr = LogManager::getInstance();
    
    MasterMD5Cracker* master = (MasterMD5Cracker*)arg;

    unordered_map<string, SlaveProxy>& slaveProxies = master->slaveProxies;

    logMgr <<"Server is listening for connections from slaves"<<endl;

    struct sockaddr_in  slave_addr;    

    while( ! master->isExisting ){

        unsigned int sin_size = sizeof(struct sockaddr_in);

        //accept new connection
        int newSlaveSocket = accept( master->listeningSocket, (struct sockaddr*)&slave_addr,&sin_size  );

        string slaveAddr = inet_ntoa(slave_addr.sin_addr);

        int slavePort = ntohs(slave_addr.sin_port);

        char buf[256] = {0};

        sprintf(buf,":%d",slavePort);

        string key = slaveAddr + string(buf);
    
        logMgr << "New slave connection ["<<slaveAddr<<"] ["<<slavePort<<"]"<<" Key is ["<<key<<"]" <<endl;
    
        if( master->isExistSlave(key) ){

            logMgr << "[WARN] Dupliate connection, remove previous one "<<endl;
            master->unregisterSlave(key); 
            
        }

        //create slave proxy
        SlaveProxy slave;
        slave.master = master;
        slave.key = key;
        slave.slaveAddr = slaveAddr;
        slave.slavePort = slavePort;
        //socket that master receive cmd from slave
        slave.socket2Master = newSlaveSocket;

        master->registerSlave(key,slave);
        
        bool ret = slaveProxies[key].run();
        
        if( !ret ){
            logMgr << "[ERROR] can't run proxy" <<endl;
            slaveProxies.erase(key);
        }

    }

    return NULL;
}

bool MasterMD5Cracker::isExistSlave(string& key){

    if ( slaveProxies.count(key) >0 )
        return true;

    return false;
}

void MasterMD5Cracker::registerSlave(string& key,SlaveProxy& proxy){

    slaveProxies[key] = proxy;
}

void MasterMD5Cracker::unregisterSlave(string& key){
    
    SlaveProxy& proxy = slaveProxies[key]; 
    
    proxy.terminate();
    
    slaveProxies.erase(key);
}



void* MasterMD5Cracker::cmdStart(MasterMD5Cracker* master, void* arg){
    
    LogManager& logMgr = LogManager::getInstance();

    logMgr << "[cmdStart]"<<endl;
 
    //If there is an on-going cracking process
    if( master->isCracking ){
        cout<<"Please stop the previous cracking process"<<endl;
        return NULL;
    }

    if( master->numOfSlaves()==0 ){
        cout<<"There is no slave connected"<<endl;
        return NULL;
    }
    
    cout << "Please Input MD5-hashed password" <<endl;

    string pwMd5;

    cin >> pwMd5;

    cout << "MD5 hash for the password is ["<<pwMd5<<"]"<<endl;
 
    Cmd cmd("start",pwMd5);

    master->issueCmd(cmd);

    master->isCracking = true;

    return NULL;
}

void* MasterMD5Cracker::cmdStop(MasterMD5Cracker* master, void* arg){

    LogManager& logMgr = LogManager::getInstance();

    logMgr << "[cmdStop]"<<endl;
   
    if( ! master->isCracking ){
        cout<<"There is no on-going cracking process"<<endl;
        return NULL;
    }
 
    if( master->numOfSlaves()==0 ){
        cout<<"There is no slave connected"<<endl;
        return NULL;
    }

    Cmd cmd("stop","");

    master->issueCmd(cmd);

    master->isCracking = false;
    
    return NULL;
}

void* MasterMD5Cracker::cmdStatus(MasterMD5Cracker* master, void* arg){
    
    LogManager& logMgr = LogManager::getInstance();

    logMgr << "[cmdStatus]"<<endl;
    
    Cmd cmd("status","");

    if( master->numOfSlaves()==0 ){
        cout<<"There is no slave connected"<<endl;
        return NULL;
    }

    master->issueCmd(cmd);
 
    return NULL;
}

void* MasterMD5Cracker::cmdList(MasterMD5Cracker* master, void* arg){
    
    LogManager& logMgr = LogManager::getInstance();
    
    logMgr << "[cmdList]"<<endl;

    //print all connected slaves

    int count = 0;

    unordered_map<string, SlaveProxy>::iterator iter = master->slaveProxies.begin();

    while( iter != master->slaveProxies.end() ){

        SlaveProxy& slave = (*iter).second;

        cout << "Slave=[No."<<count<<"] key=["<<slave.key<<"]" <<endl;

        iter++;

        count++;
    }

    return NULL;
}

void* MasterMD5Cracker::cmdQuit(MasterMD5Cracker* master, void* arg){

    LogManager& logMgr = LogManager::getInstance();

    logMgr << "[cmdQuit]"<<endl;
   
    Cmd cmd("quit","");

    master->issueCmd(cmd);

    master->isExisting = true;

    return NULL;
}

void MasterMD5Cracker::issueCmd(Cmd& cmd){

    LogManager& logMgr = LogManager::getInstance();

    logMgr <<endl<< "Issue Command " << cmd.name <<" to all slaves" <<endl;

    unordered_map<string, SlaveProxy>::iterator iter = this->slaveProxies.begin();

    while( iter != this->slaveProxies.end() ){

        (*iter).second.issueCmd(cmd);

        iter++;
    }
}

void MasterMD5Cracker:: initUserCmd(){

    cmdHandlers["start"] = &(MasterMD5Cracker::cmdStart);
    cmdHandlers["stop"] = &(MasterMD5Cracker::cmdStop);
    cmdHandlers["status"] = &(MasterMD5Cracker::cmdStatus);
    cmdHandlers["list"] = &(MasterMD5Cracker::cmdList);
    cmdHandlers["quit"] = &(MasterMD5Cracker::cmdQuit);

}

void MasterMD5Cracker::cmdHelp(){
    //No such command
    cout<<endl;  
    cout<<"Please select one name of the following commands"<<endl;
    cout<<"1. start [Start a new password cracking process]"<<endl;
    cout<<"2. stop [Stop the on-going cracking process]"<<endl;
    cout<<"3. status [Show the status of the cracking process]"<<endl;
    cout<<"4. list [List the cracking slaves connected]"<<endl;
    cout<<"5. quit [Terminate this distributed cracking system]"<<endl;
    cout<<"> ";
}

void MasterMD5Cracker::cui(){

    cmdHelp();

    while( ! isExisting ){

        string cmd;
        //Only get one word
        //cin >> cmd;
        //Get whole line
        getline(cin,cmd);

        if( cmdHandlers.count(cmd) ){
            cmdHandlers[cmd](this,NULL);
        }
        else{
            cmdHelp();
        }
    }
}

void MasterMD5Cracker::run(){

    LogManager& logMgr = LogManager::getInstance();

    logMgr << "MasterMD5Cracker is running......"<<endl;
    
    //Ignore SIGPIPE signal, which would kill the process when pipe breaks
    signal(SIGPIPE,SIG_IGN);

    //Create a background listening thread
    if( ! createListeningThread() ){
        logMgr << "Can't Create listening thread!"<<endl;
    }

    //init command
    initUserCmd();

    //Accept commands from clients
    cui();

}

