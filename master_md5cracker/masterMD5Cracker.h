#ifndef _MASTER_H
#define _MASTER_H

#include <string>
#include <map>
#include <pthread.h>

using namespace std;

class MasterMD5Cracker{
    
private:
    enum UserCmd{START, STOP, STATUS, LIST, QUIT};

public:
    MasterMD5Cracker();

    virtual ~MasterMD5Cracker();

    void run();
    
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

    void cmdHelp();

    void cui();

private:

    map<string,CmdHandler> cmdHandlers;

    bool isExisting;

    bool isCracking;

    int listeningSocket;

    pthread_t listeningThread;
};


#endif
