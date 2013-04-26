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
#include <errno.h>
#include <string.h>

using namespace std;

int MasterMD5Cracker::PASSLEN = PASS_LEN;

MasterMD5Cracker::MasterMD5Cracker(){
    isCracking = false;
    isExisting = false;
    timeSpent = 0.0;
    //initialized to be unlocked
    pthread_mutex_init(&slaves_mutex,NULL);
}

MasterMD5Cracker::~MasterMD5Cracker(){
    pthread_mutex_destroy(&slaves_mutex);
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
   
        /* unlikely exists duplicates since when a slave is gone, it's already unregistered
        if( master->isExistSlave(key) ){
            logMgr << "[WARN] Dupliate connection, remove previous one "<<endl;
            master->unregisterSlave(key); 
        }
        */

        //create slave proxy
        SlaveProxy slave;
        slave.master = master;
        slave.key = key;
        slave.slaveAddr = slaveAddr;
        slave.slavePort = slavePort;
        //socket that master receive cmd from slave
        slave.socket2Master = newSlaveSocket;

        master->registerSlave(key,slave);
        //create new dedicated thread to receive the cmd from slave
        bool ret = slaveProxies[key].run();
        
        if( !ret ){
            logMgr << "[ERROR] can't run slave proxy" <<endl;
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

    pthread_mutex_lock(&slaves_mutex);
    slaveProxies[key] = proxy;
    pthread_mutex_unlock(&slaves_mutex);
}

void MasterMD5Cracker::unregisterSlave(string& key){
    
    //LogManager& logMgr = LogManager::getInstance();
    
    pthread_mutex_lock(&slaves_mutex);
    
    //logMgr <<endl<< "unregisterSlave lock" <<endl;
    SlaveProxy& proxy = slaveProxies[key]; 
    proxy.terminate();
    slaveProxies.erase(key);

    //Test mutex to avoid race condition
    //sleep(5);
    pthread_mutex_unlock(&slaves_mutex);

    //logMgr <<endl<< "unregisterSlave unlock" <<endl;
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

    master->issueCmdAll(cmd);

    master->isCracking = true;

    //generate all possible passwords and distribute them to slaves, in pull mode!
    gettimeofday(&master->start,NULL);

    master->startDistributedCracking(pwMd5);

    return NULL;
}

bool MasterMD5Cracker::endDistributedCracking(){

    if( !this->isCracking )
        return false;

    LogManager& logMgr = LogManager::getInstance();
    
    //end the generation thread
    pthread_cancel(this->generateThread);

    void* retVal;
    pthread_join(this->generateThread,&retVal);

    logMgr << "Password generation thread terminated!" << endl;
    
    return true;
}


void* MasterMD5Cracker::cmdStop(MasterMD5Cracker* master, void* arg){

    LogManager& logMgr = LogManager::getInstance();

    logMgr << "[cmdStop]"<<endl;
   
    if( ! master->isCracking ){
        cout<<"There is no on-going cracking process"<<endl;
        return NULL;
    }

    Cmd cmd("stop","");

    master->issueCmdAll(cmd);

    master->endDistributedCracking();

    master->isCracking = false;
    
    gettimeofday(&master->end,NULL);
    
    struct timeval start = master->start;
    struct timeval end = master->end;

    double span = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec)/1000000.0;

    master->timeSpent = span; 

    logMgr << "[Distributed Mode] Time spent : "<<span << endl;

    return NULL;
}

// distributed cracking, push mode
// now trying to figure all possible passwords
bool MasterMD5Cracker::startDistributedCracking(string md5){

    LogManager& logMgr = LogManager::getInstance();

    //clear
    this->md5 = md5;
    this->pass = string("");
    this->chunkSize = CHUNK_SIZE;
    this->badPassRanges.clear();

    //create listening thread
    pthread_t thread;

    int rc = pthread_create(&thread,NULL,MasterMD5Cracker::generateThreadFunc,(void*)this);

    if(-1 == rc){
        logMgr << "[ERROR] can't create pass generation thread" << endl;
        return false;
    }

    this->generateThread = thread;

    logMgr << "Generate thread is running......" <<endl;

    return true;
}

