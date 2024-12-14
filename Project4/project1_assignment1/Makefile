./target/test: ./target/test.o ./target/pipes.o ../assignment2/target/libmythreads.a
	gcc -Wall -g ./target/test.o ./target/pipes.o -o ./target/test -lmythreads -L../assignment2/target/

./target/test.o: ./tests/test.c ./src/pipes.h
	gcc -Wall -g -c ./tests/test.c -o ./target/test.o

./target/pipes.o: ./src/pipes.c ./src/pipes.h
	gcc -Wall -g -c ./src/pipes.c -o ./target/pipes.o
