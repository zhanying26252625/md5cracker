#include "../configure.h"
#include "slaveMD5Cracker.h"

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

using namespace std;

SlaveMD5Cracker::SlaveMD5Cracker(){

    this->state = HANDSHAKE;
}

void SlaveMD5Cracker::run(){

    masterIP = getMasterIP();

    myIP = getMyIP();

    if( !masterIP.compare("") ){
        cout<<"Can't get IP of master"<<endl;
        return;
    }
    else{
        cout<<"Master is at ["<<masterIP<<"]"<<endl;
    }

    //try to connect to master
    int toMasterSocket = -1;

    bool ret = connectToMaster(masterIP,toMasterSocket);

    if( !ret ){
        cout<<"Can't connect to master"<<endl;
        return;
    }
    else{
        this->toMasterSocket = toMasterSocket;
        cout<<"Connect to master server"<<endl;
    }

    //get an available port on this machine, let system select one
    int portNum = 0;

    bool good = createAndBindSocket(portNum);

    if( !good ){
        cout<<"Can't create a socket"<<endl;
        return;
    }
    this->myListenPort = portNum;

    //create the thread which initiate data transfer to master
    pthread_t thread;

    int rc = pthread_create(&thread,NULL,SlaveMD5Cracker::masterSenderFunc, (void*)this );

    if( -1 == rc ){
        cout<<"Can't create sender thread"<<endl;
        return;
    }

    //receiver cmd from master
    masterReceiverFunc(myListenPort);
}

bool SlaveMD5Cracker::connectToMaster(string ip,int& toMasterSocket){

        int sock;  
        struct hostent *host;
        struct sockaddr_in server_addr;  

        host = gethostbyname(ip.c_str());

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            cout<<"Socket error"<<endl;
            return false;
        }

        server_addr.sin_family = AF_INET;     
        server_addr.sin_port = htons( MASTER_PORT );   
        server_addr.sin_addr = *((struct in_addr *)host->h_addr);
        bzero(&(server_addr.sin_zero),8); 

        if (connect(sock, (struct sockaddr *)&server_addr,
                    sizeof(struct sockaddr)) == -1) 
        {
            cout<<"Connect error"<<endl;
            return false;
        }

        toMasterSocket = sock;

        return true;
}

void* SlaveMD5Cracker::masterSenderFunc(void* arg){
    //senc commands to master
    //Handshake,fetch,etc
    SlaveMD5Cracker* slave = (SlaveMD5Cracker*)arg;

    cout<<"Send handshake command to master"<<endl;

    while(1){

        switch(slave->state){

            case HANDSHAKE:
                MasterProxy::handshake(slave);
                slave->state = WAIT;
                break;

            case FETCH:
                MasterProxy::fetch(slave);
                break;

            case STOP:
                MasterProxy::stop(slave);
                break;
                
            case WAIT:
                usleep(500);
                break;

            default:
                break;
        }
    }

    return NULL;
}

bool SlaveMD5Cracker::createAndBindSocket(int& socketPort){

    //create and listen the socket 
    int listeningSocket = 0;  

    struct sockaddr_in server_addr;    
        
    if((listeningSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cout << "[ERROR]Can't create Socket" << endl;
        return false;
    }

    int option = 1;
    if(setsockopt(listeningSocket,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(int)) == -1) {
        cout << "[ERROR]Can't setsocket" << endl;
        return false;
    }
        
    server_addr.sin_family = AF_INET;
    //0 means letting system to pickup an available port
    server_addr.sin_port = htons( 0 );     
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    bzero(&(server_addr.sin_zero),8); 

    if(bind(listeningSocket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))== -1) {
        cout << "[ERROR]Unable to bind" <<endl;
        return false;
    }

    if (listen(listeningSocket, 5) == -1) {
        cout << "[ERROR]Listen error" << endl;
        return false;
    }

    //find out the port number picked by system
    struct sockaddr_in myAddr;    
    unsigned int sin_len = sizeof(struct sockaddr_in);
    getsockname(listeningSocket, (struct sockaddr *)&myAddr, &sin_len);

    int port = ntohs(myAddr.sin_port);
    cout <<"My port number is "<< port <<endl;

    socketPort = port;

    this->listeningSocket = listeningSocket;

    return true;
}