//The push mode thread that send passwords to slaves 
void* MasterMD5Cracker::generateThreadFunc(void* arg){

    LogManager& logMgr = LogManager::getInstance();
    
    logMgr << "Password generation thread is running......" << endl;
    
    MasterMD5Cracker* master = (MasterMD5Cracker*)arg;

    int chunkSize = master->chunkSize;

    PassGenerator pg;
    
    len_t max = pg.getMax(master->PASSLEN); 

    //logMgr <<"Max is "<<max << " for length of "<< master->PASSLEN<<endl;

    deque<Cmd> cmds;

    unsigned int batchSize = 8;

    len_t cur = 0;
    
    while(master->isCracking && master->numOfSlaves() != 0){

        cur = 0;

        while( master->isCracking && cur < max ){

            //start
            len_t start = cur;

            //size of chunk actually
            int size = chunkSize;

            if(start+size > max){
                size = max - start;
            }

            cur += chunkSize;

            //It's not in bad password range
            if( 0 == master->badPassRanges.count(start) ){
            //send it to one slave for cracking
            //cout << "["<<start<<","<<start+size<<"]"<<endl;      
            //sleep(1);
                
                Cmd c("receiveChunk",start,chunkSize);

                cmds.push_back(c);

                if( cmds.size() == batchSize){

                    master->issueCmdRoundRobin(cmds);
                
                    cmds.clear();
                    //dont overwhelm the slaves
                    sleep(4);
                }
            }
        }

    }

    logMgr << "Password generation thread existing......" << endl;

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

    master->issueCmdAll(cmd);
 
    return NULL;
}

void* MasterMD5Cracker::cmdList(MasterMD5Cracker* master, void* arg){
    
    LogManager& logMgr = LogManager::getInstance();
    
    logMgr << "[cmdList]"<<endl;

    //print all connected slaves
    int count = 0;

    //Be care of race condition
    pthread_mutex_lock(&master->slaves_mutex);

    unordered_map<string, SlaveProxy>::iterator iter = master->slaveProxies.begin();

    while( iter != master->slaveProxies.end() ){

        SlaveProxy& slave = (*iter).second;

        cout << "Slave=[No."<<count<<"] key=["<<slave.key<<"]" <<endl;

        iter++;

        count++;
    }

    pthread_mutex_unlock(&master->slaves_mutex);
    
    return NULL;
}

void* MasterMD5Cracker::cmdQuit(MasterMD5Cracker* master, void* arg){

    LogManager& logMgr = LogManager::getInstance();

    logMgr << "[cmdQuit]"<<endl;
   
    Cmd cmd("quit","");

    master->issueCmdAll(cmd);

    master->isExisting = true;

    //A little bit brutal
    exit(1);

    return NULL;
}


void MasterMD5Cracker::reportFoundPass(string pass){

    LogManager& logMgr = LogManager::getInstance();

    logMgr <<endl <<"Password found ["<<pass<<"] => ["<< this->md5 <<"]"  <<endl;

    MasterMD5Cracker::cmdStop(this,(void*)NULL);
}

//Be care of race condition from slaveProxies

void MasterMD5Cracker::issueCmdAll(Cmd& cmd){

    LogManager& logMgr = LogManager::getInstance();

    logMgr <<endl<< "Issue Command " << cmd.name <<" to all slaves" <<endl;

    pthread_mutex_lock(&slaves_mutex);

    if( numOfSlaves() == 0 )
        return;

    unordered_map<string, SlaveProxy>::iterator iter = this->slaveProxies.begin();

    while( iter != this->slaveProxies.end() ){

        (*iter).second.issueCmd(cmd);

        iter++;
    }

    pthread_mutex_unlock(&slaves_mutex);
}

//it's useful to evenly distribute password for cracking
void MasterMD5Cracker::issueCmdRoundRobin(deque<Cmd>& cmds){

    //LogManager& logMgr = LogManager::getInstance();

    //logMgr <<endl<< "Issue Command to all slaves in a Round-Robin fashion" <<endl;

    //Be care of race condition here since slaveProxies is a shared resource for threads
    
    pthread_mutex_lock(&slaves_mutex);

    //No slaves are there around
    if(numOfSlaves() == 0){
        pthread_mutex_unlock(&slaves_mutex);
        return;
    }

    //logMgr <<endl<< "isseCmdRoundRobin lock" <<endl;
    unordered_map<string, SlaveProxy>::iterator iter = this->slaveProxies.begin();
    
    while( cmds.size() != 0 && isCracking ){

        Cmd cmd = cmds.front();

        cmds.pop_front();

        (*iter).second.issueCmd(cmd);

        iter++;

        if(iter == this->slaveProxies.end()){
            iter = this->slaveProxies.begin(); 
        }
    }

    pthread_mutex_unlock(&slaves_mutex);

    //logMgr <<endl<< "isseCmdRoundRobin unlock" <<endl;
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
//recursive password generation, different from distributed mode
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
    }
    else{
        cout << "Can't find the password. Please Make sure password complies with rules" <<endl;
    }

    logMgr << "[Single Mode] Time spent : "<<timeSpent << endl;
}

//Recursion method 
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

