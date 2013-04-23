#ifndef _SLAVE_MD5CRACKER_H
#define _SLAVE_MD5CRACKER_

#include "masterProxy.h"
#include <string>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_pstream.hpp>
#include <xmlrpc-c/girerr.hpp>
#include <xmlrpc-c/client.hpp>
#include <xmlrpc-c/client_transport.hpp>

using namespace std;

class MasterProxy;

class SlaveMD5Cracker{

private:

    enum State{HANDSHAKE,WAIT,FETCH,STOP};

private:

    string getMasterIP(string fileName=string("masterIP.txt"));

    string getMyIP();

    int getAvailablePort(string tmpFile = string("port.txt"));

    bool connectToMaster(string ip,int& toMasterSocket);

    //Send cmd to master
    static void* masterSenderFunc(void* arg); 
    //creat and bind a listening socket
    bool createAndBindSocket(int& socketPort);
    //Receive cmd from master
    bool masterReceiverFunc(int listenPort);

    MasterProxy masterProxy;

    int toMasterSocket;
    
    string myIP;

    string masterIP;

    int myListenPort;

    int listeningSocket;

    enum State state;

public:

    SlaveMD5Cracker();

    void run();
    
    friend class MasterProxy;
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
