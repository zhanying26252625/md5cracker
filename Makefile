MASTER_DIR = ./master_md5cracker

SLAVE_DIR = ./slave_md5cracker

all : master slave

master:
		$(MAKE) -C $(MASTER_DIR) -f ./Makefile

slave:
		$(MAKE) -C $(SLAVE_DIR) -f ./Makefile

clean:
		$(MAKE) -C $(MASTER_DIR) -f ./Makefile clean;  $(MAKE) -C $(SLAVE_DIR) -f ./Makefile clean
