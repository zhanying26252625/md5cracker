#include "gpuCracker.h"
#include "slaveMD5Cracker.h"
#include "cudaCracker.h"
#include <unistd.h>
#include <stdlib.h>
#include <vector>

using namespace std;

#define CUDA_MD5 1

#ifdef CUDA_MD5
#include "cudaCracker.h"
#endif

GpuCracker::GpuCracker(){

}

bool GpuCracker::init(SlaveMD5Cracker* sc){

    slaveCracker = sc;

    bool flag = isGPUsupported();

    if(!flag){
        cout <<"No GPU available"<<endl;
        return false;
    }

    for(int i=0;i<NUM_THREADS;i++){

        pthread_t thread;

        int rc = pthread_create(&thread,NULL,workThreadFunc,this);

        if( -1 == rc ){
            cout<<"Can't creat gpu cracking thread"<<endl;
            return false;
        }
    
        this->workThreads.push_back( thread );
    
    }

    cout<<"GpuCracker init, 2threads"<<endl;

    return true;
}

bool GpuCracker::isGPUsupported(){

    bool isSupported = true;

    return slaveCracker->gpuOn && isSupported; 
}

bool GpuCracker::terminate(){

    bool flag = isGPUsupported();
    //no gpu thread is running
    if(!flag)
        return true;
    
    for(unsigned int i=0;i<workThreads.size();i++){

        pthread_cancel(workThreads[i]);
    }

    for(unsigned int i=0;i<workThreads.size();i++){
    
        void* ret;

        pthread_join(workThreads[i],&ret);

        cout <<"GPU thread[" <<i <<"] terminated!" <<endl;

    }

    workThreads.clear();

    return true;
}


void* GpuCracker::workThreadFunc(void* arg){

    cout<<"Gpu Cracker::workThreadFunc"<<endl;

    GpuCracker* gpuCracker = (GpuCracker*)arg;

    SlaveMD5Cracker* slaveCracker = gpuCracker->slaveCracker; 

    while(1){
        
        if(!slaveCracker->isCracking){
            usleep(400);
            //cout<<"GPU"<<endl;
        }
        else{
            
            #ifdef CUDA_MD5
            
            if(!slaveCracker->gpuOn){
                usleep(2000);
                continue;
            }

            vector<string> passwords = slaveCracker->rwBuf.consume();
            
            string md5 = slaveCracker->targetMd5;
        
            string pass;

            bool found =false;

            found = CudaMD5Cracker::crackMd5(passwords,md5,pass);
        
            if(found){
                
                cout<<endl<<"**********************************"<<endl;
                cout<<endl<<"***********Found by GPU***********"<<endl;
                cout<<endl<<"**********************************"<<endl;
                
                slaveCracker->reportResult(pass,md5);
            }
        
            #else
                usleep(2000);
            #endif
        }
        
    }

    return NULL;
}
