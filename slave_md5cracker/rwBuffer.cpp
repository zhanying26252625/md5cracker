#include "unistd.h"
#include "stdlib.h"
#include "rwBuffer.h"
#include <iostream>

using namespace std;

RWBuffer::RWBuffer(){
    //init();
}

RWBuffer::~RWBuffer(){
    //destroy();
}

void RWBuffer::init(){

    pthread_mutex_init(&mutex_pgs,NULL);
    
    pthread_mutex_init(&mutex,NULL);

    sem_init(&sema_empty,0,QueueSize);

    sem_init(&sema_full,0,0);
}

void RWBuffer::destroy(){

    buf.clear();

    pgs.clear();

    pthread_mutex_destroy(&mutex_pgs);
    
    pthread_mutex_destroy(&mutex);

    sem_destroy(&sema_empty);

    sem_destroy(&sema_full);

}

void RWBuffer::produce(string& newPass){

    static int counter = 0;

    //This is not thread-safe,it's ok since only one producer
    static vector<string> tempBuf;

    tempBuf.push_back(newPass);
    
    //test only
    static bool foundGirl = false;
    if(!newPass.compare(string("girl"))){
        cout<<"Produce-----------GrilGirlGrilGrilGril" <<endl;
        foundGirl = true;
    }

    //cout<<"( "<<counter<<" ) New password : "<<newPass <<endl;
    counter++;

    if( tempBuf.size() >=  RWBuffer::BatchSize ){
       
        //problem here 
        if(foundGirl){
            cout<<"A new Batch into Queue"<<endl;
            foundGirl = false;        
        }

        produce(tempBuf);
        tempBuf.clear();
    }

    return;
}

void RWBuffer::produce( vector<string>& passwords){

    sem_wait(&sema_empty);

    pthread_mutex_lock(&mutex);

    buf.push_back(passwords);

    //test only. sometimes it does not contains 'girl' strange? multi-threading vs static member?
    
    for(unsigned int i =0;i<passwords.size();i++){
        string str = passwords[i];

        if(!str.compare(string("girl")))
            cout<<"Girl is int the buffer"<<endl;

    }
    //cout<<"Produce"<<endl;

    pthread_mutex_unlock(&mutex);

    sem_post(&sema_full);
}


vector<string> RWBuffer::consume(){

    sem_wait(&sema_full);

    pthread_mutex_lock(&mutex);

    vector<string> passwords = buf.front();

    buf.pop_front();

    //cout<<"Consume"<<endl;

    pthread_mutex_unlock(&mutex);

    sem_post(&sema_empty);

    return passwords;
}

void RWBuffer::clear(){ 

    pthread_mutex_lock(&mutex);
    buf.clear();
    pthread_mutex_unlock(&mutex);
}

void RWBuffer::run(){

    pthread_t thread;

    int rc = pthread_create(&thread,NULL,RWBuffer::generatorFunc, (void*)this );

    if( -1 == rc ){
        cout<<"Can't create pass generator thread"<<endl;
        exit(1);
    }

    generatorThread = thread;

}

void RWBuffer::end(){

    this->endGenerator = true;
    pthread_cancel(generatorThread);
    
    void* ret;
    pthread_join(generatorThread,&ret);

    cout << "generate thread terminated!"<<endl;
}

void RWBuffer::injectPG(PassGenerator& pg){
    pthread_mutex_lock(&mutex_pgs);
    pgs.push_back(pg);
    pthread_mutex_unlock(&mutex_pgs);
}

void* RWBuffer::generatorFunc(void* arg){

    RWBuffer* rwBuf = (RWBuffer*)arg;

    rwBuf->endGenerator = false;

    while(!rwBuf->endGenerator){

        pthread_mutex_lock(&rwBuf->mutex_pgs);

        int size = rwBuf->pgs.size();

        if(size == 0){

            pthread_mutex_unlock(&rwBuf->mutex_pgs);
        
            usleep(200);

        }else{

            PassGenerator pg = rwBuf->pgs.front();

            rwBuf->pgs.pop_front();
            
            pthread_mutex_unlock(&rwBuf->mutex_pgs);

            cout << "Start:"<< pg.getStartPosition()<<endl;
                
            for(string& pass : pg.generateAll()){

                if(rwBuf->endGenerator)
                    break;

                rwBuf->produce(pass);
            }
        }
    }

    return NULL;
}
