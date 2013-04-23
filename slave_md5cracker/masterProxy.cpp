#include "masterProxy.h"
#include "slaveMD5Cracker.h"
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <string>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <xmlrpc-c/girerr.hpp>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client.hpp>
#include <xmlrpc-c/client_transport.hpp>

/*
Packet stream XML-RPC does not use HTTP, as regular XML-RPC does. Packet stream XML-RPC has a concept of a long-lived client/server connection and you perform multiple RPCs over a single connection. That connection is typically a TCP connection. 
 * */

using namespace std;

MasterProxy::MasterProxy(){

}

void MasterProxy::handshake(SlaveMD5Cracker* slave){

    cout<<"Handshaking with master"<<endl;
    
    try {

        xmlrpc_c::clientXmlTransport_pstream myTransport(
            xmlrpc_c::clientXmlTransport_pstream::constrOpt()
            .fd(slave->toMasterSocket));

        xmlrpc_c::client_xml myClient(&myTransport);

        string const methodName("handshake");

        //The only parameter is listenPort 
        //This port is selected by system         
        int listenPort = slave->myListenPort;

        //parameters
        xmlrpc_c::paramList handShakeParms;
        handShakeParms.add(xmlrpc_c::value_int( listenPort ));

        xmlrpc_c::rpcPtr myRpcP(methodName, handShakeParms);

        xmlrpc_c::carriageParm_pstream myCarriageParm;
        // Empty; transport doesn't need any information

        myRpcP->call(&myClient, &myCarriageParm);

        assert(myRpcP->isFinished());

        const string ans(xmlrpc_c::value_string(myRpcP->getResult()));
            // Assume the method returned an integer; throws error if not

        cout << ans << endl;

    } catch (exception const& e) {
        cout << "Client threw error: " << e.what() << endl;
    } catch (...) {
        cout << "Client threw unexpected error." << endl;
    }

}

void MasterProxy::stop(SlaveMD5Cracker* slave){

    cout<<"Stop cracking"<<endl;

}

void MasterProxy::fetch(SlaveMD5Cracker* slave){

    cout<<"Fetch new batch of passwords"<<endl;

}


