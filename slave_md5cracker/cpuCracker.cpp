#include "cpuCracker.h"
#include "slaveMD5Cracker.h"
#include "md5.h"
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

CpuCracker::CpuCracker(){
    cores=0;
}

bool CpuCracker::init(SlaveMD5Cracker* sc){

    this->slaveCracker = sc;

    cores = getNumOfCores();

    cout<< "CpuCracker init, "<< cores <<"cores"<<endl;

    for( int i = 0 ;i < cores; i++){
        
        pthread_t thread;

        int rc = pthread_create(&thread,NULL,workThreadFunc,this);

        if( -1 == rc ){
            cout<<"Can't creat cpu cracking thread"<<endl;

            return false;
        }

        workThreads.push_back(thread);
    }

    return true;
}

void* CpuCracker::workThreadFunc(void* arg){

    int counter = 0;

    CpuCracker* cpuCracker = (CpuCracker*)arg;

    SlaveMD5Cracker* slaveCracker = cpuCracker->slaveCracker; 

    cout<<"CpuCracker::workThreadFunc"<<endl;

    while( 1 ){
    
        if(!slaveCracker->isCracking){
            usleep(200);
        }
        else{

            vector<string> passwords = slaveCracker->rwBuf.consume();
            
            string md5 = slaveCracker->targetMd5;

            for(unsigned int i = 0; i<passwords.size(); i++){

                counter++;

                string pass = passwords[i];
                
                cout<<"No."<<counter<<" Password :"<<pass<<endl;
                
                MD5 md5Engine;

                string ret = md5Engine.calMD5FromString(pass);

                if(!ret.compare(md5)){

                    cout <<"----------Found the password ["<<pass<<"]----------"<<endl;
                    exit(1); 
                }
            }
        }
    }

    return NULL;
}

int CpuCracker::getNumOfCores(){
    return sysconf(_SC_NPROCESSORS_ONLN);
}

