#ifndef _LOG_MGR_H
#define _LOG_MGR_H

#include <iostream>
#include <string>
#include <fstream>

#define COUT 1
#define FILELOG 2

using namespace std;
//Apache log4cxx is a good log library

//Declaration and Definition should be in header since it's template in C++

//Singleton pattern
class LogManager{

private:
    ofstream logFile;

    bool isLogFileOpen;

private:
    LogManager(){}

    LogManager(const string& logName);

    ~LogManager(){}

    LogManager(const LogManager& logMgr){}

    LogManager& operator=(const LogManager& logMgr){return *this ;}

public:
    
    static LogManager& getInstance(){
        static LogManager logMgr("log.txt");
        return logMgr;
    }
    //Support common data type
    template<typename T>
    LogManager& operator<<(const T& t){
        #ifdef COUT
        cout<<t;
        #endif

        #ifdef FILELOG
        logFile<<t;
        #endif

        return (*this);
    }
    //Support std::endl, which is a function template itself
    //endl would insert newline and flush 
    LogManager& operator<<(std::ostream& (*func) (std::ostream&) ){
        #ifdef COUT
        cout<<func;
        #endif
        
        #ifdef FILELOG
        logFile<<"\n";
        logFile.flush();
        #endif
        return (*this);
    }
};


#endif
