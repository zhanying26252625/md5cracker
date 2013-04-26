#include "logManager.h"
#include <iostream>
#include <fstream>

using namespace std;

//Declaration and Definition should be in header since it's template in C++,This problem may be solved by c++11

/*
template<class T>
T LogManager::operator+(const T a){

    return a;
}


LogManager& LogManager::operator<<(const string& s){
    cout<<s;
}

template<typename T>
LogManager& LogManager::operator<<(const T& t){
    #ifdef COUT
    cout<<t;
    #endif
    return (*this);
}
*/

LogManager::LogManager(const string& logName){

    logFile.open(logName.c_str(), ios::out| ios::trunc);

    if( !logFile.is_open() ){
        cout<<"Can't open log file : "<<logName;
        isLogFileOpen = false;
    }

    isLogFileOpen = true;
}
