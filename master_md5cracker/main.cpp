#include "../configure.h"
#include "masterMD5Cracker.h"
#include "passGenerator.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

//single mode and distributed mode

void testPassGenerator(){

    cout << "testPassGenerator" <<endl;

    PassGenerator pg(0,62*62+3);

    int count = 0;

    //c++11 syntax sugar
    for(string& str : pg.generateAll())
        cout << count++<<" : "<< str << endl;

    return ;
}

int main(int argc, char* argv[]){

    MasterMD5Cracker master;
    
    if(argc < 2){
        cout <<"./master_md5cracker single | distributed " <<endl;
        return 1;
    }
    else if(! string(argv[1]).compare(string("test"))){
        testPassGenerator();
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
