#include "cudaCracker.h"

using namespace std;

//these two functions are in cudaCracker.cuda

extern double execute_kernel(int blocks_x, int blocks_y, int threads_per_block, int shared_mem_required, int realthreads, uint *gpuWords, uint *gpuHashes, bool search = false);

extern void init_constants(uint *target = NULL);

void CudaMD5Cracker::deviceQuery(){

	int deviceCount;
	cudaGetDeviceCount(&deviceCount);
	if (deviceCount == 0)
		printf("There is no device supporting CUDA\n");
	int dev;
	for (dev = 0; dev < deviceCount; ++dev) {
		cudaDeviceProp deviceProp;
		cudaGetDeviceProperties(&deviceProp, dev);
		if (dev == 0) {
			if (deviceProp.major < 1)
				printf("There is no device supporting CUDA.\n");
			else if (deviceCount == 1)
				printf("There is 1 device supporting CUDA\n");
			else
				printf("There are %d devices supporting CUDA\n", deviceCount);
		}
		printf("\nDevice %d: \"%s\"\n", dev, deviceProp.name);
		printf("  Major revision number:                         %d\n",
			   deviceProp.major);
		printf("  Minor revision number:                         %d\n",
			   deviceProp.minor);
		printf("  Total amount of global memory:                 %d bytes\n",
			   (int)deviceProp.totalGlobalMem);
		printf("  Total amount of constant memory:               %d bytes\n",
			   (int)deviceProp.totalConstMem); 
		printf("  Total amount of shared memory per block:       %d bytes\n",
			   (int)deviceProp.sharedMemPerBlock);
		printf("  Total number of registers available per block: %d\n",
			   deviceProp.regsPerBlock);
		printf("  Warp size:                                     %d\n",
			   deviceProp.warpSize);
		printf("  Maximum number of threads per block:           %d\n",
			   deviceProp.maxThreadsPerBlock);
		printf("  Maximum sizes of each dimension of a block:    %d x %d x %d\n",
			   deviceProp.maxThreadsDim[0],
			   deviceProp.maxThreadsDim[1],
			   deviceProp.maxThreadsDim[2]);
		printf("  Maximum sizes of each dimension of a grid:     %d x %d x %d\n",
			   deviceProp.maxGridSize[0],
			   deviceProp.maxGridSize[1],
			   deviceProp.maxGridSize[2]);
		printf("  Maximum memory pitch:                          %d bytes\n",
			   (int)deviceProp.memPitch);
		printf("  Texture alignment:                             %d bytes\n",
			   (int)deviceProp.textureAlignment);
		printf("  Clock rate:                                    %d kilohertz\n",
			   deviceProp.clockRate);
	}
}

// Find the dimensions (bx,by) of a 2D grid of blocks that 
// has as close to nblocks blocks as possible
//
void CudaMD5Cracker::find_best_factorization(int &bx, int &by, int nblocks)
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



bool CudaMD5Cracker::calculate_grid_parameters(int gridDim[3], int threadsPerBlock, int neededthreads, int dynShmemPerThread, int staticShmemPerBlock)
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
void CudaMD5Cracker::md5_prep_array(std::valarray<char> &paddedWords, const std::vector<std::string> &words)
{
	paddedWords.resize(64*words.size());
	paddedWords = 0;

	for(unsigned int i=0; i != words.size(); i++)
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
int CudaMD5Cracker::cuda_compute_md5s(std::vector<md5hash> &hashes, const std::vector<std::string> &ptext, uint *target )
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
	
        //double rate = 1000 * ptext.size() / gpuTime;

	    //std::cout << "GPU MD5 time : " <<  gpuTime << " ms (" << std::fixed << rate << " hash/second)\n";

	} 

	// Download the results
	int ret[4];
	
    ( cudaMemcpy(ret, gpuHashes, sizeof(int)*4, cudaMemcpyDeviceToHost) );
	
	// free resource
	( cudaFree(gpuWords) );

    ( cudaFree(gpuHashes) );

    //if ret[3] is 1 then ret[0] has index
	return ret[3] ? ret[0] : -1;
}



void CudaMD5Cracker::printMD5(md5hash& hash){

    uint* h = hash.ui;

    for(int i = 0; i != 16; i++) { 
        printf( "%02x", (uint)(((unsigned char *)h)[i])); 
    }

    printf("\n");

}

// prepare a 56-byte (maximum) wide md5 message by appending the 64-bit length
// we assume c0 is zero-padded
void CudaMD5Cracker::md5_prep(char *c0)
{
	uint len = 0;
	char *c = c0;
	while(*c) {len++; c++;}
	c[0] = 0x80;			// bit 1 after the message
	((uint*)c0)[14] = len * 8;	// message length in bits
}


string CudaMD5Cracker::md5hash2string(md5hash& md5){
    
    uint* hash = (uint*)md5.ui;

    string retStr;
    for(int i = 0; i != 16; i++) { 
        char buf[128] = {0};
        sprintf(buf, "%02x", (uint)(((unsigned char *)hash)[i])); 
        retStr += string(buf);
    }

    return retStr;
}

int CudaMD5Cracker::convert(char c){
    if( c>='0' && c<='9')
        return c-'0';
    else if(c>='a'&&c<='f')
        return c-'a'+10;
    else
        return 0;
}

void CudaMD5Cracker::string2md5hash(string& str, md5hash& hash){

    const char* buf = str.c_str();

    for(int i=0,j=0;i<32;i+=2,j++){
        char f = convert( buf[i] );
        char s = convert(buf[i+1] );
        hash.ch[j] = f*16+s;
    }
}

bool CudaMD5Cracker::crackMd5(vector<string> passwords,string& md5,string& ret){

	std::vector<md5hash> hashes_gpu;

	md5hash target; 
        
    string2md5hash(md5,target);

	int index = cuda_compute_md5s(hashes_gpu, passwords, target.ui);

	if(index >= 0)
	{
        ret = passwords[index];
	    return true;
    }
	else
	{
        ret = string("");
        return false;
	}
}






