#include "../configure.h"
#include "slaveProxy.h"
#include "masterMD5Cracker.h"
#include "logManager.h"
#include <iostream>
#include <string>
#include <fstream>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <cassert>
#include <pthread.h>
#include <signal.h>

using namespace std;

SlaveProxy::SlaveProxy(){
    isExisting = false;
    isFullConnected = false;
}

void SlaveProxy::issueCmd(Cmd& cmd){
    this->cmdQueue.push(cmd);
}

//create the command receiver thread
bool SlaveProxy::run(){

    LogManager& logMgr = LogManager::getInstance();

    pthread_t thread;

    int rc = pthread_create(&thread,NULL,SlaveProxy::slaveReceiverFunc,(void*)this);

    if(-1 == rc){
        logMgr << "[ERROR] can't create slave proxy thread" << endl;
        return false;
    }

    this->threadMasterReceiver = thread;

    return true;
}

//Receiver thread 
void* SlaveProxy::slaveReceiverFunc(void* arg){

    SlaveProxy* slave = (SlaveProxy*)arg;

    LogManager& logMgr = LogManager::getInstance();

    logMgr << "New slave receiver thread is running......["<<slave->key<<"]" <<endl;

    try{

        xmlrpc_c::registry myReg;

        const xmlrpc_c::methodPtr feedbackMethodP(new Feedback(slave));
        const xmlrpc_c::methodPtr handShakeMethodP(new HandShake(slave));
        const xmlrpc_c::methodPtr returnRetMethodP(new ReturnRet(slave));
        
        myReg.addMethod("feedback",feedbackMethodP);
        myReg.addMethod("handshake",handShakeMethodP);
        myReg.addMethod("returnRet",returnRetMethodP);
        
        xmlrpc_c::serverPstreamConn server(
            xmlrpc_c::serverPstreamConn::constrOpt()
            .socketFd( slave->socket2Master )
            .registryP(&myReg));

        //It runs forever except when pipi breaks
        server.run();
        
        logMgr << "One slave left ["<< slave->key <<"]" <<endl;

        logMgr << "Kill both associated sender and receiver thread" <<endl;
        
        slave->isExisting = true;

        //wait sender exit
        void* retVal;
        pthread_join(slave->threadMasterSender,&retVal);

        close(slave->socket2Slave);
        close(slave->socket2Master);

        //unregister myself, be care of race condition
        slave->master->unregisterSlave(slave->key);

        //We shoud redistribute passwords which are currently worked by this slave to other slaves
    
    }
    catch(const exception& e){
        logMgr << "[Exception]"<<e.what()<<endl;
    }

    return NULL;
}

//Sender thread to slave
void* SlaveProxy::slaveSenderFunc(void* arg){

    //We should ignore SIGPIPE,it's already set
    signal(SIGPIPE,SIG_IGN);

    SlaveProxy* slave = (SlaveProxy*)arg;

    LogManager& logMgr = LogManager::getInstance();

    logMgr << "New slave sencder thread is running......["<<slave->key<<"]" <<endl;

    while(1){

        //Receiver thread would find out that the slave is gone then notify me
        if(slave->isExisting)
            break;

        if( slave->cmdQueue.size() > 0 ){
            
            Cmd cmd = slave->cmdQueue.front();
            slave->cmdQueue.pop();
           
            //logMgr <<"[SlaveProxy] Master send " << cmd.name << " command to "<< slave->slaveAddr <<endl;

            SlaveProxy::sendViaXMLRPC(slave->socket2Slave, cmd.name,cmd.strVal,cmd.longVal,cmd.intVal);      

        }
        else{
            usleep(500);
        }
    }

    return NULL;
}

