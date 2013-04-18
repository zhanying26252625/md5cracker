# -static prevents any dynamic linking from being used
# run ldconfig to refresh the ld cache after manually build and install xmlrpc library

# How to mix static library and dynamic libray
# gcc <options> <sources> -o <binary> -Wl,-Bstatic <list of static libs> -Wl,Bdynamic <list of 
# dynamic libs>
# gcc program.o -llib1 -Wl,-Bstatic -llib2 -Wl,-Bdynamic -llib3

CC = g++

# sometimes -g would not work for showing symbols in gdb, we use -gstabs+ instead
# maybe it's something related to debug info format
CFLAGS = -std=gnu++11 -Wall -o3 -g  

headers = ../configure.h masterMD5Cracker.h logManager.h slaveProxy.h 

objects = main.o masterMD5Cracker.o logManager.o slaveProxy.o

xmlrpclibs = -L/usr/local/lib -lxmlrpc++ -lxmlrpc -lxmlrpc_util -lxmlrpc_xmlparse -lxmlrpc_xmltok 

target: master_md5cracker

master_md5cracker: $(objects) 
	$(CC) $(CFLAGS) -o $@ $^ -Wl,-Bstatic $(xmlrpclibs) -Wl,-Bdynamic -lpthread -lstdc++  

%.o: %.c $(headers)
	$(CC) -$(CFLAGS) -c -o $@ $<

clean:
	rm -f master_md5cracker *.o

#clean3:
#	ls -l | grep -i "rwx" | awk '{print "rm " $9}' | bash
