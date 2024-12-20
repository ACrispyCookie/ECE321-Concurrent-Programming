ASSIGNMENT3_SRC = ./src/assignment3.c
ASSIGNMENT3 = ./target/assignment3
LIBMYTHREADS_DIR = ../assignment2/target
LIBMYTHREADS = $(LIBMYTHREADS_DIR)/libmythreads.a

$(ASSIGNMENT3): $(ASSIGNMENT3_SRC) $(LIBMYTHREADS)
	gcc -Wall -g $(ASSIGNMENT3_SRC) -o $(ASSIGNMENT3) -lmythreads -L$(LIBMYTHREADS_DIR)

clear: $(ASSIGNMENT3)
	rm -rf $(ASSIGNMENT3)