//Send command to slave through raw-xmlrpc
bool SlaveProxy::sendViaXMLRPC(int socket, string name,  string strVal, len_t longVal, int intVal){

    LogManager& logMgr = LogManager::getInstance();
    
    bool ok = true;

    try {

        xmlrpc_c::clientXmlTransport_pstream myTransport(
            xmlrpc_c::clientXmlTransport_pstream::constrOpt().fd( socket ));

        xmlrpc_c::client_xml myClient(&myTransport);

        string const methodName(name);

        //parameters
        xmlrpc_c::paramList Parms;

        Parms.add(xmlrpc_c::value_string( strVal ));
        Parms.add(xmlrpc_c::value_i8( longVal ));
        Parms.add(xmlrpc_c::value_int( intVal ));

        xmlrpc_c::rpcPtr myRpcP(methodName, Parms);

        xmlrpc_c::carriageParm_pstream myCarriageParm;
        // Empty; transport doesn't need any information

        myRpcP->call(&myClient, &myCarriageParm);

        assert(myRpcP->isFinished());

        const string ans(xmlrpc_c::value_string(myRpcP->getResult()));
            // Assume the method returned an integer; throws error if not

        //logMgr << ans << endl;

    } catch (exception const& e) {
        logMgr << "Client threw error: " << e.what() << endl;
        ok = false;
    } catch (...) {
        //socket maybe broken, slave maybe lost
        logMgr << "Client threw unexpected error." << endl;
        ok = false;
    }

    return ok;
}

void SlaveProxy::terminate(){

}

//report the unsuccessful bundle of passwords, start(len_t)
//Not implemented so far
void Feedback::execute(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* retValP ){

    LogManager& logMgr = LogManager::getInstance();
    
    logMgr <<"[Feedback] from "<< slave->key<<endl;

    //string strVal = paramList.getString(0);
    len_t longVal = paramList.getI8(1);
    //int intVal = paramList.getInt(2);

    len_t start = longVal;

    cout<< "Report unsuccessful range ["<<start<<","<<start+slave->master->chunkSize <<")" <<endl;

    *retValP = xmlrpc_c::value_string(string("Master accept my cmd"));
}

//found the password for the specific md5
void ReturnRet::execute(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* retValP ){

    LogManager& logMgr = LogManager::getInstance();
    
    logMgr <<"[PassFound] from "<< slave->key<<endl;

    string strVal = paramList.getString(0);
    //len_t longVal = paramList.getI8(1);
    //int intVal = paramList.getInt(2);

    string pass = strVal;
    //cout<<"["<<pass<<"] --> ["<<slave->master->md5<<"]" <<endl;

    slave->master->reportFoundPass(pass);
    
    *retValP = xmlrpc_c::value_string(string("Master appreciate your effort of cracking the md5"));
}

//Invite master to create new connection to slave
void HandShake::execute(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* retValP ){

    LogManager& logMgr = LogManager::getInstance();
    
    logMgr <<"[Handshake] from "<< slave->key<<endl;

    //Not likely 
    if( slave->isFullConnected ){
        logMgr << "Duplicate Handshake from same slave" <<endl;
        *retValP = xmlrpc_c::value_string(string("[WARN] duplicate handshake"));
        return;
    }

    bool ok = true;

    //We should then connect to slave trough a new socket for bi-diretional communication.
    //the paremeter is the listenPort sent by slave
    string slaveIP = slave->slaveAddr;

    //string strVal = paramList.getString(0);
    //len_t longVal = paramList.getI8(1);
    int intVal = paramList.getInt(2);

    int slaveListeningPort = intVal;
    
    logMgr <<"Slave is listening on port "<<slaveListeningPort <<endl;

    int sock;  
    
    struct hostent *host;
    struct sockaddr_in server_addr;  

    host = gethostbyname(slaveIP.c_str());

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        logMgr<<"[ERROR] In handshake : Socket error"<<endl;
        ok = false; 
    }

    server_addr.sin_family = AF_INET;     
    server_addr.sin_port = htons( slaveListeningPort );   
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(server_addr.sin_zero),8); 

    if (connect(sock, (struct sockaddr *)&server_addr,
                    sizeof(struct sockaddr)) == -1) 
    {
        cout<<"[ERROR] In handshake : Connect error"<<endl;
        ok = false;
    }

    //create a dedicated thread that initiate cmd and data transfer to slave

    pthread_t thread;

    //Command sender thread
    int rc = pthread_create(&thread,NULL,SlaveProxy::slaveSenderFunc,(void*)slave);

    if(-1 == rc){
        logMgr << "[ERROR] can't create thread sending cmd to slave" << endl;
        ok = false;
    }

    if(ok){
        slave->threadMasterSender = thread;
        slave->isFullConnected = true;
        //register the socket, sending cmd to slave
        slave->socket2Slave = sock;
        *retValP = xmlrpc_c::value_string(string("Welcome you, my new slave"));
    }
    else{
        *retValP = xmlrpc_c::value_string(string("Error in Master"));
    }
}


