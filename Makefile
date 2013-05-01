MASTER_DIR = ./master_md5cracker

SLAVE_DIR = ./slave_md5cracker

APP_DIR = ./app_md5cracker

all : master slave app cscope.out tags

master:
		$(MAKE) -C $(MASTER_DIR) -f ./Makefile

slave:
		$(MAKE) -C $(SLAVE_DIR) -f ./Makefile
	
app:
		$(MAKE) -C $(APP_DIR) -f ./Makefile

tags:
		ctags -R --c++-kinds=+p --fields=+iaS --extra=+q .

cscope.out: cscope.files
		cscope -Rbkq -i cscope.files

cscope.files:
		find . -name "*.h" -o -name "*.c" -o -name "*.cpp" -o -name "*.cuda" > cscope.files

clean:
		$(MAKE) -C $(MASTER_DIR) -f ./Makefile clean;  $(MAKE) -C $(SLAVE_DIR) -f ./Makefile clean; $(MAKE) -C $(APP_DIR) -f ./Makefile clean; rm -f log.txt tags cscope*
