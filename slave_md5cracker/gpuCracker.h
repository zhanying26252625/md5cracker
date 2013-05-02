#ifndef _GPU_CRACKER_H
#define _GPU_CRACKER_H

#include <pthread.h>
#include <vector>
using namespace std;
class SlaveMD5Cracker;

class CpuCracker;

class GpuCracker{

private:

    enum{NUM_THREADS=4};

    SlaveMD5Cracker* slaveCracker;

    static void* workThreadFunc(void* arg);

    vector<pthread_t> workThreads;

    bool isGPUsupported();

    friend class CpuCracker;

    
public:

    GpuCracker();

    bool init(SlaveMD5Cracker* sc);

    bool terminate();

};

#endif
