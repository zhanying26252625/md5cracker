#ifndef _MASTER_H
#define _MASTER_H

#include <string>

class MasterMD5Cracker{

public:
    MasterMD5Cracker();

    virtual ~MasterMD5Cracker();

    void run();

    bool createListeningThread();
    
private:
    //Background thread accepting slave's connection
    static void* listeningThread(void* arg);

    bool isExisting;
};

#endif
