#include <string.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <valarray>
#include <string>

#include <cuda_runtime_api.h>
#include <helper_cuda_drvapi.h>
#include <helper_functions.h>
#include <helper_string.h>
#include <helper_timer.h>

using namespace std;

union md5hash
{
	uint ui[4];
	char ch[16];
};

#define MD5_CPU				md5_cpu_v2
int niters = 10;

// Some declarations that should wind up in their own .h file at some point
void print_md5(uint *hash);
void md5_prep(char *c0);
double execute_kernel(int blocks_x, int blocks_y, int threads_per_block, int shared_mem_required, int realthreads, uint *gpuWords, uint *gpuHashes, bool search = false);
void init_constants(uint *target = NULL);
void md5_cpu(uint w[16], uint &a, uint &b, uint &c, uint &d);
void md5_cpu_v2(const uint *in, uint &a, uint &b, uint &c, uint &d);
int deviceQuery();

extern string md5hash2string(md5hash& hash);

///////////////////////////////////////////////////////////
// CUDA helpers

//
// Find the dimensions (bx,by) of a 2D grid of blocks that 
// has as close to nblocks blocks as possible
//
void find_best_factorization(int &bx, int &by, int nblocks)
{
	bx = -1;
	int best_r = 100000;
	for(int bytmp = 1; bytmp != 65536; bytmp++)
	{
		int r  = nblocks % bytmp;
		if(r < best_r && nblocks / bytmp < 65535)
		{
			by = bytmp;
			bx = nblocks / bytmp;
			best_r = r;
			
			if(r == 0) { break; }
			bx++;
		}
	}
	if(bx == -1) { std::cerr << "Unfactorizable?!\n"; exit(-1); }
}

//
// Given a total number of threads, their memory requirements, and the
// number of threadsPerBlock, compute the optimal allowable grid dimensions.
// Returns false if the requested number of threads are impossible to fit to
// shared memory.
//
bool calculate_grid_parameters(int gridDim[3], int threadsPerBlock, int neededthreads, int dynShmemPerThread, int staticShmemPerBlock)
{
	const int shmemPerMP =  16384;

	int dyn_shared_mem_required = dynShmemPerThread*threadsPerBlock;
	int shared_mem_required = staticShmemPerBlock + dyn_shared_mem_required;
	if(shared_mem_required > shmemPerMP) { return false; }

	// calculate the total number of threads
	int nthreads = neededthreads;
	int over = neededthreads % threadsPerBlock;
	if(over) { nthreads += threadsPerBlock - over; } // round up to multiple of threadsPerBlock

	// calculate the number of blocks
	int nblocks = nthreads / threadsPerBlock;
	if(nthreads % threadsPerBlock) { nblocks++; }

	// calculate block dimensions so that there are as close to nblocks blocks as possible
	find_best_factorization(gridDim[0], gridDim[1], nblocks);
	gridDim[2] = 1;

	return true;
}


//
// Convert an array of null-terminated strings to an array of 64-byte words
// with proper MD5 padding
//
void md5_prep_array(std::valarray<char> &paddedWords, const std::vector<std::string> &words)
{
	paddedWords.resize(64*words.size());
	paddedWords = 0;

	for(int i=0; i != words.size(); i++)
	{
		char *w = &paddedWords[i*64];
		strncpy(w, words[i].c_str(), 56);
		md5_prep(w);
	}
}


//
// GPU calculation: given a vector ptext of plain text words, compute and
// return their MD5 hashes
//
int cuda_compute_md5s(std::vector<md5hash> &hashes, const std::vector<std::string> &ptext, uint *target = NULL, bool benchmark = false)
{

	// load the MD5 constant arrays into GPU constant memory
	init_constants(target);

	// pad dictionary words to 64 bytes (MD5 block size)
	std::valarray<char> paddedWords;
	md5_prep_array(paddedWords, ptext);

	// Upload the dictionary onto the GPU device
	uint *gpuWords, *gpuHashes = NULL;
	( cudaMalloc((void **)&gpuWords, paddedWords.size()) );
	( cudaMemcpy(gpuWords, &paddedWords[0], paddedWords.size(), cudaMemcpyHostToDevice) );

	// allocate GPU memory for match signal, instead of the actual hashes
	( cudaMalloc((void **)&gpuHashes, 4*sizeof(uint)) );
	uint tmp[4] = {0}; // initialize to zero
	( cudaMemcpy(gpuHashes, tmp, 4*sizeof(uint), cudaMemcpyHostToDevice) );

	int dynShmemPerThread = 64;	// built in the algorithm
	int staticShmemPerBlock = 32;	// read from .cubin file

	double bestTime = 1e10, bestRate = 0.;
	int bestThreadsPerBlock;
	int nthreads = ptext.size();
    int tpb = 64;	// tpb is number of threads per block

    //Call GPU
	{
		int gridDim[3];
		
        if(!calculate_grid_parameters(gridDim, tpb, nthreads, dynShmemPerThread, staticShmemPerBlock)) { 
            cout << "The configuration is not proper for this device"<<endl;
            return -1;
        }

		double gpuTime = 0.0;
			
        gpuTime += execute_kernel(gridDim[0], gridDim[1], tpb, tpb*dynShmemPerThread, ptext.size(), gpuWords, gpuHashes, target != NULL);
	
        double rate = 1000 * ptext.size() / gpuTime;

		bestTime = gpuTime;
		bestRate = rate;
		bestThreadsPerBlock = tpb;
	} 
	
	std::cout << "GPU MD5 time : " <<  bestTime << " ms (" << std::fixed << 1000. * ptext.size() / bestTime << " hash/second)\n";

	// Download the results
	uint ret[4];
	
    ( cudaMemcpy(ret, gpuHashes, sizeof(uint)*4, cudaMemcpyDeviceToHost) );
	
	// free resource
	( cudaFree(gpuWords) );

    ( cudaFree(gpuHashes) );

    printf("%d\n",ret);

	return ret[3] ? ret[0] : -1;
}

md5hash single_md5(std::string &ptext)
{
	md5hash h;
	char w[64] = {0};
	strncpy(w, ptext.c_str(), 56);
	md5_prep(w);
	MD5_CPU((uint*)&w[0], h.ui[0], h.ui[1], h.ui[2], h.ui[3]);

	return h;
}

extern void string2md5hash(string& str , md5hash& hash);

int main(int argc, char **argv)
{

    if(argc<2){
        std::cout<<"cuda_md5 word"<<endl;
        return 1;
    }

    std::string target_word(argv[1]);

	bool  benchmark = false;

	deviceQuery();

	// Load plaintext dictionary
	std::vector<std::string> ptext;
	std::cerr << "Loading words from stdin ...\n";
	std::string word;

	while(std::cin >> word)
	{
		ptext.push_back(word);
	}

	std::cerr << "Loaded " << ptext.size() << " words.\n\n";

	// Do search/calculation
	std::vector<md5hash> hashes_gpu;

	if(!target_word.empty())
	{
		// search for a given target word
		md5hash target = single_md5(target_word);

        string md5 = md5hash2string(target);
        
        md5hash hash;
        
        string2md5hash(md5,hash);

        cout << md5 << endl;

        print_md5(hash.ui);
        
        //print_md5(target.ui);

		int match = cuda_compute_md5s(hashes_gpu, ptext, target.ui, benchmark);

		if(match >= 0)
		{
			std::cerr << "Found match: index=" << match << " " << " word='" << ptext[match] << "'\n";
		}
		else
		{
			std::cerr << "Match not found.\n";
		}
	}

	return 0;
}
