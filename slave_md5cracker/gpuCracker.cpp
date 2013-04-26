#include "gpuCracker.h"
#include "slaveMD5Cracker.h"

GpuCracker::GpuCracker(){

}

bool GpuCracker::init(SlaveMD5Cracker* sc){

    slaveCracker = sc;

    cout <<"GpuCracker init, no GPU available"<<endl;

    return true;
}

void GpuCracker::terminate(){

}
