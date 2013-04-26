#include "gpuCracker.h"
#include "slaveMD5Cracker.h"

#include <unistd.h>
#include <stdlib.h>

GpuCracker::GpuCracker(){

}

bool GpuCracker::init(SlaveMD5Cracker* sc){

    slaveCracker = sc;

    bool flag = isGPUsupported();

    if(!flag){
        cout <<"No GPU available"<<endl;
        return false;
    }

    pthread_t thread;

    int rc = pthread_create(&thread,NULL,workThreadFunc,this);

    if( -1 == rc ){
        cout<<"Can't creat gpu cracking thread"<<endl;
        return false;
    }

    this->thread = thread;

    return true;
}

bool GpuCracker::isGPUsupported(){

    return true;
}

bool GpuCracker::terminate(){

    pthread_cancel(thread);

    void* ret;

    pthread_join(thread,&ret);

    cout <<"GPU thread terminated!" <<endl;

    return true;
}


void* GpuCracker::workThreadFunc(void* arg){

    cout<<"Gpu Cracker::workThreadFunc"<<endl;

    GpuCracker* gpuCracker = (GpuCracker*)arg;

    SlaveMD5Cracker* slaveCracker = gpuCracker->slaveCracker; 

    while(1){

        if(!slaveCracker->isCracking){
            usleep(4000);
            cout<<"GPU"<<endl;
        }
        else{
            //vector<string> passwords = slaveCracker->rwBuf.consume();
            string md5 = slaveCracker->targetMd5;
        }
        
    }

    return NULL;
}