bool SlaveMD5Cracker::masterReceiverFunc(int listenPort){

    struct sockaddr_in  master_addr;    
    
    unsigned int sin_size = sizeof(struct sockaddr_in);

    //accept connection from master
    int master2MeSocket = accept(listeningSocket, (struct sockaddr*)&master_addr,&sin_size  );

    string masterAddr = inet_ntoa(master_addr.sin_addr);

    int masterPort = ntohs(master_addr.sin_port);

    cout << "Command Receiver at slave : Master connected to me, it is at ["<<masterAddr<<"] ["<<masterPort<<"]"<<endl;
    
    //run server for master
    try{

        xmlrpc_c::registry myReg;

        const xmlrpc_c::methodPtr startMethodP(new StartMethod(this));
        const xmlrpc_c::methodPtr stopMethodP(new StopMethod(this));
        const xmlrpc_c::methodPtr statusMethodP(new StatusMethod(this));
        const xmlrpc_c::methodPtr quitMethodP(new QuitMethod(this));
        
        myReg.addMethod("start",startMethodP);
        myReg.addMethod("stop",stopMethodP);
        myReg.addMethod("status",statusMethodP);
        myReg.addMethod("quit",quitMethodP);

        xmlrpc_c::serverPstreamConn server(
            xmlrpc_c::serverPstreamConn::constrOpt()
            .socketFd( master2MeSocket )
            .registryP(&myReg));

        server.run();

    }
    catch(const exception& e){
        cout << "[Exception]"<<e.what()<<endl;
    }

    return true;
}


void StartMethod::execute(const xmlrpc_c::paramList& paramList,xmlrpc_c::value* retValP ){

    cout << "[Command] Start cracking" <<endl;

    string md5 = paramList.getString(0);

    cout <<"The md5 is ["<<md5<<"]"<<endl;

    *retValP = xmlrpc_c::value_string(string("Accept"));

    return;
}

void StopMethod::execute(const xmlrpc_c::paramList& paramList,xmlrpc_c::value* retValP ){

    cout << "[Command] Stop cracking" <<endl;
    
    *retValP = xmlrpc_c::value_string(string("Accept"));
}

void StatusMethod::execute(const xmlrpc_c::paramList& paramList,xmlrpc_c::value* retValP ){

    cout << "[Command] Status" <<endl;
    
    *retValP = xmlrpc_c::value_string(string("Accept"));
}

void QuitMethod::execute(const xmlrpc_c::paramList& paramList,xmlrpc_c::value* retValP ){

    cout << "[Command] Quit" <<endl;
    
    *retValP = xmlrpc_c::value_string(string("Accept"));

    cout <<"Terminated by master"<<endl;
    //end this program
    exit(1);
}


string SlaveMD5Cracker::getMasterIP(string fileName){

    string ip("");

    ifstream file(fileName);

    if( file.is_open())
        getline(file,ip);
    
    file.close();

    return ip;
}

string SlaveMD5Cracker::getMyIP(){

    struct ifaddrs* ifAddrs = NULL;
    
    getifaddrs(&ifAddrs);

    struct ifaddrs* ifIter = ifAddrs;

    while(ifIter!=NULL){

        //IPV4
        if(ifIter->ifa_addr->sa_family == AF_INET){
        
            void* tmpAddrPtr=&((struct sockaddr_in *)ifIter->ifa_addr)->sin_addr;

            char ip[INET_ADDRSTRLEN];

            inet_ntop(AF_INET,tmpAddrPtr,ip, INET_ADDRSTRLEN);

            string ipAddr = string(ip);

            cout<< "My IP : "<<ipAddr<<endl;
        }

        ifIter = ifIter->ifa_next;
    }

    return "";
}


int SlaveMD5Cracker::getAvailablePort(string tmpFile){

    return SLAVE_PORT;
}






