#ifndef _GPU_CRACKER_H
#define _GPU_CRACKER_H

class SlaveMD5Cracker;

class GpuCracker{

private:

    SlaveMD5Cracker* slaveCracker;

public:

    GpuCracker();

    bool init(SlaveMD5Cracker* sc);
};

#endif
