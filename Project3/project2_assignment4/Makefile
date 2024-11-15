ASSIGNMENT4_SRC = ./src/assignment4.c
ASSIGNMENT4 = ./target/assignment4
LIBMYSEM_DIR = ../assignment1/target
LIBMYSEM = $(LIBMYSEM_DIR)/libmysem.a

$(ASSIGNMENT4): $(ASSIGNMENT4_SRC) $(LIBMYSEM)
	gcc -Wall -g $(ASSIGNMENT4_SRC) -o $(ASSIGNMENT4) -lpthread -lmysem -L$(LIBMYSEM_DIR)

clear: $(ASSIGNMENT4)
	rm -rf $(ASSIGNMENT4)
