#ifndef _MASTER_H
#define _MASTER_H

#include "../configure.h"
#include "slaveProxy.h"
#include "passGenerator.h"
#include <string>
#include <map>
#include <set>
#include <deque>
#include <unordered_map> //experimtal in c++11
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

class SlaveProxy;
class ReturnRet;
class Feedback;
class Handshake;

class MasterMD5Cracker{
    
private:
    enum UserCmd{START, STOP, STATUS, LIST, QUIT};

public:
    MasterMD5Cracker();

    virtual ~MasterMD5Cracker();

    void runLocal();

    void runDistribute();
   
    double getTimeSpent(){return timeSpent;}
    
    friend class ReturnRet;
    friend class Feedback;
    friend class Handshake;
    friend class SlaveProxy;

private:
    //Background thread accepting slave's connection
    static void* listeningThreadFunc(void* arg);

    bool createListeningThread();

    //User command
    void initUserCmd();

    typedef void* (*CmdHandler)(MasterMD5Cracker* master, void* arg);
    
    static void* cmdStart(MasterMD5Cracker* master, void* arg);

    static void* cmdStop(MasterMD5Cracker* master, void* arg);

    static void* cmdStatus(MasterMD5Cracker* master, void* arg);

    static void* cmdList(MasterMD5Cracker* master, void* arg);

    static void* cmdQuit(MasterMD5Cracker* master, void* arg);

    //used by runLocal, based on recursion, it's based on recursion to generate all password combination  and it's proper for single-threaded processing
    bool crackPasswordLen(string& md5, string& pass, int len, vector<char>& charArr, string& newPass, int level);

    
    //The push mode thread that send passwords to slaves 
    static void* generateThreadFunc(void* arg); 

    bool startDistributedCracking(string md5);

    bool endDistributedCracking();
    
    void issueCmdAll(Cmd& cmd);

    void issueCmdRoundRobin(deque<Cmd>& cmds);

    void cmdHelp();

    void cui();

    bool isExistSlave(string& key);

    //be care of race condition in these 3 functions below
    void registerSlave(string& key,SlaveProxy& proxy);

    void unregisterSlave(string& key);

    int numOfSlaves(){return slaveProxies.size();}

    void reportFoundPass(string pass);

private:
    //BST cmd handlers
    map<string,CmdHandler> cmdHandlers;

    //HASHTABLE slaves
    unordered_map<string, SlaveProxy> slaveProxies;
    //An associated advisory mutex-lock, anti race-condition
    pthread_mutex_t slaves_mutex;

    //map<string, SlaveProxy> slaveProxies;

    //Max len of password
    static int PASSLEN ;

    //state
    bool isExisting;

    bool isCracking;

    //listen socket
    int listeningSocket;

    //Listen to the connections from slaves
    pthread_t listeningThread;

    //Generate passwords and push them to slaves
    pthread_t generateThread;
 
    //A batch of passwords to be sent to slave each time
    int chunkSize;
  
    //on-going bad pass range found by slaves. no need to cal them again
    set<len_t> badPassRanges;
    //on-going md5
    string md5;
    //on-going pass
    string pass;
    
    //record already found pairs
    unordered_map<string, string> md5Records;
    
    //run time
    double timeSpent;

    struct timeval start, end;
};


#endif
