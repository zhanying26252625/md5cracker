#ifndef _MASTER_H
#define _MASTER_H

#include "../configure.h"
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

    void runLocal();

    void runDistribute();
   
    double getTimeSpent(){return timeSpent;}
    
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

    //number of all passwords possible, it seems recursion make it extremenly slow and not proper for the distribution to the slaves,we need some other algorithm to generate the combinations of password.
    int _generateAllPossiblePWs(int len, vector<char>& charArr, string& newPass, int level, int& count);

    int generateAllPossiblePWs(int len);

    bool startDistributedCracking(string md5);

    bool endDistributedCracking();
    
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

    //Max len of password
    static const int PASSLEN = PASS_LEN;

    //state
    bool isExisting;

    bool isCracking;

    //listen socket
    int listeningSocket;

    pthread_t listeningThread;

    //run time
    double timeSpent;

};


#endif
