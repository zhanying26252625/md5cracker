#include "../configure.h"
#include "slaveMD5Cracker.h"
#include "passGenerator.h"
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
    isCracking = false;
}

//Two threads with two sockets for concurrent duplex communication
void SlaveMD5Cracker::run(){
   
    masterIP = getMasterIP();

    //Not used, just list all IPs that I have
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

    int sock = 0;

    bool good = createAndBindSocket(portNum,sock);

    if( !good ){
        cout<<"Can't create a socket"<<endl;
        return;
    }

    this->myListenPort = portNum;

    this->listeningSocket = sock;

    //create the thread which would send cmds to master
    pthread_t thread;

    int rc = pthread_create(&thread,NULL,SlaveMD5Cracker::masterSenderFunc, (void*)this );

    if( -1 == rc ){
        cout<<"Can't create sender thread"<<endl;
        return;
    }

    //receiver cmd from master
    masterReceiverFunc(this->myListenPort);
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

//senc commands to master
//Handshake,etc
void* SlaveMD5Cracker::masterSenderFunc(void* arg){

    SlaveMD5Cracker* slave = (SlaveMD5Cracker*)arg;

    //Inject handshake first
    Cmd handshakeCmd("handshake",slave->myListenPort); 
    slave->cmdQueue.push(handshakeCmd);

    while(1){

        if( slave->cmdQueue.size() > 0 ){
            
            Cmd cmd = slave->cmdQueue.front();
            slave->cmdQueue.pop();

            MasterProxy::sendViaXMLRPC(slave->toMasterSocket, cmd.name,cmd.strVal,cmd.longVal,cmd.intVal);      

        }
        else{
            usleep(500);
        }
    }
    return NULL;
}

//Let system select an available port for me
bool SlaveMD5Cracker::createAndBindSocket(int& socketPort, int& sock){

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

    //std::bind is something else
    if(::bind(listeningSocket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))== -1) {
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

    sock = listeningSocket;

    return true;
}

bool SlaveMD5Cracker::masterReceiverFunc(int listenPort){

    struct sockaddr_in  master_addr;    
    
    unsigned int sin_size = sizeof(struct sockaddr_in);

    //accept connection from master
    int master2MeSocket = accept(listeningSocket, (struct sockaddr*)&master_addr,&sin_size  );

    string masterAddr = inet_ntoa(master_addr.sin_addr);

    int masterPort = ntohs(master_addr.sin_port);

    cout << "Command Receiver at slave : Master connected to me, it is at ["<<masterAddr<<"] ["<<masterPort<<"]"<<endl<<"Now we can get started to work"<<endl;
    
    //install the command handlers
    try{

        xmlrpc_c::registry myReg;

        const xmlrpc_c::methodPtr startMethodP(new StartMethod(this));
        const xmlrpc_c::methodPtr stopMethodP(new StopMethod(this));
        const xmlrpc_c::methodPtr receiveChunkMethodP(new ReceiveChunkMethod(this));
        const xmlrpc_c::methodPtr statusMethodP(new StatusMethod(this));
        const xmlrpc_c::methodPtr quitMethodP(new QuitMethod(this));
        
        myReg.addMethod("start",startMethodP);
        myReg.addMethod("stop",stopMethodP);
        myReg.addMethod("receiveChunk",receiveChunkMethodP);
        myReg.addMethod("status",statusMethodP);
        myReg.addMethod("quit",quitMethodP);

        xmlrpc_c::serverPstreamConn server(
            xmlrpc_c::serverPstreamConn::constrOpt()
            .socketFd( master2MeSocket )
            .registryP(&myReg));

        //It runs forever until pipe breaks
        server.run();

        cout<<"Master is gone! I would vanish as well"<<endl;
    }
    catch(const exception& e){
        cout << "[Exception]"<<e.what()<<endl;
    }

    return true;
}

void SlaveMD5Cracker::reportResult(string pass ,string md5){

    cout <<"Password ["<<pass<<"] => md5 ["<<md5<<"]"<<endl;

    //stop working thread and wont receive chunks any more
    //this->isCracking = false;

    //send cmd back to master
    Cmd retCmd("returnRet",pass);

    this->cmdQueue.push(retCmd);

}

//Start a bunch of threads to crack the password
//As many cores as CPU threads
//One dedicated GPU thread
//One password generation thread
void StartMethod::execute(const xmlrpc_c::paramList& paramList,xmlrpc_c::value* retValP ){

    cout << "[Command] Start cracking" <<endl;

    string md5 = paramList.getString(0);

    cout <<"The md5 is ["<<md5<<"]"<<endl;

    *retValP = xmlrpc_c::value_string(string("Accept"));

    slaveCracker->targetMd5 = md5;

    //refresh buffer,since it may be dirty becaues of previous cracking
    slaveCracker->rwBuf.init();

    //spawn cpu working threads
    slaveCracker->cpuCracker.init(slaveCracker);

    //spawn gpu working threads
    slaveCracker->gpuCracker.init(slaveCracker);

    //We have a dedicated thread producing passwords
    slaveCracker->rwBuf.run();

    slaveCracker->isCracking = true;

    cout <<"Start Done!"<<endl;

    return;
}

//Kill all pre-allocated threads
void StopMethod::execute(const xmlrpc_c::paramList& paramList,xmlrpc_c::value* retValP ){

    cout << "[Command] Stop cracking" <<endl;
    
    *retValP = xmlrpc_c::value_string(string("Accept"));

    slaveCracker->targetMd5 = string("");
    
    slaveCracker->isCracking = false;
   
    //terminate threads
    slaveCracker->cpuCracker.terminate();

    slaveCracker->gpuCracker.terminate();

    slaveCracker->rwBuf.end();

    slaveCracker->rwBuf.destroy();

    cout <<"Stop Done!"<<endl;
}

void ReceiveChunkMethod::execute(const xmlrpc_c::paramList& paramList,xmlrpc_c::value* retValP ){

    if(slaveCracker->isCracking){
        *retValP = xmlrpc_c::value_string(string("Accept"));
    }
    else{
        *retValP = xmlrpc_c::value_string(string("Deny"));
        return;
    }
    //cout << "[Command] Receive Chunk of Passwords" <<endl;
    //value_i8 is long long, which is supported by xmlrpc
    string strVal = paramList.getString(0);
    len_t start = paramList.getI8(1);
    int chunkSize = paramList.getInt(2);

    //cout << "[Command] Receive Password range ["<<start<<","<< start + chunkSize << ")" <<endl;
    PassGenerator gp(start,chunkSize);
    
    slaveCracker->rwBuf.injectPG(gp);

    //Maybe we should have a dedicated thread to generate password, then it wont block future cmd from master
    /*
    PassGenerator gp(start,chunkSize);
    //c++11
    for(string& pass : gp.generateAll()){
        //cout<<pass<<endl;
        
        if(!slaveCracker->isCracking)
            break;

        slaveCracker->rwBuf.produce(pass);
    }
    */
}

//Not implemented yet
void StatusMethod::execute(const xmlrpc_c::paramList& paramList,xmlrpc_c::value* retValP ){

    cout << "[Command] Status" <<endl;
    
    *retValP = xmlrpc_c::value_string(string("Accept"));
}

void QuitMethod::execute(const xmlrpc_c::paramList& paramList,xmlrpc_c::value* retValP ){

    cout << "[Command] Quit" <<endl;
    
    *retValP = xmlrpc_c::value_string(string("Accept"));

    cout <<"Terminated by master"<<endl;
   
    //cancel working threads
    slaveCracker->cpuCracker.terminate();

    //end this program
    exit(1);
}

//Read from configure file
string SlaveMD5Cracker::getMasterIP(string fileName){

    string ip("");

    ifstream file(fileName);

    if( file.is_open())
        getline(file,ip);
    
    file.close();

    return ip;
}

//List all ip address that I might have
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



