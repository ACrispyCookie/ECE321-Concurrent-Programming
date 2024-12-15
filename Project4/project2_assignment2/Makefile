ASSIGNMENT2_SRC = ./src/assignment2.c
ASSIGNMENT2 = ./target/assignment2
LIBMYTHREADS_DIR = ../assignment2/target
LIBMYTHREADS = $(LIBMYTHREADS_DIR)/libmythreads.a

$(ASSIGNMENT2): $(ASSIGNMENT2_SRC) $(LIBMYTHREADS)
	gcc -Wall -g $(ASSIGNMENT2_SRC) -o $(ASSIGNMENT2) -lmythreads -L$(LIBMYTHREADS_DIR)

clear: $(ASSIGNMENT2)
	rm -rf $(ASSIGNMENT2)
