ASSIGNMENT2_SRC = ./src/assignment2.c
ASSIGNMENT2 = ./target/assignment2
LIBMYSEM_DIR = ../assignment1/target
LIBMYSEM = $(LIBMYSEM_DIR)/libmysem.a

$(ASSIGNMENT2): $(ASSIGNMENT2_SRC) $(LIBMYSEM)
	gcc -Wall -g $(ASSIGNMENT2_SRC) -o $(ASSIGNMENT2) -lpthread -lmysem -L$(LIBMYSEM_DIR)

clear: $(ASSIGNMENT2)
	rm -rf $(ASSIGNMENT2)
