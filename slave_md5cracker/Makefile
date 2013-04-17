# -static prevents any dynamic linking from being used
# run ldconfig to refresh the ld cache after manually build and install xmlrpc library

# How to mix static library and dynamic libray
# gcc <options> <sources> -o <binary> -Wl,-Bstatic <list of static libs> -Wl,Bdynamic <list of 
# dynamic libs>
# gcc program.o -llib1 -Wl,-Bstatic -llib2 -Wl,-Bdynamic -llib3

CC = g++

CFLAGS = -Wall -o3 -g

headers = ../configure.h

objects = main.o 

xmlrpclibs = -L/usr/local/lib -lxmlrpc++ -lxmlrpc -lxmlrpc_util -lxmlrpc_xmlparse -lxmlrpc_xmltok 

target: slave_md5cracker

slave_md5cracker: $(objects) 
	$(CC) $(CFLAGS) -o $@ $^ -Wl,-Bstatic $(xmlrpclibs) -Wl,-Bdynamic -lpthread 

%.o: %.c $(headers)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f slave_md5cracker *.o

#clean3:
#	ls -l | grep -i "rwx" | awk '{print "rm " $9}' | bash
