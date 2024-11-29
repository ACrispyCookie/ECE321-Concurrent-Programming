ASSIGNMENT3_SRC = ./src/assignment3.c
ASSIGNMENT3 = ./target/assignment3

$(ASSIGNMENT3): $(ASSIGNMENT3_SRC)
	gcc -Wall -g $(ASSIGNMENT3_SRC) -o $(ASSIGNMENT3) -lpthread

clear: $(ASSIGNMENT3)
	rm -rf $(ASSIGNMENT3)
