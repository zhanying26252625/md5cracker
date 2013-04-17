#include "masterMD5Cracker.h"
#include "logManager.h"
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <string>

using namespace std;

MasterMD5Cracker::MasterMD5Cracker(){

    isExisting = false;
}

MasterMD5Cracker::~MasterMD5Cracker(){
    
}

bool MasterMD5Cracker::createListeningThread(){

    return true;
}

void MasterMD5Cracker::run(){

    LogManager& logMgr = LogManager::getInstance();

    logMgr << "MasterMD5Cracker is running......"<<endl;
    
    //Ignore SIGPIPE signal, which would kill the process when pipe breaks
    signal(SIGPIPE,SIG_IGN);

    //Create a background listening thread
    if( ! createListeningThread() ){
        logMgr << "Can't Create listening thread!"<<endl;
    }

    //Accept commands from clients

    while( isExisting ){

        string cmd;
        //Only get one word
        //cin >> cmd;
        //Get whole line
        getline(cin,cmd);


    }
}

