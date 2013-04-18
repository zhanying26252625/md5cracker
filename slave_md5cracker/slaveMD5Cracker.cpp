#include "../configure.h"
#include "slaveMD5Cracker.h"

#include <iostream>
#include <string>
#include <fstream>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

SlaveMD5Cracker::SlaveMD5Cracker(){

    this->state = HANDSHAKE;
}

void SlaveMD5Cracker::run(){

    string masterIP = getMasterIP();

    if( !masterIP.compare("") ){
        cout<<"Can't get IP of master"<<endl;
        return;
    }
    else{
        cout<<"Master is at ["<<masterIP<<"]"<<endl;
    }

    cout <<"Slave is runing......"<<endl;
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

    //create the thread which initiate data transfer to master
    pthread_t thread;

    int rc = pthread_create(&thread,NULL,SlaveMD5Cracker::masterSenderFunc, (void*)this );

    if( -1 == rc ){
        cout<<"Can't create sender thread"<<endl;
        return;
    }
    
    //receiver cmd from master
    masterReceiverFunc();
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
                break;

            case FETCH:
                MasterProxy::fetch(slave);
                break;

            case STOP:
                MasterProxy::stop(slave);
                break;
                
            default:
                break;
        }
    }

    return NULL;
}

void SlaveMD5Cracker::masterReceiverFunc(){

    //Receiver the commands from master
    cout<<"Receiving commands from master......"<<endl;
}

string SlaveMD5Cracker::getMasterIP(string fileName){

    string ip("");

    ifstream file(fileName);

    if( file.is_open())
        getline(file,ip);
    
    file.close();

    return ip;
}
