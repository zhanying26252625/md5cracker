#include "rwBuffer.h"
#include <iostream>

using namespace std;

RWBuffer::RWBuffer(){

    pthread_mutex_init(&mutex,NULL);

    sem_init(&sema_empty,0,QueueSize);

    sem_init(&sema_full,0,0);
}

RWBuffer::~RWBuffer(){

    pthread_mutex_destroy(&mutex);

    sem_destroy(&sema_empty);

    sem_destroy(&sema_full);
}

void RWBuffer::produce(string& newPass){

    static int counter = 0;

    static vector<string> tempBuf;

    tempBuf.push_back(newPass);

    //cout<<"( "<<counter<<" ) New password : "<<newPass <<endl;

    counter++;

    if( tempBuf.size() ==  RWBuffer::BatchSize || ! newPass.compare("$") ){

        //cout<<"size is "<<tempBuf.size()<<" BatchSize is "<<BatchSize<<endl;

        produce(tempBuf);

        tempBuf.clear();
    }

    return;
}

void RWBuffer::produce( vector<string>& passwords){

    sem_wait(&sema_empty);

    pthread_mutex_lock(&mutex);

    buf.push_back(passwords);

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



