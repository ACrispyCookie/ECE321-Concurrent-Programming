ASSIGNMENT4_SRC = ./src/assignment4.c
ASSIGNMENT4 = ./target/assignment4
LIBMYTHREADS_DIR = ../assignment2/target
LIBMYTHREADS = $(LIBMYTHREADS_DIR)/libmythreads.a

$(ASSIGNMENT4): $(ASSIGNMENT4_SRC) $(LIBMYTHREADS)
	gcc -Wall -g $(ASSIGNMENT4_SRC) -o $(ASSIGNMENT4) -lmythreads -L$(LIBMYTHREADS_DIR)

clear: $(ASSIGNMENT4)
	rm -rf $(ASSIGNMENT4)
