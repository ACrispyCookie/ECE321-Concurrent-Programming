TARGET_DIR = ./target
LIST_HEADER = ./src/list.h
LIST_SRC = ./src/list.c
LIST_OBJ = $(TARGET_DIR)/list.o

all: $(LIST_OBJ)

clear: 
	rm -rf $(LIST_OBJ)

$(LIST_OBJ): $(LIST_SRC) $(LIST_HEADER)
	gcc -Wall -g -c $(LIST_SRC) -o $(LIST_OBJ)