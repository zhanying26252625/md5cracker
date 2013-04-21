#include "../configure.h"
#include "masterMD5Cracker.h"
#include "logManager.h"
#include "md5.h"
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

//int MasterMD5Cracker::PASSLEN = 10;

MasterMD5Cracker::MasterMD5Cracker(){
    isCracking = false;
    isExisting = false;
    timeSpent = 0.0;
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
    
        if( slaveProxies.count(key) >0 ){

            logMgr << "[WARN] Dupliate connection, remove previous one "<<endl;
            
            SlaveProxy& proxy = slaveProxies[key];

            proxy.terminate();

            slaveProxies.erase(key);
        }

        //create slave proxy
        SlaveProxy slave;
        slave.master = master;
        slave.key = key;
        slave.slaveAddr = slaveAddr;
        slave.slavePort = slavePort;
        slave.socket2Master = newSlaveSocket;

        slaveProxies[key] = slave;
        bool ret = slaveProxies[key].run();
        
        if( !ret ){
            logMgr << "[ERROR] can't run proxy" <<endl;
            slaveProxies.erase(key);
        }

    }

    return NULL;
}

void* MasterMD5Cracker::cmdStart(MasterMD5Cracker* master, void* arg){
    
    LogManager& logMgr = LogManager::getInstance();

    logMgr << "[cmdStart]"<<endl;
 
    //If there is an on-going cracking process
    if( master->isCracking ){
        cout<<"Please stop the previous cracking process"<<endl;
        return NULL;
    }
    
    cout << "Please Input MD5-hashed password" <<endl;

    string pwMd5;

    cin >> pwMd5;

    cout << "MD5 hash for the password is ["<<pwMd5<<"]"<<endl;

    //generate all possible passwords and distribute them to slaves, in pull mode!

    master->startDistributedCracking(pwMd5);

    return NULL;
}

// distributed cracking, pull mode
// now trying to figure all possible passwords
bool MasterMD5Cracker::startDistributedCracking(string md5){

    int totalNum = 0;

    for(int len = 1; len <= PASSLEN; len++){
        int count = this->generateAllPossiblePWs(len);
        totalNum += count;
    }

    cout <<endl<<"Total number of passwords are ["<<totalNum<<"]"<<endl;

    return true;
}

void* MasterMD5Cracker::cmdStop(MasterMD5Cracker* master, void* arg){

    LogManager& logMgr = LogManager::getInstance();

    logMgr << "[cmdStop]"<<endl;
   
    if( ! master->isCracking ){
        cout<<"There is no on-going cracking process"<<endl;
        return NULL;
    }

    return NULL;
}

void* MasterMD5Cracker::cmdStatus(MasterMD5Cracker* master, void* arg){
    
    LogManager& logMgr = LogManager::getInstance();

    logMgr << "[cmdStatus]"<<endl;
    
    return NULL;
}

void* MasterMD5Cracker::cmdList(MasterMD5Cracker* master, void* arg){
    
    LogManager& logMgr = LogManager::getInstance();

    logMgr << "[cmdList]"<<endl;
    
    return NULL;
}

void* MasterMD5Cracker::cmdQuit(MasterMD5Cracker* master, void* arg){

    LogManager& logMgr = LogManager::getInstance();

    logMgr << "[cmdQuit]"<<endl;
    
    return NULL;
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

//A single-threaded md5 cracker
void MasterMD5Cracker::runLocal(){

    LogManager& logMgr = LogManager::getInstance();

    logMgr << "MasterMD5Cracker is running in [Single Mode]......"<<endl;

    cout << "Please input the MD5 code" <<endl;

    string md5("");

    getline(cin,md5);

    cout << "We are trying to crack MD5 ["<<md5<<"]" << endl;

/*
 *Assume Password only contains
 A-Z
 a-z
 0-9
 * */

    //init character array
    vector<char> charArr;

    for(char c = 'A'; c <= 'Z'; c++)
        charArr.push_back(c);

    for(char c = 'a'; c <= 'z'; c++)
        charArr.push_back(c);

    for(char c = '0'; c <= '9'; c++)
        charArr.push_back(c);

    bool found = false;

    struct timeval start, end;

    gettimeofday(&start,NULL);

    string pass("");

    //Test password of every length
    for(int len = 1; len <= PASSLEN; len++){
        
        string newPass;

        newPass.reserve(len);
        
        if( (found = crackPasswordLen(md5,pass,len,charArr,newPass,0)) )
                break;
    }

    gettimeofday(&end,NULL);

    double span = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec)/1000000.0;

    this->timeSpent = span;

    if(found){
        cout <<"Password Found! it's ["<<pass<<"]"<<endl;
        return ;
    }
    else{
        cout << "Can't find the password. Please Make sure password complies with rules" <<endl;
        return;
    }

}


bool MasterMD5Cracker::crackPasswordLen(string& md5, string& pass, int len, vector<char>& charArr, string& newPass, int level){

    if( level == len ){
        //Do md5 hash of the new password
        MD5 md5Engine;

        string str = md5Engine.calMD5FromString(newPass);

        //found
        if( !str.compare(md5) ){
            pass = newPass;
            return true;
        }
        else{
            return false;
        }

    }
    else{

        for(unsigned int i = 0; i< charArr.size(); i++){

            newPass.push_back( charArr[i] );

            bool found = crackPasswordLen(md5,pass,len,charArr,newPass,level+1);

            newPass.erase(level,1);

            if(found)
                return true;
        }
    }

    return false;
}

int MasterMD5Cracker::_generateAllPossiblePWs(int len, vector<char>& charArr,string& newPass, int level, int& count){
        
    if( level == len ){
        count++;
        //cout << newPass <<endl;
    }
    else{

        for(unsigned int i = 0; i< charArr.size(); i++){

            newPass.push_back( charArr[i] );

            _generateAllPossiblePWs(len,charArr,newPass,level+1,count);

            newPass.erase(level,1);
        }
    }

    return 0;
}


int MasterMD5Cracker::generateAllPossiblePWs(int len){

    int count = 0;

    string newPass;

    newPass.reserve(len);
/*
 *Assume Password only contains
 A-Z
 a-z
 0-9
 * */

    //init character array
    vector<char> charArr;

    for(char c = 'A'; c <= 'Z'; c++)
        charArr.push_back(c);

    for(char c = 'a'; c <= 'z'; c++)
        charArr.push_back(c);

    for(char c = '0'; c <= '9'; c++)
        charArr.push_back(c);


    _generateAllPossiblePWs(len,charArr,newPass,0,count);

    cout <<"There are ["<<count <<"]"<<" passwords of length "<<len <<endl;

    return count;
}



//A distributed md5 cracker
void MasterMD5Cracker::runDistribute(){

    LogManager& logMgr = LogManager::getInstance();

    logMgr << "MasterMD5Cracker is running in [Distributed Mode]......"<<endl;
    
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

