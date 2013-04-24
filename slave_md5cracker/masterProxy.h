#ifndef _MASTER_PROXY_H
#define _MASTER_PROXY_H

class SlaveMD5Cracker;

//This class is responsible for sending specific cmd to master
class MasterProxy{

private:

public:
    //Invite master to connect back to me  
    static void handshake(SlaveMD5Cracker* slave);

    static void stop(SlaveMD5Cracker* slave);
    //Feedback for previous bundle of passwords
    static void feedback(SlaveMD5Cracker* slave);

    MasterProxy();
    
    friend class SlaveMD5Cracker;
};

#endif
