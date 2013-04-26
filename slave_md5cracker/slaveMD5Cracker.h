#ifndef _SLAVE_MD5CRACKER_H
#define _SLAVE_MD5CRACKER_H

#include "rwBuffer.h"
#include "masterProxy.h"
#include "cpuCracker.h"
#include "gpuCracker.h"
#include <string>
#include <queue>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_pstream.hpp>
#include <xmlrpc-c/girerr.hpp>
#include <xmlrpc-c/client.hpp>
#include <xmlrpc-c/client_transport.hpp>

using namespace std;

class MasterProxy;
class StartMethod;
class ReceiveChunkMethod;
class StopMethod;
class StatusMethod;
class QuitMethod;

class SlaveMD5Cracker{

private:

    string getMasterIP(string fileName=string("masterIP.txt"));

    string getMyIP();

    bool connectToMaster(string ip,int& toMasterSocket);

    //Send cmd to master
    static void* masterSenderFunc(void* arg); 

    queue<Cmd> cmdQueue;
    
    MasterProxy masterProxy;
    //creat and bind a listening socket
    bool createAndBindSocket(int& socketPort,int& sock);
    //Receive cmd from master
    bool masterReceiverFunc(int listenPort);

    //Found the result
    void reportResult(string pass ,string md5);

    int toMasterSocket;
    
    string myIP;

    string masterIP;

    int myListenPort;

    int listeningSocket;

    bool isCracking ;

    string targetMd5;
    //reader writer buffer to store the passwords generated.
    RWBuffer rwBuf;

    CpuCracker cpuCracker;

    GpuCracker gpuCracker;

public:

    SlaveMD5Cracker();

    void run();

    //Cmd handler
    friend class StartMethod;
    friend class ReceiveChunkMethod;
    friend class StopMethod;
    friend class StatusMethod;
    friend class QuitMethod;

    friend class MasterProxy;
    friend class CpuCracker;
    friend class GpuCracker;
};

class StartMethod : public xmlrpc_c::method{
private:
    SlaveMD5Cracker* slaveCracker;
public:
    StartMethod( SlaveMD5Cracker* sc){
        this->slaveCracker = sc;
        this->_help = "start cracking";
    }
    void execute(const xmlrpc_c::paramList& paramList,xmlrpc_c::value* retValP );
};

class ReceiveChunkMethod : public xmlrpc_c::method{
private:
    SlaveMD5Cracker* slaveCracker;
public:
    ReceiveChunkMethod( SlaveMD5Cracker* sc){
        this->slaveCracker = sc;
        this->_help = "receive a chunk of password";
    }
    void execute(const xmlrpc_c::paramList& paramList,xmlrpc_c::value* retValP );
};

class StopMethod : public xmlrpc_c::method{
private:
    SlaveMD5Cracker* slaveCracker;
public:
    StopMethod( SlaveMD5Cracker* sc){
        this->slaveCracker = sc;
        this->_help = "stop cracking";
    }
    void execute(const xmlrpc_c::paramList& paramList,xmlrpc_c::value* retValP );
};


class StatusMethod : public xmlrpc_c::method{
private:
    SlaveMD5Cracker* slaveCracker;
public:
    StatusMethod( SlaveMD5Cracker* sc){
        this->slaveCracker = sc;
        this->_help = "status";
    }
    void execute(const xmlrpc_c::paramList& paramList,xmlrpc_c::value* retValP );
};


class QuitMethod : public xmlrpc_c::method{
private:
    SlaveMD5Cracker* slaveCracker;
public:
    QuitMethod( SlaveMD5Cracker* sc){
        this->slaveCracker = sc;
        this->_help = "quit";
    }
    void execute(const xmlrpc_c::paramList& paramList,xmlrpc_c::value* retValP );
};

#endif
