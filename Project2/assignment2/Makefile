./target/assignment2: ./src/assignment2.c ../assignment1/target/libmysem.a
	gcc -Wall -g ./src/assignment2.c -o ./target/assignment2 -lpthread -lmysem -L../assignment1/target

clear: ./target/assignment2
	rm -rf ./target/assignment2
