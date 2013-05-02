# flags
CUDA_INSTALL_PATH = /usr/local/cuda

INCLUDES          = -I. -I$(CUDA_INSTALL_PATH)/include -I$(HOME)/NVIDIA_CUDA_SDK/common/inc -I$(CUDA_INSTALL_PATH)/samples/common/inc

LIBS              = -L$(CUDA_INSTALL_PATH)/lib -L$(HOME)/NVIDIA_CUDA_SDK/lib

CXXFLAGS          = -O3

CUDA_LIB_PATH	  = $(CUDA_INSTALL_PATH)/lib64

LDFLAGS           = -lrt -lm -L$(CUDA_LIB_PATH) -lcudart 

NVCC              = nvcc

# files
CPP_SOURCES       := cuda_md5.cpp deviceQuery.cpp cuda_md5_cpu.cpp #cpu md5 
CU_SOURCES        := cuda_md5_gpu.cu #gpu md5
HEADERS           := $(wildcard *.h)
CPP_OBJS          := $(patsubst %.cpp, %.o, $(CPP_SOURCES))
CU_OBJS           := $(patsubst %.cu, %.cu_o, $(CU_SOURCES))

%.cu_o : %.cu
	$(NVCC) -c $(INCLUDES) -o $@ $<

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) -o $@ $<

app: cuda_md5cracker cleanObj

cuda_md5cracker: $(CPP_OBJS) $(CU_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(INCLUDES) $(LIBS) 

cleanObj:
	rm -f *.o *.cu_o

run: app
	cat wordlist | ./cuda_md5cracker finds 

clean:
	rm -f cuda_md5cracker *.o *.cu_o tags
