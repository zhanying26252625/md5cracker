# -static prevents any dynamic linking from being used
# run ldconfig to refresh the ld cache after manually build and install xmlrpc library

# How to mix static library and dynamic libray
# gcc <options> <sources> -o <binary> -Wl,-Bstatic <list of static libs> -Wl,Bdynamic <list of 
# dynamic libs>
# gcc program.o -llib1 -Wl,-Bstatic -llib2 -Wl,-Bdynamic -llib3

CC = g++

# sometimes -g would not work for showing symbols in gdb, we use -gstabs+ instead
# maybe it's something related to debug info format
CFLAGS = -std=c++0x -Wall -g  

headers = ../configure.h slaveMD5Cracker.h masterProxy.h md5.h passGenerator.h cpuCracker.h gpuCracker.h rwBuffer.h 

objects = main.o slaveMD5Cracker.o  masterProxy.o md5.o passGenerator.o cpuCracker.o gpuCracker.o rwBuffer.o

xmlrpclibs = -L/usr/local/lib -lxmlrpc_client++ -lxmlrpc_client -lxmlrpc_server_pstream++ -lxmlrpc_server++ -lxmlrpc_server  -lxmlrpc++ -lxmlrpc -lxmlrpc_util -lxmlrpc_xmlparse -lxmlrpc_xmltok -lxmlrpc_packetsocket #-lwwwxml -lxmltok -lxmlparse -lwwwinit -lwwwapp -lwwwhtml -lwwwtelnet -lwwwnews -lwwwhttp -lwwwmime -lwwwgopher -lwwwftp -lwwwfile -lwwwdir -lwwwcache -lwwwstream -lwwwmux -lwwwtrans -lwwwcore -lwwwutils -lmd5 #-ldl 

#libdl.so is in /usr/lib, not in /usr/local/lib, and there is no libld.a 
xmlwwwlibs = -lwwwxml -lxmltok -lxmlparse -lwwwinit -lwwwapp -lwwwhtml -lwwwtelnet -lwwwnews -lwwwhttp -lwwwmime -lwwwgopher -lwwwftp -lwwwfile -lwwwdir -lwwwcache -lwwwstream -lwwwmux -lwwwtrans -lwwwcore -lwwwutils -L/usr/lib -lmd5 

target: slave_md5cracker

slave_md5cracker: $(objects) 
	$(CC) $(CFLAGS) -o $@ $^ -Wl,-Bstatic $(xmlrpclibs) -Wl,-Bdynamic -lpthread -lstdc++ $(xmlwwwlibs) ; rm *.o 

#Be care of the suffix
%.o: %.cpp $(headers)
	$(CC) -c $(INCLUDES) -o $@ $< $(CFLAGS) ; printf "\n"

##############################
# run "sudo lspci | grep VGA" to check availability of GPU
# In addition, CUDA SDK should be already installed

CUDA_INSTALL_PATH = /usr/local/cuda

INCLUDES          = -I. -I$(CUDA_INSTALL_PATH)/include -I$(HOME)/NVIDIA_CUDA_SDK/common/inc -I$(CUDA_INSTALL_PATH)/samples/common/inc

LIBS              = -L$(CUDA_INSTALL_PATH)/lib -L$(HOME)/NVIDIA_CUDA_SDK/lib

CUDA_LIB_PATH	  = $(CUDA_INSTALL_PATH)/lib64

LDFLAGS           = -lrt -lm -L$(CUDA_LIB_PATH) -lcudart 

NVCC              = nvcc

cuda_objs         = cudaCracker.o cudaCracker.cuda_o

gpu: slave_md5cracker_gpu

slave_md5cracker_gpu: $(objects) $(cuda_objs) 
	$(CC) $(CFLAGS) -o $@ $^ -Wl,-Bstatic $(xmlrpclibs) -Wl,-Bdynamic -lpthread -lstdc++ $(xmlwwwlibs) $(LDFLAGS) $(LIBS)  

cudaCracker.cuda_o: cudaCracker.cu
	$(NVCC) -c $(INCLUDES) -o $@ $< ; printf "\n"

cudaCracker.o: cudaCracker.cpp cudaCracker.h
	$(CC) -c $(INCLUDES) -o $@ $< $(CFLAGS) ; printf "\n"


##############################
clean:
	rm -f slave_md5cracker* *.o *cuda_o

#clean3:
#	ls -l | grep -i "rwx" | awk '{print "rm " $9}' | bash
