MASTER_DIR = ./master_md5cracker

SLAVE_DIR = ./slave_md5cracker

all : master slave cscope.out tags

master:
		$(MAKE) -C $(MASTER_DIR) -f ./Makefile

slave:
		$(MAKE) -C $(SLAVE_DIR) -f ./Makefile

tags:
		ctags -R

cscope.out: cscope.files
		cscope -Rbkq -i cscope.files

#support c++, find should be followed by absolute path
cscope.files:
		find . -name "*.h" -o -name "*.c" -o -name "*.cpp" > cscope.files

clean:
		$(MAKE) -C $(MASTER_DIR) -f ./Makefile clean;  $(MAKE) -C $(SLAVE_DIR) -f ./Makefile clean; rm -f log.txt tags cscope*
