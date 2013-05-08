#include "cpuCracker.h"
#include "gpuCracker.h"
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

    workThreads.clear();

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

    cout<<"Cpu Cracker::workThreadFunc"<<endl;

    while( 1 ){
    
        if(!slaveCracker->isCracking){
            usleep(400);
        }
        else{

            vector<string> passwords = slaveCracker->rwBuf.consume();
            
            string md5 = slaveCracker->targetMd5;

            for(unsigned int i = 0; i<passwords.size(); i++){

                counter++;

                string pass = passwords[i];
                
                MD5 md5Engine;

                string ret = md5Engine.calMD5FromString(pass);

                //test only
                /*
                if(!pass.compare(string("girl")))
                    cout<<"consume-----------GrilGirlGrilGrilGril" <<endl;
                */

                if(!ret.compare(md5)){

                    cout<<endl<<"**********************************"<<endl;
                    cout<<endl<<"***********Found by CPU***********"<<endl;
                    cout<<endl<<"**********************************"<<endl;

                    //cout <<"----------Found the password ["<<pass<<"]----------"<<endl;
                    slaveCracker->reportResult(pass,md5);
                    //exit(1); 
                }
            }
        }
    }
    return NULL;
}


int CpuCracker::getNumOfCores(){
    return sysconf(_SC_NPROCESSORS_ONLN);
}

bool CpuCracker::terminate(){

    //Cancel the thrreads
    for(unsigned int i =0 ;i< workThreads.size();i++){
        pthread_cancel(workThreads[i]);
    }
    //wait for their exits
    for(unsigned int i =0 ;i< workThreads.size();i++){
        void* ret;
        pthread_join(workThreads[i],&ret);

        cout <<"CPU thread[" <<i <<"] terminated!" <<endl;
    }

    workThreads.clear();

    return true;
}



