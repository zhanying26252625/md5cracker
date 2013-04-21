#include "../configure.h"
#include "masterMD5Cracker.h"

#include <iostream>
#include <string>

using namespace std;

//single mode and distributed mode

int main(int argc, char* argv[]){

    MasterMD5Cracker master;
    
    if(argc < 2){
        cout <<"./master_md5cracker single | distributed " <<endl;
        return 1;
    }
    else if( ! string(argv[1]).compare(string("single")) ){
        master.runLocal();
    }
    else{
        master.runDistribute();
    }

    double timeSpent = master.getTimeSpent();

    cout << "Time spent : "<<timeSpent << endl;

    return 1;
}
