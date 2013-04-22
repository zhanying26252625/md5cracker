#ifndef _PASS_GENERATOR_H
#define _PASS_GENERATOR_H

#include <iostream>
#include <vector>
#include <string>

using namespace std;

typedef unsigned long long len_t;

//Generator a range of passwords [start , start+chunkSize-1]
class PassGenerator{

private:
    vector<char> strArr;

    vector<char> charArr;

    int base;

    len_t start;

    int chunkSize;

    len_t current;

    void init();

    void update();
    //increase by 1
    len_t inc();
    
    char nextChar(char c, bool& carry);

    char nextChar(int pos, bool& carry);

    char getChar(int digit);

public:
    
    PassGenerator(len_t startPosition, int chunkSize);

    vector<string> generateAll();
    
    string getCurPassword();

    //postfix ++ 
    len_t operator++(int);

};

#endif
