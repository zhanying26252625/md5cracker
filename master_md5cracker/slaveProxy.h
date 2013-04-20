#ifndef _SLAVE_PROXY_H
#define _SLAVE_PROXY_H

#include <string>
#include <pthread.h>
#include <queue>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_pstream.hpp>
#include <xmlrpc-c/girerr.hpp>
#include <xmlrpc-c/client.hpp>
#include <xmlrpc-c/client_transport.hpp>

using namespace std;

class MasterMD5Cracker;
class Fetch;
class HandShake;

struct Cmd{
    Cmd(){}
    Cmd(string n,string p):name(n),param(p){}
    string name;
    string param;
};

class SlaveProxy{

private:

    string key;

    string slaveAddr;

    int slavePort;

    //Use two sockets in two threads for concurrent bi-directional communication 
    int socket2Master; //receive cmd from slave

    int socket2Slave; //send cmd to slave

    //Receiver cmd thread for slave
    static void* slaveReceiverFunc(void* arg);

    //Sender cmd thread to slave
    static void* slaveSenderFunc(void* arg);

    static bool sendViaXMLRPC(int socket, string name, string param);

    MasterMD5Cracker* master;

    pthread_t threadMasterSender;

    pthread_t threadMasterReceiver;

    //True is duplex connection is already there
    bool isFullConnected;

    queue<Cmd> cmdQueue;

public:

    bool run();

    void terminate();

    void issueCmd(Cmd& cmd);

    SlaveProxy();

    friend class HandShake;
    friend class Fetch;
    friend class MasterMD5Cracker;
};

//Slave fetch passwords to do md5hash
class Fetch : public xmlrpc_c::method{

private:
    SlaveProxy* slave;

public:
    Fetch(SlaveProxy* sp){
        this->slave = sp;
        this->_help = "Slave fetch passwords to do md5hash";
    }

    void execute(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* retValP );

};

//Slave fetch passwords to do md5hash
class HandShake : public xmlrpc_c::method{

private:
    SlaveProxy* slave;

public:
    HandShake(SlaveProxy* sp){
        this->slave = sp;
        this->_help = "Handsake, master create another connection to slave for pseudo full duplex";
    }

    void execute(const xmlrpc_c::paramList& paramList,xmlrpc_c::value* retValP );

};

#endif
