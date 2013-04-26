#ifndef _MASTER_PROXY_H
#define _MASTER_PROXY_H

#include "passGenerator.h"

class SlaveMD5Cracker;

struct Cmd{
    //handshake port
    Cmd(string n,int i):name(n),intVal(i){longVal = 0; strVal=string("");}
    //feedback start(len_t)
    Cmd(string n,len_t l):name(n),longVal(l){intVal = 0; strVal=string("");}
    //returnret pass
    Cmd(string n,string s):name(n),strVal(s){ longVal = intVal = 0;}
    string name;
    string strVal;
    len_t longVal;
    int intVal;
};


//This class is responsible for sending specific cmd to master
class MasterProxy{

private:

public:

    static bool sendViaXMLRPC(int socket, string name,  string strVal, len_t longVal, int intVal);

    MasterProxy();
    
    friend class SlaveMD5Cracker;
};

#endif
