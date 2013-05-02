#ifndef RW_BUFFER_H
#define RW_BUFFER_H

#include "passGenerator.h"

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

    //init resource
    void init();

    void destroy();
   
    static void* generatorFunc(void* arg);

    //run a dedicated thread to produce passwords
    void run();
    //kill that thread
    void end();

    void injectPG(PassGenerator& pg);

private:
    
    sem_t sema_empty;
    
    sem_t sema_full;

    pthread_mutex_t mutex;

    deque< vector<string> > buf;

    pthread_mutex_t mutex_pgs;

    //generate password and inject into reader writer queue
    pthread_t generatorThread;

    deque< PassGenerator> pgs;

    bool endGenerator;

    //a smart way to define constant
    enum Size{BatchSize=1024*8,QueueSize=200};
};

#endif
