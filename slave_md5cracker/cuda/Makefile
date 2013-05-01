# Makefile
#
# author      : sam adams, modified by Mario Juric <mjuric@ias.edu>
# version     : 0.2
# date        : 20080212
# discription : generic Makefile for making cuda programs
#
BIN               := cuda_md5

# flags
CUDA_INSTALL_PATH := /usr/local/cuda
INCLUDES          += -I. -I$(CUDA_INSTALL_PATH)/include -I$(HOME)/NVIDIA_CUDA_SDK/common/inc -I$(CUDA_INSTALL_PATH)/samples/common/inc
LIBS              := -L$(CUDA_INSTALL_PATH)/lib -L$(HOME)/NVIDIA_CUDA_SDK/lib
CXXFLAGS          := -O3
CUDA_LIB_PATH = $(CUDA_INSTALL_PATH)/lib64
#LDFLAGS           := -lrt -lm -lcudart -lcutil
LDFLAGS           := -lrt -lm -L$(CUDA_LIB_PATH) -lcudart 
# compilers
#NVCC              := nvcc --device-emulation
NVCC              := nvcc

# files
CPP_SOURCES       := cuda_md5.cpp cuda_md5_cpu.cpp deviceQuery.cpp util.cpp
CU_SOURCES        := cuda_md5_gpu.cu
HEADERS           := $(wildcard *.h)
CPP_OBJS          := $(patsubst %.cpp, %.o, $(CPP_SOURCES))
CU_OBJS           := $(patsubst %.cu, %.cu_o, $(CU_SOURCES))

%.cu_o : %.cu
	$(NVCC) -c $(INCLUDES) -o $@ $<

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) -o $@ $<

$(BIN): $(CPP_OBJS) $(CU_OBJS)
	$(CXX) -o $(BIN) $(CU_OBJS) $(CPP_OBJS) $(LDFLAGS) $(INCLUDES) $(LIBS)

util.o: util.cpp util.h
cuda_md5.o: cuda_md5.cpp util.h

clean:
	rm -f $(BIN) *.o *.cu_o
