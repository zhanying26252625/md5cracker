#include "../configure.h"
#include "masterMD5Cracker.h"

#include <iostream>
#include <string>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/xml.hpp>
#include <xmlrpc-c/girerr.hpp>
#include <xmlrpc-c/timeout.hpp>

using namespace xmlrpc_c;
using namespace std;

int main(){

    cout<<"Master_md5cracker is starting......"<<endl;

    MasterMD5Cracker master;

/*
    paramList pList;
    pList.add(value_int(5));
    pList.add(value_int(6));
    string addXml; 

    xml::generateCall("sample.add", pList, &addXml);

    cout<<"XML is :"<<endl<<addXml<<endl;

    string methodName;
    paramList recPList;

    xml::parseCall(addXml, &methodName, &recPList);

    cout<<"Method is "<<methodName<<" with "<<endl;

    for(int i=0;i<recPList.size();i++)
        cout<<i<<":"<<recPList.getInt(i)<<endl;
*/
    return 1;
}
