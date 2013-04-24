#ifndef RW_BUFFER_H
#define RW_BUFFER_H

#include <deque>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <string>

using namespace std;

//Reader writer buffer
class RWBuffer{

public:

    RWBuffer();

    ~RWBuffer();

    void produce(string& newPass);

    void produce( vector<string>& passwords);

    vector<string> consume();

    int getBufSize(){ return buf.size(); }

    void clear();
private:
    
    sem_t sema_empty;
    
    sem_t sema_full;

    pthread_mutex_t mutex;

    deque< vector<string> > buf;

    //a smart way to define constant
    enum Size{BatchSize=128,QueueSize=1024};
};

#endif
