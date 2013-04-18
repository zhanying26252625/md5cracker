#ifndef _SLAVE_MD5CRACKER_H
#define _SLAVE_MD5CRACKER_

#include "masterProxy.h"
#include <string>

using namespace std;

class SlaveMD5Cracker{

private:

    enum State{HANDSHAKE,FETCH,STOP};

private:

    string getMasterIP(string fileName=string("masterIP.txt"));

    bool connectToMaster(string ip,int& toMasterSocket);

    //Send cmd to master
    static void* masterSenderFunc(void* arg); 

    //Receive cmd from master
    void masterReceiverFunc();

    MasterProxy masterProxy;

    int toMasterSocket;

    enum State state;

public:

    SlaveMD5Cracker();

    void run();
};

#endif
