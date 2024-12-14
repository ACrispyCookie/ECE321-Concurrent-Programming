./target/test: ./target/test.o ../assignment2/target/libmythreads.a
	gcc -Wall -g ./target/test.o -o ./target/test -lmythreads -L../assignment2/target/

./target/test.o: ./src/test.c
	gcc -Wall -g -c ./src/test.c -o ./target/test.o
