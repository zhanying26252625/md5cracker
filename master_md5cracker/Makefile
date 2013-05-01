# -static prevents any dynamic linking from being used
# run ldconfig to refresh the ld cache after manually build and install xmlrpc library

# How to mix static library and dynamic libray
# gcc <options> <sources> -o <binary> -Wl,-Bstatic <list of static libs> -Wl,Bdynamic <list of 
# dynamic libs>
# gcc program.o -llib1 -Wl,-Bstatic -llib2 -Wl,-Bdynamic -llib3

CC = g++

# sometimes -g would not work for showing symbols in gdb, we use -gstabs+ instead
# maybe it's something related to debug info format
CFLAGS = -std=c++11 -Wall -g  

headers = ../configure.h masterMD5Cracker.h logManager.h slaveProxy.h md5.h passGenerator.h 

objects = main.o masterMD5Cracker.o logManager.o slaveProxy.o md5.o passGenerator.o

xmlrpclibs = -L/usr/local/lib -lxmlrpc_client++ -lxmlrpc_client -lxmlrpc_server_pstream++ -lxmlrpc_server++ -lxmlrpc_server  -lxmlrpc++ -lxmlrpc -lxmlrpc_util -lxmlrpc_xmlparse -lxmlrpc_xmltok -lxmlrpc_packetsocket 

xmlwwwlibs = -lwwwxml -lxmltok -lxmlparse -lwwwinit -lwwwapp -lwwwhtml -lwwwtelnet -lwwwnews -lwwwhttp -lwwwmime -lwwwgopher -lwwwftp -lwwwfile -lwwwdir -lwwwcache -lwwwstream -lwwwmux -lwwwtrans -lwwwcore -lwwwutils -L/usr/lib -lmd5 

target: master_md5cracker

master_md5cracker: $(objects) 
	$(CC) $(CFLAGS) -o $@ $^ -Wl,-Bstatic $(xmlrpclibs) -Wl,-Bdynamic -lpthread -lstdc++ $(xmlwwwlibs)  

# $^ means all
# $< means first one

#Be care of the suffix
%.o: %.cpp $(headers)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f master_md5cracker *.o log.txt

#clean3:
#	ls -l | grep -i "rwx" | awk '{print "rm " $9}' | bash
