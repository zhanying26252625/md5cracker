#ifndef _SLAVE_PROXY_H
#define _SLAVE_PROXY_H

#include "passGenerator.h"

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

//command sent to slaves
struct Cmd{
    //stop,status,quit
    Cmd(string n):name(n){longVal = intVal = 0; strVal=string("");}
    //receiveChunk start chunkSize
    Cmd(string n,len_t l, int i):name(n),longVal(l),intVal(i){strVal=string("");}
    //start md5
    Cmd(string n,string s):name(n),strVal(s){ longVal = intVal = 0;}
    string name;
    string strVal;
    len_t longVal;
    int intVal;
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

    static bool sendViaXMLRPC(int socket, string name, string strVal, len_t longVal, int intVal);

    MasterMD5Cracker* master;

    pthread_t threadMasterSender;
    pthread_t threadMasterReceiver;
    //True is duplex connection is already there
    bool isFullConnected;
    queue<Cmd> cmdQueue;
    bool isExisting;

public:

    bool run();

    void terminate();

    void issueCmd(Cmd& cmd);

    SlaveProxy();

    friend class ReturnRet;
    friend class HandShake;
    friend class Feedback;
    friend class MasterMD5Cracker;
};

//Handler for received command from slave

//Slave send feedback for unsuccessful previos passwords 
//Not implemented so far
class Feedback : public xmlrpc_c::method{

private:
    SlaveProxy* slave;

public:
    Feedback(SlaveProxy* sp){
        this->slave = sp;
        this->_help = "Slave report unsuccessfull range of passwords";
    }

    void execute(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* retValP );

};

//Slave send password found 
class ReturnRet : public xmlrpc_c::method{

private:
    SlaveProxy* slave;

public:
    ReturnRet(SlaveProxy* sp){
        this->slave = sp;
        this->_help = "Slave found the pass";
    }

    void execute(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* retValP );

};

//Slave invite master to connect back to itself 
//Actually one socket is enough for concurrent duplex communication
//when associate with two threads
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
