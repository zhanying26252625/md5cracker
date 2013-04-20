#ifndef _MASTER_H
#define _MASTER_H

#include "slaveProxy.h"
#include <string>
#include <map>
#include <unordered_map> //experimtal in c++11
#include <pthread.h>

using namespace std;

class SlaveProxy;

class MasterMD5Cracker{
    
private:
    enum UserCmd{START, STOP, STATUS, LIST, QUIT};

public:
    MasterMD5Cracker();

    virtual ~MasterMD5Cracker();

    void run();
    
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

    void issueCmd(Cmd& cmd);

    void cmdHelp();

    void cui();

    bool isExistSlave(string& key);

    void registerSlave(string& key,SlaveProxy& proxy);

    void unregisterSlave(string& key);

    int numOfSlaves(){return slaveProxies.size();}

private:
    //BST cmd handlers
    map<string,CmdHandler> cmdHandlers;
    //HASHTABLE slaves
    unordered_map<string, SlaveProxy> slaveProxies;
    //map<string, SlaveProxy> slaveProxies;
    
    bool isExisting;

    bool isCracking;

    int listeningSocket;

    pthread_t listeningThread;
};


#endif
