#include "../configure.h"
#include "passGenerator.h"
#include <iostream>
using namespace std;

PassGenerator::PassGenerator(len_t startPosition, int chunkSize){

    this->start = startPosition;

    this->chunkSize = chunkSize;

    this->current = startPosition;

    init();
}

len_t PassGenerator::getStartPosition(){
    return this->start;
}

void PassGenerator::init(){

    //A-Z,a-z,0-9
    for(char c = 'A'; c <= 'Z'; c++)
        charArr.push_back(c);

    for(char c = 'a'; c <= 'z'; c++)
        charArr.push_back(c);

    for(char c = '0'; c <= '9'; c++)
        charArr.push_back(c);

    this->base = charArr.size();

    strArr.reserve(PASS_LEN);
    //init the strArr according to the start;
    
    update();

    //cout << "Current is ["<< getCurPassword() <<"]" <<endl;
}

//Be careful, that 'AA','AAA','AAA....' could not be generated below 
void PassGenerator::update(){

    strArr.clear();

    len_t num = current;

    unsigned int pos = 0;

    do{

        len_t remains = num / base;
        
        int digit = num % base;

        //increase the size of strArr
        if(strArr.size() == pos)
            strArr.push_back('A');

        strArr[pos++] = getChar(digit);

        num = remains;
    
    }while( num!=0 );
}

//increase by 1
len_t PassGenerator::inc(){

    current++;
    
    //increase the arr accordingly
    update();

    return current;
}

string PassGenerator::getCurPassword(){

    string str("");

    for(int i = strArr.size()-1; i>=0; i--){
        str.push_back(strArr[i]);
    }
    
    return str;
}

//postfix ++
len_t PassGenerator::operator++(int){
    return this->inc();
}


vector<string> PassGenerator::generateAll(){

    vector<string> allPass;

    for(int i=0;i<chunkSize;i++){

        allPass.push_back( getCurPassword() );

        (*this)++;
    }
    
    return allPass;
}

//A-Z,a-z,0-9
char PassGenerator::nextChar(int pos, bool& carry){

    if(pos==base-1){
        carry = true;
        return charArr[0];
    }

    carry = false;
    return charArr[pos+1];

}

//A-Z,a-z,0-9
char PassGenerator::nextChar(char c, bool& carry){

    carry = false;

    if( (c>='A'&&c<'Z') || (c>='a'&&c<'z') || (c>='0'&&c<'9') )
        return c+1;

    if( c=='Z' )
        return 'a';

    if( c=='z')
        return '0';

    carry = true;

    return 'A';
}

//digit is [0,base)
char PassGenerator::getChar(int digit){

    if(digit>=0 && digit <base)
        return charArr[digit];

    //indicating wrong
    return '#';
}


len_t PassGenerator::getMax(int len){

    len_t max = 0;

    int curLen = 1;

    len_t cur = 1 ;

    while( curLen <= len){

        cur = base * cur;

        max += cur;

        curLen++;
    }

    return max;
}

