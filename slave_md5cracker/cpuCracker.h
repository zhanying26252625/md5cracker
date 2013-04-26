#ifndef _CPU_CRACKER_H
#define _CPU_CRACKER_H

#include <iostream>
#include <vector>
#include <pthread.h>

using namespace std;

class SlaveMD5Cracker;

class CpuCracker{

private:

    int getNumOfCores();

    int cores;

    vector<pthread_t> workThreads;

    static void* workThreadFunc(void* arg);

    SlaveMD5Cracker* slaveCracker;

public:

    CpuCracker();

    bool init(SlaveMD5Cracker* sc);

    bool terminate();
};

#endif
