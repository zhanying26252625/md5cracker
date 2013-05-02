#include "../configure.h"
#include "slaveMD5Cracker.h"
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]){

    bool gpuOn = false;
    
    // slave_md5cracker_gpu gpu
    if(argc>=2){
        gpuOn = true;
    }

    SlaveMD5Cracker slave;

    slave.run(gpuOn);

    return 1;
}
