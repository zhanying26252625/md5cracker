#ifndef _MASTER_PROXY_H
#define _MASTER_PROXY_H

class SlaveMD5Cracker;

class MasterProxy{

private:

public:
  
    static void handshake(SlaveMD5Cracker* slave);

    static void stop(SlaveMD5Cracker* slave);

    static void fetch(SlaveMD5Cracker* slave);

    MasterProxy();
    
    friend class SlaveMD5Cracker;
};

#endif
