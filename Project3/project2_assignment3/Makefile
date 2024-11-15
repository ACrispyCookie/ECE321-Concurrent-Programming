ASSIGNMENT3_SRC = ./src/assignment3.c
ASSIGNMENT3 = ./target/assignment3
LIBMYSEM_DIR = ../assignment1/target
LIBMYSEM = $(LIBMYSEM_DIR)/libmysem.a

$(ASSIGNMENT3): $(ASSIGNMENT3_SRC) $(LIBMYSEM)
	gcc -Wall -g $(ASSIGNMENT3_SRC) -o $(ASSIGNMENT3) -lpthread -lmysem -L$(LIBMYSEM_DIR)

clear: $(ASSIGNMENT3)
	rm -rf $(ASSIGNMENT3)
