#ifndef _GPU_CRACKER_H
#define _GPU_CRACKER_H

#include <pthread.h>

class SlaveMD5Cracker;

class GpuCracker{

private:

    SlaveMD5Cracker* slaveCracker;

    static void* workThreadFunc(void* arg);

    pthread_t thread;

    bool isGPUsupported();

public:

    GpuCracker();

    bool init(SlaveMD5Cracker* sc);

    bool terminate();

};

#endif
