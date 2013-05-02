#ifndef _CUDA_MD5_H
#define _CUDA_MD5_H

#include "md5.h"
#include <string>
#include <iostream>
#include <vector>
#include <valarray>
#include <stdio.h>

#include <cuda_runtime_api.h>
#include <helper_cuda_drvapi.h>
#include <helper_functions.h>
#include <helper_string.h>
#include <helper_timer.h>

using namespace std;

typedef unsigned int uint;

union md5hash
{
	    uint ui[4];
	    char ch[16];
};

class CudaMD5Cracker{

private:

    //pad as a vector of 64bytes string
    static void md5_prep_array(std::valarray<char> &paddedWords, const std::vector<std::string> &words);
    
    static void md5_prep(char *c0);

    static string md5hash2string(md5hash& md5);
    
    static int convert(char c);
    
    static void string2md5hash(string& str, md5hash& hash);

    static void printMD5(md5hash& hash);

    static void find_best_factorization(int &bx, int &by, int nblocks);
    
    static bool calculate_grid_parameters(int gridDim[3], int threadsPerBlock, int neededthreads, int dynShmemPerThread, int staticShmemPerBlock);

    static int cuda_compute_md5s(std::vector<md5hash> &hashes, const std::vector<std::string> &ptext, uint *target = NULL);

public:
    
    static void deviceQuery();
    
    static bool crackMd5(vector<string> passwords,string& md5,string& ret);

};

#endif
