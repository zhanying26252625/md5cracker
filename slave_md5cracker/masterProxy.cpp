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

bool MasterProxy::sendViaXMLRPC(int socket, string name,  string strVal, len_t longVal, int intVal){
    
    bool ok = true;

    try {

        xmlrpc_c::clientXmlTransport_pstream myTransport(
            xmlrpc_c::clientXmlTransport_pstream::constrOpt().fd( socket ));

        xmlrpc_c::client_xml myClient(&myTransport);

        string const methodName(name);

        cout <<"Proxy is sending ["<<name<<"] to master" <<endl;

        //parameters
        xmlrpc_c::paramList Parms;

        Parms.add(xmlrpc_c::value_string( strVal ));
        Parms.add(xmlrpc_c::value_i8( longVal ));
        Parms.add(xmlrpc_c::value_int( intVal ));

        xmlrpc_c::rpcPtr myRpcP(methodName, Parms);

        xmlrpc_c::carriageParm_pstream myCarriageParm;
        // Empty; transport doesn't need any information

        myRpcP->call(&myClient, &myCarriageParm);

        assert(myRpcP->isFinished());

        const string ans(xmlrpc_c::value_string(myRpcP->getResult()));
            // Assume the method returned an integer; throws error if not

        cout << ans << endl;

    } catch (exception const& e) {
        cout << "masterProxy threw error: " << e.what() << endl;
        ok = false;
    } catch (...) {
        //socket maybe broken, slave maybe lost
        cout << "masterProxy threw unexpected error." << endl;
        ok = false;
    }

    return ok;
}

