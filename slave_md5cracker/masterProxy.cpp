#include "masterProxy.h"
#include <iostream>

using namespace std;

MasterProxy::MasterProxy(){

}

void MasterProxy::handshake(SlaveMD5Cracker* slave){

    cout<<"Handshaking with master"<<endl;
}

void MasterProxy::stop(SlaveMD5Cracker* slave){

    cout<<"Stop cracking"<<endl;

}

void MasterProxy::fetch(SlaveMD5Cracker* slave){

    cout<<"Fetch new batch of passwords"<<endl;

}


