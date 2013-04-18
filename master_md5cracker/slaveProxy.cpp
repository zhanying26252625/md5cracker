#include "slaveProxy.h"
#include "logManager.h"
#include <string>
#include <pthread.h>

using namespace std;

SlaveProxy::SlaveProxy(){

}

bool SlaveProxy::run(){

    LogManager& logMgr = LogManager::getInstance();

    pthread_t thread;

    int rc = pthread_create(&thread,NULL,SlaveProxy::slaveReceiverFunc,(void*)this);

    if(-1 == rc){
        logMgr << "[ERROR] can't create slave proxy thread" << endl;
        return false;
    }

    this->thread2Master = thread;

    return true;
}

//Receiver thread for slave
void* SlaveProxy::slaveReceiverFunc(void* arg){

    SlaveProxy* slave = (SlaveProxy*)arg;

    LogManager& logMgr = LogManager::getInstance();

    logMgr << "New slave receiver thread is running......["<<slave->key<<"]" <<endl;

    try{

        xmlrpc_c::registry myReg;

        const xmlrpc_c::methodPtr fetchMethodP(new Fetch(slave));
    
        myReg.addMethod("fetch",fetchMethodP);

        const xmlrpc_c::methodPtr handShakeMethodP(new HandShake(slave));

        myReg.addMethod("handshake",handShakeMethodP);

        xmlrpc_c::serverPstreamConn server(
            xmlrpc_c::serverPstreamConn::constrOpt()
            .socketFd( slave->socket2Master )
            .registryP(&myReg));

        server.run();
    }
    catch(const exception& e){
        logMgr << "[Exception]"<<e.what()<<endl;
    }

    return NULL;
}

//Sender thread to slave
void* SlaveProxy::slaveSenderFunc(void* arg){

    SlaveProxy* slave = (SlaveProxy*)arg;

    LogManager& logMgr = LogManager::getInstance();

    logMgr << "New slave sencder thread is running......["<<slave->key<<"]" <<endl;

    return NULL;
}

void SlaveProxy::terminate(){

}

//Get new batch of password to hash md5 code
void Fetch::execute(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* retValP ){

    LogManager& logMgr = LogManager::getInstance();
    
    logMgr << slave->key <<"[Fetch]" <<endl;

}

//Invite master to create new connection to slave
void HandShake::execute(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* retValP ){

    LogManager& logMgr = LogManager::getInstance();
    
    logMgr << slave->key <<"[Handshake]" <<endl;

}
